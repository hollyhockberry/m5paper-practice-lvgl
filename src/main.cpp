// Copyright (c) 2021 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#define LGFX_AUTODETECT
#define LGFX_USE_V1
#include <lvgl.h>
#include <M5EPD.h>
#include <LovyanGFX.hpp>

namespace {

constexpr uint16_t screenWidth  = 960;
constexpr uint16_t screenHeight = 540;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

LGFX gfx;

void my_disp_flush(
    lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  const u_long w = area->x2 - area->x1 + 1;
  const u_long h = area->y2 - area->y1 + 1;

  if (gfx.getStartCount() <= 0) {
    gfx.startWrite();
  }
  gfx.setAddrWindow(area->x1, area->y1, w, h);
  gfx.pushPixels(static_cast<uint16_t *>(&color_p->full), w * h);

  if (lv_disp_flush_is_last(disp)) {
    gfx.endWrite();
  }
  ::lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t*, lv_indev_data_t* data) {
  M5.TP.update();
  if (!M5.TP.isFingerUp()) {
    const tp_finger_t finger = M5.TP.readFinger(0);
    data->state = LV_INDEV_STATE_PR;
    data->point.x = finger.x;
    data->point.y = finger.y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void btn_event_cb(lv_event_t * e) {
  lv_event_code_t code = ::lv_event_get_code(e);
  lv_obj_t * btn = ::lv_event_get_target(e);
  uint8_t* d = static_cast<uint8_t*>(lv_event_get_user_data(e));
  if (code == LV_EVENT_CLICKED) {
    *d += 1;
    lv_obj_t * label = ::lv_obj_get_child(btn, 0);
    ::lv_label_set_text_fmt(label, "Button: %d", *d);
  }
}

lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_label;

void style_init() {
  ::lv_style_init(&style_btn);
  ::lv_style_set_radius(&style_btn, 10);
  ::lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  ::lv_style_set_bg_color(&style_btn, ::lv_color_white());
  ::lv_style_set_border_color(&style_btn, ::lv_color_black());
  ::lv_style_set_border_opa(&style_btn, LV_OPA_COVER);
  ::lv_style_set_border_width(&style_btn, 2);

  // style for the pressed state.
  ::lv_style_init(&style_btn_pressed);
  ::lv_style_set_bg_color(&style_btn_pressed, ::lv_color_black());

  // label
  ::lv_style_init(&style_label);
  ::lv_style_set_text_font(&style_label, &lv_font_montserrat_42);
}

void create_objects() {
  style_init();
  static uint8_t data;
  lv_obj_t * btn = ::lv_btn_create(::lv_scr_act());
  ::lv_obj_remove_style_all(btn);
  ::lv_obj_set_pos(btn, 50, 100);
  ::lv_obj_set_size(btn, 400, 300);
  ::lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, &data);
  ::lv_obj_add_style(btn, &style_btn, LV_STATE_DEFAULT);
  ::lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

  lv_obj_t * label = ::lv_label_create(btn);
  ::lv_obj_add_style(label, &style_label, LV_STATE_DEFAULT);
  ::lv_label_set_text(label, "Button: 0");
  ::lv_obj_center(label);
}

}  // namespace

void setup() {
  M5.begin();
  M5.TP.SetRotation(180);

  gfx.begin();
  gfx.setRotation(3);
  gfx.setEpdMode(epd_mode_t::epd_quality);

  ::lv_init();
  ::lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  static lv_disp_drv_t disp_drv;
  ::lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  ::lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  ::lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  ::lv_indev_drv_register(&indev_drv);

  create_objects();
}

void loop() {
  ::lv_timer_handler();
  ::delay(5);
}
