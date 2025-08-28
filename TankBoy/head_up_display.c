#include "head_up_display.h"
#include "ini_parser.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

/* HUD font */
static ALLEGRO_FONT* hud_font = NULL;

/* Internal state */
static int current_hp = 100;
static int current_score = 0;
static double last_time = 0;

/* HUD Colors */
static ALLEGRO_COLOR hud_text_color;
static ALLEGRO_COLOR hud_hp_color;
static ALLEGRO_COLOR hud_border_color;

/* HUD initialization (reads config.ini) */
void head_up_display_init(const char* config_file) {
    al_init_font_addon();
    al_init_ttf_addon();

    hud_font = al_create_builtin_font();
    if (!hud_font) {
        fprintf(stderr, "Failed to create HUD font!\n");
        exit(1);
    }

    // Load HUD colors from config.ini
    IniParser* parser = ini_parser_create();
    if (ini_parser_load_file(parser, config_file)) {
        int head_up_display_text_r = ini_parser_get_int(parser, "HUD Colors", "hud_text_r", 255);
        int head_up_display_text_g = ini_parser_get_int(parser, "HUD Colors", "hud_text_g", 255);
        int head_up_display_text_b = ini_parser_get_int(parser, "HUD Colors", "hud_text_b", 255);
        hud_text_color = al_map_rgb(head_up_display_text_r, head_up_display_text_g, head_up_display_text_b);

        int head_up_display_hp_r = ini_parser_get_int(parser, "HUD Colors", "hud_hp_r", 255);
        int head_up_display_hp_g = ini_parser_get_int(parser, "HUD Colors", "hud_hp_g", 0);
        int head_up_display_hp_b = ini_parser_get_int(parser, "HUD Colors", "hud_hp_b", 0);
        hud_hp_color = al_map_rgb(head_up_display_hp_r, head_up_display_hp_g, head_up_display_hp_b);

        int head_up_display_border_r = ini_parser_get_int(parser, "HUD Colors", "hud_border_r", 255);
        int head_up_display_border_g = ini_parser_get_int(parser, "HUD Colors", "hud_border_g", 255);
        int head_up_display_border_b = ini_parser_get_int(parser, "HUD", "hud_border_b", 255);
        hud_border_color = al_map_rgb(head_up_display_border_r, head_up_display_border_g, head_up_display_border_b);
    }
   
    ini_parser_destroy(parser);
   
    current_hp = 100;
    current_score = 0;
    last_time = al_get_time();
}

/* HUD update */
Head_Up_Display_Data head_up_display_update(int damage, WeaponType weapon, int stage) {
    current_hp -= damage;
    if (current_hp < 0) current_hp = 0;

    double now = al_get_time();
    double elapsed = now - last_time;
    if (elapsed >= 1.0) {
        current_score += (int)elapsed;
        last_time = now;
    }

    Head_Up_Display_Data hud;
    hud.player_hp = current_hp;
    hud.score = current_score;
    hud.weapon = weapon;
    hud.stage = stage;

    return hud;
}

/* HUD draw */
void head_up_display_draw(const Head_Up_Display_Data* hud) {
    if (!hud || !hud_font) return;

    // Weapon name
    const char* weapon_name = "Unknown";
    switch (hud->weapon) {
    case WEAPON_MACHINE_GUN: weapon_name = "Machine Gun"; break;
    case WEAPON_CANNON:      weapon_name = "Cannon"; break;
    }

    // HUD text
    al_draw_textf(hud_font, hud_text_color, 10, 10, 0,
        "Weapon: %s", weapon_name);
    al_draw_textf(hud_font, hud_text_color, 10, 40, 0,
        "Score: %d", hud->score);
    al_draw_textf(hud_font, hud_text_color, 10, 70, 0,
        "Stage: %d", hud->stage);

    // Health bar
    int bar_x = 10, bar_y = 100, bar_w = 200, bar_h = 20;
    double ratio = current_hp / 100.0;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    // Outline
    al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
        hud_border_color, 2);

    // Fill
    if (current_hp > 0) {
        al_draw_filled_rectangle(bar_x + 1, bar_y + 1,
            bar_x + (int)(bar_w * ratio) - 1,
            bar_y + bar_h - 1,
            hud_hp_color);
    }

    // Health text
    al_draw_textf(hud_font, hud_text_color,
        bar_x + bar_w + 10, bar_y, 0,
        "%d / 100", hud->player_hp);
}

/* Usage:

* game_system.c

#include "head_up_display.h"

head_up_display_init("config.ini");

// Draw Head_Up_Display
    int temp_stage = 1;
    int temp_damage = 0;
    game_system->hud = head_up_display_update(temp_damage, game_system->player_tank.weapon, temp_stage);
    head_up_display_draw(&game_system->hud);

*/