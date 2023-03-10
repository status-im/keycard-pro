/*!
 * \file CM3_1T_AES_decrypt.S
 * \brief
 *
 *
 * \author Jan Oleksiewicz <jnk0le@hotmail.com>
 * Peter Schwabe & Ko Stoffelen @2016
 * \license SPDX-License-Identifier: MIT
 * \date 9 jun 2018
 */

.syntax unified
.thumb
.text

.align 3
// void CM3_1T_AES_decrypt(uint8_t* rk, const uint8_t* in, uint8_t* out, size_t rounds) {
.global CM3_1T_AES_decrypt
.type   CM3_1T_AES_decrypt,%function
CM3_1T_AES_decrypt:
#if __ARM_ARCH_7M__ || __ARM_ARCH_7EM__
	adds r0, #16 //to compare against before final round
	push {r0, r2, r4-r11, lr} //stack rk+16, out

	movw r14, #:lower16:AES_Td2
	movt r14, #:upper16:AES_Td2

	//rk_end = rk+16 + rounds * 16
	add r12, r0, r3, lsl #4

	//load input
	ldmia r1!, {r4-r7}

	//load initial round key
	ldmdb r12!, {r0-r3}

	//initial addroundkey
	eors r4, r0
	eors r5, r1
	eors r6, r2
	eors r7, r3

1:	uxtb r0, r4
	uxtb r1, r5
	uxtb r2, r6
	uxtb r3, r7

#ifdef __ARM_ARCH_7EM__
	// aggregate loads by source in case it lies in different memory blocks
	ldr r0, [r14, r0, lsl #2]
	ldr r1, [r14, r1, lsl #2]
	ldr r2, [r14, r2, lsl #2]
	ldr r3, [r14, r3, lsl #2]
	ldr r9, [r12, #-12]
	ldr r10, [r12, #-8]
	ldr r11, [r12, #-4]
	ldr r8, [r12, #-16]!
#else  // cm3 can't pre index anywhere but first load
	ldr r8, [r12, #-16]!
	ldr r0, [r14, r0, lsl #2]
	ldr r1, [r14, r1, lsl #2]
	ldr r2, [r14, r2, lsl #2]
	ldr r3, [r14, r3, lsl #2]
	ldr r9, [r12, #4]
	ldr r10, [r12, #8]
	ldr r11, [r12, #12]
#endif

	eor r8, r8, r0, ror #16
	eor r9, r9, r1, ror #16
	eor r10, r10, r2, ror #16
	eor r11, r11, r3, ror #16

	uxtb r0, r7, ror #8
	uxtb r1, r4, ror #8
	uxtb r2, r5, ror #8
	uxtb r3, r6, ror #8
	ldr r0, [r14, r0, lsl #2]
	ldr r1, [r14, r1, lsl #2]
	ldr r2, [r14, r2, lsl #2]
	ldr r3, [r14, r3, lsl #2]
	eor r8, r8, r0, ror #8
	eor r9, r9, r1, ror #8
	eor r10, r10, r2, ror #8
	eor r11, r11, r3, ror #8

	uxtb r0, r6, ror #16
	uxtb r1, r7, ror #16
	uxtb r2, r4, ror #16
	uxtb r3, r5, ror #16

	lsrs r5, #24
	lsrs r6, #24
	lsrs r7, #24
	lsrs r4, #24

	ldr r0, [r14, r0, lsl #2]
	ldr r1, [r14, r1, lsl #2]
	ldr r2, [r14, r2, lsl #2]
	ldr r3, [r14, r3, lsl #2]

	ldr r5, [r14, r5, lsl #2]
	ldr r6, [r14, r6, lsl #2]
	ldr r7, [r14, r7, lsl #2]
	ldr r4, [r14, r4, lsl #2]

	// change xoring order to writeback r4-r7 without extra moves
	eor r0, r0, r5, ror #24
	eor r1, r1, r6, ror #24

	// set flags early to optimize speculative fetches in cm3
	// cmp have to be close to branch, otherwise speculative code loads doesn't work
	ldr r5, [sp]
	cmp r5, r12

	eor r2, r2, r7, ror #24
	eor r3, r3, r4, ror #24

	eor r4, r8, r0
	eor r5, r9, r1
	eor r6, r10, r2
	eor r7, r11, r3

	bne.w 1b //align following code to 4 bytes

	// final round
	movw r14, #:lower16:AES_inv_sbox
	movt r14, #:upper16:AES_inv_sbox

	uxtb r0, r4
	uxtb r1, r5
	uxtb r2, r6
	uxtb r3, r7
	ldr r8, [r12, #-16]
	ldr r9, [r12, #-12]
	ldr r10, [r12, #-8]
	ldr r11, [r12, #-4]
	ldrb r0, [r14, r0]
	ldrb r1, [r14, r1]
	ldrb r2, [r14, r2]
	ldrb r3, [r14, r3]
	eor r8, r0
	eor r9, r1
	eor r10, r2
	eor r11, r3

	uxtb r0, r7, ror #8
	uxtb r1, r4, ror #8
	uxtb r2, r5, ror #8
	uxtb r3, r6, ror #8
	ldrb r0, [r14, r0]
	ldrb r1, [r14, r1]
	ldrb r2, [r14, r2]
	ldrb r3, [r14, r3]
	eor r8, r8, r0, lsl #8
	eor r9, r9, r1, lsl #8
	eor r10, r10, r2, lsl #8
	eor r11, r11, r3, lsl #8

	uxtb r0, r6, ror #16
	uxtb r1, r7, ror #16
	uxtb r2, r4, ror #16
	uxtb r3, r5, ror #16

	lsrs r5, #24
	lsrs r6, #24
	lsrs r7, #24

	uxtb r12, r4, ror #24

	ldr r4, [sp, #4] // load output pointer

	ldrb r0, [r14, r0]
	ldrb r1, [r14, r1]
	ldrb r2, [r14, r2]
	ldrb r3, [r14, r3]

	ldrb r5, [r14, r5]
	ldrb r6, [r14, r6]
	ldrb r7, [r14, r7]
	ldrb r12, [r14, r12]

	eor r8, r8, r0, lsl #16
	eor r9, r9, r1, lsl #16
	eor r10, r10, r2, lsl #16
	eor r11, r11, r3, lsl #16

	eor r0, r8, r5, lsl #24
	eor r1, r9, r6, lsl #24
	eor r2, r10, r7, lsl #24
	eor r3, r11, r12, lsl #24

	add sp, #8 //less mem pressure than preindexed load + dummy pop

	str r0, [r4, #0]
	str r1, [r4, #4]
	str r2, [r4, #8]
	str r3, [r4, #12]

	pop {r4-r11, pc}
#else
	b . // crash in case the function was called on thumb1 core
#endif
