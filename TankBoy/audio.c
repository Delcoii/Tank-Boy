#include "audio.h"
#include <stdio.h>

// Global audio system instance
AudioSystem audio_system = {0};

void audio_init(void) {
    printf("Initializing audio system...\n");
    
    // Initialize Allegro audio
    if (!al_install_audio()) {
        printf("ERROR: Could not install audio system\n");
        return;
    }
    printf("Audio system installed successfully\n");
    
    // Initialize audio codec addon
    if (!al_init_acodec_addon()) {
        printf("ERROR: Could not initialize audio codec addon\n");
        return;
    }
    printf("Audio codec addon initialized successfully\n");
    
    // Reserve audio samples
    if (!al_reserve_samples(16)) {
        printf("ERROR: Could not reserve audio samples\n");
        return;
    }
    printf("Reserved 16 audio samples\n");
    
    // Load intro BGM
    audio_system.intro_bgm = al_load_sample("TankBoy/resources/flacs/intro_bgm.flac");
    if (!audio_system.intro_bgm) {
        printf("ERROR: Could not load intro_bgm.flac\n");
        return;
    }
    printf("intro_bgm.flac loaded successfully\n");
    
    // Load ingame BGM
    audio_system.ingame_bgm = al_load_sample("TankBoy/resources/flacs/ingame.flac");
    if (!audio_system.ingame_bgm) {
        printf("ERROR: Could not load ingame.flac\n");
        return;
    }
    printf("ingame.flac loaded successfully\n");
    
    // Create sample instance for intro BGM
    audio_system.intro_bgm_instance = al_create_sample_instance(audio_system.intro_bgm);
    if (!audio_system.intro_bgm_instance) {
        printf("ERROR: Could not create intro BGM sample instance\n");
        return;
    }
    printf("Intro BGM sample instance created successfully\n");
    
    // Create sample instance for ingame BGM
    audio_system.ingame_bgm_instance = al_create_sample_instance(audio_system.ingame_bgm);
    if (!audio_system.ingame_bgm_instance) {
        printf("ERROR: Could not create ingame BGM sample instance\n");
        return;
    }
    printf("Ingame BGM sample instance created successfully\n");
    
    // Configure intro BGM sample instance
    al_set_sample_instance_playmode(audio_system.intro_bgm_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(audio_system.intro_bgm_instance, 0.3); // 30% volume
    al_set_sample_instance_speed(audio_system.intro_bgm_instance, 1.0); // Normal speed
    
    // Configure ingame BGM sample instance
    al_set_sample_instance_playmode(audio_system.ingame_bgm_instance, ALLEGRO_PLAYMODE_LOOP);
    al_set_sample_instance_gain(audio_system.ingame_bgm_instance, 0.3); // 30% volume
    al_set_sample_instance_speed(audio_system.ingame_bgm_instance, 1.0); // Normal speed
    
    // Attach to default mixer
    al_attach_sample_instance_to_mixer(audio_system.intro_bgm_instance, al_get_default_mixer());
    al_attach_sample_instance_to_mixer(audio_system.ingame_bgm_instance, al_get_default_mixer());
    printf("Sample instances configured and attached to mixer\n");
    
    // Set initial audio state to menu
    audio_system.current_audio_state = AUDIO_STATE_MENU;
    
    audio_system.is_initialized = true;
    printf("Audio system initialized successfully\n");
}

void audio_cleanup(void) {
    if (audio_system.intro_bgm_instance) {
        al_destroy_sample_instance(audio_system.intro_bgm_instance);
        audio_system.intro_bgm_instance = NULL;
    }
    
    if (audio_system.ingame_bgm_instance) {
        al_destroy_sample_instance(audio_system.ingame_bgm_instance);
        audio_system.ingame_bgm_instance = NULL;
    }
    
    if (audio_system.intro_bgm) {
        al_destroy_sample(audio_system.intro_bgm);
        audio_system.intro_bgm = NULL;
    }
    
    if (audio_system.ingame_bgm) {
        al_destroy_sample(audio_system.ingame_bgm);
        audio_system.ingame_bgm = NULL;
    }
    
    audio_system.is_initialized = false;
    printf("Audio system cleaned up\n");
}

void play_intro_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        printf("WARNING: Audio system not initialized, cannot play intro BGM\n");
        return;
    }
    
    // Check if already playing
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        printf("Intro BGM is already playing\n");
        return;
    }
    
    // Play the sample instance
    al_play_sample_instance(audio_system.intro_bgm_instance);
    printf("Intro BGM playback started (normal speed, 30%% volume)\n");
    
    // Verify it's playing
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        printf("Intro BGM is now playing successfully\n");
    } else {
        printf("ERROR: Intro BGM failed to start playing\n");
    }
}

