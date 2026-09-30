/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xspips_options.c
*
* @brief Implements the functions for the configuration of the XSpiPs driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  drg/jz 01/25/10 First release
* 1.00	sdm    10/25/11 Removed the Divide by 2 in the SPI Clock Prescaler
*			options as this is not supported in the device
* 1.04a	sg     01/30/13 Added XSPIPS_MANUAL_START_OPTION. SetDelays and
*			GetDelays API's include DelayNss parameter.
* 1.05a hk 	   26/04/13 Added disable and enable in XSpiPs_SetOptions when
*				CPOL/CPHA bits are set/reset. Fix for CR#707669.
* 3.00  kvn    02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.2   aru    01/20/19 Fixes violations according to MISRAC-2012
*                       in safety mode and done changes such as
*                       Declared the pointer param as Pointer to const
* 3.15  vn     08/26/26 Fix Doxygen warnings.
* 3.15  tb     09/30/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/**
* @addtogroup spips_api SPIPS APIs
* @{
*/

/***************************** Include Files *********************************/

#include "xspips.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/**
 * @brief Describes the mapping of a driver option to its register bit mask.
 *
 * The table of options is processed to get/set the device
 * options. These options are table driven to allow easy maintenance and
 * expansion of the options.
 */
typedef struct {
	u32 Option;	/**< Driver option (XSPIPS_*_OPTION). */
	u32 Mask;	/**< Configuration register bit mask for the option. */
} OptionsMap;

/** @brief Maps the driver options to the configuration register masks. */
static OptionsMap OptionsTable[] = {
	{XSPIPS_MASTER_OPTION, XSPIPS_CR_MSTREN_MASK},
	{XSPIPS_CLK_ACTIVE_LOW_OPTION, XSPIPS_CR_CPOL_MASK},
	{XSPIPS_CLK_PHASE_1_OPTION, XSPIPS_CR_CPHA_MASK},
	{XSPIPS_DECODE_SSELECT_OPTION, XSPIPS_CR_SSDECEN_MASK},
	{XSPIPS_FORCE_SSELECT_OPTION, XSPIPS_CR_SSFORCE_MASK},
	{XSPIPS_MANUAL_START_OPTION, XSPIPS_CR_MANSTRTEN_MASK}
};

/** Number of entries in the options table. */
#define XSPIPS_NUM_OPTIONS	(sizeof(OptionsTable) / sizeof(OptionsMap))

/*****************************************************************************/
/**
*
* @brief Sets the options for the SPI device driver.
*
* The options control how the device behaves relative to the SPI bus. The
* device must be idle rather than busy transferring data before setting these
* device options.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
* @param[in]	Options The specified options to be set. This is a bit
*		mask where a 1 means to turn the option on, and a 0 means to
*		turn the option off. One or more bit values may be contained in
*		the mask. See the bit definitions named XSPIPS_*_OPTIONS in the
*		file xspips.h.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
* @note		The function is not thread-safe.
*
******************************************************************************/
s32 XSpiPs_SetOptions(const XSpiPs *InstancePtr, u32 Options)
{
	u32 ConfigReg;
	u32 Index;
	u32 CurrentConfigReg;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow the slave select to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
	} else {

		ConfigReg = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
					 XSPIPS_CR_OFFSET);

		CurrentConfigReg = ConfigReg;

		/*
		 * Loop through the options table, turning the option on or off
		 * depending on whether the bit is set in the incoming options flag.
		 */
		for (Index = 0U; Index < XSPIPS_NUM_OPTIONS; Index++) {
			if ((Options & OptionsTable[Index].Option) != (u32)0U) {
				/* Turn it on */
				ConfigReg |= OptionsTable[Index].Mask;
			}
			else {
				/* Turn it off */
				ConfigReg &= ~(OptionsTable[Index].Mask);
			}
		}


		/*
		 * If CPOL-CPHA bits are toggled from previous state,
		 * disable before writing the configuration register and then enable.
		 */
		if( ((CurrentConfigReg & XSPIPS_CR_CPOL_MASK) !=
			(ConfigReg & XSPIPS_CR_CPOL_MASK)) ||
			((CurrentConfigReg & XSPIPS_CR_CPHA_MASK) !=
			(ConfigReg & XSPIPS_CR_CPHA_MASK)) ) {
				XSpiPs_Disable(InstancePtr);
			}

		/*
		 * Now write the Config register. Leave it to the upper layers
		 * to restart the device.
		 */
		XSpiPs_WriteReg(InstancePtr->Config.BaseAddress,
					XSPIPS_CR_OFFSET, ConfigReg);

		/*
		 * Enable
		 */
		if( ((CurrentConfigReg & XSPIPS_CR_CPOL_MASK) !=
			(ConfigReg & XSPIPS_CR_CPOL_MASK)) ||
			((CurrentConfigReg & XSPIPS_CR_CPHA_MASK) !=
			(ConfigReg & XSPIPS_CR_CPHA_MASK)) ) {
				XSpiPs_Enable(InstancePtr);
			}

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* @brief Gets the options for the SPI device.
*
* The options control how the device behaves relative to the SPI bus.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
*
* @return	The options currently set. This is a bit value where a 1
*		means the option is on, and a 0 means the option is off.
*		See the bit definitions named XSPIPS_*_OPTIONS in file
*		xspips.h.
*
******************************************************************************/
u32 XSpiPs_GetOptions(const XSpiPs *InstancePtr)
{
	u32 OptionsFlag = 0U;
	u32 ConfigReg;
	u32 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the current options
	 */
	ConfigReg =
		XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
				 XSPIPS_CR_OFFSET);

	/*
	 * Loop through the options table to grab options
	 */
	for (Index = 0U; Index < XSPIPS_NUM_OPTIONS; Index++) {
		if ((ConfigReg & OptionsTable[Index].Mask) != FALSE) {
			OptionsFlag |= OptionsTable[Index].Option;
		}
	}

	return OptionsFlag;
}

