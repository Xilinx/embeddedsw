/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xsysmonpsu_ams_example.c
*
* This file contains a design example using the driver functions of the
* System Monitor driver.
* This example here shows the usage of the driver/device in single channel
* sequencer off mode to measure AMS block voltages.
*
*
* @note
*
* This code assumes that no Operating System is being used.
*
* The value of the on-chip Vccint voltage is read from the device and then the
* alarm thresholds are set in such a manner that the alarm occurs.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   mn     12/13/17 First release
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmonpsu.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "sleep.h"


/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#define SYSMON_DEVICE_ID	XPAR_XSYSMONPSU_0_DEVICE_ID
#define SCUGIC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID			XPAR_XSYSMONPSU_INTR


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonPsuAMSExample(XScuGic* XScuGicInstancePtr,
			XSysMonPsu* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId);
static int SysMonPsuFractionToInt(float FloatNum);



/************************** Variable Definitions ****************************/

static XSysMonPsu SysMonInst; 	  /* System Monitor driver instance */
static XScuGic InterruptController; /* Instance of the XScuGic driver. */

/* Shared variables used to test the callbacks. */
volatile static int EocFlag = FALSE;	  	/* EOC interrupt */
volatile static int VccintIntr = FALSE;	  	/* VCCINT alarm interrupt */


