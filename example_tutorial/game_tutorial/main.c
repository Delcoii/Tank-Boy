#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <allegro5/allegro_image.h> // 이미지 로드
#include "tank_sprite.h"

/* --- constants --- */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BUFFER_W 1280
#define BUFFER_H 720
#define MAP_W 200          /* number of ground samples */
#define TILE_W 4           /* world pixels per ground sample */
#define MAX_BULLETS 100
#define MAX_ENEMIES 20
#define MAX_FLY_ENEMIES 10
#define MAX_ROUNDS 10

/* ===== window scale setting =====
   Render to BUFFER_W x BUFFER_H, but show a WINDOW_SCALE-times larger window. */
#define WINDOW_SCALE 2
#define WINDOW_W (BUFFER_W * WINDOW_SCALE)
#define WINDOW_H (BUFFER_H * WINDOW_SCALE)

   /* object sizes */
#define TANK_W 32
#define TANK_H 20
#define ENEMY_W 32
#define ENEMY_H 20
#define FLY_W 28
#define FLY_H 16

/* HP / Damage tuning */
#define TANK_MAX_HP 100
#define INVINCIBLE_TIME 1.0 /* seconds */

#define ENEMY_BASE_HP 20
#define ENEMY_HP_PER_ROUND 5

#define FLY_BASE_HP 12
#define FLY_HP_PER_ROUND 3

#define DMG_MG 5
#define DMG_CANNON 25
#define DMG_ENEMY_CONTACT 10

/* Knockback tuning (contact / explosion) */
#define KNOCKBACK_TANK_VX 6.0
#define KNOCKBACK_TANK_VY 4.0
#define KNOCKBACK_ENEMY_VX 4.5
#define KNOCKBACK_ENEMY_VY 3.5

/* Cannon splash radius & knockback */
#define CANNON_SPLASH_RADIUS 90.0
#define CANNON_SPLASH_KB     6.0

/* =========================
   data types
========================= */
typedef struct { bool left, right, jump, change_weapon, esc; } Input;
typedef struct { double x, y; } Camera;
typedef struct { double height[MAP_W]; } Map;

/* bullet: weapon 0 = MG, 1 = Cannon */
typedef struct {
    double x, y, vx, vy;
    bool alive;
    int weapon;
    bool from_enemy; /* true if shot by enemies */
} Bullet;

typedef struct {
    double x, y, vx, vy;
    double cannon_angle;
    bool on_ground;
    int weapon;            /* 0=MG, 1=Cannon */

    /* MG/Cannon state */
    bool charging;
    double cannon_power;
    bool mg_firing;
    double mg_fire_time;
    double mg_shot_cooldown;
    bool mg_reloading;
    double mg_reload_time;

    /* HP & brief invincibility after taking damage */
    int hp, max_hp;
    double invincible; /* seconds left */
} Tank;

/* ground enemy: chases player; jumps if stuck ~2s */
typedef struct {
    double x, y, vx, vy;
    bool alive;
    bool on_ground;

    int hp;

    double last_x;
    double stuck_time;

    double speed;
    double accel;
    double friction;
} Enemy;

/* flying enemy: sine flight, burst fire (10 shots in ~0.5s) then rest */
typedef struct {
    double x, y, vx;
    double base_y;
    double angle;
    bool alive;

    bool in_burst;
    int  burst_shots_left;
    double shot_interval;
    double shot_timer;
    double rest_timer;

    int hp;
} FlyingEnemy;

/* =========================
   globals
========================= */
ALLEGRO_DISPLAY* disp;
ALLEGRO_BITMAP* buffer;
Input input;
Camera camera;
Map map_g;
Tank tank;
Bullet bullets[MAX_BULLETS];
Enemy enemies[MAX_ENEMIES];
FlyingEnemy f_enemies[MAX_FLY_ENEMIES];
int round_number = 1;

