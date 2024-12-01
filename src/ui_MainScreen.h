// ui_mainscreen.h
#ifndef UI_MAINSCREEN_H
#define UI_MAINSCREEN_H

#include "lvgl.h"

void ui_MainScreen_screen_init(void);
void update_main_screen(void);
void create_combined_scale(void);
void create_segmented_ring(lv_obj_t * parent);

#endif // MAINSCREEN_H
