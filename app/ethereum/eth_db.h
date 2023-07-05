#ifndef _ETH_DB_H_
#define _ETH_DB_H_

#include "storage/fs.h"
#include "error.h"

typedef struct {
  uint32_t chain_id;
  const char *ticker;
  const char *name;
  const char *short_name;
} chain_desc_t;

typedef struct {
  uint32_t chain;
  const uint8_t* addr;
  const char *ticker;
  uint8_t decimals;
} erc20_desc_t;

app_err_t eth_db_lookup_chain(chain_desc_t* chain);
app_err_t eth_db_lookup_erc20(erc20_desc_t* erc20);

#endif