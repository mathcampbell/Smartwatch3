#include "AudioManager.h"

/* Expose a plain C function that any .c file can call */
extern "C" void playSound(const char *path)
{
    AudioManager::instance().play(path);
}
