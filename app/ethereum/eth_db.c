#include "eth_db.h"
#include <string.h>

#define FS_PAIRING_CHAIN 0x4348
#define FS_PAIRING_ERC20 0x3020

#define ERC20_NET_LEN 24

struct __attribute__((packed)) chain_raw_desc {
  fs_entry_t _entry;
  uint32_t chain_id;
  char data[];
};

struct __attribute__((packed)) erc20_raw_desc {
  fs_entry_t _entry;
  uint8_t net_count;
  uint8_t data[];
};

fs_action_t _eth_db_match_chain(void* ctx, fs_entry_t* entry) {
  if (entry->magic != FS_PAIRING_CHAIN) {
    return FS_REJECT;
  }

  chain_desc_t* chain = (chain_desc_t*) ctx;
  struct chain_raw_desc* chain_data = (struct chain_raw_desc*) entry;

  return chain_data->chain_id == chain->chain_id ? FS_ACCEPT : FS_REJECT;
}

fs_action_t _eth_db_match_erc20(void* ctx, fs_entry_t* entry) {
  if (entry->magic != FS_PAIRING_ERC20) {
    return FS_REJECT;
  }

  erc20_desc_t* erc20 = (erc20_desc_t*) ctx;
  struct erc20_raw_desc* erc20_data = (struct erc20_raw_desc*) entry;

  int count = erc20_data->net_count;
  uint8_t* data = erc20_data->data;

  while (count--) {
    if (!memcmp(&erc20->chain, data, 4) && !memcmp(erc20->addr, &data[4], 20)) {
      return FS_ACCEPT;
    }

    data += ERC20_NET_LEN;
  }

  return FS_REJECT;
}

fs_action_t _eth_db_match_all(void* ctx, fs_entry_t* entry) {
  return ((entry->magic == FS_PAIRING_CHAIN) || (entry->magic == FS_PAIRING_ERC20)) ? FS_REJECT : FS_ACCEPT;
}

app_err_t eth_db_lookup_chain(chain_desc_t* chain) {
  struct chain_raw_desc* chain_data = (struct chain_raw_desc*) fs_find(_eth_db_match_chain, chain);

  if (!chain_data) {
    return ERR_DATA;
  }

  chain->ticker = chain_data->data;
  chain->name = chain->ticker + strlen(chain->ticker) + 1;
  chain->short_name = chain->name + strlen(chain->name) + 1;

  return ERR_OK;
}

app_err_t eth_db_lookup_erc20(erc20_desc_t* erc20) {
  struct erc20_raw_desc* erc20_data = (struct erc20_raw_desc*) fs_find(_eth_db_match_erc20, erc20);

  if (!erc20_data) {
    return ERR_DATA;
  }

  uint8_t* data = erc20_data->data + (erc20_data->net_count * ERC20_NET_LEN);
  erc20->decimals = *(data++);
  erc20->ticker = (char *) data;

  return ERR_OK;
}

app_err_t eth_db_update(fs_entry_t* entries, size_t len) {
  app_err_t err = fs_erase_all(_eth_db_match_all, NULL);

  // since our matcher doesn't know when it has reached
  // completion, if everything went OK the error code
  // will be ERR_DATA on success.
  if (err != ERR_DATA) {
    return err;
  }

  return fs_write(entries, len);
}
