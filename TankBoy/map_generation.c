#define _CRT_SECURE_NO_WARNINGS
#include "map_generation.h"
#include "ini_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro5/allegro5.h>

#define INITIAL_BLOCK_CAPACITY 1000
#define INITIAL_SPAWN_CAPACITY 10

// Global configuration cache
static MapConfig g_map_config = {0};

map_sprites_t map_sprites;

// Initialize configuration (load once)
void map_config_init(void) {
    IniParser* parser = ini_parser_create();

    // Load config.ini file (try multiple paths)
    bool loaded = false;
    if (ini_parser_load_file(parser, "config.ini")) {
        loaded = true;
    }
    else if (ini_parser_load_file(parser, "TankBoy/config.ini")) {
        loaded = true;
    }

    if (!loaded) {
        printf("Warning: Could not load config.ini from any location, using default values\n");
    }
    else {
        printf("Successfully loaded config.ini\n");
    }

    // Load all configuration values (with fallbacks)
    g_map_config.buffer_width = ini_parser_get_int(parser, "Buffer", "buffer_width", 1280);
    g_map_config.buffer_height = ini_parser_get_int(parser, "Buffer", "buffer_height", 720);
    g_map_config.block_size = ini_parser_get_int(parser, "Map", "block_size", 50);
    g_map_config.map_width_multiplier = ini_parser_get_int(parser, "Map", "map_width_multiplier", 10);
    g_map_config.map_height_multiplier = ini_parser_get_int(parser, "Map", "map_height_multiplier", 3);

    // Load enemy behavior settings
    g_map_config.enemy_jump_interval_min = ini_parser_get_double(parser, "Enemy", "enemy_jump_interval_min", 1.8);
    g_map_config.enemy_jump_interval_max = ini_parser_get_double(parser, "Enemy", "enemy_jump_interval_max", 2.2);

    // Load enemy physics settings
    g_map_config.enemy_base_speed = ini_parser_get_double(parser, "Enemy", "enemy_base_speed", 0.1);
    g_map_config.enemy_speed_per_difficulty = ini_parser_get_double(parser, "Enemy", "enemy_speed_per_difficulty", 0.5);

    ini_parser_destroy(parser);

    // Calculate derived values
    g_map_config.map_width = g_map_config.buffer_width * g_map_config.map_width_multiplier;
    g_map_config.map_height = g_map_config.buffer_height * g_map_config.map_height_multiplier;
}

// Cleanup configuration (for future use if needed)
void map_config_cleanup(void) {
    // Nothing to cleanup for now
}

// Get configuration (read-only access)
const MapConfig* map_get_config(void) {
    return &g_map_config;
}

// Initialize empty map
bool map_init(Map* map) {
    if (!map) return false;

    // Ensure configuration is loaded
    const MapConfig* config = map_get_config();

    // Use cached configuration values
    map->map_width = config->map_width;
    map->map_height = config->map_height;

    map->blocks = malloc(INITIAL_BLOCK_CAPACITY * sizeof(Block));
    if (!map->blocks) return false;

    map->block_count = 0;
    map->block_capacity = INITIAL_BLOCK_CAPACITY;

    return true;
}

// Free map memory
void map_free(Map* map) {
    if (map && map->blocks) {
        free(map->blocks);
        map->blocks = NULL;
        map->block_count = 0;
        map->block_capacity = 0;
    }
}

// Convert string to block type
BlockType map_string_to_block_type(const char* type_str) {
    if (strcmp(type_str, "grass") == 0) {
        return BLOCK_GRASS;
    }
    if (strcmp(type_str, "ground") == 0) {
        return BLOCK_GROUND;
    }
    return BLOCK_GROUND; // Default to ground
}

// Get color for block type
ALLEGRO_COLOR map_get_block_color(BlockType type) {
    switch (type) {
        case BLOCK_GRASS:
            return al_map_rgb(34, 139, 34);  // Forest green
        case BLOCK_GROUND:
        default:
            return al_map_rgb(139, 69, 19);  // Saddle brown
    }
}

