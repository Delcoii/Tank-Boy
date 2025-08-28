#pragma once
#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "bullet.h"
#include "tank.h"   // 적이 탱크를 조준할 때 Tank 참조

#define MAX_ENEMIES 20
#define MAX_FLY_ENEMIES 10

// Ground Enemy (땅 적)
typedef struct {
    double x, y;       // 위치
    double vx, vy;     // 속도
    double cannon_angle;
    int weapon;        // 0=MG, 1=Cannon
    bool alive;
    double fire_cooldown;
    int hp;
} Enemy;

// Flying Enemy (공중 적)
typedef struct {
    double x, y;       // 위치
    double vx;         // 수평 이동 속도
    double base_y;     // 기준 높이
    double angle;      // 진동 각도
    bool alive;
    double fire_timer;
} FlyingEnemy;

// --- 함수 선언 ---
// 초기화
void enemies_init(Enemy* enemies, int max_enemies);
void flying_enemies_init(FlyingEnemy* f_enemies, int max_fly);

// 생성
void spawn_enemies(Enemy* enemies, int max_enemies, int round_number);
void spawn_flying_enemy(FlyingEnemy* f_enemies, int max_fly, int round_number);

// 업데이트
void enemies_update(Enemy* enemies, int max_enemies, double dt,
    Tank* tank, Bullet* bullets, int max_bullets);
void flying_enemies_update(FlyingEnemy* f_enemies, int max_fly, double dt,
    Tank* tank, Bullet* bullets, int max_bullets);

// 렌더링
void enemies_draw(Enemy* enemies, int max_enemies, double camera_x, double camera_y);
void flying_enemies_draw(FlyingEnemy* f_enemies, int max_fly, double camera_x, double camera_y);

#endif // ENEMY_H
