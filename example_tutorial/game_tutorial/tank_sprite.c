#include "tank_sprite.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline void draw_region_flippable(
    ALLEGRO_BITMAP* bmp,
    float sx, float sy, float sw, float sh,
    float dx, float dy,
    int face
) {
    int flags = (face < 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
    al_draw_bitmap_region(bmp, sx, sy, sw, sh, dx, dy, flags);
}

bool tank_sprite_load(TankSprite* s, const char* png_path) {
    if (!s) return false;
    s->sheet = al_load_bitmap(png_path);
    if (!s->sheet) return false;

    s->fw = TSPR_FW;
    s->fh = TSPR_FH;
    s->idle_frames = TSPR_IDLE_FRAMES;
    s->move_frames = TSPR_MOVE_FRAMES;
    s->jump_frames = TSPR_JUMP_FRAMES;
    s->turret_frames = TSPR_TURRET_FRAMES;

    s->turret_off_x = 48;
    s->turret_off_y = 22;

    return true;
}

void tank_sprite_unload(TankSprite* s) {
    if (!s) return;
    if (s->sheet) { al_destroy_bitmap(s->sheet); s->sheet = NULL; }
}

static int turret_angle_to_index(double ang_rad, int frames, int face) {
    double deg = ang_rad * 180.0 / M_PI;
    if (face < 0) deg = 180.0 - deg;
    if (deg < -40.0) deg = -40.0;
    if (deg > +40.0) deg = +40.0;
    double step = 80.0 / (frames - 1);
    int idx = (int)round((deg + 40.0) / step);
    if (idx < 0) idx = 0;
    if (idx > frames - 1) idx = frames - 1;
    return idx;
}

void tank_sprite_draw(
    TankSprite* s,
    float screen_x, float screen_y,
    bool moving, bool on_ground,
    double anim_time_sec,
    int face,
    double aim_angle_rad
) {
    if (!s || !s->sheet) return;

    int row = TSPR_ROW_IDLE;
    int count = s->idle_frames;
    int fps = 8;

    if (!on_ground) { row = TSPR_ROW_JUMP; count = s->jump_frames; fps = 6; }
    else if (moving) { row = TSPR_ROW_MOVE; count = s->move_frames; fps = 12; }

    int body_idx = (int)(anim_time_sec * fps) % count;
    float sx = body_idx * s->fw;
    float sy = row * s->fh;
    draw_region_flippable(s->sheet, sx, sy, s->fw, s->fh, screen_x, screen_y, face);

    int tidx = turret_angle_to_index(aim_angle_rad, s->turret_frames, face);
    float tsx = tidx * s->fw;
    float tsy = TSPR_ROW_TURRET * s->fh;
    float toff_x = (face > 0) ? s->turret_off_x : (s->fw - s->turret_off_x);
    float toff_y = s->turret_off_y;
    draw_region_flippable(s->sheet, tsx, tsy, s->fw, s->fh,
        screen_x + toff_x - s->fw / 2.0f,
        screen_y + toff_y - s->fh / 2.0f,
        face);
}
