/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file si570drv.c
 *
 * This file contains low-level driver functions for controlling the
 * SiliconLabs Si570 clock generator as mounted on the ZCU106 demo board.
 * The user should refer to the hardware device specification for more details
 * of the device operation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date         Changes
 * ----- --- ----------   -----------------------------------------------
 * 1.0   ssh 07/05/2018	  Initial release.
 * </pre>
 *
 ****************************************************************************/

#include "si570drv.h"
#include <stdlib.h>
#include "xil_types.h"
#include "xiic.h"
#include "xparameters.h"

/******************************************************************************
 * Definitions independent on the specific board design. Should not be changed.
 *****************************************************************************/
#define PHASE_WORD_BYTE_6	7
#define PHASE_WORD_BYTE_8	9
#define PHASE_WORD_BYTE_10	11
#define MAX_REGISTERS		18

/*****************************************************************************/
/**
 * Send a list of register settings to the Si570 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress1 contains the 7 bit IIC address of the Si570 device.
 * @param	BufPtr is a pointer to an array with alternating register addresses
 *		and register values to program into the Si570. The array length
 *		must be at least 2*NumRegs.
 * @param	NumRegs contains the number of registers to write.
 *
 * @return	SI570_SUCCESS for success
 *
 * @return	SI570_ERR_IIC for IIC access failure,
 *
 * @return	SI570_ERR_FREQ when the requested frequency cannot be generated
 *
 * @return  SI570_ERR_PARM when the ClkSrc or ClkDest parameters are invalid
 *		or the ClkInFreq or ClkOutFreq are out of range.
 *
 * @note	Private function. Does not modify the contents of the buffer
 *		pointed to by BufPtr.
 *****************************************************************************/
int Si570_DoSettings(u32 IICBaseAddress, u8 IICAddress1, u8 *BufPtr,
		     int NumRegs) {
	int result;
	int i;

	/* Check the number of registers to write. It must be at least one. */
	if (NumRegs < 1) {
		if (SI570_DEBUG) {
			xil_printf("Si570: ERROR: Illegal number of registers write.");
		}
		return SI570_ERR_PARM;
	}
	for (i = 0; i < NumRegs; i++) {
		result = XIic_Send(IICBaseAddress, IICAddress1,
				   BufPtr + (i << 1), 2, XIIC_STOP);
		if (result != 2) {
			if (SI570_DEBUG) {
				xil_printf("Si570: ERROR: IIC write request error.");
			}
			return SI570_ERR_IIC;
		}
	}
	return SI570_SUCCESS;
}

/*****************************************************************************/
/**
 * Set the output frequency of the Si570 clock generator.
 *
 * @param	IICBaseAddress contains the base address of the IIC master
 *		device.
 * @param	IICAddress1 contains the 7 bit IIC address of the Si570 device.
 *
 * @param	RxRefClk contains the value to be written in Si570 register for
 *		that particular clock frequency.
 *
 * @return	SI570_SUCCESS for success
 *
 * @return	SI570_ERR_IIC for IIC access failure,
 *
 * @return	SI570_ERR_FREQ when the requested frequency cannot be generated
 *
 * @return	SI570_ERR_PARM when the ClkSrc or ClkDest parameters are invalid
 *		or the ClkInFreq or ClkOutFreq are out of range.
 *****************************************************************************/
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1, u32 RxRefClk)
{

	int result;
	u8 buf[] = {137, 0x10, 7, 0x01, 8, 0xC2, 9, 0x99, 10, 0x46, 11, 0xFF,
		    12, 0xC4, 137, 0x00, 135, 0x40};

	/* Set the clock settings */
	if (SI570_DEBUG) {
		xil_printf("Si570: Programming frequency settings.\n");
	}

	/* Free running mode or use a reference clock */
	switch (RxRefClk) {
	case FREQ_SI570_148_5_MHz:
		buf[PHASE_WORD_BYTE_6] = 0x99;
		break;
	case FREQ_SI570_148_35_MHz:
		buf[PHASE_WORD_BYTE_6] = 0x98;
		break;
	default:
		buf[PHASE_WORD_BYTE_6] = 0x00;
		break;
	}

	switch (RxRefClk) {
	case FREQ_SI570_148_5_MHz:
		buf[PHASE_WORD_BYTE_8] = 0x46;
		break;
	case FREQ_SI570_148_35_MHz:
		buf[PHASE_WORD_BYTE_8] = 0x9A;
		break;
	default:
		buf[PHASE_WORD_BYTE_8] = 0x00;
		break;
	}

	switch (RxRefClk) {
	case FREQ_SI570_148_5_MHz:
		buf[PHASE_WORD_BYTE_10] = 0xFF;
		break;
	case FREQ_SI570_148_35_MHz:
		buf[PHASE_WORD_BYTE_10] = 0xFF;
		break;
	default:
		buf[PHASE_WORD_BYTE_10] = 0xFF;
		break;
	}

	/* Send all register settings to the Si570 */
	result = Si570_DoSettings(IICBaseAddress, IICAddress1,
				  buf, MAX_REGISTERS / 2);
	return result;
}
