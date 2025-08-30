#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
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

// Text input structure
typedef struct {
    char buffer[32];  // Text buffer
    int cursor_pos;   // Cursor position
    int max_length;   // Maximum text length
    bool active;      // Is text input active
} TextInput;

// Function declarations
void input_system_init(InputState* input);
void input_system_update(InputState* input, ALLEGRO_EVENT* event);
bool input_is_key_pressed(InputState* input, int key);

// Text input functions
void text_input_init(TextInput* text_input, int max_length);
void text_input_handle_key(TextInput* text_input, int key);
void text_input_handle_char(TextInput* text_input, int unichar);
void text_input_draw(TextInput* text_input, int x, int y, ALLEGRO_FONT* font);
void text_input_reset(TextInput* text_input);

#endif // INPUT_SYSTEM_H