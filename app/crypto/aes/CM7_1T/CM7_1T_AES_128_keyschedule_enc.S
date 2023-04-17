/*!
 * \file CM7_1T_AES_128_keyschedule_enc.S
 * \brief
 *
 *
 * \author Jan Oleksiewicz <jnk0le@hotmail.com>
 * \license SPDX-License-Identifier: MIT
 * \date 17 Jun 2018
 */

// 10 rounds of rcon can be computed as left shift + conditional reload of rcon to 0x1b after 0x80
// it can also serve as loop counter to reduce register pressure

.syntax unified
.thumb
.text
.section .ramfunc.$SRAM_ITC

.align 3
// void CM7_1T_AES_128_keyschedule_enc(uint8_t *rk, const uint8_t *key) {
.global CM7_1T_AES_128_keyschedule_enc
.type   CM7_1T_AES_128_keyschedule_enc,%function
CM7_1T_AES_128_keyschedule_enc:
#if __ARM_ARCH_7EM__
	push {r4-r8, lr}

	//load key once
	ldmia.w r1, {r2-r5} // align loop entry

	// calculate rcon in highest byte to use a carry flag
	// use constructed immediate rather than shifted one for more issuable constants
	mov.w r1, #0x01010101
	str.w r2, [r0], #4 // just copy a keys

	movw r14, #:lower16:AES_Te2
	str.w r3, [r0], #4

	movt r14, #:upper16:AES_Te2
	str.w r4, [r0], #4 // r5 stored inside of the loop

1:	and.w r12, r5, #0xff
	eor r2, r2, r1, lsr #24 // rcon is in highest byte

	lsr.w r8, r5, #24
	lsls r1, #1

	uxtb r7, r5, ror #16
	ldrb r12, [r14, r12, lsl #2] // load sbox from Te2

	uxtb r6, r5, ror #8
	ldrb r8, [r14, r8, lsl #2] // load sbox from Te2

	// better than `it` instruction // works the same in younger and older slot
	// needs 2 cycle clearance from lsls, .n instruction within 3 cycles above
	bcc 2f
	mov r1, #0x1b1b1b1b
2:	ldrb r7, [r14, r7, lsl #2] // load sbox from Te2

	eor r2, r2, r12, lsl #24
	ldrb r6, [r14, r6, lsl #2] // load sbox from Te2

	eor r2, r2, r8, lsl #16
	str r5, [r0], #4 // from a previous loop or prologue

	eor r2, r2, r7, lsl #8
	cmp.w r1, #0x6c6c6c6c

	eor.w r2, r6
	str r2, [r0], #4

	eor.w r3, r2
	str r3, [r0], #4

	eor.w r4, r3
	str r4, [r0], #4

	eor.w r5, r4
	bne 1b
	str r5, [r0]

	pop {r4-r8, pc}
#else
	b . //crash in case the function was called on thumb1 core
#endif
