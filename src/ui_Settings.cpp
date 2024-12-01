// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.1
// LVGL version: 8.3.11
// Project name: SmartWatch

#include "ui_Settings.h"
#include "SettingsManager.h"

lv_obj_t * arc_segments[NUM_SEGMENTS];
const char * segment_labels[NUM_SEGMENTS] = {"Wi-Fi", "General", "Home", "Bluetooth", "Weather", "Sound"};
// Replace these with your actual icons
// lv_img_dsc_t * segment_icons[NUM_SEGMENTS] = {&wifi_icon, &brightness_icon, &sleep_icon, &home_icon};
//const char * segment_symbols[NUM_SEGMENTS] = {LV_SYMBOL_WIFI, LV_SYMBOL_SETTINGS, LV_SYMBOL_HOME, LV_SYMBOL_BLUETOOTH, LV_SYMBOL_REFRESH, LV_SYMBOL_AUDIO};


lv_obj_t * content_area;
lv_obj_t * ui_SettingsRadialMenu;
lv_obj_t * content_label;
int segment_index;
bool doScreenBrightnessUpdate;


void create_content_area(void) {
    content_area = lv_obj_create(ui_Settings);
    lv_obj_set_size(content_area, 240, 240); // Adjust size to avoid overlapping with symbols
    lv_obj_center(content_area);
    lv_obj_add_flag(content_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(content_area, lv_color_white(), 0);
    lv_obj_set_style_radius(content_area, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_set_style_pad_all(content_area, 10, 0); // Add padding if needed

    
}

void show_wifi_settings(void) {
    lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Wi-Fi Settings");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Add Wi-Fi controls, e.g., list of networks, switches, etc.
    // For example, create a list of available networks
}

void show_brightness_settings(void) {
    lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Brightness");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Create brightness slider
    lv_obj_t * slider = lv_slider_create(content_area);
    lv_obj_set_width(slider, lv_pct(80));
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, currentSettings.brightness_level, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void show_sleep_settings(void) {
    //lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Sleep Timer");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 20);

    // Create sleep timer slider
    lv_obj_t * slider = lv_slider_create(content_area);
    lv_obj_set_width(slider, lv_pct(80));
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 30);
    lv_slider_set_range(slider, 5, 60);
    lv_slider_set_value(slider, currentSettings.sleep_duration, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, sleep_timer_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void create_radial_menu(void)
{
// Create the arc
    ui_SettingsRadialMenu = lv_arc_create(ui_Settings);
    lv_obj_set_size(ui_SettingsRadialMenu, 350, 350);
    lv_obj_center(ui_SettingsRadialMenu);
    lv_arc_set_bg_angles(ui_SettingsRadialMenu, 0, 360); // Full circle
    lv_arc_set_range(ui_SettingsRadialMenu, 0, 600);
   // lv_arc_set_value(ui_SettingsRadialMenu, 0);
    lv_obj_set_style_arc_width(ui_SettingsRadialMenu, 50, LV_PART_MAIN); // Adjust thickness
    lv_obj_set_style_arc_width(ui_SettingsRadialMenu, 50, LV_PART_INDICATOR); // Adjust thickness
    lv_obj_set_style_arc_color(ui_SettingsRadialMenu, lv_color_hex(0x2c2836), LV_PART_MAIN);
    
    //lv_obj_remove_style(ui_SettingsRadialMenu, NULL, LV_PART_KNOB); // Remove knob
    // Make the knob invisible but functional
    lv_obj_set_style_bg_opa(ui_SettingsRadialMenu, LV_OPA_TRANSP, LV_PART_KNOB);

    lv_obj_set_style_arc_rounded(ui_SettingsRadialMenu, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_arc_set_rotation(ui_SettingsRadialMenu, -90);

    // Disable adjustability
    //lv_obj_clear_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_SCROLLABLE);
   // lv_obj_add_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_CLICKABLE);
   // lv_obj_add_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_SCROLLABLE);
   // lv_obj_add_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_set_align(ui_SettingsRadialMenu, LV_ALIGN_CENTER);

   // lv_arc_set_change_rate(ui_SettingsRadialMenu, 360);
       lv_obj_clear_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(ui_SettingsRadialMenu, LV_OBJ_FLAG_SCROLLABLE);
    // Enable touch events
   // lv_obj_add_event_cb(ui_SettingsRadialMenu, arc_event_cb, LV_EVENT_ALL, NULL);
    
/*     // Setting initial position
    int arcvalue = lv_arc_get_value(ui_SettingsRadialMenu);
    Serial.printf("Arc value chaged, it's now: %d", arcvalue);
    int arc_max_value = 600; // As per your setup

    // Compute segment index
    int segment_index = (arcvalue * NUM_SEGMENTS) / arc_max_value;
   // int segment_index = (360 * NUM_SEGMENTS) / 360;
    if(segment_index >= NUM_SEGMENTS) segment_index = NUM_SEGMENTS - 1;

    // Calculate indicator angles
    int total_angle = 360;
    int section_angle = total_angle / NUM_SEGMENTS;
    int indicator_start_angle = segment_index * section_angle;
    int indicator_end_angle = (segment_index + 1) * section_angle;

    // Set the start and end angles of the indicator
    lv_arc_set_start_angle(ui_SettingsRadialMenu, indicator_start_angle);
    
    lv_arc_set_end_angle(ui_SettingsRadialMenu, indicator_end_angle); */



    // Initialize the indicator to the first segment
    lv_arc_set_start_angle(ui_SettingsRadialMenu, 0);
    lv_arc_set_end_angle(ui_SettingsRadialMenu, 360 / NUM_SEGMENTS);

}


void ui_Settings_screen_init(void)
{
    Serial.println("ui_Settings_screen_init was called");
    ui_Settings = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Settings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_Settings, lv_color_hex(0x100820), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Settings, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(ui_Settings, 360, 360);
    //lv_obj_set_style_radius(ui_Settings, LV_RADIUS_CIRCLE, 0);
    //lv_obj_set_style_clip_corner(ui_Settings, true, 0);

  /*   ui_MainArcSettingsMenu = lv_arc_create(ui_Settings);
    lv_obj_set_width(ui_MainArcSettingsMenu, 110);
    lv_obj_set_height(ui_MainArcSettingsMenu, 110);
    lv_obj_set_align(ui_MainArcSettingsMenu, LV_ALIGN_CENTER);
    lv_arc_set_range(ui_MainArcSettingsMenu, 0, 500);
    lv_arc_set_value(ui_MainArcSettingsMenu, 100);
    lv_obj_set_style_arc_color(ui_MainArcSettingsMenu, lv_color_hex(0x141721), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_MainArcSettingsMenu, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_MainArcSettingsMenu, false, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_MainArcSettingsMenu, lv_color_hex(0x79CBFC), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_MainArcSettingsMenu, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_MainArcSettingsMenu, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_MainArcSettingsMenu, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_MainArcSettingsMenu, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
        lv_obj_add_flag(ui_MainArcSettingsMenu, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(ui_MainArcSettingsMenu, LV_OBJ_FLAG_CLICKABLE); */


   // lv_obj_add_event_cb(ui_MainArcSettingsMenu, ui_event_MainArcSettingsMenu, LV_EVENT_ALL, NULL);

    create_radial_menu();
    create_content_area();
    //add_segment_labels();
     add_segment_buttons(); // Use this if you're creating buttons

    // Optionally, display default content
    //show_wifi_settings();
    
}

void add_segment_labels(void) {
    int angle_per_segment = 360 / NUM_SEGMENTS;
    const char * segment_symbols[NUM_SEGMENTS] = {LV_SYMBOL_WIFI, LV_SYMBOL_SETTINGS, LV_SYMBOL_HOME, LV_SYMBOL_BLUETOOTH, LV_SYMBOL_REFRESH, LV_SYMBOL_AUDIO};

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        int mid_angle = i * angle_per_segment + angle_per_segment / 2;
        int radius = 140; // Adjust radius to position the symbol within the segment
        mid_angle -= 90;

        // Convert angle to radians
        float rad = mid_angle * 3.14159265 / 180.0;

        int x = (int)(radius * cos(rad));
        int y = (int)(radius * sin(rad));

        // Create a label for the symbol
        lv_obj_t * symbol_label = lv_label_create(ui_Settings);
        lv_label_set_text(symbol_label, segment_symbols[i]);
        lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_24, 0); // Adjust font size as needed
        lv_obj_set_style_text_color(symbol_label, lv_color_white(), 0); // Set text color
        lv_obj_align(symbol_label, LV_ALIGN_CENTER, x, y);
    }
}

void add_segment_buttons(void) {
    int angle_per_segment = 360 / NUM_SEGMENTS;
    const char * segment_symbols[NUM_SEGMENTS] = {LV_SYMBOL_WIFI, LV_SYMBOL_SETTINGS, LV_SYMBOL_HOME, LV_SYMBOL_BLUETOOTH, LV_SYMBOL_REFRESH, LV_SYMBOL_AUDIO};

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        int mid_angle = i * angle_per_segment + angle_per_segment / 2;
        int radius = 150; // Adjust radius to position the button within the segment
        mid_angle -= 90;
        if (mid_angle <= 0)
        {
            mid_angle += 360;
        }

        // Convert angle to radians
        float rad = mid_angle * 3.14159265 / 180.0;

        int x = (int)(radius * cos(rad));
        int y = (int)(radius * sin(rad));

        // Create a button for the segment
        lv_obj_t * btn = lv_btn_create(ui_Settings);
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_size(btn, 40, 40); // Adjust size as needed
        lv_obj_align(btn, LV_ALIGN_CENTER, x, y);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0); // Make button background transparent
        lv_obj_set_style_border_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(btn, 0, 0);

        // Create a label for the symbol inside the button
        lv_obj_t * symbol_label = lv_label_create(btn);
        lv_label_set_text(symbol_label, segment_symbols[i]);
        lv_obj_align(symbol_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_24, 0); // Adjust font size as needed
        lv_obj_set_style_text_color(symbol_label, lv_color_white(), 0); // Set text color

        // Assign the segment index as user data
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        //lv_obj_set_style_pad_all(btn, 30, LV_STATE_PRESSED);
        lv_obj_set_ext_click_area(btn, 20);
        // Add event callback to the button
        lv_obj_add_event_cb(btn, segment_btn_event_cb, LV_EVENT_CLICKED, NULL);
    }
}

void segment_btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    int segment_index = (int)(intptr_t)lv_obj_get_user_data(btn);

    // Update the arc's indicator to highlight the selected segment
    int total_angle = 360;
    int section_angle = total_angle / NUM_SEGMENTS;
    
    int indicator_start_angle = segment_index * section_angle;
    int indicator_end_angle = (segment_index + 1) * section_angle;

    lv_arc_set_start_angle(ui_SettingsRadialMenu, indicator_start_angle);
    lv_arc_set_end_angle(ui_SettingsRadialMenu, indicator_end_angle);

    // Perform the action based on the segment index
    switch(segment_index) {
        case 0: // Wi-Fi
            show_wifi_settings();
            break;
        case 1: // General
            show_general_settings();
            break;
        case 2: // Home
            _ui_screen_change(&ui_MainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, ui_MainScreen_screen_init);
            break;
        case 3: // Bluetooth
            show_bluetooth_settings();
            break;
        case 4: // Weather
            show_weather();
            break;
        case 5: // Sound
            show_sound_settings();
            break;
        default:
            break;
    }
}