/* =========================
   display
========================= */
/* Create a window at WINDOW_W x WINDOW_H, but render to an offscreen buffer (BUFFER_W x BUFFER_H). */
void disp_init(void) {
    disp = al_create_display(WINDOW_W, WINDOW_H);
    if (!disp) exit(1);
    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    if (!buffer) exit(1);
}
void disp_pre_draw(void) { al_set_target_bitmap(buffer); }
/* Scale the offscreen buffer to the window size. */
void disp_post_draw(void) {
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(
        buffer,
        0, 0, BUFFER_W, BUFFER_H,
        0, 0, WINDOW_W, WINDOW_H,
        0
    );
    al_flip_display();
}
// sprite
TankSprite g_tankSpr;
double g_anim_time = 0.0;
int g_face = 1;

/* =========================
   map / ground
========================= */
/* Sample ground height from world x (in pixels). */
double map_ground_y_from_worldx(double world_x) {
    int x_tile = (int)(world_x / TILE_W);
    if (x_tile < 0) x_tile = 0;
    if (x_tile >= MAP_W) x_tile = MAP_W - 1;
    return map_g.height[x_tile];
}
/* Initialize a simple bumpy ground line. */
void map_init(void) {
    double base = BUFFER_H - 220;
    for (int i = 0; i < MAP_W; i++)
        map_g.height[i] = base + (rand() % 20 - 10);
}
/* Draw ground columns to the bottom of the screen. */
void draw_map(void) {
    for (int i = 0; i < MAP_W; i++) {
        double sx = i * TILE_W - camera.x;
        double sy = map_g.height[i] - camera.y;
        al_draw_filled_rectangle(sx, sy, sx + TILE_W, BUFFER_H, al_map_rgb(30, 150, 40));
    }
}

/* =========================
   UI: world-space HP bar
========================= */
static void draw_hp_bar_world(double wx, double wy, int hp, int hp_max, double bar_w) {
    if (hp_max <= 0) return;
    if (hp < 0) hp = 0;
    if (hp > hp_max) hp = hp_max;
    double ratio = (double)hp / (double)hp_max;

    double sx = wx - camera.x;
    double sy = wy - camera.y;

    al_draw_filled_rectangle(sx, sy, sx + bar_w, sy + 5, al_map_rgb(35, 35, 35));
    al_draw_filled_rectangle(sx, sy, sx + bar_w * ratio, sy + 5, al_map_rgb(220, 40, 40));
    al_draw_rectangle(sx, sy, sx + bar_w, sy + 5, al_map_rgb(255, 255, 255), 1);
}

/* =========================
   FX: simple hit ring
========================= */
typedef struct {
    double x, y;
    double t, max_t;
    bool used;
} HitFX;

#define MAX_FX 128
HitFX hitfx[MAX_FX];

static void fx_init(void) {
    for (int i = 0; i < MAX_FX; i++) hitfx[i].used = false;
}
static void fx_add(double x, double y) {
    for (int i = 0; i < MAX_FX; i++) {
        if (!hitfx[i].used) {
            hitfx[i].used = true;
            hitfx[i].x = x; hitfx[i].y = y;
            hitfx[i].max_t = 0.25;
            hitfx[i].t = hitfx[i].max_t;
            return;
        }
    }
}
static void fx_update(double dt) {
    for (int i = 0; i < MAX_FX; i++) {
        if (!hitfx[i].used) continue;
        hitfx[i].t -= dt;
        if (hitfx[i].t <= 0) hitfx[i].used = false;
    }
}
static void fx_draw(void) {
    for (int i = 0; i < MAX_FX; i++) {
        if (!hitfx[i].used) continue;
        double k = hitfx[i].t / hitfx[i].max_t; /* 1 → 0 */
        double r = 10 + 12 * (1.0 - k);
        int alpha = (int)(180 * k);
        if (alpha < 0) alpha = 0;
        double sx = hitfx[i].x - camera.x;
        double sy = hitfx[i].y - camera.y;
        al_draw_circle(sx, sy, r, al_map_rgba(255, 220, 120, alpha), 2.0f);
    }
}

