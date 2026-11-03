#include "face.h"
#include "koi.h"
#include "settings.h"

static Window *s_window;
static Layer *s_layer;
static GBitmap *s_pond;

static int s_hour;
static int s_minute;
static int s_second;
static int s_wday;
static int s_mday;
static int s_mon;

// Clock angle: 0° at 12 o'clock, increasing clockwise (Pebble hand convention).
static GPoint point_on_ring(GPoint center, int radius, int angle_deg) {
  int32_t a = (TRIG_MAX_ANGLE * (angle_deg % 360)) / 360;
  return GPoint(
      center.x + (int)((sin_lookup(a) * radius) / TRIG_MAX_RATIO),
      center.y - (int)((cos_lookup(a) * radius) / TRIG_MAX_RATIO));
}

static void draw_koi(GContext *ctx, GBitmap *bmp, GPoint center) {
  if (!bmp) return;
  GSize sz = gbitmap_get_bounds(bmp).size;
  GRect dest = GRect(center.x - sz.w / 2, center.y - sz.h / 2, sz.w, sz.h);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, bmp, dest);
}

static bool use_24h(void) {
  const FaceSettings *s = settings_get();
  if (s->time_format == TIME_FMT_24H) return true;
  if (s->time_format == TIME_FMT_12H) return false;
  return clock_is_24h_style();
}

static void format_time(char *buf, size_t n) {
  int hour = s_hour;
  const bool is24 = use_24h();
  if (!is24) {
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  if (settings_get()->leading_zero) {
    snprintf(buf, n, "%02d:%02d", hour, s_minute);
  } else {
    snprintf(buf, n, "%d:%02d", hour, s_minute);
  }
}

static void format_date(char *buf, size_t n) {
  static const char *const DOW[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *const MON[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  snprintf(buf, n, "%s %d %s", DOW[s_wday % 7], s_mday, MON[s_mon % 12]);
}

// Pond art water circle ~58px from center (half≈72). Koi sprites are 32px, so
// the swim *center* must sit ~16px inside the water rim or fish land on stones.
#define POND_ART_HALF 72
#define POND_SWIM_R 42

static void update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const FaceSettings *cfg = settings_get();

  graphics_context_set_fill_color(ctx, GColorDarkGreen);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Pebble does not stretch bitmaps — draw 1:1, centered. Emery ships a
  // full-size pond resource so this fills Time 2 as well.
  GRect pond_rect = bounds;
  if (s_pond) {
    GSize psz = gbitmap_get_bounds(s_pond).size;
    pond_rect = GRect(
        (bounds.size.w - psz.w) / 2,
        (bounds.size.h - psz.h) / 2,
        psz.w, psz.h);
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    graphics_draw_bitmap_in_rect(ctx, s_pond, pond_rect);
  }

  GPoint center = GPoint(
      pond_rect.origin.x + pond_rect.size.w / 2,
      pond_rect.origin.y + pond_rect.size.h / 2);
  int half = (pond_rect.size.w < pond_rect.size.h
                  ? pond_rect.size.w
                  : pond_rect.size.h) / 2;
  int swim_r = (half * POND_SWIM_R) / POND_ART_HALF;
  int radius = swim_r - (cfg->ring_inset - 18);
  if (radius < 16) radius = 16;
  if (radius > swim_r) radius = swim_r;

  // Clock angles: 0° at 12, clockwise.
  int hour_angle = ((s_hour % 12) * 30) + (s_minute / 2);
  int minute_angle = s_minute * 6;
  int second_angle = s_second * 6;

  if (!cfg->swim_clockwise) {
    hour_angle = (360 - hour_angle) % 360;
    minute_angle = (360 - minute_angle) % 360;
    second_angle = (360 - second_angle) % 360;
  }

  // Seconds lily (small filled circle on the ring).
  if (cfg->show_seconds) {
    GPoint sp = point_on_ring(center, radius, second_angle);
    graphics_context_set_fill_color(ctx, GColorIslamicGreen);
    graphics_fill_circle(ctx, sp, 3);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_circle(ctx, sp, 3);
  }

  // White koi = hour, orange koi = minute.
  KoiDir hour_dir = koi_dir_from_clock_angle(hour_angle, cfg->swim_clockwise);
  KoiDir minute_dir = koi_dir_from_clock_angle(minute_angle, cfg->swim_clockwise);
  GPoint hp = point_on_ring(center, radius, hour_angle);
  GPoint mp = point_on_ring(center, radius, minute_angle);

  draw_koi(ctx, koi_white(hour_dir), hp);
  draw_koi(ctx, koi_orange(minute_dir), mp);

  // Soft digital readout centered in the pond.
  graphics_context_set_text_color(ctx, GColorWhite);
  if (cfg->show_digital) {
    char time_buf[8];
    format_time(time_buf, sizeof(time_buf));
    GRect time_box = GRect(0, center.y - 18, bounds.size.w, 24);
    graphics_draw_text(ctx, time_buf, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       time_box, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
  }
  if (cfg->show_date) {
    char date_buf[20];
    format_date(date_buf, sizeof(date_buf));
    GRect date_box = GRect(0, center.y + 6, bounds.size.w, 18);
    graphics_draw_text(ctx, date_buf, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       date_box, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
  }
}

static void refresh_time(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_second = t->tm_sec;
  s_wday = t->tm_wday;
  s_mday = t->tm_mday;
  s_mon = t->tm_mon;
}

void face_tick(void) {
  refresh_time();
  if (s_layer) {
    layer_mark_dirty(s_layer);
  }
}

void face_apply_settings(void) {
  face_tick();
}

void face_init(Window *window) {
  s_window = window;
  window_set_background_color(window, GColorBlack);

  s_pond = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POND);
  koi_init();

  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);

  refresh_time();
}

void face_deinit(void) {
  if (s_layer) {
    layer_destroy(s_layer);
    s_layer = NULL;
  }
  koi_deinit();
  if (s_pond) {
    gbitmap_destroy(s_pond);
    s_pond = NULL;
  }
}
