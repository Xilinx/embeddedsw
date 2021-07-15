/******************************************************************************
* Copyright (c) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilskey_utils.c
 *
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 2.00  hk      22/01/14 Corrected PL voltage checks to VCCINT and VCCAUX.
*                        CR#768077
* 2.1   kvn     04/01/15 Fixed warnings. CR#716453.
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
* 4.0   vns     10/01/15 Modified conditional compilation
*                        to support ZynqMp platform also.
*                        Added new API Xsk_Ceil
*                        Modified Xilskey_CrcCalculation() API for providing
*                        support for efuse ZynqMp also.
* 6.0   vns     07/07/16 Modified XilSKey_Timer_Intialise API to initialize
*                        TimerTicks to 10us. As Hardware module only takes
*                        care of programming time(5us), through software we
*                        only need to control hardware module.
*                        Modified sysmon read to 16 bit resolution as
*                        sysmon driver has modified conversion formulae
*                        to 16 bit resolution.
*       vns     07/18/16 Initialized sysmonpsu driver and added
*                        XilSKey_ZynqMP_EfusePs_ReadSysmonVol and
*                        XilSKey_ZynqMP_EfusePs_ReadSysmonTemp functions
* 6.6   vns     06/06/18 Added doxygen tags
* 6.7   arc     01/05/19 Fixed MISRA-C violations.
*       vns     02/09/19 Fixed buffer overflow access in
*                        XilSKey_Efuse_ConvertStringToHexLE()
*       arc     25/02/19 Added asserts for pointer parameter for NULL
*                        verification
*                        Fixed Length parameter as length in bits for
*                        XilSKey_Efuse_ConvertStringToHexBE and added length
*                        validations
*       arc     03/13/19 Added assert to validate lengths in
*                        XilSKey_Efuse_ValidateKey()
*       arc     03/15/19 Modified initial default status value as XST_FAILURE
* 6.7   psl     03/21/19 Fixed MISRA-C violation.
*       vns     03/23/19 Fixed CRC calculation for Ultra plus
*       arc     04/04/19 Fixed CPP warnings.
* 6.8   psl     06/07/19 Added doxygen tags.
*       psl     06/25/19 Fixed Coverity warnings.
*       psl     06/28/19 Added doxygen tags.
*       psl     07/29/19 Fixed MISRA-C violation.
*       vns     08/29/19 Initialized Status variables
*       mmd     07/31/19 Avoided reconfiguration of sysmon, if it is in use
* 6.9   kpt     02/16/20 Fixed coverity warnings
*               02/27/20 Replaced XSYSMON_DEVICE_ID with XSYSMON_PSU_DEVICE_ID
*       vns     03/18/20 Fixed Armcc compilation errors
* 7.0	am      10/04/20 Resolved MISRA C violations
* 7.1   am      11/26/20 Resolved Coverity warnings
*       kpt     03/17/21 Corrected error code when VCCINT levels are out of
*                        range in XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks
* 7.2   am      07/13/21 Fixed doxygen warnings
*
* </pre>
*
 *****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xilskey_utils.h"
/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
#ifdef XSK_ZYNQ_PLATFORM
static XAdcPs XAdcInst;     /**< XADC driver instance */
u16 XAdcDevId;	/**< XADC Device ID */
#endif
#ifdef XSK_MICROBLAZE_PLATFORM
XTmrCtr XTmrCtrInst;
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
static XSysMonPsu XSysmonInst; /* Sysmon PSU instance */
static u16 XSysmonDevId; /* Sysmon PSU device ID */
#endif

static u32 TimerTicksfor100ns; /**< Global static Variable to store ticks/100ns*/
u32 TimerTicksfor1000ns; /**< Global Variable for 10 micro secs for microblaze */
/************************** Function Prototypes *****************************/
static u32 XilSKey_EfusePs_ConvertCharToNibble (char InChar, u8 *Num);
#ifdef XSK_MICROBLAZE_PLATFORM
extern void Jtag_Read_Sysmon(u8 Row, u32 *Row_Data);
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
static INLINE void XilSKey_ZynqMP_EfusePs_ReadSysmonVol(
					XSKEfusePs_XAdc *XAdcInstancePtr);
static INLINE void XilSKey_ZynqMP_EfusePs_ReadSysmonTemp(
					XSKEfusePs_XAdc *XAdcInstancePtr);
#ifndef XSK_OVERRIDE_SYSMON_CFG
static u32 XilSKey_Is_Valid_SysMon_Cfg(XSysMonPsu *InstancePtr);
#endif
#endif
/***************************************************************************/
/**
* This function is used to initialize the XADC driver
*
* @return
* 		- XST_SUCCESS in case of no errors.
* 		- XSK_EFUSEPS_ERROR_XADC_CONFIG Error occurred with XADC config.
* 		- XSK_EFUSEPS_ERROR_XADC_INITIALIZE Error occurred while XADC initialization
* 		- XSK_EFUSEPS_ERROR_XADC_SELF_TEST Error occurred in XADC self test.
*
* TDD Cases:
*
****************************************************************************/

