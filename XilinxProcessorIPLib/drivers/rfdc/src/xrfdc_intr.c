/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_intr.c
* @addtogroup rfdc_v8_1
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
* 5.0   sk     08/24/18 Reorganize the code to improve readability and
*                       optimization.
* 5.1   cog    01/29/19 Replace structure reference ADC checks with
*                       function.
* 6.0   cog    02/20/19	Added handling for new ADC common mode over/under
*                       voltage interrupts.
*       cog    02/20/19	XRFdc_GetIntrStatus now populates a pointer with the
*                       status and returns an error code.
*       cog	   02/20/19	XRFdc_IntrClr, XRFdc_IntrDisable and XRFdc_IntrEnable
*                       now return error codes.
*       cog    03/25/19 The new common mode over/under voltage interrupts mask
*                       bits were clashing with other interrupt bits.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    05/13/19 Re-factor of interrupt clear/status handling.
*       cog    05/13/19 Added handling for common power up interrupt.
*       cog    07/29/19 Added XRFdc_GetEnabledInterrupts() API.
*       cog    08/02/19 Formatting changes.
* 7.1   aad    12/01/19 Fixed static analysis errors.
*              12/20/19 Metal log messages are now more descriptive.
*       cog    01/29/20 Fixed metal log typos.
* 8.0   cog    02/10/20 Updated addtogroup.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Added observation FIFO interrupts.
*       cog    08/28/20 Only error out if both analogue and digital paths
*                       are disabled.
*       cog    09/28/20 Added more DAC interrupts and fixed issue with
*                       GetEnabledInterrupts.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xrfdc.h"

