/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information  of Xilinx, Inc.
* and is protected under U.S. and  international copyright and other
* intellectual property  laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any  rights to the
* materials distributed herewith. Except as  otherwise provided in a valid
* license issued to you by  Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and
* (2) Xilinx shall not be liable (whether in contract or tort,  including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature  related to, arising under or in connection with these
* materials, including for any direct, or any indirect,  special, incidental,
* or consequential loss or damage  (including loss of data, profits,
* goodwill, or any type of  loss or damage suffered as a result of any
* action brought  by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the  possibility
* of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-  safe, or for use
* in any application requiring fail-safe  performance, such as life-support
* or safety devices or  systems, Class III medical devices, nuclear
* facilities,  applications related to the deployment of airbags, or any
* other applications that could lead to death, personal  injury, or severe
* property or environmental damage  (individually and collectively,
* "Critical  Applications"). Customer assumes the sole risk and  liability
* of any use of Xilinx products in Critical  Applications, subject only to
* applicable laws and  regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
* OF THIS FILE AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnandps8_onfi.c
*
* This file contains the implementation of ONFI specific functions.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	   Changes
* ----- ----   ----------  -----------------------------------------------
* 1.0   nm     05/06/2014  First release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xnandps8_onfi.h"
#include "xnandps8.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* This function calculates ONFI paramater page CRC.
*
* @param	Parambuf is a pointer to the ONFI paramater page buffer.
* @param	StartOff is the starting offset in buffer to calculate CRC.
* @param	Length is the number of bytes for which CRC is calculated.
*
* @return
*		CRC value.
* @note
*		None.
*
******************************************************************************/
u32 XNandPs8_OnfiParamPageCrc(u8 *ParamBuf, u32 StartOff, u32 Length)
{
	const u32 CrcInit = 0x4F4EU;
	const u32 Order = 16U;
	const u32 Polynom = 0x8005U;
	u32 i, j, c, Bit;
	u32 Crc = CrcInit;
	u32 DataIn;
	u32 DataByteCount = 0U;
	u32 CrcMask, CrcHighBit;

	CrcMask = ((u32)(((u32)1 << (Order - (u32)1)) -(u32)1) << (u32)1) | (u32)1;
	CrcHighBit = (u32)((u32)1 << (Order - (u32)1));
	/*
	 * CRC covers the data bytes between byte 0 and byte 253
	 * (ONFI 1.0, section 5.4.1.36)
	 */
	for(i = StartOff; i < Length; i++) {
		DataIn = ParamBuf[i];
		c = (u32)DataIn;
		DataByteCount++;
		for(j = 0x80U; j; j >>= 1U) {
			Bit = Crc & CrcHighBit;
			Crc <<= 1U;
			if ((c & j) != 0U) {
				Bit ^= CrcHighBit;
			}
			if (Bit != 0U) {
				Crc ^= Polynom;
			}
		}
		Crc &= CrcMask;
	}
	return Crc;
}
