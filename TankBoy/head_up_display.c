#include "head_up_display.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

/* HUD 폰트 */
static ALLEGRO_FONT* hud_font = NULL;

/* 내부 상태 */
static int current_hp = 100;
static int current_score = 0;
static double last_time = 0;

/* HUD 초기화 */
void head_up_display_init() {
    al_init_font_addon();
    al_init_ttf_addon();

    hud_font = al_create_builtin_font();
    if (!hud_font) {
        fprintf(stderr, "Failed to create HUD font!\n");
        exit(1);
    }

    current_hp = 100;
    current_score = 0;
    last_time = al_get_time();
}

/* HUD 상태 갱신 */ // 외부에서 사용 목적. (데미지, 무기 입력)(체력, 점수, 무기 출력)
Head_Up_Display_Data head_up_display_update(int damage, WeaponType weapon) {
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

    return hud;
}

/* HUD 그리기 */
void head_up_display_draw(const Head_Up_Display_Data* hud) {
    if (!hud || !hud_font) return;

    // 무기 이름
    const char* weapon_name = "Unknown";
    switch (hud->weapon) {
    case WEAPON_MACHINE_GUN: weapon_name = "Machine Gun"; break;
    case WEAPON_CANNON:      weapon_name = "Cannon"; break;
    }

    // HUD 텍스트
    al_draw_textf(hud_font, al_map_rgb(255, 255, 255), 10, 10, 0,
        "Weapon: %s", weapon_name);
    al_draw_textf(hud_font, al_map_rgb(255, 255, 255), 10, 40, 0,
        "Score: %d", hud->score);

    // 체력바
    int bar_x = 10, bar_y = 70, bar_w = 200, bar_h = 20;
    double ratio = hud->player_hp / 100.0;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    al_draw_rectangle(bar_x, bar_y, bar_x + bar_w, bar_y + bar_h,
        al_map_rgb(255, 255, 255), 2);

    if (hud->player_hp > 0) {
        al_draw_filled_rectangle(bar_x + 1, bar_y + 1,
            bar_x + (int)(bar_w * ratio) - 1,
            bar_y + bar_h - 1,
            al_map_rgb(255, 0, 0));
    }

    // 체력 텍스트
    al_draw_textf(hud_font, al_map_rgb(255, 255, 255),
        bar_x + bar_w + 10, bar_y, 0,
        "%d / 100", hud->player_hp);
}



/* 사용법

* 추가
#include "head_up_display.h"

* HUD 초기화
head_up_display_init();

* 외부에서 쓸 때
Head_Up_Display_Data hud = head_up_display_update(damage, tank.weapon);

* head_up_display 출력
head_up_display_draw(&hud);

*/