/*
 * Generated using zcbor version 0.8.1
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "btc_decode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

static bool decode_psbt(zcbor_state_t *state, struct zcbor_string *result);


static bool decode_psbt(
		zcbor_state_t *state, struct zcbor_string *result)
{
	zcbor_log("%s\r\n", __func__);

	bool tmp_result = (((zcbor_bstr_decode(state, (&(*result))))));

	if (!tmp_result) {
		zcbor_trace_file(state);
		zcbor_log("%s error: %s\r\n", __func__, zcbor_error_str(zcbor_peek_error(state)));
	} else {
		zcbor_log("%s success\r\n", __func__);
	}

	return tmp_result;
}



int cbor_decode_psbt(
		const uint8_t *payload, size_t payload_len,
		struct zcbor_string *result,
		size_t *payload_len_out)
{
	zcbor_state_t states[2];

	return zcbor_entry_function(payload, payload_len, (void *)result, payload_len_out, states,
		(zcbor_decoder_t *)decode_psbt, sizeof(states) / sizeof(zcbor_state_t), 1);
}
