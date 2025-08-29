#ifndef RANKING_H
#define RANKING_H

#include <stdbool.h>

// ===== Constants =====
#define MAX_RANKINGS 100
#define MAX_NAME_LENGTH 32
#define DEFAULT_PLAYER_NAME "aaaaa"

// ===== Data Types =====
typedef struct {
    char name[MAX_NAME_LENGTH];
    int score;
    int stage;
    char date[20];  // YYYY-MM-DD HH:MM format
} RankingEntry;

typedef struct {
    RankingEntry entries[MAX_RANKINGS];
    int count;
} RankingSystem;

// ===== Function Declarations =====

// Ranking system management
void ranking_init(void);
void ranking_cleanup(void);
void ranking_deinit(void);  // Alias for ranking_cleanup

// Add new score to ranking
void ranking_add_score(int score, int stage);

// Load/Save rankings from/to CSV
bool ranking_load_from_csv(const char* filename);
bool ranking_save_to_csv(const char* filename);

// Get ranking data
RankingEntry* ranking_get_entry(int index);
int ranking_get_count(void);
int ranking_get_player_rank(int score);

// Display rankings
void ranking_draw(double camera_x, double camera_y);

// Check if score qualifies for ranking
bool ranking_is_high_score(int score);

#endif // RANKING_H
