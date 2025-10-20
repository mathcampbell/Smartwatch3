#include "SPD2010Touch.h"

// Static instance for interrupt handling
SPD2010Touch* SPD2010Touch::_instance = nullptr;

SPD2010Touch::SPD2010Touch(TwoWire& wire, int reset_pin, int interrupt_pin, Adafruit_XCA9554* expander)
    : _wire(&wire), _reset_pin(reset_pin), _interrupt_pin(interrupt_pin), _expander(expander), _interrupt_flag(false) {
    memset(&_touch_data, 0, sizeof(_touch_data));
    _instance = this;
}

// NEW: track low-power state
bool _in_lpm = false;  // add as a private member


bool SPD2010Touch::begin() {
    if (!_wire) return false;

    // Reset pin is on the expander in this design
    _expander->pinMode(_reset_pin, OUTPUT);
    _expander->digitalWrite(_reset_pin, HIGH);

    // Interrupt pin
    if (_interrupt_pin >= 0) {
        pinMode(_interrupt_pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(_interrupt_pin), interruptHandler, FALLING);
    }

    // Reset the controller
    reset();

    // Optional: print version info (don’t return here; we still need to configure)
    delay(100);
    readFirmwareVersion();

    // Make sure we’re in CPU mode and scanning in point mode
    setActive();   // will CPU-start if BIOS, set point mode, start scan, clear INT

    // ===== Mask AUX events: only allow Point + Gesture + Key =====
    // Write an explicit mask so AUX will NOT assert INT ever.
    // (Some firmwares ignore read-modify-write; set the bits explicitly.)
    {
        uint16_t mask = SPD2010_INT_MASK_POINT | SPD2010_INT_MASK_GEST | SPD2010_INT_MASK_KEY; // 0x0007
        uint8_t m[2] = { (uint8_t)(mask & 0xFF), (uint8_t)(mask >> 8) };
        writeCommand(SPD2010_INT_MASK_REG, m, 2);
    }

    // If you *never* use keys, you can reduce to touches+gestures only:
    // uint16_t mask = SPD2010_INT_MASK_POINT | SPD2010_INT_MASK_GEST; // 0x0003

    // Default to Normal mode at boot
    setPowerModeNormal();

    return true;
}

void SPD2010Touch::reset() {
    _expander->digitalWrite(_reset_pin, LOW);
    delay(50);
    _expander->digitalWrite(_reset_pin, HIGH);
    delay(50);
}

bool SPD2010Touch::available() {
    if (_interrupt_flag) {
        Serial.printf("INT fired at %lu ms\n", millis());
    }
    return _interrupt_flag;
}

bool SPD2010Touch::read(TouchData& data) {
    if (readTouchData(data)) {
        _interrupt_flag = false;
        return true;
    }
    return false;
}

bool SPD2010Touch::getTouch(uint16_t& x, uint16_t& y, uint8_t& weight) {
    TouchData data;
    if (readTouchData(data) && data.touch_count > 0) {
        x = data.points[0].x;
        y = data.points[0].y;
        weight = data.points[0].weight;
        _interrupt_flag = false;
        return true;
    }
    return false;
}

uint8_t SPD2010Touch::getTouchPoints(TouchPoint* points, uint8_t max_points) {
    TouchData data;
    if (readTouchData(data)) {
        uint8_t count = min(data.touch_count, max_points);
        for (uint8_t i = 0; i < count; i++) {
            points[i] = data.points[i];
        }
        _interrupt_flag = false;
        return count;
    }
    return 0;
}

bool SPD2010Touch::isTouched() {
    TouchData data;
    if (readTouchData(data)) {
        _interrupt_flag = false;
        return data.touch_count > 0;
    }
    return false;
}

uint8_t SPD2010Touch::getGesture() {
    TouchData data;
    if (readTouchData(data)) {
        _interrupt_flag = false;
        return data.gesture;
    }
    return 0;
}

void SPD2010Touch::setInterruptCallback(void (*callback)()) {
    // Not used in this implementation; available() is polled.
}

/* -------------------- NEW: power-mode helpers -------------------- */

