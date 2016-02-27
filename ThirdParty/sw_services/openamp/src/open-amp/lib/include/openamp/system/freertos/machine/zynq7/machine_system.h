/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MACHINE_SYSTEM_H
#define _MACHINE_SYSTEM_H

/* Memory barrier */
#if (defined(__CC_ARM))
#define MEM_BARRIER() __schedule_barrier()
#elif (defined(__GNUC__))
#define MEM_BARRIER() asm volatile("dsb" : : : "memory")
#else
#define MEM_BARRIER()
#endif

static inline unsigned int xchg(void* plock, unsigned int lockVal)
{
	volatile unsigned int tmpVal = 0;
	volatile unsigned int tmpVal1 = 0;

#ifdef __GNUC__

	asm (
		"1:                                \n\t"
		"LDREX  %[tmpVal], [%[plock]]      \n\t"
		"STREX  %[tmpVal1], %[lockVal], [%[plock]] \n\t"
		"CMP    %[tmpVal1], #0                     \n\t"
		"BNE    1b                         \n\t"
		"DMB                               \n\t"
		: [tmpVal] "=&r"(tmpVal)
		: [tmpVal1] "r" (tmpVal1), [lockVal] "r"(lockVal), [plock] "r"(plock)
		: "cc", "memory"
	);

#endif

	return tmpVal;
}


#endif				/* _MACHINE_SYSTEM_H */
