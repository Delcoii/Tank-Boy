#ifndef MAP_GENERATION_H
#define MAP_GENERATION_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

/* ��� Ÿ�� */
typedef enum {
    BLOCK_GROUND,
    BLOCK_GRASS
} BlockType;

/* ��� ����ü */
typedef struct {
    int x, y;
    int width, height;
    BlockType type;
} Block;

/* �� ����ü */
typedef struct {
    Block* blocks;
    size_t block_count;
    size_t block_capacity;
    int map_width;
    int map_height;
} Map;

/* �� ���� */
bool map_load(Map* map, const char* csv_path);
bool map_init(Map* map);
void map_free(Map* map);

/* �浹 �� ROI */
size_t map_query_roi(const Map* map, int center_x, int center_y, int width, int height, 
                     Block* out_blocks, size_t max_blocks);
bool map_point_collision(const Map* map, int x, int y);
bool map_rect_collision(const Map* map, int x, int y, int width, int height);
int map_get_ground_level(const Map* map, int x);

/* ������ */
void map_draw(const Map* map, double camera_x, double camera_y, int buffer_width, int buffer_height);

/* ���� */
BlockType map_string_to_block_type(const char* type_str);
ALLEGRO_COLOR map_get_block_color(BlockType type);

#endif // MAP_GENERATION_H
