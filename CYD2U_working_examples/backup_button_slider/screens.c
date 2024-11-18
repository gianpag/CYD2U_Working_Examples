#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;

//ChatGPT suggested code for slider based dimming
static void event_handler_slider_cb_main_obj0(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int brightness = lv_slider_get_value(slider);
    set_brightness(brightness); // Update brightness state
}

static void event_handler_checked_cb_main_obj0(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (lv_obj_has_state(ta, LV_STATE_CHECKED)) {
      set_light_state(true);
    }
}

static void event_handler_unchecked_cb_main_obj0(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (!lv_obj_has_state(ta, LV_STATE_CHECKED)) {
      set_light_state(false);
    }
}

static void event_handler_checked_cb_main_obj1(lv_event_t *e) {
    lv_obj_t *ta = lv_event_get_target(e);
    if (lv_obj_has_state(ta, LV_STATE_CHECKED)) {
      LV_LOG_USER(" Switch Obj1 Checked ");
    }
}

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 320, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xff11383e), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 41, 84);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, event_handler_checked_cb_main_obj0, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_add_event_cb(obj, event_handler_unchecked_cb_main_obj0, LV_EVENT_VALUE_CHANGED, (void *)0);
        }
        {
            lv_obj_t *obj = lv_switch_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 41, 141);
            lv_obj_set_size(obj, 50, 25);
            lv_obj_add_event_cb(obj, event_handler_checked_cb_main_obj1, LV_EVENT_VALUE_CHANGED, (void *)0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffa0c8c8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff50e6e6), LV_PART_MAIN | LV_STATE_CHECKED);
        }
        {
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.obj2 = obj; // Store reference for the first slider
            lv_obj_set_pos(obj, 112, 88);
            lv_obj_set_size(obj, 150, 17);
            lv_slider_set_range(obj, 0, 255); // Set range for brightness (0-255)
            lv_slider_set_value(obj, 25, LV_ANIM_OFF); // Initial brightness value
            lv_obj_add_event_cb(obj, event_handler_slider_cb_main_obj0, LV_EVENT_VALUE_CHANGED, NULL);
        }
        {
            lv_obj_t *obj = lv_slider_create(parent_obj);
            lv_obj_set_pos(obj, 112, 145);
            lv_obj_set_size(obj, 150, 17);
            lv_slider_set_value(obj, 25, LV_ANIM_OFF);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 77, 16);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Luci Salotto");
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffeff1f1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

void tick_screen_main() {
}


void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}

typedef void (*tick_screen_func_t)();

tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};

void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}

