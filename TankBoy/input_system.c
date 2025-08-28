#include "input_system.h"

// 초기화: 모든 키 false
void input_system_init(InputState* input) {
    input->left = false;
    input->right = false;
    input->jump = false;
    input->change_weapon = false;
    input->esc = false;
    input->fire = false;
}

// 이벤트를 기반으로 키 상태 갱신
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

// 특정 키가 눌려 있는지 확인 (선택적)
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