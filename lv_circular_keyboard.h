/**
 * @file lv_circular_keyboard.h
 *
 * Header file for the circular keyboard LVGL widget.
 */

#ifndef LV_CIRCULAR_KEYBOARD_H
#define LV_CIRCULAR_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "Arduino.h"

/*********************
 *      DEFINES
 *********************/
#define MAX_KEYS 26
#define MAX_EXTRA_KEYS 16

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t obj;              // Base LVGL object
    lv_obj_t *arc;             // Arc for scrolling
    lv_obj_t *text_area;       // Linked text area
    lv_obj_t *key_buttons[MAX_KEYS + MAX_EXTRA_KEYS]; // Labels/buttons for keys
    lv_coord_t radius;         // Keyboard radius
    lv_coord_t center_x;       // Center x-coordinate for the keyboard
    lv_coord_t center_y;       // Center y-coordinate for the keyboard
    const char *layout[MAX_KEYS];    // Layout for keys
    lv_color_t bg_color;       // Background color
    lv_color_t text_color;     // Text color
    lv_color_t band_color;     // Circular band color
    lv_coord_t band_thickness; // Thickness of the circular band
    float angle_offset;        // Angle of the keybord's offset
    bool transparent;          // Transparent background option
    bool circular_band;        // Enable/Disable circular band
    bool is_uppercase;         // Uppercase toggle
    bool is_numbers;           // Numbers toggle
    bool is_symbols;           // Symbols toggle
    void (*on_close_cb)(void); // Close callback
    void (*on_ok_cb)(void);    // OK button callback
} lv_circular_keyboard_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a circular keyboard widget.
 * @param parent The parent LVGL object.
 * @param radius The radius of the keyboard.
 * @return Pointer to the created circular keyboard object.
 */
lv_obj_t * lv_circular_keyboard_create(lv_obj_t * parent, lv_coord_t radius);

/**
 * Set the linked text area for the keyboard.
 * @param obj Pointer to the circular keyboard object.
 * @param text_area Pointer to the linked text area.
 */
void lv_circular_keyboard_set_textarea(lv_obj_t * obj, lv_obj_t * text_area);

/**
 * Set the close callback for the keyboard.
 * @param obj Pointer to the circular keyboard object.
 * @param on_close_cb Callback function to invoke on close.
 */
void lv_circular_keyboard_set_on_close(lv_obj_t * obj, void (*on_close_cb)(void));

/**
 * Set the OK button callback for the keyboard.
 * @param obj Pointer to the circular keyboard object.
 * @param on_ok_cb Callback function to invoke on OK button press.
 */
void lv_circular_keyboard_set_on_ok(lv_obj_t * obj, void (*on_ok_cb)(void));

/**
 * Set the style for the keyboard.
 * @param obj Pointer to the circular keyboard object.
 * @param bg_color Background color for the band or buttons.
 * @param text_color Text color for the keys.
 * @param transparent True for transparent keys, false otherwise.
 */
void lv_circular_keyboard_set_style(lv_obj_t * obj, lv_color_t bg_color, lv_color_t text_color, bool transparent);

/**
 * Set the position of the keyboard.
 * @param obj Pointer to the circular keyboard object.
 * @param x X-coordinate of the center.
 * @param y Y-coordinate of the center.
 */
void lv_circular_keyboard_set_position(lv_obj_t * obj, lv_coord_t x, lv_coord_t y);

/**
 * Set the circular band style.
 * @param obj Pointer to the circular keyboard object.
 * @param color Color of the circular band.
 * @param thickness Thickness of the circular band.
 */
void lv_circular_keyboard_set_band_style(lv_obj_t * obj, lv_color_t color, lv_coord_t thickness);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* LV_CIRCULAR_KEYBOARD_H */
