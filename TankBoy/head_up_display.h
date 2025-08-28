#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>

/* Weapon type enum */
typedef enum {
    WEAPON_MACHINE_GUN = 0,
    WEAPON_CANNON = 1
} WeaponType;

/* HUD data structure */
typedef struct {
    int player_hp;      // Health (0~100)
    int score;          // Time-based score
    WeaponType weapon;  // Current weapon
    int stage;          // Current stage
} Head_Up_Display_Data;

/* HUD initialization */
void head_up_display_init(const char* config_file);

/* HUD update */
Head_Up_Display_Data head_up_display_update(int damage, WeaponType weapon, int stage);

/* HUD draw */
void head_up_display_draw(const Head_Up_Display_Data* hud);

#endif
