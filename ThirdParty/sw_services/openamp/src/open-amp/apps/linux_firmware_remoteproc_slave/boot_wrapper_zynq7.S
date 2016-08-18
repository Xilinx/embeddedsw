@*
@* Copyright (c) 2014, Mentor Graphics Corporation
@* All rights reserved.
@*
@* Redistribution and use in source and binary forms, with or without
@* modification, are permitted provided that the following conditions are met:
@*
@* 1. Redistributions of source code must retain the above copyright notice,
@*	this list of conditions and the following disclaimer.
@* 2. Redistributions in binary form must reproduce the above copyright notice,
@*	this list of conditions and the following disclaimer in the documentation
@*	and/or other materials provided with the distribution.
@* 3. Neither the name of Mentor Graphics Corporation nor the names of its
@*	contributors may be used to endorse or promote products derived from this
@*	software without specific prior written permission.
@*
@* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
@* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
@* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
@* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
@* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
@* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
@* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
@* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
@* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
@* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
@* POSSIBILITY OF SUCH DAMAGE.



	.extern boot_linux
	.extern _bss_end
	.extern linux_kernel_start
	.extern dtb_start

	@ include FIT format linux image
	.section fit_image, "a"
	.incbin "image.ub";

	.section	bootwrapper, "ax"

	.code 32

BOOTSTRAP_BSS_End:
	.long _bss_end

	.global  bootwrapper_start
bootwrapper_start:

	MOV		r0, #0xdF

	@ Switch to SVC mode with IRQs and FIQs OFF
	MSR	 CPSR_cxsf, r0

	@ Disable MMU and caches

	MRC p15,#0,r0,c1,c0,#0
	BIC r0,r0,#0x00001000			   @ Clear I bit to disable instruction cache
	BIC r0,r0,#0x00000005			   @ Clear C and M bits (-C-M)

	@ Disable alignment checking
	BIC r0,r0,#0x00000002			   @ Clear alignement checking bit

	MCR p15,#0,r0,c1,c0,#0

	MRC p15,#0,r0,c1,c0,#2
	MOV r1,#0xF0
	LSL r1,r1,#16
	ORR r0,r0,r1
	MCR p15,#0,r0,c1,c0,#2


	MOV r0,#0x40000000
	FMXR FPEXC,r0


	@ Setup stack pointer for function calls
	LDR r0, BOOTSTRAP_BSS_End

	ADD r0,r0,#64512

	@ align stack pointer
	BIC r0,r0,#0x07
	MOV sp,r0

	@ branch to C linux boot function
	B boot_linux

	.code 32
	.global start_linux_with_dtb
	.type start_linux_with_dtb, %function
start_linux_with_dtb:

	@ Set registers as per Linux boot requirements
	@ For details see:
	@ https://www.kernel.org/doc/Documentation/arm/Booting

	MOV r0, #0   @ Set r0 =0

	LDR r1, =3343  @ Set r1 = machine type (Xilinx)

	LDR r4, =dtb_start

	LDR r2, [r4] @ Point r2 to DTB

	MOV r3, #0

	LDR r4, =linux_kernel_start

	LDR lr, [r4]

	MOV pc, lr @ Jump to start of Linux kernel binary