/* =========================
   Tank
========================= */
/* Initialize player tank state, HP, weapons, etc. */
void tank_init(void) {
    tank.x = 50;
    tank.y = map_ground_y_from_worldx(tank.x) - TANK_H;
    tank.vx = 0.0; tank.vy = 0.0;
    tank.on_ground = true;
    tank.cannon_angle = M_PI / 4.0;
    tank.weapon = 0;
    tank.charging = false;
    tank.cannon_power = 0.0;
    tank.mg_firing = false;
    tank.mg_fire_time = 0.0;
    tank.mg_shot_cooldown = 0.0;
    tank.mg_reloading = false;
    tank.mg_reload_time = 0.0;

    tank.max_hp = TANK_MAX_HP;
    tank.hp = tank.max_hp;
    tank.invincible = 0.0;
}
/* Draw tank, cannon, gauges, and tank HP bar. */
void draw_tank(void) {
    double sx = tank.x - camera.x;
    double sy = tank.y - camera.y;

    ALLEGRO_COLOR body_col = (tank.invincible > 0.0)
        ? al_map_rgb(160, 160, 160)
        : al_map_rgb(60, 120, 180);

    al_draw_filled_rectangle(sx, sy, sx + TANK_W, sy + TANK_H, body_col);

    double cx = sx + TANK_W / 2.0, cy = sy + TANK_H / 2.0;
    double bx = cx + cos(tank.cannon_angle) * 18;
    double by = cy + sin(tank.cannon_angle) * 18;
    al_draw_line(cx, cy, bx, by, al_map_rgb(200, 200, 0), 4);

    /* Cannon charge gauge */
    if (tank.charging && tank.weapon == 1) {
        double gw = tank.cannon_power * 10;
        if (gw > 150) gw = 150;
        al_draw_filled_rectangle(sx, sy - 20, sx + gw, sy - 10, al_map_rgb(255, 0, 0));
        al_draw_rectangle(sx, sy - 20, sx + 150, sy - 10, al_map_rgb(255, 255, 255), 2);
    }
    /* MG reload gauge */
    if (tank.mg_reloading) {
        double total = 2.0;
        double filled = (total - tank.mg_reload_time) / total;
        if (filled < 0) filled = 0; if (filled > 1) filled = 1;
        double gw = 150 * filled;
        al_draw_rectangle(sx, sy - 35, sx + 150, sy - 20, al_map_rgb(255, 255, 255), 2);
        al_draw_filled_rectangle(sx + 1, sy - 34, sx + 1 + gw, sy - 21, al_map_rgb(0, 200, 255));
    }

    draw_hp_bar_world(tank.x, tank.y + TANK_H + 6, tank.hp, tank.max_hp, 50.0);
}
/* Handle input, physics (ground, gravity), weapon timers, and camera follow. */
void tank_update(double dt) {
    const double accel = 0.4, maxspeed = 3.0, friction = 0.85, gravity = 0.5;

    if (tank.invincible > 0.0) tank.invincible -= dt;

    if (input.left)  tank.vx -= accel;
    if (input.right) tank.vx += accel;
    tank.vx *= friction;
    if (tank.vx > maxspeed) tank.vx = maxspeed;
    if (tank.vx < -maxspeed) tank.vx = -maxspeed;
    tank.x += tank.vx;

    if (input.jump && tank.on_ground) { tank.vy = -8; tank.on_ground = false; }
    tank.vy += gravity; tank.y += tank.vy;
    double ground = map_ground_y_from_worldx(tank.x);
    if (tank.y > ground - TANK_H) { tank.y = ground - TANK_H; tank.vy = 0; tank.on_ground = true; }

    if (input.change_weapon) { tank.weapon = 1 - tank.weapon; input.change_weapon = false; }

    if (tank.weapon == 1 && tank.charging) {
        tank.cannon_power += 0.2;
        if (tank.cannon_power > 15) tank.cannon_power = 15;
    }

    /* MG burst & reload */
    if (tank.weapon == 0) {
        if (tank.mg_shot_cooldown > 0) tank.mg_shot_cooldown -= dt;
        if (tank.mg_reloading) {
            tank.mg_reload_time -= dt;
            if (tank.mg_reload_time <= 0) {
                tank.mg_reloading = false;
                tank.mg_fire_time = 0;
                tank.mg_shot_cooldown = 0;
            }
        }
        else {
            if (tank.mg_firing) {
                tank.mg_fire_time += dt;
                if (tank.mg_shot_cooldown <= 0) {
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].alive) {
                            bullets[i].alive = true;
                            bullets[i].x = tank.x + TANK_W / 2.0;
                            bullets[i].y = tank.y + TANK_H / 2.0;
                            bullets[i].weapon = 0;
                            bullets[i].from_enemy = false;
                            bullets[i].vx = cos(tank.cannon_angle) * 12.0;
                            bullets[i].vy = sin(tank.cannon_angle) * 12.0;
                            tank.mg_shot_cooldown = 0.1;
                            break;
                        }
                    }
                }
                if (tank.mg_fire_time >= 3.0) {
                    tank.mg_reloading = true;
                    tank.mg_reload_time = 2.0;
                    tank.mg_firing = false;
                }
            }
            else {
                tank.mg_fire_time = 0;
            }
        }
    }

    /* camera follow */
    camera.x = tank.x - BUFFER_W / 3.0;
    camera.y = tank.y - BUFFER_H / 1.5;
}

