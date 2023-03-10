/*!
 * \file CM7_1T_AES_decrypt.S
 * \brief
 *
 *
 * \author Jan Oleksiewicz <jnk0le@hotmail.com>
 * \license SPDX-License-Identifier: MIT
 * \date 17 jun 2018
 */

// uses backward expanded round key

.syntax unified
.thumb
.text
.section .ramfunc.$SRAM_ITC

.align 3
// void CM7_1T_AES_decrypt(uint8_t* rk, const uint8_t* in, uint8_t* out, size_t rounds) {
.global CM7_1T_AES_decrypt
.type   CM7_1T_AES_decrypt,%function
	nop // align loop entry
	nop // align loop entry
CM7_1T_AES_decrypt:
#if __ARM_ARCH_7EM__
	//pld here at +1 cycle?
	push {r2, r4-r11,lr} // stack out

	movw r14, #:lower16:AES_Td2
	add r11, r0, #16 // to compare against before final round

	movt r14, #:upper16:AES_Td2
	add r12, r11, r3, lsl #4 // rk_end = rk+16 + rounds * 16

	//load input
	ldmia.w r1!, {r4-r7}

	//load key
	ldmdb r12!, {r0-r3}

	//initial addroundkey in loop

	// global allocation
	// r11 - rk + 16
	// r12 - current rk ptr
	// r14 - Td2

	// r0 - s00 | s10 | s20 | s30
	// r1 - s01 | s11 | s21 | s31
	// r2 - s02 | s12 | s22 | s32
	// r3 - s03 | s13 | s23 | s33

	// rotation after Td2
	// xor   16 |  8  |  0  |  24

	// r4 - s00T ^ s13T ^ s22T ^ s31T
	// r5 - s01T ^ s10T ^ s23T ^ s32T
	// r6 - s02T ^ s11T ^ s20T ^ s33T
	// r7 - s03T ^ s12T ^ s21T ^ s30T

