#include "ranking.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ===== Globals =====
static RankingSystem g_ranking_system;
static ALLEGRO_FONT* g_ranking_font = NULL;

// ===== Ranking System Management =====

void ranking_init(void) {
    // Initialize ranking system
    g_ranking_system.count = 0;
    memset(g_ranking_system.entries, 0, sizeof(g_ranking_system.entries));
    
    // Load existing rankings
    ranking_load_from_csv("TankBoy/rankings.csv");
    
    // Create font for ranking display
    g_ranking_font = al_create_builtin_font();
    if (!g_ranking_font) {
        printf("Warning: Could not create ranking font\n");
    }
    
    printf("Ranking system initialized with %d entries\n", g_ranking_system.count);
}

void ranking_cleanup(void) {
    // Save rankings before cleanup
    ranking_save_to_csv("TankBoy/rankings.csv");
    
    // Destroy font
    if (g_ranking_font) {
        al_destroy_font(g_ranking_font);
        g_ranking_font = NULL;
    }
    
    printf("Ranking system cleaned up\n");
}

// Alias function for compatibility
void ranking_deinit(void) {
    ranking_cleanup();
}

// ===== Score Management =====

void ranking_add_score(int score, int stage) {
    if (g_ranking_system.count >= MAX_RANKINGS) {
        // Remove lowest score if at capacity
        int lowest_index = 0;
        for (int i = 1; i < g_ranking_system.count; i++) {
            if (g_ranking_system.entries[i].score < g_ranking_system.entries[lowest_index].score) {
                lowest_index = i;
            }
        }
        
        // Replace lowest score if new score is higher
        if (score > g_ranking_system.entries[lowest_index].score) {
            g_ranking_system.entries[lowest_index].score = score;
            g_ranking_system.entries[lowest_index].stage = stage;
            strcpy(g_ranking_system.entries[lowest_index].name, DEFAULT_PLAYER_NAME);
            
            // Get current date/time
            time_t now = time(NULL);
            struct tm* tm_info = localtime(&now);
            strftime(g_ranking_system.entries[lowest_index].date, 20, "%Y-%m-%d %H:%M", tm_info);
        }
    } else {
        // Add new entry
        int index = g_ranking_system.count;
        g_ranking_system.entries[index].score = score;
        g_ranking_system.entries[index].stage = stage;
        strcpy(g_ranking_system.entries[index].name, DEFAULT_PLAYER_NAME);
        
        // Get current date/time
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(g_ranking_system.entries[index].date, 20, "%Y-%m-%d %H:%M", tm_info);
        
        g_ranking_system.count++;
    }
    
    // Sort rankings by score (highest first)
    for (int i = 0; i < g_ranking_system.count - 1; i++) {
        for (int j = 0; j < g_ranking_system.count - i - 1; j++) {
            if (g_ranking_system.entries[j].score < g_ranking_system.entries[j + 1].score) {
                RankingEntry temp = g_ranking_system.entries[j];
                g_ranking_system.entries[j] = g_ranking_system.entries[j + 1];
                g_ranking_system.entries[j + 1] = temp;
            }
        }
    }
    
    printf("Added score %d (stage %d) to rankings\n", score, stage);
}

void ranking_add_score_with_name(int score, int stage, const char* name) {
    if (g_ranking_system.count >= MAX_RANKINGS) {
        // Remove lowest score if at capacity
        int lowest_index = 0;
        for (int i = 1; i < g_ranking_system.count; i++) {
            if (g_ranking_system.entries[i].score < g_ranking_system.entries[lowest_index].score) {
                lowest_index = i;
            }
        }
        
        // Replace lowest score if new score is higher
        if (score > g_ranking_system.entries[lowest_index].score) {
            g_ranking_system.entries[lowest_index].score = score;
            g_ranking_system.entries[lowest_index].stage = stage;
            strncpy(g_ranking_system.entries[lowest_index].name, name, MAX_NAME_LENGTH - 1);
            g_ranking_system.entries[lowest_index].name[MAX_NAME_LENGTH - 1] = '\0';
            
            // Get current date/time
            time_t now = time(NULL);
            struct tm* tm_info = localtime(&now);
            strftime(g_ranking_system.entries[lowest_index].date, 20, "%Y-%m-%d %H:%M", tm_info);
        }
    } else {
        // Add new entry
        int index = g_ranking_system.count;
        g_ranking_system.entries[index].score = score;
        g_ranking_system.entries[index].stage = stage;
        strncpy(g_ranking_system.entries[index].name, name, MAX_NAME_LENGTH - 1);
        g_ranking_system.entries[index].name[MAX_NAME_LENGTH - 1] = '\0';
        
        // Get current date/time
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(g_ranking_system.entries[index].date, 20, "%Y-%m-%d %H:%M", tm_info);
        
        g_ranking_system.count++;
    }
    
    // Sort rankings by score (highest first)
    for (int i = 0; i < g_ranking_system.count - 1; i++) {
        for (int j = 0; j < g_ranking_system.count - i - 1; j++) {
            if (g_ranking_system.entries[j].score < g_ranking_system.entries[j + 1].score) {
                RankingEntry temp = g_ranking_system.entries[j];
                g_ranking_system.entries[j] = g_ranking_system.entries[j + 1];
                g_ranking_system.entries[j + 1] = temp;
            }
        }
    }
    
    printf("Added score %d (stage %d) with name '%s' to rankings\n", score, stage, name);
}

