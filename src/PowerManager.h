#pragma once
#include "Arduino.h"
#include <Adafruit_XCA9554.h>  // For I/O Expander backlight control

// Define power states and timing thresholds
enum DeviceState { ACTIVE, SLEEP, RESTART, SHUTDOWN };

// Default timing values (in units of 50 ms)
#define DEFAULT_SLEEP_TIME     10
#define DEFAULT_RESTART_TIME   40
#define DEFAULT_SHUTDOWN_TIME  60

class PowerManager {
public:
    PowerManager(uint8_t pwrKeyPin, uint8_t controlPin, Adafruit_XCA9554 *expander);
    void init();
    void update();
    void setSleepTime(uint16_t time);
    void setRestartTime(uint16_t time);
    void setShutdownTime(uint16_t time);

private:
    void goToSleep();
    void restart();
    void shutdown();

    uint8_t _pwrKeyPin;
    uint8_t _controlPin;
    uint16_t _longPressCount;
    uint16_t _sleepTime;
    uint16_t _restartTime;
    uint16_t _shutdownTime;
    DeviceState _state;
    Adafruit_XCA9554 *_expander;
};
