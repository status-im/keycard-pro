#include "fs.h"

struct fs_find_ctx {
  uint16_t magic;
  fs_predicate_t predicate;
  void* ctx;
  fs_entry_t* found;
};

struct fs_write_ctx {
  size_t count;
  uint8_t* last_write;
  fs_entry_t* next_entry;
  app_err_t err;
};

enum fs_iterator_action {
  FS_ITER_END,
  FS_ITER_NEXT,
  FS_ITER_SKIP_PAGE
};

#define FS_MAGIC_FREE 0xffff
#define FS_MAGIC_PAD_MASK 0xc000
#define FS_MAGIC_PAD 0x8000

typedef enum fs_iterator_action (*fs_iterator_cb_t)(void* ctx, fs_entry_t* entry, size_t off);

static uint32_t* _fs_get_page(int idx) {
  return NULL;
}

static enum fs_iterator_action _fs_get_entry(void* ctx, fs_entry_t* entry, size_t off) {
  struct fs_find_ctx* find_ctx = (struct fs_find_ctx *) ctx;

  if (entry->magic == find_ctx->magic) {
    if (find_ctx->predicate(find_ctx->ctx, entry)) {
      find_ctx->found = entry;
      return FS_ITER_END;
    }
  } else if (entry->magic == FS_MAGIC_FREE) {
    return FS_ITER_SKIP_PAGE;
  }

  return FS_ITER_NEXT;
}

static void _fs_pad(uint8_t* addr) {
  if (addr == NULL) {
    return;
  }

  size_t pad = (HAL_FLASH_WORD_SIZE - (addr & (HAL_FLASH_WORD_SIZE - 1))) & (HAL_FLASH_WORD_SIZE - 1);

  while(pad) {
    //TODO: write (0x80 | pad--)
  }
}

static enum fs_iterator_action _fs_write_entry(void* ctx, fs_entry_t* entry, size_t off) {
  struct fs_write_ctx* write_ctx = (struct fs_write_ctx *) ctx;

  if (entry->magic == FS_MAGIC_FREE) {
    size_t total_entry_len = (write_ctx->next_entry->len + 4);
    if ((HAL_FLASH_BLOCK_SIZE - off) < total_entry_len) {
      _fs_pad(write_ctx->last_write);
      write_ctx->last_write = NULL;
      return FS_ITER_SKIP_PAGE;
    }

    //TODO: write entry

    write_ctx->last_write = ((uint8_t*) entry) + total_entry_len;

    if (--write_ctx->count == 0) {
      _fs_pad(write_ctx->last_write);
      return FS_ITER_END;
    } else {
      write_ctx->next_entry = (fs_entry_t*) (((uint8_t*) write_ctx->next_entry) + total_entry_len);
    }
  }

  return FS_ITER_NEXT;
}

void _fs_iterate(fs_iterator_cb_t cb, void *ctx) {
  for(int i = 0; i < HAL_FLASH_BLOCK_COUNT; i++) {
    uint32_t *p = _fs_get_page(i);

    size_t off = 0;

    while (off <= (HAL_FLASH_BLOCK_SIZE - 4)) {
      fs_entry_t* entry = (fs_entry_t*)&p[off];
      if ((entry->magic & FS_MAGIC_PAD_MASK) == FS_MAGIC_PAD) {
        off += (entry->magic & ~FS_MAGIC_PAD_MASK) >> 8;
        continue;
      }

      switch(cb(ctx, entry, off)) {
      case FS_ITER_END:
        return;
      case FS_ITER_NEXT:
        off += entry->len;
        break;
      case FS_ITER_SKIP_PAGE:
        off = HAL_FLASH_BLOCK_SIZE;
        break;
      }

      break;
    }
  }
}

fs_entry_t* fs_find(uint16_t magic, fs_predicate_t predicate, void* ctx) {
  struct fs_find_ctx finder_ctx = { .magic = magic, .predicate = predicate, .ctx = ctx, .found = NULL };
  _fs_iterate(_fs_get_entry, &finder_ctx);
  return finder_ctx.found;
}

app_err_t fs_write(fs_entry_t* first_entry, size_t count) {
  if (hal_flash_begin_program() != HAL_SUCCESS) {
    return ERR_HW;
  }

  struct fs_write_ctx write_ctx = { .count = count, .last_write = NULL, .next_entry = first_entry, .err = ERR_OK };
  _fs_iterate(_fs_write_entry, &write_ctx);

  hal_flash_end_program();

  return write_ctx.count ? ERR_FULL : write_ctx.err;
}

app_err_t fs_erase(fs_entry_t* entry) {
  return ERR_HW;
}

app_err_t fs_erase_all(uint16_t magic, fs_predicate_t predicate, void* ctx) {
  return ERR_HW;
}
