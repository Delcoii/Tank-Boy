#ifndef TANK_SPRITE_H
#define TANK_SPRITE_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>

#define SPRITE_ROWS 5
#define SPRITE_WIDTH 64
#define SPRITE_HEIGHT 64

typedef enum {
    TANK_MOVE = 0,
    TANK_ATTACK = 1,
    TANK_IDLE = 3,
    TANK_JUMP = 4
} TankSpriteState;

typedef struct {
    ALLEGRO_BITMAP* sprite_sheet;
    ALLEGRO_BITMAP* sprites[SPRITE_ROWS];
    TankSpriteState current_state;
    bool facing_left;
} TankSprite;

bool load_tank_sprites(TankSprite* ts, const char* filename);
void draw_tank_sprite(TankSprite* ts, float x, float y);
void destroy_tank_sprites(TankSprite* ts);
void set_tank_sprite_state(TankSprite* ts, TankSpriteState state, bool facing_left);

#endif
