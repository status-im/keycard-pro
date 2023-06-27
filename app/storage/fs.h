#ifndef __FS_H
#define __FS_H

#include <stdint.h>
#include <stdbool.h>

#include "hal.h"

typedef struct __attribute__((packed)) {
  uint16_t magic;
  uint16_t len;
  uint8_t data[];
} fs_entry_t;

typedef bool (*fs_predicate_t)(void* ctx, fs_entry_t* entry);

fs_entry_t* fs_find(uint16_t magic, fs_predicate_t predicate, void* ctx);
app_err_t fs_write(fs_entry_t* first_entry, size_t count);
app_err_t fs_erase(fs_entry_t* entry);
app_err_t fs_erase_all(uint16_t magic, fs_predicate_t predicate, void* ctx);

#endif
