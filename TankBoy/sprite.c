#include "sprite.h"
#include <stdio.h>
#include <stdlib.h>

/* 내부 전역 */
static ALLEGRO_BITMAP* sprite_sheet = NULL;

/* 헬리콥터 (여러 프레임 애니메이션 가능) */
static ALLEGRO_BITMAP* sprites[SPRITE_COUNT][10];
static int sprite_frames[SPRITE_COUNT] = {0};

/* 에러 체크 */
static void must_init(bool test, const char* description) {
    if (!test) {
        fprintf(stderr, "Failed to initialize %s!\n", description);
        exit(1);
    }
}

/* 시트에서 잘라오기 */
static ALLEGRO_BITMAP* sprite_grab(int x, int y, int w, int h) {
    ALLEGRO_BITMAP* bmp = al_create_sub_bitmap(sprite_sheet, x, y, w, h);
    must_init(bmp, "sprite grab");
    return bmp;
}

/* 초기화 */
void sprite_manager_init(const char* sheet_filename) {
    sprite_sheet = al_load_bitmap(sheet_filename);
    must_init(sprite_sheet, "sprite sheet");

    /*
        예시: spritesheet.png에서 헬리콥터가 (0,0)에 있고 크기가 64x64라고 가정
        만약 애니메이션 프레임이 여러 개라면 좌표만 늘려주면 됨
    */
    sprites[SPRITE_HELICOPTER][0] = sprite_grab(0, 0, 64, 64);
    //sprites[SPRITE_HELICOPTER][1] = sprite_grab(64, 0, 64, 64);  // 2번째 프레임 (필요 시)
    sprite_frames[SPRITE_HELICOPTER] = 1;
}

/* 자원 해제 */
void sprite_manager_shutdown(void) {
    for (int i = 0; i < SPRITE_COUNT; i++) {
        for (int f = 0; f < sprite_frames[i]; f++) {
            if (sprites[i][f]) {
                al_destroy_bitmap(sprites[i][f]);
                sprites[i][f] = NULL;
            }
        }
    }
    if (sprite_sheet) {
        al_destroy_bitmap(sprite_sheet);
        sprite_sheet = NULL;
    }
}

/* 스프라이트 가져오기 */
ALLEGRO_BITMAP* sprite_get(SpriteType type, int frame) {
    if (type >= SPRITE_COUNT) return NULL;
    if (frame >= sprite_frames[type]) frame = 0;
    return sprites[type][frame];
}

/* 스프라이트 그리기 */
void sprite_draw(SpriteType type, int frame, float x, float y) {
    ALLEGRO_BITMAP* bmp = sprite_get(type, frame);
    if (bmp) {
        al_draw_bitmap(bmp, x, y, 0);
    }
}
