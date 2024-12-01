/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */

//////////////////// INCLUDES ////////////////////////////

#include <lvgl.h>
#include "ui.h"
#include "clock.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "BLEClient_NimBLE.h"
#include <esp_sleep.h>
#include <time.h>
//#include <SPIFFS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui_roundmsgbox.h"
#include "ui_events.h"
#include <Arduino_GFX_Library.h>

#include <semphr.h>
//#include "ANCS_NimBLE.h" 
#include "ANCS.h"
#include "Wire.h"  // For I2C communication with the I/O expander
#include <Adafruit_XCA9554.h>
#include "CST816S.h"
#include "PowerManager.h"
#include "FS.h"
//#include <LITTLEFS.h>
#include <LittleFS.h>
//#include "filesys.h"
#include "WeatherManager.h"
#include "SettingsManager.h"
#include "ui_Settings.h"


//////////////////// DEFINITIONS ///////////////////////////////

#define I2C_SCL 10
#define I2C_SDA 11
#define TCA9554_ADDRESS 0x20  // I2C address for the IO expander
//#define CST816_ADDRESS 0x3F  // I2C address for the touch panel

#define FORMAT_LITTLEFS_IF_FAILED true


#define PWR_KEY_PIN 6
#define BAT_Control_Pin 7

#define TOUCH_PIN 4 // Change based on your setup
#define TOUCH_RST 0
#define TOUCH_SCL 3
#define TOUCH_SDA 1

#define SLEEP_DURATION 25000 // 25 seconds in milliseconds

// Define the pin connections for the QSPI interface
#define LCD_CS 21  // Chip select
#define LCD_SCK 40  // Serial clock
#define LCD_D0 46  // Data line 0
#define LCD_D1 45  // Data line 1
#define LCD_D2 42  // Data line 2
#define LCD_D3 41  // Data line 3
#define LCD_RST_PIN 1 // EXIO2 pin (mapped to TCA9554 I/O expander)
#define LCD_BL 5  // Backlight control pin

#define LCD_RST 14     // On the expander
#define LCD_WIDTH 360
#define LCD_HEIGHT 360
#define LVGL_BUF_LEN (LCD_WIDTH * LCD_HEIGHT / 10)

//#define DIRECT_MODE // Uncomment to enable full frame buffer


#define DRAW_BUF_SIZE (360 * 360 / 10 * (LV_COLOR_DEPTH / 8))
#define FULL_BUF_SIZE (360 * 360 * (LV_COLOR_DEPTH / 8))
//#define DRAW_BUF_SIZE (LCD_WIDTH * LCD_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))


////////////////////// VARIABLES //////////////////////////////

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

uint32_t full_buf[FULL_BUF_SIZE / 4];  // Divide by 4 for 32-bit alignment

//UIRoundMsgBox notificationUI;  // Create an instance of UIRoundMsgBox
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 120000;  // 2 minutes in milliseconds
extern void processBLE();
extern void SampleSecureServer(void);
static unsigned long lastTickTime = 0;
static unsigned long last_weather_update = 0;
static unsigned long last_power_update = 0;
static unsigned long last_battery_update = 0;


SemaphoreHandle_t xGuiSemaphore;

// Create an instance of the Arduino_ESP32QSPI class
Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
// Initialize the GFX instance for the ST77916 display
Arduino_GFX *gfx = new Arduino_ST77916(bus, LCD_RST_PIN, 0 /* rotation */, true /* IPS */);


/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

/*******************************************************************************
 * Please config the touch panel in touch.h
 ******************************************************************************/

extern void ui_init();



extern void clock_init();
unsigned long lastInteractionTime = 0;
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;

uint32_t *disp_draw_buf;

static lv_color_t buf1[ LVGL_BUF_LEN ];
static lv_color_t buf2[ LVGL_BUF_LEN];
lv_display_t *disp;

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif


Adafruit_XCA9554 expander;
PowerManager powerManager(PWR_KEY_PIN, BAT_Control_Pin, &expander);

CST816S touch(1, 3, 0, 4);


/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
#ifdef DIRECT_MODE
  // In DIRECT_MODE, we don’t need to explicitly draw on the framebuffer.
  // The buffer in LVGL is already mapped to the framebuffer in setup.
  // However, make sure to mark the flush as ready.
