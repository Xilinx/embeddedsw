/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xsysmon.c
* @addtogroup sysmon_v7_5
* @{
*
* This file contains the driver API functions that can be used to access
* the System Monitor/ADC device.
*
* Refer to the xsysmon.h header file for more information about this driver.
*
* @note 	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 2.00a sv     07/07/08 Modified the ADC data functions to return 16 bits of
*						data.
* 3.00a sdm    09/02/09 Added APIs for V6 SysMon.
* 4.00a ktn    10/22/09 Updated the file to use the HAL processor APIs/macros.
*			The macros have been renamed to remove _m from the name
*			of the macro.
* 5.00a sdm    06/15/11 Added support for XADC (7 Series Families).
* 5.01a bss    02/28/12 Added support for Zynq.
* 5.02a bss    11/23/12 Added XSysMon_EnableTempUpdate,
*			XSysMon_DisableTempUpdate and XSysMon_SetTempWaitCycles
*			APIs (CR #679872)
* 5.03a bss    04/25/13 Modified XSysMon_SetSeqChEnables,
*			XSysMon_SetSeqAvgEnables, XSysMon_SetSeqInputMode
*			and XSysMon_SetSeqAcqTime APIs to check for Safe Mode
*			instead of Single Channel mode. CR #703729
* 7.0	bss    7/25/14  Modified XSysMon_GetAdcData,
*			XSysMon_GetMinMaxMeasurement,
*			XSysMon_SetSingleChParams, XSysMon_SetAlarmEnables,
*			XSysMon_GetAlarmEnables,XSysMon_SetSeqChEnables,
*			XSysMon_GetSeqChEnables,XSysMon_SetSeqAvgEnables,
*			XSysMon_GetSeqAvgEnables,XSysMon_SetAlarmThreshold
*			and XSysMon_GetAlarmThreshold to support Ultrascale
* 7.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XSysMon_CfgInitialize API.
* 7.2   asa Made changes to use XSM_CFR3_OFFSET (configuration register 3)
*           only for Ultrascale. Changes were made in APIs XSysMon_SetAlarmEnables
*           and XSysMon_GetAlarmEnables. This is to fix CR#910905.
* 7.5	mn    07/06/18  Fixed Doxygen warnings
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xsysmon.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/

/*****************************************************************************/
/**
*
* This function initializes a specific XSysMon device/instance. This function
* must be called prior to using the System Monitor/ADC device.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	ConfigPtr points to the XSysMon device configuration structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		The user needs to first call the XSysMon_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XSysMon_CfgInitialize() API.
*
******************************************************************************/
int XSysMon_CfgInitialize(XSysMon *InstancePtr, XSysMon_Config *ConfigPtr,
			  UINTPTR EffectiveAddr)
{
	/*
	 * Assert the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set the values read from the device config and the base address.
	 */
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.IncludeInterrupt = ConfigPtr->IncludeInterrupt;

	/* Store the default Waitcycles value in Mask */
	InstancePtr->Mask = XSM_CONVST_WAITCYCLES_DEFAULT <<
					XSM_CONVST_WAITCYCLES_SHIFT;

	/*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the device such that it is in a known state.
	 */
	XSysMon_Reset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function forces the software reset of the complete SystemMonitor/ADC
* Hard Macro and the SYSMON ADC Core Logic.
*
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		The Control registers in the SystemMonitor/ADC Hard Macro
*		are not affected by this reset, only the Status registers
*		are reset.
*		Refer to the device data sheet for the device status and
*		register values after the reset.
*		Use the XSysMon_ResetAdc() to reset only the SystemMonitor/ADC
*		Hard Macro.
*
******************************************************************************/
void XSysMon_Reset(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write the reset value to the Software Reset Register (SRR) to
	 * Reset the device.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_SRR_OFFSET,
			 XSM_SRR_IPRST_MASK);
}

/****************************************************************************/
/**
*
* The functions reads the contents of the Status Register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	A 32-bit value representing the contents of the Status Register.
*		Use the XSM_SR_*_MASK constants defined in xsysmon_hw.h to
*		interpret the returned value.
*
* @note		None.
*
*****************************************************************************/
u32 XSysMon_GetStatus(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the Status Register and return the value.
	 */
	return XSysMon_ReadReg(InstancePtr->Config.BaseAddress, XSM_SR_OFFSET);
}

/****************************************************************************/
/**
*
* This function reads the contents of Alarm Output Register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	A 32-bit value read from the Alarm Output Register.
*		Use the XSM_AOR_*_MASK constants defined in xsysmon_hw.h to
*		interpret the value.
*
* @note		None.
*
*****************************************************************************/
u32 XSysMon_GetAlarmOutputStatus(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the Alarm Output Register and return the value.
	 */
	return (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
		XSM_AOR_OFFSET) & XSM_AOR_ALARM_ALL_MASK);
}

/****************************************************************************/
/**
*
* This function starts the ADC conversion in the Single Channel event driven
* sampling mode. The EOC bit in Status Register will be set once the conversion
* is finished. Refer to the device specification for more details.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		The default state of the CONVST bit is a logic 0. The conversion
*		is started when the CONVST bit is set to 1 from 0.
*		This bit is cleared in this function so that the next conversion
*		can be started by setting this bit.
*
*****************************************************************************/
void XSysMon_StartAdcConversion(XSysMon *InstancePtr)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Modify only CONVST bit using the InstancePtr->Mask, which
	   stores previously written value in CONVST register */

	RegVal = (InstancePtr->Mask) | XSM_CONVST_CONVST_MASK;

	/*
	 * Start the conversion by setting the CONVST bit to 1 and to 0.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_CONVST_OFFSET, RegVal);

	RegVal = (InstancePtr->Mask) & ~(XSM_CONVST_CONVST_MASK);

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_CONVST_OFFSET, RegVal);
}

/*****************************************************************************/
/**
*
* This function resets the SystemMonitor/ADC Hard Macro in the device.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		The Control registers in the SystemMonitor/ADC Hard Macro
*		are not affected by this reset, only the Status registers
*		are reset.
*		This reset causes the ADC to begin with a new conversion.
* 		Refer to the device data sheet for the device status and
*		register values after the reset.
* 		Use the XSysMon_Reset() API to reset both the SystemMonitor/ADC
* 		Hard Macro and the SYSMON ADC Core Logic.
*
******************************************************************************/
void XSysMon_ResetAdc(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set the reset bit to the ADC Reset Register (ARR) to
	 * put the SystemMonitor/ADC Hard Macro in Reset.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_ARR_OFFSET,
			 XSM_ARR_RST_MASK);
	/*
	 * Clear the reset bit to the ADC Reset Register (ARR) to
	 * release the reset of SystemMonitor/ADC Hard Macro.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_ARR_OFFSET, 0x0);
}

/****************************************************************************/
/**
*
* Get the ADC converted data for the specified channel.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Channel is the channel number. Use the XSM_CH_* defined in
*		the file xsysmon.h.
*		The valid channels are 0 to 5 and 16 to 31 for all the device
*		families. Channel 6 is valid for 7 Series and Zynq.
*		Channel 13, 14, 15 are valid for Zynq. 32 to 35 are valid for
*		Ultrascale.
*
* @return	A 16-bit value representing the ADC converted data for the
*		specified channel. The System Monitor/ADC device guarantees
* 		a 10 bit resolution for the ADC converted data and data is the
*		10 MSB bits of the 16 data read from the device.
*
* @note		The channels 7,8,9 are used for calibration of the device and
*           	hence there is no associated data with this channel.
*		Please make sure that the proper channel number is passed.
*
*****************************************************************************/
u16 XSysMon_GetAdcData(XSysMon *InstancePtr, u8 Channel)
{
	u16 AdcData;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Channel <= XSM_CH_VBRAM) ||
			  ((Channel >= XSM_CH_VCCPINT) &&
			  (Channel <= XSM_CH_AUX_MAX)) ||
			  ((Channel >= XSM_CH_VUSR0) &&
			  (Channel <= XSM_CH_VUSR3)));

	/*
	 * Read the selected ADC converted data for the specified channel
	 * and return the value.
	 */
	if (Channel <= XSM_CH_AUX_MAX) {
		AdcData = (u16) (XSysMon_ReadReg(InstancePtr->
				Config.BaseAddress, XSM_TEMP_OFFSET +
				(Channel << 2)));
	} else {
		AdcData = (u16) (XSysMon_ReadReg(InstancePtr->
				Config.BaseAddress, XSM_VUSR0_OFFSET +
				((Channel - XSM_CH_VUSR0) << 2)));
	}

	return AdcData;
}

