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
    int x, y;
    int width, height;
    BlockType type;
} Block;

// Spawn point types
typedef enum {
    SPAWN_TANK
} SpawnType;

// Spawn point structure
typedef struct {
    int x, y;
    SpawnType type;
} SpawnPoint;

// Spawn points collection
typedef struct {
    SpawnPoint* points;
    size_t count;
    size_t capacity;
} SpawnPoints;

// Map structure
typedef struct {
    Block* blocks;
    size_t block_count;
    size_t block_capacity;
    int map_width;
    int map_height;
    int stage;  // 현재 스테이지 번호 (1, 2, 3)
} Map;

// sprite structure
typedef struct _map_sprites
{
    ALLEGRO_BITMAP* _sheet;
    ALLEGRO_BITMAP** grass_sprites;
    ALLEGRO_BITMAP** ground_sprites;
    
} map_sprites_t;

// Map management
bool map_load(Map* map, const char* csv_path);
bool map_init(Map* map);
void map_free(Map* map);

// Spawn point management
bool spawn_points_init(SpawnPoints* spawns);
void spawn_points_free(SpawnPoints* spawns);
bool spawn_points_load(SpawnPoints* spawns, const char* csv_path);
SpawnPoint* spawn_points_get_tank_spawn(const SpawnPoints* spawns);

// Collision detection ROI
//size_t map_query_roi(const Map* map, int center_x, int center_y, int width, int height, 
//                     Block* out_blocks, size_t max_blocks);

// Check if a point collides with any block
bool map_point_collision(const Map* map, int x, int y);
bool map_rect_collision(const Map* map, int x, int y, int width, int height);

// Get ground level at specific x coordinate (for tank landing)
int map_get_ground_level(const Map* map, int x, int tank_width);

// Rendering
void map_draw(const Map* map, double camera_x, double camera_y, int buffer_width, int buffer_height);

// Utilities
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
} MapConfig;

// Configuration functions
void map_config_init(void);
void map_config_cleanup(void);
const MapConfig* map_get_config(void);
int map_get_block_size(void);
int map_get_map_width(void);
int map_get_map_height(void);

//sprite
void map_sprites_init(const char* sprite_path);
//void must_init(bool test, const char* description);
void map_sprites_deinit();

#endif // MAP_GENERATION_H
