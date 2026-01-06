/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiic_options.c
* @addtogroup Overview
* @{
*
* Contains options functions for the XIic component. This file is not required
* unless the functions in this file are called.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- --- ------- -----------------------------------------------
* 1.01b jhl 3/26/02 repartioned the driver
* 1.01c ecm 12/05/02 new rev
* 1.13a wgr 03/22/07 Converted to new coding style.
* 2.00a ktn 10/22/09 Converted all register accesses to 32 bit access.
* 		     Updated to use the HAL APIs/macros.
* 3.15  vlt 12/04/25 Add XIic_SetClk API for dynamic clock configuration.
* </pre>
*
****************************************************************************/

/***************************** Include Files *******************************/

#include "xiic.h"
#include "xiic_i.h"

/************************** Constant Definitions ***************************/


/**************************** Type Definitions *****************************/
#ifdef SDT
struct TimingRegs {
        unsigned int Tsusta;
        unsigned int Tsusto;
        unsigned int Thdsta;
        unsigned int Tsudat;
        unsigned int Tbuf;
};
#endif


/***************** Macros (Inline Functions) Definitions *******************/
#define XIIC_REG_VALUES_100KHZ   0
#define XIIC_REG_VALUES_400KHZ   1
#define XIIC_REG_VALUES_1MHZ     2

#define XIIC_STANDARD_MODE   100000U
#define XIIC_FAST_MODE       400000U
#define XIIC_FAST_MODE_PLUS  1000000U

#define XIIC_NANOSEC_TO_SEC_DIVISOR    1000000000U


/************************** Function Prototypes ****************************/


/************************** Variable Definitions **************************/
#ifdef SDT
static const struct TimingRegs TimingRegValues[] = {
        { 5700, 5000, 4300, 550, 5000 }, /* Reg values for 100KHz */
        { 900, 900, 900, 400, 1600 },    /* Reg values for 400KHz */
        { 380, 380, 380, 170, 620 },     /* Reg values for 1MHz   */
};
#endif


/*****************************************************************************/
/**
*
* This function sets the options for the IIC device driver. The options control
* how the device behaves relative to the IIC bus. If an option applies to
* how messages are sent or received on the IIC bus, it must be set prior to
* calling functions which send or receive data.
*
* To set multiple options, the values must be ORed together. To not change
* existing options, read/modify/write with the current options using
* XIic_GetOptions().
*
* <b>USAGE EXAMPLE:</b>
*
* Read/modify/write to enable repeated start:
* <pre>
*   u8 Options;
*   Options = XIic_GetOptions(&Iic);
*   XIic_SetOptions(&Iic, Options | XII_REPEATED_START_OPTION);
* </pre>
*
* Disabling General Call:
* <pre>
*   Options = XIic_GetOptions(&Iic);
*   XIic_SetOptions(&Iic, Options &= ~XII_GENERAL_CALL_OPTION);
* </pre>
*
* @param	InstancePtr is a pointer to the XIic instance to be worked on.
* @param	NewOptions are the options to be set.  See xiic.h for a list of
*		the available options.
*
* @return	None.
*
* @note
*
* Sending or receiving messages with repeated start enabled, and then
* disabling repeated start, will not take effect until another master
* transaction is completed. i.e. After using repeated start, the bus will
* continue to be throttled after repeated start is disabled until a master
* transaction occurs allowing the IIC to release the bus.
* <br><br>
* Options enabled will have a 1 in its appropriate bit position.
*
****************************************************************************/
void XIic_SetOptions(XIic *InstancePtr, u32 NewOptions)
{
	u32 CntlReg;

	Xil_AssertVoid(InstancePtr != NULL);

	XIic_IntrGlobalDisable(InstancePtr->BaseAddress);

	/*
	 * Update the options in the instance and get the contents of the
	 * control register such that the general call option can be modified.
	 */
	InstancePtr->Options = NewOptions;
	CntlReg = XIic_ReadReg(InstancePtr->BaseAddress, XIIC_CR_REG_OFFSET);

	/*
	 * The general call option is the only option that maps directly to
	 * a hardware register feature.
	 */
	if (NewOptions & XII_GENERAL_CALL_OPTION) {
		CntlReg |= XIIC_CR_GENERAL_CALL_MASK;
	} else {
		CntlReg &= ~XIIC_CR_GENERAL_CALL_MASK;
	}

	/*
	 * Write the new control register value to the register.
	 */
	XIic_WriteReg(InstancePtr->BaseAddress, XIIC_CR_REG_OFFSET, CntlReg);

	XIic_IntrGlobalEnable(InstancePtr->BaseAddress);
}

/*****************************************************************************/
/**
*
* This function gets the current options for the IIC device. Options control
* the how the device behaves on the IIC bus. See SetOptions for more information
* on options.
*
* @param	InstancePtr is a pointer to the XIic instance to be worked on.
*
* @return	The options of the IIC device. See xiic.h for a list of
*		available options.
*
* @note
*
* Options enabled will have a 1 in its appropriate bit position.
*
****************************************************************************/
u32 XIic_GetOptions(XIic *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return InstancePtr->Options;
}

/*****************************************************************************/
/**
 * @brief
 * Sets the serial clock rate for the IIC device.
 * The device must be idle before setting these device options.
 * The timing register values are calculated according to the input clock
 * frequency and configured scl frequency. For details, please refer the
 * AXI I2C PG and NXP I2C Spec.
 *
 *@param    InstancePtr is a pointer to the XIic instance to be worked on.
 *@param    FsclHz is Clock frequency in Hz(100kHz, 400kHz, 1MHz).
 *
 *@return
 *              - XST_SUCCESS if options are successfully set.
 *              - XST_DEVICE_IS_STARTED if the device is currently transferring
 *              data. The transfer must complete or be aborted before setting
 *              options.
 *              - XST_INVALID_PARAM if frequency is unsupported or invalid.
 *
 *
 * **************************************************************************/
#ifdef SDT
u32 XIic_SetClk(XIic *InstancePtr, u32 FsclHz)
{
	u64 ClkInMhz;
	u32 TimingIndex= 0U;
	u32 RegVal;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	if (XIic_ReadReg(InstancePtr->BaseAddress, XIIC_DTR_REG_OFFSET) != 0U) {
		return XST_DEVICE_IS_STARTED;
	}
	/* Select timing register values based on target SCL frequency */
	if (FsclHz <=XIIC_STANDARD_MODE) {           /* Standard-mode */
	TimingIndex = XIIC_REG_VALUES_100KHZ;
	} else if (FsclHz <= XIIC_FAST_MODE) {       /* Fast-mode */
	TimingIndex= XIIC_REG_VALUES_400KHZ;
	} else if (FsclHz <=XIIC_FAST_MODE_PLUS) {   /* Fast-mode Plus */
	TimingIndex = XIIC_REG_VALUES_1MHZ;
	} else {
	return XST_INVALID_PARAM;
	}
	/*
	* Calculate SCL HIGH and LOW period register values.
	* Formula:((AXI Clock frequency in Hz)/(2 × IIC frequency in Hz)) – 7 – SCL_INERTIAL_DELAY
	*/
	RegVal = (u32)(InstancePtr->AxiClkFreq / (2U * FsclHz)) - 7U - InstancePtr->SerialClkDelay;
	if (RegVal == 0U) {
		return XST_INVALID_PARAM;
	}
	/* Configure SCL high and low period */
	XIic_Out32(InstancePtr->BaseAddress + XIIC_THIGH_REG_OFFSET, RegVal - 1U);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_TLOW_REG_OFFSET, RegVal - 1U);
	/* Convert nanoseconds to clock cycles:(ns × MHz) / 1000 */
	/* Configure START condition setup time */
	ClkInMhz = (u64)TimingRegValues[TimingIndex].Tsusta * InstancePtr->AxiClkFreq;
	RegVal = (u32)(ClkInMhz / XIIC_NANOSEC_TO_SEC_DIVISOR);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_TSUSTA_REG_OFFSET, RegVal - 1U);
	/* Configure STOP condition setup time */
	ClkInMhz = (u64)TimingRegValues[TimingIndex].Tsusto * InstancePtr->AxiClkFreq;
	RegVal = (u32)(ClkInMhz / XIIC_NANOSEC_TO_SEC_DIVISOR);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_TSUSTO_REG_OFFSET, RegVal - 1U);
	/* Configure START condition hold time */
	ClkInMhz = (u64)TimingRegValues[TimingIndex].Thdsta * InstancePtr->AxiClkFreq;
	RegVal = (u32)(ClkInMhz / XIIC_NANOSEC_TO_SEC_DIVISOR);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_THDSTA_REG_OFFSET, RegVal - 1U);
	/* Configure data setup time */
	ClkInMhz = (u64)TimingRegValues[TimingIndex].Tsudat * InstancePtr->AxiClkFreq;
	RegVal = (u32)(ClkInMhz / XIIC_NANOSEC_TO_SEC_DIVISOR);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_TSUDAT_REG_OFFSET, RegVal - 1U);
	/* Configure bus free time between STOP and START */
	ClkInMhz = (u64)TimingRegValues[TimingIndex].Tbuf * InstancePtr->AxiClkFreq;
	RegVal = (u32)(ClkInMhz / XIIC_NANOSEC_TO_SEC_DIVISOR);
	XIic_Out32(InstancePtr->BaseAddress + XIIC_TBUF_REG_OFFSET, RegVal - 1U);
	/* Configure data hold time */
	XIic_Out32(InstancePtr->BaseAddress + XIIC_THDDAT_REG_OFFSET, 1U);

	return XST_SUCCESS;
}
#endif
/** @} */
