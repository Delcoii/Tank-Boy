#if 0

#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <allegro5/allegro5.h>
#include <stdbool.h>

// Key states
typedef struct {
    bool key_up;
    bool key_down;
    bool key_left;
    bool key_right;
    bool key_escape;
    bool key_space;
} InputState;

// Function declarations
void input_system_init(InputState* input);
void input_system_update(InputState* input, ALLEGRO_EVENT* event);
bool input_is_key_pressed(InputState* input, int key);

#endif // INPUT_SYSTEM_H
#endif

#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <allegro5/allegro5.h>
#include <stdbool.h>

// 키 입력 상태 구조체
typedef struct {
    bool left;        // A / ←
    bool right;       // D / →
    bool jump;        // W / ↑
    bool change_weapon; // R
    bool esc;         // ESC
    bool fire;        // 마우스 클릭(발사)
} InputState;

// 함수 선언
void input_system_init(InputState* input);
void input_system_update(InputState* input, ALLEGRO_EVENT* event);
bool input_is_key_pressed(InputState* input, int key);

#endif // INPUT_SYSTEM_H