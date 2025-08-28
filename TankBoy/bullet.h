#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>
#include <allegro5/allegro_primitives.h>

#define MAX_BULLETS 100

// Forward declaration to avoid circular includes
struct Map;

typedef struct {
    double x, y;
    double vx, vy;
    bool alive;
    int weapon; // 0=MG, 1=Cannon
} Bullet;

void bullets_init(Bullet* bullets, int max_bullets);
void bullets_update(Bullet* bullets, int max_bullets, const struct Map* map);
void bullets_draw(Bullet* bullets, int max_bullets, double camera_x, double camera_y);

#endif