/****************************************************************************/
/**
*
* This function gets the calibration coefficient data for the specified
* parameter.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	CoeffType specifies the calibration coefficient
*		to be read. Use XSM_CALIB_* constants defined in xsysmon.h to
*		specify the calibration coefficient to be read.
*
* @return	A 16-bit value representing the calibration coefficient.
*		The System Monitor/ADC device guarantees a 10 bit resolution for
*		the ADC converted data and data is the 10 MSB bits of the 16
*		data read from the device.
*
* @note		None.
*
*****************************************************************************/
u16 XSysMon_GetCalibCoefficient(XSysMon *InstancePtr, u8 CoeffType)
{
	u16 CalibData;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CoeffType <= XSM_CALIB_GAIN_ERROR_COEFF);

	/*
	 * Read the selected calibration coefficient.
	 */
	CalibData = (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			  XSM_SUPPLY_CALIB_OFFSET + (CoeffType << 2));

	return CalibData;
}

/****************************************************************************/
/**
*
* This function reads the Minimum/Maximum measurement for one of the
* following parameters :
*		- Minimum Temperature (XSM_MIN_TEMP) - All families
*		- Minimum VCCINT (XSM_MIN_VCCINT) - All families
*		- Minimum VCCAUX (XSM_MIN_VCCAUX) - All families
*		- Maximum Temperature (XSM_MAX_TEMP) - All families
*		- Maximum VCCINT (XSM_MAX_VCCINT) - All families
*		- Maximum VCCAUX (XSM_MAX_VCCAUX) - All families
*		- Maximum VCCBRAM (XSM_MAX_VCCBRAM) - 7 series and Zynq only
*		- Minimum VCCBRAM (XSM_MIN_VCCBRAM) - 7 series and Zynq only
* 		- Maximum VCCPINT (XSM_MAX_VCCPINT) - Zynq only
* 		- Maximum VCCPAUX (XSM_MAX_VCCPAUX) - Zynq only
* 		- Maximum VCCPDRO (XSM_MAX_VCCPDRO) - Zynq only
* 		- Minimum VCCPINT (XSM_MIN_VCCPINT) - Zynq only
* 		- Minimum VCCPAUX (XSM_MIN_VCCPAUX) - Zynq only
* 		- Minimum VCCPDRO (XSM_MIN_VCCPDRO) - Zynq only
*		- Maximum VUSER0 (XSM_MAX_VUSR0) - Ultrascale
*		- Maximum VUSER1 (XSM_MAX_VUSR1) - Ultrascale
*		- Maximum VUSER2 (XSM_MAX_VUSR2) - Ultrascale
*		- Maximum VUSER3 (XSM_MAX_VUSR3) - Ultrascale
*		- Minimum VUSER0 (XSM_MIN_VUSR0) - Ultrascale
*		- Minimum VUSER1 (XSM_MIN_VUSR1) - Ultrascale
*		- Minimum VUSER2 (XSM_MIN_VUSR2) - Ultrascale
*		- Minimum VUSER3 (XSM_MIN_VUSR3) - Ultrascale
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	MeasurementType specifies the parameter for which the
*		Minimum/Maximum measurement has to be read.
*		Use XSM_MAX_* and XSM_MIN_* constants defined in xsysmon.h to
*		specify the data to be read.
*
* @return	A 16-bit value representing the maximum/minimum measurement for
*		specified parameter.
*		The System Monitor/ADC device guarantees a 10 bit resolution for
*		the ADC converted data and data is the 10 MSB bits of  16 bit
*		data read from the device.
*
*****************************************************************************/
u16 XSysMon_GetMinMaxMeasurement(XSysMon *InstancePtr, u8 MeasurementType)
{
	u16 MinMaxData;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((MeasurementType <= XSM_MAX_VCCPDRO) ||
			((MeasurementType >= XSM_MIN_VCCPINT) &&
			(MeasurementType <= XSM_MIN_VCCPDRO)) ||
			((MeasurementType >= XSM_MAX_VUSR0) &&
			(MeasurementType <= XSM_MAX_VUSR3)) ||
			((MeasurementType >= XSM_MIN_VUSR0) &&
			(MeasurementType <= XSM_MIN_VUSR3)))


	/*
	 * Read and return the specified Minimum/Maximum measurement.
	 */
	MinMaxData = (u16) (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			    XSM_MAX_TEMP_OFFSET + (MeasurementType << 2)));
	return MinMaxData;
}

