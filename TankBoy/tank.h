#ifndef TANK_H
#define TANK_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>
#include "input_system.h"
#include "bullet.h"


// Forward declaration to avoid circular includes
struct Map; 

// Tank structure
typedef struct {
    double x, y;         // Position
    double vx, vy;       // Velocity
    double cannon_angle; // Cannon angle
    bool on_ground;      // On the ground
    int weapon;          // 0=MG, 1=Cannon

    // Cannon charging
    bool charging;
    double cannon_power;

    // Machine gun firing/reloading
    bool mg_firing;
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;

} Tank;

// Function declarations
void tank_init(Tank* tank, double x, double y);
void tank_update(Tank* tank, InputState* input, double dt, Bullet* bullets, int max_bullets, const struct Map* map);
void tank_draw(Tank* tank, double camera_x, double camera_y);

#endif // TANK_H