u32 XilSKey_EfusePs_XAdcInit (void)
{
	u32 Status = (u32)XST_FAILURE;

#if defined(XSK_ZYNQ_PLATFORM)
	XAdcPs_Config *ConfigPtr;
	XAdcPs *XAdcInstPtr = &XAdcInst;

	/**
	 * specify the Device ID that is
	 * generated in xparameters.h
	 */
	XAdcDevId = XADC_DEVICE_ID;

	/**
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XAdcPs_LookupConfig(XAdcDevId);
	if (NULL == ConfigPtr) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_CONFIG;
		goto END;
	}

	Status = (u32)XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_INITIALIZE;
		goto END;
	}
	/**
	 * Self Test the XADC/ADC device
	 */
	Status = (u32)XAdcPs_SelfTest(XAdcInstPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_SELF_TEST;
		goto END;
	}

	/**
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_SAFE);

	Status = (u32)XST_SUCCESS;

#elif defined(XSK_ZYNQ_ULTRA_MP_PLATFORM)
	XSysMonPsu_Config *ConfigPtr;
	XSysMonPsu *XSysmonInstPtr = &XSysmonInst;

	/**
	 * specify the Device ID that is
	 * generated in xparameters.h
	 */
	XSysmonDevId = (u16)XSYSMON_PSU_DEVICE_ID;

	/**
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XSysMonPsu_LookupConfig(XSysmonDevId);
	if (NULL == ConfigPtr) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_CONFIG;
		goto END;
	}

	Status = (u32)XSysMonPsu_CfgInitialize(XSysmonInstPtr, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_INITIALIZE;
		goto END;
	}
	/**
	 * Self Test for sysmon device
	 */
	Status = (u32)XSysMonPsu_SelfTest(XSysmonInstPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_SELF_TEST;
		goto END;
	}

	/**
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMonPsu_SetSequencerMode(XSysmonInstPtr,
				XSM_SEQ_MODE_SAFE, XSYSMON_PS);

	Status = (u32)XST_SUCCESS;

#else
	goto END;
#endif
END:
	return Status;

}

/***************************************************************************/
/**
* This function reads current value of the temperature from sysmon.
*
* @param	XAdcInstancePtr Pointer to the XSKEfusePs_XAdc.
*
* @return	None
*
* @note		Read temperature will be stored in XSKEfusePS_XAdc pointer's
*		temperature
*
****************************************************************************/
#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
static INLINE void XilSKey_ZynqMP_EfusePs_ReadSysmonTemp(
					XSKEfusePs_XAdc *XAdcInstancePtr)
{
	XSysMonPsu *XSysmonInstPtr = &XSysmonInst;
	if (NULL == XAdcInstancePtr) {
		goto END;
	}

	/**
	 * Read the on-chip Temperature Data (Current)
	 * from the Sysmon PSU data registers.
	 */
	XAdcInstancePtr->Temp = (u32)XSysMonPsu_GetAdcData(XSysmonInstPtr,
						XSM_CH_TEMP, XSYSMON_PS);
	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
		"Read Temperature Value: %0x -> %d in Centigrades \n",
		XAdcInstancePtr->Temp,
	(int )XSysMonPsu_RawToTemperature_OnChip((float)XAdcInstancePtr->Temp));
END:
	return;

}

/***************************************************************************/
/**
* This function reads current value of the specified voltage from sysmon.
*
* @param	XAdcInstancePtr Pointer to the XSKEfusePs_XAdc.
*		Voltage typ should be specified in XAdcInstancePtr's
*		structure member VType.
*		XSK_EFUSEPS_VPAUX - Reads PS VCC Auxiliary voltage
*		XSK_EFUSEPS_VPINT - Reads VCC INT LP voltage
*
* @return	None
*
* @note		Read voltage will be stored in XSKEfusePS_XAdc pointer's
*		voltage
*
****************************************************************************/
static INLINE void XilSKey_ZynqMP_EfusePs_ReadSysmonVol(
				XSKEfusePs_XAdc *XAdcInstancePtr)
{
	XSysMonPsu *XSysmonInstPtr = &XSysmonInst;
	u8 V;
	if (NULL == XAdcInstancePtr) {
		goto END;
	}

	/**
	 * Read the VccPint/PAUX Voltage Data (Current/Maximum/Minimum) from
	 * Sysmon data registers.
	 */
	switch (XAdcInstancePtr->VType)
	{
	case XSK_EFUSEPS_VPAUX:
		V = XSM_CH_SUPPLY3;
		break;

	case XSK_EFUSEPS_VPINT:
	default:
		V = XSM_CH_SUPPLY1;
		break;
	}

	XAdcInstancePtr->V = (u32)XSysMonPsu_GetAdcData(XSysmonInstPtr, V,
						XSYSMON_PS);
	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
			"Read Voltage Value: %0x -> %d in Volts \n",
			XAdcInstancePtr->V,
			(int )XSysMonPsu_RawToVoltage((float)XAdcInstancePtr->V));
END:
	return;
}
#endif
/***************************************************************************/
/**
* This function is used to copy the min, max and current value of the
* temperature and voltage which are read from XADC.
*
* @param XAdcInstancePtr Pointer to the XSKEfusePs_XAdc. User has to
* 		 fill the VType to specify the type of voltage. Valid values
* 		 for VType are
* 		 	- XSK_EFUSEPS_VPINT
* 		 	- XSK_EFUSEPS_VPDRO
* 		 	- XSK_EFUSEPS_VPAUX
*                       - XSK_EFUSEPS_VINT
*                       - XSK_EFUSEPS_VAUX
*
* @return none
*
* TDD Cases:
*
****************************************************************************/
void XilSKey_EfusePs_XAdcReadTemperatureAndVoltage(
		XSKEfusePs_XAdc *XAdcInstancePtr)
{

#ifdef XSK_ZYNQ_PLATFORM
	XAdcPs *XAdcInstPtr = &XAdcInst;
	u8 V, VMin, VMax;
#endif

	if (NULL == XAdcInstancePtr) {
		goto END;
	}

#ifdef XSK_MICROBLAZE_PLATFORM
	/* Temperature */
	Jtag_Read_Sysmon(XSK_SYSMON_TEMP_ROW, &(XAdcInstancePtr->Temp));

	/* Voltage */
	Jtag_Read_Sysmon(XSK_SYSMON_VOL_ROW, &(XAdcInstancePtr->V));
#endif
#ifdef XSK_ZYNQ_PLATFORM

	/**
	 * Read the on-chip Temperature Data (Current/Maximum/Minimum)
	 * from the ADC data registers.
	 */
	XAdcInstancePtr->Temp = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);

	XAdcInstancePtr->TempMin =
			XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MIN_TEMP);

	XAdcInstancePtr->TempMax =
			XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, XADCPS_MAX_TEMP);

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
				"Read Temperature Value: %0x -> %d in Centigrades \n",
				XAdcInstancePtr->Temp,
				(int )XAdcPs_RawToTemperature(XAdcInstancePtr->Temp));

	/**
	 * Read the VccPint Voltage Data (Current/Maximum/Minimum) from the
	 * ADC data registers.
	 */

	switch (XAdcInstancePtr->VType)
	{
	case XSK_EFUSEPS_VINT:
		V = XADCPS_CH_VCCINT;
		VMax = XADCPS_MAX_VCCINT;
		VMin = XADCPS_MIN_VCCINT;
		break;

	case XSK_EFUSEPS_VAUX:
		V = XADCPS_CH_VCCAUX;
		VMax = XADCPS_MAX_VCCAUX;
		VMin = XADCPS_MIN_VCCAUX;
		break;

	case XSK_EFUSEPS_VPAUX:
		V = XADCPS_CH_VCCPAUX;
		VMax = XADCPS_MAX_VCCPAUX;
		VMin = XADCPS_MIN_VCCPAUX;
		break;

	case XSK_EFUSEPS_VPDRO:
		V = XADCPS_CH_VCCPDRO;
		VMax = XADCPS_MAX_VCCPDRO;
		VMin = XADCPS_MIN_VCCPDRO;
		break;

	case XSK_EFUSEPS_VPINT:
	default:
		V = XADCPS_CH_VCCPINT;
		VMax = XADCPS_MAX_VCCPINT;
		VMin = XADCPS_MIN_VCCPINT;
		break;
	}

	XAdcInstancePtr->V = XAdcPs_GetAdcData(XAdcInstPtr, V);
	XAdcInstancePtr->VMin = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, VMin);
	XAdcInstancePtr->VMax = XAdcPs_GetMinMaxMeasurement(XAdcInstPtr, VMax);

	xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL,
				"Read Voltage Value: %0x -> %d in Volts \n",
				XAdcInstancePtr->V,
				(int )XAdcPs_RawToVoltage(XAdcInstancePtr->V));