// Add block to map (resize array if needed)
static bool map_add_block(Map* map, const Block* block) {
    if (map->block_count >= map->block_capacity) {
        size_t new_capacity = map->block_capacity * 2;
        Block* new_blocks = realloc(map->blocks, new_capacity * sizeof(Block));
        if (!new_blocks) return false;

        map->blocks = new_blocks;
        map->block_capacity = new_capacity;
    }

    map->blocks[map->block_count] = *block;
    map->block_count++;
    return true;
}

// Load map from CSV file
bool map_load(Map* map, const char* csv_path) {
    if (!map || !csv_path) return false;

    FILE* file = fopen(csv_path, "r");
    if (!file) {
        printf("Failed to open map file: %s\n", csv_path);
        return false;
    }

    // Initialize map
    if (!map_init(map)) {
        fclose(file);
        return false;
    }

    // Extract stage number from filename (e.g., "stage1.csv" -> stage = 1)
    const char* filename = strrchr(csv_path, '/');
    if (!filename) filename = strrchr(csv_path, '\\');
    if (!filename) filename = csv_path;
    else filename++; // Skip the slash

    if (strncmp(filename, "stage", 5) == 0) {
        map->stage = atoi(filename + 5); // Skip "stage" and convert number
        printf("Loading stage %d map from %s\n", map->stage, csv_path);
    } else {
        map->stage = 1; // Default to stage 1 if filename doesn't match pattern
        printf("Could not determine stage from filename, defaulting to stage 1\n");
    }

    char line[256];
    bool first_line = true;

    while (fgets(line, sizeof(line), file)) {
        // Skip header line
        if (first_line) {
            first_line = false;
            continue;
        }

        // Parse CSV line: type,start_x,start_y,end_x,end_y
        char* token = strtok(line, ",");
        if (!token) continue;

        char type_str[32];
        strncpy(type_str, token, sizeof(type_str) - 1);
        type_str[sizeof(type_str) - 1] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        int start_x = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        int start_y = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        int end_x = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        int end_y = atoi(token);

        // Create block
        Block block;
        block.x = start_x;
        block.y = start_y;
        block.width = end_x - start_x;
        block.height = end_y - start_y;
        block.type = map_string_to_block_type(type_str);

        // Add to map
        if (!map_add_block(map, &block)) {
            map_free(map);
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}



// Check point collision with any block
bool map_point_collision(const Map* map, int x, int y) {
    if (!map) return false;

    for (size_t i = 0; i < map->block_count; i++) {
        const Block* block = &map->blocks[i];
        if (x >= block->x && x < block->x + block->width &&
            y >= block->y && y < block->y + block->height) {
            return true;
        }
    }
    return false;
}

// Check rectangle collision with any block
bool map_rect_collision(const Map* map, int x, int y, int width, int height) {
    if (!map) return false;

    for (size_t i = 0; i < map->block_count; i++) {
        const Block* block = &map->blocks[i];

        // AABB collision detection
        if (x < block->x + block->width && x + width > block->x &&
            y < block->y + block->height && y + height > block->y) {
            return true;
        }
    }
    return false;
}

// Get ground level at specific x coordinate (improved version)
int map_get_ground_level(const Map* map, int x, int tank_width) {
    // Get cached configuration
    const MapConfig* config = map_get_config();
    int map_height = config->map_height;

    if (!map) return map_height; // Return bottom if no map

    int ground_level = map_height;

    // Check blocks within tank width for more accurate ground detection
    for (size_t i = 0; i < map->block_count; i++) {
        const Block* block = &map->blocks[i];

        // Check if block overlaps with tank's x range
        if (block->x < x + tank_width && block->x + block->width > x) {
            // Find the highest solid surface (top of block)
            if (block->y < ground_level) {
                ground_level = block->y;
            }
        }
    }

    return ground_level;
}

// Render map within camera view
void map_draw(const Map* map, double camera_x, double camera_y, int buffer_width, int buffer_height) {
    if (!map) return;

    // Calculate visible area
    int left = (int)camera_x;
    int right = (int)camera_x + buffer_width;
    int top = (int)camera_y;
    int bottom = (int)camera_y + buffer_height;

    // Draw blocks that are visible
    for (size_t i = 0; i < map->block_count; i++) {
        const Block* block = &map->blocks[i];

        // Check if block is visible
        if (block->x + block->width >= left && block->x <= right &&
            block->y + block->height >= top && block->y <= bottom) {

            // Calculate screen position
            float screen_x = block->x - camera_x;
            float screen_y = block->y - camera_y;

            // Draw block based on stage and block type
            int sprite_index = map->stage - 1; // Convert stage 1,2,3 to index 0,1,2
            
            if (sprite_index < 0 || sprite_index > 2) {
                sprite_index = 0; // Default to stage 1 sprites if out of range
            }
            
            if (block->type == BLOCK_GROUND) {
                int bitmap_width = al_get_bitmap_width(map_sprites.ground_sprites[sprite_index]);
                int bitmap_height = al_get_bitmap_height(map_sprites.ground_sprites[sprite_index]);
                // al_draw_bitmap(map_sprites.ground_sprites[sprite_index], screen_x, screen_y, 0);
                al_draw_scaled_bitmap(map_sprites.ground_sprites[sprite_index],
                                    0, 0,
                                    bitmap_width, bitmap_height,
                                    screen_x, screen_y, block->width, block->height, 0);

            } else if (block->type == BLOCK_GRASS) {
                int bitmap_width = al_get_bitmap_width(map_sprites.grass_sprites[sprite_index]);
                int bitmap_height = al_get_bitmap_height(map_sprites.grass_sprites[sprite_index]);
                al_draw_scaled_bitmap(map_sprites.grass_sprites[sprite_index],
                                    0, 0,
                                    bitmap_width, bitmap_height,
                                    screen_x, screen_y, block->width, block->height, 0);
            }
        }
    }
}

// Configuration functions, used in other files
int map_get_block_size(void) {
    const MapConfig* config = map_get_config();
    return config->block_size;
}

int map_get_map_width(void) {
    const MapConfig* config = map_get_config();
    return config->map_width;
}

int map_get_map_height(void) {
    const MapConfig* config = map_get_config();
    return config->map_height;
}

// Convert string to spawn type
static SpawnType spawn_string_to_type(const char* type_str) {
    if (strcmp(type_str, "tank") == 0) {
        return SPAWN_TANK;
    }
    return SPAWN_TANK; // Default to tank
}

// Initialize spawn points collection
bool spawn_points_init(SpawnPoints* spawns) {
    if (!spawns) return false;

    spawns->points = malloc(INITIAL_SPAWN_CAPACITY * sizeof(SpawnPoint));
    if (!spawns->points) return false;

    spawns->count = 0;
    spawns->capacity = INITIAL_SPAWN_CAPACITY;

    return true;
}

// Free spawn points memory
void spawn_points_free(SpawnPoints* spawns) {
    if (spawns && spawns->points) {
        free(spawns->points);
        spawns->points = NULL;
        spawns->count = 0;
        spawns->capacity = 0;
    }
}

// Add spawn point to collection (resize array if needed)
static bool spawn_points_add(SpawnPoints* spawns, const SpawnPoint* point) {
    if (spawns->count >= spawns->capacity) {
        size_t new_capacity = spawns->capacity * 2;
        SpawnPoint* new_points = realloc(spawns->points, new_capacity * sizeof(SpawnPoint));
        if (!new_points) return false;

        spawns->points = new_points;
        spawns->capacity = new_capacity;
    }

    spawns->points[spawns->count] = *point;
    spawns->count++;
    return true;
}

// Load spawn points from CSV file
bool spawn_points_load(SpawnPoints* spawns, const char* csv_path) {
    if (!spawns || !csv_path) return false;

    FILE* file = fopen(csv_path, "r");
    if (!file) {
        printf("Info: No spawn points file found at: %s (using default position)\n", csv_path);
        return false; // Not an error, just use default spawn
    }

    // Initialize spawn points collection
    if (!spawn_points_init(spawns)) {
        fclose(file);
        return false;
    }

    char line[256];
    bool first_line = true;

    while (fgets(line, sizeof(line), file)) {
        // Skip header line
        if (first_line) {
            first_line = false;
            continue;
        }

        // Parse CSV line: x,y,spawn_type
        char* token = strtok(line, ",");
        if (!token) continue;
        int x = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        int y = atoi(token);

        token = strtok(NULL, ",\n\r");
        if (!token) continue;

        // Remove trailing whitespace
        char* end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
            *end = '\0';
            end--;
        }

        // Create spawn point
        SpawnPoint point;
        point.x = x;
        point.y = y;
        point.type = spawn_string_to_type(token);

        // Add to collection
        if (!spawn_points_add(spawns, &point)) {
            spawn_points_free(spawns);
            fclose(file);
            return false;
        }
    }

    fclose(file);
    printf("Loaded %zu spawn points from %s\n", spawns->count, csv_path);
    return true;
}

// Get tank spawn point (returns first tank spawn found, or NULL)
SpawnPoint* spawn_points_get_tank_spawn(const SpawnPoints* spawns) {
    if (!spawns) return NULL;

    for (size_t i = 0; i < spawns->count; i++) {
        if (spawns->points[i].type == SPAWN_TANK) {
            return &spawns->points[i];
        }
    }
    return NULL;
}

//sprite





void map_sprites_init(const char* sprite_path)
{
    map_sprites._sheet = al_load_bitmap(sprite_path);
    if (map_sprites._sheet == NULL) {
        printf("wrong location of map sprite!!\n");
    }
    
    map_sprites.ground_sprites = malloc(3 * sizeof(ALLEGRO_BITMAP*));
    map_sprites.grass_sprites = malloc(3 * sizeof(ALLEGRO_BITMAP*));

    int sheet_width = 1912;
    int block_width = 1912 / 4;
    int sheet_height = 957;
    int block_height = 957 / 2;
    
    // sprites idx 0 == stage 1
    map_sprites.ground_sprites[0] = al_create_sub_bitmap(map_sprites._sheet, 0, 0, block_width, block_height);
    map_sprites.grass_sprites[0] = al_create_sub_bitmap(map_sprites._sheet, block_width, 0, block_width, block_height);
    map_sprites.grass_sprites[1] = al_create_sub_bitmap(map_sprites._sheet, block_width * 2, 0, block_width, block_height);
    map_sprites.ground_sprites[1] = al_create_sub_bitmap(map_sprites._sheet, block_width * 3, 0, block_width, block_height);
    map_sprites.ground_sprites[2] = al_create_sub_bitmap(map_sprites._sheet, 0, block_height, block_width, block_height);
    map_sprites.grass_sprites[2] = al_create_sub_bitmap(map_sprites._sheet, block_width, block_height, block_width, block_height);
}


//void must_init(bool test, const char* description)
//{
//    if (test) return;
//
//    printf("couldn't initialize %s\n", description);
//    exit(1);
//}


void map_sprites_deinit()
{
    // Free individual sprites
    for (int i = 0; i < 3; i++) {
        if (map_sprites.ground_sprites[i]) {
            al_destroy_bitmap(map_sprites.ground_sprites[i]);
        }
        if (map_sprites.grass_sprites[i]) {
            al_destroy_bitmap(map_sprites.grass_sprites[i]);
        }
    }
    
    // Free sprite arrays
    if (map_sprites.ground_sprites) {
        free(map_sprites.ground_sprites);
    }
    if (map_sprites.grass_sprites) {
        free(map_sprites.grass_sprites);
    }

    // Free sprite sheet
    if (map_sprites._sheet) {
        al_destroy_bitmap(map_sprites._sheet);
    }
}