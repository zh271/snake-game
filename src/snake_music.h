#ifndef SNAKE_MUSIC_H
#define SNAKE_MUSIC_H

#define SOUND_BGM_START      1
#define SOUND_BGM_STOP       2

#define SOUND_TURN           3

#define SOUND_EAT_FOOD       4
#define SOUND_EAT_BIG_FOOD   5

#define SOUND_EAT_MAGNET     6
#define SOUND_EAT_DRILL      7
#define SOUND_EAT_DIGEST     8
#define SOUND_EAT_SLOW       9

#define SOUND_DEAD          10

#ifdef __cplusplus
extern "C" {
#endif

void GameAudio_play(int soundType);
void GameAudio_update(void);

#ifdef __cplusplus
}
#endif

#endif