#endif

END:
	return;
}

/***************************************************************************/
/**
* This function checks temperature and voltage ranges of ZynqMP to access
* PS eFUSE
*
* @return
*		Error code: On failure
*		XST_SUCCESS on Success
*
* @note		This function returns XST_SUCCESS if we try to access eFUSE
*		on the Remus, as Sysmon access is not permitted on Remus.
*
****************************************************************************/
u32 XilSKey_ZynqMp_EfusePs_Temp_Vol_Checks(void)
{
	u32 Status = (u32)XST_FAILURE;
	/**
	 * Check the temperature and voltage(VCC_AUX and VCC_PINT_LP)
	 */
#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
	XSKEfusePs_XAdc XAdcInstance = {0U};
	XilSKey_ZynqMP_EfusePs_ReadSysmonTemp(&XAdcInstance);
	if ((XAdcInstance.Temp < (u32)XSK_EFUSEPS_TEMP_MIN_RAW) ||
			((XAdcInstance.Temp > (u32)XSK_EFUSEPS_TEMP_MAX_RAW))) {
		Status = (u32)XSK_EFUSEPS_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
		goto END;
	}
	XAdcInstance.VType = XSK_EFUSEPS_VPAUX;
	XilSKey_ZynqMP_EfusePs_ReadSysmonVol(&XAdcInstance);
	if ((XAdcInstance.V < (u32)XSK_EFUSEPS_VPAUX_MIN_RAW) ||
			((XAdcInstance.V > (u32)XSK_EFUSEPS_VPAUX_MAX_RAW))) {
		Status = (u32)XSK_EFUSEPS_ERROR_READ_VCCPAUX_VOLTAGE_OUT_OF_RANGE;
		goto END;
	}
	XAdcInstance.VType = XSK_EFUSEPS_VPINT;
	XilSKey_ZynqMP_EfusePs_ReadSysmonVol(&XAdcInstance);
	if ((XAdcInstance.V < (u32)XSK_EFUSEPS_VCC_PSINTLP_MIN_RAW) ||
			((XAdcInstance.V > (u32)XSK_EFUSEPS_VCC_PSINTLP_MAX_RAW))) {
		Status = (u32)XSK_EFUSEPS_ERROR_READ_VCCPINT_VOLTAGE_OUT_OF_RANGE;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
#endif
	return Status;

}

/****************************************************************************/
/**
*
* Initializes the global timer & starts it.
* Calculates the timer ticks for 1us.
*
* @return
*
*	None
*
* @note		None.
*
*****************************************************************************/
void XilSKey_Efuse_StartTimer(void)
{
#ifdef XSK_ARM_PLATFORM
		/**
         * Disable the Timer counter
         */
        Xil_Out32(XSK_GLOBAL_TIMER_CTRL_REG,0);
        /**
         * Write the lower 32 bit timer counter register.
         */
        Xil_Out32(XSK_GLOBAL_TIMER_COUNT_REG_LOW, 0x0);
        /**
         * Write the upper 32 bit timer counter register.
         */
        Xil_Out32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH, 0x0);
        /**
         * Enable the Timer counter
         */
        Xil_Out32(XSK_GLOBAL_TIMER_CTRL_REG,0x1);
#else
	XTmrCtr_SetOptions(&XTmrCtrInst, XSK_TMRCTR_NUM,
					XTC_AUTO_RELOAD_OPTION);
		XTmrCtr_Start(&XTmrCtrInst, XSK_TMRCTR_NUM);

#endif
}

/****************************************************************************/
/**
*
* Returns the timer ticks from start of timer to till now.
*
* @return
*
*	t - Timer ticks lapsed till now.
*
* @note		None.
*
*****************************************************************************/

u64 XilSKey_Efuse_GetTime(void)
{
    volatile u64 t;
#ifdef XSK_ARM_PLATFORM
	volatile u32 t_hi, t_lo;
	u32 TiHi;
	u32 TiLo;

    do {
		t_hi = Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH);
		t_lo = Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_LOW);
		TiHi = t_hi;
		TiLo = t_lo;
	}while(TiHi != Xil_In32(XSK_GLOBAL_TIMER_COUNT_REG_HIGH));

	t = (((u64) TiHi) << 32U) | (u64) TiLo;

#else
		 t = XTmrCtr_GetValue(&XTmrCtrInst, XSK_TMRCTR_NUM);
#endif
        return t;
}
/****************************************************************************/
/**
*
* Calculates the timer ticks to wait to set the time out
*
* @param	t  - timer ticks to wait to set the timeout
* @param	us - Timeout period in us
*
*
* @return
*	None.
*
* @note		None.
*
*****************************************************************************/
void XilSKey_Efuse_SetTimeOut(volatile u64* t, u64 us)
{
        volatile u64 t_end;
        t_end = XilSKey_Efuse_GetTime();
        /**
         * us: time to wait in microseconds. Convert to clock ticks and
         * add to current time.
         */
		t_end += (us * TimerTicksfor100ns);
        *t = t_end;
}