/****************************************************************************/
/**
*
* This function sets the number of samples of averaging that is to be done for
* all the channels in both the single channel mode and sequence mode of
* operations.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Average is the number of samples of averaging programmed to the
*		Configuration Register 0. Use the XSM_AVG_* definitions defined
*		in xsysmon.h file :
*		- XSM_AVG_0_SAMPLES for no averaging
*		- XSM_AVG_16_SAMPLES for 16 samples of averaging
*		- XSM_AVG_64_SAMPLES for 64 samples of averaging
*		- XSM_AVG_256_SAMPLES for 256 samples of averaging
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMon_SetAvg(XSysMon *InstancePtr, u8 Average)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Average <= XSM_AVG_256_SAMPLES);

	/*
	 * Write the averaging value into the Configuration Register 0.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				   XSM_CFR0_OFFSET) &
				   (~XSM_CFR0_AVG_VALID_MASK);
	RegValue |= (((u32) Average << XSM_CFR0_AVG_SHIFT));
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR0_OFFSET,
			 RegValue);
}

/****************************************************************************/
/**
*
* This function returns the number of samples of averaging configured for all
* the channels in the Configuration Register 0.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	The averaging read from the Configuration Register 0 is
*		returned. Use the XSM_AVG_* bit definitions defined in xsysmon.h
*		file to interpret the returned value :
*		- XSM_AVG_0_SAMPLES means no averaging
*		- XSM_AVG_16_SAMPLES means 16 samples of averaging
*		- XSM_AVG_64_SAMPLES means 64 samples of averaging
*		- XSM_AVG_256_SAMPLES means 256 samples of averaging
*
* @note		None.
*
*****************************************************************************/
u8 XSysMon_GetAvg(XSysMon *InstancePtr)
{
	u32 Average;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the averaging value from the Configuration Register 0.
	 */
	Average = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				  XSM_CFR0_OFFSET) & XSM_CFR0_AVG_VALID_MASK;

	return ((u8) (Average >> XSM_CFR0_AVG_SHIFT));
}

/****************************************************************************/
/**
*
* The function sets the given parameters in the Configuration Register 0 in
* the single channel mode.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Channel is the channel number for conversion. The valid
*		channels are 0 to 5, 8, and 16 to 31. Channel 6 is
*		valid for 7 series and Zynq XADC. Channel 32 to 35 are valid
*		for Ultrascale.
* @param	IncreaseAcqCycles is a boolean parameter which specifies whether
*		the Acquisition time for the external channels has to be
*		increased to 10 ADCCLK cycles (specify TRUE) or remain at the
*		default 4 ADCCLK cycles (specify FALSE). This parameter is
*		only valid for the external channels.
* @param	IsEventMode is a boolean parameter that specifies continuous
*		sampling (specify FALSE) or event driven sampling mode (specify
*		TRUE) for the given channel.
* @param	IsDifferentialMode is a boolean parameter which specifies
*		unipolar(specify FALSE) or differential mode (specify TRUE) for
*		the analog inputs. The 	input mode is only valid for the
*		external channels.
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the Configuration Register 0.
*		- XST_FAILURE if the channel sequencer is enabled or the input
*		parameters are not valid for the selected channel.
*
* @note
*		- The number of samples for the averaging for all the channels
*		is set by using the function XSysMon_SetAvg.
*		- The calibration of the device is done by doing a ADC
*		conversion on the calibration channel(channel 8). The input
*		parameters IncreaseAcqCycles, IsDifferentialMode and
*		IsEventMode are not valid for this channel.
*
*****************************************************************************/
int XSysMon_SetSingleChParams(XSysMon *InstancePtr, u8 Channel,
				int IncreaseAcqCycles, int IsEventMode,
				int IsDifferentialMode)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Channel <= XSM_CH_VREFN) ||
			  (Channel == XSM_CH_CALIBRATION) ||
			  ((Channel >= XSM_CH_AUX_MIN) &&
			  (Channel <= XSM_CH_AUX_MAX)) ||
			  ((Channel >= XSM_CH_VUSR0) &&
			  (Channel <= XSM_CH_VUSR3)));
	Xil_AssertNonvoid((IncreaseAcqCycles == TRUE) ||
			  (IncreaseAcqCycles == FALSE));
	Xil_AssertNonvoid((IsEventMode == TRUE) || (IsEventMode == FALSE));
	Xil_AssertNonvoid((IsDifferentialMode == TRUE) ||
			  (IsDifferentialMode == FALSE));

	/*
	 * Check if the device is in single channel mode else return failure
	 */
	if ((XSysMon_GetSequencerMode(InstancePtr) != XSM_SEQ_MODE_SINGCHAN)) {
		return XST_FAILURE;
	}

	/*
	 * Read the Configuration Register 0.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				   XSM_CFR0_OFFSET) & XSM_CFR0_AVG_VALID_MASK;

	/*
	 * Select the number of acquisition cycles. The acquisition cycles is
	 * only valid for the external channels.
	 */
	if (IncreaseAcqCycles == TRUE) {
		if (((Channel >= XSM_CH_AUX_MIN) && (Channel <= XSM_CH_AUX_MAX))
		    || (Channel == XSM_CH_VPVN)) {
			RegValue |= XSM_CFR0_ACQ_MASK;
		} else {
			return XST_FAILURE;
		}
	}

	/*
	 * Select the input mode. The input mode is only valid for the
	 * external channels.
	 */
	if (IsDifferentialMode == TRUE) {

		if (((Channel >= XSM_CH_AUX_MIN) && (Channel <= XSM_CH_AUX_MAX))
		    || (Channel == XSM_CH_VPVN)) {
			RegValue |= XSM_CFR0_DU_MASK;
		} else {
			return XST_FAILURE;
		}
	}

	/*
	 * Select the ADC mode.
	 */
	if (IsEventMode == TRUE) {
		RegValue |= XSM_CFR0_EC_MASK;
	}

	/*
	 * Write the given values into the Configuration Register 0.
	 */
	RegValue |= (Channel & XSM_CFR0_CHANNEL_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR0_OFFSET,
			 RegValue);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function enables the alarm outputs for the specified alarms in the
