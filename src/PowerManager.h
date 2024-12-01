#pragma once
#include "Arduino.h"
#include <Adafruit_XCA9554.h>  // For I/O Expander backlight control

// Define power states and timing thresholds
enum DeviceState { ACTIVE, SLEEP, RESTART, SHUTDOWN };

// Default timing values (in units of 50 ms)
#define DEFAULT_SLEEP_TIME     10
#define DEFAULT_RESTART_TIME   40
#define DEFAULT_SHUTDOWN_TIME  60

// Define LEDC parameters
#define BACKLIGHT_PIN      5      // Replace with your actual backlight control pin
#define LEDC_CHANNEL       1       // LEDC channel (0-15)
//#define LEDC_TIMER         500     // LEDC timer (0-3)
#define LEDC_FREQUENCY     20000    // PWM frequency in Hertz
#define LEDC_RESOLUTION    10      // PWM resolution in bits (1-16)

class PowerManager {
public:
    PowerManager(uint8_t pwrKeyPin, uint8_t controlPin, Adafruit_XCA9554 *expander);
    void init();
    void update();
    void setSleepTime(uint16_t time);
    void setRestartTime(uint16_t time);
    void setShutdownTime(uint16_t time);
    void goToSleep();
    void restart();
    void shutdown();
    void initBacklight();
    void setBacklightBrightness(uint8_t brightness);
    void turnOffBacklight();
    void handle_wakeup_callback(void);


private:
    
    

    uint8_t _pwrKeyPin;
    uint8_t _controlPin;
    uint16_t _longPressCount;
    uint16_t _sleepTime;
    uint16_t _restartTime;
    uint16_t _shutdownTime;
    DeviceState _state;
    Adafruit_XCA9554 *_expander;
};
