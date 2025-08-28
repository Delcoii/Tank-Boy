#define _CRT_SECURE_NO_WARNINGS
#include "map_generation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BLOCK_CAPACITY 1000
#define BLOCK_SIZE 50

// Initialize empty map
bool map_init(Map* map) {
    if (!map) return false;
    
    map->blocks = malloc(INITIAL_BLOCK_CAPACITY * sizeof(Block));
    if (!map->blocks) return false;
    
    map->block_count = 0;
    map->block_capacity = INITIAL_BLOCK_CAPACITY;
    map->map_width = 12800;
    map->map_height = 2160;
    
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
            printf("Failed to add block to map\n");
            map_free(map);
            fclose(file);
            return false;
        }
    }
    
    fclose(file);
    printf("Loaded %zu blocks from %s\n", map->block_count, csv_path);
    return true;
}

// Query blocks in ROI (Region of Interest)
size_t map_query_roi(const Map* map, int center_x, int center_y, int width, int height, 
                     Block* out_blocks, size_t max_blocks) {
    if (!map || !out_blocks || max_blocks == 0) return 0;
    
    int left = center_x - width / 2;
    int right = center_x + width / 2;
    int top = center_y - height / 2;
    int bottom = center_y + height / 2;
    
    size_t found_count = 0;
    
    for (size_t i = 0; i < map->block_count && found_count < max_blocks; i++) {
        const Block* block = &map->blocks[i];
        
        // Check if block intersects with ROI rectangle
        if (block->x < right && block->x + block->width > left &&
            block->y < bottom && block->y + block->height > top) {
            out_blocks[found_count] = *block;
            found_count++;
        }
    }
    
    return found_count;
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

// Get ground level at specific x coordinate
int map_get_ground_level(const Map* map, int x) {
    if (!map) return map->map_height; // Return bottom if no map
    
    int ground_level = map->map_height;
    
    for (size_t i = 0; i < map->block_count; i++) {
        const Block* block = &map->blocks[i];
        
        // Check if block is at this x coordinate
        if (x >= block->x && x < block->x + block->width) {
            // Find the highest (lowest y value) block
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