/****************************************************************************/
/**
*
* Checks whether the timer has been expired or not
*
* @param	t - timeout value in us
*
* @return
*
*	t_end - Returns the global timer value in case of timer expired
*
* @note		None.
*
*****************************************************************************/

u8 XilSKey_Efuse_IsTimerExpired(u64 t)
{
        u64 t_end;
        t_end = XilSKey_Efuse_GetTime();
        return t_end >= t;
}

/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param InChar is input character. It has to be between 0-9,a-f,A-F
 * @param Num is the output nibble.
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- XST_FAILURE an error when input parameters are not valid
 ****************************************************************************/
static u32 XilSKey_EfusePs_ConvertCharToNibble (char InChar, u8 *Num)
{
	u32 Status = (u32)XST_FAILURE;
	/**
	 * Convert the char to nibble
	 */
	if ((InChar >= '0') && (InChar <= '9')) {
		*Num = (u8)InChar - (u8)'0';
	}
	else if ((InChar >= 'a') && (InChar <= 'f')) {
		*Num = (u8)InChar - (u8)'a' + 10U;
	}
	else if ((InChar >= 'A') && (InChar <= 'F')) {
		*Num = (u8)InChar - (u8)'A' + 10U;
	}
	else {
		Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string in bits and it should have even values
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- XST_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
	---Initialization---
	Len is odd
	Len is zero
	Str is NULL
	Buf is NULL
	---Functionality---
	Str input with only numbers
	Str input with All values in A-F
	Str input with All values in a-f
	Str input with values in a-f, 0-9, A-F
	Str input with values in a-z, 0-9, A-Z
	Boundary Cases
	Memory Bounds of buffer checking
  ****************************************************************************/
u32 XilSKey_Efuse_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ConvertedLen;
	u8 LowerNibble = 0U, UpperNibble = 0U;

	/**
	 * Check the parameters
	 */
	if (Str == NULL) {
		Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
		goto END;
	}

	if (Buf == NULL) {
		Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
		goto END;
	}

	/**
	 * Len has to be multiple of 2
	 */
	if ((Len == 0U) || ((Len % 2U) == 1U)) {
		Status = (u32)XSK_EFUSEPS_ERROR_INVALID_PARAM;
		goto END;
	}

	if(Len != (Xil_Strnlen(Str, Len) * 4U)) {
		Status = (u32)XSK_EFUSEPS_ERROR_INVALID_PARAM;
		goto END;
	}

	ConvertedLen = 0U;
	while (ConvertedLen < (Len / 4U)) {
		/**
		 * Convert char to nibble
		 */
		if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen],&UpperNibble)
				== (u32)XST_SUCCESS) {
			/**
			 * Convert char to nibble
			 */
			if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen + 1U],
					&LowerNibble) == (u32)XST_SUCCESS) {
				/**
				 * Merge upper and lower nibble to Hex
				 */
				Buf[ConvertedLen / 2U] =
						(UpperNibble << 4U) | LowerNibble;
			}
			else {
				/**
				 * Error converting Lower nibble
				 */
				Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
				goto END;
			}
		}
		else {
			/**
			 * Error converting Upper nibble
			 */
			Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
			goto END;
		}
		/**
		 * Converted upper and lower nibbles
		 */
		xeFUSE_printf(XSK_EFUSE_DEBUG_GENERAL, "Converted %c%c to %0x\n",
				Str[ConvertedLen], Str[ConvertedLen + 1U], Buf[ConvertedLen / 2U]);
		ConvertedLen += 2U;
	}
	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0x23, 0xc1, 0xab}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string in bits and it should have even values
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- XST_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
	---Initialization---
	Len is odd
	Len is zero
	Str is NULL
	Buf is NULL
	---Functionality---
	Str input with only numbers
	Str input with All values in A-F
	Str input with All values in a-f
	Str input with values in a-f, 0-9, A-F
	Str input with values in a-z, 0-9, A-Z
	Boundary Cases
	Memory Bounds of buffer checking
  ****************************************************************************/
u32 XilSKey_Efuse_ConvertStringToHexLE(const char * Str, u8 * Buf, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ConvertedLen;
	u8 LowerNibble = 0U, UpperNibble = 0U;
	u32 StrIndex;

	/**
	 * Check the parameters
	 */
	if (Str == NULL) {
		Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
		goto END;
	}

	if (Buf == NULL) {
		Status = (u32)XSK_EFUSEPS_ERROR_PARAMETER_NULL;
		goto END;
	}

	/**
	 * Len has to be multiple of 2
	 */
	if ((Len == 0U) || ((Len % 2U) == 1U)) {
		Status = (u32)XSK_EFUSEPS_ERROR_INVALID_PARAM;
		goto END;
	}

	if(Len != (Xil_Strnlen(Str, Len) * 4U)) {
		Status = (u32)XSK_EFUSEPS_ERROR_INVALID_PARAM;
		goto END;
	}

	StrIndex = (Len / 8U) - 1U;
	ConvertedLen = 0U;
	while (ConvertedLen < (Len / 4U)) {
		/**
		 * Convert char to nibble
		 */
		if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen],
				&UpperNibble) == (u32)XST_SUCCESS) {
			/**
			 * Convert char to nibble
			 */
			if (XilSKey_EfusePs_ConvertCharToNibble (Str[ConvertedLen + 1U],
					&LowerNibble) == (u32)XST_SUCCESS)	{
				/**
				 * Merge upper and lower nibble to Hex
				 */
				Buf[StrIndex] = (UpperNibble << 4U) | LowerNibble;
				StrIndex = StrIndex - 1U;
			}
			else {
				/**
				 * Error converting Lower nibble
				 */
				Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
				goto END;
			}
		}
		else {
			/**
			 * Error converting Upper nibble
			 */
			Status = (u32)XSK_EFUSEPS_ERROR_STRING_INVALID;
			goto END;
		}
		/**
		 * Converted upper and lower nibbles
		 */
		ConvertedLen += 2U;
	}
	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/***************************************************************************/
