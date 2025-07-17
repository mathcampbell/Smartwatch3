/**
 * @file mc_circular_keyboard.c
 */

/*********************
 *      INCLUDES
 *********************/
#include "mc_circular_keyboard.h"
#include "lvgl.h"
#include <math.h>
#include <string.h> /* for strcmp, memcpy, etc. */

/*********************
 *      DEFINES
 *********************/

/* We have 26 letters plus these 6 control items. */
#define TOTAL_ITEMS  (26 + 6)  // 32 items
#define MAX_KEYS     26
#define CTRL_ITEMS   6

/* The 6 "control" strings we place among the letters */
static const char * control_items[CTRL_ITEMS] = {
    LV_SYMBOL_OK,        /* "OK" symbol */
    LV_SYMBOL_BACKSPACE, /* backspace symbol */
    LV_SYMBOL_CLOSE,     /* close symbol (X) */
    "123",
    "ABC",
    "@!#"
};

/* A gap at the top (in degrees) for the "select" button. */
#define TOP_GAP_ANGLE  30.0f

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void mc_circular_keyboard_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

static void draw_key_circle(lv_event_t * e);
static void mc_circular_keyboard_arc_event_cb(lv_event_t * e);
static void mc_circular_keyboard_select_event_cb(lv_event_t * e);

static void layout_all(lv_obj_t * obj);
static void layout_ring_items(lv_obj_t * obj);

static void switch_keyboard_layout(mc_circular_keyboard_t * kb, const char * mode_key);

static void mc_circular_keyboard_close_event(mc_circular_keyboard_t * kb);
static void mc_circular_keyboard_backspace_event(mc_circular_keyboard_t * kb);
static void mc_circular_keyboard_ok_event(mc_circular_keyboard_t * kb);



static void switch_layout(lv_obj_t * obj, kb_layout_t layout);
static void shift_key_event_cb(lv_event_t * e);
static void number_key_event_cb(lv_event_t * e);
static void symbol_key_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/

/* Our widget class definition */
const lv_obj_class_t mc_circular_keyboard_class = {
    .base_class     = &lv_obj_class,
    .constructor_cb = mc_circular_keyboard_constructor,
    .width_def      = 410,  /* Default widget size, adjustable as needed */
    .height_def     = 410,
    .instance_size  = sizeof(mc_circular_keyboard_t),
    .name           = "mc_circular_keyboard"
};

/* We’ll define a default array of 26 letters (lowercase) and 6 control items. */
static const char * default_letters[MAX_KEYS] = {
    "a","b","c","d","e","f","g","h","i","j",
    "k","l","m","n","o","p","q","r","s","t",
    "u","v","w","x","y","z"
};

/* static const char * layout_symbols[MAX_EXTRA_KEYS] = {
    "[", "]", "{", "}", "#", "%", "^", "*", "+", "=", "_", "\\", "|", "~", "<", ">"
}; */

static const char * layout_numbers[MAX_EXTRA_KEYS] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "/", ":", ";", "(", ")"
};

static const char * lowercase_keys[] = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, LV_SYMBOL_CLOSE, "123", "ABC", "@!#"
};

static const char * uppercase_keys[] = {
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, LV_SYMBOL_CLOSE, "123", "abc", "@!#"
};

static const char * number_keys[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, LV_SYMBOL_CLOSE, "ABC", "@!#"
};

static const char * symbol_keys[] = {
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", 
    "-", "_", "=", "+", "[", "]", "{", "}", ";", ":", "\"", ".", "/", "?",
    "~",
    LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, LV_SYMBOL_CLOSE, "123", "ABC"
};
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create the circular keyboard object.
 * This version always centers the widget in its parent 
 * and uses a default size of 300x300 unless changed externally.
 */
