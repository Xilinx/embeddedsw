/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law:
* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL FAULTS, AND
* XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS, IMPLIED,
* OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage of
* any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits, goodwill,
* or any type of  loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was  reasonably foreseeable
* or Xilinx had been advised of the  possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe  performance, such as life-support or
* safety devices or  systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any  other applications
* that could lead to death, personal  injury, or severe property or environmental
* damage  (individually and collectively, "Critical  Applications").
* Customer assumes the sole risk and liability of any use of Xilinx products in
* Critical  Applications, subject only to applicable laws and  regulations
* governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_io.c
*
* Contains I/O functions for memory-mapped architectures.  These functions
* encapsulate generic CPU I/O requirements.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a hbm  07/28/09 Initial release
* 3.00a hbm  07/21/10 Added Xil_EndianSwap32/16, Xil_Htonl/s, Xil_Ntohl/s
*
* </pre>
*
* @note
*
* This file may contain architecture-dependent code.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/***************** Macros (Inline Functions) and Functions Definitions *******/

/*****************************************************************************/
/**
*
* Perform an input operation for an 8-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
u8 Xil_In8(u32 Addr) {
	return *(volatile u8 *)Addr;
}

/*****************************************************************************/
/**
*
* Perform an input operation for a 16-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
u16 Xil_In16(u32 Addr) {
	return *(volatile u16 *)Addr;
}

/*****************************************************************************/
/**
*
* Performs an input operation for a 32-bit memory location by reading from the
* specified address and returning the Value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
u32 Xil_In32(u32 Addr) {
	return *(volatile u32 *)Addr;
}


/*****************************************************************************/
/**
*
* Perform an output operation for an 8-bit memory location by writing the
* specified value to the specified address.
*
* @param	Addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void Xil_Out8(u32 Addr, u8 Value) {
	u8 *LocalAddr = (u8 *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* Perform an output operation for a 16-bit memory location by writing the
* specified value to the specified address.
*
* @param	Addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void Xil_Out16(u32 Addr, u16 Value) {
	u16 *LocalAddr = (u16 *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* Perform an output operation for a 32-bit memory location by writing the
* specified value to the specified address.
*
* @param	addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void Xil_Out32(u32 Addr, u32 Value) {
	u32 *LocalAddr = (u32 *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* Perform a 16-bit endian converion.
*
* @param	Data contains the value to be converted.
*
* @return	converted value.
*
* @note		None.
*
******************************************************************************/
u16 Xil_EndianSwap16(u16 Data)
{
	return (u16) (((Data & 0xFF00U) >> 8U) | ((Data & 0x00FFU) << 8U));
}

/*****************************************************************************/
/**
*
* Perform a 32-bit endian converion.
*
* @param	Data contains the value to be converted.
*
* @return	converted value.
*
* @note		None.
*
******************************************************************************/
u32 Xil_EndianSwap32(u32 Data)
{
	u16 LoWord;
	u16 HiWord;

	/* get each of the half words from the 32 bit word */

	LoWord = (u16) (Data & 0x0000FFFFU);
	HiWord = (u16) ((Data & 0xFFFF0000U) >> 16U);

	/* byte swap each of the 16 bit half words */

	LoWord = (((LoWord & 0xFF00U) >> 8U) | ((LoWord & 0x00FFU) << 8U));
	HiWord = (((HiWord & 0xFF00U) >> 8U) | ((HiWord & 0x00FFU) << 8U));

	/* swap the half words before returning the value */

	return ((((u32)LoWord) << (u32)16U) | (u32)HiWord);
}

/*****************************************************************************/
/**
*
* Perform a little-endian input operation for a 16-bit memory location
* by reading from the specified address and returning the byte-swapped value
* read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address with the
*		proper endianness. The return value has the same endianness
*		as that of the processor, i.e. if the processor is big-engian,
*		the return value is the byte-swapped value read from the
*		address.
*
*
* @note		None.
*
******************************************************************************/
#ifndef __LITTLE_ENDIAN__
u16 Xil_In16LE(u32 Addr)
#else
u16 Xil_In16BE(u32 Addr)
#endif
{
	u16 Value;

	/* get the data then swap it */
	Value = Xil_In16(Addr);

	return Xil_EndianSwap16(Value);
}

/*****************************************************************************/
/**
*
* Perform a little-endian input operation for a 32-bit memory location
* by reading from the specified address and returning the byte-swapped value
* read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address with the
*		proper endianness. The return value has the same endianness
*		as that of the processor, i.e. if the processor is big-engian,
*		the return value is the byte-swapped value read from the
*		address.
*
* @note		None.
*
******************************************************************************/
#ifndef __LITTLE_ENDIAN__
u32 Xil_In32LE(u32 Addr)
#else
u32 Xil_In32BE(u32 Addr)
#endif
{
	u32 InValue;

	/* get the data then swap it */
	InValue = Xil_In32(Addr);
	return Xil_EndianSwap32(InValue);
}

/*****************************************************************************/
/**
*
* Perform a little-endian output operation for a 16-bit memory location by
* writing the specified value to the the specified address. The value is
* byte-swapped before being written.
*
* @param	Addr contains the address to perform the output operation at.
* @param	Value contains the value to be output at the specified address.
*		The value has the same endianness as that of the processor.
*		If the processor is big-endian, the byte-swapped value is
*		written to the address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifndef __LITTLE_ENDIAN__
void Xil_Out16LE(u32 Addr, u16 Value)
#else
void Xil_Out16BE(u32 Addr, u16 Value)
#endif
{
	u16 OutValue;

	/* swap the data then output it */
	OutValue = Xil_EndianSwap16(Value);

	Xil_Out16(Addr, OutValue);
}

/*****************************************************************************/
/**
*
* Perform a little-endian output operation for a 32-bit memory location
* by writing the specified value to the the specified address. The value is
* byte-swapped before being written.
*
* @param	Addr contains the address at which the output operation at.
* @param	Value contains the value to be output at the specified address.
*		The value has the same endianness as that of the processor.
*		If the processor is big-endian, the byte-swapped value is
*		written to the address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifndef __LITTLE_ENDIAN__
void Xil_Out32LE(u32 Addr, u32 Value)
#else
void Xil_Out32BE(u32 Addr, u32 Value)
#endif
{
	u32 OutValue;

	/* swap the data then output it */
	OutValue = Xil_EndianSwap32(Value);
	Xil_Out32(Addr, OutValue);
}