/****************************************************************************/
/**
*
* Main function that invokes the Single Channel Interrupt example.
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

	xil_printf("Entering Sysmon AMS Example Test\r\n");
	/*
	 * Run the SysMonitor interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	Status = SysMonPsuAMSExample(&InterruptController,
				   &SysMonInst,
				   SYSMON_DEVICE_ID,
				   INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("\r\nSysmon AMS Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\r\nSuccessfully ran Sysmon AMS Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarm for VCCINT
*	- Set up the configuration registers for single channel continuous mode
*	for VCCINT channel
*	- Setup interrupt system
*	- Enable interrupts
*	- Wait until the VCCINT alarm interrupt occurs
*
* @param	XScuGicInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<SYSMON_instance>_VEC_ID
*		value from xparameters_ps.h
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int SysMonPsuAMSExample(XScuGic* XScuGicInstancePtr,
					XSysMonPsu* SysMonInstPtr,
					u16 SysMonDeviceId,
					u16 SysMonIntrId)
{
	int Status;
	XSysMonPsu_Config *ConfigPtr;
	u32 VccPsPllRawData;
	u32 VccPsBattRawData;
	u32 VccIntRawData;
	u32 VccBramRawData;
	u32 VccAuxRawData;
	u32 VccPsDdrPllRawData;
	u32 VccPsDdrPhyRawData;
	u32 VccPsIntFpRawData;
	float VccPsPllData;
	float VccPsBattData;
	float VccIntData;
	float VccBramData;
	float VccAuxData;
	float VccPsDdrPllData;
	float VccPsDdrPhyData;
	float VccPsIntFpData;

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsu_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMonPsu_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/* Self Test the System Monitor device. */
	Status = XSysMonPsu_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set the sequencer in Single channel mode. */
	XSysMonPsu_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SINGCHAN, XSYSMON_PS);

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_PSPLL channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCC_PSLL0,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccPsPllRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCC_PSLL0, XSYSMON_AMS);
	if (VccPsPllRawData == 0U)
		return XST_FAILURE;

	VccPsPllData = XSysMonPsu_RawToVoltage(VccPsPllRawData);
	xil_printf("\r\nThe VCC_PSPLL is %0d.%03d Volts\r\n", (int)(VccPsPllData), SysMonPsuFractionToInt(VccPsPllData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_PSBATT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCC_PSLL3,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccPsBattRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCC_PSLL3, XSYSMON_AMS);
	if (VccPsBattRawData == 0U)
		return XST_FAILURE;

	VccPsBattData = XSysMonPsu_RawToVoltage(VccPsBattRawData);
	xil_printf("\r\nThe VCC_PSBATT is %0d.%03d Volts\r\n", (int)(VccPsBattData), SysMonPsuFractionToInt(VccPsBattData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCCINT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCCINT,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccIntRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT, XSYSMON_AMS);
	if (VccIntRawData == 0U)
		return XST_FAILURE;

	VccIntData = XSysMonPsu_RawToVoltage(VccIntRawData);
	xil_printf("\r\nThe VCCINT is %0d.%03d Volts\r\n", (int)(VccIntData), SysMonPsuFractionToInt(VccIntData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCCBRAM channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCCBRAM,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccBramRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCCBRAM, XSYSMON_AMS);
	if (VccBramRawData == 0U)
		return XST_FAILURE;

	VccBramData = XSysMonPsu_RawToVoltage(VccBramRawData);
	xil_printf("\r\nThe VCCBRAM is %0d.%03d Volts\r\n", (int)(VccBramData), SysMonPsuFractionToInt(VccBramData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_PSBATT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCCAUX,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccAuxRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX, XSYSMON_AMS);
	if (VccAuxRawData == 0U)
		return XST_FAILURE;

	VccAuxData = XSysMonPsu_RawToVoltage(VccAuxRawData);
	xil_printf("\r\nThe VCCAUX is %0d.%03d Volts\r\n", (int)(VccAuxData), SysMonPsuFractionToInt(VccAuxData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_PSBATT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_VCC_PSDDRPLL,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccPsDdrPllRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_VCC_PSDDRPLL, XSYSMON_AMS);
	if (VccPsDdrPllRawData == 0U)
		return XST_FAILURE;

	VccPsDdrPllData = XSysMonPsu_RawToVoltage(VccPsDdrPllRawData);
	xil_printf("\r\nThe VCC_PSDDRPLL is %0d.%03d Volts\r\n", (int)(VccPsDdrPllData), SysMonPsuFractionToInt(VccPsDdrPllData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_DDRPHY_REF channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_DDRPHY_VREF,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccPsDdrPhyRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_DDRPHY_VREF, XSYSMON_AMS);
	if (VccPsDdrPhyRawData == 0U)
		return XST_FAILURE;

	VccPsDdrPhyData = XSysMonPsu_RawToVoltage(VccPsDdrPhyRawData);
	xil_printf("\r\nThe VCC_PSDDRPHY_REF is %0d.%03d Volts\r\n", (int)(VccPsDdrPhyData), SysMonPsuFractionToInt(VccPsDdrPhyData));

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCC_PSBATT channel.
	 */
	Status=  XSysMonPsu_SetSingleChParams(SysMonInstPtr, XSM_CH_RESERVE1,
						FALSE, FALSE, FALSE, XSYSMON_PS);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	usleep(1000);

	VccPsIntFpRawData = XSysMonPsu_GetAdcData(SysMonInstPtr, XSM_CH_RESERVE1, XSYSMON_AMS);
	if (VccPsIntFpRawData == 0U)
		return XST_FAILURE;

	VccPsIntFpData = XSysMonPsu_RawToVoltage(VccPsIntFpRawData);
	xil_printf("\r\nThe VCC_PSINTFP_DDR is %0d.%03d Volts\r\n", (int)(VccPsIntFpData), SysMonPsuFractionToInt(VccPsIntFpData));

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function converts the fraction part of the given floating point number
* (after the decimal point)to an integer.
*
* @param        FloatNum is the floating point number.
*
* @return       Integer number to a precision of 3 digits.
*
* @note
* This function is used in the printing of floating point data to a STDIO
* device using the xil_printf function. The xil_printf is a very small
* foot-print printf function and does not support the printing of floating
* point numbers.
*
*****************************************************************************/
int SysMonPsuFractionToInt(float FloatNum)
{
        float Temp;

        Temp = FloatNum;
        if (FloatNum < 0) {
                Temp = -(FloatNum);
        }

        return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}