lv_obj_t * mc_circular_keyboard_create(lv_obj_t * parent, lv_coord_t radius)
{
    if(!parent) {
        printf("Error: Parent is null.\n");
        return NULL;
    }

    /* Create the base object */
    lv_obj_t * obj = lv_obj_class_create_obj(&mc_circular_keyboard_class, parent);
    if(!obj) {
        printf("Error: Failed to create mc_circular_keyboard object.\n");
        return NULL;
    }
    lv_obj_class_init_obj(obj);
    
    /* Access our data struct */
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->radius = radius;  // store user’s radius choice

    /* Optionally set a default size if none is set by parent’s layout. 
     * This ensures we have at least 300x300. 
     * The class has .width_def=300, .height_def=300, so usually that’s enough. 
     * But we can also explicitly do:
     */
    lv_obj_set_size(obj, 410, 410);

    /* Force the keyboard to be aligned center of the parent, ignoring user pos calls. */
    //lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_layout(parent, LV_LAYOUT_NONE);
    lv_obj_center(obj);
    //lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    /* Minimal style for the object so we can see the band if needed */
    lv_obj_set_style_bg_opa(obj, LV_OPA_20, LV_PART_MAIN);
   // lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(obj, LV_OPA_20, LV_PART_MAIN);

    printf("Keyboard size before layout: w=%d, h=%d\n", lv_obj_get_width(obj), lv_obj_get_height(obj));

    /* Make sure this container doesn’t scroll */
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    /* Attach our custom draw event to draw the ring band. */
    lv_obj_add_event_cb(obj, draw_key_circle, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    // Implementing our own scrolling.

    lv_obj_add_event_cb(obj, kb_touch_event_cb, LV_EVENT_ALL, NULL);

    /* Layout the ring items & create the top select button. */
    layout_all(obj);

    printf("Final keyboard size: w=%d, h=%d\n", lv_obj_get_width(obj), lv_obj_get_height(obj));

    return obj;
}

void mc_circular_keyboard_set_textarea(lv_obj_t * obj, lv_obj_t * text_area)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->text_area = text_area;
}

void mc_circular_keyboard_set_on_close(lv_obj_t * obj, void (*on_close_cb)(void))
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->on_close_cb = on_close_cb;
}

void mc_circular_keyboard_set_on_ok(lv_obj_t * obj, void (*on_ok_cb)(void))
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->on_ok_cb = on_ok_cb;
}

void mc_circular_keyboard_set_style(lv_obj_t * obj,
                                    lv_color_t bg_color,
                                    lv_color_t text_color,
                                    bool transparent)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    if(kb->circular_band) {
        kb->band_color = bg_color;
    } else {
        kb->bg_color = bg_color;
    }
    kb->text_color = text_color;
    kb->transparent = transparent;

    layout_all(obj);
    lv_obj_invalidate(obj);
}

/**
 * We ignore user attempts to set custom positions, 
 * because we want the widget to always be in the center.
 * If you do want to allow it, remove or comment out this approach.
 */
void mc_circular_keyboard_set_position(lv_obj_t * obj, lv_coord_t x, lv_coord_t y)
{
    printf("Ignoring set_position request, keyboard always centered.\n");
    /* no-op */
}

void mc_circular_keyboard_set_band_style(lv_obj_t * obj, lv_color_t color, lv_coord_t thickness)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->band_color     = color;
    kb->band_thickness = thickness;

    lv_obj_invalidate(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*--------------------------------------------------
 * 1) Constructor
 *--------------------------------------------------*/
static void mc_circular_keyboard_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;

    kb->text_area      = NULL;
    kb->is_uppercase   = false;
    kb->is_numbers     = false;
    kb->is_symbols     = false;
    kb->on_close_cb    = NULL;
    kb->on_ok_cb       = NULL;

    
    kb->transparent    = true;
    kb->circular_band  = true;
    kb->band_thickness = 40;

   // kb->arc            = NULL;
    kb->radius         = 175;  // default, can be overwritten in create
    kb->letter_radius = 190;

       kb->dragging     = false;
    kb->last_angle   = 0.0f;
    kb->offset_index = 0;  // Start with no offset

    kb->select_btn     = NULL;
    kb->select_btn_label = NULL;
    kb->text_color = lv_color_black();
    /* Zero out ring arrays. */
    memset(kb->ring_items, 0, sizeof(kb->ring_items));
    memset(kb->ring_labels, 0, sizeof(kb->ring_labels));

    /* Fill kb->ring_items with the 26 letters + 6 control items. */
    int idx = 0;
    for(int i=0; i<MAX_KEYS; i++) {
        kb->ring_items[idx++] = default_letters[i];
    }
    for(int c=0; c<CTRL_ITEMS; c++) {
        kb->ring_items[idx++] = control_items[c];
    }
    /* Now kb->ring_items[0..31] have the full ring content. */
}

