/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_options.c
* @addtogroup Overview
* @{
*
* This file implements functions to configure the OSPIPSV component,
* specifically some optional settings, clock and flash related information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   nsk  02/19/18 First release
*       sk   01/09/19 Updated XOspiPsv_SetOptions() API to support
*                     DAC mode switching.
*                     Removed Legacy/STIG mode option in OptionsTable.
*       sk   02/04/19 Added support for SDR+PHY and DDR+PHY modes.
* 1.1   sk   07/22/19 Added RX Tuning algorithm for SDR and DDR modes.
* 1.6   sk   02/07/22 Replaced driver version in addtogroup with Overview.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xospipsv.h"
#include "xospipsv_control.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * Create the table of options which are processed to get/set the device
 * options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
typedef struct {
	u32 Option;	/**< Device configuration option */
	u32 Mask;	/**< Mask value for the corresponding Device option */
} OptionsMap;

static OptionsMap OptionsTable[] = {
	{XOSPIPSV_CLK_POL_OPTION, (XOSPIPSV_CONFIG_REG_SEL_CLK_POL_FLD_MASK)},
	{XOSPIPSV_CLK_PHASE_OPTION, (XOSPIPSV_CONFIG_REG_SEL_CLK_PHASE_FLD_MASK)},
	{XOSPIPSV_DAC_EN_OPTION, (((u32)XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK) |
			(u32)XOSPIPSV_CONFIG_REG_ENB_AHB_ADDR_REMAP_FLD_MASK)},
	{XOSPIPSV_IDAC_EN_OPTION, (XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK)},
	{XOSPIPSV_CRC_EN_OPTION, (XOSPIPSV_CONFIG_REG_CRC_ENABLE_FLD_MASK)},
	{XOSPIPSV_DB_OP_EN_OPTION, (XOSPIPSV_CONFIG_REG_DUAL_BYTE_OPCODE_EN_FLD_MASK)},
};

/**
 * Total number of configuration options.
 */
#define XOSPIPSV_NUM_OPTIONS	(sizeof(OptionsTable) / sizeof(OptionsMap))

/*****************************************************************************/
/**
* @brief
* This function sets the options for the OSPIPSV device driver.The options
* control how the device behaves relative to the OSPIPSV bus. The device must be
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
* 		This function is not thread-safe.
*
******************************************************************************/
u32 XOspiPsv_SetOptions(XOspiPsv *InstancePtr, u32 Options)
{
	u32 Index;
	u32 Status;
	u32 ConfigReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow to modify the Control Register while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = XST_DEVICE_BUSY;
	} else {
		XOspiPsv_Disable(InstancePtr);
		ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
		for (Index = 0U; Index < XOSPIPSV_NUM_OPTIONS; Index++) {
			if ((Options & OptionsTable[Index].Option) != (u32)FALSE) {
				ConfigReg |= OptionsTable[Index].Mask;
				if((OptionsTable[Index].Mask &
						XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK) != 0U) {
					XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
							XOSPIPSV_REMAP_ADDR_REG, XOSPIPSV_REMAP_ADDR_VAL);
					InstancePtr->OpMode = XOSPIPSV_DAC_MODE;
					/* IOU_SLCR MUX selection */
					#if defined (__aarch64__) && (EL1_NONSECURE == 1)
					/*
					 * Execution is happening in non secure world, configure MUX
					 * settings through SMC calls
					 */

					/* Request for OSPI node */
					Xil_Smc(PM_REQUEST_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
					/* Change MUX settings to select LINEAR mode */
					Xil_Smc(PM_IOCTL_SMC_FID, (((u64)PM_IOCTL_OSPI_MUX_SELECT << 32) | OSPI_NODE_ID) , PM_OSPI_MUX_SEL_LINEAR, 0,0,0,0,0);
					/* Release OSPI node */
					Xil_Smc(PM_RELEASE_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
					#else
					XOspiPsv_WriteReg(XPMC_IOU_SLCR_BASEADDR,
						XPMC_IOU_SLCR_OSPI_MUX_SEL,
						XOspiPsv_ReadReg(XPMC_IOU_SLCR_BASEADDR,
							XPMC_IOU_SLCR_OSPI_MUX_SEL) |
							(u32)XPMC_IOU_SLCR_OSPI_MUX_SEL_DAC_MASK);
					#endif
				} else {
					XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
								XOSPIPSV_REMAP_ADDR_REG, 0x0U);
					if((OptionsTable[Index].Mask &
						XOSPIPSV_CONFIG_REG_ENB_DMA_IF_FLD_MASK) != 0U) {
						InstancePtr->OpMode = XOSPIPSV_IDAC_MODE;
						#if defined (__aarch64__) && (EL1_NONSECURE == 1)
						/* Request for OSPI node */
						Xil_Smc(PM_REQUEST_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0, 0, 0, 0, 0);
						/* Change MUX settings to select DMA mode */
						Xil_Smc(PM_IOCTL_SMC_FID, (((u64)PM_IOCTL_OSPI_MUX_SELECT << 32) | OSPI_NODE_ID), PM_OSPI_MUX_SEL_DMA, 0, 0, 0, 0, 0);
						/* Release OSPI node */
						Xil_Smc(PM_RELEASE_DEVICE_SMC_FID,OSPI_NODE_ID, 0, 0, 0, 0, 0, 0);
						#else
						XOspiPsv_WriteReg(XPMC_IOU_SLCR_BASEADDR,
							XPMC_IOU_SLCR_OSPI_MUX_SEL,
								XOspiPsv_ReadReg(XPMC_IOU_SLCR_BASEADDR,
										XPMC_IOU_SLCR_OSPI_MUX_SEL) &
										~(u32)XPMC_IOU_SLCR_OSPI_MUX_SEL_DAC_MASK);
						#endif
					}
				}

			} else {
				if (OptionsTable[Index].Option == XOSPIPSV_DAC_EN_OPTION) {
					if ((ConfigReg & XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_MASK) != 0U) {
						#if defined (__aarch64__) && (EL1_NONSECURE == 1)
						/*
						 * Execution is happening in non secure world, configure MUX
						 * settings through SMC calls
						 */

						/* Request for OSPI node */
						Xil_Smc(PM_REQUEST_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
						/* Change MUX settings to select DMA mode */
						Xil_Smc(PM_IOCTL_SMC_FID, (((u64)PM_IOCTL_OSPI_MUX_SELECT << 32) | OSPI_NODE_ID) , PM_OSPI_MUX_SEL_DMA, 0,0,0,0,0);
						/* Release OSPI node */
						Xil_Smc(PM_RELEASE_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
						#else
						XOspiPsv_WriteReg(XPMC_IOU_SLCR_BASEADDR,
							XPMC_IOU_SLCR_OSPI_MUX_SEL,
							XOspiPsv_ReadReg(XPMC_IOU_SLCR_BASEADDR,
								XPMC_IOU_SLCR_OSPI_MUX_SEL) &
								~(u32)XPMC_IOU_SLCR_OSPI_MUX_SEL_DAC_MASK);
						#endif
					}
				}
				ConfigReg &= ~(OptionsTable[Index].Mask);
			}
		}
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
					ConfigReg);
		XOspiPsv_Enable(InstancePtr);
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function gets the options for the OSPIPSV device. The options control how
* the device behaves relative to the OSPIPSV bus.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return
* 		Options contains the specified options currently set. This is a bit Value
* 		where a 1 means the option is on, and a 0 means the option is off.
* 		See the bit definitions named XOSPIPSV_*_OPTIONS in file xospipsv.h.
*
******************************************************************************/
u32 XOspiPsv_GetOptions(const XOspiPsv *InstancePtr)
{
	u32 OptionsFlag = 0;
	u32 ConfigReg;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Loop through the options table to grab options */
	for (Index = 0U; Index < XOSPIPSV_NUM_OPTIONS; Index++) {
		/*
		 * Get the current options from OSPIPSV configuration register.
		 */
		ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_CONFIG_REG);
		if ((ConfigReg & OptionsTable[Index].Mask) != (u32)FALSE) {
			OptionsFlag |= OptionsTable[Index].Option;
		}
	}

	return OptionsFlag;
}

/*****************************************************************************/
/**
* @brief
* Configures the clock according to the prescaler passed.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Prescaler - clock prescaler to be set.
*
* @return
*		- XST_SUCCESS if successful.
*       - XST_FAILURE if failed to set prescaler.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		It must be stopped to re-initialize.
*
******************************************************************************/
u32 XOspiPsv_SetClkPrescaler(XOspiPsv *InstancePtr, u8 Prescaler)
{
	u32 ConfigReg;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Prescaler <= XOSPIPSV_CR_PRESC_MAXIMUM);

	/*
	 * Do not allow the slave select to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == (u32)TRUE) {
		Status = (u32)XST_DEVICE_BUSY;
	} else {
		if ((InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_SDR_NON_PHY) &&
			(Prescaler == XOSPIPSV_CLK_PRESCALE_2)) {
			Status = XST_FAILURE;
			goto ERROR_PATH;
		}
		XOspiPsv_Disable(InstancePtr);

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
		XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
						ConfigReg);

		XOspiPsv_Enable(InstancePtr);
		Status = XOspiPsv_SetDllDelay(InstancePtr);
	}

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configures the edge mode (SDR or DDR)
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	Mode is Edge mode. XOSPIPSV_EDGE_MODE_* represents valid values.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail to set.
*
******************************************************************************/
u32 XOspiPsv_SetSdrDdrMode(XOspiPsv *InstancePtr, u32 Mode)
{
	u32 ConfigReg;
	u32 Status;
	u32 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if ((Mode != XOSPIPSV_EDGE_MODE_DDR_PHY) &&
			(Mode != XOSPIPSV_EDGE_MODE_SDR_PHY) &&
			(Mode != XOSPIPSV_EDGE_MODE_SDR_NON_PHY)) {
		Status = XST_FAILURE;
		goto ERROR_PATH;
	}
	InstancePtr->SdrDdrMode = Mode;

	XOspiPsv_Disable(InstancePtr);
	ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
							XOSPIPSV_CONFIG_REG);
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
							XOSPIPSV_WRITE_COMPLETION_CTRL_REG);
	ConfigReg &= ~XOSPIPSV_CONFIG_REG_ENABLE_DTR_PROTOCOL_FLD_MASK;
	ConfigReg &= ~XOSPIPSV_CONFIG_REG_PHY_MODE_ENABLE_FLD_MASK;
	ReadReg &= ~XOSPIPSV_WRITE_COMPLETION_CTRL_REG_POLL_COUNT_FLD_MASK;
	ReadReg |= ((u32)XOSPIPSV_POLL_CNT_FLD_NON_PHY <<
			XOSPIPSV_WRITE_COMPLETION_CTRL_REG_POLL_COUNT_FLD_SHIFT);
	if ((InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) ||
			(InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_SDR_PHY)) {
		ConfigReg |= XOSPIPSV_CONFIG_REG_PHY_MODE_ENABLE_FLD_MASK;
		ReadReg |= ((u32)XOSPIPSV_POLL_CNT_FLD_PHY <<
				XOSPIPSV_WRITE_COMPLETION_CTRL_REG_POLL_COUNT_FLD_SHIFT);
		if (InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			ConfigReg |= XOSPIPSV_CONFIG_REG_ENABLE_DTR_PROTOCOL_FLD_MASK;
		}
	}

	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
								ConfigReg);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_WRITE_COMPLETION_CTRL_REG, ReadReg);
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_RD_DATA_CAPTURE_REG);
	ReadReg &= ~XOSPIPSV_RD_DATA_CAPTURE_REG_DELAY_FLD_MASK;
	if (InstancePtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_SDR_NON_PHY) {
		ReadReg |= (XOSPIPSV_NON_PHY_RD_DLY <<
			XOSPIPSV_RD_DATA_CAPTURE_REG_DELAY_FLD_SHIFT);
	}
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_RD_DATA_CAPTURE_REG, ReadReg);
	XOspiPsv_Enable(InstancePtr);
	Status = XOspiPsv_SetDllDelay(InstancePtr);

ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* This function should be used to tell the OSPIPSV driver the HW flash
* configuration being used. This API should be called at least once in the
* application. If desired, it can be called multiple times when switching
* between communicating to different flash devices/using different configs.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	chip_select - Flash Chip Select.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*		It must be stopped to re-initialize.
*
* @note
* 		If this function is not called at least once in the application,
*		the driver assumes there is a single flash connected to the
*		lower bus and CS line.
*
******************************************************************************/
u32 XOspiPsv_SelectFlash(XOspiPsv *InstancePtr, u8 chip_select)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	if(chip_select >= 2U) {
		Status = (u32)XST_FAILURE;
		goto ERROR_PATH;
	}

	InstancePtr->ChipSelect = chip_select;

	Status = (u32)XST_SUCCESS;
