#ifndef TANK_SPRITE_H
#define TANK_SPRITE_H

#include <allegro5/allegro5.h>

#define TSPR_FW 96
#define TSPR_FH 64
#define TSPR_IDLE_FRAMES   4
#define TSPR_MOVE_FRAMES   6
#define TSPR_JUMP_FRAMES   2
#define TSPR_TURRET_FRAMES 9
#define TSPR_ROW_IDLE   0
#define TSPR_ROW_MOVE   1
#define TSPR_ROW_JUMP   2
#define TSPR_ROW_TURRET 3

typedef struct {
    ALLEGRO_BITMAP* sheet;
    int fw, fh;
    int idle_frames, move_frames, jump_frames, turret_frames;
    int turret_off_x, turret_off_y;
} TankSprite;

bool tank_sprite_load(TankSprite* s, const char* png_path);
void tank_sprite_unload(TankSprite* s);
void tank_sprite_draw(
    TankSprite* s,
    float screen_x, float screen_y,
    bool moving, bool on_ground,
    double anim_time_sec,
    int face,
    double aim_angle_rad
);

#endif
