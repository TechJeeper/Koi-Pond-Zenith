#pragma once

#include <pebble.h>

// Eight compass facings for PixelLab high top-down objects.
// Order matches angle buckets: 0=N, then clockwise NE..NW.
typedef enum {
  KOI_DIR_N = 0,
  KOI_DIR_NE,
  KOI_DIR_E,
  KOI_DIR_SE,
  KOI_DIR_S,
  KOI_DIR_SW,
  KOI_DIR_W,
  KOI_DIR_NW,
  KOI_DIR_COUNT
} KoiDir;

void koi_init(void);
void koi_deinit(void);

GBitmap *koi_white(KoiDir dir);
GBitmap *koi_orange(KoiDir dir);

// Convert a clock angle (0 = 12 o'clock, clockwise degrees) into a facing
// so the fish points along its swim path (tangential).
KoiDir koi_dir_from_clock_angle(int angle_deg, bool clockwise);