/* =========================
   Cannon trajectory preview (50% alpha white)
========================= */
void draw_cannon_trajectory(void) {
    if (!(tank.weapon == 1 && tank.charging)) return;

    double v0 = tank.cannon_power * 1.4;
    if (v0 < 0.5) return;

    double x = tank.x + TANK_W / 2.0;
    double y = tank.y + TANK_H / 2.0;
    double vx = cos(tank.cannon_angle) * v0;
    double vy = sin(tank.cannon_angle) * v0;

    double prev_sx = -1.0, prev_sy = -1.0;

    ALLEGRO_COLOR col_line = al_map_rgba(255, 255, 255, 50);
    ALLEGRO_COLOR col_dot = al_map_rgba(255, 255, 255, 50);

    for (int step = 0; step < 300; ++step) {
        x += vx;
        y += vy;
        vy += 0.3; /* gravity */

        if (x < 0 || x > MAP_W * TILE_W || y > BUFFER_H + 200) break;
        double gy = map_ground_y_from_worldx(x);
        if (y >= gy - 2.0) break;

        double sx = x - camera.x;
        double sy = y - camera.y;

        if (prev_sx >= 0.0) {
            al_draw_line(prev_sx, prev_sy, sx, sy, col_line, 2.0f);
        }
        if (step % 3 == 0) {
            al_draw_filled_circle(sx, sy, 2.0, col_dot);
        }
        prev_sx = sx; prev_sy = sy;
    }
}

/* =========================
   Bullets & splash helpers
========================= */
/* Distance falloff damage: dist==0 → full, dist==radius → ~0. */
static int splash_damage_falloff(double base, double dist, double radius) {
    if (dist >= radius) return 0;
    double k = 1.0 - (dist / radius);
    double dmg = base * (0.25 + 0.75 * k);
    return (int)(dmg + 0.5);
}
/* Apply cannon explosion: splash damage to enemies + knockback to ground enemies. */
static void apply_cannon_explosion(double ex, double ey, double radius) {
    fx_add(ex, ey);

    /* ground enemies */
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double cx = e->x + ENEMY_W * 0.5;
        double cy = e->y + ENEMY_H * 0.5;
        double dx = cx - ex, dy = cy - ey;
        double dist = sqrt(dx * dx + dy * dy);
        int dmg = splash_damage_falloff(DMG_CANNON, dist, radius);
        if (dmg > 0) {
            e->hp -= dmg;
            if (e->hp <= 0) { e->alive = false; continue; }
            if (dist < 1.0) dist = 1.0;
            double nx = dx / dist, ny = dy / dist;
            e->vx += nx * CANNON_SPLASH_KB;
            e->vy -= fabs(ny) * KNOCKBACK_ENEMY_VY;
        }
    }

    /* flying enemies: apply damage only */
    for (int i = 0; i < MAX_FLY_ENEMIES; ++i) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        double cx = fe->x + FLY_W * 0.5;
        double cy = fe->y + FLY_H * 0.5;
        double dx = cx - ex, dy = cy - ey;
        double dist = sqrt(dx * dx + dy * dy);
        int dmg = splash_damage_falloff(DMG_CANNON, dist, radius);
        if (dmg > 0) {
            fe->hp -= dmg;
            if (fe->hp <= 0) fe->alive = false;
        }
    }
}
/* Integrate bullet physics and handle ground impact for cannon. */
void bullets_update(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        if (bullets[i].weapon == 1) bullets[i].vy += 0.3; /* gravity for cannon */
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;

        /* Cannon → explode on ground contact */
        if (bullets[i].weapon == 1) {
            double gy = map_ground_y_from_worldx(bullets[i].x);
            if (bullets[i].y >= gy) {
                apply_cannon_explosion(bullets[i].x, gy - 2, CANNON_SPLASH_RADIUS);
                bullets[i].alive = false;
                continue;
            }
        }

        if (bullets[i].x < 0 || bullets[i].x > MAP_W * TILE_W || bullets[i].y > BUFFER_H + 100)
            bullets[i].alive = false;
    }
}
/* Draw bullets: yellow (MG) & orange (Cannon). */
void draw_bullets(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].alive) continue;
        double sx = bullets[i].x - camera.x;
        double sy = bullets[i].y - camera.y;
        ALLEGRO_COLOR col = (bullets[i].weapon == 0) ? al_map_rgb(255, 255, 0) : al_map_rgb(255, 128, 0);
        al_draw_filled_circle(sx, sy, (bullets[i].weapon == 0) ? 3 : 4, col);
    }
}