void arc_event_cb(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_PRESSING) {
        settingsMenu_valuechange(e);
    }
    if(event_code == LV_EVENT_RELEASED) {
        settingsMenu_select(e);
    }
}

void settingsMenu_valuechange(lv_event_t * e)
{
    lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
   /*  // Get the current value of the arc
    int arcvalue = lv_arc_get_value(arc);
    Serial.printf("Arc value chaged, it's now: %d", arcvalue);
    int arc_max_value = 600; // As per your setup

    // Compute segment index
    int segment_index = (arcvalue * NUM_SEGMENTS) / arc_max_value;
   // int segment_index = (360 * NUM_SEGMENTS) / 360;
    if(segment_index >= NUM_SEGMENTS) segment_index = NUM_SEGMENTS - 1;

    // Calculate indicator angles
    int total_angle = 360;
    int section_angle = total_angle / NUM_SEGMENTS;
    int indicator_start_angle = segment_index * section_angle;
    int indicator_end_angle = (segment_index + 1) * section_angle;

    // Set the start and end angles of the indicator
    lv_arc_set_start_angle(arc, indicator_start_angle);
    
    lv_arc_set_end_angle(arc, indicator_end_angle);
    Serial.printf("Setting Angles: %d", indicator_start_angle);
    // Optionally, update a label or visual indicator
    //const char * label_text = segment_labels[segment_index];
    //lv_label_set_text(ui_SettingsLabel, label_text); */

      // Get the touch point coordinates
// Get the touch point coordinates
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t touch_point;
    lv_indev_get_point(indev, &touch_point);

    // Get the arc's center coordinates
    lv_area_t arc_coords;
    lv_obj_get_coords(arc, &arc_coords);
    lv_coord_t arc_center_x = arc_coords.x1 + lv_area_get_width(&arc_coords) / 2;
    lv_coord_t arc_center_y = arc_coords.y1 + lv_area_get_height(&arc_coords) / 2;

    // Calculate the difference in x and y from the center
    int dx = touch_point.x - arc_center_x;
    int dy = arc_center_y - touch_point.y ;

    // Calculate the angle in degrees (0 to 360)
    float angle = atan2f(dx, dy) * (180.0f / 3.14159265f);
    if (angle < 0) {
        angle += 360.0f;
    }



    // Adjust the angle based on the arc's rotation
   // angle -= 90.0f; // Since we rotated the arc by -90 degrees
    if (angle < 0) {
        angle += 360.0f;
    }

    // Calculate the segment index

    float segment_angle = 360.0f / NUM_SEGMENTS;
     segment_index = (int)(angle / segment_angle);
    if(segment_index >= NUM_SEGMENTS) segment_index = 0;

    // Update the arc's indicator to highlight the selected segment
    int indicator_start_angle = segment_index * segment_angle;
    int indicator_end_angle = (segment_index + 1) * segment_angle;

    lv_arc_set_start_angle(arc, indicator_start_angle);
    lv_arc_set_end_angle(arc, indicator_end_angle);

    // Debug output
    Serial.printf("dx: %d, dy: %d, angle: %.2f, segment_index: %d\n", dx, dy, angle, segment_index);

}

