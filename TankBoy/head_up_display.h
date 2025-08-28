#pragma once
#ifndef HEAD_UP_DISPLAY_H
#define HEAD_UP_DISPLAY_H

#include <allegro5/allegro5.h>
#include <stdbool.h>

// HUD data structure
typedef struct {
    int player_hp;      // Health (0~100)
    int player_max_hp;  // Maximum health
    int score;          // Time-based score
    int weapon;         // Current weapon (0=MG, 1=Cannon)
    int stage;          // Current stage
    int round;          // Current round
    int enemies_alive;  // Number of alive enemies
    int flying_enemies_alive; // Number of alive flying enemies
} Head_Up_Display_Data;

// HUD initialization
void head_up_display_init(const char* config_file);

// HUD update
Head_Up_Display_Data head_up_display_update(int damage, int weapon, int stage);

// HUD draw
void head_up_display_draw(const Head_Up_Display_Data* hud);

// Enemy HP display functions
void draw_enemy_hp_bars(void);
void draw_flying_enemy_hp_bars(void);

// World-space HP bar drawing
void draw_hp_bar_world(double wx, double wy, int hp, int hp_max, double bar_w);

// HUD update functions
void update_tank_hp_display(int new_hp);
void update_enemy_count_display(int ground_count, int flying_count);

#endif
