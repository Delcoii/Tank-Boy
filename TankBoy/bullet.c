#include "bullet.h"
#include "map_generation.h"
#include "ini_parser.h"
#include <allegro5/allegro_primitives.h>


void bullets_init(Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_bullets; i++) {
        bullets[i].alive = false;
        bullets[i].from_enemy = false;
    }
}

void bullets_update(Bullet* bullets, int max_bullets, const struct Map* map) {
    // Load bullet physics settings from config.ini
    // TODO : remove reading ini online
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "config.ini");
    const double bullet_gravity = ini_parser_get_double(parser, "Bullets", "bullet_gravity", 0.3);
    const int map_width = map_get_map_width(); // Use function instead of hardcoded value
    const int map_height = map_get_map_height(); // Use function instead of hardcoded value
    ini_parser_destroy(parser);
    
    for (int i = 0; i < max_bullets; i++) {
        if (!bullets[i].alive) continue;

        // Gravity for cannon bullets
        if (bullets[i].weapon == 1) bullets[i].vy += bullet_gravity;

        // Store old position for collision checking
        double old_x = bullets[i].x;
        double old_y = bullets[i].y;
        
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;

        // Check collision with map
        if (map && map_point_collision(map, (int)bullets[i].x, (int)bullets[i].y)) {
            bullets[i].alive = false;
            continue;
        }

        // Remove if out of bounds
        if (bullets[i].x < 0 || bullets[i].x > map_width || bullets[i].y > map_height || bullets[i].y < 0)
            bullets[i].alive = false;
    }
}

void bullets_draw(Bullet* bullets, int max_bullets, double camera_x, double camera_y) {
    for (int i = 0; i < max_bullets; i++) {
        if (!bullets[i].alive) continue;
        double sx = bullets[i].x - camera_x;
        double sy = bullets[i].y - camera_y;
        ALLEGRO_COLOR col = (bullets[i].weapon == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        al_draw_filled_circle(sx, sy, 4, col);
    }
}

/* ===== Getter Functions ===== */

// Global bullet array (needed for getter functions)
static Bullet* g_bullets = NULL;
static int g_max_bullets = MAX_BULLETS;

// Set global bullet reference (called from bullets_init)
void set_global_bullet_ref(Bullet* bullets, int max_bullets) {
    g_bullets = bullets;
    g_max_bullets = max_bullets;
}

Bullet* get_bullets(void) {
    return g_bullets;
}

int get_max_bullets(void) {
    return g_max_bullets;
}