* Configuration Registers 1 and 3:
*
*		- OT for Over Temperature (XSM_CFR_OT_MASK)
*		- ALM0 for On board Temperature (XSM_CFR_ALM_TEMP_MASK)
*		- ALM1 for VCCINT (XSM_CFR_ALM_VCCINT_MASK)
*		- ALM2 for VCCAUX (XSM_CFR_ALM_VCCAUX_MASK)
* 		- ALM3 for VBRAM (XSM_CFR_ALM_VBRAM_MASK)for 7 Series and Zynq
* 		- ALM4 for VCCPINT (XSM_CFR_ALM_VCCPINT_MASK) for Zynq
*		- ALM5 for VCCPAUX (XSM_CFR_ALM_VCCPAUX_MASK) for Zynq
* 		- ALM6 for VCCPDRO (XSM_CFR_ALM_VCCPDRO_MASK) for Zynq
* 		- ALM8 for VUSER0 (XSM_CFR_ALM_VUSR0_MASK) for Ultrascale
* 		- ALM9 for VUSER1 (XSM_CFR_ALM_VUSR1_MASK) for Ultrascale
* 		- ALM10 for VUSER2 (XSM_CFR_ALM_VUSR2_MASK) for Ultrascale
* 		- ALM11 for VUSER3 (XSM_CFR_ALM_VUSR3_MASK) for Ultrascale
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	AlmEnableMask is the bit-mask of the alarm outputs to be enabled
*		in the Configuration Register 1.
*		Bit positions of 1 will be enabled. Bit positions of 0 will be
*		disabled. This mask is formed by OR'ing XSM_CFR_ALM_*_MASK,
*		XSM_CFR_ALM_*_MASK and XSM_CFR_OT_MASK masks defined in
*		xsysmon_hw.h.
*
* @return	None.
*
* @note		The implementation of the alarm enables in the Configuration
*		register 1 is such that the alarms for bit positions of 1 will
*		be disabled and alarms for bit positions of 0 will be enabled.
*		The alarm outputs specified by the AlmEnableMask are negated
*		before writing to the Configuration Register 1.
*
*****************************************************************************/
void XSysMon_SetAlarmEnables(XSysMon *InstancePtr, u32 AlmEnableMask)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(AlmEnableMask <= XSM_CFR_ALM_ALL_MASK);

	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				   XSM_CFR1_OFFSET);
	RegValue &= (u32)~XSM_CFR1_ALM_ALL_MASK;
	RegValue |= (~AlmEnableMask & XSM_CFR1_ALM_ALL_MASK);

	/*
	 * Enable/disables the alarm enables for the specified alarm bits in the
	 * Configuration Register 1.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR1_OFFSET,
			 RegValue);

#if XPAR_SYSMON_0_IP_TYPE == SYSTEM_MANAGEMENT
	/*
	 * Enable/disables the alarm enables for the specified alarm bits in the
	 * Configuration Register 3.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					   XSM_CFR3_OFFSET);
	RegValue &= (u32)~XSM_CFR3_ALM_ALL_MASK;
	RegValue |= (~(AlmEnableMask >> 16) & XSM_CFR3_ALM_ALL_MASK);

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR3_OFFSET,
			 RegValue);
#endif
}

/****************************************************************************/
/**
*
* This function gets the status of the alarm output enables in the
* Configuration Register 1.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	This is the bit-mask of the enabled alarm outputs in the
*		Configuration Register 1. Use the masks XSM_CFR_ALM_*,
*		XSM_CFR_ALM*_* and XSM_CFR_OT_MASK defined in
*		xsysmon_hw.h to interpret the returned value.
*
*		Bit positions of 1 indicate that the alarm output is enabled.
*		Bit positions of 0 indicate that the alarm output is disabled.
*
*
* @note		The implementation of the alarm enables in the Configuration
*		register 1 is such that alarms for the bit positions of 1 will
*		be disabled and alarms for bit positions of 0 will be enabled.
*		The enabled alarm outputs returned by this function is the
*		negated value of the the data read from the Configuration
*		Register 1.
*
*****************************************************************************/
u32 XSysMon_GetAlarmEnables(XSysMon *InstancePtr)
{
	u32 RegValue1;
#if XPAR_SYSMON_0_IP_TYPE == SYSTEM_MANAGEMENT
	u32 RegValue2;
#endif
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the status of alarm output enables from the Configuration
	 * Register 1.
	 */
	RegValue1 = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			XSM_CFR1_OFFSET) & XSM_CFR1_ALM_ALL_MASK;
	RegValue1 = (~RegValue1 & XSM_CFR1_ALM_ALL_MASK);

#if XPAR_SYSMON_0_IP_TYPE == SYSTEM_MANAGEMENT
	/*
	* Read the status of alarm output enables from the Configuration
	* Register 3.
	*/
	RegValue2 = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_CFR3_OFFSET) & XSM_CFR3_ALM_ALL_MASK;
	RegValue2 = (~RegValue2 & XSM_CFR3_ALM_ALL_MASK);

	return ((RegValue2 << 16) | RegValue1);
#else
	return RegValue1;
#endif
}

/****************************************************************************/
/**
*
* This function enables the specified calibration in the Configuration
* Register 1 :
*
*	- XSM_CFR1_CAL_ADC_OFFSET_MASK : Calibration 0 -ADC offset correction
*	- XSM_CFR1_CAL_ADC_GAIN_OFFSET_MASK : Calibration 1 -ADC gain and offset
*	correction
*	- XSM_CFR1_CAL_PS_OFFSET_MASK : Calibration 2 -Power Supply sensor
*	offset correction
*	- XSM_CFR1_CAL_PS_GAIN_OFFSET_MASK : Calibration 3 -Power Supply sensor
*	gain and offset correction
*	- XSM_CFR1_CAL_DISABLE_MASK : No Calibration
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Calibration is the Calibration to be applied.
*		Use XSM_CFR1_CAL*_* bits defined in xsysmon_hw.h.
*		Multiple calibrations can be enabled at a time by oring the
*		XSM_CFR1_CAL_ADC_* and XSM_CFR1_CAL_PS_* bits.
*		Calibration can be disabled by specifying
*		XSM_CFR1_CAL_DISABLE_MASK;
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMon_SetCalibEnables(XSysMon *InstancePtr, u16 Calibration)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(((Calibration >= XSM_CFR1_CAL_ADC_OFFSET_MASK) &&
			(Calibration <= XSM_CFR1_CAL_VALID_MASK)) ||
			(Calibration == XSM_CFR1_CAL_DISABLE_MASK));

	/*
	 * Set the specified calibration in the Configuration Register 1.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					XSM_CFR1_OFFSET);

	RegValue &= (~ XSM_CFR1_CAL_VALID_MASK);
	RegValue |= (Calibration & XSM_CFR1_CAL_VALID_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR1_OFFSET,
			 RegValue);
}

/****************************************************************************/
/**
*
* This function reads the value of the calibration enables from the
* Configuration Register 1.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	The value of the calibration enables in the Configuration
*		Register 1 :
*		- XSM_CFR1_CAL_ADC_OFFSET_MASK : ADC offset correction
*		- XSM_CFR1_CAL_ADC_GAIN_OFFSET_MASK : ADC gain and offset
*		correction
*		- XSM_CFR1_CAL_PS_OFFSET_MASK : Power Supply sensor offset
*		correction
*		- XSM_CFR1_CAL_PS_GAIN_OFFSET_MASK : Power Supply sensor gain
*		and offset correction
*		- XSM_CFR1_CAL_DISABLE_MASK : No Calibration
*
* @note		None.
*
*****************************************************************************/
u16 XSysMon_GetCalibEnables(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the calibration enables from the Configuration Register 1.
	 */
	return (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			XSM_CFR1_OFFSET) & XSM_CFR1_CAL_VALID_MASK;
}

