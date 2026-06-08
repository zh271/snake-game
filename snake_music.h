#ifndef AUDIO_H
#define AUDIO_H

/*
========================================
游戏音效类型定义
========================================
*/

/* 背景音乐 */
#define SOUND_BGM_START      1
#define SOUND_BGM_STOP       2

/* 转向 */
#define SOUND_TURN           3

/* 食物 */
#define SOUND_EAT_FOOD       4
#define SOUND_EAT_BIG_FOOD   5

/* 道具 */
#define SOUND_EAT_MAGNET     6
#define SOUND_EAT_DRILL      7
#define SOUND_EAT_DIGEST     8
#define SOUND_EAT_SLOW       9

/* 死亡 */
#define SOUND_DEAD          10

/*
========================================
音效播放函数
========================================
*/
void GameAudio_play(int soundType);

#endif