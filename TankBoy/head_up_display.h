#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>

/* 무기 타입 enum */
typedef enum {
    WEAPON_MACHINE_GUN = 0,
    WEAPON_CANNON = 1
} WeaponType;

/* HUD 데이터 구조체 */
typedef struct {
    int player_hp;      // 체력 (0~100)
    int score;          // 생존 시간 기반 점수
    WeaponType weapon;  // 무기 종류
} Head_Up_Display_Data;

/* HUD 초기화 */
void head_up_display_init();

/* HUD 상태 갱신 (damage, weapon 입력) */
Head_Up_Display_Data head_up_display_update(int damage, WeaponType weapon);

/* HUD 그리기 */
void head_up_display_draw(const Head_Up_Display_Data* hud);

#endif
