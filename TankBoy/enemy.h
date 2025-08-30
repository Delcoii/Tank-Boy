#ifndef ENEMY_H
#define ENEMY_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>
#include "map_generation.h"
#include "bullet.h"



// ===== Constants =====
#define MAX_ENEMIES 20
#define MAX_FLY_ENEMIES 10
#define MAX_BULLETS 100

// HP / Damage tuning
#define ENEMY_BASE_HP 20
#define ENEMY_HP_PER_ROUND 5

// Jump timing tuning - loaded from config.ini
extern double enemy_jump_interval_min;
extern double enemy_jump_interval_max;

// Enemy physics - loaded from config.ini
extern double enemy_base_speed;
extern double enemy_speed_per_difficulty;

// Flying enemy bullet settings - loaded from config.ini
extern int flying_enemy_burst_count;
extern double flying_enemy_shot_interval;
extern double flying_enemy_rest_time;
extern double flying_enemy_bullet_speed;
extern int flying_enemy_bullet_width;
extern int flying_enemy_bullet_height;
extern double roi_multiplier;
extern double max_shooting_distance;

#define FLY_BASE_HP 12
#define FLY_HP_PER_ROUND 3

#define DMG_MG 5
#define DMG_CANNON 25
#define DMG_ENEMY_CONTACT 10

// Knockback tuning (contact / explosion)
#define KNOCKBACK_TANK_VX 6.0
#define KNOCKBACK_TANK_VY 4.0
#define KNOCKBACK_ENEMY_VX 4.5
#define KNOCKBACK_ENEMY_VY 3.5

// Cannon splash radius & knockback
#define CANNON_SPLASH_RADIUS 90.0
#define CANNON_SPLASH_KB     6.0

// ===== Data Types =====

// Ground enemy: chases player; jumps if stuck ~2s
typedef struct {
    double x, y, vx, vy;
    bool alive;
    bool on_ground;

    int hp;
    int max_hp;

    double last_x;
    double stuck_time;

    double speed;
    double jump_timer;  // Timer for random jumping
    
    // Enemy dimensions (loaded from config)
    int width, height;
    
    // Difficulty for scoring
    int difficulty;
} Enemy;

// Flying enemy: sine flight, burst fire (10 shots in ~0.5s) then rest
typedef struct {
    double x, y, vx;
    double base_y;
    double spawn_x;  // 스폰 위치 x좌표 저장
    double angle;
    double x_angle;  // x축 움직임용 각도
    bool alive;

    bool in_burst;
    int  burst_shots_left;
    double shot_interval;
    double shot_timer;
    double rest_timer;

    int hp;
    int max_hp;
    
    // Flying enemy dimensions (loaded from config)
    int width, height;
    
    // Difficulty for scoring
    int difficulty;
} FlyingEnemy;


typedef struct _enemy_sprites
{
    ALLEGRO_BITMAP* land_enemy_sheet;
    ALLEGRO_BITMAP* flying_enemy_sheet;

    ALLEGRO_BITMAP** land_enemy_sprites;
    ALLEGRO_BITMAP** flying_enemy_sprites;
} enemy_sprites_t;

// ===== Function Declarations =====

// Enemy initialization and management
void enemies_init(void);
void flying_enemies_init(void);

// Enemy spawning
void load_enemies_from_csv(int stage_number);
void load_enemies_from_csv_with_map(int stage_number, const Map* map);
void spawn_enemies(int round_number);
void spawn_flying_enemy(int round_number);

// Enemy updates
void enemies_update(double dt);
void enemies_update_with_map(double dt, const Map* map);
void flying_enemies_update(double dt);
void enemies_update_roi(double dt, double camera_x, double camera_y, int buffer_width, int buffer_height);
void enemies_update_roi_with_map(double dt, double camera_x, double camera_y, int buffer_width, int buffer_height, const Map* map);
void flying_enemies_update_roi(double dt, double camera_x, double camera_y, int buffer_width, int buffer_height);

// Enemy rendering
void enemies_draw(double camera_x, double camera_y);
void flying_enemies_draw(double camera_x, double camera_y);

// Enemy utilities
bool any_ground_enemies_alive(void);
bool any_flying_enemies_alive(void);
int get_alive_enemy_count(void);
int get_alive_flying_enemy_count(void);

// Enemy damage and effects
void damage_enemy(Enemy* enemy, int damage);
void damage_flying_enemy(FlyingEnemy* fe, int damage);
void apply_cannon_explosion(double ex, double ey, double radius);

// Enemy movement helpers
double get_enemy_ground_y(double x);
void handle_enemy_stuck_jump(Enemy* enemy, double dt);

// Getter functions for external access
Enemy* get_enemies(void);
FlyingEnemy* get_flying_enemies(void);

//sprite
void flying_enemy_sprites_init();
void enemy_sprites_init();
//void must_init(bool test, const char* description);
void flying_enemy_sprites_deinit();

#endif // ENEMY_H

