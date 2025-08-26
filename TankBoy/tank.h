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

#endif // TANK_H