void play_ingame_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.ingame_bgm_instance) {
        printf("WARNING: Audio system not initialized, cannot play ingame BGM\n");
        return;
    }
    
    // Check if already playing
    if (al_get_sample_instance_playing(audio_system.ingame_bgm_instance)) {
        printf("Ingame BGM is already playing\n");
        return;
    }
    
    // Play the sample instance
    al_play_sample_instance(audio_system.ingame_bgm_instance);
    printf("Ingame BGM playback started (normal speed, 30%% volume)\n");
    
    // Verify it's playing
    if (al_get_sample_instance_playing(audio_system.ingame_bgm_instance)) {
        printf("Ingame BGM is now playing successfully\n");
    } else {
        printf("ERROR: Ingame BGM failed to start playing\n");
    }
}

void stop_intro_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return;
    }
    
    if (al_get_sample_instance_playing(audio_system.intro_bgm_instance)) {
        al_stop_sample_instance(audio_system.intro_bgm_instance);
        printf("Intro BGM stopped\n");
    }
}

void stop_ingame_bgm(void) {
    if (!audio_system.is_initialized || !audio_system.ingame_bgm_instance) {
        return;
    }
    
    if (al_get_sample_instance_playing(audio_system.ingame_bgm_instance)) {
        al_stop_sample_instance(audio_system.ingame_bgm_instance);
        printf("Ingame BGM stopped\n");
    }
}

bool is_intro_bgm_playing(void) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return false;
    }
    
    return al_get_sample_instance_playing(audio_system.intro_bgm_instance);
}

bool is_ingame_bgm_playing(void) {
    if (!audio_system.is_initialized || !audio_system.ingame_bgm_instance) {
        return false;
    }
    
    return al_get_sample_instance_playing(audio_system.ingame_bgm_instance);
}

void set_intro_bgm_volume(float volume) {
    if (!audio_system.is_initialized || !audio_system.intro_bgm_instance) {
        return;
    }
    
    // Clamp volume between 0.0 and 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    al_set_sample_instance_gain(audio_system.intro_bgm_instance, volume);
    printf("Intro BGM volume set to %.1f%%\n", volume * 100.0f);
}

void set_ingame_bgm_volume(float volume) {
    if (!audio_system.is_initialized || !audio_system.ingame_bgm_instance) {
        return;
    }
    
    // Clamp volume between 0.0 and 1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    al_set_sample_instance_gain(audio_system.ingame_bgm_instance, volume);
    printf("Ingame BGM volume set to %.1f%%\n", volume * 100.0f);
}

void switch_to_menu_audio(void) {
    if (!audio_system.is_initialized) {
        return;
    }
    
    if (audio_system.current_audio_state == AUDIO_STATE_MENU) {
        return; // Already in menu audio state
    }
    
    // Stop ingame BGM
    stop_ingame_bgm();
    
    // Start intro BGM
    play_intro_bgm();
    
    audio_system.current_audio_state = AUDIO_STATE_MENU;
    printf("Switched to menu audio (intro BGM)\n");
}

void switch_to_game_audio(void) {
    if (!audio_system.is_initialized) {
        return;
    }
    
    if (audio_system.current_audio_state == AUDIO_STATE_GAME) {
        return; // Already in game audio state
    }
    
    // Stop intro BGM
    stop_intro_bgm();
    
    // Start ingame BGM
    play_ingame_bgm();
    
    audio_system.current_audio_state = AUDIO_STATE_GAME;
    printf("Switched to game audio (ingame BGM)\n");
}
