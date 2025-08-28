#include "collision.h"
#include "enemy.h"
#include "tank.h"
#include "bullet.h"
#include "head_up_display.h"
#include <math.h>

/* ===== Collision Detection Utilities ===== */

bool point_in_rect(double px, double py, double rx, double ry, double rw, double rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

bool rect_rect_overlap(double x1, double y1, double w1, double h1, 
                      double x2, double y2, double w2, double h2) {
    return !(x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2);
}

/* ===== Bullet Collision Detection ===== */

void bullets_hit_enemies(void) {
    Bullet* bullets = get_bullets();
    int max_bullets = get_max_bullets();
    
    for (int b = 0; b < max_bullets; b++) {
        if (!bullets[b].alive) continue;
        if (bullets[b].from_enemy) continue;

        double bx = bullets[b].x;
        double by = bullets[b].y;

        bool hit = false;

        /* ground enemies */
        Enemy* enemies = get_enemies();
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy* e = &enemies[i];
            if (!e->alive) continue;
            if (point_in_rect(bx, by, e->x, e->y, 32, 20)) { // ENEMY_W, ENEMY_H
                if (bullets[b].weapon == 1) {
                    // Cannon explosion
                    apply_cannon_explosion(bx, by, CANNON_SPLASH_RADIUS);
                }
                else {
                    // MG damage
                    damage_enemy(e, DMG_MG);
                }
                bullets[b].alive = false;
                hit = true;
                if (e->hp <= 0) e->alive = false;
                break;
            }
        }
        if (hit) continue;

        /* flying enemies */
        FlyingEnemy* f_enemies = get_flying_enemies();
        for (int i = 0; i < MAX_FLY_ENEMIES; i++) {
            FlyingEnemy* fe = &f_enemies[i];
            if (!fe->alive) continue;
            if (point_in_rect(bx, by, fe->x, fe->y, 28, 16)) { // FLY_W, FLY_H
                if (bullets[b].weapon == 1) {
                    // Cannon explosion
                    apply_cannon_explosion(bx, by, CANNON_SPLASH_RADIUS);
                }
                else {
                    // MG damage
                    damage_flying_enemy(fe, DMG_MG);
                }
                bullets[b].alive = false;
                if (fe->hp <= 0) fe->alive = false;
                break;
            }
        }
    }
}

void bullets_hit_tank(void) {
    if (get_tank_hp() <= 0) return;
    
    Bullet* bullets = get_bullets();
    int max_bullets = get_max_bullets();
    
    for (int b = 0; b < max_bullets; b++) {
        if (!bullets[b].alive) continue;
        if (!bullets[b].from_enemy) continue;

        double tank_x = get_tank_x();
        double tank_y = get_tank_y();
        int tank_w = get_tank_width();
        int tank_h = get_tank_height();

        if (point_in_rect(bullets[b].x, bullets[b].y, tank_x, tank_y, tank_w, tank_h)) {
            bullets[b].alive = false;
            if (get_tank_invincible() <= 0.0) {
                apply_damage_to_tank(DMG_MG);
            }
        }
    }
}

/* ===== Tank-Enemy Collision Detection ===== */

void tank_touch_ground_enemy(void) {
    if (get_tank_hp() <= 0) return;
    
    double tank_x = get_tank_x();
    double tank_y = get_tank_y();
    int tank_w = get_tank_width();
    int tank_h = get_tank_height();
    
    Enemy* enemies = get_enemies();
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* e = &enemies[i];
        if (!e->alive) continue;

        bool overlap = rect_rect_overlap(tank_x, tank_y, tank_w, tank_h,
                                       e->x, e->y, 32, 20); // ENEMY_W, ENEMY_H

        if (overlap) {
            handle_tank_enemy_collision(i);
        }
    }
}

/* ===== Damage Application ===== */

void apply_damage_to_tank(int damage) {
    if (get_tank_invincible() <= 0.0) {
        int current_hp = get_tank_hp();
        int new_hp = current_hp - damage;
        if (new_hp < 0) new_hp = 0;
        set_tank_hp(new_hp);
        
        // Set invincibility
        set_tank_invincible(1.0); // INVINCIBLE_TIME
        
        // Update HUD
        update_tank_hp_display(new_hp);
    }
}

void apply_damage_to_enemy(int enemy_index, int damage) {
    if (enemy_index < 0 || enemy_index >= MAX_ENEMIES) return;
    
    Enemy* enemies = get_enemies();
    Enemy* e = &enemies[enemy_index];
    
    if (e->alive) {
        damage_enemy(e, damage);
        // Update HUD if needed
    }
}

void apply_damage_to_flying_enemy(int enemy_index, int damage) {
    if (enemy_index < 0 || enemy_index >= MAX_FLY_ENEMIES) return;
    
    FlyingEnemy* f_enemies = get_flying_enemies();
    FlyingEnemy* fe = &f_enemies[enemy_index];
    
    if (fe->alive) {
        damage_flying_enemy(fe, damage);
        // Update HUD if needed
    }
}

/* ===== Knockback Effects ===== */

void apply_knockback_to_tank(double vx, double vy) {
    set_tank_velocity(vx, vy);
}

void apply_knockback_to_enemy(int enemy_index, double vx, double vy) {
    if (enemy_index < 0 || enemy_index >= MAX_ENEMIES) return;
    
    Enemy* enemies = get_enemies();
    Enemy* e = &enemies[enemy_index];
    
    if (e->alive) {
        e->vx = vx;
        e->vy = vy;
    }
}

/* ===== Collision Response ===== */

void handle_tank_enemy_collision(int enemy_index) {
    if (enemy_index < 0 || enemy_index >= MAX_ENEMIES) return;
    
    Enemy* enemies = get_enemies();
    Enemy* e = &enemies[enemy_index];
    
    if (!e->alive) return;
    
    /* contact damage (with invincibility window) */
    if (get_tank_invincible() <= 0.0) {
        apply_damage_to_tank(DMG_ENEMY_CONTACT);
    }

    /* symmetric knockback */
    double tank_cx = get_tank_x() + get_tank_width() * 0.5;
    double enemy_cx = e->x + 32 * 0.5; // ENEMY_W * 0.5
    double dir = (tank_cx < enemy_cx) ? -1.0 : 1.0;

    apply_knockback_to_tank(dir * KNOCKBACK_TANK_VX, -KNOCKBACK_TANK_VY);
    apply_knockback_to_enemy(enemy_index, -dir * KNOCKBACK_ENEMY_VX, -KNOCKBACK_ENEMY_VY);

    /* small separation to resolve overlap */
    double tank_x = get_tank_x();
    if (dir > 0) {
        set_tank_x(tank_x + 2.0);
        e->x -= 2.0;
    } else {
        set_tank_x(tank_x - 2.0);
        e->x += 2.0;
    }
}

void handle_bullet_enemy_collision(int bullet_index, int enemy_index, bool is_flying) {
    if (bullet_index < 0) return;
    
    Bullet* bullets = get_bullets();
    if (bullet_index >= get_max_bullets() || !bullets[bullet_index].alive) return;
    
    bullets[bullet_index].alive = false;
    
    if (is_flying) {
        if (enemy_index >= 0 && enemy_index < MAX_FLY_ENEMIES) {
            apply_damage_to_flying_enemy(enemy_index, DMG_MG);
        }
    } else {
        if (enemy_index >= 0 && enemy_index < MAX_ENEMIES) {
            apply_damage_to_enemy(enemy_index, DMG_MG);
        }
    }
}
