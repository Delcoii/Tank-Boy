#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

/* ===== Function Declarations ===== */

/* Collision detection utilities */
bool point_in_rect(double px, double py, double rx, double ry, double rw, double rh);
bool rect_rect_overlap(double x1, double y1, double w1, double h1, 
                      double x2, double y2, double w2, double h2);

/* Bullet collision detection */
void bullets_hit_enemies(void);
void bullets_hit_tank(void);

/* Tank-enemy collision detection */
void tank_touch_ground_enemy(void);

/* Damage application */
void apply_damage_to_tank(int damage);
void apply_damage_to_enemy(int enemy_index, int damage);
void apply_damage_to_flying_enemy(int enemy_index, int damage);

/* Knockback effects */
void apply_knockback_to_tank(double vx, double vy);
void apply_knockback_to_enemy(int enemy_index, double vx, double vy);

/* Collision response */
void handle_tank_enemy_collision(int enemy_index);
void handle_bullet_enemy_collision(int bullet_index, int enemy_index, bool is_flying);

#endif /* COLLISION_H */