// Public helpers you can call around sleep/display state.
bool SPD2010Touch::setPowerModeNormal() {
  const uint8_t nm[2] = { 0x00, 0x90 };
  return writeCommand(0x0046, nm, 2)   // power = NM
      && writeClearIntCommand(false)   // ACK only
      && writePointModeCommand()       // 0x0050 ← 0x0000
      && writeStartCommand();          // 0x0046 ← 0x0000 (Touch START)
}


// 0x0046 ← 0x0190 (LPM)
bool SPD2010Touch::setPowerModeLow() {
  const uint8_t lpm[2] = { 0x01, 0x90 };
  // ACK once and DO NOT re-arm while sleeping
  return writeCommand(0x0046, lpm, 2) && writeClearIntCommand(false);
}

/* -------------------- NEW: runtime mode helpers -------------------- */

void SPD2010Touch::setActive() {
  // If BIOS, start CPU first
  TouchStatus s;
  if (readStatusLength(s) && s.status_high.tic_in_bios) {
    writeClearIntCommand(false);  // ACK-only
    writeCpuStartCommand();
    delay(2);
  }
  attachInterrupt(digitalPinToInterrupt(_interrupt_pin), interruptHandler, FALLING);

  // Back to NM and resume scanning
  setPowerModeNormal();           // does NM + ACK + POINT + START
  _in_lpm = false;
}

void SPD2010Touch::setIdle() {
  // Low Power Mode (LPM): 0x0046 ← 0x0190 (low,high)
  setPowerModeLow();
  // Clear once WITHOUT re-arming so the line stays quiet
  writeClearIntCommand(false);
  _in_lpm = true;
}

/* --------- prepareForSleepWake becomes a simple LPM wrapper ----- */
bool SPD2010Touch::waitIntHigh(uint32_t timeout_ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < timeout_ms) {
    if (_interrupt_pin < 0 || digitalRead(_interrupt_pin) == HIGH) return true;
    // TINT requested a clear; ACK-only so we do not re-arm
    writeClearIntCommand(false);
    delay(2);
  }
  return false;
}
bool SPD2010Touch::prepareForSleepWake() {
  Serial.println("Preparing SPD2010 for sleep-based wake-on-touch...");

  // LPM via 0x0046<-0x0190, ACK-only is correct
  //setPowerModeLow();

  // Drain TINT until pin goes HIGH (you already had this)
  bool ok = waitIntHigh(120);            // give it a bit more headroom
  if (!ok) {
    Serial.println("Abort sleep: SPD2010 INT stuck LOW after LPM.");
    _in_lpm = false;                     // we’re not sleeping
    return false;
  }

  // Belt-and-braces: confirm with one last SnL read that TINT==0
  TouchStatus s;
  if (readStatusLength(s) && s.status_high.tint_low) {
    // Still requesting clear? ACK once and re-check pin
    writeClearIntCommand(false);
    delay(2);
    if (_interrupt_pin >= 0 && digitalRead(_interrupt_pin) == LOW) {
      Serial.println("Abort sleep: SnL shows TINT low.");
      _in_lpm = false;
      return false;
    }
  }

 // detachInterrupt(digitalPinToInterrupt(_interrupt_pin));
 // _in_lpm = true;
//  Serial.println("SPD2010 in Low-Power Mode (LPM); will wake only on touch/gesture.");
  return true;
}

/* ------------------------------------------------------------------- */

bool SPD2010Touch::writeCommand(uint16_t reg, const uint8_t* data, uint8_t length) {
    _wire->beginTransmission(SPD2010_I2C_ADDRESS);
    _wire->write((uint8_t)reg);
    _wire->write((uint8_t)(reg >> 8));
    for (uint8_t i = 0; i < length; i++) _wire->write(data[i]);
    return _wire->endTransmission() == 0;
}

bool SPD2010Touch::readRegister(uint16_t reg, uint8_t* data, uint8_t length) {
    _wire->beginTransmission(SPD2010_I2C_ADDRESS);
    _wire->write((uint8_t)reg);
    _wire->write((uint8_t)(reg >> 8));
    if (_wire->endTransmission() != 0) return false;

    uint8_t received = _wire->requestFrom(SPD2010_I2C_ADDRESS, length);
    if (received != length) return false;

    for (uint8_t i = 0; i < length; i++) data[i] = _wire->read();
    return true;
}

bool SPD2010Touch::writePointModeCommand() {
    uint8_t data[2] = {0x00, 0x00};
    return writeCommand(0x0050, data, 2);
}