/****************************************************************************/
/**
*
* This function sets the specified Channel Sequencer Mode in the Configuration
* Register 1 :
*		- Default safe mode (XSM_SEQ_MODE_SAFE)
*		- One pass through sequence (XSM_SEQ_MODE_ONEPASS)
*		- Continuous channel sequencing (XSM_SEQ_MODE_CONTINPASS)
*		- Single Channel/Sequencer off (XSM_SEQ_MODE_SINGCHAN)
*		- Simulataneous sampling mode (XSM_SEQ_MODE_SIMUL)
*		- Independent mode (XSM_SEQ_MODE_INDEPENDENT)
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	SequencerMode is the sequencer mode to be set.
*		Use XSM_SEQ_MODE_* bits defined in xsysmon.h.
*
* @return	None.
*
* @note		Only one of the modes can be enabled at a time.
*
*****************************************************************************/
void XSysMon_SetSequencerMode(XSysMon *InstancePtr, u8 SequencerMode)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((SequencerMode <= XSM_SEQ_MODE_SIMUL) ||
			(SequencerMode == XSM_SEQ_MODE_INDEPENDENT));

	/*
	 * Set the specified sequencer mode in the Configuration Register 1.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					XSM_CFR1_OFFSET);
	RegValue &= (~ XSM_CFR1_SEQ_VALID_MASK);
	RegValue |= ((SequencerMode  << XSM_CFR1_SEQ_SHIFT) &
					XSM_CFR1_SEQ_VALID_MASK);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR1_OFFSET,
			 RegValue);
}

/****************************************************************************/
/**
*
* This function gets the channel sequencer mode from the Configuration
* Register 1.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	The channel sequencer mode :
*		- XSM_SEQ_MODE_SAFE : Default safe mode
*		- XSM_SEQ_MODE_ONEPASS : One pass through sequence
*		- XSM_SEQ_MODE_CONTINPASS : Continuous channel sequencing
*		- XSM_SEQ_MODE_SINGCHAN : Single channel/Sequencer off
*		- XSM_SEQ_MODE_SIMUL : Simulataneous sampling mode
*		- XSM_SEQ_MODE_INDEPENDENT : Independent mode
*
* @note		None.
*
*****************************************************************************/
u8 XSysMon_GetSequencerMode(XSysMon *InstancePtr)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the channel sequencer mode from the Configuration Register 1.
	 */
	return ((u8) ((XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			XSM_CFR1_OFFSET) & XSM_CFR1_SEQ_VALID_MASK) >>
			XSM_CFR1_SEQ_SHIFT));
}

/****************************************************************************/
/**
*
* The function enables the Event mode or Continuous mode in the sequencer mode.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	IsEventMode is a boolean parameter that specifies continuous
*		sampling (specify FALSE) or event driven sampling mode (specify
*		TRUE) for the given channel.
*
* @return	None.
*
* @note		The Event mode is only available in 7 Series XADC and Zynq.
*		This API should be used only with 7 Series XADC and Zynq .
*
*****************************************************************************/
void XSysMon_SetSequencerEvent(XSysMon *InstancePtr, int IsEventMode)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((IsEventMode == TRUE) || (IsEventMode == FALSE));

	/*
	 * Read the Configuration Register 0.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				   XSM_CFR0_OFFSET);

	/*
	 * Set the ADC mode.
	 */
	if (IsEventMode == TRUE) {
		RegValue |= XSM_CFR0_EC_MASK;
	} else {
		RegValue &= ~XSM_CFR0_EC_MASK;
	}

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR0_OFFSET,
			 RegValue);
}

/****************************************************************************/
/**
*
* The function enables the external mux and connects a channel to the mux.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Channel is the channel number used to connect to the external
*		Mux. The valid channels are 0 to 6, 8, and 16 to 31.
*
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the Configuration Register 0.
*		- XST_FAILURE if the channel sequencer is enabled or the input
*		parameters are not valid for the selected channel.
*
* @note		The External Mux is only available in 7 Series and Zynq XADC.
*		This API should be used only with 7 Series and Zynq XADC.
*
*****************************************************************************/
void XSysMon_SetExtenalMux(XSysMon *InstancePtr, u8 Channel)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Channel <= XSM_CH_VREFN) ||
			  (Channel == XSM_CH_CALIBRATION) ||
			  ((Channel >= XSM_CH_AUX_MIN) &&
			  (Channel <= XSM_CH_AUX_MAX)));

	/*
	 * Read the Configuration Register 0 and the clear the channel selection
	 * bits.
	 */
	RegValue = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				   XSM_CFR0_OFFSET);
	RegValue &= ~(XSM_CFR0_CHANNEL_MASK);

	/*
	 * Enable the External Mux and select the channel.
	 */
	RegValue |= (XSM_CFR0_MUX_MASK | Channel);
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR0_OFFSET,
			 RegValue);
}

/****************************************************************************/
/**
*
* The function sets the frequency of the ADCCLK by configuring the DCLK to
* ADCCLK ratio in the Configuration Register #2.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Divisor is clock divisor used to derive ADCCLK from DCLK.
*		Valid values of the divisor are
*		 - 8 to 255 for V5 SysMon.
*		 - 0 to 255 for V6/7 Series and Zynq XADC.
*                Values 0, 1, 2 are all mapped to 2.
*		Refer to the device specification for more details.
*
* @return	None.
*
* @note		- The ADCCLK is an internal clock used by the ADC and is
*		synchronized to the DCLK clock. The ADCCLK is equal to DCLK
*		divided by the user selection in the Configuration Register 2.
*		- There is no Assert on the minimum value of the Divisor. Users
*		must take care such that the minimum value of Divisor used is
*		8, in case of V5 SysMon.
*
*****************************************************************************/
void XSysMon_SetAdcClkDivisor(XSysMon *InstancePtr, u8 Divisor)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Write the divisor value into the Configuration Register #2.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress, XSM_CFR2_OFFSET,
			 Divisor << XSM_CFR2_CD_SHIFT);

}

/****************************************************************************/
/**
*
* The function gets the ADCCLK divisor from the Configuration Register 2.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	The divisor read from the Configuration Register 2.
*
* @note		The ADCCLK is an internal clock used by the ADC and is
*		synchronized to the DCLK clock. The ADCCLK is equal to DCLK
*		divided by the user selection in the Configuration Register 2.
*
*****************************************************************************/
u8 XSysMon_GetAdcClkDivisor(XSysMon *InstancePtr)
{
	u16 Divisor;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the divisor value from the Configuration Register 2.
	 */
	Divisor = (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					XSM_CFR2_OFFSET);

	return (u8) (Divisor >> XSM_CFR2_CD_SHIFT);
}

