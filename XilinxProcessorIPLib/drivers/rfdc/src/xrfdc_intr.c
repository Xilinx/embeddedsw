/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xrfdc_intr.c
* @addtogroup rfdc_v3_2
* @{
*
* This file contains functions related to RFdc interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   sk     05/16/17 First release
* 2.1   sk     09/15/17 Remove Libmetal library dependency for MB.
*              09/18/17 Add API to clear the interrupts.
*       sk     09/21/17 Add support for Over voltage and Over
*                       Range interrupts.
* 2.2   sk     10/18/17 Add support for FIFO and DATA overflow interrupt
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrfdc.h"

/************************** Constant Definitions *****************************/
#ifdef __MICROBLAZE__
#define IRQ_HANDLED		1
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* This function sets the interrupt mask.
*
* @param	InstancePtr is a pointer to the XRFdc instance
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	IntrMask contains the interrupts to be enabled.
*			'1' enables an interrupt, and '0' disables.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XRFdc_IntrEnable(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 IsBlockAvail;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
							Block_Id);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			/* DAC */
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {

			ReadReg = XRFdc_ReadReg16(InstancePtr, 0x0,
									XRFDC_COMMON_INTR_ENABLE);
			if (Type == XRFDC_ADC_TILE) {
				ReadReg |= (1 << (Tile_Id + 4));
				XRFdc_WriteReg16(InstancePtr, 0x0,
							XRFDC_COMMON_INTR_ENABLE, ReadReg);
				BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_INTR_ENABLE);
				ReadReg |= (1 << Index);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_INTR_ENABLE, ReadReg);
				ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_EN(Index));
				if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
					ReadReg |= (XRFDC_ADC_OVR_VOLTAGE_MASK >> 24);
				}
				if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
					ReadReg |= (XRFDC_ADC_OVR_RANGE_MASK >> 24);
				}
				if ((IntrMask & XRFDC_ADC_FIFO_OVR_MASK) != 0U)
					ReadReg |=
					(XRFDC_ADC_FIFO_OVR_MASK >> 16);
				if ((IntrMask & XRFDC_ADC_DAT_OVR_MASK) != 0U)
					ReadReg |=
					(XRFDC_ADC_DAT_OVR_MASK >> 16);

				XRFdc_WriteReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_EN(Index), ReadReg);

				BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
						XRFDC_BLOCK_ADDR_OFFSET(Index);

				/* Check for FIFO interface interrupts */
				if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_FABRIC_IMR_OFFSET);
					ReadReg |= (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_FABRIC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_DEC_IMR_OFFSET);
					ReadReg |= (IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) >> 16;
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_DEC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET);
					ReadReg |= (IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) >> 4;
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET, ReadReg);
				}
			} else {
				ReadReg |= (1 << Tile_Id);
				XRFdc_WriteReg16(InstancePtr, 0x0,
									XRFDC_COMMON_INTR_ENABLE, ReadReg);
				BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_INTR_ENABLE);
				ReadReg |= (1 << Index);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_INTR_ENABLE, ReadReg);
				BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
						XRFDC_BLOCK_ADDR_OFFSET(Index);

				if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_DAC_FABRIC_IMR_OFFSET);
					ReadReg |= (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_DAC_FABRIC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
											XRFDC_DATPATH_IMR_OFFSET);
					ReadReg |= (IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) >> 4;
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_DATPATH_IMR_OFFSET, ReadReg);
				}
			}
		}
	}
	(void)BaseAddr;
RETURN_PATH:
	return;
}

