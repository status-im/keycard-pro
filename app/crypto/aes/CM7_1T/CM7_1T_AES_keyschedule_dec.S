/*!
 * \file CM7_1T_AES_keyschedule_dec.S
 * \brief
 *
 *
 * \author Jan Oleksiewicz <jnk0le@hotmail.com>
 * \license SPDX-License-Identifier: MIT
 * \date 17 jun 2018
 */

// performs perform equivalent inverse cipher transformation on expanded encryption key
// order of round keys is not inverted - decryption will read it in reverse

.syntax unified
.thumb
.text
.section .ramfunc.$SRAM_ITC

.align 3
// void CM7_1T_AES_keyschedule_dec(uint8_t* rk, size_t rounds) {
.global CM7_1T_AES_keyschedule_dec
.type   CM7_1T_AES_keyschedule_dec,%function
CM7_1T_AES_keyschedule_dec:
#if __ARM_ARCH_7EM__
	push {r4-r10, lr}

	//first and last block are ommited
	add r10, r0, #16
	add r0, r0, r1, lsl #4 //rk_end-16 = rk + rounds * 16

	movw r14, #:lower16:AES_Te2
	movt r14, #:upper16:AES_Te2
	movw r12, #:lower16:AES_Td2
	movt r12, #:upper16:AES_Td2

	nop // align loop entry, wasted cycle
	b 2f // skip skewed part

	//expand 2 columns in parallel to avoid stalls
	//expand backwards for better code density, redoing to forward needs care for stalls due to .w/.n instr combinations

	//Td[sbox[]] - 4 regs, cannot be dual issued with itself or other block
	//dsp invMixColums - 4+1 regs + few more insns, cannot be dual issued with itself or other block
	// r2 = S{1}
	// r5 = S{2}
	// r6 = S{4}
	// r7 = S{8}
	// r9 = tmp

	// r2 - S{9} = S{8} ^ S{1}
	// r5 - S{b} = S{9} ^ S{2}
	// r6 - S{d} = S{9} ^ S{4}
	// r7 - S{e} = S{8} ^ S{4} ^ S{2}

	// r2 = s0{e}^s1{b}^s2{d}^s3{9} | s1{e}^s2{b}^s3{d}^s0{9} | s2{e}^s3{b}^s0{d}^s1{9} | s3{e}^s0{b}^s1{d}^s2{9}

1:	eor r2, r7, r6, ror #16
	str.w r2, [r0, #4] // cant .n due to skip

2:	ldrd r1,r2, [r0, #-8]!

	uadd8 r5, r2, r2 // quad lsl #1
	and.w r3, r1, #0xff

	uxtb r8, r1, ror #16
	eor r9, r5, #0x1b1b1b1b

	sel r5, r9, r5 // S{2} // if uadd carried then take reduced byte
	ldrb r3, [r14, r3, lsl #2]

	uadd8 r6, r5, r5 // quad lsl #1
	ldrb r8, [r14, r8, lsl #2]

	uxtb r4, r1, ror #8
	eor r9, r6, #0x1b1b1b1b

	sel r6, r9, r6 // S{4}
	ldr r3, [r12, r3, lsl #2]

	uadd8 r7, r6, r6 // quad lsl #1
	ldrb r4, [r14, r4, lsl #2]

	eor r9, r7, #0x1b1b1b1b
	ldr r8, [r12, r8, lsl #2]

	sel r7, r9, r7 // S{8}
	lsr.w r1, r1, #24 // .n adds 1 loop invariant cycle

	eor.w r8, r8, r3, ror #16
	ldr r4, [r12, r4, lsl #2]

	eors r2, r7 // S{9}
	ldrb r1, [r14, r1, lsl #2]

	eors r7, r5
	eor.w r8, r8, r4, ror #8

	eors r7, r6 // S{e}
	cmp r10, r0

	eor.w r7, r7, r2, ror #24
	ldr r1, [r12, r1, lsl #2]

	eor.w r5, r2 // S{b}
	eor.w r6, r2 // S{d}

	eor.w r1, r8, r1, ror #24
	str r1, [r0]

	eor r7, r7, r5, ror #8
	bne 1b

	//skewed also at beggining of the loop
	eor r2, r7, r6, ror #16
	str.w r2, [r0, #4] // if everything is .w, +1 cycle here

	pop {r4-r10, pc}
#else
	b . //crash in case the function was called on non dsp core
#endif