/************************** Constant Definitions *****************************/

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
* @param    InstancePtr is a pointer to the XRFdc instance
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    IntrMask contains the interrupts to be enabled.
*           '1' enables an interrupt, and '0' disables.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not available.
*
* @note     None.
*
*****************************************************************************/
u32 XRFdc_IntrEnable(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Index;
	u32 NoOfBlocks;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	ReadReg = XRFdc_ReadReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_ENABLE);
	if ((IntrMask & XRFDC_COMMON_MASK) != 0U) {
		ReadReg |= (XRFDC_COMMON_MASK >> XRFDC_COMMON_SHIFT);
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_ENABLE, ReadReg);
		if ((IntrMask & ~XRFDC_COMMON_MASK) == XRFDC_DISABLED) {
			Status = XRFDC_SUCCESS;
			goto RETURN_PATH;
		}
	}

	for (; Index < NoOfBlocks; Index++) {
		ReadReg = XRFdc_ReadReg16(InstancePtr, 0x0, XRFDC_COMMON_INTR_ENABLE);
		if (Type == XRFDC_ADC_TILE) {
			ReadReg |= (1U << (Tile_Id + 4));
			XRFdc_WriteReg16(InstancePtr, 0x0, XRFDC_COMMON_INTR_ENABLE, ReadReg);
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_INTR_ENABLE, (1U << Index), (1U << Index));
			/* Enable Converter interrupts */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
				ReadReg |= (XRFDC_ADC_OVR_VOLTAGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
				ReadReg |= (XRFDC_ADC_OVR_RANGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				ReadReg |= (XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				ReadReg |= (XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_OVR_MASK) != 0U) {
				ReadReg |= (XRFDC_ADC_CMODE_OVR_MASK >> XRFDC_ADC_CMODE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_UNDR_MASK) != 0U) {
				ReadReg |= (XRFDC_ADC_CMODE_UNDR_MASK >> XRFDC_ADC_CMODE_SHIFT);
			}

			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index), ReadReg);

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);

			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OFFSET,
						XRFDC_IXR_FIFOUSRDAT_MASK, ReadReg);
			}
			/* Check for FIFO interface interrupts (Observation FIFO)*/
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) >> XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT;
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OBS_OFFSET,
						XRFDC_IXR_FIFOUSRDAT_MASK, ReadReg);
			}
			/* Check for SUBADC interrupts */
			if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) >> XRFDC_ADC_SUBADC_DCDR_SHIFT;
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_ADC_DEC_IMR_OFFSET, XRFDC_DEC_IMR_MASK,
						ReadReg);
			}
			/* Check for DataPath interrupts */
			if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) >> XRFDC_DATA_PATH_SHIFT;
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, XRFDC_ADC_DAT_IMR_MASK,
						ReadReg);
			}
		} else {
			ReadReg |= (1U << Tile_Id);
			XRFdc_WriteReg16(InstancePtr, 0x0, XRFDC_COMMON_INTR_ENABLE, ReadReg);
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_INTR_ENABLE, (1U << Index), (1U << Index));
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				ReadReg |= (XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				ReadReg |= (XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index), ReadReg);

			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
				ReadReg |= (IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_MASK) >>
					   XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT;
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_IMR_OFFSET,
						XRFDC_DAC_FIFO_IMR_MASK, ReadReg);
			}
			if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) >> XRFDC_DATA_PATH_SHIFT;
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, XRFDC_DAC_DAT_IMR_MASK,
						ReadReg);
			}
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/****************************************************************************/
/**
*
* This function clears the interrupt mask.
*
* @param    InstancePtr is a pointer to the XRFdc instance
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    IntrMask contains the interrupts to be disabled.
*           '1' disables an interrupt, and '0' remains no change.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not available.
*
* @note     None.
*
*****************************************************************************/
u32 XRFdc_IntrDisable(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	ReadReg = XRFdc_ReadReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_ENABLE);
	if ((IntrMask & XRFDC_COMMON_MASK) != 0U) {
		ReadReg &= ~(XRFDC_COMMON_MASK >> XRFDC_COMMON_SHIFT);
		XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_ENABLE, ReadReg);
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Check for Over Voltage and Over Range */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
				ReadReg &= ~(XRFDC_ADC_OVR_VOLTAGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
				ReadReg &= ~(XRFDC_ADC_OVR_RANGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			/* Disable Converter interrupts */
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_OVR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_ADC_CMODE_OVR_MASK >> XRFDC_ADC_CMODE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_UNDR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_ADC_CMODE_UNDR_MASK >> XRFDC_ADC_CMODE_SHIFT);
			}

			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index), ReadReg);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
				ReadReg = IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK;
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OFFSET, ReadReg);
			}
			/* Check for FIFO interface interrupts (observaion FIFO)*/
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) != 0U) {
				ReadReg =
					((IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) >> XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT);
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OBS_OFFSET, ReadReg);
			}
			/* Check for SUBADC interrupts */
			if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
				ReadReg = ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) >> XRFDC_ADC_SUBADC_DCDR_SHIFT);
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_ADC_DEC_IMR_OFFSET, ReadReg);
			}
			/* Check for DataPath interrupts */
			if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
				ReadReg = ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) >> XRFDC_DATA_PATH_SHIFT);
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, ReadReg);
			}
		} else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			/* Disable Converter interrupts */
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				ReadReg &= ~(XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index), ReadReg);

			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_MASK) != 0U) {
				ReadReg = (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK);
				ReadReg |= (IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_MASK) >>
					   XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT;
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_IMR_OFFSET, ReadReg);
			}
			/* Check for FIFO DataPath interrupts */
			if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
				ReadReg = ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) >> XRFDC_DATA_PATH_SHIFT);
				XRFdc_ClrReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, ReadReg);
			}
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/****************************************************************************/
/**
*
* This function gets a mask of enabled interrupts.
*
* @param    InstancePtr is a pointer to the XRFdc instance
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*              are 0-3.
* @param    IntrMask is a pointer to the mask of enabled interrupts.
*              '1' denotes an enabled interrupt, and '0' denotes a disabled
*        interrupt.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not available.
*
* @note     None.
*
*****************************************************************************/
u32 XRFdc_GetEnabledInterrupts(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *IntrMask)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Index;
	u32 NoOfBlocks;
	u32 Status;
	u32 TileIdMask;
	u32 BlockIntrEn;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(IntrMask != NULL);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	*IntrMask = 0;
	ReadReg = XRFdc_ReadReg16(InstancePtr, XRFDC_IP_BASE, XRFDC_COMMON_INTR_ENABLE);
	TileIdMask = XRFDC_ENABLED << XRFDC_TILE_GLBL_ADDR(Type, Tile_Id);
	if ((ReadReg & TileIdMask) == XRFDC_DISABLED) {
		metal_log(METAL_LOG_DEBUG, "\n Tile interrupt bit not set for %s %u in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_SUCCESS;
		goto RETURN_PATH;
	}

	BlockIntrEn = XRFdc_ReadReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_ENABLE);
	if (BlockIntrEn & (XRFDC_COMMON_MASK >> XRFDC_COMMON_SHIFT)) {
		*IntrMask |= XRFDC_COMMON_MASK;
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if ((BlockIntrEn & (XRFDC_ENABLED << Index)) == XRFDC_DISABLED) {
			continue;
		}
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Get Converter interrupts */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			if (ReadReg & (XRFDC_ADC_OVR_VOLTAGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT)) {
				*IntrMask |= XRFDC_ADC_OVR_VOLTAGE_MASK;
			}
			if (ReadReg & (XRFDC_ADC_OVR_RANGE_MASK >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT)) {
				*IntrMask |= XRFDC_ADC_OVR_RANGE_MASK;
			}
			if (ReadReg & (XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT)) {
				*IntrMask |= XRFDC_FIFO_OVR_MASK;
			}
			if (ReadReg & (XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT)) {
				*IntrMask |= XRFDC_DAT_OVR_MASK;
			}
			if (ReadReg & (XRFDC_ADC_CMODE_OVR_MASK >> XRFDC_ADC_CMODE_SHIFT)) {
				*IntrMask |= XRFDC_ADC_CMODE_OVR_MASK;
			}
			if (ReadReg & (XRFDC_ADC_CMODE_UNDR_MASK >> XRFDC_ADC_CMODE_SHIFT)) {
				*IntrMask |= XRFDC_ADC_CMODE_UNDR_MASK;
			}

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			*IntrMask |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OFFSET,
						 XRFDC_IXR_FIFOUSRDAT_MASK);
			/* Check for Obs FIFO interface interrupts */
			*IntrMask |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OBS_OFFSET,
						 XRFDC_IXR_FIFOUSRDAT_MASK)
				     << XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT;
			/* Check for SUBADC interrupts */
			*IntrMask |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_DEC_IMR_OFFSET, XRFDC_DEC_IMR_MASK)
				     << XRFDC_ADC_SUBADC_DCDR_SHIFT;
			/* Check for DataPath interrupts */
			*IntrMask |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, XRFDC_ADC_DAT_IMR_MASK)
				<< XRFDC_DATA_PATH_SHIFT;

		} else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Get Converter interrupts */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_EN(Index));
			if (ReadReg & (XRFDC_FIFO_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT)) {
				*IntrMask |= XRFDC_FIFO_OVR_MASK;
			}
			if (ReadReg & (XRFDC_DAT_OVR_MASK >> XRFDC_DAT_FIFO_OVR_SHIFT)) {
				*IntrMask |= XRFDC_DAT_OVR_MASK;
			}

			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			*IntrMask |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_IMR_OFFSET,
						 XRFDC_IXR_FIFOUSRDAT_MASK);
			*IntrMask |= XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_IMR_OFFSET,
						 XRFDC_DAC_FIFO_IMR_SUPP_MASK)
				     << XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT;
			/* Check for DataPath interrupts */
			*IntrMask |=
				XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DATPATH_IMR_OFFSET, XRFDC_DAC_DAT_IMR_MASK)
				<< XRFDC_DATA_PATH_SHIFT;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR).
