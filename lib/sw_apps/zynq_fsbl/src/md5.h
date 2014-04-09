/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file md5.h
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ----- ---- -------- -------------------------------------------------------
* 5.00a sgd	05/17/13 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef ___MD5_H___
#define ___MD5_H___


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/

#define MD5_SIGNATURE_BYTE_SIZE	64

/**************************** Type Definitions *******************************/

typedef u8 boolean;
typedef u8 signature[ MD5_SIGNATURE_BYTE_SIZE ];

struct MD5Context
	{
	u32			buffer[ 4 ];
	u32			bits[ 2 ];
	signature	intermediate;
	};
typedef struct MD5Context MD5Context;

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * The four core functions - F1 is optimized somewhat
 */
#define F1( x, y, z ) ( z ^ ( x & ( y ^ z ) ) )
#define F2( x, y, z ) F1( z, x, y )
#define F3( x, y, z ) ( x ^ y ^ z )
#define F4( x, y, z ) ( y ^ ( x | ~z ) )


/*
 * This is the central step in the MD5 algorithm
 */
#define MD5_STEP( f, w, x, y, z, data, s ) \
	( w += f( x, y, z ) + data,  w = w << s | w >> ( 32 - s ),  w += x )


/************************** Function Prototypes ******************************/

void * MD5Memset( void *dest, int ch, u32 count );

void * MD5Memcpy( void *dest, const void *src, u32 count, boolean doByteSwap );

void MD5Transform( u32 *buffer, u32 *intermediate );

void MD5Init( MD5Context *context );

void MD5Update( MD5Context *context, u8 *buffer, u32 len, boolean doByteSwap );

void MD5Final( MD5Context *context, u8 *digest, boolean doByteSwap );

void md5( u8 *input, u32	len, u8 *digest, boolean doByteSwap );

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif


#endif /* ___MD5_H___ */
