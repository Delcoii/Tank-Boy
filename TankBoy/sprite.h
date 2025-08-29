#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <allegro5/allegro5.h>

/* 헬리콥터 스프라이트 타입 */
typedef enum {
    SPRITE_HELICOPTER,   // 헬리콥터
    SPRITE_COUNT
} SpriteType;

/* 초기화 / 해제 */
void sprite_manager_init(const char* sheet_filename);
void sprite_manager_shutdown(void);

/* 스프라이트 가져오기 */
ALLEGRO_BITMAP* sprite_get(SpriteType type, int frame);

/* 스프라이트 그리기 */
void sprite_draw(SpriteType type, int frame, float x, float y);

#endif // SPRITE_MANAGER_H