void settingsMenu_select(lv_event_t * e)
{
    lv_obj_t * arc = (lv_obj_t *)lv_event_get_target(e);
    
    
   /*  // Get the current value of the arc
    int arcvalue = lv_arc_get_value(arc);

    int arc_max_value = 600;


    int segment_index = (arcvalue * NUM_SEGMENTS) / arc_max_value;
   //  int segment_index = (360 * NUM_SEGMENTS) / 360;
    if(segment_index >= NUM_SEGMENTS) segment_index = NUM_SEGMENTS - 1;
    Serial.println("Setting Segment");
    Serial.println(segment_index);
        int total_angle = 360;

    int section_angle = total_angle / NUM_SEGMENTS;
    int indicator_start_angle = segment_index * section_angle;
    int indicator_end_angle = (segment_index + 1) * section_angle;

    // Set the start and end angles of the indicator
    lv_arc_set_start_angle(arc, indicator_start_angle);
    
    lv_arc_set_end_angle(arc, indicator_end_angle);

    // Perform the action based on the segment index
    switch(segment_index) {
        case 0: // Wi-Fi
            show_wifi_settings();
            break;
        case 1: // General
            show_general_settings();
            break;
        case 2: // Home
            _ui_screen_change(&ui_MainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, ui_MainScreen_screen_init);
            break;
        case 3: // Bluetooth
            show_bluetooth_settings();
            break;
        case 4: // Weather
            show_weather();
            break;
        case 5: // Sound
            show_sound_settings();
            break;
        default:
            break;
    } */


/*         // Get the touch point coordinates
    lv_indev_t * indev = lv_indev_get_act();
    lv_point_t touch_point;
    lv_indev_get_point(indev, &touch_point);

    // Get the arc's center coordinates
    lv_area_t arc_coords;
    lv_obj_get_coords(arc, &arc_coords);
    lv_coord_t arc_center_x = arc_coords.x1 + lv_area_get_width(&arc_coords) / 2;
    lv_coord_t arc_center_y = arc_coords.y1 + lv_area_get_height(&arc_coords) / 2;

    // Calculate the difference in x and y from the center
    int dx = touch_point.x - arc_center_x;
    int dy = arc_center_y - touch_point.y;

    // Calculate the angle in degrees (0 to 360)
    float angle = atan2f(dx, dy) * (180.0f / 3.14159265f);
    if (angle < 0) {
        angle += 360.0f;
    }



    // Adjust the angle based on the arc's rotation
    //angle -= 90.0f; // Since we rotated the arc by -90 degrees
    if (angle < 0) {
        angle += 360.0f;
    }

    // Calculate the segment index
 
    float segment_angle = 360.0f / NUM_SEGMENTS;
     segment_index = (int)(angle / segment_angle);
    if(segment_index >= NUM_SEGMENTS) segment_index = 0; */

    // Debug output
    Serial.printf("Segment Selected: %d\n", segment_index);

    // Perform the action based on the segment index
    switch(segment_index) {
        case 0: // Wi-Fi
            show_wifi_settings();
            break;
        case 1: // General
            show_general_settings();
            break;
        case 2: // Home
         lv_arc_set_start_angle(ui_MainArcMenu, 120);
         lv_arc_set_end_angle(ui_MainArcMenu, 180);
         lv_label_set_text(ui_screenselectlabel, "Main");
            _ui_screen_change(&ui_MainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, ui_MainScreen_screen_init);
            break;
        case 3: // Bluetooth
            show_bluetooth_settings();
            break;
        case 4: // Weather
            show_weather();
            break;
        case 5: // Sound
            show_sound_settings();
            break;
        default:
            break;
    }
}
 



