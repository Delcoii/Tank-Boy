#ifndef TANK_H
#define TANK_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>
#include "input_system.h"
#include "bullet.h"

// Forward declaration
struct Map;

/* Tank 구조체 */
typedef struct {
    double x, y;
    double vx, vy;
    double cannon_angle;
    bool on_ground;
    int weapon; // 0=MG, 1=Cannon

    // Cannon charging
    bool charging;
    double cannon_power;

    // Machine gun
    bool mg_firing;
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;

} Tank;

/* 함수 */
void tank_init(Tank* tank, double x, double y);
void tank_update(Tank* tank, InputState* input, double dt, Bullet* bullets, int max_bullets, const struct Map* map);
void tank_draw(Tank* tank, double camera_x, double camera_y);

#endif // TANK_H
