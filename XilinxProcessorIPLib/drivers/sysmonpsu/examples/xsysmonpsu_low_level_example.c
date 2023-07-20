/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsu_low_level_example.c
*
* This file contains a design example using the basic driver functions
* of the System Monitor driver. The example here shows using the
* driver/device in polled mode to check the on-chip temperature and voltages.
*
*
* @note
*
* The values of the on-chip temperature and the on-chip Vccaux voltage are read
* from the device and then the alarm thresholds are set in such a manner that
* the alarms occur.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   kvn    12/15/15 First release
*       mn     03/08/18 Update code to run at higher frequency
*
* 2.6   aad    11/21/19 Removed reading of AUX channels
* 2.9   cog    07/20/23 Added support for SDT flow
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#include "xsysmonpsu_hw.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xstatus.h"

/************************** Constant Definitions ****************************/
#ifndef SDT
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_BASEADDR		XPAR_XSYSMONPSU_0_BASEADDR
#else
#define SYSMON_BASEADDR		0xffa50000
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonPsuLowLevelExample(u32 BaseAddress);

/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* Main function that invokes the example given in this file.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the SysMonitor Low level example, specify the Base Address that
	 * is generated in xparameters.h.
	 */
	Status = SysMonPsuLowLevelExample(SYSMON_BASEADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("SysMonPsu low level Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SysMonPsu low level Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* basic driver functions.
* The function does the following tasks:
*	- Reset the device
*	- Setup alarm thresholds for on-chip temperature and VCCAUX.
*	- Setup sequence registers to continuously monitor on-chip temperature
*	 and VCCAUX.
*	- Setup configuration registers to start the sequence.
*	- Read latest on-chip temperature and VCCAUX, as well as their maximum
*	 and minimum values. Also check if alarm(s) are set.
*
* @param	BaseAddress is the XPAR_<SYSMON_instance>_BASEADDRESS value
*		from xparameters.h.
*
* @return	XST_SUCCESS
*
* @note		None.
*
****************************************************************************/
int SysMonPsuLowLevelExample(u32 BaseAddress)
{
	u32 RegValue;
	u16 TempData;
	u16 VccauxData;
	u32 IntrStatusRegister;

	/* Reset the PS SysMon device. */
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_VP_VN_OFFSET, XSYSMONPSU_VP_VN_MASK);

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG1_OFFSET) &
			(~ XSYSMONPSU_CFG_REG1_SEQ_MDE_MASK);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG1_OFFSET,	RegValue |
			(XSM_SEQ_MODE_SAFE << XSYSMONPSU_CFG_REG1_SEQ_MDE_SHIFT));


	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG0_OFFSET) & (~XSYSMONPSU_CFG_REG0_AVRGNG_MASK);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG0_OFFSET,
				RegValue | (XSM_AVG_16_SAMPLES << XSYSMONPSU_CFG_REG0_AVRGNG_SHIFT));


	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 */
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_SEQ_AVERAGE0_OFFSET,
			(XSYSMONPSU_SEQ_CH0_TEMP_MASK | XSYSMONPSU_SEQ_CH0_SUP3_MASK));


	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 */
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_SEQ_CH0_OFFSET,
			(XSYSMONPSU_SEQ_CH0_TEMP_MASK | XSYSMONPSU_SEQ_CH0_SUP3_MASK));


	/* Clear any bits set in the Interrupt Status Register. */
	IntrStatusRegister = XSysmonPsu_ReadReg(BaseAddress + XSYSMONPSU_ISR_0_OFFSET);
	XSysmonPsu_WriteReg(BaseAddress + XSYSMONPSU_ISR_0_OFFSET, IntrStatusRegister);
	IntrStatusRegister = XSysmonPsu_ReadReg(BaseAddress + XSYSMONPSU_ISR_1_OFFSET);
	XSysmonPsu_WriteReg(BaseAddress + XSYSMONPSU_ISR_1_OFFSET, IntrStatusRegister);

	/* Enable the Channel Sequencer in continuous sequencer cycling mode. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG1_OFFSET) & (~ XSYSMONPSU_CFG_REG1_SEQ_MDE_MASK);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG1_OFFSET,	RegValue |
			XSM_SEQ_MODE_CONTINPASS << XSYSMONPSU_CFG_REG1_SEQ_MDE_SHIFT);


	/* Wait till the End of Sequence occurs */
	IntrStatusRegister = XSysmonPsu_ReadReg(BaseAddress + XSYSMONPSU_ISR_1_OFFSET);
	while ((IntrStatusRegister & XSYSMONPSU_ISR_1_EOS_MASK) != XSYSMONPSU_ISR_1_EOS_MASK) {
		IntrStatusRegister = XSysmonPsu_ReadReg(BaseAddress + XSYSMONPSU_ISR_1_OFFSET);
	}

	/* Read the current value of the on-chip temperature. */
	TempData = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_TEMP_OFFSET);

	/* Read the current value of the on-chip VCCAUX voltage. */
	VccauxData = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_SUP3_OFFSET);

	/* Disable all the alarms in the Configuration Register 1. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_CFG_REG1_OFFSET);
	RegValue |= XSYSMONPSU_CFG_REG1_ALRM_ALL_MASK;
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_CFG_REG1_OFFSET,
			RegValue);

	/*
	 * Setup Alarm threshold registers for
	 * On-chip Temperature High limit
	 * VCCAUX High limit
	 * VCCAUX Low limit
	 */
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_ALRM_TEMP_UPR_OFFSET,
					TempData - 0x6F);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_ALRM_SUP3_UPR_OFFSET,
					VccauxData - 0x6F);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_ALRM_SUP3_LWR_OFFSET,
					VccauxData + 0x6F);

	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 2 for on-chip
	 * VCCAUX in the Configuration Register 1.
	 */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET +
			XSYSMONPSU_CFG_REG1_OFFSET) & (~XSYSMONPSU_CFG_REG1_ALRM_ALL_MASK);
	RegValue |= ((~(XSYSMONPSU_CFR_REG1_ALRM_SUP3_MASK | XSYSMONPSU_CFR_REG1_ALRM_TEMP_MASK)) &
			XSYSMONPSU_CFG_REG1_ALRM_ALL_MASK);
	XSysmonPsu_WriteReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_CFG_REG1_OFFSET,
			RegValue);

	/* Read the current value of on-chip temperature. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_TEMP_OFFSET);

	/* Read the Maximum value of on-chip temperature. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_MAX_TEMP_OFFSET);

	/* Read the Minimum value of on-chip temperature. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_MIN_TEMP_OFFSET);

	/* Check if alarm for on-chip temperature is set. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_STS_FLAG_OFFSET) &
			1U << XSYSMONPSU_STS_FLAG_ALM_2_0_SHIFT;
	if (RegValue) {
		/*
		 * Alarm for on-chip temperature is set.
		 * The required processing should be put here.
		 */
	}


	/* Read the current value of on-chip VCCAUX. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_SUP3_OFFSET);

	/* Read the Maximum value of on-chip VCCAUX. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_MAX_SUP3_OFFSET);

	/* Read the Minimum value of on-chip VCCAUX. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_MIN_SUP3_OFFSET);

	/* Check if the alarm for on-chip VCCAUX is set. */
	RegValue = XSysmonPsu_ReadReg(BaseAddress + XPS_BA_OFFSET + XSYSMONPSU_STS_FLAG_OFFSET) &
				1U << XSYSMONPSU_STS_FLAG_ALM_6_3_WIDTH;
	if (RegValue) {
		/*
		 * Alarm for on-chip VCCAUX is set.
		 * The required processing should be put here.
		 */
	}

	return XST_SUCCESS;
}