/*****************************************************************************/
/**
*
* @brief Sets the clock prescaler for an SPI device.
*
* The device must be idle rather than busy transferring data before setting
* these device options.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
* @param[in]	Prescaler The value that determine how much the clock should
*		be divided by. Use the XSPIPS_CLK_PRESCALE_* constants defined
*		in xspips.h for this setting.
*
* @return
*		- XST_SUCCESS if options are successfully set.
*		- XST_DEVICE_BUSY if the device is currently transferring data.
*		The transfer must complete or be aborted before setting options.
*
* @note		The function is not thread-safe.
*
******************************************************************************/
s32 XSpiPs_SetClkPrescaler(const XSpiPs *InstancePtr, u8 Prescaler)
{
	u32 ConfigReg;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Prescaler > 0U) && (Prescaler <= XSPIPS_CR_PRESC_MAXIMUM));

	/*
	 * Do not allow the prescaler to be changed while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
	} else {

		/*
		 * Read the Config register, mask out the interesting bits, and set
		 * them with the shifted value passed into the function. Write the
		 * results back to the Config register.
		 */
		ConfigReg = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
					 XSPIPS_CR_OFFSET);

		ConfigReg &= (u32)(~XSPIPS_CR_PRESC_MASK);
		ConfigReg |= (u32) ((u32)Prescaler & (u32)XSPIPS_CR_PRESC_MAXIMUM) <<
			XSPIPS_CR_PRESC_SHIFT;

		XSpiPs_WriteReg(InstancePtr->Config.BaseAddress,
				XSPIPS_CR_OFFSET,
				ConfigReg);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* @brief Gets the clock prescaler of an SPI device.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
*
* @return	The prescaler value.
*
******************************************************************************/
u8 XSpiPs_GetClkPrescaler(const XSpiPs *InstancePtr)
{
	u32 ConfigReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	ConfigReg = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
			XSPIPS_CR_OFFSET);

	ConfigReg &= XSPIPS_CR_PRESC_MASK;

	return (u8)(ConfigReg >> XSPIPS_CR_PRESC_SHIFT);

}

/*****************************************************************************/
/**
*
* @brief Sets the delay register for the SPI device driver.
*
* The delay register controls the Delay Between Transfers, Delay After
* Transfers, and the Delay Initially. The default value is 0x0. The range of
* each delay value is 0-255.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
* @param[in]	DelayNss The delay for which the chip select outputs are
*		de-asserted between words when CPHA=0.
* @param[in]	DelayBtwn The delay between one Slave Select being
*		de-activated and the activation of another slave. The delay is
*		the number of master clock periods given by DelayBtwn + 2.
* @param[in]	DelayAfter Define the delay between the last bit of the current
*		byte transfer and the first bit of the next byte transfer.
*		The delay in number of master clock periods is given as:
*		CPHA=0:DelayInit+DelayAfter+3
*		CPHA=1:DelayAfter+1
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
s32 XSpiPs_SetDelays(const XSpiPs *InstancePtr, u8 DelayNss, u8 DelayBtwn,
			 u8 DelayAfter, u8 DelayInit)
{
	u32 DelayRegister;
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Do not allow the delays to change while a transfer is in
	 * progress. Not thread-safe.
	 */
	if (InstancePtr->IsBusy == TRUE) {
		Status = (s32)XST_DEVICE_BUSY;
	} else {

		/* Shift, Mask and OR the values to build the register settings */
		DelayRegister = (u32) DelayNss << XSPIPS_DR_NSS_SHIFT;
		DelayRegister |= (u32) DelayBtwn << XSPIPS_DR_BTWN_SHIFT;
		DelayRegister |= (u32) DelayAfter << XSPIPS_DR_AFTER_SHIFT;
		DelayRegister |= (u32) DelayInit;

		XSpiPs_WriteReg(InstancePtr->Config.BaseAddress,
				XSPIPS_DR_OFFSET, DelayRegister);

		Status = (s32)XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* @brief Gets the delay settings for an SPI device.
*
* The delay register controls the Delay Between Transfers, Delay After
* Transfers, and the Delay Initially. The default value is 0x0.
*
* @param[in]	InstancePtr Pointer to the XSpiPs instance.
* @param[out]	DelayNss Pointer to the delay for which the chip select
*		outputs are de-asserted between words when CPHA=0.
* @param[out]	DelayBtwn Pointer to the Delay Between transfers value.
* @param[out]	DelayAfter Pointer to the Delay After transfer value.
* @param[out]	DelayInit Pointer to the Delay Initially value.
*
******************************************************************************/
void XSpiPs_GetDelays(const XSpiPs *InstancePtr,u8 *DelayNss, u8 *DelayBtwn,
			u8 *DelayAfter, u8 *DelayInit)
{
	u32 DelayRegister;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	DelayRegister = XSpiPs_ReadReg(InstancePtr->Config.BaseAddress,
					XSPIPS_DR_OFFSET);

	*DelayInit = (u8)(DelayRegister & XSPIPS_DR_INIT_MASK);

	*DelayAfter = (u8)((DelayRegister & XSPIPS_DR_AFTER_MASK) >>
				 XSPIPS_DR_AFTER_SHIFT);

	*DelayBtwn = (u8)((DelayRegister & XSPIPS_DR_BTWN_MASK) >>
				XSPIPS_DR_BTWN_SHIFT);

	*DelayNss = (u8)((DelayRegister & XSPIPS_DR_NSS_MASK) >>
				XSPIPS_DR_NSS_SHIFT);

}
/** @} */
