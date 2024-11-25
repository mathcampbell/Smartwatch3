#include "Touch_CST816.h"

struct CST816_Touch touch_data = {0};
uint8_t Touch_interrupts=0;
Adafruit_XCA9554 *expanderPtr;
/*!
    @brief  handle interrupts
*/
void ARDUINO_ISR_ATTR Touch_CST816_ISR(void) {
  Touch_interrupts = true;
}

uint8_t Touch_Init(Adafruit_XCA9554 *expander) {
  expanderPtr = expander;
  //Wire.begin(GPIO_NUM_1, GPIO_NUM_3, I2C_MASTER_FREQ_HZ);
  Wire.begin(GPIO_NUM_1, GPIO_NUM_3, 100000); // Set I2C clock speed to 100kHz

  CST816_Touch_Reset();
  uint16_t Verification = CST816_Read_cfg();
  CST816_AutoSleep(true);
   
  pinMode(CST816_INT_PIN, INPUT_PULLUP);
  attachInterrupt(CST816_INT_PIN, Touch_CST816_ISR, FALLING); 

  return true;
}

/* Reset controller */
uint8_t CST816_Touch_Reset(void)
{
  expanderPtr->digitalWrite(EXIO_PIN1, false);
  vTaskDelay(pdMS_TO_TICKS(100));
  expanderPtr->digitalWrite(EXIO_PIN1, true);
  vTaskDelay(pdMS_TO_TICKS(100));
  return true;
}

uint16_t CST816_Read_cfg(void) {

  uint8_t buf[3];
  I2C_Read(CST816_ADDR, CST816_REG_Version, buf, 1);
  printf("TouchPad_Version:0x%02x\r\n", buf[0]);
  I2C_Read(CST816_ADDR, CST816_REG_ChipID, buf, 3);
  printf("ChipID:0x%02x   ProjID:0x%02x   FwVersion:0x%02x \r\n", buf[0], buf[1], buf[2]);

  return true;
}

/*!
    @brief  Fall asleep automatically
*/
void CST816_AutoSleep(bool Sleep_State) {
  CST816_Touch_Reset();
  uint8_t Sleep_State_Set = (uint8_t)(!Sleep_State);
  Sleep_State_Set = 10;
  I2C_Write(CST816_ADDR, CST816_REG_DisAutoSleep, &Sleep_State_Set, 1);
}

// reads sensor and touches
// updates Touch Points
uint8_t Touch_Read_Data(void) {
  uint8_t buf[6]; // Buffer to hold the touch data
  uint8_t touchpad_cnt = 0;

  // Read the data from the touch controller
  if (!I2C_Read(CST816_ADDR, CST816_REG_GestureID, buf, 6)) {
    Serial.println("Failed to read touch data");
    for (int i = 0; i < 6; i++) {
    Serial.printf("%02X ", buf[i]); // Print each byte in hexadecimal format
  }
    return 0; // Indicate failure to read
  }

  // Print the raw data for debugging
  Serial.print("Raw Touch Data: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X ", buf[i]); // Print each byte in hexadecimal format
  }
  Serial.println();

  // Interpret the data (if it’s valid)
  if (buf[0] != 0x00) {
    touch_data.gesture = (GESTURE)buf[0]; // Store the gesture
  }
  if (buf[1] != 0x00) {
    noInterrupts(); 
    touch_data.points = (uint8_t)buf[1]; // Number of touch points
    if (touch_data.points > CST816_LCD_TOUCH_MAX_POINTS)
        touch_data.points = CST816_LCD_TOUCH_MAX_POINTS;

    // Extract X and Y coordinates from the data
    touch_data.x = ((buf[2] & 0x0F) << 8) + buf[3];
    touch_data.y = ((buf[4] & 0x0F) << 8) + buf[5];
    interrupts();

    // Check if the touch is valid and within reasonable bounds
    if (touch_data.x >= 0 && touch_data.x <= 4095 && touch_data.y >= 0 && touch_data.y <= 4095) {
      // Scale the values to fit the 360x360 display
      int scaled_x = map(touch_data.x, 0, 4095, 0, 360);
      int scaled_y = map(touch_data.y, 0, 4095, 0, 360);

      // Apply a threshold to filter out minor touches (example: ignore very small movements)
      if (scaled_x > 20 && scaled_y > 20) {
        Serial.printf("Touch : X=%u Y=%u points=%d\r\n", scaled_x, scaled_y, touch_data.points);
      } else {
        Serial.println("Minor or spurious touch detected, ignoring.");
      }
    } else {
      Serial.println("Touch coordinates out of bounds, ignoring.");
    }
  }

  return 1; // Indicate successful read
}

void example_touchpad_read(void){
  Touch_Read_Data();
  if (touch_data.gesture != NONE || touch_data.points != 0x00) {
      printf("Touch : X=%u Y=%u points=%d\r\n",  touch_data.x , touch_data.y, touch_data.points);
  }
}

void Touch_Loop(void) {
    if (Touch_interrupts) {
        Serial.println("Touch interrupt detected");
        Touch_interrupts = false;

        // Only proceed if touch data is valid
        if (Touch_Read_Data()) {
            if (touch_data.points > 0) {
                Serial.printf("Touch valid: X=%u Y=%u points=%d\r\n", touch_data.x, touch_data.y, touch_data.points);
            } else {
                Serial.println("No valid touch points detected.");
            }
        }
    }
}

/*!
    @brief  get the gesture event name
*/
String Touch_GestureName(void) {
  switch (touch_data.gesture) {
    case NONE:
      return "NONE";
    case SWIPE_DOWN:
      return "SWIPE DOWN";
    case SWIPE_UP:
      return "SWIPE UP";
    case SWIPE_LEFT:
      return "SWIPE LEFT";
    case SWIPE_RIGHT:
      return "SWIPE RIGHT";
    case SINGLE_CLICK:
      return "SINGLE CLICK";
    case DOUBLE_CLICK:
      return "DOUBLE CLICK";
    case LONG_PRESS:
      return "LONG PRESS";
    default:
      return "UNKNOWN";
  }
}

bool I2C_Read(uint16_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr); 
  int endTransStatus = Wire.endTransmission(true);
  if (endTransStatus != 0) {
    printf("I2C Read Error: Transmission failed with error code %d\n", endTransStatus);
    return false;
  }
  Wire.requestFrom(Driver_addr, Length);
  for (int i = 0; i < Length; i++) {
    Reg_data[i] = Wire.read();
  }
  return true;
}

bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  Wire.beginTransmission(Driver_addr);
  Wire.write(Reg_addr);       
  for (int i = 0; i < Length; i++) {
    Wire.write(Reg_data[i]);
  }
  int endTransStatus = Wire.endTransmission(true);
  if (endTransStatus != 0) {
    printf("I2C Write Error: Transmission failed with error code %d\n", endTransStatus);
    return false;
  }
  return true;
}