/****************************************************************************/
/**
*
* This function clears the interrupt mask.
*
* @param	InstancePtr is a pointer to the XRFdc instance
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	IntrMask contains the interrupts to be disabled.
*			'1' disables an interrupt, and '0' remains no change.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XRFdc_IntrDisable(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 IsBlockAvail;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
							Block_Id);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			/* DAC */
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if (Type == XRFDC_ADC_TILE) {
				BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
				/* Check for Over Voltage and Over Range */
				ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_EN(Index));
				if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
					ReadReg &= ~(XRFDC_ADC_OVR_VOLTAGE_MASK >> 24);
				}
				if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
					ReadReg &= ~(XRFDC_ADC_OVR_RANGE_MASK >> 24);
				}
				if ((IntrMask & XRFDC_ADC_FIFO_OVR_MASK) != 0U)
					ReadReg &=
					~(XRFDC_ADC_FIFO_OVR_MASK >> 16);
				if ((IntrMask & XRFDC_ADC_DAT_OVR_MASK) != 0U)
					ReadReg &=
					~(XRFDC_ADC_DAT_OVR_MASK >> 16);
				XRFdc_WriteReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_EN(Index), ReadReg);
				BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
										XRFDC_BLOCK_ADDR_OFFSET(Index);
				/* Check for FIFO interface interrupts */
				if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_FABRIC_IMR_OFFSET);
					ReadReg &= ~(IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_FABRIC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_DEC_IMR_OFFSET);
					ReadReg &= ~((IntrMask &
								XRFDC_SUBADC_IXR_DCDR_MASK) >> 16);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_DEC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET);
					ReadReg &= ~((IntrMask &
								XRFDC_ADC_IXR_DATAPATH_MASK) >> 4);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET, ReadReg);
				}
			} else {
				if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_DAC_FABRIC_IMR_OFFSET);
					ReadReg &= ~(IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_DAC_FABRIC_IMR_OFFSET, ReadReg);
				}
				if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET);
					ReadReg &= ~((IntrMask &
								XRFDC_DAC_IXR_DATAPATH_MASK) >> 4);
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_DATPATH_IMR_OFFSET, ReadReg);
				}
			}
		}
	}
	(void)BaseAddr;
RETURN_PATH:
	return;
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR).
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return	A 32-bit value representing the contents of the Interrupt Status
* 			Registers (FIFO interface, Decoder interface, Data Path Interface)
*
* @note		None.
*
*****************************************************************************/
u32 XRFdc_GetIntrStatus(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Intrsts = 0;
	u32 IsBlockAvail;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
					(Type == XRFDC_ADC_TILE)) {
		if ((Block_Id == 2U) || (Block_Id == 3U))
			Block = 1U;
		if (Block_Id == 1U)
			Block = 0U;
	}
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
					Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Intrsts = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Check for Over Voltage and Over Range */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr,
							XRFDC_CONV_INTR_STS(Block_Id));
			Intrsts |= ((ReadReg & XRFDC_INTR_OVR_VOLTAGE_MASK) << 24);
			Intrsts |= ((ReadReg & XRFDC_INTR_OVR_RANGE_MASK) << 24);
			Intrsts |= ((ReadReg & XRFDC_INTR_FIFO_OVR_MASK) << 16);
			Intrsts |= ((ReadReg & XRFDC_INTR_DAT_OVR_MASK) << 16);

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
			/* Check for FIFO interface interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_FABRIC_ISR_OFFSET);
			Intrsts |= (ReadReg & XRFDC_IXR_FIFOUSRDAT_MASK);

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_DEC_ISR_OFFSET);
			Intrsts |= ((ReadReg & XRFDC_DEC_ISR_SUBADC_MASK) << 16);

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DATPATH_ISR_OFFSET);
			Intrsts |= ((ReadReg & XRFDC_ADC_DAT_PATH_ISR_MASK) << 4);
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_FABRIC_ISR_OFFSET);
			Intrsts |= (ReadReg & XRFDC_IXR_FIFOUSRDAT_MASK);

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DATPATH_ISR_OFFSET);
			Intrsts |= (ReadReg & XRFDC_DAC_DAT_PATH_ISR_MASK) << 4;
		}
	}
	(void)BaseAddr;
RETURN_PATH:
	return Intrsts;
}

/****************************************************************************/
/**
*
* This function clear the interrupts.
*
* @param	InstancePtr is a pointer to the XSysMonPsu instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	IntrMask contains the interrupts to be cleared.
*
* @return	A 32-bit value representing the contents of the Interrupt Status
* 			Registers (FIFO interface, Decoder interface, Data Path Interface)
*
* @note		None.
*
*****************************************************************************/
void XRFdc_IntrClr(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 IsBlockAvail;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
					(Type == XRFDC_ADC_TILE)) {
		if ((Block_Id == 2U) || (Block_Id == 3U))
			Block = 1U;
		if (Block_Id == 1U)
			Block = 0U;
	}
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
					Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_STS(Block_Id),
								(IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) >> 24);
			}
			if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr,
								XRFDC_CONV_INTR_STS(Block_Id),
								(IntrMask & XRFDC_ADC_OVR_RANGE_MASK) >> 24);
			}
			if ((IntrMask & XRFDC_ADC_FIFO_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr,
					XRFDC_CONV_INTR_STS(Block_Id),
				(IntrMask & XRFDC_ADC_FIFO_OVR_MASK) >> 16);
			}
			if ((IntrMask & XRFDC_ADC_DAT_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr,
					XRFDC_CONV_INTR_STS(Block_Id),
				(IntrMask & XRFDC_ADC_DAT_OVR_MASK) >> 16);
			}

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_FABRIC_ISR_OFFSET,
						(IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK));
			}
			if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_DEC_ISR_OFFSET,
						(u16)(IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) >> 16);
			}
			if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_DATPATH_ISR_OFFSET,
						(u16)(IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) >> 4);
			}
		} else {
			/* DAC */
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_FABRIC_ISR_OFFSET,
						(u16)(IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK));
			}
			if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_DATPATH_ISR_OFFSET,
						(u16)(IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) >> 4);
			}
		}
	}

	(void)BaseAddr;
