#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>

/* ���� Ÿ�� enum */ // �׽�Ʈ �Է�
typedef enum {
    WEAPON_MACHINE_GUN = 0,
    WEAPON_CANNON = 1
} WeaponType;

/* Head-Up Display ������ ����ü */
typedef struct {
    WeaponType weapon_type; // ���� ����
    int player_hp;          // 0~100
    int damage;             // �޴� ������
    int stage;              // �������� ��ȣ
    int score;              // ����
} Head_Up_Display_Data;

/* HUD �ʱ�ȭ */
void head_up_display_init();

/* HUD �׸��� */
void head_up_display_draw(const Head_Up_Display_Data* head_up_display);

#endif