/****************************************************************************/
/**
*
* This function enables the specified channels in the ADC Channel Selection
* Sequencer Registers. The sequencer must be in the Safe Mode before writing
* to these registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	ChEnableMask is the bit mask of all the channels to be enabled.
*		Use XSM_SEQ_CH_* defined in xsysmon_hw.h to specify the Channel
*		numbers. Bit masks of 1 will be enabled and bit mask of 0 will
*		be disabled.
*		The ChEnableMask is a 64 bit mask that is written to the three
*		16 bit ADC Channel Selection Sequencer Registers.
*
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the ADC Channel Selection Sequencer Registers.
*		- XST_FAILURE if the channel sequencer is enabled.
*
* @note		None.
*
*****************************************************************************/
int XSysMon_SetSeqChEnables(XSysMon *InstancePtr, u64 ChEnableMask)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The sequencer must be in the Safe Mode before writing
	 * to these registers. Return XST_FAILURE if the channel sequencer
	 * is enabled.
	 */
	if ((XSysMon_GetSequencerMode(InstancePtr) != XSM_SEQ_MODE_SAFE)) {
		return XST_FAILURE;
	}

	/*
	 * Enable the specified channels in the ADC Channel Selection Sequencer
	 * Registers.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ00_OFFSET,
			 (ChEnableMask & XSM_SEQ00_CH_VALID_MASK));

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ01_OFFSET,
			 (ChEnableMask >> XSM_SEQ_CH_AUX_SHIFT) &
			 XSM_SEQ01_CH_VALID_MASK);

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			XSM_SEQ08_OFFSET,
			(ChEnableMask >> XSM_SEQ_CH_VUSR_SHIFT) &
			 XSM_SEQ08_CH_VALID_MASK);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function gets the channel enable bits status from the ADC Channel
* Selection Sequencer Registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	Gets the channel enable bits. Use XSM_SEQ_CH_* defined in
*		xsysmon_hw.h to interpret the Channel numbers. Bit masks of 1
*		are the channels that are enabled and bit mask of 0 are
*		the channels that are disabled.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
u64 XSysMon_GetSeqChEnables(XSysMon *InstancePtr)
{
	u32 RegValEnable;
	u32 RegValEnable1;
	u64 RetVal = 0x0;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the channel enable bits for all the channels from the ADC
	 * Channel Selection Register.
	 */
	RegValEnable = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_SEQ00_OFFSET) & XSM_SEQ00_CH_VALID_MASK;
	RegValEnable |= (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ01_OFFSET) & XSM_SEQ01_CH_VALID_MASK) <<
			 XSM_SEQ_CH_AUX_SHIFT;

	RegValEnable1 = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_SEQ08_OFFSET) & XSM_SEQ08_CH_VALID_MASK;

	RetVal = RegValEnable1;
	RetVal = (RetVal << XSM_SEQ_CH_VUSR_SHIFT);
	RetVal = RetVal | RegValEnable;

	return RetVal;
}

/****************************************************************************/
/**
*
* This function enables the averaging for the specified channels in the ADC
* Channel Averaging Enable Sequencer Registers. The sequencer must be in
* the Safe Mode before writing to these registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	AvgEnableChMask is the bit mask of all the channels for which
*		averaging is to be enabled. Use XSM_SEQ_CH__* defined in
*		xsysmon_hw.h to specify the Channel numbers. Averaging will be
*		enabled for bit masks of 1 and disabled for bit mask of 0.
*		The AvgEnableChMask is a 64 bit mask that is written to the
*		three 16 bit ADC Channel Averaging Enable Sequencer Registers.
*
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the ADC Channel Averaging Enables Sequencer Registers.
*		- XST_FAILURE if the channel sequencer is enabled.
*
* @note		None.
*
*****************************************************************************/
int XSysMon_SetSeqAvgEnables(XSysMon *InstancePtr, u64 AvgEnableChMask)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The sequencer must be disabled for writing any of these registers.
	 * Return XST_FAILURE if the channel sequencer is enabled.
	 */
	if ((XSysMon_GetSequencerMode(InstancePtr) != XSM_SEQ_MODE_SAFE)) {
		return XST_FAILURE;
	}

	/*
	 * Enable/disable the averaging for the specified channels in the
	 * ADC Channel Averaging Enables Sequencer Registers.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ02_OFFSET,
			 (AvgEnableChMask & XSM_SEQ02_CH_VALID_MASK));

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ03_OFFSET,
			 (AvgEnableChMask >> XSM_SEQ_CH_AUX_SHIFT) &
			 XSM_SEQ03_CH_VALID_MASK);

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			XSM_SEQ09_OFFSET,
			(AvgEnableChMask >> XSM_SEQ_CH_VUSR_SHIFT) &
			 XSM_SEQ09_CH_VALID_MASK);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns the channels for which the averaging has been enabled
* in the ADC Channel Averaging Enables Sequencer Registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @returns 	The status of averaging (enabled/disabled) for all the channels.
*		Use XSM_SEQ_CH__* defined in xsysmon_hw.h to interpret the
*		Channel numbers. Bit masks of 1 are the channels for which
*		averaging is enabled and bit mask of 0 are the channels for
*		averaging is disabled.
*
* @note		None.
*
*****************************************************************************/
u64 XSysMon_GetSeqAvgEnables(XSysMon *InstancePtr)
{
	u32 RegValAvg;
	u32 RegValAvg1;
	u64 RetVal = 0x0;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the averaging enable status for all the channels from the
	 * ADC Channel Averaging Enables Sequencer Registers.
	 */
	RegValAvg = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_SEQ02_OFFSET) & XSM_SEQ02_CH_VALID_MASK;
	RegValAvg |= (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			XSM_SEQ03_OFFSET) & XSM_SEQ03_CH_VALID_MASK) <<
			XSM_SEQ_CH_AUX_SHIFT;

	RegValAvg1 = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_SEQ09_OFFSET) & XSM_SEQ09_CH_VALID_MASK;

	RetVal = RegValAvg1;
	RetVal = (RetVal << XSM_SEQ_CH_VUSR_SHIFT);
	RetVal = RetVal | RegValAvg;

	return RetVal;
}