/* =========================
   Ground enemies
========================= */
/* Spawn ground enemies at left/right edges, scaled by round. */
void spawn_enemies(void) {
    int count = round_number + 2;
    for (int i = 0; i < MAX_ENEMIES && count > 0; i++) {
        if (!enemies[i].alive) {
            enemies[i].alive = true;

            if (rand() % 2) enemies[i].x = -50 + rand() % 30;
            else enemies[i].x = MAP_W * TILE_W + (rand() % 30);

            enemies[i].y = map_ground_y_from_worldx(enemies[i].x) - ENEMY_H;
            enemies[i].vx = 0.0;
            enemies[i].vy = 0.0;
            enemies[i].on_ground = true;

            enemies[i].hp = ENEMY_BASE_HP + ENEMY_HP_PER_ROUND * round_number;

            enemies[i].last_x = enemies[i].x;
            enemies[i].stuck_time = 0.0;

            enemies[i].speed = 1.2 + round_number * 0.15;
            enemies[i].accel = 0.15;
            enemies[i].friction = 0.90;

            count--;
        }
    }
}
/* Update chase logic, gravity, stuck jump, and horizontal bounce at map edges. */
void enemies_update(double dt) {
    const double gravity = 0.5;
    const double jump_power = -8.5;
    const double stuck_threshold = 1.0;
    const double stuck_jump_time = 2.0;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        double dir = (tank.x > e->x) ? 1.0 : -1.0;
        double target_vx = dir * e->speed;

        double dv = target_vx - e->vx;
        if (dv > e->accel) dv = e->accel;
        if (dv < -e->accel) dv = -e->accel;
        e->vx += dv;
        e->vx *= e->friction;

        e->vy += gravity;

        e->x += e->vx;
        e->y += e->vy;

        double ground = map_ground_y_from_worldx(e->x);
        if (e->y > ground - ENEMY_H) {
            e->y = ground - ENEMY_H;
            e->vy = 0.0;
            e->on_ground = true;
        }
        else {
            e->on_ground = false;
        }

        if (e->x < 0) { e->x = 0; e->vx = fabs(e->vx); }
        if (e->x > MAP_W * TILE_W) { e->x = MAP_W * TILE_W; e->vx = -fabs(e->vx); }

        /* stuck detection */
        if (fabs(e->x - e->last_x) <= stuck_threshold) {
            e->stuck_time += dt;
        }
        else {
            e->stuck_time = 0.0;
            e->last_x = e->x;
        }

        if (e->stuck_time >= stuck_jump_time && e->on_ground) {
            e->vy = jump_power;
            e->vx += dir * 1.5; /* small horizontal boost */
            e->stuck_time = 0.0;
        }
    }
}
/* Draw ground enemies + HP bars. */
void enemies_draw(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;
        double sx = e->x - camera.x, sy = e->y - camera.y;
        al_draw_filled_rectangle(sx, sy, sx + ENEMY_W, sy + ENEMY_H, al_map_rgb(200, 50, 50));
        draw_hp_bar_world(e->x, e->y + ENEMY_H + 4, e->hp, ENEMY_BASE_HP + ENEMY_HP_PER_ROUND * round_number, 40.0);
    }
}

