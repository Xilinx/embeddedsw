/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
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

#include "metal/io.h"

void *metal_generic_memset_io(void *dst, int c, size_t size)
{
	void *retdst = dst;
	unsigned int cint = (unsigned char)c;
	unsigned int i;
	for (i = 1; i < sizeof(int); i++)
		cint |= (c << (8 * i));

	for (; size && ((uintptr_t)dst % sizeof(int)); dst++, size--)
		*(unsigned char volatile *)dst = (unsigned char) c;
	for(; size >= sizeof(int); dst += sizeof(int), size -= sizeof(int))
		*(unsigned int volatile *)dst = cint;
	for(; size != 0; dst++, size--)
		*(unsigned char volatile*)dst = (unsigned char) c;
	return retdst;
}

void *metal_generic_memcpy_io(void *dst, const void *src, size_t size)
{
	void *retdst = dst;
	while (size && (
		((uintptr_t)dst % sizeof(int)) ||
		((uintptr_t)src % sizeof(int)))) {
		*(unsigned char volatile *)dst =
			*(const unsigned char volatile *)src;
		dst++;
		src++;
		size--;
	}
	for (; size >= sizeof(int);
		dst += sizeof(int), src += sizeof(int), size -= sizeof(int))
		*(unsigned int volatile *)dst =
			*(const unsigned int volatile *)src;
	for (; size != 0; dst++, src++, size--)
		*(unsigned char volatile *)dst =
			*(const unsigned char volatile *)src;
	return retdst;
}

void *metal_memset_io(void *dst, int c, size_t size)
{
	return metal_generic_memset_io(dst, c, size);
}

void *metal_memcpy_io(void *dst, const void *src, size_t size)
{
	return metal_generic_memcpy_io(dst, src, size);
}
