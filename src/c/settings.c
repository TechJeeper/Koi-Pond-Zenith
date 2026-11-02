#include "settings.h"

enum {
  PERSIST_TIME_FORMAT = 1,
  PERSIST_LEADING_ZERO = 2,
  PERSIST_SHOW_DIGITAL = 3,
  PERSIST_SHOW_DATE = 4,
  PERSIST_SHOW_SECONDS = 5,
  PERSIST_SWIM_CW = 6,
  PERSIST_RING_INSET = 7,
};

static FaceSettings s_settings;
static SettingsChangedFn s_changed;
static bool s_inbox_open;

static int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static void normalize(void) {
  s_settings.ring_inset = clamp_int(s_settings.ring_inset, 8, 40);
}

static void settings_set_defaults(void) {
  s_settings.time_format = TIME_FMT_SYSTEM;
  s_settings.leading_zero = true;
  s_settings.show_digital = true;
  s_settings.show_date = false;
  s_settings.show_seconds = true;
  s_settings.swim_clockwise = true;
  s_settings.ring_inset = 18;
}

static void settings_load(void) {
  settings_set_defaults();
  if (persist_exists(PERSIST_TIME_FORMAT)) {
    s_settings.time_format = (TimeFormat)clamp_int(
        persist_read_int(PERSIST_TIME_FORMAT), TIME_FMT_SYSTEM, TIME_FMT_24H);
  }
  if (persist_exists(PERSIST_LEADING_ZERO)) {
    s_settings.leading_zero = persist_read_bool(PERSIST_LEADING_ZERO);
  }
  if (persist_exists(PERSIST_SHOW_DIGITAL)) {
    s_settings.show_digital = persist_read_bool(PERSIST_SHOW_DIGITAL);
  }
  if (persist_exists(PERSIST_SHOW_DATE)) {
    s_settings.show_date = persist_read_bool(PERSIST_SHOW_DATE);
  }
  if (persist_exists(PERSIST_SHOW_SECONDS)) {
    s_settings.show_seconds = persist_read_bool(PERSIST_SHOW_SECONDS);
  }
  if (persist_exists(PERSIST_SWIM_CW)) {
    s_settings.swim_clockwise = persist_read_bool(PERSIST_SWIM_CW);
  }
  if (persist_exists(PERSIST_RING_INSET)) {
    s_settings.ring_inset = persist_read_int(PERSIST_RING_INSET);
  }
  normalize();
}

static void settings_save(void) {
  normalize();
  persist_write_int(PERSIST_TIME_FORMAT, (int)s_settings.time_format);
  persist_write_bool(PERSIST_LEADING_ZERO, s_settings.leading_zero);
  persist_write_bool(PERSIST_SHOW_DIGITAL, s_settings.show_digital);
  persist_write_bool(PERSIST_SHOW_DATE, s_settings.show_date);
  persist_write_bool(PERSIST_SHOW_SECONDS, s_settings.show_seconds);
  persist_write_bool(PERSIST_SWIM_CW, s_settings.swim_clockwise);
  persist_write_int(PERSIST_RING_INSET, s_settings.ring_inset);
}

static int tuple_as_int(Tuple *t, int fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return (int)t->value->int32;
  if (t->type == TUPLE_CSTRING) return atoi(t->value->cstring);
  return fallback;
}

static bool tuple_as_bool(Tuple *t, bool fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return t->value->int32 != 0;
  if (t->type == TUPLE_CSTRING) {
    return strcmp(t->value->cstring, "true") == 0 ||
           strcmp(t->value->cstring, "1") == 0;
  }
  return fallback;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  (void)context;
  Tuple *t;

  t = dict_find(iter, MESSAGE_KEY_TimeFormat);
  if (t) {
    s_settings.time_format = (TimeFormat)clamp_int(
        tuple_as_int(t, (int)s_settings.time_format), TIME_FMT_SYSTEM, TIME_FMT_24H);
  }
  t = dict_find(iter, MESSAGE_KEY_LeadingZero);
  if (t) {
    s_settings.leading_zero = tuple_as_bool(t, s_settings.leading_zero);
  }
  t = dict_find(iter, MESSAGE_KEY_ShowDigital);
  if (t) {
    s_settings.show_digital = tuple_as_bool(t, s_settings.show_digital);
  }
  t = dict_find(iter, MESSAGE_KEY_ShowDate);
  if (t) {
    s_settings.show_date = tuple_as_bool(t, s_settings.show_date);
  }
  t = dict_find(iter, MESSAGE_KEY_ShowSeconds);
  if (t) {
    s_settings.show_seconds = tuple_as_bool(t, s_settings.show_seconds);
  }
  t = dict_find(iter, MESSAGE_KEY_SwimClockwise);
  if (t) {
    s_settings.swim_clockwise = tuple_as_bool(t, s_settings.swim_clockwise);
  }
  t = dict_find(iter, MESSAGE_KEY_RingInset);
  if (t) {
    s_settings.ring_inset = tuple_as_int(t, s_settings.ring_inset);
  }

  settings_save();
  if (s_changed) {
    s_changed();
  }
}

void settings_init(void) {
  settings_load();
  if (!s_inbox_open) {
    app_message_register_inbox_received(inbox_received);
    app_message_open(app_message_inbox_size_maximum(),
                     app_message_outbox_size_maximum());
    s_inbox_open = true;
  }
}

void settings_deinit(void) {
  settings_save();
  if (s_inbox_open) {
    app_message_deregister_callbacks();
    s_inbox_open = false;
  }
}

void settings_set_changed_callback(SettingsChangedFn cb) {
  s_changed = cb;
}

const FaceSettings *settings_get(void) {
  return &s_settings;
}
