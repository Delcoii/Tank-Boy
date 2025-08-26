#include "input_system.h"

// Initialize input system
void input_system_init(InputState* input) {
    input->key_up = false;
    input->key_down = false;
    input->key_left = false;
    input->key_right = false;
    input->key_escape = false;
    input->key_space = false;
}

// Update input state based on events
void input_system_update(InputState* input, ALLEGRO_EVENT* event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
                input->key_up = true;
                break;
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                input->key_down = true;
                break;
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                input->key_left = true;
                break;
            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
                input->key_right = true;
                break;
            case ALLEGRO_KEY_ESCAPE:
                input->key_escape = true;
                break;
            case ALLEGRO_KEY_SPACE:
                input->key_space = true;
                break;
        }
    }
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
                input->key_up = false;
                break;
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                input->key_down = false;
                break;
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                input->key_left = false;
                break;
            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
                input->key_right = false;
                break;
            case ALLEGRO_KEY_ESCAPE:
                input->key_escape = false;
                break;
            case ALLEGRO_KEY_SPACE:
                input->key_space = false;
                break;
        }
    }
}

// Check if specific key is pressed
bool input_is_key_pressed(InputState* input, int key) {
    switch (key) {
        case ALLEGRO_KEY_UP:
        case ALLEGRO_KEY_W:
            return input->key_up;
        case ALLEGRO_KEY_DOWN:
        case ALLEGRO_KEY_S:
            return input->key_down;
        case ALLEGRO_KEY_LEFT:
        case ALLEGRO_KEY_A:
            return input->key_left;
        case ALLEGRO_KEY_RIGHT:
        case ALLEGRO_KEY_D:
            return input->key_right;
        case ALLEGRO_KEY_ESCAPE:
            return input->key_escape;
        case ALLEGRO_KEY_SPACE:
            return input->key_space;
        default:
            return false;
    }
}