/*--------------------------------------------------
 * 2) Layout & geometry
 *--------------------------------------------------*/
static float get_angle_from_center(mc_circular_keyboard_t * kb, int px, int py)
{
    // This is in absolute screen coords or parent coords, so either:
    //  1) convert (px,py) to local coords by subtracting obj->coords.x1
    //  2) or measure from kb->center_x in the same coordinate space used for p.x/p.y

    // If your kb->center_x, kb->center_y are in local coords, do:
    lv_area_t coords;
    lv_obj_get_coords((lv_obj_t *)kb, &coords);
    int obj_x = coords.x1; 
    int obj_y = coords.y1;

    // local coords relative to keyboard object
    int local_x = px - obj_x;
    int local_y = py - obj_y;

    float dx = local_x - kb->center_x;
    float dy = local_y - kb->center_y;

    float angle_deg = atan2f(dy, dx) * (180.0f / 3.14159f);
    // ensure angle_deg in [0..360)
    if(angle_deg < 0.0f) angle_deg += 360.0f;
    return angle_deg;
}


static void layout_all(lv_obj_t * obj)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;

     printf("layout_all: w=%d h=%d\n", lv_obj_get_width(obj), lv_obj_get_height(obj));

    /* Always center the circle in the object */
    kb->center_x = lv_obj_get_width(obj)  / 2;
    kb->center_y = lv_obj_get_height(obj) / 2;

    // kb->center_x = 180;  // e.g. 180
    //  kb->center_y = 180;  // e.g. 180

    /* Create or update the top "select" button if not existing. */
    if(!kb->select_btn) {
        kb->select_btn = lv_btn_create(obj);
        lv_obj_set_size(kb->select_btn, 50, 50);
       // lv_obj_set_style_radius(kb->select_btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_radius(kb->select_btn, 0, 0);
        kb->select_btn_label = lv_label_create(kb->select_btn);
        lv_obj_center(kb->select_btn_label);

        /* The only button event: user taps to confirm selection. */
        lv_obj_add_event_cb(kb->select_btn, mc_circular_keyboard_select_event_cb,
                            LV_EVENT_CLICKED, obj);
    }


    /* Layout all ring items as labels. */
    layout_ring_items(obj);

    /* Position the big select button at the top gap. 
     * We offset it from center_y by (radius + half button size). 
     */
    lv_coord_t btn_x = kb->center_x - 25;  // half of 50
    lv_coord_t btn_y = kb->center_y - (kb->radius + 10);
    lv_obj_set_pos(kb->select_btn, btn_x, btn_y);
}

