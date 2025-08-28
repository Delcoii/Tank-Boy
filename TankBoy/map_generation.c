#define _CRT_SECURE_NO_WARNINGS
#include "map_generation.h"
#include "ini_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BLOCK_CAPACITY 1000

// Global configuration cache
static MapConfig g_map_config = {0};

// Initialize configuration (load once)
void map_config_init(void) {
    IniParser* parser = ini_parser_create();
    
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
    g_map_config.enemy_base_speed = ini_parser_get_double(parser, "Enemy", "enemy_base_speed", 2.0);
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
            
            // Draw block
            ALLEGRO_COLOR color = map_get_block_color(block->type);
            al_draw_filled_rectangle(screen_x, screen_y, 
                                   screen_x + block->width, screen_y + block->height, 
                                   color);
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
