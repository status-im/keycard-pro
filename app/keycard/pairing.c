#include <string.h>

#include "pairing.h"
#include "hal.h"

#define PAIRING_BK_COUNT 1
#define PAIRING_BK_END HAL_FLASH_BLOCK_COUNT
#define PAIRING_BK_START (PAIRING_BK_END - PAIRING_BK_COUNT)

#define PAIRING_START_ADDR ((uint32_t*) HAL_FLASH_BLOCK_ADDR(PAIRING_BK_START))
#define PAIRING_END_ADDR ((uint32_t*) HAL_FLASH_BLOCK_ADDR(PAIRING_BK_END))
#define PAIRING_WORD_SIZE (((sizeof(pairing_t)/HAL_FLASH_WORD_SIZE) + 1) * (HAL_FLASH_WORD_SIZE / 4))

static uint32_t* pairing_find(pairing_t *p) {
  uint32_t* read = PAIRING_START_ADDR;
  p->idx = 0xff;

  uint32_t dw;
  while((((dw = *read) & 0xff) != 0xff) && (read < PAIRING_END_ADDR)) {
    if (dw == 0) {
      read += PAIRING_WORD_SIZE;
      continue;
    }

    p->idx = (dw & 0xff);
    read += (HAL_FLASH_WORD_SIZE / 4);

    for (int i = 0; i < (APP_INFO_INSTANCE_UID_LEN/4); i++) {
      dw = read[i];

      if (dw != ((uint32_t *) p->instance_uid)[i]) {
        p->idx = 0xff;
        break;
      }
    }

    if (p->idx == 0xff) {
      read += (PAIRING_WORD_SIZE - (HAL_FLASH_WORD_SIZE / 4));
      continue;
    }

    return read;
  }

  return NULL;
}

static app_err_t pairing_compact(uint32_t block) {
  //TODO: data is too big for the stack, move it
  uint32_t data[HAL_FLASH_BLOCK_SIZE/4];
  uint32_t data_len = 0;
  uint32_t* p = (uint32_t *) HAL_FLASH_BLOCK_ADDR(block);

  for (int i = 0; i < (HAL_FLASH_BLOCK_SIZE/4); i += PAIRING_WORD_SIZE) {
    if (p[i] == 0) {
      continue;
    }

    for (int j = 0; j < PAIRING_WORD_SIZE; j++) {
      data[data_len++] = p[i+j];
    }
  }

  if (hal_flash_erase(block) != HAL_SUCCESS) {
    return ERR_HW;
  }

  for (int i = 0; i < data_len; i += (HAL_FLASH_WORD_SIZE/4)) {
    if (hal_flash_program(&data[i], &p[i]) != HAL_SUCCESS) {
      return ERR_HW;
    }
  }

  return ERR_OK;
}

uint8_t pairing_read(pairing_t* out) {
  uint32_t* read = pairing_find(out);
  if (!read) {
    return ERR_DATA;
  }

  read += (APP_INFO_INSTANCE_UID_LEN/4);

  for (int i = 0; i < (SHA256_DIGEST_LENGTH/4); i++) {
    ((uint32_t*) out->key)[i] = *(read++);
  }

  return ERR_OK;
}

app_err_t pairing_write(pairing_t* in) {
  uint32_t* write = PAIRING_START_ADDR;
  uint32_t* first_invalid = NULL;

  while(write < PAIRING_END_ADDR) {
    if ((*write & 0xff) == 0xff) {
      if (hal_flash_begin_program() != HAL_SUCCESS) {
        return ERR_HW;
      }

      uint32_t idx_val[HAL_FLASH_WORD_SIZE/4];
      memset(idx_val, 0xa5, HAL_FLASH_WORD_SIZE);
      ((uint8_t*)idx_val)[0] = in->idx;

      if (hal_flash_program(idx_val, write) != HAL_SUCCESS) {
        return ERR_HW;
      }

      write += (HAL_FLASH_WORD_SIZE/4);

      for (int i = 0; i < (APP_INFO_INSTANCE_UID_LEN/4); i += HAL_FLASH_WORD_SIZE/4) {
        if (hal_flash_program(&((uint32_t *) in->instance_uid)[i], &write[i]) != HAL_SUCCESS) {
          return ERR_HW;
        }
      }

      write += (APP_INFO_INSTANCE_UID_LEN/4);

      for (int i = 0; i < (SHA256_DIGEST_LENGTH/4); i += HAL_FLASH_WORD_SIZE/4) {
        if (hal_flash_program(&((uint32_t *) in->key)[i], &write[i]) != HAL_SUCCESS) {
          return ERR_HW;
        }
      }      

      if (hal_flash_end_program() != HAL_SUCCESS) {
        return ERR_HW;
      } 

      return ERR_OK;
    } else if (!first_invalid && !*write) {
      first_invalid = write;
    }

    write += PAIRING_WORD_SIZE;
  }

  if (first_invalid) {
    if (pairing_compact(HAL_FLASH_ADDR_TO_BLOCK((uint32_t) first_invalid)) != ERR_OK) {
      return ERR_HW;
    }

    return pairing_write(in);
  }

  return ERR_DATA;
}

app_err_t pairing_erase(pairing_t* in) {
  uint32_t* erase = pairing_find(in);
  if (!erase) {
    return ERR_DATA;
  }

  erase -= (HAL_FLASH_WORD_SIZE / 4);
  
  if (hal_flash_begin_program() != HAL_SUCCESS) {
    return ERR_HW;
  }

  uint32_t erase_val[HAL_FLASH_WORD_SIZE/4];
  memset(erase_val, 0, HAL_FLASH_WORD_SIZE);

  if (hal_flash_program(erase_val, erase) != HAL_SUCCESS) {
    return ERR_HW;
  }

  if (hal_flash_end_program() != HAL_SUCCESS) {
    return ERR_HW;
  }

  return ERR_OK;
}