static void layout_ring_items(lv_obj_t * obj)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;

    /* 1) Choose which keys[] array to use based on kb->current_layout */
    const char ** keys;
    int key_count;

    switch (kb->current_layout) {
        case KB_LAYOUT_UPPERCASE:
            keys = uppercase_keys;
            key_count = sizeof(uppercase_keys) / sizeof(uppercase_keys[0]);
            break;
        case KB_LAYOUT_NUMBERS:
            keys = number_keys;
            key_count = sizeof(number_keys) / sizeof(number_keys[0]);
            break;
        case KB_LAYOUT_SYMBOLS:
            keys = symbol_keys;
            key_count = sizeof(symbol_keys) / sizeof(symbol_keys[0]);
            break;
        case KB_LAYOUT_LOWERCASE:
        default:
            keys = lowercase_keys;
            key_count = sizeof(lowercase_keys) / sizeof(lowercase_keys[0]);
            break;
    }

    /* 2) The ring covers (360 - TOP_GAP_ANGLE) degrees, 
          and we have key_count items physically. */
    float angle_range   = 360.0f - TOP_GAP_ANGLE;  
    float step          = angle_range / (float)key_count;  

    /* 3) Place key_count labels around the circle.
          We skip the item at offset_index (the “active” item),
          so let's shift by +1 in the formula. */
    for(int i = 0; i < key_count; i++)
    {
        float angle_deg =  -90.0f
                         + (TOP_GAP_ANGLE / 2.0f)
                         + (i * step);
        float angle_rad = angle_deg * (3.14159f / 180.0f);

        /* letter_idx is the “rotated” index of the new layout array
           so we skip the “active” item at offset_index. */
        int letter_idx = (kb->offset_index + 1 + i) % key_count;
        const char * txt = keys[letter_idx];

        /* Compute label position on the circle (kb->letter_radius from center). */
        lv_coord_t x = kb->center_x + (lv_coord_t)(kb->letter_radius * cosf(angle_rad));
        lv_coord_t y = kb->center_y + (lv_coord_t)(kb->letter_radius * sinf(angle_rad));

        /* If ring_labels[i] doesn’t exist yet, create it. */
        if(!kb->ring_labels[i]) {
            kb->ring_labels[i] = lv_label_create(obj);
        }

        /* Update the label text and style. */
        lv_obj_t * label = kb->ring_labels[i];
        lv_label_set_text(label, txt);
        lv_obj_set_style_text_color(label, kb->text_color, LV_PART_MAIN);

        /* Position the label. */
        lv_coord_t lw = lv_obj_get_width(label);
        lv_coord_t lh = lv_obj_get_height(label);
        lv_obj_set_pos(label, x - lw/2, y - lh/2);

        /* Make sure it’s visible in case we hid it before. */
        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    }

    /* 4) Hide leftover labels if the new layout has fewer items 
          than a previous layout. For example, if ring_labels[] 
          can hold up to 32, we hide [key_count..31]. */
    for(int i = key_count; i < 32; i++) {
        if(kb->ring_labels[i]) {
            lv_obj_add_flag(kb->ring_labels[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}



/*--------------------------------------------------
 * 3) Arc event: user scrolled the ring
 *--------------------------------------------------*/
static void mc_circular_keyboard_arc_event_cb(lv_event_t * e)
{
    lv_obj_t * arc = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    if(!kb) return;

    // Arc value in [0..(TOTAL_ITEMS-1)]
    int16_t val = lv_arc_get_value(arc);
    printf("Arc scrolled to %d\n", val);

    // Re-layout so the text on each label "slot" is shifted
    layout_ring_items(obj);

    // The top letter (the one behind the gap) is ring_items[val]
    if(val >= 0 && val < TOTAL_ITEMS && kb->select_btn_label) {
        const char * item_txt = kb->ring_items[val];
        lv_label_set_text(kb->select_btn_label, item_txt);
    }
    // Redraw band if needed
    lv_obj_invalidate(obj);
}

static void kb_touch_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    lv_event_code_t code = lv_event_get_code(e);

    switch(code) {
        case LV_EVENT_PRESSED:
        case LV_EVENT_PRESSING:
            handle_press_drag(kb, e);
            break;

        case LV_EVENT_RELEASED:
        case LV_EVENT_PRESS_LOST:
            kb->dragging = false;
            break;

        default:
            break;
    }
}

static void handle_press_drag(mc_circular_keyboard_t * kb, lv_event_t * e)
{
    // Mark that we are dragging
    if(!kb->dragging) {
        kb->dragging = true;
    }

    // 1) Get finger position in screen coords
    lv_point_t p;
    lv_indev_get_point(lv_event_get_indev(e), &p);

    // 2) Convert to local coords
    lv_area_t coords;
    lv_obj_get_coords((lv_obj_t *)kb, &coords);
    int local_x = p.x - coords.x1;
    int local_y = p.y - coords.y1;

    float dx = (float)local_x - kb->center_x;
    float dy = (float)local_y - kb->center_y;

    // 3) Convert to angle in degrees, shift so 0° is top if you like
    float new_angle = (atan2f(dy, dx) * 180.0f / 3.14159f) - 90.0f;
    if(new_angle < 0) new_angle += 360.0f;

    // 4) On first drag, initialize last_angle
    if(!kb->dragging || kb->last_angle < 0.1f) {
        kb->last_angle = new_angle;
    }

    // 5) Compute angle delta
    float delta = new_angle - kb->last_angle;
    if(delta > 180.0f)  delta -= 360.0f;
    else if(delta < -180.0f) delta += 360.0f;

    kb->last_angle = new_angle; // update for next event

    // 6) Convert angle delta to item steps
    float degrees_per_item = 10.0f; // e.g. 1 item per 10 degrees
    float drag_factor      = 0.5f;
    float items_f          = (delta / degrees_per_item) * drag_factor;
    
    // 7) Update offset_index
    kb->offset_index -= (int)lrintf(items_f);
    // wrap offset in [0..(TOTAL_ITEMS-1)]
    kb->offset_index = (kb->offset_index + TOTAL_ITEMS) % TOTAL_ITEMS;

    // 8) Re-layout ring items with new offset
    layout_ring_items((lv_obj_t *)kb);

    // 9) Update the top button label from the correct layout array
    if(kb->select_btn_label) {
        // figure out which array we’re using right now
        const char ** keys;
        int key_count = 0;
        switch (kb->current_layout) {
            case KB_LAYOUT_UPPERCASE:
                keys = uppercase_keys;
                key_count = sizeof(uppercase_keys) / sizeof(uppercase_keys[0]);
                break;
            case KB_LAYOUT_NUMBERS:
                keys = number_keys;
                key_count = sizeof(number_keys) / sizeof(number_keys[0]);
                break;
            case KB_LAYOUT_SYMBOLS:
                keys = symbol_keys;
                key_count = sizeof(symbol_keys) / sizeof(symbol_keys[0]);
                break;
            case KB_LAYOUT_LOWERCASE:
            default:
                keys = lowercase_keys;
                key_count = sizeof(lowercase_keys) / sizeof(lowercase_keys[0]);
                break;
        }

        // clamp offset_index if new layout has fewer items
        int top_idx = kb->offset_index % key_count;
        lv_label_set_text(kb->select_btn_label, keys[top_idx]);
    }

    // 10) Force redraw if needed
    lv_obj_invalidate((lv_obj_t *)kb);
}





/*--------------------------------------------------
 * 4) Selecting the top item
 *--------------------------------------------------*/
static void mc_circular_keyboard_select_event_cb(lv_event_t * e)
{
    /* The user tapped the big button. Let’s see which item is currently displayed. */
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * obj = lv_event_get_user_data(e);
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    if(!kb) return;

    const char * selected_txt = lv_label_get_text(kb->select_btn_label);
    if(!selected_txt) return;

    printf("Selected item: %s\n", selected_txt);

    /* Check if it matches any control keywords. */
    if(strcmp(selected_txt, LV_SYMBOL_BACKSPACE) == 0) {
        mc_circular_keyboard_backspace_event(kb);
    }
    else if(strcmp(selected_txt, LV_SYMBOL_CLOSE) == 0) {
        mc_circular_keyboard_close_event(kb);
    }
    else if(strcmp(selected_txt, LV_SYMBOL_OK) == 0) {
        mc_circular_keyboard_ok_event(kb);
    }
    
    else if(strcmp(selected_txt, "123") == 0) {
        switch_layout(kb, KB_LAYOUT_NUMBERS);
    }
    else if (strcmp(selected_txt, "ABC") == 0) {
        switch_layout(obj, KB_LAYOUT_UPPERCASE);
    }
     else if (strcmp(selected_txt, "abc") == 0) {
        switch_layout(obj, KB_LAYOUT_LOWERCASE);
    }
    else if (strcmp(selected_txt, "@!#") == 0) {
        switch_layout(obj, KB_LAYOUT_SYMBOLS);
    }
    else {
        /* Otherwise treat it as a letter or symbol input. */
        if(kb->text_area) {
            lv_textarea_add_text(kb->text_area, selected_txt);
        }
    }
}

/*--------------------------------------------------
 * 5) Control item operations
 *--------------------------------------------------*/
static void mc_circular_keyboard_close_event(mc_circular_keyboard_t * kb)
{
    if(kb->on_close_cb) {
        kb->on_close_cb();
    }
    /* self-delete */
    lv_obj_del((lv_obj_t *)kb);
}

static void mc_circular_keyboard_backspace_event(mc_circular_keyboard_t * kb)
{
    if(!kb->text_area) return;
    printf("Backspace pressed\n");
    lv_textarea_delete_char(kb->text_area);
}

static void mc_circular_keyboard_ok_event(mc_circular_keyboard_t * kb)
{
    printf("OK pressed\n");
    if(kb->on_ok_cb) {
        kb->on_ok_cb();
    }
}

/*--------------------------------------------------
 * 6) Switch layouts (numbers, symbols, etc.)
 *--------------------------------------------------*/
static void switch_keyboard_layout(mc_circular_keyboard_t * kb, const char * mode_key)
{
    /* You might replace the ring_items[] with numeric or symbol items, 
     * or toggle uppercase, etc. 
     */
    if(strcmp(mode_key, "123") == 0) {
        kb->is_numbers  = true;
        kb->is_symbols  = false;
        kb->is_uppercase= false;
        /* Replace kb->ring_items with your numeric array, etc. */
    }
    else if(strcmp(mode_key, "ABC") == 0) {
        kb->is_numbers  = false;
        kb->is_symbols  = false;
        kb->is_uppercase= false;
        /* Replace with your default letters array, etc. */
    }
    else if(strcmp(mode_key, "@!#") == 0) {
        kb->is_numbers  = false;
        kb->is_symbols  = true;
        kb->is_uppercase= false;
        /* Replace with your symbol array, etc. */
    }

    /* Re-layout everything. */
    layout_all((lv_obj_t *)kb);
    lv_obj_invalidate((lv_obj_t *)kb);
}

static void switch_layout(lv_obj_t * obj, kb_layout_t layout)
{
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    kb->current_layout = layout;
    layout_ring_items(obj);
}

static void shift_key_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    switch_layout(obj, KB_LAYOUT_UPPERCASE);
}

static void number_key_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    switch_layout(obj, KB_LAYOUT_NUMBERS);
}