/* =========================
   Flying enemies
========================= */
/* Reset flying enemy array. */
void flying_enemies_init(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        f_enemies[i].alive = false;
        f_enemies[i].in_burst = false;
        f_enemies[i].burst_shots_left = 0;
        f_enemies[i].shot_interval = 0.05;
        f_enemies[i].shot_timer = 0.0;
        f_enemies[i].rest_timer = 2.0;
        f_enemies[i].hp = 0;
    }
}
/* Spawn one flying enemy with sine flight params. */
void spawn_flying_enemy(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        if (!f_enemies[i].alive) {
            f_enemies[i].alive = true;
            f_enemies[i].x = rand() % (MAP_W * TILE_W);
            f_enemies[i].base_y = 100 + rand() % 100;
            f_enemies[i].y = f_enemies[i].base_y;
            f_enemies[i].vx = (rand() % 2 ? 1.0 : -1.0) * (1.0 + round_number * 0.2);
            f_enemies[i].angle = 0.0;

            f_enemies[i].in_burst = false;
            f_enemies[i].burst_shots_left = 0;
            f_enemies[i].shot_interval = 0.05;
            f_enemies[i].shot_timer = 0.0;
            f_enemies[i].rest_timer = 0.5 + (rand() % 50) / 100.0;

            f_enemies[i].hp = FLY_BASE_HP + FLY_HP_PER_ROUND * round_number;
            break;
        }
    }
}
/* Update sine movement, edge bounce, and MG burst cadence. */
void flying_enemies_update(double dt) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;

        fe->angle += dt * 2.0;
        fe->y = fe->base_y + sin(fe->angle) * 30.0;
        fe->x += fe->vx;

        if (fe->x < 0) { fe->x = 0; fe->vx *= -1; }
        if (fe->x > MAP_W * TILE_W) { fe->x = MAP_W * TILE_W; fe->vx *= -1; }

        if (fe->in_burst) {
            fe->shot_timer -= dt;

            while (fe->shot_timer <= 0.0 && fe->burst_shots_left > 0) {
                for (int j = 0; j < MAX_BULLETS; j++) {
                    if (!bullets[j].alive) {
                        bullets[j].alive = true;
                        bullets[j].x = fe->x;
                        bullets[j].y = fe->y;
                        bullets[j].weapon = 0;      /* MG round */
                        bullets[j].from_enemy = true;

                        double dx = tank.x - fe->x;
                        double dy = tank.y - fe->y;
                        double ang = atan2(dy, dx);

                        bullets[j].vx = cos(ang) * 8.0;
                        bullets[j].vy = sin(ang) * 8.0;
                        break;
                    }
                }

                fe->burst_shots_left--;
                fe->shot_timer += fe->shot_interval;

                if (fe->burst_shots_left <= 0) {
                    fe->in_burst = false;
                    fe->rest_timer = 2.0;
                }
            }
        }
        else {
            fe->rest_timer -= dt;
            if (fe->rest_timer <= 0.0) {
                fe->in_burst = true;
                fe->burst_shots_left = 10;
                fe->shot_timer = 0.0; /* fire immediately */
            }
        }
    }
}
/* Draw flying enemies + HP bars. */
void flying_enemies_draw(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
        FlyingEnemy* fe = &f_enemies[i];
        if (!fe->alive) continue;
        double sx = fe->x - camera.x, sy = fe->y - camera.y;
        al_draw_filled_rectangle(sx, sy, sx + FLY_W, sy + FLY_H, al_map_rgb(180, 0, 180));
        draw_hp_bar_world(fe->x, fe->y + FLY_H + 4, fe->hp, FLY_BASE_HP + FLY_HP_PER_ROUND * round_number, 36.0);
    }
}

