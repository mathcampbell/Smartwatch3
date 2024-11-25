#include "PowerManager.h"

PowerManager::PowerManager(uint8_t pwrKeyPin, uint8_t controlPin, Adafruit_XCA9554 *expander)
    : _pwrKeyPin(pwrKeyPin), _controlPin(controlPin), _expander(expander),
      _longPressCount(0), _state(ACTIVE),
      _sleepTime(DEFAULT_SLEEP_TIME), _restartTime(DEFAULT_RESTART_TIME), _shutdownTime(DEFAULT_SHUTDOWN_TIME) {}

void PowerManager::init() {
    pinMode(_pwrKeyPin, INPUT);
    //_expander->pinMode(_controlPin, OUTPUT);
    pinMode(_controlPin, OUTPUT);
    pinMode(15, OUTPUT); 
    //_expander->digitalWrite(_controlPin, LOW);
    digitalWrite(_controlPin, LOW);
    // Check initial power button state
    delay(100);
    if (!digitalRead(_pwrKeyPin)) {
        _state = ACTIVE;
        //_expander->digitalWrite(_controlPin, HIGH);
        digitalWrite(_controlPin, HIGH);
    }
}

void PowerManager::update() {
    static unsigned long pressStartTime = 0;

    if (_state == ACTIVE && digitalRead(_pwrKeyPin) == LOW) {
        // Button is pressed
        if (pressStartTime == 0) {
            pressStartTime = millis();  // Record the start time of the press
        }
        
        unsigned long pressDuration = millis() - pressStartTime;

        if (pressDuration >= _shutdownTime) {
            _state = SHUTDOWN;
            shutdown();
            pressStartTime = 0;  // Reset the start time
        } else if (pressDuration >= _restartTime) {
            _state = RESTART;
            restart();
            pressStartTime = 0;
        } else if (pressDuration >= _sleepTime) {
            _state = SLEEP;
            goToSleep();
            pressStartTime = 0;
        }
    } else {
        // Button is released
        if (pressStartTime != 0) {
            // Reset long press counter if button was released before any action was taken
            pressStartTime = 0;
            _longPressCount = 0;
        }
    }
}

void PowerManager::setSleepTime(uint16_t time) { _sleepTime = time; }
void PowerManager::setRestartTime(uint16_t time) { _restartTime = time; }
void PowerManager::setShutdownTime(uint16_t time) { _shutdownTime = time; }

void PowerManager::goToSleep() {
    Serial.println("Going to sleep...");
    //_expander->digitalWrite(_controlPin, LOW);  // Turn off backlight
    digitalWrite(_controlPin, LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
    esp_deep_sleep_start();
}

void PowerManager::restart() {
    Serial.println("Restarting device...");
    // Restart logic here if needed
}

void PowerManager::shutdown() {
    Serial.println("Shutting down device...");
    //_expander->digitalWrite(_controlPin, LOW);  // Turn off backlight
    digitalWrite(_controlPin, LOW);
    // Additional shutdown logic if necessary
}