1:	// skew addroundkey to begginning of loop
	eors r0, r4
	eors r1, r5

	eors r2, r6
	eors r3, r7

	// start from col 0
	uxtb.w r4, r2, ror #16
	lsrs r5, r1, #24

	uxtb r6, r0
	ldr.w r10, [r14, r5, lsl #2]

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 - s01 | s11 | s21 |
	// r2 - s02 | s12 |     | s32
	// r3 - s03 | s13 | s23 | s33
	// r4 - s22 // c0
	// r5 -
	// r6 - s00 // c0 ror16
	// r7 -
	// r8 -
	// r9 -
	// r10 - s31T // c0 ror24

	lsrs r5, r2, #24
	ldr.w r4, [r14, r4, lsl #2]

	uxtb r7, r1
	ldr.w r9, [r14, r6, lsl #2]

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 | s13 | s23 | s33
	// r4 - s22T // c0
	// r5 - s32 // c1 ror24
	// r6 -
	// r7 - s01 // c1 ror16
	// r8 -
	// r9 - s00T // c0 ror16
	// r10 - s31T // c0 ror24

	uxtb.w r6, r3, ror #8
	ldr.w r8, [r14, r5, lsl #2]

	uxtb.w r5, r3, ror #16
	eor.w r4, r4, r10, ror #24

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 - s22T ^ s31T // c0
	// r5 - s23 // c1
	// r6 - s13 // c0 ror8
	// r7 - s01 // c1 ror16
	// r8 - s32T // c1 ror24
	// r9 - s00T // c0 ror16
	// r10 -

	eor.w r4, r4, r9, ror #16
	ldr.w r10, [r14, r7, lsl #2]

	uxtb.w r9, r0, ror #8
	ldr.w r5, [r14, r5, lsl #2]

	//current alloctaion
	// r0 -     |     | s20 | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 - s22T ^ s31T ^ s00T // c0
	// r5 - s23T // c1
	// r6 - s13 // c0 ror8
	// r7 -
	// r8 - s32T // c1 ror24
	// r9 - s10 // c1 ror8
	// r10 - s01T // c1 ror16

	uxtb.w r7, r0, ror #16
	ldr.w r6, [r14, r6, lsl #2]

	eor.w r5, r5, r10, ror #16
	ldr.w r9, [r14, r9, lsl #2]

	// r0 -     |     |     | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 - s22T ^ s31T ^ s00T // c0
	// r5 - s23 ^ s01T // c1
	// r6 - s13T // c0 ror8
	// r7 - s20 // c2
	// r8 - s32T // c1 ror24
	// r9 - s10T // c1 ror8
	// r10 -

	eor.w r4, r4, r6, ror #8
	ldr.w r10, [r14, r7, lsl #2]

	uxtb.w r7, r2, ror #8
	eor.w r5, r5, r8, ror #24

	// r0 -     |     |     | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 |     |     |
	// r3 - s03 |     |     | s33
	// r4 - col 0
	// r5 - s23 ^ s01T ^ s32T // c1
	// r6 -
	// r7 - s12 // c3 ror8
	// r8 -
	// r9 - s10T // c1 ror8
	// r10 - s20T // c2

	uxtb r2, r2
	eor.w r5, r5, r9, ror #8

	lsrs r0, #24
	ldr.w r8, [r14, r7, lsl #2]

	// r0 - s30 // c3 ror24
	// r1 -     | s11 | s21 |
	// r2 - s02 // c2 ror16
	// r3 - s03 |     |     | s33
	// r4 - col 0
	// r5 - col 1
	// r6 -
	// r7 -
	// r8 - s12T // c3 ror8
	// r9 -
	// r10 - s20T // c2

	uxtb.w r9, r1, ror #16
	ldr.w r2, [r14, r2, lsl #2]

	uxtb.w r1, r1, ror #8
	ldr.w r6, [r14, r0, lsl #2]

	// r0 -
	// r1 - s11 // c2 ror8
	// r2 - s02T // c2 ror16
	// r3 - s03 |     |     | s33
	// r4 - col 0
	// r5 - col 1
	// r6 - s30T // c3 ror24
	// r7 -
	// r8 - s12T // c3 ror8
	// r9 - s21 // c3
	// r10 - s20T // c2

	lsrs r0, r3, #24
	ldr.w r9, [r14, r9, lsl #2]

	uxtb r7, r3
	eor.w r2, r10, r2, ror #16

	// r0 - s33 // c2 ror24
	// r1 - s11 // c2 ror8
	// r2 - s20T ^ s02T // c2
	// r3 -
	// r4 - col 0
	// r5 - col 1
	// r6 - s30T // c3 ror24
	// r7 - s03 // c3 ror16
	// r8 - s12T // c3 ror8
	// r9 - s21T // c3
	// r10 -

	eor.w r9, r9, r6, ror #24
	ldr.w r10, [r14, r1, lsl #2]

	eor.w r9, r9, r8, ror #8
	ldr.w r6, [r14, r0, lsl #2]

	ldrd r0,r1, [r12, #-16]!

	// r0 - rk[0]
	// r1 - rk[1]
	// r2 - s20T ^ s02T // c2
	// r3 -
	// r4 - col 0
	// r5 - col 1
	// r6 - s33T // c2 ror24
	// r7 - s03 // c3 ror16
	// r8 -
	// r9 - s21T ^ s30T ^ s12T // c3
	// r10 - s11T // c2 ror8

	cmp r11, r12 // cmp early
	ldr.w r7, [r14, r7, lsl #2]

	eor.w r10, r2, r10, ror #8
	ldr r2, [r12, #8]

	eor.w r6, r10, r6, ror #24
	ldr r3, [r12, #12]

	eor.w r7, r9, r7, ror #16
	bne 1b

	eors r0, r4
	eors r1, r5

	eors r2, r6
	eors r3, r7

	// final round
	movw r14, #:lower16:AES_inv_sbox
	movt r14, #:upper16:AES_inv_sbox

	// r0 - s00 | s10 | s20 | s30
	// r1 - s01 | s11 | s21 | s31
	// r2 - s02 | s12 | s22 | s32
	// r3 - s03 | s13 | s23 | s33

	//final
	//    - s00`| s13`| s22`| s31`
	//    - s01`| s10`| s23`| s32`
	//    - s02`| s11`| s20`| s33`
	//    - s03`| s12`| s21`| s30`

	// start from col 0
	uxtb r4, r0
	lsrs r7, r1, #24

	uxtab r6, r14, r2, ror #16
	ldrb r10, [r14, r7]

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 - s01 | s11 | s21 |
	// r2 - s02 | s12 |     | s32
	// r3 - s03 | s13 | s23 | s33
	// r4 - s00 //r0c0
	// r5 -
	// r6 - s22 //r2c0
	// r7 -
	// r8 -
	// r9 -
	// r10 - s31` //r3c0
	// r11 -

	uxtab r7, r14, r3, ror #8
	ldrb r9, [r14, r4]

	lsrs r5, r2, #24
	ldrb r6, [r6]

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 - s01 | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     | s23 | s33
	// r4 -
	// r5 - s32 //r3c1
	// r6 - s22` //r2c0
	// r7 - s13 //r1c0
	// r8 -
	// r9 - s00` //r0c0
	// r10 - s31` //r3c0
	// r11 -

	uxtab r4, r14, r3, ror #16
	ldrb r7, [r7]

	orr.w r10, r6, r10, lsl #8
	ldrb r8, [r14, r5]

	//current alloctaion
	// r0 -     | s10 | s20 | s30
	// r1 - s01 | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 - s23 //r2c1
	// r5 -
	// r6 -
	// r7 - s13` //r1c0
	// r8 - s32` //r3c1
	// r9 - s00` //r0c0
	// r10 - s22` | s31` //c0 upper
	// r11 -

	uxtab r5, r14, r0, ror #8
	ldrb r4, [r4]

	uxtb r6, r1
	orr.w r9, r9, r7, lsl #8

	//current alloctaion
	// r0 -     |     | s20 | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 - s23` //r2c1
	// r5 - s10 //r1c1
	// r6 - s01 //r0c1
	// r7 -
	// r8 - s32` //r3c1
	// r9 - s00` | s13` //c0 bottom
	// r10 - s22` | s31` //c0 upper
	// r11 -

	orr.w r7, r9, r10, lsl #16
	ldrb r5, [r5]

	orr.w r8, r4, r8, lsl #8
	ldrb r10, [r14, r6]

	//current alloctaion
	// r0 -     |     | s20 | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 |     |     | s33
	// r4 -
	// r5 - s10` //r1c1
	// r6 -
	// r7 - col 0
	// r8 - s23` | s32` //c1 upper
	// r9 -
	// r10 - s01` //r0c1
	// r11 -

	uxtab r4, r14, r0, ror #16
	lsrs r6, r3, #24

	uxtb r3, r3
	orr.w r10, r10, r5, lsl #8

	//current alloctaion
	// r0 -     |     |     | s30
	// r1 -     | s11 | s21 |
	// r2 - s02 | s12 |     |
	// r3 - s03 //r0c3
	// r4 - s20 //r2c2
	// r5 -
	// r6 - s33 //r3c2
	// r7 - col 0
	// r8 - s23` | s32` //c1 upper
	// r9 -
	// r10 - s01` | s10` //c1 bottom
	// r11 -

	uxtb r5, r2
	ldrb r9, [r14, r6]

	uxtab r6, r14, r1, ror #8
	ldrb r4, [r4]

	//current alloctaion
	// r0 -     |     |     | s30
	// r1 -     |     | s21 |
	// r2 -     | s12 |     |
	// r3 - s03 //r0c3
	// r4 - s20` //r2c2
	// r5 - s02 //r0c2
	// r6 - s11 //r1c2
	// r7 - col 0
	// r8 - s23` | s32` //c1 upper
	// r9 - s33` //r3c2
	// r10 - s01` | s10` //c1 bottom
	// r11 -

	lsrs r0, #24
	ldrb r5, [r14, r5]

	uxtab r1, r14, r1, ror #16
	ldrb r6, [r6]

	//current alloctaion
	// r0 - s30 //r3c3
	// r1 - s21 //r2c3
	// r2 -     | s12 |     |
	// r3 - s03 //r0c3
	// r4 - s20` //r2c2
	// r5 - s02` //r0c2
	// r6 - s11` //r1c2
	// r7 - col 0
	// r8 - s23` | s32` //c1 upper
	// r9 - s33` //r3c2
	// r10 - s01` | s10` //c1 bottom
	// r11 -

	uxtab r2, r14, r2, ror #8
	ldrb.w r0, [r14, r0]

	orr.w r9, r4, r9, lsl #8
	ldrb r1, [r1]

	//current alloctaion
	// r0 - s30` //r3c3
	// r1 - s21` //r2c3
	// r2 - s12 //r1c3
	// r3 - s03 //r0c3
	// r4 -
	// r5 - s02` //r0c2
	// r6 - s11` //r1c2
	// r7 - col 0
	// r8 - s23` | s32` //c1 upper
	// r9 - s20` | s33` //c2 upper
	// r10 - s01` | s10` //c1 bottom
	// r11 -

	orr.w r11, r5, r6, lsl #8
	ldrb r2, [r2]

	orr.w r6, r10, r8, lsl #16
	ldrb r3, [r14, r3]

	//current alloctaion
	// r0 - s30` //r3c3
	// r1 - s21` //r2c3
	// r2 - s12` //r1c3
	// r3 - s03` //r0c3
	// r4 -
	// r5 -
	// r6 - col 1
	// r7 - col 0
	// r8 -
	// r9 - s20` | s33` //c2 upper
	// r10 -
	// r11 - s02` | s11` //c2 bottom

	orr.w r10, r1, r0, lsl #8
	ldr r4, [sp, #4] // pop early to pop even number of registers later

	orr.w r8, r3, r2, lsl #8
	ldr r5, [sp], #8 // load output pointer and clear stack

	ldmdb r12, {r0-r3}

	//current alloctaion
	// r0 - rk[0]
	// r1 - rk[1]
	// r2 - rk[2]
	// r3 - rk[3]
	// r4 - unstacked
	// r5 - out p
	// r6 - col 1
	// r7 - col 0
	// r8 - s03` | s12` //c3 bottom
	// r9 - s20` | s33` //c2 upper
	// r10 - s21` | s30` //c3 upper
	// r11 - s02` | s11` //c2 bottom

	eors r0, r7
	orr.w r7, r11, r9, lsl #16

	eors r1, r6
	orr.w r6, r8, r10, lsl #16

	eors r2, r7
	eors r3, r6

	stmia r5!, {r0-r3}

	pop {r5-r11, pc}
#else
	b . //crash in case the function was called on thumb1 core
#endif
