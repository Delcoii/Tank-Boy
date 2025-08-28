#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <allegro5/allegro5.h>
#include <stdbool.h>

// key states structure
typedef struct {
    bool left;        // A / ←
    bool right;       // D / →
    bool jump;        // W / ↑
    bool change_weapon; // R
    bool esc;         // ESC
    bool fire;        // mouse click
} InputState;

// Function declarations
void input_system_init(InputState* input);
void input_system_update(InputState* input, ALLEGRO_EVENT* event);
bool input_is_key_pressed(InputState* input, int key);

#endif // INPUT_SYSTEM_H