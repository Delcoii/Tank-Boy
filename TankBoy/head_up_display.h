#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>

/* 무기 타입 enum */ // 테스트 입력
typedef enum {
    WEAPON_MACHINE_GUN = 0,
    WEAPON_CANNON = 1
} WeaponType;

/* Head-Up Display 데이터 구조체 */
typedef struct {
    WeaponType weapon_type; // 무기 종류
    int player_hp;          // 0~100
    int damage;             // 받는 데미지
    int stage;              // 스테이지 번호
    int score;              // 점수
} Head_Up_Display_Data;

/* HUD 초기화 */
void head_up_display_init();

/* HUD 그리기 */
void head_up_display_draw(const Head_Up_Display_Data* head_up_display);

#endif
