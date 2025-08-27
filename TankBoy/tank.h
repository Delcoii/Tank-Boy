#if 0

#ifndef TANK_H
#define TANK_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "input_system.h"

// Tank structure
typedef struct {
    float x, y;          // Position
    float angle;         // Rotation angle in radians
    float speed;         // Movement speed
    float turn_speed;    // Rotation speed
    int width, height;   // Size
    ALLEGRO_COLOR color; // Tank color
} Tank;

// Function declarations
void tank_init(Tank* tank, float x, float y, ALLEGRO_COLOR color);
void tank_update(Tank* tank, InputState* input, float map_width, float map_height);
void tank_draw(Tank* tank);

#endif // TANK_H#
#endif

#ifndef TANK_H
#define TANK_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include "input_system.h"
#include "bullet.h" 

// Tank structure
typedef struct {
    float x, y;          // Position
    float angle;         // Rotation angle in radians
    float speed;         // Movement speed
    float turn_speed;    // Rotation speed
    bool on_ground;      // on the map
    int width, height;   // Size
    ALLEGRO_COLOR color; // Tank color
    double cannon_angle; // camera angle

    bool charging;       // canon guage
    double cannon_power; // canon power
    int weapon; // 0 = m60,1=cannon

    bool mg_firing;      // m60, reloading
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;

} Tank;

// Function declarations
void tank_init(Tank* tank, float x, float y, ALLEGRO_COLOR color);
void tank_update(Tank* tank, InputState* input, float map_width, float map_height);
void tank_draw(Tank* tank, double camera_x, double camera_y, Bullet* bullets, int max_bullets);

#endif // TANK_H
