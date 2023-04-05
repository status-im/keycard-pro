// Font structures from Adafruit_GFX (1.1 and later).

#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>

/// Font data stored PER GLYPH
typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} glyph_t;

/// Data stored for FONT AS A WHOLE
typedef struct {
  const uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  const glyph_t *glyph;  ///< Glyph array
  uint16_t first;   ///< ASCII extents (first char)
  uint16_t last;    ///< ASCII extents (last char)
  uint8_t baseline; ///< Baseline
  uint8_t yAdvance; ///< Newline distance (y axis)
} font_t;

extern const font_t free_sans_9pt;

#endif // _FONT_H_