/* =========================
   Collision
========================= */
static bool point_in_rect(double px, double py, double rx, double ry, double rw, double rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}
/* Player bullets → enemies (ground/flying). Cannon triggers splash on direct hit. */
static void bullets_hit_enemies(void) {
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].alive) continue;
        if (bullets[b].from_enemy) continue;

        double bx = bullets[b].x;
        double by = bullets[b].y;

        bool hit = false;

        /* ground enemies */
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy* e = &enemies[i];
            if (!e->alive) continue;
            if (point_in_rect(bx, by, e->x, e->y, ENEMY_W, ENEMY_H)) {
                if (bullets[b].weapon == 1) {
                    apply_cannon_explosion(bx, by, CANNON_SPLASH_RADIUS);
                }
                else {
                    e->hp -= DMG_MG;
                    fx_add(bx, by);
                }
                bullets[b].alive = false;
                hit = true;
                if (e->hp <= 0) e->alive = false;
                break;
            }
        }
        if (hit) continue;

        /* flying enemies */
        for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
            FlyingEnemy* fe = &f_enemies[i];
            if (!fe->alive) continue;
            if (point_in_rect(bx, by, fe->x, fe->y, FLY_W, FLY_H)) {
                if (bullets[b].weapon == 1) {
                    apply_cannon_explosion(bx, by, CANNON_SPLASH_RADIUS);
                }
                else {
                    fe->hp -= DMG_MG;
                    fx_add(bx, by);
                }
                bullets[b].alive = false;
                if (fe->hp <= 0) fe->alive = false;
                break;
            }
        }
    }
}
/* Enemy bullets → tank (respects brief invincibility). */
static void bullets_hit_tank(void) {
    if (tank.hp <= 0) return;
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].alive) continue;
        if (!bullets[b].from_enemy) continue;

        if (point_in_rect(bullets[b].x, bullets[b].y, tank.x, tank.y, TANK_W, TANK_H)) {
            bullets[b].alive = false;
            if (tank.invincible <= 0.0) {
                tank.hp -= DMG_MG; /* flying enemies use MG only */
                if (tank.hp < 0) tank.hp = 0;
                tank.invincible = INVINCIBLE_TIME;
                fx_add(tank.x + TANK_W / 2.0, tank.y + TANK_H / 2.0);
            }
        }
    }
}
/* Tank ↔ ground enemy contact damage + symmetric knockback. */
static void tank_touch_ground_enemy(void) {
    if (tank.hp <= 0) return;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        bool overlap =
            !(tank.x > e->x + ENEMY_W ||
                tank.x + TANK_W < e->x ||
                tank.y > e->y + ENEMY_H ||
                tank.y + TANK_H < e->y);

        if (overlap) {
            /* contact damage (with invincibility window) */
            if (tank.invincible <= 0.0) {
                tank.hp -= DMG_ENEMY_CONTACT;
                if (tank.hp < 0) tank.hp = 0;
                tank.invincible = INVINCIBLE_TIME;
                fx_add((tank.x + e->x) / 2.0 + TANK_W / 2.0, (tank.y + e->y) / 2.0 + TANK_H / 2.0);
            }

            /* symmetric knockback */
            double tank_cx = tank.x + TANK_W * 0.5;
            double enemy_cx = e->x + ENEMY_W * 0.5;
            double dir = (tank_cx < enemy_cx) ? -1.0 : 1.0;

            tank.vx = dir * KNOCKBACK_TANK_VX;
            tank.vy = -KNOCKBACK_TANK_VY;
            e->vx = -dir * KNOCKBACK_ENEMY_VX;
            e->vy -= KNOCKBACK_ENEMY_VY;

            /* small separation to resolve overlap */
            if (dir > 0) tank.x += 2.0; else tank.x -= 2.0;
            if (dir > 0) e->x -= 2.0; else e->x += 2.0;
        }
    }
}

/* =========================
   Round utilities
========================= */
static bool any_ground_enemies_alive(void) {
    for (int i = 0; i < MAX_ENEMIES; ++i)
        if (enemies[i].alive) return true;
    return false;
}
static bool any_flying_enemies_alive(void) {
    for (int i = 0; i < MAX_FLY_ENEMIES; ++i)
        if (f_enemies[i].alive) return true;
    return false;
}
/* When all enemies are cleared, advance round and spawn new ones. */
void next_round_if_cleared(void) {
    if (!any_ground_enemies_alive() && !any_flying_enemies_alive()) {
        round_number++;
        if (round_number > MAX_ROUNDS) round_number = MAX_ROUNDS;
        spawn_enemies();
        spawn_flying_enemy();
        if (round_number % 2 == 0) spawn_flying_enemy();
    }
}

