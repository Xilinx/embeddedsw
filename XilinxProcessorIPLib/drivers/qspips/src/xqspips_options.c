/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xqspips_options.c
* @brief Contains functions for the configuration of the XQspiPs driver
* component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.00  sdm 11/25/10 First release
* 2.00a kka 07/25/12 Removed the selection for the following options:
*		     Master mode (XQSPIPS_MASTER_OPTION) and
*		     Flash interface mode (XQSPIPS_FLASH_MODE_OPTION) option
*		     as the QSPI driver supports the Master mode
*		     and Flash Interface mode. The driver doesn't support
*		     Slave mode or the legacy mode.
* 		     Added the option for setting the Holdb_dr bit in the
*		     configuration options, XQSPIPS_HOLD_B_DRIVE_OPTION
*		     is the option to be used for setting this bit in the
*		     configuration register.
* 2.01a sg  02/03/13 SetDelays and GetDelays API's include DelayNss parameter.
*
* 2.02a hk  26/03/13 Removed XQspi_Reset() in Set_Options() function when
*			 LQSPI_MODE_OPTION is set. Moved Enable() to XQpsiPs_LqspiRead().
* 3.3   sk  11/07/15 Modified the API prototypes according to MISRAC standards
*                    to remove compilation warnings. CR# 868893.
* 3.16  vn  08/26/26 Fix Doxygen warnings
* 3.16  tb  09/30/26 Align Doxygen comments with AMD documentation standards
*</pre>
*
******************************************************************************/

/**
* @addtogroup qspips_api QSPIPS APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xqspips.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/**
 * @brief Holds an entry of the table of options which are processed to get/set
 * the device options.
 *
 * These options are table driven to allow easy maintenance and expansion of
 * the options.
 */
typedef struct {
	u32 Option;	/**< Driver option bit. */
	u32 Mask;	/**< Configuration register mask of the option. */
} OptionsMap;

/** Table of the options and their configuration register masks. */
static OptionsMap OptionsTable[] = {
	{XQSPIPS_CLK_ACTIVE_LOW_OPTION, XQSPIPS_CR_CPOL_MASK},
	{XQSPIPS_CLK_PHASE_1_OPTION, XQSPIPS_CR_CPHA_MASK},
	{XQSPIPS_FORCE_SSELECT_OPTION, XQSPIPS_CR_SSFORCE_MASK},
	{XQSPIPS_MANUAL_START_OPTION, XQSPIPS_CR_MANSTRTEN_MASK},
	{XQSPIPS_HOLD_B_DRIVE_OPTION, XQSPIPS_CR_HOLD_B_MASK},
};

/** Number of entries in the options table. */
#define XQSPIPS_NUM_OPTIONS	(sizeof(OptionsTable) / sizeof(OptionsMap))

