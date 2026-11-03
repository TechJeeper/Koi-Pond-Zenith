#pragma once

#include <pebble.h>
#include <stdbool.h>

typedef enum {
  TIME_FMT_SYSTEM = 0,
  TIME_FMT_12H,
  TIME_FMT_24H
} TimeFormat;

typedef struct {
  TimeFormat time_format;
  bool leading_zero;
  bool show_digital;
  bool show_date;
  bool show_seconds;
  bool swim_clockwise;
  int ring_inset;  // higher = swim tighter inside the water (default 18)
} FaceSettings;

typedef void (*SettingsChangedFn)(void);

void settings_init(void);
void settings_deinit(void);
void settings_set_changed_callback(SettingsChangedFn cb);
const FaceSettings *settings_get(void);
