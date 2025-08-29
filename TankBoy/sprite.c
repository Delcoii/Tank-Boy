// sprite.c
#include <stdio.h>
#include "sprite.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

static ALLEGRO_BITMAP* enemy_fly_sprite = NULL;

void sprite_init() {
    // 다른 스프라이트들 초기화...
    enemy_fly_sprite = al_load_bitmap("assets/enemy_fly.png");
    if (!enemy_fly_sprite) {
        fprintf(stderr, "Failed to load enemy_fly.png\n");
    }
}

ALLEGRO_BITMAP* get_enemy_fly_sprite() {
    return enemy_fly_sprite;
}

void sprite_shutdown() {
    // 다른 스프라이트들 해제...
    if (enemy_fly_sprite) {
        al_destroy_bitmap(enemy_fly_sprite);
        enemy_fly_sprite = NULL;
    }
}
