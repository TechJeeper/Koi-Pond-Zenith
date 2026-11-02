#include "koi.h"

static GBitmap *s_white[KOI_DIR_COUNT];
static GBitmap *s_orange[KOI_DIR_COUNT];

static const uint32_t WHITE_RES[KOI_DIR_COUNT] = {
  RESOURCE_ID_IMAGE_KOI_WHITE_N,
  RESOURCE_ID_IMAGE_KOI_WHITE_NE,
  RESOURCE_ID_IMAGE_KOI_WHITE_E,
  RESOURCE_ID_IMAGE_KOI_WHITE_SE,
  RESOURCE_ID_IMAGE_KOI_WHITE_S,
  RESOURCE_ID_IMAGE_KOI_WHITE_SW,
  RESOURCE_ID_IMAGE_KOI_WHITE_W,
  RESOURCE_ID_IMAGE_KOI_WHITE_NW,
};

static const uint32_t ORANGE_RES[KOI_DIR_COUNT] = {
  RESOURCE_ID_IMAGE_KOI_ORANGE_N,
  RESOURCE_ID_IMAGE_KOI_ORANGE_NE,
  RESOURCE_ID_IMAGE_KOI_ORANGE_E,
  RESOURCE_ID_IMAGE_KOI_ORANGE_SE,
  RESOURCE_ID_IMAGE_KOI_ORANGE_S,
  RESOURCE_ID_IMAGE_KOI_ORANGE_SW,
  RESOURCE_ID_IMAGE_KOI_ORANGE_W,
  RESOURCE_ID_IMAGE_KOI_ORANGE_NW,
};

void koi_init(void) {
  for (int i = 0; i < KOI_DIR_COUNT; i++) {
    s_white[i] = gbitmap_create_with_resource(WHITE_RES[i]);
    s_orange[i] = gbitmap_create_with_resource(ORANGE_RES[i]);
  }
}

void koi_deinit(void) {
  for (int i = 0; i < KOI_DIR_COUNT; i++) {
    if (s_white[i]) {
      gbitmap_destroy(s_white[i]);
      s_white[i] = NULL;
    }
    if (s_orange[i]) {
      gbitmap_destroy(s_orange[i]);
      s_orange[i] = NULL;
    }
  }
}

GBitmap *koi_white(KoiDir dir) {
  return s_white[dir % KOI_DIR_COUNT];
}

GBitmap *koi_orange(KoiDir dir) {
  return s_orange[dir % KOI_DIR_COUNT];
}

KoiDir koi_dir_from_clock_angle(int angle_deg, bool clockwise) {
  // Tangential heading: fish nose points along the swim direction.
  int heading = clockwise ? (angle_deg + 90) : (angle_deg - 90);
  while (heading < 0) heading += 360;
  heading %= 360;
  // Buckets centered on N/NE/E/... (22.5° half-width).
  int bucket = ((heading + 22) / 45) % KOI_DIR_COUNT;
  return (KoiDir)bucket;
}
