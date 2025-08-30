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

typedef struct _hud_sprites {
    ALLEGRO_BITMAP* button_sheet;
    ALLEGRO_BITMAP* tank_bullet_sheet;
    ALLEGRO_BITMAP* cannon_bullet_sheet;
} hud_sprites_t;

// HUD display settings
typedef struct {
    int hud_weapon_x;
    int hud_weapon_y;
    int hud_weapon_width;
    int hud_weapon_height;
} hud_settings_t;

// HUD initialization
void head_up_display_init(const char* config_file);

// HUD update
Head_Up_Display_Data head_up_display_update(int score, int weapon, int stage);

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

// sprite
void hud_sprites_init();

#endif
