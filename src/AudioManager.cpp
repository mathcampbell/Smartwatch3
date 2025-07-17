#include "AudioManager.h"
#include <SD.h>

void AudioManager::begin(int bck, int lrck, int data, uint32_t hz)
{
    /* 1. bring up SD – the standard Arduino SD.h works fine */
    if (!SD.begin()) {
        Serial.println("SD init failed");   // keep loud until stable
        return;
    }

    /* 2. configure the PCM5101 I²S bus */
    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck  = bck;
    cfg.pin_ws   = lrck;
    cfg.pin_data = data;
    cfg.sample_rate     = hz;
    cfg.bits_per_sample = 16;
    cfg.channels        = 2;
    i2s.begin(cfg);

    /* 3. prime the player – nothing plays until play() is called */
    player.begin();
}

/* Non-blocking one-shot playback */
void AudioManager::play(const char* path)
{
    source.selectStream(path);   // point the source at that file
    player.begin();              // (re)start decoding pipeline
}
