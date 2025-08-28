#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "bullet.h"
#include "tank.h"   // Reference to Tank when enemies aim at the player

#define MAX_ENEMIES 20
#define MAX_FLY_ENEMIES 10

// Ground Enemy
typedef struct {
    double x, y;       // position
    double vx, vy;     // velocity
    double cannon_angle;
    int weapon;        // 0 = Machine Gun, 1 = Cannon
    bool alive;
    double fire_cooldown;
    int hp;
} Enemy;

// Flying Enemy
typedef struct {
    double x, y;       // position
    double vx;         // horizontal speed
    double base_y;     // base height
    double angle;      // oscillation angle
    bool alive;
    double fire_timer;
} FlyingEnemy;

// --- Function Declarations ---
// Initialization
void enemies_init(Enemy* enemies, int max_enemies);
void flying_enemies_init(FlyingEnemy* f_enemies, int max_fly);

// Spawning
void spawn_enemies(Enemy* enemies, int max_enemies, int round_number);
void spawn_flying_enemy(FlyingEnemy* f_enemies, int max_fly, int round_number);

// Updating
void enemies_update(Enemy* enemies, int max_enemies, double dt,
    Tank* tank, Bullet* bullets, int max_bullets);
void flying_enemies_update(FlyingEnemy* f_enemies, int max_fly, double dt,
    Tank* tank, Bullet* bullets, int max_bullets);

// Rendering
void enemies_draw(Enemy* enemies, int max_enemies, double camera_x, double camera_y);
void flying_enemies_draw(FlyingEnemy* f_enemies, int max_fly, double camera_x, double camera_y);

#endif // ENEMY_H