#else
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#endif // #ifdef DIRECT_MODE

  /*Call it to tell LVGL you are ready*/
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  //Serial.print("Trying to read touch");
  int last_x = 0;
  int last_y = 0;
  if(touch.available()) {
     //Serial.printf("TOuch x reads %d", touch.data.x);
    last_x = touch.data.x;
    last_y = touch.data.y;
    data->state = LV_INDEV_STATE_PRESSED;
     lastInteractionTime = millis(); // Update last interaction time
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  data->point.x = last_x;
  data->point.y = last_y;
}

void adc_init()
{
  // Configure ADC
    analogReadResolution(12);  // Set resolution to 12 bits
    analogSetAttenuation(ADC_6db);  // Set attenuation, e.g., no attenuation
  
   //Serial.print("ADC init passed");
}

float read_battery_voltage()
{
    int adc_reading = analogReadMilliVolts(8); // ADC reading in millivolts
    float adc_voltage = adc_reading / 1000.0f; // Convert millivolts to volts
    float battery_voltage = adc_voltage * 3.0f; // Adjust for voltage divider
    Serial.println("Battery voltage is:");
    Serial.println(battery_voltage);
    Serial.println("adc reading is:");
    Serial.println(adc_reading);
    return battery_voltage;
}

float get_battery_percentage(float voltage)
{
    float battery_voltage = voltage;
    float battery_percentage = ((battery_voltage - 3.0f) / (4.1f - 3.0f)) * 100.0f;
    battery_percentage = constrain(battery_percentage, 0.0f, 100.0f); // Ensure it's between 0% and 100%
    Serial.print("Battery percentage is: ");
    Serial.println(battery_percentage);
    return battery_percentage;
}

void update_battery_arc()
{
    
  //Serial.print("Updating battery arc.");
    float voltage = read_battery_voltage();
    float battery_percentage = get_battery_percentage(voltage);
    //Serial.printf("Battery voltage reads: %u%%\n", battery_percentage);

    if (voltage > 4.1)
    {
        // Probably charging
       // Serial.print("Voltage above 4.1v, likely charged and plugged in.");
        lv_obj_set_style_arc_color(ui_BatteryArc, lv_color_hex(0x008deb), LV_PART_INDICATOR); // Blue color
        lv_arc_set_angles(ui_BatteryArc, 0, 359); // Full circle
        lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0x008deb), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_CHARGE);
        return;
    }
    if (voltage < 2.0)
    {
    // No battery detected
    //Serial.print("No battery detected");

    lv_obj_set_style_arc_color(ui_BatteryArc, lv_color_hex(0xff0000), LV_PART_INDICATOR); // red color
    lv_arc_set_angles(ui_BatteryArc, 0, 359); // Full circle
    lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);

    return;
    }

    


    // Set arc angle
    uint16_t end_angle = (uint16_t)((battery_percentage * 360) / 100);
    lv_arc_set_angles(ui_BatteryArc, 0, end_angle);

    // Determine color based on battery percentage
    lv_color_t arc_color;