/**
* This function is used to convert the Big Endian Byte data to
* Little Endian Byte data
* For ex: 1234567890abcdef -> 78563412efcdab90
*
* @param Be Big endian data
* @param Le Little endian data
* @param Len Length of data to be converted and it should be in number of
* 	 words (multiple of 4 bytes)
* @return	None.
*
* TDD Test Cases:
*
****************************************************************************/
void XilSKey_EfusePs_ConvertBytesBeToLe(const u8 *Be, u8 *Le, u32 Len)
{
	u32 Index;
	u32 Length;

	if ((Be == NULL) || (Le == NULL) || (Len == 0U)) {
		goto END;
	}
	Length = Len * 4U;
	for (Index = 0U; Index < Length; Index = Index + 4U) {
		Le[Index + 3U] = Be[Index];
		Le[Index + 2U] = Be[Index + 1U];
		Le[Index + 1U] = Be[Index + 2U];
		Le[Index] = Be[Index + 3U];
	}

END:
	return;
}

/****************************************************************************/
/**
 * Convert the Bits to Bytes in Little Endian format
 *       Ex: 0x5C -> {0, 0, 1, 1, 1, 0, 1, 0}
 *
 * @param       Bits Input Buffer.
 * @param       Bytes is Output buffer.
 * @param       Len of the input buffer in bits
 * @return		None
  Test Cases:
	Input with All Zeroes
	Input with All Ones
	Input Little Endian (General Cases )
	Input Check Big Endian - False case
	Check for Len not a multiple of 8
	Check for Len 0
	Memory Bounds of buffer checking
 ****************************************************************************/
void XilSKey_Efuse_ConvertBitsToBytes(const u8 * Bits, u8 * Bytes, u32 Len)
{
	u8 Data;
	u32 Index, BitIndex = 0U, ByteIndex = 0U;
	u32 BytLen = Len;

	/* Assert validates the input arguments */
	Xil_AssertVoid(Bits != NULL);
	Xil_AssertVoid(Bytes != NULL);

	/**
	 * Make sure the bytes array is 0'ed first.
	 */
	for(Index = 0U; Index < BytLen; Index++) {
		Bytes[Index] = 0U;
	}

	while(BytLen != 0U) {
		/**
		 * Convert 8 Bit One Byte to 1 Bit 8 Bytes
		 */
		for(Index = 0U; Index < 8U; Index++) {
			/**
			 * Convert from LSB -> MSB - Little Endian
			 */
			Data = (Bits[BitIndex] >> Index) & 0x1U;
			Bytes[ByteIndex] = Data;
			ByteIndex++;
			BytLen--;
			/**
			 * If len is not Byte aligned
			 */
			if(BytLen == 0U) {
				goto END;
			}
		}
		BitIndex++;
	}
END:
	return;
}

/****************************************************************************/
/**
 * Convert the Bytes to Bits in little endian format
 * 0th byte is LSB, 7th byte is MSB
 *       Ex: {0, 0, 1, 1, 1, 0, 1, 0} -> 0x5C
 *
 * @param        Bytes Input Buffer.
 * @param        Bits is Output buffer.
 * @param        Len of the input buffer.
 * @return       None
  Test Cases:
	Input with All Zeroes
	Input with All Ones
	Input Little Endian (General Cases )
	Input Check Big Endian - False case
	Check for Len not a multiple of 8
	Check for Len 0
	Memory Bounds of buffer checking
 ****************************************************************************/
void XilSKey_EfusePs_ConvertBytesToBits(const u8 * Bytes, u8 * Bits , u32 Len)
{
	u8 Tmp;
	u32 Index, BitIndex = 0U, ByteIndex = 0U;
	u32 BytLen = Len;

	/* Assert validates the input arguments */
	Xil_AssertVoid(Bytes != NULL);
	Xil_AssertVoid(Bits != NULL);

	/**
	 * Make sure the bits array is 0 first.
	 */
	for(Index = 0U; Index < (((BytLen % 8U) != 0U) ? ((BytLen / 8U) + 1U) : (BytLen / 8U)); Index++) {
		Bits[Index] = 0U;
	}

	while(BytLen != 0U) {
		/**
		 * Convert 1 Bit 8 Bytes to 8 Bit 1 Byte
		 */
		for(Index = 0U; Index < 8U; Index++) {
			/**
			 * Store from LSB -> MSB - Little Endian
			 */
			Tmp = (Bytes[ByteIndex]) & 0x1U;
			Bits[BitIndex] |= (Tmp << Index);
			ByteIndex++;
			BytLen--;
			/**
			 * If Len is not Byte aligned
			 */
			if(BytLen == 0U) {
				goto END;
			}
		}
		BitIndex++;
	}
END:
	return;
}

/****************************************************************************/
/**
 * Validate the key for proper characters & proper length
 *
 *
 * @param        Key - Hash Key
 * @param        Len - Valid length of key
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/
u32 XilSKey_Efuse_ValidateKey(const char *Key, u32 Len)
{
	u32 Status = (u32)XST_FAILURE;
	u32 KeyLen = Xil_Strnlen(Key, Len);
	u32 Index;

	Xil_AssertNonvoid(Key != NULL);
	Xil_AssertNonvoid((Len == XSK_STRING_SIZE_2) ||
			  (Len == XSK_STRING_SIZE_6) ||
			  (Len == XSK_STRING_SIZE_8) ||
			  (Len == XSK_STRING_SIZE_64) ||
			  (Len == XSK_STRING_SIZE_96));

	/**
	 * Make sure the key has valid length
	 */
	if (KeyLen != Len) {
		Status = ((u32)XSK_EFUSEPL_ERROR_KEY_VALIDATION |
			(u32)XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH);
		goto END;
	}

	/**
	* Make sure the key has valid characters
	*/
	for(Index = 0U; Index < KeyLen; Index++) {
		if(XilSKey_Efuse_IsValidChar(&Key[Index]) != (u32)XST_SUCCESS) {
			Status = ((u32)XSK_EFUSEPL_ERROR_KEY_VALIDATION |
				(u32)XSK_EFUSEPL_ERROR_NOT_VALID_KEY_CHAR);
			goto END;
		}
	}
	
	Status = (u32)XSK_EFUSEPL_ERROR_NONE;

