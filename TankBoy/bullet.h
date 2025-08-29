#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include <allegro5/allegro_primitives.h>
#include "map_generation.h"

#define MAX_BULLETS 100



typedef struct {
    double x, y;
    double vx, vy;
    bool alive;
    int weapon; // 0=MG, 1=Cannon
    bool from_enemy; // Whether bullet was fired by enemy
    
    // Bullet dimensions (different for MG vs Cannon)
    int width, height;
    
    // Bullet rotation angle (in radians)
    double angle;
} Bullet;

void bullets_init(Bullet* bullets, int max_bullets);
void bullets_update(Bullet* bullets, int max_bullets, const Map* map);
void bullets_draw(Bullet* bullets, int max_bullets, double camera_x, double camera_y);

// Getter functions for external access
Bullet* get_bullets(void);
int get_max_bullets(void);

// Global reference setter
void set_global_bullet_ref(Bullet* bullets, int max_bullets);

#endif