//OLD COLOR SCHEME
 /*    if (battery_percentage > 30)
    {
        // Green to Yellow
        uint8_t green = 255;
        uint8_t red = (uint8_t)(255 * (100 - battery_percentage) / 70);
        arc_color = lv_color_make(red, green, 0);
    }
    else if (battery_percentage > 20)
    {
        // Yellow to Orange
        uint8_t green = (uint8_t)(255 * (battery_percentage - 20) / 10);
        arc_color = lv_color_make(255, green, 0);
    }
    else
    {
        // Orange to Red
        uint8_t red = 255;
        uint8_t green = (uint8_t)(255 * battery_percentage / 20);
        arc_color = lv_color_make(red, green, 0);
    } */
    //char voltage_str[16];
    //snprintf(voltage_str, sizeof(voltage_str), "Voltage: %.3f V", voltage);
    //lv_label_set_text(ui_BatteryLabel, voltage_str);

    // OLD COLOR SCHEME ENDS

     if (battery_percentage > 70)
    {
        // From #00ffbb (100%) to #5D64EB (70%)
        uint8_t factor = (uint8_t)((100 - battery_percentage) * 255 / 30); // 0 - 255 range
        arc_color = lv_color_make(
            0x00 + (0x5D - 0x00) * factor / 255,
            0xFF + (0x64 - 0xFF) * factor / 255,
            0xBB + (0xEB - 0xBB) * factor / 255
        );
    }
    else if (battery_percentage > 30)
    {
        // From #5D64EB (70%) to #A343B8 (30%)
        uint8_t factor = (uint8_t)((70 - battery_percentage) * 255 / 40); // 0 - 255 range
        arc_color = lv_color_make(
            0x5D + (0xA3 - 0x5D) * factor / 255,
            0x64 + (0x43 - 0x64) * factor / 255,
            0xEB + (0xB8 - 0xEB) * factor / 255
        );
    }
    else
    {
        // From #A343B8 (30%) to #C23D53 (0%)
        uint8_t factor = (uint8_t)((30 - battery_percentage) * 255 / 30); // 0 - 255 range
        arc_color = lv_color_make(
            0xA3 + (0xC2 - 0xA3) * factor / 255,
            0x43 + (0x3D - 0x43) * factor / 255,
            0xB8 + (0x53 - 0xB8) * factor / 255
        );
    }


    if (battery_percentage >= 90)
      {
        lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_FULL);
        lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0x03fc4e), LV_PART_MAIN | LV_STATE_DEFAULT);

      }
      else if (battery_percentage >= 75)
        {
          lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_3);
          lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0x03fc4e), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      else if (battery_percentage >= 50)
        {
          lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_2);
          lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0x77fc03), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      else if (battery_percentage >= 20)
        {
          lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_1);
          lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0xfcf803), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
      else    
      {
        lv_label_set_text(ui_BatteryLabel, LV_SYMBOL_BATTERY_EMPTY);
        lv_obj_set_style_text_color(ui_BatteryLabel, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
  


    lv_obj_set_style_arc_color(ui_BatteryArc, arc_color, LV_PART_INDICATOR);
}

// Function to put the ESP32 into deep sleep
/* void goToSleep() {
    Serial.println("Going to sleep...");

     touch.sleep(); // Put the touch panel into standby mode

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);

  
  esp_deep_sleep_start();
}
 */

uint32_t millis_cb(void)
{
  return millis();
}


// MAIN SETUP FUNCTION ------------------------------------------------

void setup()
{
  Serial.begin(115200);
 // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX LVGL_Arduino example v8");

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");
 // Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin(I2C_SDA, I2C_SCL);
   // Initialize the I2C-based I/O expander
  if (!expander.begin(0x20)) {  // Use the correct I2C address
    Serial.println("Failed to initialize TCA9554 I/O expander");
    while (1);  // Stop if the expander is not found
  }
      Serial.println("Initialized TCA9554 I/O expander");
   powerManager.init();
 

 // Configure TCA9554 pins
 // expander.pinMode(LCD_RST_PIN, OUTPUT);  // Configure as output for LCD reset
    

  // Reset the LCD
  //expander.digitalWrite(LCD_RST_PIN, LOW);  // Hold in reset
 // delay(50);
 // expander.digitalWrite(LCD_RST_PIN, HIGH);  // Release reset
//  delay(120);

  // Turn on the backlight
  //pinMode(LCD_BL, OUTPUT);
  //digitalWrite(LCD_BL, HIGH);
  
 
    // Initialize the PowerManager
    powerManager.initBacklight();
    
    // Set initial brightness to 80%
    powerManager.setBacklightBrightness(80);


   // Initialize the semaphore before using LVGL
    xGuiSemaphore = xSemaphoreCreateMutex();

   // if (!SPIFFS.begin(true)) {
     //   Serial.println("SPIFFS Mount Failed");
    //    return;
    //}
      
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
  }
  else{
    Serial.println("Little FS Mounted Successfully");
  }
  
  //Loading the settings now
  initializeSettingsData();
 // powerManager.setBacklightBrightness(currentSettings.brightness_level);

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BLACK);

Serial.println("gfx successful");


  // Init touch device
  touch.begin(FALLING, &expander, Wire1); 

    //pinMode(CST816_INT_PIN, INPUT_PULLUP);
   // Serial.printf("Interrupt pin set to: %d\n", CST816_INT_PIN);

  
 

  Serial.println("Touch screen initialization complete.");
 // Confirm interrupt pin setup



  lv_init();
  //lv_disp_draw_buf_init( &draw_buf, buf1, buf2, LVGL_BUF_LEN);
lv_tick_set_cb(millis_cb);
//lv_lodepng_init();

 
screenWidth = gfx->width();
screenHeight = gfx->height();


