#include "bullet.h"
#include "map_generation.h"
#include "ini_parser.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>


bullet_sprites_t bullet_sprites;

void bullets_init(Bullet* bullets, int max_bullets) {
    // Load bullet dimensions from config.ini
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
    int mg_width = ini_parser_get_int(parser, "Bullets", "mg_bullet_width", 40);
    int mg_height = ini_parser_get_int(parser, "Bullets", "mg_bullet_height", 10);
    int cannon_width = ini_parser_get_int(parser, "Bullets", "cannon_bullet_width", 20);
    int cannon_height = ini_parser_get_int(parser, "Bullets", "cannon_bullet_height", 20);
    ini_parser_destroy(parser);
    
    for (int i = 0; i < max_bullets; i++) {
        bullets[i].alive = false;
        bullets[i].from_enemy = false;
        bullets[i].x = 0.0;
        bullets[i].y = 0.0;
        bullets[i].vx = 0.0;
        bullets[i].vy = 0.0;
        bullets[i].weapon = 0;
        // Set default dimensions based on weapon type
        bullets[i].width = mg_width;      // Default to MG dimensions
        bullets[i].height = mg_height;
        bullets[i].angle = 0.0;
    }
}

void bullets_update(Bullet* bullets, int max_bullets, const Map* map) {
    // Load bullet physics settings from config.ini
    // TODO: remove reading ini online
    IniParser* parser = ini_parser_create();
    ini_parser_load_file(parser, "TankBoy/config.ini");
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
        
        // Different colors for player vs enemy bullets
        ALLEGRO_COLOR col;
        if (bullets[i].from_enemy) {
            // Enemy bullets: red color
            col = (bullets[i].weapon == 0) ? al_map_rgb(255, 0, 0) : al_map_rgb(200, 0, 0);
        } else {
            // Player bullets: original colors
            col = (bullets[i].weapon == 0) ? al_map_rgb(255, 0, 0) : al_map_rgb(255, 128, 0);
        }
        
        // Rotate rectangle around center point (sx, sy)
        double half_w = bullets[i].width / 2.0;
        double half_h = bullets[i].height / 2.0;
        double cos_a = cos(bullets[i].angle);
        double sin_a = sin(bullets[i].angle);
        
        // Calculate four corners of rectangle (rotated around sx, sy)
        float x1 = sx + (-half_w * cos_a - (-half_h) * sin_a);
        float y1 = sy + (-half_w * sin_a + (-half_h) * cos_a);
        
        float x2 = sx + (half_w * cos_a - (-half_h) * sin_a);
        float y2 = sy + (half_w * sin_a + (-half_h) * cos_a);
        
        float x3 = sx + (half_w * cos_a - half_h * sin_a);
        float y3 = sy + (half_w * sin_a + half_h * cos_a);
        
        float x4 = sx + (-half_w * cos_a - half_h * sin_a);
        float y4 = sy + (-half_w * sin_a + half_h * cos_a);
        
        // Draw rotated rectangle using two triangles
        // al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, col);
        // al_draw_filled_triangle(x1, y1, x3, y3, x4, y4, col);

        
        if (bullets[i].weapon != 0) {
            int bullet_sprite_width = al_get_bitmap_width(bullet_sprites.cannon_bullet_sheet);
            int bullet_sprite_height = al_get_bitmap_height(bullet_sprites.cannon_bullet_sheet);

            double rotation_angle_rad = bullets[i].angle;
            double scale_x = (double)(bullets[i].width) / (double)(bullet_sprite_width);
            double scale_y = (double)(bullets[i].height) / (double)(bullet_sprite_height);

            al_draw_scaled_rotated_bitmap(bullet_sprites.cannon_bullet_sheet,
                                            bullet_sprite_width / 2.0, bullet_sprite_height / 2.0,  // rotation center
                                            sx, sy,                                                 // position to draw in display
                                            scale_x, scale_y,                                       // scale
                                            rotation_angle_rad,                                     // rotation angle
                                            0);
            continue;
        }


        int bullet_sprite_width = al_get_bitmap_width(bullet_sprites.mg_bullet_sheet);
        int bullet_sprite_height = al_get_bitmap_height(bullet_sprites.mg_bullet_sheet);
        
        // double rotation_angle_rad = bullets[i].angle * ALLEGRO_PI / 180.0;
        double rotation_angle_rad = bullets[i].angle;
        double scale_x = (double)(bullets[i].width) / (double)(bullet_sprite_width);
        double scale_y = (double)(bullets[i].height) / (double)(bullet_sprite_height);
        
        if (bullets[i].from_enemy) {
            al_draw_scaled_rotated_bitmap(bullet_sprites.enemy_bullet_sheet,
                                            bullet_sprite_width / 2.0, bullet_sprite_height / 2.0,  // rotation center
                                            sx, sy,                                                 // position to draw in display
                                            scale_x, scale_y,                                       // scale
                                            rotation_angle_rad,                                     // rotation angle
                                            0);
        } else {
            al_draw_scaled_rotated_bitmap(bullet_sprites.mg_bullet_sheet,
                                            bullet_sprite_width / 2.0, bullet_sprite_height / 2.0,  // rotation center
                                            sx, sy,                                                 // position to draw in display
                                            scale_x, scale_y,                                       // scale
                                            rotation_angle_rad,                                     // rotation angle
                                            0);
        }
    }
}

// ===== Getter Functions =====

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

void bullet_sprites_init() {
    bullet_sprites.mg_bullet_sheet = al_load_bitmap("TankBoy/resources/sprites/tank_bullet.png");
    bullet_sprites.cannon_bullet_sheet = al_load_bitmap("TankBoy/resources/sprites/cannon_bullet.png");
    bullet_sprites.enemy_bullet_sheet = al_load_bitmap("TankBoy/resources/sprites/enemy_bullet.png");
    if (bullet_sprites.mg_bullet_sheet == NULL || bullet_sprites.cannon_bullet_sheet == NULL || bullet_sprites.enemy_bullet_sheet == NULL) {
        printf("wrong location of bullet sprite!!\n");
    }
}