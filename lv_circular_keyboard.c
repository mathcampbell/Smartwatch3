/**
 * @file lv_circular_keyboard.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_circular_keyboard.h"
#include "lvgl.h"
#include <math.h>

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_circular_keyboard_class
#define MAX_KEYS 26
#define MAX_EXTRA_KEYS 16

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_circular_keyboard_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_circular_keyboard_arc_event_cb(lv_event_t * e);
static void lv_circular_keyboard_key_event_cb(lv_event_t * e);
static void lv_circular_keyboard_close_event_cb(lv_event_t * e);
static void lv_circular_keyboard_backspace_event_cb(lv_event_t * e);
static void lv_circular_keyboard_shift_event_cb(lv_event_t * e);
static void lv_circular_keyboard_mode_event_cb(lv_event_t * e);
static void lv_circular_keyboard_update_layout(lv_obj_t * obj);
static void layout_keys(lv_obj_t * obj);
static void draw_key_circle(lv_event_t * e);
static void set_button_style(lv_obj_t * btn, lv_color_t bg_color, lv_color_t text_color, bool transparent);
static void switch_keyboard_layout(lv_circular_keyboard_t * kb, const char * mode_key);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_circular_keyboard_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = lv_circular_keyboard_constructor,
    .width_def = 200,
    .height_def = 200,
    .instance_size = sizeof(lv_circular_keyboard_t),
    .name = "circular_keyboard"
};

static const char * layout_symbols[MAX_EXTRA_KEYS] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=", "_", "\\", "|", "~", "<", ">"
};

static const char * layout_numbers[MAX_EXTRA_KEYS] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "/", ":", ";", "(", ")"
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_circular_keyboard_create(lv_obj_t * parent, lv_coord_t radius) {
    if (!parent) {
        printf("Error: Parent object is null.\n");
        return NULL;
    }

    // Padding to account for labels around the circle
    const lv_coord_t padding = 12;  // Adjust this as needed

    // Create the keyboard object
    lv_obj_t * obj = lv_obj_class_create_obj(&lv_circular_keyboard_class, parent);
    if (!obj) {
        printf("Error: Failed to create circular keyboard object.\n");
        return NULL;
    }

    lv_obj_class_init_obj(obj);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->radius = radius;
    kb->center_x = radius + padding;
    kb->center_y = radius + padding;
    lv_color_t blue_color = lv_palette_main(LV_PALETTE_BLUE);
    kb->circular_band = true;
    kb->band_color = blue_color;
    kb->band_thickness = 30;

    kb->on_close_cb = NULL;
    kb->on_ok_cb = NULL;

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); 

    // Set the size of the keyboard object with padding
    lv_obj_set_size(obj, 2 * radius + 2 * padding, 2 * radius + 2 * padding);

    // Add the custom draw event callback
    
    
    lv_obj_set_style_bg_opa(obj, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_border_opa(obj, LV_OPA_20, LV_PART_MAIN);

    lv_obj_add_event_cb(obj, draw_key_circle, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    // Create an arc for scrolling
    lv_obj_t * arc = lv_arc_create(obj);
    kb->arc = arc;
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_range(arc, 0, MAX_KEYS - 1);
    lv_arc_set_value(arc, 0);
    lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN); // Make the arc invisible
    lv_obj_add_event_cb(arc, lv_circular_keyboard_arc_event_cb, LV_EVENT_VALUE_CHANGED, obj);

    // Layout keys
    layout_keys(obj);

    return obj;
}

void lv_circular_keyboard_set_textarea(lv_obj_t * obj, lv_obj_t * text_area) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->text_area = text_area;
}

void lv_circular_keyboard_set_on_close(lv_obj_t * obj, void (*on_close_cb)(void)) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->on_close_cb = on_close_cb;
}

void lv_circular_keyboard_set_on_ok(lv_obj_t * obj, void (*on_ok_cb)(void)) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->on_ok_cb = on_ok_cb;
}

void lv_circular_keyboard_set_style(lv_obj_t * obj, lv_color_t bg_color, lv_color_t text_color, bool transparent) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    if (kb->circular_band)
    {
        kb->band_color = bg_color;
    }
    else
    {
        kb->bg_color = bg_color;
    }
    kb->text_color = text_color;
    kb->transparent = transparent;
    layout_keys(obj);
    lv_obj_invalidate(obj); // Force redraw to see updated style

}

void lv_circular_keyboard_set_position(lv_obj_t * obj, lv_coord_t x, lv_coord_t y) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->center_x = x;
    kb->center_y = y;

    layout_keys(obj);
    lv_obj_set_pos(obj, x - kb->radius, y - kb->radius);
     // Redraw to update the band style.
   lv_obj_invalidate(obj);
}

void lv_circular_keyboard_set_band_style(lv_obj_t * obj, lv_color_t color, lv_coord_t thickness) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    kb->band_color = color;
    kb->band_thickness = thickness;

   // Redraw to update the band style.
   lv_obj_invalidate(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_circular_keyboard_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj) {
    LV_UNUSED(class_p);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;

    kb->radius = 100;
    kb->text_area = NULL;
    kb->is_uppercase = false;
    kb->is_numbers = false;
    kb->is_symbols = false;
    kb->on_close_cb = NULL;
    kb->on_ok_cb = NULL;
    kb->bg_color = lv_color_black();
    kb->text_color = lv_color_white();
    kb->transparent = true;
    kb->circular_band = true;
    kb->band_thickness = 30;

    kb->angle_offset = 0;

    static const char * layout_lower[MAX_KEYS] = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
        "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
    };

    memcpy(kb->layout, layout_lower, sizeof(layout_lower));

    layout_keys(obj);
}

static void layout_keys(lv_obj_t * obj) {
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;

    if (kb->radius == 0 || kb->layout == NULL) {
        printf("Error: Invalid radius or layout\n");
        return;
    }

    const char * control_keys[] = {
        LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, LV_SYMBOL_CLOSE,
        "123", "ABC", "@!#"
    };

    const int control_keys_count = sizeof(control_keys) / sizeof(control_keys[0]);

    int total_keys = MAX_KEYS + control_keys_count;
    float angle_step = 2 * M_PI / total_keys;

    for (int i = 0; i < total_keys; i++) {
        const char * key_label;
        if (i < MAX_KEYS) {
            key_label = kb->layout[i];
        } else {
            key_label = control_keys[i - MAX_KEYS];
        }

        float angle = i * angle_step;
        lv_coord_t x = kb->center_x + kb->radius * cos(angle);
        lv_coord_t y = kb->center_y + kb->radius * sin(angle);

        // Create button if not already created
        if (!kb->key_buttons[i]) {
            kb->key_buttons[i] = lv_btn_create(obj);
        }
        lv_obj_t * btn = kb->key_buttons[i];

        lv_obj_set_size(btn, 30, 30);
        lv_obj_set_pos(btn, x - 15, y - 15);

        if (kb->circular_band) {
            lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_shadow_opa(btn, 0, 0);
        } else {
            set_button_style(btn, kb->bg_color, kb->text_color, kb->transparent);
        }

        // Create or update label
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        if (!label) {
            label = lv_label_create(btn);
        }
        lv_label_set_text(label, key_label);
        lv_obj_center(label);

        if (strcmp(key_label, LV_SYMBOL_CLOSE) == 0) {
            lv_obj_add_event_cb(btn, lv_circular_keyboard_close_event_cb, LV_EVENT_CLICKED, obj);
        } else if (strcmp(key_label, "@!#") == 0 || strcmp(key_label, "ABC") == 0 || strcmp(key_label, "123") == 0) {
            lv_obj_add_event_cb(btn, lv_circular_keyboard_mode_event_cb, LV_EVENT_CLICKED, obj);
        } else if (strcmp(key_label, LV_SYMBOL_BACKSPACE) == 0) {
            lv_obj_add_event_cb(btn, lv_circular_keyboard_backspace_event_cb, LV_EVENT_CLICKED, obj);
        } else {
            lv_obj_add_event_cb(btn, lv_circular_keyboard_key_event_cb, LV_EVENT_CLICKED, obj);
        }

        lv_obj_set_user_data(btn, obj);
    }
}

static void switch_keyboard_layout(lv_circular_keyboard_t * kb, const char * mode_key) {
    if (strcmp(mode_key, "123") == 0) {
        kb->is_numbers = true;
        kb->is_symbols = false;
        kb->is_uppercase = false;
    } else if (strcmp(mode_key, "ABC") == 0) {
        kb->is_numbers = false;
        kb->is_symbols = false;
        kb->is_uppercase = false;
    } else if (strcmp(mode_key, "@!#") == 0) {
        kb->is_numbers = false;
        kb->is_symbols = true;
        kb->is_uppercase = false;
    }

    layout_keys((lv_obj_t *)kb);
     lv_obj_invalidate((lv_obj_t *)kb);
}

static void draw_key_circle(lv_event_t * e)
{
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    if(!draw_task) return;

    lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);
    if(!base_dsc) return;

    if(base_dsc->part != LV_PART_MAIN) return;

    lv_obj_t * obj = lv_event_get_target(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;
    if(!kb || kb->radius <= 0) return;

    // We'll draw a ring from radius -> radius - band_thickness
    lv_draw_rect_dsc_t ring_dsc;
    lv_draw_rect_dsc_init(&ring_dsc);
    ring_dsc.bg_opa       = LV_OPA_TRANSP;          // no fill
    ring_dsc.border_color = kb->band_color;
    ring_dsc.border_opa   = LV_OPA_COVER;
    ring_dsc.border_width = kb->band_thickness;     // thickness inside the radius
    ring_dsc.radius       = LV_RADIUS_CIRCLE;       // full circle

    // The bounding box for the ring is just the outer circle
    lv_area_t ring_area;
    ring_area.x1 = kb->center_x - kb->radius;
    ring_area.y1 = kb->center_y - kb->radius;
    ring_area.x2 = kb->center_x + kb->radius;
    ring_area.y2 = kb->center_y + kb->radius;

    printf("Drawing inward ring now.\n");
    lv_draw_rect(base_dsc->layer, &ring_dsc, &ring_area);
}

static void lv_circular_keyboard_arc_event_cb(lv_event_t * e) {
    lv_obj_t * arc = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)obj;

    if (!kb) return;

    uint16_t value = lv_arc_get_value(arc);
    printf("Arc value changed: %u\n", value);

    // Example: rotate all keys by (value * 10) degrees
    kb->angle_offset = value * 10;
    layout_keys(obj);         // Re-layout to apply the new offset
    lv_obj_invalidate(obj);   // Force a redraw (so the band re-draws if needed)

    // [Optional] If you also want to highlight a "selected" button
    for (int i = 0; i < MAX_KEYS + MAX_EXTRA_KEYS; i++) {
        if (!kb->key_buttons[i]) continue;
        lv_obj_t * btn = kb->key_buttons[i];
        if (i == value) lv_obj_add_state(btn, LV_STATE_CHECKED);
        else lv_obj_clear_state(btn, LV_STATE_CHECKED);
    }
}

static void lv_circular_keyboard_close_event_cb(lv_event_t * e) {
    lv_obj_t * keyboard = lv_event_get_target(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)lv_obj_get_user_data(keyboard);

    if (!kb || !kb->on_close_cb) return;

    printf("Close button event triggered.\n");
    kb->on_close_cb();
    lv_obj_del(keyboard); // Cleanup the keyboard
}

static void lv_circular_keyboard_backspace_event_cb(lv_event_t * e) {
    lv_obj_t * keyboard = lv_event_get_target(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)lv_obj_get_user_data(keyboard);

    if (!kb || !kb->text_area) return;

    printf("Backspace pressed.\n");
    lv_textarea_delete_char(kb->text_area);
}

static void lv_circular_keyboard_key_event_cb(lv_event_t * e) {
    lv_obj_t * keyboard = lv_event_get_target(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)lv_obj_get_user_data(keyboard);

    if (!kb || !kb->text_area) return;

    lv_obj_t * btn = lv_event_get_target(e);
    const char * text = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (text) {
        printf("Key pressed: %s\n", text);
        lv_textarea_add_text(kb->text_area, text);
    }
}

static void lv_circular_keyboard_mode_event_cb(lv_event_t * e) {
    lv_obj_t * keyboard = lv_event_get_target(e);
    lv_circular_keyboard_t * kb = (lv_circular_keyboard_t *)lv_obj_get_user_data(keyboard);

    lv_obj_t * btn = lv_event_get_target(e);
    const char * text = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (text) {
        printf("Mode switch: %s\n", text);
        switch_keyboard_layout(kb, text);
    }
}

static void set_button_style(lv_obj_t * btn, lv_color_t bg_color, lv_color_t text_color, bool transparent) {
    lv_obj_set_style_bg_color(btn, bg_color, 0);
    lv_obj_set_style_text_color(btn, text_color, 0);

    if (transparent) {
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
    }
}
