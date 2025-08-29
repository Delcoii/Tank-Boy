#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <allegro5/allegro5.h>
#include <stdbool.h>

// Key states structure
typedef struct {
    bool left;        // A / Left arrow
    bool right;       // D / Right arrow
    bool jump;        // W / Up arrow
    bool change_weapon; // R
    bool esc;         // ESC
    bool fire;        // Mouse click
} InputState;

// Function declarations
void input_system_init(InputState* input);
void input_system_update(InputState* input, ALLEGRO_EVENT* event);
bool input_is_key_pressed(InputState* input, int key);

#endif // INPUT_SYSTEM_H