/**
 * @file mc_circular_keyboard.h
 * @brief Header for the mc_circular_keyboard widget
 */

#ifndef MC_CIRCULAR_KEYBOARD_H
#define MC_CIRCULAR_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "Arduino.h"  /* If needed for your environment */

/*********************
 *      DEFINES
 *********************/
/* If you want to define the total size here, or keep them private in the .c file. */
#define MAX_KEYS       26
#define MAX_EXTRA_KEYS 16
/* 26 letters + 6 control items = 32 total, but you can adjust as needed. */
#define TOTAL_ITEMS    (MAX_KEYS + 6)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    KB_LAYOUT_LOWERCASE,
    KB_LAYOUT_UPPERCASE,
    KB_LAYOUT_NUMBERS,
    KB_LAYOUT_SYMBOLS
} kb_layout_t;

/**
 * A custom data structure that extends `lv_obj_t` with extra data
 * needed for the circular keyboard widget.
 */
typedef struct {
    /**********************************************
     * Core LVGL object data
     **********************************************/
    lv_obj_t obj;  /**< Base object. Usually not accessed directly. */

    /**********************************************
     * Geometry and visual style
     **********************************************/
    lv_coord_t radius;         /**< Base radius for the ring. */
    lv_coord_t center_x;       /**< X coordinate of the ring center (local coords). */
    lv_coord_t center_y;       /**< Y coordinate of the ring center (local coords). */

    lv_color_t band_color;     /**< Color of the circular band. */
    lv_color_t bg_color;       /**< Fallback background color (unused if band is active). */
    lv_color_t text_color;     /**< Color of the text in ring labels. */
    lv_coord_t band_thickness; /**< Thickness for the ring band (drawn inward). */
    bool       circular_band;  /**< If true, draws the band ring. */
    bool       transparent;    /**< If true, background is transparent. */
    
    /**********************************************
     * (Optional) Arc-based scrolling & layout
     * - If not using lv_arc, you can remove these.
     **********************************************/
    lv_obj_t * arc;            /**< The invisible arc (if using arc-based scrolling). */
    float      angle_offset;   /**< Current angle offset for the ring (arc-based approach). */

    /**********************************************
     * New direct-drag scrolling method
     **********************************************/
    bool  dragging;      /**< Are we currently dragging? */
    float last_angle;    /**< Last angle (in degrees) of the pointer. */
    int   offset_index;  /**< How many steps we’ve scrolled around the circle (for rotating letters). */

    /**********************************************
     * Letters/Items arrays
     * - 26 letters + up to 6 control items
     **********************************************/
    const char * ring_items[32];  /**< The text items: letters + control strings. */
    lv_obj_t   * ring_labels[32]; /**< Each slot in the ring has a label object. */
    lv_coord_t letter_radius;
     kb_layout_t current_layout; 

    /**********************************************
     * Single “select” button at the top
     **********************************************/
    lv_obj_t * select_btn;        /**< The big button that the user taps to confirm selection. */
    lv_obj_t * select_btn_label;  /**< Label inside select_btn to show the current item. */

    /**********************************************
     * State flags for layout switching
     **********************************************/
    bool is_uppercase;
    bool is_numbers;
    bool is_symbols;

    /**********************************************
     * Text area to insert text into
     **********************************************/
    lv_obj_t * text_area;         /**< The target lv_textarea pointer */

    /**********************************************
     * Callbacks for close and OK
     **********************************************/
    void (*on_close_cb)(void);    /**< Called when the “Close” command is selected */
    void (*on_ok_cb)(void);       /**< Called when the “OK” command is selected */

} mc_circular_keyboard_t;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**
 * Create a new mc_circular_keyboard widget.
 * @param parent The parent LVGL object
 * @param radius The radius of the ring
 * @return       Pointer to the created keyboard object (lv_obj_t*)
 */
lv_obj_t * mc_circular_keyboard_create(lv_obj_t * parent, lv_coord_t radius);

/**
 * Set a textarea for the keyboard to insert text into.
 * @param obj       The keyboard object
 * @param text_area An `lv_textarea` pointer
 */
void mc_circular_keyboard_set_textarea(lv_obj_t * obj, lv_obj_t * text_area);

/**
 * Set a callback to be called when user selects the "Close" item.
 * @param obj         The keyboard object
 * @param on_close_cb Function pointer for the callback
 */
void mc_circular_keyboard_set_on_close(lv_obj_t * obj, void (*on_close_cb)(void));

/**
 * Set a callback to be called when user selects the “OK” item.
 * @param obj     The keyboard object
 * @param on_ok_cb Callback for the “OK” item
 */
void mc_circular_keyboard_set_on_ok(lv_obj_t * obj, void (*on_ok_cb)(void));

/**
 * Set the style (color, text color, transparency, etc.) for the keyboard.
 * @param obj         The keyboard object
 * @param bg_color    If circular_band=true, this is the band color; otherwise a fallback BG color
 * @param text_color  The color used for label text
 * @param transparent If true, background is transparent
 */
void mc_circular_keyboard_set_style(lv_obj_t * obj,
                                    lv_color_t bg_color,
                                    lv_color_t text_color,
                                    bool transparent);

/**
 * Set the band color and thickness if drawing the circular band inward.
 * @param obj        The keyboard object
 * @param color      The band color
 * @param thickness  The band thickness in pixels
 */
void mc_circular_keyboard_set_band_style(lv_obj_t * obj,
                                         lv_color_t color,
                                         lv_coord_t thickness);

/**
 * (Optional) If you want to allow positioning, or ignore it if you always center.
 * @param obj The keyboard object
 * @param x   X coordinate for top-left
 * @param y   Y coordinate for top-left
 */
void mc_circular_keyboard_set_position(lv_obj_t * obj, lv_coord_t x, lv_coord_t y);


static void kb_touch_event_cb(lv_event_t * e);

static void handle_press_drag(mc_circular_keyboard_t * kb, lv_event_t * e);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* MC_CIRCULAR_KEYBOARD_H */