/*     #ifdef DIRECT_MODE
        //disp_draw_buf = (lv_color_t *)heap_caps_malloc(FULL_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        disp_draw_buf = (uint32_t *)heap_caps_malloc(FULL_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM);
    #else
        disp_draw_buf = (uint32_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    #endif

    if (!disp_draw_buf) {
        Serial.println("LVGL disp_draw_buf allocation failed!");
       // return;
    }  */


    disp = lv_display_create(screenWidth, screenHeight);
    lv_display_set_flush_cb(disp, my_disp_flush);
  
    #ifdef DIRECT_MODE
        lv_display_set_buffers(disp, full_buf, NULL, FULL_BUF_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
    #else  
        lv_display_set_buffers(disp, draw_buf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    #endif

 

      /*Initialize the (dummy) input device driver*/
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);


    adc_init(); // Initialize ADC
    
        // Check if the time is still close to the Unix epoch (1970-01-01)
    time_t now = time(NULL);
    
    // Define a threshold (e.g., 60 seconds after the epoch)
    const time_t epochThreshold = 60;

    /* if (now < epochThreshold) {
        //Serial.println("System time is not set. Synchronizing with BLE server...");

        initBLEClient(); // Initialize BLE connection to set time

        if (updateTimeFromBLEServer()) {
           // Serial.println("Time successfully set from BLE server.");
        } else {
           // Serial.println("Failed to set time from BLE server.");
        }
    } else {
       // Serial.println("System time is already set. Skipping BLE time synchronization.");
    }  */

    clock_init();
    Serial.println("clock_init success");
    ui_init();
     //Serial.println("ui_init success");


      // Initial update
    update_battery_arc();
    //Serial.println("Update_battery_arc success");
    lastInteractionTime = millis(); // Initialize the last interaction time
    
  
   // Initialize ANCS and set the notification callback
    
    //SampleSecureServer();

    WeatherInit();
        if (!loadWeatherDataFromFile("/weather.json", currentWeatherData)) {
        Serial.println("No valid weather data found, fetching new data...");
        updateWeatherData();
    }


 
}

void loop()
{
       //if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
 /*   if (xSemaphoreTake(xGuiSemaphore, pdMS_TO_TICKS(20)) == pdTRUE) {
        //lv_task_handler(); // Call lvgl task handler frequently
        lv_timer_handler();
        xSemaphoreGive(xGuiSemaphore);
    } */
     esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    lv_timer_handler();

   //Touch_Loop();  // Regularly poll for touch events

    // Update LVGL tick increment based on actual time elapsed
  unsigned long currentTime = millis();

   // if (currentTime - lastTickTime >= 5) {  // Increment every 5ms
   //     lv_tick_inc(currentTime - lastTickTime);
   //     lastTickTime = currentTime;
   // }
    
    //lv_task_handler();
    
    //lv_timer_handler();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        // This means the device woke from GPIO pin 4, likely from light sleep.
        Serial.println("Woke up from light sleep...");
       powerManager.initBacklight(); // Re-enable the backlight
    }

  #ifdef DIRECT_MODE
#if defined(CANVAS) || defined(RGB_PANEL)
  gfx->flush();
#else // !(defined(CANVAS) || defined(RGB_PANEL))
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, screenWidth, screenHeight);
#endif // !(defined(CANVAS) || defined(RGB_PANEL))
#else  // !DIRECT_MODE
#ifdef CANVAS
  gfx->flush();
#endif
#endif // !DIRECT_MODE



    // Inactivity check for sleep
    if (millis() - lastInteractionTime > SLEEP_DURATION) {
        powerManager.goToSleep();
    }

    // Battery and BLE process logic (same as before)

    if (currentTime - last_battery_update >= 60000) {
        update_battery_arc();
        last_battery_update = currentTime;
    }

  
    if (currentTime - last_power_update >= 1000){
          powerManager.update();
          last_power_update = currentTime;

    }


    if (currentTime - last_weather_update >= 600000) {
        updateWeatherData();
        last_weather_update = currentTime;
    }
    
    
    static unsigned long lastProcessTime = 0;
    if (currentTime - lastProcessTime >= 30000) {
        // processBLE();  // Assuming this will be uncommented later
        lastProcessTime = currentTime;
    }

    if (doScreenBrightnessUpdate)
    {
      powerManager.setBacklightBrightness(currentSettings.brightness_level);
      doScreenBrightnessUpdate = false;
    }

    lv_tick_inc(5);

    delay(5);  // Small delay to avoid overwhelming the loop

 

    
      
}
