#include "bullet.h"
#include "map_generation.h"
#include <allegro5/allegro_primitives.h>

#define BUFFER_H 720
#define MAP_W 200

void bullets_init(Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_bullets; i++)
        bullets[i].alive = false;
}

void bullets_update(Bullet* bullets, int max_bullets, const struct Map* map) {
    for (int i = 0; i < max_bullets; i++) {
        if (!bullets[i].alive) continue;

        // Gravity for cannon bullets
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3;

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
        if (bullets[i].x < 0 || bullets[i].x > 12800 || bullets[i].y > 2160 || bullets[i].y < 0)
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