ERROR_PATH:
	return Status;
}

/*****************************************************************************/
/**
* @brief
* Configures how the controller will poll the device following a write
* transfer in DAC mode.
*
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
* @param	FlashMode is Edge mode. XOSPIPSV_EDGE_MODE_* represents valid values.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if fail to set.
*
******************************************************************************/
void XOspiPsv_ConfigureAutoPolling(const XOspiPsv *InstancePtr, u32 FlashMode)
{
	u32 ReadReg;
	u8 Dummy;

	Xil_AssertVoid(InstancePtr != NULL);

	XOspiPsv_Disable(InstancePtr);
	ReadReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
				XOSPIPSV_POLLING_FLASH_STATUS_REG);
	ReadReg &= ~XOSPIPSV_POLLING_FLASH_STATUS_REG_DEVICE_STATUS_NB_DUMMY_MASK;
	Dummy = InstancePtr->Extra_DummyCycle;
	if (FlashMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Dummy += XOSPIPSV_DDR_STATS_REG_DUMMY;
	}
	ReadReg |= ((u32)Dummy <<
		XOSPIPSV_POLLING_FLASH_STATUS_REG_DEVICE_STATUS_NB_DUMMY_SHIFT);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
					XOSPIPSV_POLLING_FLASH_STATUS_REG, ReadReg);
	XOspiPsv_Enable(InstancePtr);
}

/** @} */
