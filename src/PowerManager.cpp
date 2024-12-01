#include "PowerManager.h"
#include "esp_sleep.h"


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
            pressStartTime = 0; 
            shutdown();
             // Reset the start time
        } else if (pressDuration >= _restartTime) {
            _state = RESTART;
            pressStartTime = 0;
            restart();
            
        } else if (pressDuration >= _sleepTime) {
            _state = SLEEP;
            pressStartTime = 0;
            goToSleep();
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

void PowerManager::handle_wakeup_callback(void) {
    Serial.println("Waking up from light sleep...");
    initBacklight(); // Re-enable the backlight
}


void PowerManager::initBacklight() {
    // Configure LEDC timer
    ledcSetup(LEDC_CHANNEL, LEDC_FREQUENCY, LEDC_RESOLUTION);
    
    // Attach the LEDC channel to the GPIO pin
    ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL);
    
    // Set the initial brightness (e.g., 100%)
    setBacklightBrightness(100);
}

void PowerManager::setBacklightBrightness(uint8_t brightness) {
    if (brightness > 100) brightness = 100; // Cap the brightness at 100%
    
    // Calculate the duty cycle based on the resolution
    uint32_t dutyCycle = (pow(2, LEDC_RESOLUTION) - 1) * brightness / 100;
    
    // Write the duty cycle to the LEDC channel
    ledcWrite(LEDC_CHANNEL, dutyCycle);
}

void PowerManager::turnOffBacklight() {
    setBacklightBrightness(0); // Set brightness to 0%
    // Optionally, detach the pin to reduce power consumption
    ledcDetachPin(BACKLIGHT_PIN);
}



void PowerManager::setSleepTime(uint16_t time) { _sleepTime = time; }
void PowerManager::setRestartTime(uint16_t time) { _restartTime = time; }
void PowerManager::setShutdownTime(uint16_t time) { _shutdownTime = time; }

void PowerManager::goToSleep() {
    Serial.println("Going to sleep now...");
    //_expander->digitalWrite(_controlPin, LOW);  // Turn off backlight
    digitalWrite(_controlPin, HIGH);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
    //esp_deep_sleep_start();
    turnOffBacklight();
    esp_light_sleep_start();
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
