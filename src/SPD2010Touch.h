#ifndef SPD2010_TOUCH_H
#define SPD2010_TOUCH_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_XCA9554.h>

#define SPD2010_I2C_ADDRESS        0x53
#define SPD2010_MAX_TOUCH_POINTS   10

// Register used to enable/disable individual interrupts

#define SPD2010_AUX_INT_BIT        0x08
#define SPD2010_INT_MASK_POINT   0x0001
#define SPD2010_INT_MASK_REG  0x0062  // INT flag/mask register (16-bit)
#define SPD2010_INT_MASK_GEST    0x0002
#define SPD2010_INT_MASK_KEY     0x0004
#define SPD2010_INT_MASK_AUX     0x0008
#define SPD2010_POWER_MODE_REG   0x002F  // 0x0001 = NM, 0x0002 = LPM




// ---------------------- Touch Data Structures ----------------------

struct TouchPoint {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t weight;
};

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

struct StatusLow {
    uint8_t pt_exist    : 1;
    uint8_t gesture     : 1;
    uint8_t key         : 1;
    uint8_t aux         : 1;
    uint8_t keep        : 1;
    uint8_t raw_or_pt   : 1;
    uint8_t none6       : 1;
    uint8_t none7       : 1;
};

struct StatusHigh {
    uint8_t none0       : 1;
    uint8_t none1       : 1;
    uint8_t none2       : 1;
    uint8_t cpu_run     : 1;
    uint8_t tint_low    : 1;
    uint8_t tic_in_cpu  : 1;
    uint8_t tic_in_bios : 1;
    uint8_t tic_busy    : 1;
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

// ---------------------- Class Declaration ----------------------

class SPD2010Touch {
public:
    SPD2010Touch(TwoWire& wire = Wire, int reset_pin = -1, int interrupt_pin = -1,
                 Adafruit_XCA9554* expander = nullptr);

    bool begin();
    void reset();

    bool available();
    bool read(TouchData& data);
    bool getTouch(uint16_t& x, uint16_t& y, uint8_t& weight);
    uint8_t getTouchPoints(TouchPoint* points, uint8_t max_points);
    bool isTouched();
    uint8_t getGesture();

    bool readFirmwareVersion();
    void setInterruptCallback(void (*callback)());
    bool writeClearIntCommand(bool rearm);
    bool enableAuxInterrupt(bool enable);

    bool prepareForSleepWake();
    void setActive();
    bool setPowerModeNormal();
    bool setPowerModeLow();

    bool waitIntHigh(uint32_t timeout_ms);

    

private:
    TwoWire* _wire;
    int _reset_pin;
    int _interrupt_pin;
    TouchData _touch_data;
    bool _interrupt_flag;
    Adafruit_XCA9554* _expander;

   
    void setIdle();

    bool writeCommand(uint16_t reg, const uint8_t* data, uint8_t length);
    bool readRegister(uint16_t reg, uint8_t* data, uint8_t length);

    bool writePointModeCommand();
    bool writeStartCommand();
    bool writeCpuStartCommand();

    bool readStatusLength(TouchStatus& status);
    bool readHDP(const TouchStatus& status, TouchData& touch);
    bool readHDPStatus(HDPStatus& hdp_status);
    bool readHDPRemainData(const HDPStatus& hdp_status);

    bool readTouchData(TouchData& touch);

    static void IRAM_ATTR interruptHandler();
    static SPD2010Touch* _instance;
};

#endif // SPD2010_TOUCH_H
