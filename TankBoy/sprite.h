// sprite.h
#ifndef SPRITE_H
#define SPRITE_H

#include <allegro5/allegro.h>

void sprite_init();
void sprite_shutdown();

ALLEGRO_BITMAP* get_enemy_fly_sprite();

#endif
