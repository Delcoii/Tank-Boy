#ifndef MAP_GENERATION_H
#define MAP_GENERATION_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

// Block types
typedef enum {
    BLOCK_GROUND,
    BLOCK_GRASS
} BlockType;

// Block structure
typedef struct {
    int x, y;           // Top-left position
    int width, height;  // Size (usually 50x50)
    BlockType type;
} Block;

// Map structure
typedef struct {
    Block* blocks;      // Dynamic array of blocks
    size_t block_count; // Number of blocks
    size_t block_capacity; // Allocated capacity
    int map_width;      // Map dimensions
    int map_height;
} Map;

// Map management functions
bool map_load(Map* map, const char* csv_path);
void map_free(Map* map);
bool map_init(Map* map);

// ROI (Region of Interest) query for collision detection
// Returns blocks within the specified rectangle
size_t map_query_roi(const Map* map, int center_x, int center_y, int width, int height, 
                     Block* out_blocks, size_t max_blocks);

// Check if a point collides with any block
bool map_point_collision(const Map* map, int x, int y);

// Check if a rectangle collides with any block
bool map_rect_collision(const Map* map, int x, int y, int width, int height);

// Get ground level at specific x coordinate (for tank landing)
int map_get_ground_level(const Map* map, int x);

// Rendering functions
void map_draw(const Map* map, double camera_x, double camera_y, int buffer_width, int buffer_height);

// Helper functions
BlockType map_string_to_block_type(const char* type_str);
ALLEGRO_COLOR map_get_block_color(BlockType type);

#endif // MAP_GENERATION_H
