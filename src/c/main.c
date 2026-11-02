#include <pebble.h>
#include "face.h"
#include "settings.h"

static Window *s_window;

static void on_tick(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  face_tick();
}

static void on_settings_changed(void) {
  const FaceSettings *s = settings_get();
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(
      s->show_seconds ? SECOND_UNIT : MINUTE_UNIT, on_tick);
  face_apply_settings();
}

static void window_load(Window *window) {
  face_init(window);
}

static void window_unload(Window *window) {
  (void)window;
  face_deinit();
}

static void init(void) {
  settings_init();
  settings_set_changed_callback(on_settings_changed);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  const FaceSettings *s = settings_get();
  tick_timer_service_subscribe(
      s->show_seconds ? SECOND_UNIT : MINUTE_UNIT, on_tick);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  settings_deinit();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