/*****************************************************************************/
/**
*
* @brief Sets the options for the QSPI device driver.
*
* The options control how the device behaves relative to the QSPI bus. The
* device must be idle rather than busy transferring data before setting these
* device options.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
* @param[in]	Options The specified options to be set. This is a bit
*		mask where a 1 means to turn the option on, and a 0 means to
*		turn the option off. One or more bit values may be contained in
*		the mask. See the bit definitions named XQSPIPS_*_OPTIONS in
*		the file xqspips.h.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
* @note		The function is not thread-safe.
*
******************************************************************************/
s32 XQspiPs_SetOptions(XQspiPs *InstancePtr, u32 Options)
{
	u32 ConfigReg;
	unsigned int Index;
	u32 QspiOptions;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow to modify the Control Register while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy) {
		return XST_DEVICE_BUSY;
	}

	QspiOptions = Options & XQSPIPS_LQSPI_MODE_OPTION;
	Options &= ~XQSPIPS_LQSPI_MODE_OPTION;

	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_CR_OFFSET);

	/*
	 * Loop through the options table, turning the option on or off
	 * depending on whether the bit is set in the incoming options flag.
	 */
	for (Index = 0; Index < XQSPIPS_NUM_OPTIONS; Index++) {
		if (Options & OptionsTable[Index].Option) {
			/* Turn it on */
			ConfigReg |= OptionsTable[Index].Mask;
		} else {
			/* Turn it off */
			ConfigReg &= ~(OptionsTable[Index].Mask);
		}
	}

	/*
	 * Now write the control register. Leave it to the upper layers
	 * to restart the device.
	 */
	XQspiPs_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPS_CR_OFFSET,
			 ConfigReg);

	/*
	 * Check for the LQSPI configuration options.
	 */
	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_LQSPI_CR_OFFSET);


	if (QspiOptions & XQSPIPS_LQSPI_MODE_OPTION) {
		XQspiPs_WriteReg(InstancePtr->Config.BaseAddress,
				 XQSPIPS_LQSPI_CR_OFFSET,
				 XQSPIPS_LQSPI_CR_RST_STATE);
		XQspiPs_SetSlaveSelect(InstancePtr);
	} else {
		ConfigReg &= ~XQSPIPS_LQSPI_CR_LINEAR_MASK;
		XQspiPs_WriteReg(InstancePtr->Config.BaseAddress,
				 XQSPIPS_LQSPI_CR_OFFSET, ConfigReg);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Gets the options for the QSPI device.
*
* The options control how the device behaves relative to the QSPI bus.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
*
* @return	The options currently set. This is a bit value where a 1 means
*		the option is on, and a 0 means the option is off. See the bit
*		definitions named XQSPIPS_*_OPTIONS in file xqspips.h.
*
******************************************************************************/
u32 XQspiPs_GetOptions(XQspiPs *InstancePtr)
{
	u32 OptionsFlag = 0;
	u32 ConfigReg;
	unsigned int Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the current options from QSPI configuration register.
	 */
	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_CR_OFFSET);

	/*
	 * Loop through the options table to grab options
	 */
	for (Index = 0; Index < XQSPIPS_NUM_OPTIONS; Index++) {
		if (ConfigReg & OptionsTable[Index].Mask) {
			OptionsFlag |= OptionsTable[Index].Option;
		}
	}

	/*
	 * Check for the LQSPI configuration options.
	 */
	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_LQSPI_CR_OFFSET);

	if ((ConfigReg & XQSPIPS_LQSPI_CR_LINEAR_MASK) != 0) {
		OptionsFlag |= XQSPIPS_LQSPI_MODE_OPTION;
	}

	return OptionsFlag;
}

/*****************************************************************************/
/**
*
* @brief Sets the clock prescaler for an QSPI device.
*
* The device must be idle rather than busy transferring data before setting
* these device options.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
* @param[in]	Prescaler The value that determine how much the clock should
*		be divided by. Use the XQSPIPS_CLK_PRESCALE_* constants defined
*		in xqspips.h for this setting.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
* @note		The function is not thread-safe.
*
******************************************************************************/
s32 XQspiPs_SetClkPrescaler(XQspiPs *InstancePtr, u8 Prescaler)
{
	u32 ConfigReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Prescaler <= XQSPIPS_CR_PRESC_MAXIMUM);

	/*
	 * Do not allow the slave select to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy) {
		return XST_DEVICE_BUSY;
	}

	/*
	 * Read the configuration register, mask out the interesting bits, and set
	 * them with the shifted value passed into the function. Write the
	 * results back to the configuration register.
	 */
	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_CR_OFFSET);

	ConfigReg &= ~XQSPIPS_CR_PRESC_MASK;
	ConfigReg |= (u32) (Prescaler & XQSPIPS_CR_PRESC_MAXIMUM) <<
		     XQSPIPS_CR_PRESC_SHIFT;

	XQspiPs_WriteReg(InstancePtr->Config.BaseAddress,
			 XQSPIPS_CR_OFFSET,
			 ConfigReg);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Gets the clock prescaler of an QSPI device.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
