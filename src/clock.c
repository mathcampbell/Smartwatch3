#include "clock.h"
#include "ui.h"
//#include "ui_ClockScreen.c" // Assuming ui.h contains declarations for ui_hourhandimg, ui_minutehandimg, and ui_secondhandimg
#include <time.h>

/* // Initialize the clock functionality
void clock_init(void) {
    // Start the clock timer
    start_clock_timer();
}

// Function to update the clock hands based on the current time
void clock_update(lv_timer_t *timer) {
    // Get the current time
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

 // Calculate angles for the hour, minute, and second hands
    int hour_angle = (current_time->tm_hour % 12) * 300 + (current_time->tm_min * 5);  // 360° = 12 * 30° = 300° in LVGL units, adjusted for minutes
    int minute_angle = current_time->tm_min * 60;  // 360° = 60 * 6° = 360° in LVGL units
    int second_angle = current_time->tm_sec * 60;  // 360° = 60 * 6° = 360° in LVGL units

    // Subtract 900 to adjust for the initial image position
    hour_angle -= 900;
    minute_angle -= 900;
    second_angle -= 900;

    // Set the angles to the respective LVGL image objects
    lv_img_set_angle(hour_hand_img, hour_angle);
    lv_img_set_angle(minute_hand_img, minute_angle);
    lv_img_set_angle(second_hand_img, second_angle);
}

// Function to start the clock timer
void start_clock_timer(void) {
    // Create a timer that updates the clock every second
    lv_timer_create(clock_update, 1000, NULL); // 1000 ms = 1 second
}


 */

int hour_value = 0;
int minute_value = 0;
int second_value = 0;


void clock_init(void) {
    // Create a timer that updates the clock every second
    lv_timer_create(clock_update, 1000, NULL); // 1000 ms = 1 second
}

void clock_update(lv_timer_t * timer) {
    LV_UNUSED(timer);

    // Get the current time
    time_t now = time(NULL);
    struct tm * current_time = localtime(&now);

    // Update global variables
    hour_value = current_time->tm_hour % 12; // 0 to 11
    minute_value = current_time->tm_min;     // 0 to 59
    second_value = current_time->tm_sec;     // 0 to 59

    // Notify screens to update
    update_clock_screen();
    update_main_screen();
}