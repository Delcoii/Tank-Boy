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


// Check if a point collides with any block
bool map_point_collision(const Map* map, int x, int y);

// Check if a rectangle collides with any block
bool map_rect_collision(const Map* map, int x, int y, int width, int height);

// Get ground level at specific x coordinate (for tank landing)
int map_get_ground_level(const Map* map, int x, int tank_width);

// Rendering functions
void map_draw(const Map* map, double camera_x, double camera_y, int buffer_width, int buffer_height);

// Helper functions
BlockType map_string_to_block_type(const char* type_str);
ALLEGRO_COLOR map_get_block_color(BlockType type);

// Configuration structure
typedef struct {
    int block_size;
    int map_width;
    int map_height;
    int buffer_width;
    int buffer_height;
    int map_width_multiplier;
    int map_height_multiplier;
    
    // Enemy behavior settings
    double enemy_jump_interval_min;
    double enemy_jump_interval_max;
    
    // Enemy physics settings
    double enemy_base_speed;
    double enemy_speed_per_difficulty;
    double enemy_base_acceleration;
    double enemy_acceleration_per_difficulty;
    double enemy_base_friction;
    double enemy_friction_per_difficulty;
} MapConfig;

// Configuration functions
void map_config_init(void);
void map_config_cleanup(void);
const MapConfig* map_get_config(void);
int map_get_block_size(void);
int map_get_map_width(void);
int map_get_map_height(void);

#endif // MAP_GENERATION_H
