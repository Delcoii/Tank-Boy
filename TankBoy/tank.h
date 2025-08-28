#ifndef TANK_H
#define TANK_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>
#include "input_system.h"
#include "bullet.h"

// Forward declaration
struct Map;

// Tank structure
typedef struct {
    double x, y;
    double vx, vy;
    double cannon_angle;
    bool on_ground;
    int weapon; // 0=MG, 1=Cannon
    
    // Tank dimensions (loaded from config)
    int width, height;

    // HP & invincibility
    int hp;
    int max_hp;
    double invincible;   // Invincibility time remaining

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

// Functions
void tank_init(Tank* tank, double x, double y);
void tank_update(Tank* tank, InputState* input, double dt, Bullet* bullets, int max_bullets, const struct Map* map);
void tank_draw(Tank* tank, double camera_x, double camera_y);

// Getter functions for external access
double get_tank_x(void);
double get_tank_y(void);
int get_tank_width(void);
int get_tank_height(void);
int get_tank_hp(void);
int get_tank_max_hp(void);
double get_tank_invincible(void);
void set_tank_hp(int hp);
void set_tank_invincible(double time);
void set_tank_velocity(double vx, double vy);
void set_tank_x(double x);

// Camera getter (needed for HP bar drawing)
double get_camera_x(void);
double get_camera_y(void);

// Global reference setters
void set_global_tank_ref(Tank* tank);
void set_camera_position(double x, double y);

#endif // TANK_H