END:
	return Status;
}

/****************************************************************************/
/**
 * Checks whether the passed character is a valid hash key character
 *
 *
 * @param        c - Character to check proper value
 *
 * @return
 *			XST_SUCCESS	- In case of Success
 *			XST_FAILURE - In case of Failure
 ****************************************************************************/
u32 XilSKey_Efuse_IsValidChar(const char *c)
{
	const char ValidChars[] = "0123456789abcdefABCDEF";
	const char *RetVal;
	u32 Status = (u32)XST_FAILURE;

	if(c == NULL) {
		Status = (u32)XST_FAILURE;
		goto END;
	}

	RetVal = strchr(ValidChars, (int)*c);
	if(RetVal != NULL) {
		Status = (u32)XST_SUCCESS;
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * This API initializes the Timer based on platform
 *
 * @return	RefClk will be returned.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 XilSKey_Timer_Intialise(void)
{
	u32 RefClk;

#if defined(XSK_ZYNQ_PLATFORM)
	u32 ArmPllFdiv;
	u32 ArmClkDivisor;
	TimerTicksfor100ns = 0U;
		/**
		 *  Extract PLL FDIV value from ARM PLL Control Register
		 */
	ArmPllFdiv = ((Xil_In32(XSK_ARM_PLL_CTRL_REG) >> 12U) & 0x7FU);
	if(ArmPllFdiv == 0U) {
		return (u32)XST_FAILURE;
	}
		/**
		 *  Extract Clock divisor value from ARM Clock Control Register
		 */
	ArmClkDivisor = ((Xil_In32(XSK_ARM_CLK_CTRL_REG) >> 8U) & 0x3FU);
	if( ArmClkDivisor == 0U) {
		return (u32)XST_FAILURE;
	}

		/**
		 * Initialize the variables
		 */
	RefClk = ((XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ * ArmClkDivisor)/
					ArmPllFdiv);
	/**
	 * Calculate the Timer ticks per 100ns
	 */
	TimerTicksfor100ns =
		(((RefClk * ArmPllFdiv)/ArmClkDivisor) / 2U) / 10000000U;
#elif defined(XSK_MICROBLAZE_PLATFORM)

	u32 Status = (u32)XST_FAILURE;
	TimerTicksfor1000ns = 0U;

	RefClk = XSK_EFUSEPL_CLCK_FREQ_ULTRA;

	Status = (u32)XTmrCtr_Initialize(&XTmrCtrInst, (u16)XTMRCTR_DEVICE_ID);
	if (Status == (u32)XST_FAILURE) {
		return (u32)XST_FAILURE;
	}
	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly.
	 */
	Status = (u32)XTmrCtr_SelfTest(&XTmrCtrInst, XSK_TMRCTR_NUM);
	if (Status != (u32)XST_SUCCESS) {
		return (u32)XST_FAILURE;
	}

	TimerTicksfor1000ns =  XSK_EFUSEPL_CLCK_FREQ_ULTRA/100000U;
#else
	RefClk = 0U;
#endif

	return RefClk;

}

/****************************************************************************/
/**
 * This function Calculates CRC value based on hexadecimal string passed.
 *
 * @param	Key	Pointer to the string contains AES key in hexadecimal
 *		 of length less than or equal to 64.
 *
 * @return
 *		- On Success returns the Crc of AES key value.
 *		- On failure returns the error code when string length
 *		  is greater than 64
 *
 * @note
 *		If the length of the string provided is less than 64, this function
 *		appends the string with zeros.
 *		For calculation of AES key's CRC one can use
 *		u32 XilSKey_CrcCalculation(u8 *Key) API or reverse polynomial
 *      	0x82F63B78.
 * @cond xilskey_internal
 * @{
 * 		In Microblaze CRC will be calculated from 8th word of key to 0th
 * 		word whereas in ZynqMp Ultrascale's PS eFuse from 0th word to
 * 		8th word
 * @}
 @endcond
 *
 ****************************************************************************/
u32 XilSKey_CrcCalculation(const u8 *Key)
{
	u32 Status = (u32)XST_FAILURE;
	u32 CrcReturn = 0U;
	u32 Index;
	u8 MaxIndex = 8U;

#if defined (XSK_MICROBLAZE_PLATFORM) || \
	defined (XSK_ZYNQ_ULTRA_MP_PLATFORM)
	u32 Key_32 = 0U;
	u8 Key_Hex[4U] = {0};
	u8 Key_8[9U] = {0};
#endif
#ifdef XSK_MICROBLAZE_PLATFORM
	u8 Row = 0U;
#endif
	u8 FullKey[XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U] = {0U};
	u32 Length = Xil_Strnlen((const char *)Key,
		XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U);

	if (Length > XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS) {
		CrcReturn = (u32)XSK_EFUSEPL_ERROR_NOT_VALID_KEY_LENGTH;
		goto END;
	}
	if (Length < XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS) {
		Status = (u32)Xil_StrCpyRange(Key,
			&FullKey[XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS - Length + 1U],
			0U, Length - 1U, XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U,
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
	}
	else {
		Status = (u32)Xil_StrCpyRange(Key, FullKey, 0U, Length - 1U,
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U,
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
	}
#ifdef XSK_MICROBLAZE_ULTRA_PLUS
	MaxIndex = 16U;
	Row = 5U;
#endif
#ifdef XSK_MICROBLAZE_ULTRA
	MaxIndex = 8U;
	Row = 20U;
#endif


	for (Index = 0U; Index < MaxIndex; Index++) {
#ifdef XSK_MICROBLAZE_PLATFORM
#ifdef XSK_MICROBLAZE_ULTRA
		Status = (u32)Xil_StrCpyRange(FullKey, Key_8, ((7U - Index) * 8U),
			((((7U - Index) + 1U) * 8U) - 1U),
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U, 9U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
#endif
#ifdef XSK_MICROBLAZE_ULTRA_PLUS
		Status = (u32)Xil_StrCpyRange(FullKey, Key_8,
			(XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS - ((Index + 1U) * 4U)),
			(63U - (Index * 4U)),
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U, 9U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
#endif
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
		Status = (u32)Xil_StrCpyRange(FullKey, Key_8, ((Index) * 8U),
			(((Index * 8U) + 8U) - 1U),
			XSK_EFUSEPS_AES_KEY_SIZE_IN_CHARS + 1U, 9U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
#endif

#if defined (XSK_MICROBLAZE_ULTRA) || \
	defined (XSK_ZYNQ_ULTRA_MP_PLATFORM)
		Status = XilSKey_Efuse_ConvertStringToHexBE((char *)Key_8,
			Key_Hex, 32U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
		Key_32 = (((u32)Key_Hex[0U] << 24U) | ((u32)Key_Hex[1U] << 16U) |
			((u32)Key_Hex[2U] << 8U) | ((u32)Key_Hex[3U]));
#endif
#ifdef	XSK_MICROBLAZE_ULTRA_PLUS
		Status = XilSKey_Efuse_ConvertStringToHexBE((char *)Key_8,
			Key_Hex, 16U);
		if (Status != (u32)XST_SUCCESS) {
			CrcReturn = Status;
			goto END;
		}
		Key_32 = ((u32)Key_Hex[0U] << 8U) | (u32)Key_Hex[1U];
#endif
#ifdef XSK_MICROBLAZE_PLATFORM
		CrcReturn = XilSKey_RowCrcCalculation(CrcReturn, Key_32, (u32)Row + Index);
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
		CrcReturn = XilSKey_RowCrcCalculation(CrcReturn, Key_32, (u32)8U - Index);
#endif

	}

END:
	return CrcReturn;
}

/****************************************************************************/
/**
 * Calculates CRC value for each row of AES key.
 *
 * @param	PrevCRC holds the prev row's CRC.
 * @param	Data holds the present row's key.
 * @param	Addr stores the current row number.
 *
 * @return	Crc of current row.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 XilSKey_RowCrcCalculation(u32 PrevCRC, u32 Data, u32 Addr)
{
	u32 Crc = PrevCRC;
	u32 Value = Data;
	u32 Row = Addr;
	u32 Index;

	for (Index = 0U; Index < 32U; Index++) {
		if ((((Value & 0x1U) ^ Crc) & 0x1U) != 0U) {
			Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >> 1U;
		}
		Value = Value >> 1U;
	}

	for (Index = 0U; Index < 5U; Index++) {
		if ((((Row & 0x1U) ^ Crc) & 0x1U) != 0U) {
			Crc = ((Crc >> 1U) ^ REVERSE_POLYNOMIAL);
		}
		else {
			Crc = Crc >> 1U;
		}
		Row = Row >> 1U;
	}

	return Crc;

}

/****************************************************************************/
/**
 * This API reverse the value.
 *
 * @param	Input is a 32 bit variable
 *
 * @return	Reverse the given value.
 *
 * @note	None.
 *
 ****************************************************************************/
u32 XilSKey_Efuse_ReverseHex(u32 Input)
{
	u32 Index = 0U;
	u32 Rev = 0U;
	u32 Bit;
	u32 InputVar = Input;

	while (Index < 32U) {
		Index = Index + 1U;
		Bit = InputVar & 1U;
		InputVar = InputVar >> 1U;
		Rev = Rev ^ Bit;
		if (Index < 32U) {
			Rev = Rev << 1U;
		}
	}

	return Rev;
}

/****************************************************************************/
/**
 * Calculates CRC value of the provided key. Key should be provided in
 * hexa buffer.
 *
 * @param	Key 	Pointer to an array of 32 bytes, which holds an AES key.
 *
 * @return	Crc of provided AES key value.
 *
 * @cond xilskey_internal
 * @{
 * 		In Microblaze CRC will be calculated from 8th word of key to 0th
 * 		word whereas in ZynqMp Ultrascale's PS eFuse from 0th word to
 * 		8th word
 * @}
 @endcond
 *		To calculate CRC on the AES key in string format please use
 *		XilSKey_CrcCalculation.
 *
 ****************************************************************************/
u32 XilSkey_CrcCalculation_AesKey(const u8 *Key)
{
	u32 Crc = 0U;
	u32 Index;
	u32 MaxIndex;
	u32 Index1 = 0xFFFFFFFFU;
#if defined (XSK_MICROBLAZE_PLATFORM) || \
	defined (XSK_ZYNQ_ULTRA_MP_PLATFORM)
		u32 Key_32 = 0U;
#endif
#ifdef XSK_MICROBLAZE_PLATFORM
	u32 Row = 0U;
#endif

#if defined(XSK_MICROBLAZE_ULTRA_PLUS)
	MaxIndex = 16U;
	Row = 5U;
#elif defined(XSK_MICROBLAZE_ULTRA)
	MaxIndex = 8U;
	Row = 20U;
#elif defined(XSK_ZYNQ_ULTRA_MP_PLATFORM)
	MaxIndex = 8U;
#else
	/* Not supported for other than above platforms */
	MaxIndex = 0U;
	(void) Key;
#endif

	for (Index = 0U; Index < MaxIndex; Index++) {

#if defined(XSK_MICROBLAZE_ULTRA)
		Index1 = (Index * 4U);
#elif defined(XSK_MICROBLAZE_ULTRA_PLUS)
		Index1 = (Index * 2U);
#elif defined(XSK_ZYNQ_ULTRA_MP_PLATFORM)
		Index1 = (((u32)7U - Index) * 4U);
#else
		Crc = Index1;
		break;

#endif

#if defined XSK_MICROBLAZE_ULTRA || \
	defined XSK_ZYNQ_ULTRA_MP_PLATFORM
	Key_32 = ((u32)Key[Index1 + 3U] << 24U) | ((u32)Key[Index1 + 2U] << 16U) |
			((u32)Key[Index1 + 1U] << 8U) | ((u32)Key[Index1 + 0U]);
#endif

#if defined(XSK_MICROBLAZE_ULTRA_PLUS)
	Key_32 = ((u32)Key[Index1 + 1U] << 8U) | (u32)Key[Index1];
#endif

#ifdef XSK_MICROBLAZE_PLATFORM
		Crc = XilSKey_RowCrcCalculation(Crc, Key_32, (u32)(Row + Index));
#endif

#ifdef XSK_ZYNQ_ULTRA_MP_PLATFORM
		Crc = XilSKey_RowCrcCalculation(Crc, Key_32, (u32)8U - Index);
#endif
	}

	return Crc;
}

#if defined (XSK_ZYNQ_ULTRA_MP_PLATFORM) && !defined (XSK_OVERRIDE_SYSMON_CFG)

/***************************************************************************/
/**
* This function is used to check ADC configuration is suitable for XilsKey
*
* @return
* 		- XST_SUCCESS in case of valid ADC configuration for XilSKey
*		- XST_FAILURE in case of invalid ADC configuration for XilSKey
* 		- XSK_EFUSEPS_ERROR_XADC_CONFIG Error occurred with XADC config
*
* TDD Cases:
*
****************************************************************************/

u32 XilSKey_EfusePs_XAdcCfgValidate (void)
{
	u32 Status = (u32)XST_FAILURE;

	XSysMonPsu_Config *ConfigPtr;
	XSysMonPsu *XSysmonInstPtr = &XSysmonInst;

	/**
	 * specify the Device ID that is
	 * generated in xparameters.h
	 */
	XSysmonDevId = (u16)XSYSMON_PSU_DEVICE_ID;

	/**
	 * Initialize the XAdc driver.
	 */
	ConfigPtr = XSysMonPsu_LookupConfig(XSysmonDevId);
	if (NULL == ConfigPtr) {
		Status = (u32)XSK_EFUSEPS_ERROR_XADC_CONFIG;
		goto END;
	}

	XSysMonPsu_InitInstance(XSysmonInstPtr, ConfigPtr);
	Status = XilSKey_Is_Valid_SysMon_Cfg(XSysmonInstPtr);

END:
	return Status;
}

/****************************************************************************/
/**
 * This function checks if sysmon is configured correctly for XilSKey
 * functions to monitor LPD temperature, VPINT, and VPAUX.
 *
 * @param	InstancePtr Instance pointer of SysMon.
 *
 * @return	XST_SUCCESS - If sysmon is configured for XilSKey library usage.
 *		XST_FAILURE - If sysmon is not configured correctly for XilSKey
 *		              library usage.
 * @note	None.
 *
 ****************************************************************************/
static u32 XilSKey_Is_Valid_SysMon_Cfg(XSysMonPsu *InstancePtr)
{
	u32 Status = XST_FAILURE;
	u32 CfgData;
	u32 SeqMode;
	u32 SleepMode;
	u32 Mask;

	/* Calculate the effective baseaddress based on the Sysmon instance. */
	u32 EffectiveBaseAddress =
		XSysMonPsu_GetEffBaseAddress(InstancePtr->Config.BaseAddress,
					XSYSMON_PS);

	/* Read Cfg1 and make sure channels are configured to read in loop */
	CfgData = Xil_In32(EffectiveBaseAddress + XSYSMONPSU_CFG_REG1_OFFSET);
	SeqMode = (CfgData & XSYSMONPSU_CFG_REG1_SEQ_MDE_MASK) >>
	                                     XSYSMONPSU_CFG_REG1_SEQ_MDE_SHIFT;
	if ((SeqMode != XSM_SEQ_MODE_SAFE) &&
	    (SeqMode != XSM_SEQ_MODE_CONTINPASS)) {
		goto END;
	}

	/* When in continuous pass mode, make sure it includes below channels
	 * for sampling
	 *  1. Supply 1 (VPINT)
	 *  2. Supply 3 (VPAUX)
	 *  3. LPD Temperature
	 */
	if (XSM_SEQ_MODE_CONTINPASS == SeqMode) {
		/* Get Channel sequence mask */
		CfgData = Xil_In32(EffectiveBaseAddress +
		                  XSYSMONPSU_SEQ_CH0_OFFSET);

		Mask = XSYSMONPSU_SEQ_CH0_SUP3_MASK |
		       XSYSMONPSU_SEQ_CH0_SUP1_MASK |
		       XSYSMONPSU_SEQ_CH0_TEMP_MASK;

		/* Make sure required channels for XilSKey are enabled */
		if ((CfgData & Mask) != Mask) {
			goto END;
		}
	}

	/* Read Cfg2 and make sure sysmon is not in power save mode */
	CfgData = Xil_In32(EffectiveBaseAddress + XSYSMONPSU_CFG_REG2_OFFSET);
	SleepMode = (CfgData & XSYSMONPSU_CFG_REG2_PWR_DOWN_MASK) >>
	                                    XSYSMONPSU_CFG_REG2_PWR_DOWN_SHIFT;
	if (XSM_PWR_MODE_NORMAL == SleepMode) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
#endif

#ifdef XSK_MICROBLAZE_PLATFORM
/****************************************************************************/
/**
*
* This function provides correct SLR number to be accessed for corresponding
* config order index.
*
* @param	ConfigOrderIndex	provide config order index of SLR
* @param	SlrNum		Pointer to SLR number, where this API updates with
* 			corresponding SLR number.
*
* @return	None.
*
* @note	If master SLR of target device is SLR0 then both config order index
* 		and SLR number are same in order.
* 		If master SLR of target device is SLR1 then config order and SLR
* 		numbers are as below.
* 		SLR 1 = config order index 0
* 		SLR 0 = config order index 1
* 		SLR 2 = config order index 2
* 		SLR 3 = config order index 3
* 		Configuration order index is the order of SLRs starting from master
* 		followed by slaves.
*
*****************************************************************************/
void XilSKey_GetSlrNum(u32 MasterSlrNum, u32 ConfigOrderIndex, u32 *SlrNum)
{
	/* If master SLR is SLR 0 */
	if (MasterSlrNum == XSK_SLR_NUM_0) {
		*SlrNum = ConfigOrderIndex;
	}
	else {
		/* If master SLR is SLR 1 */
		if (ConfigOrderIndex == XSK_SLR_CONFIG_ORDER_0) {
			/* Master SLR is 1 */
			*SlrNum = XSK_SLR_NUM_1;
		}
		else if (ConfigOrderIndex == XSK_SLR_CONFIG_ORDER_1) {
			/* SLave 0 is SLR 0 */
			*SlrNum = XSK_SLR_NUM_0;
		}
		else {
			/* remaining are same */
			*SlrNum = ConfigOrderIndex;
		}
	}
}
#endif