*
* @return	The prescaler value.
*
******************************************************************************/
u8 XQspiPs_GetClkPrescaler(XQspiPs *InstancePtr)
{
	u32 ConfigReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	ConfigReg = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
				    XQSPIPS_CR_OFFSET);

	ConfigReg &= XQSPIPS_CR_PRESC_MASK;

	return (u8)(ConfigReg >> XQSPIPS_CR_PRESC_SHIFT);
}

/*****************************************************************************/
/**
*
* @brief Sets the delay register for the QSPI device driver.
*
* The delay register controls the Delay Between Transfers, Delay After
* Transfers, and the Delay Initially. The default value is 0x0. The range of
* each delay value is 0-255.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
* @param[in]	DelayNss The delay to de-assert slave select between
*		two word transfers.
* @param[in]	DelayBtwn The delay between one Slave Select being
*		de-activated and the activation of another slave. The delay is
*		the number of master clock periods given by DelayBtwn + 2.
* @param[in]	DelayAfter Define the delay between the last bit of the current
*		byte transfer and the first bit of the next byte transfer.
*		The delay in number of master clock periods is given as:
*		CHPA=0:DelayInit+DelayAfter+3
*		CHPA=1:DelayAfter+1
* @param[in]	DelayInit The delay between asserting the slave select signal
*		and the first bit transfer. The delay int number of master clock
*		periods is DelayInit+1.
*
* @return
*		- XST_SUCCESS if delays are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
******************************************************************************/
int XQspiPs_SetDelays(XQspiPs *InstancePtr, u8 DelayNss, u8 DelayBtwn,
		      u8 DelayAfter, u8 DelayInit)
{
	u32 DelayRegister;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow the delays to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy) {
		return XST_DEVICE_BUSY;
	}

	/* Shift, Mask and OR the values to build the register settings */
	DelayRegister = (u32) DelayNss << XQSPIPS_DR_NSS_SHIFT;
	DelayRegister |= (u32) DelayBtwn << XQSPIPS_DR_BTWN_SHIFT;
	DelayRegister |= (u32) DelayAfter << XQSPIPS_DR_AFTER_SHIFT;
	DelayRegister |= (u32) DelayInit;

	XQspiPs_WriteReg(InstancePtr->Config.BaseAddress,
			 XQSPIPS_DR_OFFSET, DelayRegister);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Gets the delay settings for an QSPI device.
*
* The delay register controls the Delay Between Transfers, Delay After
* Transfers, and the Delay Initially. The default value is 0x0.
*
* @param[in]	InstancePtr Pointer to the XQspiPs instance.
* @param[out]	DelayNss Pointer to the Delay to de-assert slave select
*		between two word transfers.
* @param[out]	DelayBtwn Pointer to the Delay Between transfers value.
* @param[out]	DelayAfter Pointer to the Delay After transfer value.
* @param[out]	DelayInit Pointer to the Delay Initially value.
*
******************************************************************************/
void XQspiPs_GetDelays(XQspiPs *InstancePtr, u8 *DelayNss, u8 *DelayBtwn,
		       u8 *DelayAfter, u8 *DelayInit)
{
	u32 DelayRegister;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	DelayRegister = XQspiPs_ReadReg(InstancePtr->Config.BaseAddress,
					XQSPIPS_DR_OFFSET);

	*DelayInit = (u8)(DelayRegister & XQSPIPS_DR_INIT_MASK);

	*DelayAfter = (u8)((DelayRegister & XQSPIPS_DR_AFTER_MASK) >>
			   XQSPIPS_DR_AFTER_SHIFT);

	*DelayBtwn = (u8)((DelayRegister & XQSPIPS_DR_BTWN_MASK) >>
			  XQSPIPS_DR_BTWN_SHIFT);

	*DelayNss = (u8)((DelayRegister & XQSPIPS_DR_NSS_MASK) >>
			 XQSPIPS_DR_NSS_SHIFT);
}
/** @} */
