#include "enemy.h"
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ====== Ground Enemies ======
void enemies_init(Enemy* enemies, int max_enemies) {
    for (int i = 0; i < max_enemies; i++) enemies[i].alive = false;
}

void spawn_enemies(Enemy* enemies, int max_enemies, int round_number) {
    int count = round_number + 2;
    for (int i = 0; i < max_enemies && count > 0; i++) {
        if (!enemies[i].alive) {
            enemies[i].alive = true;
            if (rand() % 2) enemies[i].x = -50 + rand() % 30;
            else enemies[i].x = 1280 + rand() % 30; // 화면 오른쪽 스폰

            // Ground collision with proper calculation
            double ground = 500.0; // Fixed ground level for now
            enemies[i].y = ground-20;
            enemies[i].vx = (rand() % 2 ? 1 : -1) * (1.0 + round_number * 0.2);
            enemies[i].cannon_angle = M_PI / 4;
            enemies[i].weapon = rand() % 2;
            enemies[i].fire_cooldown = rand() % 60 / 60.0;
            enemies[i].hp = 10 + round_number * 5;
            count--;
        }
    }
}

void enemies_update(Enemy* enemies, int max_enemies, double dt,
    Tank* tank, Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_enemies; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        // 이동
        e->x += e->vx;
        // Ground collision with proper calculation
        double ground = 500.0; // Fixed ground level for now
        if (e->y > ground - 20) { e->y = ground - 20; e->vy = 0; }

        // 화면 끝 튕기기
        if (e->x < 0) { e->x = 0; e->vx *= -1; }
        int temp_MAP_W = 20; // 임시 화면 끝
        if (e->x > temp_MAP_W * 4) { e->x = temp_MAP_W * 4; e->vx *= -1; }

        // 플레이어 조준 & 발사
        double dx = tank->x - e->x, dy = tank->y - e->y;
        e->cannon_angle = atan2(dy, dx);
        e->fire_cooldown -= dt;
        if (e->fire_cooldown <= 0) {
            for (int j = 0; j < max_bullets; j++) {
                if (!bullets[j].alive) {
                    bullets[j].alive = true;
                    bullets[j].x = e->x + 16;
                    bullets[j].y = e->y + 10;
                    bullets[j].weapon = e->weapon;
                    bullets[j].vx = cos(e->cannon_angle) * 6.0;
                    bullets[j].vy = sin(e->cannon_angle) * 6.0;
                    e->fire_cooldown = 1.0 + rand() % 60 / 30.0;
                    break;
                }
            }
        }
    }
}

void enemies_draw(Enemy* enemies, int max_enemies, double camera_x, double camera_y) {
    for (int i = 0; i < max_enemies; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double sx = e->x - camera_x, sy = e->y - camera_y;
        al_draw_filled_rectangle(sx, sy, sx + 32, sy + 20, al_map_rgb(200, 50, 50));
        al_draw_line(sx + 16, sy + 10,
            sx + 16 + cos(e->cannon_angle) * 18,
            sy + 10 + sin(e->cannon_angle) * 18,
            al_map_rgb(255, 255, 0), 4);
    }
}

// ====== Flying Enemies ======
void flying_enemies_init(FlyingEnemy* f_enemies, int max_fly) {
    for (int i = 0; i < max_fly; i++) f_enemies[i].alive = false;
}

void spawn_flying_enemy(FlyingEnemy* f_enemies, int max_fly, int round_number) {
    for (int i = 0; i < max_fly; i++) {
        if (!f_enemies[i].alive) {
            f_enemies[i].alive = true;
            f_enemies[i].x = rand() % 1280;
            f_enemies[i].base_y = 100 + rand() % 100;
            f_enemies[i].y = f_enemies[i].base_y;
            f_enemies[i].vx = (rand() % 2 ? 1 : -1) * (1.0 + round_number * 0.2);
            f_enemies[i].angle = 0;
            f_enemies[i].fire_timer = 0;
            break;
        }
    }
}

void flying_enemies_update(FlyingEnemy* f_enemies, int max_fly, double dt,
    Tank* tank, Bullet* bullets, int max_bullets) {
    for (int i = 0; i < max_fly; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;

        // 위아래 진동 이동
        fe->angle += dt * 2.0;
        fe->y = fe->base_y + sin(fe->angle) * 30;
        fe->x += fe->vx;

        if (fe->x < 0) { fe->x = 0; fe->vx *= -1; }
        int temp_MAP_W = 20; // 임시 화면 끝
        if (fe->x > temp_MAP_W * 4) { fe->x = temp_MAP_W * 4; fe->vx *= -1; }

        // 총알 발사
        fe->fire_timer -= dt;
        if (fe->fire_timer <= 0) {
            for (int j = 0; j < max_bullets; j++) {
                if (!bullets[j].alive) {
                    bullets[j].alive = true;
                    bullets[j].x = fe->x;
                    bullets[j].y = fe->y;
                    bullets[j].weapon = 0; // MG
                    double dx = tank->x - fe->x, dy = tank->y - fe->y;
                    double angle = atan2(dy, dx);
                    bullets[j].vx = cos(angle) * 8;
                    bullets[j].vy = sin(angle) * 8;
                    break;
                }
            }
            if (fe->fire_timer <= -0.5) fe->fire_timer = 2.0;
            else fe->fire_timer = -0.5;
        }
    }
}

void flying_enemies_draw(FlyingEnemy* f_enemies, int max_fly, double camera_x, double camera_y) {
    for (int i = 0; i < max_fly; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        double sx = fe->x - camera_x, sy = fe->y - camera_y;
        al_draw_filled_rectangle(sx, sy, sx + 28, sy + 16, al_map_rgb(180, 0, 180));
    }
}


/* usage
* 
* 


* game_system.c

// enemy test global
Enemy enemies[MAX_ENEMIES];
FlyingEnemy f_enemies[MAX_FLY_ENEMIES];

void init_game_system(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* queue, GameSystem* game_system) {
// Initialize player enemy test
    enemies_init(enemies, MAX_ENEMIES);
    flying_enemies_init(f_enemies, MAX_FLY_ENEMIES);
}

void update_game_state(ALLEGRO_EVENT* event, GameSystem* game_system){

// Update enemy test
        enemies_update(enemies, MAX_ENEMIES, 1.0 / 60, &game_system->player_tank, game_system->bullets, MAX_BULLETS);
        flying_enemies_update(f_enemies, MAX_FLY_ENEMIES, 1.0 / 60, &game_system->player_tank, game_system->bullets, MAX_BULLETS);
}

void draw_game(ALLEGRO_FONT* font, GameConfig* config, GameSystem* game_system) {
enemies_draw(enemies, MAX_ENEMIES, camera.x, camera.y);
flying_enemies_draw(f_enemies, MAX_FLY_ENEMIES, camera.x, camera.y);
}

*/