bool SPD2010Touch::writeStartCommand() {
    uint8_t data[2] = {0x00, 0x00};
    return writeCommand(0x0046, data, 2);
}

bool SPD2010Touch::writeCpuStartCommand() {
    uint8_t data[2] = {0x01, 0x00};
    return writeCommand(0x0004, data, 2);
}

bool SPD2010Touch::writeClearIntCommand(bool rearm) {
  static const uint8_t ack[2]  = { 0x01, 0x00 }; // ACK
  static const uint8_t arm[2]  = { 0x00, 0x00 }; // re‑arm
  if (!writeCommand(0x0002, ack, 2)) return false;
  if (rearm) {
    delayMicroseconds(200);
    if (!writeCommand(0x0002, arm, 2)) return false;
  }
  return true;
}

bool SPD2010Touch::readStatusLength(TouchStatus& status) {
    uint8_t data[4];
    if (!readRegister(0x0020, data, 4)) return false;

    // length, little-endian
    uint16_t len = (data[3] << 8) | data[2];
    if (len < 4 || len > 64) len = 0; // clamp
    status.read_len = len;

    delayMicroseconds(200);

    status.status_low.pt_exist  = (data[0] & 0x01);
    status.status_low.gesture   = (data[0] & 0x02);
    status.status_low.aux       = (data[0] & 0x08);

    status.status_high.tic_busy   = (data[1] & 0x80) >> 7;
    status.status_high.tic_in_bios= (data[1] & 0x40) >> 6;
    status.status_high.tic_in_cpu = (data[1] & 0x20) >> 5;
    status.status_high.tint_low   = (data[1] & 0x10) >> 4;
    status.status_high.cpu_run    = (data[1] & 0x08) >> 3;

    return true;
}

bool SPD2010Touch::enableAuxInterrupt(bool enable) {
    // Keep as a helper, but prefer explicit mask write in begin()
    uint8_t data[2];
    if (!readRegister(SPD2010_INT_MASK_REG, data, 2)) return false;
    uint16_t ms = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    if (enable) ms |=  SPD2010_INT_MASK_AUX;
    else        ms &= ~SPD2010_INT_MASK_AUX;
    data[0] = (uint8_t)(ms & 0xFF);
    data[1] = (uint8_t)(ms >> 8);
    return writeCommand(SPD2010_INT_MASK_REG, data, 2);
}

bool SPD2010Touch::readHDP(const TouchStatus& status, TouchData& touch) {
    uint8_t data[64];
    if (!readRegister(0x0300, data, status.read_len)) return false;

    uint8_t check_id = data[4];

    if (check_id <= 0x0A && status.status_low.pt_exist) {
        touch.touch_count = (status.read_len - 4) / 6;
        touch.gesture = 0x00;
        if (touch.touch_count > SPD2010_MAX_TOUCH_POINTS) {
            touch.touch_count = SPD2010_MAX_TOUCH_POINTS;
        }

        for (uint8_t i = 0; i < touch.touch_count; i++) {
            uint8_t offset = i * 6;
            touch.points[i].id = data[4 + offset];
            touch.points[i].x  = ((data[7 + offset] & 0xF0) << 4) | data[5 + offset];
            touch.points[i].y  = ((data[7 + offset] & 0x0F) << 8) | data[6 + offset];
            touch.points[i].weight = data[8 + offset];
        }

        if (touch.points[0].weight != 0 && !touch.down) {
            touch.down = true;  touch.up = false;
            touch.down_x = touch.points[0].x;
            touch.down_y = touch.points[0].y;
        } else if (touch.points[0].weight == 0 && touch.down) {
            touch.up = true; touch.down = false;
            touch.up_x = touch.points[0].x;
            touch.up_y = touch.points[0].y;
        }
    } else if (check_id == 0xF6 && status.status_low.gesture) {
        touch.touch_count = 0;
        touch.up = false; touch.down = false;
        touch.gesture = data[6] & 0x07;
    } else {
        touch.touch_count = 0;
        touch.gesture = 0;
    }

    return true;
}

