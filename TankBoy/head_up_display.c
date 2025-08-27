#include "head_up_display.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>

/* HUD 폰트 */
static ALLEGRO_FONT* hud_font = NULL;

/* HUD 초기화 */
void head_up_display_init() {
    al_init_font_addon();
    al_init_ttf_addon();
    hud_font = al_create_builtin_font();
    if (!hud_font) {
        fprintf(stderr, "Failed to create HUD font!\n");
        exit(1);
    }
}

/* HUD 그리기 */
void head_up_display_draw(const Head_Up_Display_Data* head_up_display) {
    if (!head_up_display || !hud_font) return;

    // 무기 이름
    const char* weapon_name = "Unknown";
    switch (head_up_display->weapon_type) {
    case WEAPON_MACHINE_GUN: weapon_name = "Machine Gun"; break;
    case WEAPON_CANNON:      weapon_name = "Cannon"; break;
    }

    // HUD 텍스트
    al_draw_textf(hud_font, al_map_rgb(255, 255, 255), 10, 10, 0,
        "Weapon: %s", weapon_name);
    al_draw_textf(hud_font, al_map_rgb(255, 255, 255), 10, 50, 0,
        "Stage: %d  Score: %d", head_up_display->stage, head_up_display->score);

    // HP 계산
    int display_hp = head_up_display->player_hp - head_up_display->damage;
    if (display_hp < 0) display_hp = 0;
    if (display_hp > 100) display_hp = 100;

    // HP 바 위치 및 크기
    int bar_x = 10;
    int bar_y = 70;
    int bar_w = 200;
    int bar_h = 20;

    double ratio = display_hp / 100.0;

    // HP 바 테두리 (흰색)
    al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h, al_map_rgb(255, 255, 255), 2);

    // HP 바 채우기 (빨간색)
    al_draw_filled_rectangle(bar_x + 1, bar_y + 1,
        bar_x + (int)(bar_w * ratio) - 1,
        bar_y + bar_h - 1,
        al_map_rgb(255, 0, 0));
}


/* main 초기화 부분

   headupdisplay_init(); // HUD 폰트 초기화

*/

/* 사용 부분

   // 테스트용 데이터

Head_Up_Display_Data hud {0};
hud.weapon_type = WEAPON_MACHINE_GUN;
hud.player_hp = 100;
hud.stage = 1;
hud.score = 0;

    head_up_display_draw(&hud); 이런 식으로? 사용.

*/