*
* @param    InstancePtr is a pointer to the XRFdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    IntrStsPtr is pointer to a32-bit value representing the contents of
* 			the Interrupt Status Registers (FIFO interface, Decoder interface,
* 			Data Path Interface).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not available.
*
* @note     None.
*
*****************************************************************************/
u32 XRFdc_GetIntrStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *IntrStsPtr)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(IntrStsPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	*IntrStsPtr = 0;
	ReadReg = XRFdc_ReadReg16(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_INTR_STS);
	*IntrStsPtr |= ((ReadReg & XRFDC_INTR_COMMON_MASK) << XRFDC_COMMON_SHIFT);
	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Check for Over Voltage and Over Range */
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index));
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_OVR_VOLTAGE_MASK) << XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_OVR_RANGE_MASK) << XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_FIFO_OVR_MASK) << XRFDC_DAT_FIFO_OVR_SHIFT);
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_DAT_OVR_MASK) << XRFDC_DAT_FIFO_OVR_SHIFT);

			/* Check for Common Mode Over/Under Voltage */
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_CMODE_OVR_MASK) << XRFDC_ADC_CMODE_SHIFT);
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_CMODE_UNDR_MASK) << XRFDC_ADC_CMODE_SHIFT);

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_ISR_OFFSET);
			*IntrStsPtr |= (ReadReg & XRFDC_IXR_FIFOUSRDAT_MASK);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_ISR_OBS_OFFSET);
			*IntrStsPtr |= (ReadReg & XRFDC_IXR_FIFOUSRDAT_MASK) << XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT;
			/* Check for SUBADC interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_DEC_ISR_OFFSET);
			*IntrStsPtr |= ((ReadReg & XRFDC_DEC_ISR_SUBADC_MASK) << XRFDC_ADC_SUBADC_DCDR_SHIFT);
			/* Check for DataPath interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DATPATH_ISR_OFFSET);
			*IntrStsPtr |= ((ReadReg & XRFDC_ADC_DAT_PATH_ISR_MASK) << XRFDC_DATA_PATH_SHIFT);
		} else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			ReadReg = XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index));
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_FIFO_OVR_MASK) << XRFDC_DAT_FIFO_OVR_SHIFT);
			*IntrStsPtr |= ((ReadReg & XRFDC_INTR_DAT_OVR_MASK) << XRFDC_DAT_FIFO_OVR_SHIFT);

			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_ISR_OFFSET);
			*IntrStsPtr |= (ReadReg & XRFDC_IXR_FIFOUSRDAT_MASK);
			*IntrStsPtr |= (ReadReg & XRFDC_DAC_FIFO_IMR_SUPP_MASK) << XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT;
			/* Check for DataPath interrupts */
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DATPATH_ISR_OFFSET);
			*IntrStsPtr |= ((ReadReg & XRFDC_DAC_DAT_PATH_ISR_MASK) << XRFDC_DATA_PATH_SHIFT);
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/****************************************************************************/
/**
*
* This function clear the interrupts.
*
* @param    InstancePtr is a pointer to the XRFdc instance
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    IntrMask contains the interrupts to be cleared.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not available.
*
* @note     None.
*
*****************************************************************************/
u32 XRFdc_IntrClr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask)
{
	u32 BaseAddr;
	u32 Status;
	u32 Index;
	u32 NoOfBlocks;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Check for Converter interrupts */
			if ((IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_ADC_OVR_VOLTAGE_MASK) >>
						       XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_ADC_OVR_RANGE_MASK) >> XRFDC_ADC_OVR_VOL_RANGE_SHIFT);
			}
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_FIFO_OVR_MASK) >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_DAT_OVR_MASK) >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_ADC_CMODE_OVR_MASK) >> XRFDC_ADC_CMODE_SHIFT);
			}
			if ((IntrMask & XRFDC_ADC_CMODE_UNDR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_ADC_CMODE_UNDR_MASK) >> XRFDC_ADC_CMODE_SHIFT);
			}

			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_ISR_OFFSET,
						 (IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK));
			}
			if ((IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_ISR_OBS_OFFSET,
						 (IntrMask & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) >>
							 XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT);
			}
			/* Check for SUBADC interrupts */
			if ((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_DEC_ISR_OFFSET,
						 (u16)((IntrMask & XRFDC_SUBADC_IXR_DCDR_MASK) >>
						       XRFDC_ADC_SUBADC_DCDR_SHIFT));
			}
			/* Check for DataPath interrupts */
			if ((IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DATPATH_ISR_OFFSET,
						 (u16)(IntrMask & XRFDC_ADC_IXR_DATAPATH_MASK) >>
							 XRFDC_DATA_PATH_SHIFT);
			}
		} else {
			/* DAC */
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			/* Check for Converter interrupts */
			if ((IntrMask & XRFDC_FIFO_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_FIFO_OVR_MASK) >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}
			if ((IntrMask & XRFDC_DAT_OVR_MASK) != 0U) {
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CONV_INTR_STS(Index),
					       (IntrMask & XRFDC_DAT_OVR_MASK) >> XRFDC_DAT_FIFO_OVR_SHIFT);
			}

			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Index);
			/* Check for FIFO interface interrupts */
			if ((IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_ISR_OFFSET,
						 (u16)((IntrMask & XRFDC_IXR_FIFOUSRDAT_MASK) |
						       ((IntrMask & XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_MASK) >>
							XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT)));
			}
			/* Check for DataPath interrupts */
			if ((IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DATPATH_ISR_OFFSET,
						 (u16)(IntrMask & XRFDC_DAC_IXR_DATAPATH_MASK) >>
							 XRFDC_DATA_PATH_SHIFT);
			}
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/****************************************************************************/
/**
*
* This function is the interrupt handler for the driver.
* It must be connected to an interrupt system by the application such that it
* can be called when an interrupt occurs.
*
* @param    Vector is interrupt vector number. Libmetal status handler
*           expects two parameters in the handler prototype, hence
*           kept this parameter. This is not used inside
*           the interrupt handler API.
* @param    XRFdcPtr contains a pointer to the driver instance
*
* @note     None.
*
* @note     Vector param is not useful inside the interrupt handler, hence
*           typecast with void to remove compilation warning.
*
******************************************************************************/
u32 XRFdc_IntrHandler(u32 Vector, void *XRFdcPtr)
{
	XRFdc *InstancePtr = (XRFdc *)XRFdcPtr;
	u32 Intrsts = 0x0U;
	u32 Tile_Id = XRFDC_TILE_ID_INV;
	s32 Block_Id;
	u32 ReadReg;
	u16 Type = 0U;
	u32 BaseAddr;
	u32 IntrMask = 0x0U;
	u32 Block = XRFDC_BLK_ID_INV;

	Xil_AssertNonvoid(InstancePtr != NULL);

	(void)Vector;
	/*
	 * Read the interrupt ID register to determine which
	 * interrupt is active
	 */
	ReadReg = XRFdc_ReadReg16(InstancePtr, 0x0, XRFDC_COMMON_INTR_STS);
	if ((ReadReg & XRFDC_EN_INTR_DAC_TILE0_MASK) != 0U) {
		Type = XRFDC_DAC_TILE;
		Tile_Id = XRFDC_TILE_ID0;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE1_MASK) != 0U) {
		Type = XRFDC_DAC_TILE;
		Tile_Id = XRFDC_TILE_ID1;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE2_MASK) != 0U) {
		Type = XRFDC_DAC_TILE;
		Tile_Id = XRFDC_TILE_ID2;
	} else if ((ReadReg & XRFDC_EN_INTR_DAC_TILE3_MASK) != 0U) {
		Type = XRFDC_DAC_TILE;
		Tile_Id = XRFDC_TILE_ID3;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE0_MASK) != 0U) {
		Type = XRFDC_ADC_TILE;
		Tile_Id = XRFDC_TILE_ID0;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE1_MASK) != 0U) {
		Type = XRFDC_ADC_TILE;
		Tile_Id = XRFDC_TILE_ID1;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE2_MASK) != 0U) {
		Type = XRFDC_ADC_TILE;
		Tile_Id = XRFDC_TILE_ID2;
	} else if ((ReadReg & XRFDC_EN_INTR_ADC_TILE3_MASK) != 0U) {
		Type = XRFDC_ADC_TILE;
		Tile_Id = XRFDC_TILE_ID3;
	} else {
		metal_log(METAL_LOG_DEBUG, "\n Invalid Tile_Id \r\n");
		goto END_OF_BLOCK_LEVEL;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_INTR_STS);
	if ((ReadReg & XRFDC_EN_INTR_SLICE0_MASK) != 0U) {
		Block_Id = XRFDC_BLK_ID0;
	} else if ((ReadReg & XRFDC_EN_INTR_SLICE1_MASK) != 0U) {
		Block_Id = XRFDC_BLK_ID1;
	} else if ((ReadReg & XRFDC_EN_INTR_SLICE2_MASK) != 0U) {
		Block_Id = XRFDC_BLK_ID2;
	} else if ((ReadReg & XRFDC_EN_INTR_SLICE3_MASK) != 0U) {
		Block_Id = XRFDC_BLK_ID3;
	} else if ((ReadReg & XRFDC_INTR_COMMON_MASK) != 0U) {
		Block = XRFDC_BLK_ID_NONE;
		IntrMask |= XRFDC_COMMON_MASK;
		goto END_OF_BLOCK_LEVEL;
	} else {
		metal_log(METAL_LOG_DEBUG, "\n Invalid ADC Block_Id \r\n");
		goto END_OF_BLOCK_LEVEL;
	}

	IntrMask |= (ReadReg & XRFDC_INTR_COMMON_MASK) << XRFDC_COMMON_SHIFT;

	Block = Block_Id;

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		if ((Block_Id == XRFDC_BLK_ID0) || (Block_Id == XRFDC_BLK_ID1)) {
			Block = XRFDC_BLK_ID0;
		} else {
			Block = XRFDC_BLK_ID1;
		}
	}

	(void)XRFdc_GetIntrStatus(InstancePtr, Type, Tile_Id, Block, &Intrsts);
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		if ((Intrsts & XRFDC_ADC_OVR_VOLTAGE_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_ADC_OVR_VOLTAGE_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC Over Voltage interrupt \r\n");
		}
		if ((Intrsts & XRFDC_ADC_OVR_RANGE_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_ADC_OVR_RANGE_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC Over Range interrupt \r\n");
		}
		if ((Intrsts & XRFDC_FIFO_OVR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_FIFO_OVR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC FIFO OF interrupt \r\n");
		}
		if ((Intrsts & XRFDC_DAT_OVR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_DAT_OVR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC DATA OF interrupt \r\n");
		}
		if ((Intrsts & XRFDC_ADC_CMODE_OVR_MASK) != 0U) {
			IntrMask |= XRFDC_ADC_CMODE_OVR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC CMODE OV interrupt \r\n");
		}
		if ((Intrsts & XRFDC_ADC_CMODE_UNDR_MASK) != 0U) {
			IntrMask |= XRFDC_ADC_CMODE_UNDR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC CMODE UV interrupt \r\n");
		}
		if ((Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_IXR_FIFOUSRDAT_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC FIFO interface interrupt \r\n");
		}
		if ((Intrsts & XRFDC_IXR_FIFOUSRDAT_OBS_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_IXR_FIFOUSRDAT_OBS_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC Obs FIFO interface interrupt \r\n");
		}
		if ((Intrsts & XRFDC_SUBADC_IXR_DCDR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_SUBADC_IXR_DCDR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC Decoder interface interrupt \r\n");
		}
		if ((Intrsts & XRFDC_ADC_IXR_DATAPATH_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_ADC_IXR_DATAPATH_MASK;
			metal_log(METAL_LOG_DEBUG, "\n ADC Data Path interface interrupt \r\n");
		}
	} else {
		/* DAC */
		if ((Intrsts & XRFDC_FIFO_OVR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_FIFO_OVR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n DAC FIFO OF interrupt \r\n");
		}
		if ((Intrsts & XRFDC_DAT_OVR_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_DAT_OVR_MASK;
			metal_log(METAL_LOG_DEBUG, "\n DAC DATA OF interrupt \r\n");
		}
		if ((Intrsts & XRFDC_DAC_IXR_FIFOUSRDAT_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_DAC_IXR_FIFOUSRDAT_MASK;
			metal_log(METAL_LOG_DEBUG, "\n DAC FIFO interface interrupt \r\n");
		}
		if ((Intrsts & XRFDC_DAC_IXR_DATAPATH_MASK) != 0U) {
			IntrMask |= Intrsts & XRFDC_DAC_IXR_DATAPATH_MASK;
			metal_log(METAL_LOG_DEBUG, "\n DAC Data Path interface interrupt \r\n");
		}
	}

	/* Clear the interrupt */
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		XRFdc_IntrClr(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block, Intrsts);

	} else {
		/* DAC */
		XRFdc_IntrClr(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block, Intrsts);
	}
END_OF_BLOCK_LEVEL:
	InstancePtr->StatusHandler(InstancePtr->CallBackRef, Type, Tile_Id, Block, IntrMask);
	return (u32)METAL_IRQ_HANDLED;
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
* @param    InstancePtr is a pointer to the XRFdc instance.
* @param    CallBackRef is the upper layer callback reference passed back
*           when the callback function is invoked.
* @param    FunctionPtr is the pointer to the callback function.
*
* @note     None.
*
* @note     The handler is called within interrupt context, so it should finish
*           its work quickly.
*
******************************************************************************/
void XRFdc_SetStatusHandler(XRFdc *InstancePtr, void *CallBackRef, XRFdc_StatusHandler FunctionPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XRFDC_COMPONENT_IS_READY);

	InstancePtr->StatusHandler = FunctionPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/** @} */
