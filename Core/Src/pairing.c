#include "pairing.h"

#define PAIRING_PG_COUNT 1
#define PAIRING_PG_END (FLASH_SIZE/FLASH_PAGE_SIZE)
#define PAIRING_PG_START (PAIRING_PG_END - PAIRING_PG_COUNT)

#define PAIRING_START_ADDR ((uint64_t*)(FLASH_BASE + (PAIRING_PG_START * FLASH_PAGE_SIZE)))
#define PAIRING_END_ADDR ((uint64_t*)(PAIRING_START_ADDR + (PAIRING_PG_COUNT * FLASH_PAGE_SIZE)))
#define PAIRING_WORD_SIZE 7

uint64_t* Pairing_Find(Pairing *p) {
  uint64_t* read = PAIRING_START_ADDR;
  p->idx = 0xff;

  uint64_t dw;
  while((((dw = *read) & 0xff) != 0xff) && (read < PAIRING_END_ADDR)) {
    if (dw == 0) {
      read += PAIRING_WORD_SIZE;
      continue;
    }

    p->idx = (dw & 0xff);
    read++;

    for (int i = 0; i < 9; i += 8) {
      dw = read[i >> 3];
      for (int j = 0; j < (APP_INFO_INSTANCE_UID_LEN/2); j++) {
        if (((uint8_t*)&dw)[j] != p->instance_uid[i+j]) {
          p->idx = 0xff;
          break;
        }
      }

      if(p->idx == 0xff) {
        break;
      }
    }

    if (p->idx == 0xff) {
      read += (PAIRING_WORD_SIZE - 1);
      continue;
    }

    return read;
  }

  return NULL;
}

uint8_t Pairing_Read(Pairing* out) {
  uint64_t* read = Pairing_Find(out);
  if (!read) {
    return 0;
  }

  read += (APP_INFO_INSTANCE_UID_LEN/8);

  for (int i = 0; i < (SHA256_DIGEST_LENGTH/8); i++) {
    ((uint64_t*) out->key)[i] = *(read++);
  }

  return 1;
}

uint8_t Pairing_Compact(uint32_t page) {
  uint64_t data[FLASH_PAGE_SIZE/8];
  uint32_t page_size = 0;
  uint64_t* p = (uint64_t*)(FLASH_BASE + (page * FLASH_PAGE_SIZE));

  for (int i = 0; i < (FLASH_PAGE_SIZE/8); i += PAIRING_WORD_SIZE) {
    if (p[i] == 0) {
      continue;
    }

    for (int j = 0; j < PAIRING_WORD_SIZE; j++) {
      data[page_size++] = p[i+j];
    }
  }

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.NbPages = 1;

  if (FLASH_PAGE_NB < page) {
    erase.Page = page + (256 - FLASH_PAGE_NB);
    erase.Banks = FLASH_BANK_2;  
  } else {
    erase.Page = page;
    erase.Banks = FLASH_BANK_1;
  }

  uint32_t err;
  if (HAL_FLASHEx_Erase(&erase, &err) != HAL_OK) {
    return 0;
  }

  for (int i = 0; i < page_size; i++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)&p[i], data[i]) != HAL_OK) {
      return 0;
    }
  }

  return 0;
}

uint8_t Pairing_Write(Pairing* in) {
  uint64_t* write = PAIRING_START_ADDR;
  uint64_t* first_invalid = NULL;

  while(write < PAIRING_END_ADDR) {
    if ((*write & 0xff) == 0xff) {
      if (HAL_FLASH_Unlock() != HAL_OK) {
        return 0;
      }

      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)write++, (0xa500 | in->idx)) != HAL_OK) {
        return 0;
      }

      for (int i = 0; i < (APP_INFO_INSTANCE_UID_LEN/8); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)&write[i], ((uint64_t*) in->instance_uid)[i]) != HAL_OK) {
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)write, 0);
          return 0;
        }
      }

      write += (APP_INFO_INSTANCE_UID_LEN/8);

      for (int i = 0; i < (SHA256_DIGEST_LENGTH/8); i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)&write[i], ((uint64_t*) in->key)[i]) != HAL_OK) {
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)(write - (APP_INFO_INSTANCE_UID_LEN/8)), 0);
          return 0;
        }
      }      

      if (HAL_FLASH_Lock()) {
        return 0;
      } 

      return 1;     
    } else if (!first_invalid && !*write) {
      first_invalid = write;
    }

    write += PAIRING_WORD_SIZE;
  }

  if (first_invalid) {
    if (!Pairing_Compact(((uint32_t)first_invalid) - FLASH_BASE)/FLASH_PAGE_SIZE) {
      return 0;
    }

    return Pairing_Write(in);
  }

  return 0;
}

uint8_t Pairing_Erase(Pairing* in) {
  uint64_t* erase = Pairing_Find(in);
  if (!erase) {
    return 0;
  }

  erase--;
  
  if (HAL_FLASH_Unlock() != HAL_OK) {
    return 0;
  }

  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)erase, 0) != HAL_OK) {
    return 0;
  }

  if (HAL_FLASH_Lock()) {
    return 0;
  }

  return 1;
}