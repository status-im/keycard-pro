#include "app_tasks.h"
#include "screen.h"
#include "common.h"
#include "log/log.h"
#include "qrcode/qrcodegen.h"
#include "FreeRTOS.h"
#include "task.h"

#define SCREEN_TIMEOUT 100

#define CAM_OUT_WIDTH (CAMERA_WIDTH/2)
#define CAM_OUT_HEIGHT (CAMERA_HEIGHT/2)

#define MAX_GLYPHS_PER_LINE 50

const screen_area_t screen_fullarea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
const screen_area_t screen_camarea = { (SCREEN_WIDTH - CAM_OUT_WIDTH)/2, (SCREEN_HEIGHT - CAM_OUT_HEIGHT)/2, CAM_OUT_WIDTH, CAM_OUT_HEIGHT };

struct screen_render_ctx {
  const uint8_t* data;
  int y;
  int qr_size;
  int render_size;
  int scale;
  int y_repeat;
};

static struct screen_render_ctx g_screen_render_ctx;

APP_NOCACHE(uint16_t g_screen_fb[SCREEN_WIDTH], 2);

static void screen_signal() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(APP_TASK(ui), SCREEN_TASK_NOTIFICATION_IDX, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void screen_camera_line() {
  if (g_screen_render_ctx.y >= CAM_OUT_HEIGHT) {
    screen_signal();
    return;
  }

  for(int x = 0; x < CAM_OUT_WIDTH; x++) {
    uint8_t luma = *g_screen_render_ctx.data;
    g_screen_render_ctx.data += 2;
    g_screen_fb[x] = luma ? SCREEN_COLOR_BLACK : SCREEN_COLOR_WHITE;
  }

  g_screen_render_ctx.data += CAMERA_WIDTH;
  g_screen_render_ctx.y++;

  screen_draw_pixels(g_screen_fb, CAM_OUT_WIDTH, screen_camera_line);
}

hal_err_t screen_camera_passthrough(const uint8_t* fb) {
  if (screen_set_drawing_window(&screen_camarea) != HAL_OK) {
    return HAL_ERROR;
  }

  g_screen_render_ctx.data = (uint8_t*) fb;
  g_screen_render_ctx.y = 0;

  screen_camera_line();

  return HAL_OK;
}

static void screen_qr_line() {
  if (g_screen_render_ctx.y_repeat == 0) {
    if (g_screen_render_ctx.y >= g_screen_render_ctx.qr_size) {
      screen_signal();
      return;
    }

    int fb_x = 0;

    for(int x = 0; x < g_screen_render_ctx.qr_size; x++) {
      uint8_t module = qrcodegen_getModule(g_screen_render_ctx.data, x, g_screen_render_ctx.y);
      int x_repeat = g_screen_render_ctx.scale;
      while(x_repeat--) {
        g_screen_fb[fb_x++] = module ? SCREEN_COLOR_BLACK : SCREEN_COLOR_WHITE;
      }
    }

    g_screen_render_ctx.y++;
    g_screen_render_ctx.y_repeat = (g_screen_render_ctx.scale - 1);
  } else {
    g_screen_render_ctx.y_repeat--;
  }

  screen_draw_pixels(g_screen_fb, g_screen_render_ctx.render_size, screen_qr_line);
}

hal_err_t screen_draw_qrcode(const screen_area_t* area, const uint8_t* qrcode) {
  if (screen_set_drawing_window(area) != HAL_OK) {
    return HAL_ERROR;
  }

  g_screen_render_ctx.data = qrcode;
  g_screen_render_ctx.y = 0;
  g_screen_render_ctx.qr_size = qrcodegen_getSize(qrcode);
  g_screen_render_ctx.render_size = area->width;
  g_screen_render_ctx.scale =  g_screen_render_ctx.render_size / g_screen_render_ctx.qr_size;
  g_screen_render_ctx.y_repeat = 0;

  screen_qr_line();

  return screen_wait();
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

    if (screen_wait() != HAL_OK) {
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
    const glyph_t* glyph = screen_lookup_glyph(ctx->font, c);

    if (screen_draw_glyph(ctx, glyph) != HAL_OK) {
      return HAL_ERROR;
    }

    ctx->x += glyph->xAdvance;
  }

  return HAL_OK;
}

hal_err_t screen_draw_glyphs(screen_text_ctx_t* ctx, const glyph_t* glyphs[], size_t len) {
  for (int i = 0; i < len; i++) {
    if (screen_draw_glyph(ctx, glyphs[i]) != HAL_OK) {
      return HAL_ERROR;
    }

    ctx->x += glyphs[i]->xAdvance;
  }

  return HAL_OK;
}

hal_err_t screen_draw_text(screen_text_ctx_t* ctx, uint16_t max_x, uint16_t max_y, const uint8_t* text, size_t len) {
  uint16_t start_x = ctx->x;

  while(len) {
    // TODO: improve support for carriage return, for now since we break on newline we support skipping it at the beginning of a line only.
    if (text[0] == '\r') {
      text++;
      if (!--len) {
        break;
      }
    }

    size_t line_len = 0;
    size_t line_width = start_x;
    const glyph_t* line[MAX_GLYPHS_PER_LINE];

    size_t lim = MIN(MAX_GLYPHS_PER_LINE, len);
    uint8_t wrapped = 0;

    for (int i = 0; i < lim; i++) {
      char c = (char) text[i];
      if (c == '\n') {
        line_len = i;
        wrapped = 1;
        break;
      } else if (c == ' ') {
        line_len = i;
      }

      line[i] = screen_lookup_glyph(ctx->font, c);
      line_width += line[i]->xAdvance;
      if (line_width > max_x) {
        if (line_len == 0) {
          line_len = i - 1;
        }

        wrapped = 1;
        break;
      }
    }

    if (!wrapped) {
      line_len = lim;
    }

    if (screen_draw_glyphs(ctx, line, line_len) != HAL_OK) {
      return HAL_ERROR;
    }

    text += line_len;
    len -= line_len;

    ctx->x = start_x;
    ctx->y += ctx->font->yAdvance;

    if (ctx->y > max_y) {
      break;
    }
  }

  return HAL_OK;
}

hal_err_t screen_wait() {
  return ulTaskNotifyTakeIndexed(SCREEN_TASK_NOTIFICATION_IDX, pdTRUE, pdMS_TO_TICKS(SCREEN_TIMEOUT)) ? HAL_OK : HAL_ERROR;
}