static void symbol_key_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    switch_layout(obj, KB_LAYOUT_SYMBOLS);
}

/*--------------------------------------------------
 * 7) Draw the circular band
 *--------------------------------------------------*/
static void draw_key_circle(lv_event_t * e)
{
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    if(!draw_task) return;

    lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);
    if(!base_dsc) return;

    if(base_dsc->part != LV_PART_MAIN) return;

    lv_obj_t * obj = lv_event_get_target(e);
    mc_circular_keyboard_t * kb = (mc_circular_keyboard_t *)obj;
    if(!kb || kb->radius <= 0) return;

    /* We'll draw a ring from radius -> radius - band_thickness, i.e. inward ring. */
    lv_draw_rect_dsc_t ring_dsc;
    lv_draw_rect_dsc_init(&ring_dsc);
    ring_dsc.bg_opa       = LV_OPA_TRANSP;   // no fill
    ring_dsc.border_color = kb->band_color;
    ring_dsc.border_opa   = LV_OPA_COVER;
    ring_dsc.border_width = kb->band_thickness;     
    ring_dsc.radius       = LV_RADIUS_CIRCLE;       

    /* The bounding box is the outer circle from (center - radius) to (center + radius). */

     kb->center_x =  180;
    kb->center_y =  180;

    lv_area_t ring_area;
    ring_area.x1 = kb->center_x - kb->radius;
    ring_area.y1 = kb->center_y - kb->radius;
    ring_area.x2 = kb->center_x + kb->radius;
    ring_area.y2 = kb->center_y + kb->radius;

    printf("Drawing inward ring now.\n");
    lv_draw_rect(base_dsc->layer, &ring_dsc, &ring_area);
}
