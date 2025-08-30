#include "input_system.h"

// Initialize all keys to false
void input_system_init(InputState* input) {
    input->left = false;
    input->right = false;
    input->jump = false;
    input->change_weapon = false;
    input->esc = false;
    input->fire = false;
}

// Update key status based on events
void input_system_update(InputState* input, ALLEGRO_EVENT* event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_A:
        case ALLEGRO_KEY_LEFT:  input->left = true; break;
        case ALLEGRO_KEY_D:
        case ALLEGRO_KEY_RIGHT: input->right = true; break;
        case ALLEGRO_KEY_W:
        case ALLEGRO_KEY_UP:    input->jump = true; break;
        case ALLEGRO_KEY_R:     input->change_weapon = true; break;
        case ALLEGRO_KEY_ESCAPE: input->esc = true; break;
        }
    }
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_A:
        case ALLEGRO_KEY_LEFT:  input->left = false; break;
        case ALLEGRO_KEY_D:
        case ALLEGRO_KEY_RIGHT: input->right = false; break;
        case ALLEGRO_KEY_W:
        case ALLEGRO_KEY_UP:    input->jump = false; break;
        case ALLEGRO_KEY_R:     input->change_weapon = false; break;
        case ALLEGRO_KEY_ESCAPE: input->esc = false; break;
        }
    }
    else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->mouse.button == 1) input->fire = true;
    }
    else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
        if (event->mouse.button == 1) input->fire = false;
    }
}

// Check if a specific key is pressed (optional)
bool input_is_key_pressed(InputState* input, int key) {
    switch (key) {
    case ALLEGRO_KEY_A:
    case ALLEGRO_KEY_LEFT:  return input->left;
    case ALLEGRO_KEY_D:
    case ALLEGRO_KEY_RIGHT: return input->right;
    case ALLEGRO_KEY_W:
    case ALLEGRO_KEY_UP:    return input->jump;
    case ALLEGRO_KEY_R:     return input->change_weapon;
    case ALLEGRO_KEY_ESCAPE:return input->esc;
    default: return false;
    }
}

// ===== Text Input Functions =====

void text_input_init(TextInput* text_input, int max_length) {
    memset(text_input->buffer, 0, sizeof(text_input->buffer));
    text_input->cursor_pos = 0;
    text_input->max_length = max_length;
    text_input->active = true;
}

void text_input_handle_key(TextInput* text_input, int key) {
    if (!text_input->active) return;
    
    if (key == ALLEGRO_KEY_BACKSPACE) {
        if (text_input->cursor_pos > 0) {
            text_input->cursor_pos--;
            text_input->buffer[text_input->cursor_pos] = '\0';
        }
    }
    else if (key == ALLEGRO_KEY_ENTER) {
        text_input->active = false;
    }
    else if (key >= ALLEGRO_KEY_A && key <= ALLEGRO_KEY_Z) {
        if (text_input->cursor_pos < text_input->max_length - 1) {
            text_input->buffer[text_input->cursor_pos] = 'A' + (key - ALLEGRO_KEY_A);
            text_input->cursor_pos++;
        }
    }
    else if (key >= ALLEGRO_KEY_0 && key <= ALLEGRO_KEY_9) {
        if (text_input->cursor_pos < text_input->max_length - 1) {
            text_input->buffer[text_input->cursor_pos] = '0' + (key - ALLEGRO_KEY_0);
            text_input->cursor_pos++;
        }
    }
}

// 새로운 함수: 문자 입력 처리
void text_input_handle_char(TextInput* text_input, int unichar) {
    if (!text_input->active) return;
    
    // 백스페이스 처리
    if (unichar == '\b') {
        if (text_input->cursor_pos > 0) {
            text_input->cursor_pos--;
            text_input->buffer[text_input->cursor_pos] = '\0';
        }
        return;
    }
    
    // 엔터 처리
    if (unichar == '\n' || unichar == '\r') {
        text_input->active = false;
        return;
    }
    
    // 일반 문자 처리 (ASCII 범위)
    if (unichar >= 32 && unichar <= 126) {
        if (text_input->cursor_pos < text_input->max_length - 1) {
            text_input->buffer[text_input->cursor_pos] = (char)unichar;
            text_input->cursor_pos++;
        }
    }
}

void text_input_draw(TextInput* text_input, int x, int y, ALLEGRO_FONT* font) {
    if (!font) return;
    
    // Draw text input box
    al_draw_rectangle(x, y, x + 300, y + 30, al_map_rgb(255, 255, 255), 2);
    
    // Draw current text
    al_draw_text(font, al_map_rgb(255, 255, 255), x + 5, y + 5, 0, text_input->buffer);
    
    // Draw cursor (blinking)
    static double cursor_timer = 0;
    cursor_timer += 0.016; // Assuming 60 FPS
    
    if ((int)(cursor_timer * 2) % 2 == 0) {
        int cursor_x = x + 5 + al_get_text_width(font, text_input->buffer);
        al_draw_line(cursor_x, y + 5, cursor_x, y + 25, al_map_rgb(255, 255, 255), 2);
    }
}

void text_input_reset(TextInput* text_input) {
    memset(text_input->buffer, 0, sizeof(text_input->buffer));
    text_input->cursor_pos = 0;
    text_input->active = true;
}