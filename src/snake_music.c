#include "snake_music.h"

#include "raylib.h"
#include <stdio.h>

typedef struct EffectSlot {
    int soundType;
    const char *filename;
    Sound sound;
    bool loaded;
} EffectSlot;

static bool audioReady = false;
static bool bgmLoaded = false;
static Music bgm;

static EffectSlot effects[] = {
    { SOUND_TURN, "turn.wav", { 0 }, false },
    { SOUND_EAT_FOOD, "eat.wav", { 0 }, false },
    { SOUND_EAT_BIG_FOOD, "bigfood.wav", { 0 }, false },
    { SOUND_EAT_MAGNET, "magnet.wav", { 0 }, false },
    { SOUND_EAT_DRILL, "drill.wav", { 0 }, false },
    { SOUND_EAT_DIGEST, "digest.wav", { 0 }, false },
    { SOUND_EAT_SLOW, "slow.wav", { 0 }, false },
    { SOUND_DEAD, "dead.wav", { 0 }, false },
};

static bool resolve_audio_path(const char *filename, char *outPath, int outSize)
{
    const char *workingFormats[] = {
        "src/music/%s",
        "resources/music/%s",
        "music/%s",
        "%s",
    };

    for (int i = 0; i < (int)(sizeof(workingFormats) / sizeof(workingFormats[0])); ++i) {
        snprintf(outPath, outSize, workingFormats[i], filename);
        if (FileExists(outPath)) {
            return true;
        }
    }

    const char *appDir = GetApplicationDirectory();
    const char *appFormats[] = {
        "%s../../src/music/%s",
        "%smusic/%s",
        "%s%s",
    };

    for (int i = 0; i < (int)(sizeof(appFormats) / sizeof(appFormats[0])); ++i) {
        snprintf(outPath, outSize, appFormats[i], appDir, filename);
        if (FileExists(outPath)) {
            return true;
        }
    }

    snprintf(outPath, outSize, "src/music/%s", filename);
    return false;
}

static bool ensure_audio_ready(void)
{
    if (!audioReady) {
        InitAudioDevice();
        audioReady = IsAudioDeviceReady();

        if (!audioReady) {
            TraceLog(LOG_WARNING, "Audio device failed to initialize");
        }
    }

    return audioReady;
}

static EffectSlot *find_effect(int soundType)
{
    for (int i = 0; i < (int)(sizeof(effects) / sizeof(effects[0])); ++i) {
        if (effects[i].soundType == soundType) {
            return &effects[i];
        }
    }

    return NULL;
}

static void unload_all_audio(void)
{
    if (bgmLoaded) {
        StopMusicStream(bgm);
        UnloadMusicStream(bgm);
        bgmLoaded = false;
    }

    for (int i = 0; i < (int)(sizeof(effects) / sizeof(effects[0])); ++i) {
        if (effects[i].loaded) {
            UnloadSound(effects[i].sound);
            effects[i].loaded = false;
        }
    }

    if (audioReady) {
        CloseAudioDevice();
        audioReady = false;
    }
}

static void play_bgm(void)
{
    char path[512];

    if (!ensure_audio_ready()) {
        return;
    }

    if (!bgmLoaded) {
        if (!resolve_audio_path("bgm.wav", path, (int)sizeof(path))) {
            TraceLog(LOG_WARNING, "Audio file not found: bgm.wav");
            return;
        }

        bgm = LoadMusicStream(path);
        if (bgm.stream.buffer == NULL) {
            TraceLog(LOG_WARNING, "Failed to load music: %s", path);
            return;
        }

        bgm.looping = true;
        SetMusicVolume(bgm, 0.65f);
        bgmLoaded = true;
    }

    if (!IsMusicStreamPlaying(bgm)) {
        PlayMusicStream(bgm);
    }
}

static void play_effect(int soundType)
{
    char path[512];
    EffectSlot *slot = find_effect(soundType);

    if (slot == NULL || !ensure_audio_ready()) {
        return;
    }

    if (!slot->loaded) {
        if (!resolve_audio_path(slot->filename, path, (int)sizeof(path))) {
            TraceLog(LOG_WARNING, "Audio file not found: %s", slot->filename);
            return;
        }

        slot->sound = LoadSound(path);
        if (slot->sound.stream.buffer == NULL) {
            TraceLog(LOG_WARNING, "Failed to load sound: %s", path);
            return;
        }

        SetSoundVolume(slot->sound, 0.85f);
        slot->loaded = true;
    }

    PlaySound(slot->sound);
}

void GameAudio_update(void)
{
    if (audioReady && bgmLoaded) {
        UpdateMusicStream(bgm);
    }
}

void GameAudio_play(int soundType)
{
    switch (soundType) {
    case SOUND_BGM_START:
        play_bgm();
        break;

    case SOUND_BGM_STOP:
        unload_all_audio();
        break;

    case SOUND_DEAD:
        play_effect(SOUND_DEAD);
        break;

    case SOUND_TURN:
    case SOUND_EAT_FOOD:
    case SOUND_EAT_BIG_FOOD:
    case SOUND_EAT_MAGNET:
    case SOUND_EAT_DRILL:
    case SOUND_EAT_DIGEST:
    case SOUND_EAT_SLOW:
        play_effect(soundType);
        break;

    default:
        break;
    }
}