bool SPD2010Touch::readHDPStatus(HDPStatus& hdp_status) {
/*     uint8_t data[8];
    if (!readRegister(0xFC02, data, 8)) return false;

    hdp_status.status = data[5]; // reference impl uses [5]
    hdp_status.next_packet_len = data[2] | (data[3] << 8);
    return true; */

    //these changes were suggested to match the reference.
        uint8_t data[8];
    if (!readRegister(0x02FC, data, 8)) return false;

    hdp_status.status = data[5]; // reference impl uses [5]
    hdp_status.next_packet_len = data[2] | (data[3] << 8);
    return true;
}

bool SPD2010Touch::readHDPRemainData(const HDPStatus& hdp_status) {
    uint8_t data[32];
    return readRegister(0x0300, data, hdp_status.next_packet_len);
}

bool SPD2010Touch::readFirmwareVersion() {
    uint8_t data[18];
    if (!readRegister(0x2600, data, 18)) return false;

    uint32_t dummy     = (data[0] << 24) | (data[1] << 16) | (data[3] << 8) | data[0];
    uint16_t dver      = (data[5] << 8) | data[4];
    uint32_t pid       = (data[9] << 24) | (data[8] << 16) | (data[7] << 8) | data[6];
    uint32_t ic_name_l = (data[13] << 24) | (data[12] << 16) | (data[11] << 8) | data[10];
    uint32_t ic_name_h = (data[17] << 24) | (data[16] << 16) | (data[15] << 8) | data[14];

    Serial.print("SPD2010 - Dummy: ");
    Serial.print(dummy);
    Serial.print(", Version: ");
    Serial.print(dver);
    Serial.print(", PID: ");
    Serial.print(pid);
    Serial.print(", IC Name: ");
    Serial.print(ic_name_h);
    Serial.print("-");
    Serial.println(ic_name_l);

    return true;
}

bool SPD2010Touch::readTouchData(TouchData& touch)
{
    if (_in_lpm) return false;
    
    // Gate reads: if there’s no IRQ edge and the line is not low, don’t poll the chip
    if (!_interrupt_flag && (_interrupt_pin >= 0) && digitalRead(_interrupt_pin) != LOW) {
        return false;
    }

    // Consume the flag set by ISR so subsequent polls wait
    _interrupt_flag = false;

    TouchStatus tp_status;
    HDPStatus   hdp_status;
    memset(&touch, 0, sizeof(touch));

    if (!readStatusLength(tp_status)) {
        return false;
    }

#if 1
  Serial.printf("INT state: pt=%d gest=%d aux=%d cpu_run=%d cpu=%d bios=%d busy=%d tint=%d\n",
  tp_status.status_low.pt_exist,
  tp_status.status_low.gesture,
  tp_status.status_low.aux,
  tp_status.status_high.cpu_run,
  tp_status.status_high.tic_in_cpu,
  tp_status.status_high.tic_in_bios,
  tp_status.status_high.tic_busy,
  tp_status.status_high.tint_low);
    
#endif

    // BIOS / CPU housekeeping
    if (tp_status.status_high.tic_in_bios) {
        writeClearIntCommand(false);
        writeCpuStartCommand();
        return false;
    }
    if (tp_status.status_high.tic_in_cpu) {
        writePointModeCommand();
        writeStartCommand();
        writeClearIntCommand(false);
        return false;
    }
    if (tp_status.status_high.cpu_run && tp_status.read_len == 0) {
        writeClearIntCommand(false);
        return false;
    }

    // Touch / Gesture present
    if (tp_status.status_low.pt_exist || tp_status.status_low.gesture) {
        readHDP(tp_status, touch);
        writeClearIntCommand(false);

        // Pull any remaining chunks
        while (true) {
            if (!readHDPStatus(hdp_status)) break;
            if (hdp_status.status == 0x82) {
                writeClearIntCommand(false);
                break;
            } else if (hdp_status.status == 0x00) {
                readHDPRemainData(hdp_status);
                continue;
            } else break;
        }
        return true;
    }

    // AUX only (shouldn’t wake now that it’s masked, but safe-guard)
    if (tp_status.status_high.cpu_run && tp_status.status_low.aux) {
        writeClearIntCommand(false);
        Serial.println("Skipping AUX-only INT");
        return false;
    }

    return false; // no touch data
}

void IRAM_ATTR SPD2010Touch::interruptHandler() {
    if (_instance) {
        _instance->_interrupt_flag = true;
    }
}