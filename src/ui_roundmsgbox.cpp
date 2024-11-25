#include "ui_roundmsgbox.h"
#include <Arduino.h>

extern void sendActionToiPhone(const char* action);

UIRoundMsgBox::UIRoundMsgBox() 
    : msgbox(nullptr), category_label(nullptr), message_label(nullptr), dismiss_btn(nullptr), positive_btn(nullptr) {}

UIRoundMsgBox::~UIRoundMsgBox() {
    if (msgbox) {
        lv_obj_del(msgbox);
    }
}

void UIRoundMsgBox::init_styles() {
    // Add style initialization here if necessary
}

lv_obj_t* UIRoundMsgBox::create(lv_obj_t* parent, const std::string& category, const std::string& message, const std::string& dismiss, bool hasPositiveAction) {
    msgbox = lv_obj_create(parent);
    if (!msgbox) {
        Serial.println("Failed to create msgbox");
        return nullptr;
    }
    lv_obj_set_size(msgbox, 200, 200); // Size of the messagebox (adjust as needed)
    lv_obj_set_style_radius(msgbox, LV_RADIUS_CIRCLE, 0); // Make it circular
    lv_obj_set_style_bg_color(msgbox, lv_color_black(), 0); // Background color
    lv_obj_set_style_pad_all(msgbox, 10, 0); // Padding for better layout
    lv_obj_center(msgbox); // Center the message box

    // Category label
    category_label = lv_label_create(msgbox);
    lv_label_set_text(category_label, category.c_str());
    lv_obj_set_style_text_color(category_label, lv_color_white(), 0); // Text color
    lv_obj_align(category_label, LV_ALIGN_TOP_MID, 0, 10); // Align near the top

    // Message content label
    message_label = lv_label_create(msgbox);
    lv_label_set_text(message_label, message.c_str());
    lv_obj_set_style_text_color(message_label, lv_color_white(), 0); // Text color
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 10); // Center the label

      // Dismiss button
    dismiss_btn = lv_btn_create(msgbox);
    lv_obj_set_size(dismiss_btn, 100, 40);
    lv_obj_align(dismiss_btn, LV_ALIGN_BOTTOM_LEFT, -20, -10);
    dismiss_label = lv_label_create(dismiss_btn);
    lv_label_set_text(dismiss_label, dismiss.c_str());
    lv_obj_center(dismiss_label);

    // Positive action button if required
    if (hasPositiveAction) {
        positive_btn = lv_btn_create(msgbox);
        lv_obj_set_size(positive_btn, 100, 40);
        lv_obj_align(positive_btn, LV_ALIGN_BOTTOM_RIGHT, 20, -10);
        positive_label = lv_label_create(positive_btn);
        lv_label_set_text(positive_label, "Accept"); // Default label
        lv_obj_center(positive_label);
    }

    lv_refr_now(NULL);  // Force screen refresh to display

    return msgbox;
}

void UIRoundMsgBox::setDismissCallback(std::function<void()> callback) {
    lv_obj_add_event_cb(dismiss_btn, [](lv_event_t* e) {
        UIRoundMsgBox* instance = static_cast<UIRoundMsgBox*>(lv_event_get_user_data(e));
        if (instance->dismissCallback) {
            instance->dismissCallback();
        }
    }, LV_EVENT_CLICKED, this);
    dismissCallback = callback;
}

void UIRoundMsgBox::setPositiveCallback(std::function<void()> callback) {
    if (positive_btn) {
        lv_obj_add_event_cb(positive_btn, [](lv_event_t* e) {
            UIRoundMsgBox* instance = static_cast<UIRoundMsgBox*>(lv_event_get_user_data(e));
            if (instance->positiveCallback) {
                instance->positiveCallback();
            }
        }, LV_EVENT_CLICKED, this);
        positiveCallback = callback;
    }
}

// Add these new functions to set the button labels dynamically
void UIRoundMsgBox::setPositiveLabel(const char* label) {
    if (positive_btn && positive_label) {
        lv_label_set_text(positive_label, label);
        lv_refr_now(NULL);  // Force screen refresh to display changes
    }
}

void UIRoundMsgBox::setDismissLabel(const char* label) {
    if (dismiss_label) {
        lv_label_set_text(dismiss_label, label);
        lv_refr_now(NULL);  // Force screen refresh to display changes
    }
}

