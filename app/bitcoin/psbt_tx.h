
#ifndef PSBT_TX_H
#define PSBT_TX_H

#include <stdint.h>

typedef enum {
  PSBT_OK,
  PSBT_COMPACT_READ_ERROR,
  PSBT_READ_ERROR,
  PSBT_WRITE_ERROR,
  PSBT_INVALID_STATE,
  PSBT_NOT_IMPLEMENTED,
  PSBT_OOB_WRITE
} psbt_result_t;

typedef struct {
  unsigned char *txid;
  unsigned int index; /* output number referred to by above */
  unsigned char *script;
  unsigned int script_len;
  unsigned int sequence_number;
} psbt_txin_t;

typedef struct {
  uint64_t amount;
  unsigned char *script;
  unsigned int script_len;
} psbt_txout_t;

typedef struct {
  int input_index;
  int item_index;
  unsigned char *item;
  unsigned int item_len;
} psbt_witness_item_t;

typedef struct {
  unsigned int version;
  unsigned int lock_time;
} psbt_tx_t;

enum psbt_txelem_type {
  PSBT_TXELEM_TXIN,
  PSBT_TXELEM_TXOUT,
  PSBT_TXELEM_TX,
  PSBT_TXELEM_WITNESS_ITEM,
};

typedef struct {
  enum psbt_txelem_type elem_type;
  void *user_data;
  union {
    psbt_txin_t *txin;
    psbt_txout_t *txout;
    psbt_tx_t *tx;
    psbt_witness_item_t *witness_item;
  } elem;
} psbt_txelem_t;

typedef void (psbt_txelem_handler_t)(psbt_txelem_t *handler);

psbt_result_t psbt_btc_tx_parse(uint8_t *data, uint32_t data_size, void *user_data, psbt_txelem_handler_t *handler);

#endif /* PSBT_TX_H */