/* =========================
   main
========================= */
int main(void) {
    memset(&input, 0, sizeof(input));
    camera.x = 0.0; camera.y = 0.0;

    srand((unsigned int)time(NULL));
    if (!al_init()) return 1;
    al_install_keyboard(); al_install_mouse(); al_init_primitives_addon();
    disp_init();

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    map_init(); tank_init();
    flying_enemies_init();
    spawn_enemies();
    spawn_flying_enemy();
    fx_init();
    al_init_image_addon(); // 이미지 관련 알레그로 초기화
    if (!tank_sprite_load(&g_tankSpr, "final_tank_sheet.png")) {
        fprintf(stderr, "스프라이트 로드 실패\n");
        return 1;
    }

    al_start_timer(timer);
    bool done = false; ALLEGRO_EVENT event;

    while (!done) {
        al_wait_for_event(queue, &event);
        switch (event.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE: done = true; break;
        case ALLEGRO_EVENT_TIMER: {
            double dt = 1.0 / 60.0;
            g_anim_time += dt;
            if (input.left)  g_face = -1;
            if (input.right) g_face = 1;
            tank_update(dt);
            bullets_update();
            enemies_update(dt);
            flying_enemies_update(dt);

            /* collisions */
            bullets_hit_enemies();
            bullets_hit_tank();
            tank_touch_ground_enemy();

            fx_update(dt);

            next_round_if_cleared();
            if (tank.hp <= 0) {
                done = true;
            }
            break;
            
        }
        case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_R) input.change_weapon = true;
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) done = true;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            if (event.keyboard.keycode == ALLEGRO_KEY_A) input.left = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_D) input.right = false;
            if (event.keyboard.keycode == ALLEGRO_KEY_W) input.jump = false;
            break;
        case ALLEGRO_EVENT_MOUSE_AXES: {
            /* Convert window mouse coords → buffer coords for aiming */
            double mx = event.mouse.x / (double)WINDOW_SCALE;
            double my = event.mouse.y / (double)WINDOW_SCALE;

            double cx = tank.x - camera.x + TANK_W / 2.0;
            double cy = tank.y - camera.y + TANK_H / 2.0;
            double dx = mx - cx, dy = my - cy;
            tank.cannon_angle = atan2(dy, dx);
            break;
        }
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (event.mouse.button == 1) {
                if (tank.weapon == 1) { tank.charging = true; tank.cannon_power = 0; }
                else if (!tank.mg_reloading) tank.mg_firing = true;
            }
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            if (event.mouse.button == 1) {
                if (tank.weapon == 1 && tank.charging) {
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].alive) {
                            bullets[i].alive = true;
                            bullets[i].x = tank.x + TANK_W / 2.0;
                            bullets[i].y = tank.y + TANK_H / 2.0;
                            bullets[i].weapon = 1;
                            bullets[i].from_enemy = false;
                            bullets[i].vx = cos(tank.cannon_angle) * tank.cannon_power * 1.4;
                            bullets[i].vy = sin(tank.cannon_angle) * tank.cannon_power * 1.4;
                            break;
                        }
                    }
                    tank.charging = false; tank.cannon_power = 0;
                }
                else if (tank.weapon == 0) {
                    tank.mg_firing = false; tank.mg_fire_time = 0;
                }
            }
            break;
        }

        disp_pre_draw();
        al_clear_to_color(al_map_rgb(20, 20, 30));
        draw_map();
        draw_cannon_trajectory();
        tank_sprite_draw(&g_tankSpr,      
            (float)(tank.x - camera.x),
            (float)(tank.y - camera.y),
            fabs(tank.vx) > 0.01,
            tank.on_ground,
            g_anim_time,
            g_face,
            tank.cannon_angle);
        draw_bullets();
        enemies_draw();
        flying_enemies_draw();
        fx_draw();
        disp_post_draw();
    }
    tank_sprite_unload(&g_tankSpr);
    al_shutdown_image_addon();

    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}