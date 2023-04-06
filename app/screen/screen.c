#include "app_tasks.h"
#include "screen.h"
#include "common.h"
#include "log/log.h"
#include "FreeRTOS.h"
#include "task.h"

#define SCREEN_TIMEOUT 100

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

struct _screen_camera_passthrough_ctx {
  uint8_t* fb;
  int y;
};

static struct _screen_camera_passthrough_ctx _cp_ctx;

APP_NOCACHE(uint16_t g_screen_fb[SCREEN_WIDTH], 2);

static void screen_signal() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(APP_TASK(ui), SCREEN_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void screen_camera_line() {
  if (_cp_ctx.y >= SCREEN_HEIGHT) {
    screen_signal();
    return;
  }

  for(int x = 0; x < SCREEN_WIDTH; x++) {
    uint8_t luma = *_cp_ctx.fb;
    _cp_ctx.fb += 2;
    g_screen_fb[x] = luma ? SCREEN_COLOR_BLACK : SCREEN_COLOR_WHITE;
  }

  _cp_ctx.fb += CAMERA_WIDTH;
  _cp_ctx.y++;

  screen_draw_pixels(g_screen_fb, SCREEN_WIDTH, screen_camera_line);
}

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  if (screen_set_drawing_window(&screen_fullarea) != HAL_OK) {
    return HAL_ERROR;
  }

  _cp_ctx.fb = (uint8_t*) fb;
  _cp_ctx.y = 0;

  screen_camera_line();

  return HAL_OK;
}

hal_err_t screen_fill_area(const screen_area_t* area, uint16_t color) {
  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  for(int x = 0; x < area->width; x++) {
    g_screen_fb[x] = color;
  }

  for(int y = 0; y < area->height; y++) {
    if (screen_draw_pixels(g_screen_fb, area->width, screen_signal) != HAL_OK) {
      return HAL_ERROR;
    }

    if (screen_wait() != HAL_OK) {
      return HAL_ERROR;
    }
  }

  return HAL_OK;
}

hal_err_t screen_draw_area(const screen_area_t* area, const uint16_t* pixels) {
  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  if (screen_draw_pixels(pixels, (area->width*area->height), screen_signal) != HAL_OK) {
    return HAL_ERROR;
  }

  return screen_wait();
}

const glyph_t *screen_lookup_glyph(const font_t* font, char c) {
  if (c < font->first || c > font->last) {
    c = 0x0a; // '*' for now, substitute with placeholder later
  } else {
    c -= font->first;
  }

  return &font->glyph[(int)c];
}

static inline hal_err_t _screen_char_flush(uint16_t* to_write, uint16_t threshold) {
  if (*to_write >= threshold) {
    if (screen_draw_pixels(g_screen_fb, *to_write, screen_signal) != HAL_OK) {
      return HAL_ERROR;
    }

    if (screen_wait()) {
      return HAL_ERROR;
    }

    *to_write = 0;
  }

  return HAL_OK;
}

hal_err_t screen_draw_glyph(const screen_text_ctx_t* ctx, const glyph_t* glyph) {
  screen_area_t area = { ctx->x, ctx->y, glyph->xAdvance, ctx->font->yAdvance};
  if (screen_set_drawing_window(&area) != HAL_OK) {
    return HAL_ERROR;
  }

  int y = 0;
  uint16_t to_write = 0;
  uint16_t ystart = ctx->font->baseline + glyph->yOffset;
  uint16_t yend = ystart + glyph->height;
  uint16_t xend = glyph->xOffset + glyph->width;
  uint16_t used_buf = (SCREEN_WIDTH / glyph->xAdvance) * glyph->xAdvance;

  while(y < ystart) {
    for(int x = 0; x < area.width; x++) {
      g_screen_fb[to_write++] = ctx->bg;
    }

    if (_screen_char_flush(&to_write, used_buf) != HAL_OK) {
      return HAL_ERROR;
    }

    y++;
  }

  const uint8_t* bitmap = &ctx->font->bitmap[glyph->bitmapOffset];
  uint8_t pixel = *(bitmap++);
  uint8_t pixcount = 0;

  while(y < yend) {
    int x = 0;

    for(; x < glyph->xOffset; x++) {
      g_screen_fb[to_write++] = ctx->bg;
    }

    for(; x < xend; x++) {
      g_screen_fb[to_write++] = (pixel & 0x80) ? ctx->fg : ctx->bg;
      pixel <<= 1;
      if (pixcount++ == 7) {
        pixcount = 0;
        pixel = *(bitmap++);
      }
    }

    for(; x < area.width; x++) {
      g_screen_fb[to_write++] = ctx->bg;
    }

    if (_screen_char_flush(&to_write, used_buf) != HAL_OK) {
      return HAL_ERROR;
    }

    y++;
  }

  while(y < area.height) {
    for(int x = 0; x < area.width; x++) {
      g_screen_fb[to_write++] = ctx->bg;
    }

    if (_screen_char_flush(&to_write, used_buf) != HAL_OK) {
      return HAL_ERROR;
    }

    y++;
  }

  return _screen_char_flush(&to_write, 1);
}

hal_err_t screen_draw_char(const screen_text_ctx_t* ctx, char c) {
  return screen_draw_glyph(ctx, screen_lookup_glyph(ctx->font, c));
}

hal_err_t screen_draw_string(screen_text_ctx_t* ctx, const char* str) {
  char c;

  while((c = *(str++))) {
    switch(c) {
    case '\n':
      ctx->y += ctx->font->yAdvance;
      ctx->x = ctx->xStart;
      break;
    case '\r':
      ctx->x = ctx->xStart;
      break;
    default: {
        const glyph_t* glyph = screen_lookup_glyph(ctx->font, c);
        if (screen_draw_char(ctx, c) != HAL_OK) {
          return HAL_ERROR;
        }
        ctx->x += glyph->xAdvance;
        break;
      }
    }
  }

  return HAL_OK;
}

hal_err_t screen_wait() {
  return ulTaskNotifyTakeIndexed(SCREEN_TASK_NOTIFICATION_IDX, pdTRUE, pdMS_TO_TICKS(SCREEN_TIMEOUT)) ? HAL_OK : HAL_ERROR;
}
