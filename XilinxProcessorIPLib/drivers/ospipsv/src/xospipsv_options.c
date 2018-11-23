/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xospipsv_options.c
* @addtogroup ospips_v1_0
* @{
*
* This file implements funcitons to configure the OSPIPS component,
* specifically some optional settings, clock and flash related information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   nsk  02/19/18 First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*
 * Create the table of options which are processed to get/set the device
 * options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
typedef struct {
	u32 Option;
	u32 Mask;
} OptionsMap;

static OptionsMap OptionsTable[] = {
	{XOSPIPSV_CLK_POL_OPTION, (XOSPIPSV_CONFIG_REG_SEL_CLK_POL_FLD_MASK)},
	{XOSPIPSV_CLK_PHASE_OPTION, (XOSPIPSV_CONFIG_REG_SEL_CLK_PHASE_FLD_MASK)},
	{XOSPIPSV_PHY_EN_OPTION, (XOSPIPSV_CONFIG_REG_PHY_MODE_ENABLE_FLD_MASK)},
	{XOSPIPSV_DAC_EN_OPTION, (XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK)},
	{XOSPIPSV_LEGIP_EN_OPTION, (XOSPIPSV_CONFIG_REG_ENB_LEGACY_IP_MODE_FLD_MASK)},
	{XOSPIPSV_IDAC_EN_OPTION, (XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK)},
	{XOSPIPSV_DTR_EN_OPTION, (XOSPIPSV_CONFIG_REG_ENABLE_DTR_PROTOCOL_FLD_MASK)},
	{XOSPIPSV_CRC_EN_OPTION, (XOSPIPSV_CONFIG_REG_CRC_ENABLE_FLD_MASK)},
	{XOSPIPSV_DB_OP_EN_OPTION, (XOSPIPSV_CONFIG_REG_DUAL_BYTE_OPCODE_EN_FLD_MASK)},
	{XOSPIPSV_IO_EN_OPTION, (XOSPIPSV_CONFIG_REG_IO_EN_FLD_MASK)},
};

#define XOSPIPSV_NUM_OPTIONS	(sizeof(OptionsTable) / sizeof(OptionsMap))

/*****************************************************************************/
/**
*
* This function sets the options for the OSPIPS device driver.The options
* control how the device behaves relative to the OSPIPS bus. The device must be
* idle rather than busy transferring data before setting these device options.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Options contains the specified options to be set. This is a bit
*		mask where a 1 indicates the option should be turned ON and
*		a 0 indicates no action. One or more bit Values may be
*		contained in the mask. See the bit definitions named
*		XOSPIPSV_*_OPTIONS in the file xospipsv.h.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
* @note
* This function is not thread-safe.
*
******************************************************************************/
s32 XOspiPsv_SetOptions(XOspiPsv *InstancePtr, u32 Options)
{
	u32 Index;
	s32 Status;
	u32 ConfigReg = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow to modify the Control Register while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
	} else {
		for (Index = 0U; Index < XOSPIPSV_NUM_OPTIONS; Index++) {
			if ((Options & OptionsTable[Index].Option) != FALSE) {
				ConfigReg |= OptionsTable[Index].Mask;

				if(OptionsTable[Index].Mask &
						XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK) {
					InstancePtr->OpMode = XOSPIPSV_READMODE_DMA;
				} else if(OptionsTable[Index].Mask &
						XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK) {
					InstancePtr->OpMode = XOSPIPSV_READMODE_DAC;
				} else {
					InstancePtr->OpMode = XOSPIPSV_READMODE_IO;
				}

			} else {
				ConfigReg &= ~(OptionsTable[Index].Mask);
			}
		}
		Status = (s32)XST_SUCCESS;
	}
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
			ConfigReg);

	return Status;
}

/*****************************************************************************/
/**
*
* This function gets the options for the OSPIPS device. The options control how
* the device behaves relative to the OSPIPS bus.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
*
* Options contains the specified options currently set. This is a bit Value
* where a 1 means the option is on, and a 0 means the option is off.
* See the bit definitions named XOSPIPSV_*_OPTIONS in file xospipsv.h.
*
* @note		None.
*
******************************************************************************/
u32 XOspiPsv_GetOptions(XOspiPsv *InstancePtr)
{
	u32 OptionsFlag = 0;
	u32 ConfigReg;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the current options from OSPIPS configuration register.
	 */
	ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	/* Loop through the options table to grab options */
	for (Index = 0U; Index < XOSPIPSV_NUM_OPTIONS; Index++) {
		if ((ConfigReg & OptionsTable[Index].Mask) != FALSE) {
			OptionsFlag |= OptionsTable[Index].Option;
		}
	}

	return OptionsFlag;
}

/*****************************************************************************/
/**
*
* Configures the clock according to the prescaler passed.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Prescaler - clock prescaler to be set.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		It must be stopped to re-initialize.
*
* @note		None.
*
******************************************************************************/
s32 XOspiPsv_SetClkPrescaler(XOspiPsv *InstancePtr, u8 Prescaler)
{
	u32 ConfigReg;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Prescaler <= XOSPIPSV_CR_PRESC_MAXIMUM);

	/*
	 * Do not allow the slave select to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
	} else {

		/*
		 * Read the configuration register, mask out the relevant bits, and set
		 * them with the shifted Value passed into the function. Write the
		 * results back to the configuration register.
		 */
		ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
				ConfigReg &= (u32)(~(XOSPIPSV_CONFIG_REG_MSTR_BAUD_DIV_FLD_MASK));
				ConfigReg |= (u32) ((u32)Prescaler & XOSPIPSV_CR_PRESC_MAXIMUM)
								<< (u32)XOSPIPSV_CONFIG_REG_MSTR_BAUD_DIV_FLD_SHIFT;

				XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
						XOSPIPSV_CONFIG_REG, ConfigReg);
				Status = XST_SUCCESS;

	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function should be used to tell the OSPIPS driver the HW flash
* configuration being used. This API should be called atleast once in the
* application. If desired, it can be called multiple times when switching
* between communicating to different flash devices/using different configs.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Chip_Cs - Flash Chip Select.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		It must be stopped to re-initialize.
*
* @note		If this funciton is not called atleast once in the application,
*		the driver assumes there is a single flash connected to the
*		lower bus and CS line.
*
******************************************************************************/
s32 XOspiPsv_SelectFlash(XOspiPsv *InstancePtr, u8 chip_select)
{
	if(chip_select > 2U) {
		return (s32)XST_FAILURE;
	}

	InstancePtr->ChipSelect = chip_select;

	return (s32)XST_SUCCESS;
}

/** @} */
