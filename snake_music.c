/*
函数名称：GameAudio_play

函数功能：
    播放指定类型的游戏音效。

    本函数统一管理游戏中的所有声音效果，
    包括背景音乐、转向音效、吃食物音效、
    吃道具音效以及死亡音效。

    调用者只需要传入对应的音效类型，
    本函数会自动播放对应声音文件。

传入参数：
    soundType：
        要播放的音效类型。

        SOUND_BGM_START
            开始循环播放背景音乐。

        SOUND_BGM_STOP
            停止背景音乐。

        SOUND_TURN
            蛇转向音效。

        SOUND_EAT_FOOD
            吃普通食物音效。

        SOUND_EAT_BIG_FOOD
            吃大食物音效。

        SOUND_EAT_MAGNET
            吃磁铁音效。

        SOUND_EAT_DRILL
            吃钻头音效。

        SOUND_EAT_DIGEST
            吃消食片音效。

        SOUND_EAT_SLOW
            吃减速道具音效。

        SOUND_DEAD
            蛇死亡音效。

返回值：
    无返回值。
*/
void GameAudio_play(int soundType)
{
#ifdef _WIN32

    switch (soundType)
    {
        /*
            开始播放背景音乐。

            SND_LOOP：
                循环播放。

            SND_ASYNC：
                异步播放，不阻塞游戏运行。
        */
        case SOUND_BGM_START:

            PlaySound(
                TEXT("bgm.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC |
                SND_LOOP
            );

            break;

        /*
            停止当前播放的背景音乐。
        */
        case SOUND_BGM_STOP:

            PlaySound(NULL, NULL, 0);

            break;

        /*
            播放蛇转向音效。
        */
        case SOUND_TURN:

            PlaySound(
                TEXT("turn.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放普通食物音效。
        */
        case SOUND_EAT_FOOD:

            PlaySound(
                TEXT("eat.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放大食物音效。
        */
        case SOUND_EAT_BIG_FOOD:

            PlaySound(
                TEXT("bigfood.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放磁铁道具音效。
        */
        case SOUND_EAT_MAGNET:

            PlaySound(
                TEXT("magnet.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放钻头道具音效。
        */
        case SOUND_EAT_DRILL:

            PlaySound(
                TEXT("drill.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放消食片音效。
        */
        case SOUND_EAT_DIGEST:

            PlaySound(
                TEXT("digest.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放减速道具音效。
        */
        case SOUND_EAT_SLOW:

            PlaySound(
                TEXT("slow.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        /*
            播放蛇死亡音效。
        */
        case SOUND_DEAD:

            PlaySound(
                TEXT("dead.wav"),
                NULL,
                SND_FILENAME |
                SND_ASYNC
            );

            break;

        default:
            break;
    }

#endif
}