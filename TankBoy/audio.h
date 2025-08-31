#ifndef AUDIO_H
#define AUDIO_H

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

// Game states for audio management
typedef enum {
    AUDIO_STATE_MENU,
    AUDIO_STATE_GAME
} AudioState;

// Audio system structure
typedef struct {
    ALLEGRO_SAMPLE* intro_bgm;
    ALLEGRO_SAMPLE_INSTANCE* intro_bgm_instance;
    ALLEGRO_SAMPLE* ingame_bgm;
    ALLEGRO_SAMPLE_INSTANCE* ingame_bgm_instance;
    ALLEGRO_SAMPLE* machine_sound;  // Machine gun sound effect
    ALLEGRO_SAMPLE* cannon_sound;    // Cannon sound effect
    AudioState current_audio_state;
    bool is_initialized;
} AudioSystem;

// Function declarations
void audio_init(void);
void audio_cleanup(void);
void play_intro_bgm(void);
void play_ingame_bgm(void);
void stop_intro_bgm(void);
void stop_ingame_bgm(void);
bool is_intro_bgm_playing(void);
bool is_ingame_bgm_playing(void);
void set_intro_bgm_volume(float volume);
void set_ingame_bgm_volume(float volume);
void switch_to_menu_audio(void);
void switch_to_game_audio(void);
void play_machine_sound(void);  // Play machine gun sound effect
void play_cannon_sound(void);   // Play cannon sound effect

// Global audio system instance
extern AudioSystem audio_system;

#endif // AUDIO_H
