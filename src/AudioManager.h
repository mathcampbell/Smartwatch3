#pragma once
#include "AudioTools.h"
#include "AudioTools/Disk/AudioSourceSD.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/Disk/AudioSourceLittleFS.h"

using namespace audio_tools;          // shortens the names

class AudioManager {
public:
    static AudioManager& instance() {
        static AudioManager mgr;
        return mgr;
    }

    // Call once from setup()
    void begin(int bck, int lrck, int data, uint32_t hz = 44100);

    // Play any WAV or MP3 on the SD card (non-blocking)
    void play(const char* path);

    // Call every loop() tick
    inline void poll() { player.copy(); }

private:
    AudioManager() = default;

    /* -- AudioTools objects ------------------------------------------------ */
    AudioSourceLittleFS    source{"/", ".mp3"};   // start path, default ext filter
    I2SStream        i2s;
    MP3DecoderHelix  decoder;
    AudioPlayer      player{source, i2s, decoder};
};
