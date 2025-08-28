#include "bullet.h"
#include "map_generation.h"
#include "ini_parser.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>


void bullets_init(Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_bullets; i++) {
        bullets[i].alive = false;
        bullets[i].from_enemy = false;
        bullets[i].x = 0.0;
        bullets[i].y = 0.0;
        bullets[i].vx = 0.0;
        bullets[i].vy = 0.0;
        bullets[i].weapon = 0;
        bullets[i].width = 0;
        bullets[i].height = 0;
        bullets[i].angle = 0.0;
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
        
        // Different colors for different weapons
        ALLEGRO_COLOR col = (bullets[i].weapon == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        
        // 간단하게 빨간점(sx, sy) 위치를 중심으로 직사각형 회전
        double half_w = bullets[i].width / 2.0;
        double half_h = bullets[i].height / 2.0;
        double cos_a = cos(bullets[i].angle);
        double sin_a = sin(bullets[i].angle);
        
        // 직사각형의 네 모서리 계산 (sx, sy를 중심으로 회전)
        float x1 = sx + (-half_w * cos_a - (-half_h) * sin_a);
        float y1 = sy + (-half_w * sin_a + (-half_h) * cos_a);
        
        float x2 = sx + (half_w * cos_a - (-half_h) * sin_a);
        float y2 = sy + (half_w * sin_a + (-half_h) * cos_a);
        
        float x3 = sx + (half_w * cos_a - half_h * sin_a);
        float y3 = sy + (half_w * sin_a + half_h * cos_a);
        
        float x4 = sx + (-half_w * cos_a - half_h * sin_a);
        float y4 = sy + (-half_w * sin_a + half_h * cos_a);
        
        // 삼각형 두 개로 직사각형 그리기
        float triangle1[6] = {x1, y1, x2, y2, x3, y3};
        float triangle2[6] = {x1, y1, x3, y3, x4, y4};
        
        al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, col);
        al_draw_filled_triangle(x1, y1, x3, y3, x4, y4, col);
        
        // Debug circle at bullet center
        al_draw_filled_circle(sx, sy, 2, al_map_rgb(255, 0, 0));
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