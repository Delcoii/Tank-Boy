#include "tank_sprite.h"
#include <stdio.h>

bool load_tank_sprites(TankSprite* ts, const char* filename) {
    ts->sprite_sheet = al_load_bitmap(filename);
    if (!ts->sprite_sheet) {
        fprintf(stderr, "Failed to load tank sprite sheet: %s\n", filename);
        return false;
    }
    for (int i = 0; i < SPRITE_ROWS; i++) {
        ts->sprites[i] = al_create_sub_bitmap(ts->sprite_sheet, 0, i * SPRITE_HEIGHT, SPRITE_WIDTH, SPRITE_HEIGHT);
    }
    ts->current_state = TANK_IDLE;
    ts->facing_left = false;
    return true;
}

void draw_tank_sprite(TankSprite* ts, float x, float y) {
    ALLEGRO_BITMAP* bmp = ts->sprites[ts->current_state];
    if (ts->facing_left) {
        al_draw_scaled_bitmap(bmp, 0, 0, SPRITE_WIDTH, SPRITE_HEIGHT, x, y, SPRITE_WIDTH, SPRITE_HEIGHT, ALLEGRO_FLIP_HORIZONTAL);
    } else {
        al_draw_bitmap(bmp, x, y, 0);
    }
}

void destroy_tank_sprites(TankSprite* ts) {
    for (int i = 0; i < SPRITE_ROWS; i++) {
        al_destroy_bitmap(ts->sprites[i]);
    }
    al_destroy_bitmap(ts->sprite_sheet);
}

void set_tank_sprite_state(TankSprite* ts, TankSpriteState state, bool facing_left) {
    ts->current_state = state;
    ts->facing_left = facing_left;
}