/****************************************************************************/
/**
*
* This function sets the Analog input mode for the specified channels in the
* ADC Channel Analog-Input Mode Sequencer Registers. The sequencer must be in
* the Safe Mode before writing to these registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	InputModeChMask is the bit mask of all the channels for which
*		the input mode is differential mode. Use XSM_SEQ_CH__* defined
*		in xsysmon_hw.h to specify the channel numbers. Differential
*		input mode will be set for bit masks of 1 and unipolar input
*		mode for bit masks of 0.
*		The InputModeChMask is a 32 bit mask that is written to the two
*		16 bit ADC Channel Analog-Input Mode Sequencer Registers.
*
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the ADC Channel Analog-Input Mode Sequencer Registers.
*		- XST_FAILURE if the channel sequencer is enabled.
*
* @note		None.
*
*****************************************************************************/
int XSysMon_SetSeqInputMode(XSysMon *InstancePtr, u32 InputModeChMask)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The sequencer must be in the Safe Mode before writing to
	 * these registers. Return XST_FAILURE if the channel sequencer
	 * is enabled.
	 */
	if ((XSysMon_GetSequencerMode(InstancePtr) != XSM_SEQ_MODE_SAFE)) {
		return XST_FAILURE;
	}

	/*
	 * Set the input mode for the specified channels in the ADC Channel
	 * Analog-Input Mode Sequencer Registers.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ04_OFFSET,
			 (InputModeChMask & XSM_SEQ04_CH_VALID_MASK));

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ05_OFFSET,
			 (InputModeChMask >> XSM_SEQ_CH_AUX_SHIFT) &
			 XSM_SEQ05_CH_VALID_MASK);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function gets the Analog input mode for all the channels from
* the ADC Channel Analog-Input Mode Sequencer Registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @returns 	The input mode for all the channels.
*		Use XSM_SEQ_CH_* defined in xsysmon_hw.h to interpret the
*		Channel numbers. Bit masks of 1 are the channels for which
*		input mode is differential and bit mask of 0 are the channels
*		for which input mode is unipolar.
*
* @note		None.
*
*****************************************************************************/
u32 XSysMon_GetSeqInputMode(XSysMon *InstancePtr)
{
	u32 InputMode;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 *  Get the input mode for all the channels from the ADC Channel
	 * Analog-Input Mode Sequencer Registers.
	 */
	InputMode = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_SEQ04_OFFSET) & XSM_SEQ04_CH_VALID_MASK;
	InputMode |= (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_SEQ05_OFFSET) & XSM_SEQ05_CH_VALID_MASK) <<
				XSM_SEQ_CH_AUX_SHIFT;

	return InputMode;
}

/****************************************************************************/
/**
*
* This function sets the number of Acquisition cycles in the ADC Channel
* Acquisition Time Sequencer Registers. The sequencer must be in the Safe Mode
* before writing to these registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	AcqCyclesChMask is the bit mask of all the channels for which
*		the number of acquisition cycles is to be extended.
*		Use XSM_SEQ_CH__* defined in xsysmon_hw.h to specify the Channel
*		numbers. Acquisition cycles will be extended to 10 ADCCLK cycles
*		for bit masks of 1 and will be the default 4 ADCCLK cycles for
*		bit masks of 0.
*		The AcqCyclesChMask is a 32 bit mask that is written to the two
*		16 bit ADC Channel Acquisition Time Sequencer Registers.
*
* @return
*		- XST_SUCCESS if the given values were written successfully to
*		the Channel Sequencer Registers.
*		- XST_FAILURE if the channel sequencer is enabled.
*
* @note		None.
*
*****************************************************************************/
int XSysMon_SetSeqAcqTime(XSysMon *InstancePtr, u32 AcqCyclesChMask)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The sequencer must be in the Safe Mode before writing
	 * to these registers. Return XST_FAILURE if the channel
	 * sequencer is enabled.
	 */
	if ((XSysMon_GetSequencerMode(InstancePtr) != XSM_SEQ_MODE_SAFE)) {
		return XST_FAILURE;
	}

	/*
	 * Set the Acquisition time for the specified channels in the
	 * ADC Channel Acquisition Time Sequencer Registers.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ06_OFFSET,
			 (AcqCyclesChMask & XSM_SEQ06_CH_VALID_MASK));

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_SEQ07_OFFSET,
			 (AcqCyclesChMask >> XSM_SEQ_CH_AUX_SHIFT) &
			 XSM_SEQ07_CH_VALID_MASK);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function gets the status of acquisition from the ADC Channel Acquisition
* Time Sequencer Registers.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @returns 	The acquisition time for all the channels.
*		Use XSM_SEQ_CH__* defined in xsysmon_hw.h to interpret the
*		Channel numbers. Bit masks of 1 are the channels for which
*		acquisition cycles are extended and bit mask of 0 are the
*		channels for which acquisition cycles are not extended.
*
* @note		None.
*
*****************************************************************************/
u32 XSysMon_GetSeqAcqTime(XSysMon *InstancePtr)
{
	u32 RegValAcq;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Get the Acquisition cycles for the specified channels from the ADC
	 * Channel Acquisition Time Sequencer Registers.
	 */
	RegValAcq = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				    XSM_SEQ06_OFFSET) & XSM_SEQ06_CH_VALID_MASK;
	RegValAcq |= (XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
			XSM_SEQ07_OFFSET) & XSM_SEQ07_CH_VALID_MASK) <<
			XSM_SEQ_CH_AUX_SHIFT;

	return RegValAcq;
}

/****************************************************************************/
/**
*
* This functions sets the contents of the given Alarm Threshold Register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	AlarmThrReg is the index of an Alarm Threshold Register to
*		be set. Use XSM_ATR_* constants defined in xsysmon.h to
*		specify the index.
* @param	Value is the 16-bit threshold value to write into the register.
*
* @return	None.
*
* @note		Over Temperature upper threshold is programmable only in V6,
*		7 Series/Zynq  XADC and UltraScale.
*		BRAM high and low voltage threshold registers are available only
*		in 7 Series XADC and UltraScale.
*		VUSER0 to VUSER3 threshold registers are available only in
*		UltraScale.
*		All the remaining Alarm Threshold registers specified by the
*		constants XSM_ATR_*, are available in all the families of the
*		Sysmon.
*
*****************************************************************************/
void XSysMon_SetAlarmThreshold(XSysMon *InstancePtr, u8 AlarmThrReg, u16 Value)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((AlarmThrReg <= XSM_ATR_VUSR3_UPPER) ||
			((AlarmThrReg >= XSM_ATR_VUSR0_LOWER) &&
			(AlarmThrReg <= XSM_ATR_VUSR3_LOWER)));

	/*
	 * Write the value into the specified Alarm Threshold Register.
	 */
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_ATR_TEMP_UPPER_OFFSET + (AlarmThrReg << 2),
			 Value);
}

