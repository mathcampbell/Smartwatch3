#ifndef SPD2010_TOUCH_H
#define SPD2010_TOUCH_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_XCA9554.h>

#define SPD2010_I2C_ADDRESS 0x53
#define SPD2010_MAX_TOUCH_POINTS 10

// Touch point structure
struct TouchPoint {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t weight;
};

// Touch data structure
struct TouchData {
    TouchPoint points[SPD2010_MAX_TOUCH_POINTS];
    uint8_t touch_count;
    uint8_t gesture;
    bool down;
    bool up;
    uint16_t down_x;
    uint16_t down_y;
    uint16_t up_x;
    uint16_t up_y;
};

// Status structures (internal use)
struct StatusLow {
    uint8_t pt_exist : 1;
    uint8_t gesture : 1;
    uint8_t key : 1;
    uint8_t aux : 1;
    uint8_t keep : 1;
    uint8_t raw_or_pt : 1;
    uint8_t none6 : 1;
    uint8_t none7 : 1;
};

struct StatusHigh {
    uint8_t none0 : 1;
    uint8_t none1 : 1;
    uint8_t none2 : 1;
    uint8_t cpu_run : 1;
    uint8_t tint_low : 1;
    uint8_t tic_in_cpu : 1;
    uint8_t tic_in_bios : 1;
    uint8_t tic_busy : 1;
};

struct TouchStatus {
    StatusLow status_low;
    StatusHigh status_high;
    uint16_t read_len;
};

struct HDPStatus {
    uint8_t status;
    uint16_t next_packet_len;
};

class SPD2010Touch {
public:
    // Constructor
    SPD2010Touch(TwoWire& wire = Wire, int reset_pin = -1, int interrupt_pin = -1, 
                 Adafruit_XCA9554* expander = nullptr);
    
    // Initialize the touch controller
    bool begin();
    
    // Hardware reset
    void reset();
    
    // Check if touch data is available
    bool available();
    
    // Read touch data
    bool read(TouchData& data);
    
    // Get single touch point (for simple use cases)
    bool getTouch(uint16_t& x, uint16_t& y, uint8_t& weight);
    
    // Get multiple touch points
    uint8_t getTouchPoints(TouchPoint* points, uint8_t max_points);
    
    // Check if currently touched
    bool isTouched();
    
    // Get gesture (if available)
    uint8_t getGesture();
    
    // Read firmware version
    bool readFirmwareVersion();
    
    // Set interrupt callback
    void setInterruptCallback(void (*callback)());    
    bool writeClearIntCommand();

    
private:
    TwoWire* _wire;
    int _reset_pin;
    int _interrupt_pin;
    TouchData _touch_data;
    bool _interrupt_flag;
    Adafruit_XCA9554* _expander;
    
    // Internal communication methods
    bool writeCommand(uint16_t reg, const uint8_t* data, uint8_t length);
    bool readRegister(uint16_t reg, uint8_t* data, uint8_t length);
    
    // Command methods
    bool writePointModeCommand();
    bool writeStartCommand();
    bool writeCpuStartCommand();
    
    // Status reading methods
    bool readStatusLength(TouchStatus& status);
    bool readHDP(const TouchStatus& status, TouchData& touch);
    bool readHDPStatus(HDPStatus& hdp_status);
    bool readHDPRemainData(const HDPStatus& hdp_status);
    
    // Main data reading method
    bool readTouchData(TouchData& touch);
    
    // Interrupt service routine (static)
    static void IRAM_ATTR interruptHandler();
    static SPD2010Touch* _instance;
};

#endif // SPD2010_TOUCH_H