#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>

/* HUD data structure */
typedef struct {
    int player_hp;      // Health (0~100)
    int score;          // Time-based score
    int weapon;         // Current weapon (0=MG, 1=Cannon)
    int stage;          // Current stage
} Head_Up_Display_Data;

/* HUD initialization */
void head_up_display_init(const char* config_file);

/* HUD update */
Head_Up_Display_Data head_up_display_update(int damage, int weapon, int stage);

/* HUD draw */
void head_up_display_draw(const Head_Up_Display_Data* hud);

#endif