/****************************************************************************/
/**
*
* This function returns the contents of the specified Alarm Threshold Register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	AlarmThrReg is the index of an Alarm Threshold Register
*		to be read. Use XSM_ATR_* constants defined in xsysmon.h
*		to specify the index.
*
* @return	A 16-bit value representing the contents of the selected Alarm
*		Threshold Register.
*
* @note		Over Temperature upper threshold is programmable only in V6 and
*		7 Series XADC
*		BRAM high and low voltage threshold registers are available only
*		in 7 Series and Zynq XADC.
*		All the remaining Alarm Threshold registers specified by the
*		constants XSM_ATR_*, are available in all the families of the
*		Sysmon.
*
*****************************************************************************/
u16 XSysMon_GetAlarmThreshold(XSysMon *InstancePtr, u8 AlarmThrReg)
{
	u16 AlarmThreshold;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((AlarmThrReg <= XSM_ATR_VUSR3_UPPER) ||
			 ((AlarmThrReg >= XSM_ATR_VUSR0_LOWER) &&
			 (AlarmThrReg <= XSM_ATR_VUSR3_LOWER)));

	/*
	 * Read the specified Alarm Threshold Register and return
	 * the value.
	 */
	AlarmThreshold = (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				XSM_ATR_TEMP_UPPER_OFFSET + (AlarmThrReg << 2));

	return AlarmThreshold;
}

/****************************************************************************/
/**
*
* This function sets the powerdown temperature for the OverTemp signal in the
* OT Powerdown register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	Value is the 16-bit OT Upper Alarm Register powerdown value.
*		Valid values are 0 to 0x0FFF.
*
* @return	None.
*
* @note		This API has been deprecated. Use XSysMon_SetAlarmThreshold(),
*		instead.
*		This API should be used only with V6/7 Series since the
*		upper threshold of OverTemp is programmable in in only V6
*		SysMon/7 Series and Zynq XADC.
*
*****************************************************************************/
void XSysMon_SetOverTemp(XSysMon *InstancePtr, u16 Value)
{
	u16 OtUpper;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Value <= XSM_ATR_OT_UPPER_VAL_MAX);

	/*
	 * Read the OT Upper Alarm Threshold Register.
	 */
	OtUpper = (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					XSM_ATR_OT_UPPER_OFFSET);
	OtUpper &= ~(XSM_ATR_OT_UPPER_VAL_MASK);

	/*
	 * Preserve the OT enable value and write the powerdown value into the
	 * OT Upper Alarm Threshold Register.
	 */
	Value = (Value << XSM_ATR_OT_UPPER_VAL_SHIFT) | OtUpper;
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_ATR_OT_UPPER_OFFSET, Value);
}

/****************************************************************************/
/**
*
* This function returns the powerdown temperature of the OverTemp signal in
* the OT Powerdown register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	A 12-bit OT Upper Alarm Register powerdown value.
*
* @note		This API has been deprecated. Use XSysMon_GetAlarmThreshold(),
*		instead.
*		This API should be used only with V6/7 Series since the
*		upper threshold of OverTemp is programmable in  only V6
*		SysMon/7 Series and Zynq XADC.
*
*****************************************************************************/
u16 XSysMon_GetOverTemp(XSysMon *InstancePtr)
{
	u16 OtUpper;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the OT upper Alarm Threshold Register and return
	 * the value.
	 */
	OtUpper = (u16) XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
					XSM_ATR_OT_UPPER_OFFSET);
	OtUpper >>= XSM_ATR_OT_UPPER_VAL_SHIFT;

	return OtUpper;
}

/****************************************************************************/
/**
*
* This function enables programming of the powerdown temperature for the
* OverTemp signal in the OT Powerdown register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		This API should be used only with V6/7 Series since the
*		upper threshold of OverTemp is programmable in only V6
*		SysMon/7 Series and Zynq XADC.
*
*****************************************************************************/
void XSysMon_EnableUserOverTemp(XSysMon *InstancePtr)
{
	u16 OtUpper;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the OT upper Alarm Threshold Register.
	 */
	OtUpper = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				  XSM_ATR_OT_UPPER_OFFSET);
	OtUpper &= ~(XSM_ATR_OT_UPPER_ENB_MASK);

	/*
	 * Preserve the powerdown value and write OT enable value the into the
	 * OT Upper Alarm Threshold Register.
	 */
	OtUpper |= XSM_ATR_OT_UPPER_ENB_VAL;
	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_ATR_OT_UPPER_OFFSET, OtUpper);
}

/****************************************************************************/
/**
*
* This function disables programming of the powerdown temperature for the
* OverTemp signal in the OT Powerdown register.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		This API should be used only with V6 SysMon/7 Series and Zynq
* 		XADC since the upper threshold of OverTemp is programmable
*		in only V6 SysMon/7 Series and Zynq XADC.
*
*****************************************************************************/
void XSysMon_DisableUserOverTemp(XSysMon *InstancePtr)
{
	u16 OtUpper;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the OT Upper Alarm Threshold Register.
	 */
	OtUpper = XSysMon_ReadReg(InstancePtr->Config.BaseAddress,
				  XSM_ATR_OT_UPPER_OFFSET);
	OtUpper &= ~(XSM_ATR_OT_UPPER_ENB_MASK);

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			 XSM_ATR_OT_UPPER_OFFSET, OtUpper);
}

/****************************************************************************/
/**
*
* This function enables the Temperature updation logic so that temperature
* can be sent over TEMP_OUT port.
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		None
*
*****************************************************************************/
void XSysMon_EnableTempUpdate(XSysMon *InstancePtr)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	RegVal = (InstancePtr->Mask) | XSM_CONVST_TEMPUPDT_MASK ;

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
			XSM_CONVST_OFFSET, RegVal);

	/* Store the written value in Mask */
	InstancePtr->Mask = RegVal;
}

/****************************************************************************/
/**
*
* This function disables the Temperature updation logic for TEMP_OUT port
*
* @param	InstancePtr is a pointer to the XSysMon instance.
*
* @return	None.
*
* @note		None
*
*****************************************************************************/
void XSysMon_DisableTempUpdate(XSysMon *InstancePtr)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = (InstancePtr->Mask) & ~(XSM_CONVST_TEMPUPDT_MASK) ;

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
		XSM_CONVST_OFFSET, RegVal);

	/* Store the written value in Mask */
	InstancePtr->Mask = RegVal;
}

/****************************************************************************/
/**
*
* This function sets the number of Wait Cycles for Temperature updation logic
*
* @param	InstancePtr is a pointer to the XSysMon instance.
* @param	WaitCycles is number of wait cycles
*
* @return	None.
*
* @note		The default number of wait cycles are 1000(0x3E8).
*
*****************************************************************************/
void XSysMon_SetTempWaitCycles(XSysMon *InstancePtr, u16 WaitCycles)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	RegVal = ((InstancePtr->Mask) & ~(XSM_CONVST_WAITCYCLES_MASK)) ;

	RegVal = (WaitCycles << XSM_CONVST_WAITCYCLES_SHIFT) | RegVal;

	XSysMon_WriteReg(InstancePtr->Config.BaseAddress,
				XSM_CONVST_OFFSET, RegVal);

	/* Store the written value in Mask */
	InstancePtr->Mask = RegVal;
}
/** @} */
