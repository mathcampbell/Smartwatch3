#ifndef UI_ROUNDMSGBOX_H
#define UI_ROUNDMSGBOX_H

#include "lvgl.h"
#include <string>
#include <functional>  // Include functional for callback support

class UIRoundMsgBox {
public:
    // Constructor and Destructor
    UIRoundMsgBox();
    ~UIRoundMsgBox();

    // Method to create and show the round messagebox
    lv_obj_t* create(lv_obj_t* parent, const std::string& category, const std::string& message, const std::string& dismiss, bool positive);


       // Static callback function for the dismiss button
    static void dismiss_event_cb(lv_event_t* e);
    static void positive_event_cb(lv_event_t* e);

    // Set callbacks for dismiss and positive actions
    void setDismissCallback(std::function<void()> callback);
    void setPositiveCallback(std::function<void()> callback);

    // New functions to set button labels dynamically
    void setPositiveLabel(const char* label);
    void setDismissLabel(const char* label);

    
    
     lv_obj_t* message_label;  // Making this public to directly modify in the callback
         lv_obj_t* category_label;
    lv_obj_t* dismiss_btn;
    lv_obj_t* positive_btn;
    lv_obj_t* dismiss_label;
    lv_obj_t* positive_label;


private:
    // LVGL object references
    lv_obj_t* msgbox;


    // Callback functions for dismiss and positive action
    std::function<void()> dismissCallback;
    std::function<void()> positiveCallback;

    // Initialize styles (optional)
    void init_styles();
};

#endif // UI_ROUNDMSGBOX_H