// **Wi-Fi Switch Event Callback**
 void wifi_switch_event_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * page = (lv_obj_t *)lv_event_get_user_data(e); // Get the page from user data
    lv_obj_t * list_networks = lv_obj_get_child(page, NULL); // Assume it's the first child

    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        wifi_enable();
        scan_and_display_wifi_networks(list_networks);
    } else {
        wifi_disable();
        lv_obj_clean(list_networks); // Clear the list
    }
}

void brightness_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    int16_t value = lv_slider_get_value(slider);
    currentSettings.brightness_level = value;
    doScreenBrightnessUpdate = true;
    
    // Adjust the display brightness accordingly
    // Implement set_display_brightness(value);
}

void sleep_timer_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    int16_t value = lv_slider_get_value(slider);
    // Adjust the sleep timer accordingly
    // Implement set_sleep_timer(value);
}

void show_general_settings(void) {
    lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "General Settings");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Add general settings controls here
    show_brightness_settings();
    show_sleep_settings();
}

void show_bluetooth_settings(void) {
    lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Bluetooth Settings");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Add Bluetooth controls here
}

void show_weather(void) {
   lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Weather");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Display weather information here
}

void show_sound_settings(void) {
   lv_obj_clean(content_area); // Clear previous content

    content_label = lv_label_create(content_area);
    lv_label_set_text(content_label, "Sound Settings");
    lv_obj_align(content_label, LV_ALIGN_TOP_MID, 0, 10);

    // Add sound settings controls here
}


// Wi-Fi Enable Function
void wifi_enable() {
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.disconnect(); // Ensure we're starting fresh
}

// Wi-Fi Disable Function
void wifi_disable() {
    WiFi.disconnect(true);
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);
}

void scan_and_display_wifi_networks(lv_obj_t * parent_list) {
    int networkCount = WiFi.scanNetworks();
    if (networkCount == 0) {
        printf("No Wi-Fi networks found.\n");
        lv_obj_t * list_btn = lv_list_add_btn(parent_list, NULL, "No networks found");
        lv_obj_add_state(list_btn, LV_STATE_DISABLED);
    } else {
        printf("Found %d Wi-Fi networks.\n", networkCount);
        for (int i = 0; i < networkCount; ++i) {
            const char * ssid = WiFi.SSID(i).c_str();
            // Create a list item for each network
            lv_obj_t * list_btn = lv_list_add_btn(parent_list, NULL, ssid);
            // Add event callback to handle network selection
            lv_obj_add_event_cb(list_btn, wifi_network_selected_cb, LV_EVENT_CLICKED, (void *)ssid);
        }
    }
    WiFi.scanDelete();
}