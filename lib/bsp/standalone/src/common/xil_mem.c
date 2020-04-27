/******************************************************************************/
/**
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_mem.c
*
* This file contains xil mem copy function to use in case of word aligned
* data copies.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.1   nsk      11/07/16 First release.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_types.h"

/***************** Inline Functions Definitions ********************/
/*****************************************************************************/
/**
* @brief       This  function copies memory from once location to other.
*
* @param       dst: pointer pointing to destination memory
*
* @param       src: pointer pointing to source memory
*
* @param       cnt: 32 bit length of bytes to be copied
*
*****************************************************************************/
void Xil_MemCpy(void* dst, const void* src, u32 cnt)
{
	char *d = (char*)(void *)dst;
	const char *s = src;

	while (cnt >= sizeof (int)) {
		*(int*)d = *(int*)s;
		d += sizeof (int);
		s += sizeof (int);
		cnt -= sizeof (int);
	}
	while (cnt >= sizeof (u16)) {
		*(u16*)d = *(u16*)s;
		d += sizeof (u16);
		s += sizeof (u16);
		cnt -= sizeof (u16);
	}
	while ((cnt) > 0U){
		*d = *s;
		d += 1U;
		s += 1U;
		cnt -= 1U;
	}
}