RETURN_PATH:
	return;
}

/****************************************************************************/
/**
*
* This function is the interrupt handler for the driver.
* It must be connected to an interrupt system by the application such that it
* can be called when an interrupt occurs.
*
* @param	Vector is interrupt vector number. Libmetal status handler
*           expects two parameters in the handler prototype, hence
*           kept this parameter. This is not used inside
*           the interrupt handler API.
* @param	XRFdcPtr contains a pointer to the driver instance
*
* @return	None.
*
* @note		Vector param is not useful inside the interrupt handler, hence
*           typecast with void to remove compilation warning.
*
******************************************************************************/
int XRFdc_IntrHandler(int Vector, void * XRFdcPtr)
{
	XRFdc *InstancePtr = (XRFdc *)XRFdcPtr;
	u32 Intrsts;
	u32 Tile_Id = 0U;
	u32 Block_Id = 0U;
	u32 ReadReg;
	u16 Type = 0U;
	u32 BaseAddr;
	u32 IntrMask = 0x0U;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
#endif

	(void) Vector;
	/*
	 * Read the interrupt ID register to determine which
	 * interrupt is active
	 */
	ReadReg = XRFdc_ReadReg16(InstancePtr, 0x0,
								XRFDC_COMMON_INTR_STS);
	if ((ReadReg & XRFDC_EN_INTR_DAC_TILE0_MASK) != 0U) {
		Type = 1U;
		Tile_Id = 0U;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE1_MASK) != 0U) {
		Type = 1U;
		Tile_Id = 1U;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE2_MASK) != 0U) {
		Type = 1U;
		Tile_Id = 2U;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE3_MASK) != 0U) {
		Type = 1U;
		Tile_Id = 3U;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE0_MASK) != 0U) {
		Type = 0U;
		Tile_Id = 0U;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE1_MASK) != 0U) {
		Type = 0U;
		Tile_Id = 1U;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE2_MASK) != 0U) {
		Type = 0U;
		Tile_Id = 2U;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE3_MASK) != 0U) {
		Type = 0U;
		Tile_Id = 3U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_INTR_STS);
		if ((ReadReg & XRFDC_EN_INTR_SLICE0_MASK) != 0U) {
			Block_Id = 0;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE1_MASK) != 0U) {
			Block_Id = 1;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE2_MASK) != 0U) {
			Block_Id = 2;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE3_MASK) != 0U){
			Block_Id = 3;
		}

		Intrsts = XRFdc_GetIntrStatus(InstancePtr, 0U, Tile_Id, Block_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id)
							+ XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
		if (Intrsts & XRFDC_ADC_OVR_VOLTAGE_MASK) {
			IntrMask |= Intrsts & XRFDC_ADC_OVR_VOLTAGE_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC Over Voltage interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n ADC Over Voltage interrupt \r\n");
#endif
		}
		if (Intrsts & XRFDC_ADC_OVR_RANGE_MASK) {
			IntrMask |= Intrsts & XRFDC_ADC_OVR_RANGE_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC Over Range interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n ADC Over Range interrupt \r\n");
#endif
		}
		if (Intrsts & XRFDC_ADC_FIFO_OVR_MASK) {
			IntrMask |= Intrsts & XRFDC_ADC_FIFO_OVR_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"\n ADC FIFO OF interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG,
				"\n ADC FIFO OF interrupt \r\n");