// ===== CSV File Operations =====

bool ranking_load_from_csv(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("No existing ranking file found, starting fresh\n");
        return false;
    }
    
    char line[256];
    int line_count = 0;
    
    // Skip header line
    if (fgets(line, sizeof(line), file)) {
        line_count++;
    }
    
    g_ranking_system.count = 0;
    
    while (fgets(line, sizeof(line), file) && g_ranking_system.count < MAX_RANKINGS) {
        line_count++;
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        char* token = strtok(line, ",");
        if (!token) continue;
        
        // Parse name
        strncpy(g_ranking_system.entries[g_ranking_system.count].name, token, MAX_NAME_LENGTH - 1);
        g_ranking_system.entries[g_ranking_system.count].name[MAX_NAME_LENGTH - 1] = 0;
        
        // Parse score
        token = strtok(NULL, ",");
        if (!token) continue;
        g_ranking_system.entries[g_ranking_system.count].score = atoi(token);
        
        // Parse stage
        token = strtok(NULL, ",");
        if (!token) continue;
        g_ranking_system.entries[g_ranking_system.count].stage = atoi(token);
        
        // Parse date
        token = strtok(NULL, ",");
        if (token) {
            strncpy(g_ranking_system.entries[g_ranking_system.count].date, token, 19);
            g_ranking_system.entries[g_ranking_system.count].date[19] = 0;
        } else {
            strcpy(g_ranking_system.entries[g_ranking_system.count].date, "Unknown");
        }
        
        g_ranking_system.count++;
    }
    
    fclose(file);
    printf("Loaded %d ranking entries from %s\n", g_ranking_system.count, filename);
    return true;
}

bool ranking_save_to_csv(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not create ranking file %s\n", filename);
        return false;
    }
    
    // Write header
    fprintf(file, "Name,Score,Stage,Date\n");
    
    // Write entries
    for (int i = 0; i < g_ranking_system.count; i++) {
        fprintf(file, "%s,%d,%d,%s\n",
                g_ranking_system.entries[i].name,
                g_ranking_system.entries[i].score,
                g_ranking_system.entries[i].stage,
                g_ranking_system.entries[i].date);
    }
    
    fclose(file);
    printf("Saved %d ranking entries to %s\n", g_ranking_system.count, filename);
    return true;
}

// ===== Getter Functions =====

RankingEntry* ranking_get_entry(int index) {
    if (index >= 0 && index < g_ranking_system.count) {
        return &g_ranking_system.entries[index];
    }
    return NULL;
}

int ranking_get_count(void) {
    return g_ranking_system.count;
}

int ranking_get_player_rank(int score) {
    for (int i = 0; i < g_ranking_system.count; i++) {
        if (score >= g_ranking_system.entries[i].score) {
            return i + 1;  // Return 1-based rank
        }
    }
    return g_ranking_system.count + 1;  // Last place
}

// ===== Display Functions =====

void ranking_draw(double camera_x, double camera_y) {
    if (!g_ranking_font) return;
    
    // Draw title
    al_draw_text(g_ranking_font, al_map_rgb(255, 255, 255), 50, 50, 0, "=== HIGH SCORES ===");
    
    // Draw header
    al_draw_text(g_ranking_font, al_map_rgb(200, 200, 200), 50, 100, 0, "Rank  Name      Score   Stage   Date");
    
    // Draw rankings
    for (int i = 0; i < g_ranking_system.count && i < 20; i++) {  // Show top 20
        RankingEntry* entry = &g_ranking_system.entries[i];
        char line[256];
        
        // Format: "1.    aaaaa    15000   3      2024-01-01 12:00"
        snprintf(line, sizeof(line), "%2d.   %-10s %6d   %2d      %s",
                i + 1, entry->name, entry->score, entry->stage, entry->date);
        
        // Highlight current player's score
        ALLEGRO_COLOR color = (strcmp(entry->name, DEFAULT_PLAYER_NAME) == 0) ? 
                             al_map_rgb(255, 255, 0) : al_map_rgb(255, 255, 255);
        
        al_draw_text(g_ranking_font, color, 50, 130 + i * 25, 0, line);
    }
    
    // Draw instructions
    al_draw_text(g_ranking_font, al_map_rgb(150, 150, 150), 50, 700, 0, "Press ESC to return to main menu");
}

// ===== Utility Functions =====

bool ranking_is_high_score(int score) {
    if (g_ranking_system.count < MAX_RANKINGS) {
        return true;  // Always qualify if not at capacity
    }
    
    // Check if score is higher than lowest existing score
    int lowest_score = g_ranking_system.entries[g_ranking_system.count - 1].score;
    return score > lowest_score;
}