#endif
		}
		if (Intrsts & XRFDC_ADC_DAT_OVR_MASK) {
			IntrMask |= Intrsts & XRFDC_ADC_DAT_OVR_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"\n ADC DATA OF interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG,
				"\n ADC DATA OF interrupt \r\n");
#endif
				}
		if ((Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC FIFO interface interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n ADC FIFO interface interrupt \r\n");
#endif
		}
		if ((Intrsts & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_SUBADC_IXR_DCDR_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC Decoder interface interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n ADC Decoder interface interrupt \r\n");
#endif
		}
		if ((Intrsts & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_ADC_IXR_DATAPATH_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC Data Path interface interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n ADC Data Path interface interrupt \r\n");
#endif
		}
	} else {
		/* DAC */
		BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_INTR_STS);
		if ((ReadReg & XRFDC_EN_INTR_SLICE0_MASK) != 0U) {
			Block_Id = 0;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE1_MASK) != 0U) {
			Block_Id = 1;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE2_MASK) != 0U) {
			Block_Id = 2;
		} else if ((ReadReg & XRFDC_EN_INTR_SLICE3_MASK) != 0U) {
			Block_Id = 3;
		}
		Intrsts = XRFdc_GetIntrStatus(InstancePtr, 1U, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id)
							+ XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
		if ((Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n DAC FIFO interface interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n DAC FIFO interface interrupt \r\n");
#endif
		}
		if ((Intrsts & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_DAC_IXR_DATAPATH_MASK;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_GENERAL, "\n DAC Data Path interface interrupt \r\n");
#else
			metal_log(METAL_LOG_DEBUG, "\n DAC Data Path interface interrupt \r\n");
#endif
		}
	}
	Block = Block_Id;
	(void)BaseAddr;

	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		if ((Block_Id == 0U) || (Block_Id == 1U)) {
			Block = 0U;
		} else if ((Block_Id == 2U) || (Block_Id == 3U)) {
			Block = 1U;
		}
	}
	InstancePtr->StatusHandler(InstancePtr->CallBackRef, Type, Tile_Id,
								Block, IntrMask);

	/* Clear the interrupt */
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		Intrsts = XRFdc_GetIntrStatus(InstancePtr, 0U, Tile_Id, Block_Id);
		XRFdc_IntrClr(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, Intrsts);

	} else {
		/* DAC */
		Intrsts = XRFdc_GetIntrStatus(InstancePtr, 1U, Tile_Id, Block_Id);
		XRFdc_IntrClr(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, Intrsts);
	}
#ifdef __MICROBLAZE__
	return IRQ_HANDLED;
#else
	return METAL_IRQ_HANDLED;
#endif
}

/*****************************************************************************/
/**
*
* This function sets the status callback function, the status handler, which the
* driver calls when it encounters conditions that should be reported to the
* higher layer software. The handler executes in an interrupt context, so
* the amount of processing should be minimized
*
*
* @param	InstancePtr is a pointer to the XRFdc instance.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
* @param	FunctionPtr is the pointer to the callback function.
*
* @return	None.
*
* @note
*
* The handler is called within interrupt context, so it should finish its
* work quickly.
*
******************************************************************************/
void XRFdc_SetStatusHandler(XRFdc *InstancePtr, void *CallBackRef,
				XRFdc_StatusHandler FunctionPtr)
{
#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XRFDC_COMPONENT_IS_READY);
#endif

	InstancePtr->StatusHandler = FunctionPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/** @} */
