/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv.c
* @addtogroup sysmonpsv_v2_0
*
* Functions in this file are the minimum required functions for the XSysMonPsv
* driver. See xsysmonpsv.h for a detailed description of the driver.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	    Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    11/20/18 First release.
* 1.2   aad    06/11/20 Corrected the configuration assignment
* 2.0   aad    07/31/20 Added new APIs to set threshold values, alarm
*                       config and modes for temperature and voltages
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmonpsv_hw.h"
#include "xsysmonpsv.h"
#include "xil_assert.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/*****************************************************************************/
/**
*
* This function initializes XSysMonPsv device/instance. This function
* must be called prior to using the System Monitor device.
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
* @param	CfgPtr points to the XSysMonPsv device configuration
*		structure.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		The user needs to first call the XSysMonPsv_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XSysMonPsv_CfgInitialize() API.
*
******************************************************************************/
s64 XSysMonPsv_CfgInitialize(XSysMonPsv *InstancePtr, XSysMonPsv_Config *CfgPtr)
{
	u32 i;

	/* Assert the input arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Set the values read from the device config and the base address. */
	InstancePtr->Config.BaseAddress = CfgPtr->BaseAddress;

	/* Map Supplies to supply registers */
	for(i = 0U; i < XSYSMONPSV_MAX_SUPPLIES; i++) {
		InstancePtr->Config.Supply_List[i] = CfgPtr->Supply_List[i];
	}

	/* Indicate the instance is now ready to use, initialized without error */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets the SystemMonitor
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XSysMonPsv_SystemReset(XSysMonPsv *InstancePtr)
{
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Mask PCSR Register */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_PCSR_MASK,
			    XSYSMONPSV_PCSR_MASK_SYS_RST_MASK_MASK);

	/* RESET the SYSMON */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_PCSR_CONTROL,
			    XSYSMONPSV_PCSR_CONTROL_SYS_RST_MASK_MASK);
}

/*****************************************************************************/
/**
*
* This function Gates the register outputs
*
* @param	InstancePtr is a pointer to the XSysMonPsv instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XSysMonPsv_EnRegGate(XSysMonPsv *InstancePtr, u8 Enable)
{
	u32 RegVal;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Enable <= 1U);

	/* Mask PCSR Register */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_PCSR_MASK,
			    XSYSMONPSV_PCSR_MASK_GATEREG_MASK);

	RegVal= ((u32)Enable << XSYSMONPSV_PCSR_CONTROL_GATEREG_SHIFT);
	/* RESET the SYSMON */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_PCSR_CONTROL, RegVal);

}

/*****************************************************************************/
/**
*
* This function sets the PMBus address for the System Monitor.
*
* @param	InstancePtr is a pointer to the device instance.
* @param	Address is a value at which the Sysmon will be addressed on
*		the PMBus.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XSysMonPsv_SetPMBusAddress(XSysMonPsv *InstancePtr, u8 Address)
{
	u32 Reg;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Address < 128U);

	Reg = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				XSYSMONPSV_CONFIG0);
	Reg &= ~(XSYSMONPSV_CONFIG0_PMBUS_ADDRESS_MASK);
	Reg |= Address;

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_CONFIG0, Reg);
}

/*****************************************************************************/
/**
*
*  This function enables/disbles the PMBus on the System Monitor
*
*  @param	InstancePtr is a pointer to the driver instance.
*  @param	Enable is the value which enables or disables the PMBus.
*
*  @return	None.
*
*  @note	If Enable = 0, PMBus lines will be in tristate.
*		If Enable = 1, PMBus lines will be active.
*
******************************************************************************/
void XSysMonPsv_PMBusEnable(XSysMonPsv *InstancePtr, u8 Enable)
{
	u32 Reg;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Enable <= 1U);

	Reg = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				 XSYSMONPSV_CONFIG0);

	Reg &= ~(XSYSMONPSV_CONFIG0_PMBUS_ENABLE_MASK);
	Reg |= ((u32)Enable << XSYSMONPSV_CONFIG0_PMBUS_ENABLE_SHIFT);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_CONFIG0, Reg);
}

/*****************************************************************************/
/**
*
*  This function restricts or de-restrict all  PMBs commands
*
*  @param	InstancePtr is a pointer to the driver instance.
*  @param	Enable is the value which restrics or de-restrict the PMBus
*		commands.
*
*  @return	None.
*
*  @note	If Enable = 0, PMBus commands are disabled.
*		If Enable = 1, PMBus commands are enabled.
*
******************************************************************************/
void XSysMonPsv_PMBusEnableCmd(XSysMonPsv *InstancePtr, u8 Enable)
{
	u32 Reg;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Enable <= 1U);

	Reg = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				 XSYSMONPSV_CONFIG0);

	Reg &= ~(XSYSMONPSV_CONFIG0_PMBUS_UNRESTRICTED_MASK);
	Reg |= ((u32)Enable << XSYSMONPSV_CONFIG0_PMBUS_UNRESTRICTED_SHIFT);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_CONFIG0, Reg);
}

/*****************************************************************************/
/**
*
*  This function selects I2C or PMBus interface to which traffic from SMBus
*  is routed to.
*
*  @param	InstancePtr is a pointer to the driver instance.
*  @param	Interface is a value which determies where the SMBus traffic
*		gets routed to
*
*  @return	None.
*
*  @note	If Interface = 1, I2C command interface is enabled.
*		If Interface = 0, PMBus command interface is enabled.
*
******************************************************************************/
void XSysMonPsv_SelectExtInterface(XSysMonPsv *InstancePtr, u8 Interface)
{
	u32 Reg;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Interface <= 1U);

	Reg = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				 XSYSMONPSV_CONFIG0);

	Reg &= ~(XSYSMONPSV_CONFIG0_I2C_NOT_PMBUS_MASK);
	Reg |= ((u32)Interface << XSYSMONPSV_CONFIG0_I2C_NOT_PMBUS_SHIFT);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_CONFIG0, Reg);

}

/*****************************************************************************/
/**
*
* This function Resets the Min and Max values of Supplies and Temperature to
* negative and positive saturation respectively.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	ResetSupply will reset the MIN and MAX values reached by the
*		internal supplies since the last reset.
* @param	ResetTemperature will reset the MIN and MAX values reached by
*		the device since the last reset.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XSysMonPsv_StatusReset(XSysMonPsv *InstancePtr, u8 ResetSupply,
			    u8 ResetTemperature)
{
	u8 Value;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ResetSupply <= 1U);
	Xil_AssertVoid(ResetTemperature <= 1U);

	Value = ResetTemperature |
		(ResetSupply << XSYSMONPSV_STATUS_RESET_SUPPLY_SHIFT);
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_STATUS_RESET, Value);
}

/*****************************************************************************/
/**
*
* This function returns the device temperature threshold for min and max values
* in signed Q8.7 format.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	ThresholdType is an enum which indicates the type of threshold
*
* @return	Device Temperature threshold in signed Q8.7 format.
*
* @note		To get the value in Deg Celsius, use XSysMonPsv_FixedToFloat.
*
******************************************************************************/
u16 XSysMonPsv_ReadDevTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType)
{
	u32 Offset = XSYSMONPSV_DEVICE_TEMP_TH + ((u32)ThresholdType * 4U);
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (u16)XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset);
}

/*****************************************************************************/
/**
*
* This function sets Device Temperature Threshold values.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	ThresholdType is an enum which indicates the type of threshold.
* @param	Value is the raw ADC threshold value.
*
* @return	None.
*
* @note		To get the raw ADC value, use XSysMonPsv_FloatToFixed.
*
******************************************************************************/
void XSysMonPsv_SetDevTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType,
				   u16 Value)
{
	u32 Offset = XSYSMONPSV_DEVICE_TEMP_TH + ((u32)ThresholdType * 4U);
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Value);
}

/*****************************************************************************/
/**
*
* This function returns the OT temperature threshold for min and max values in
* signed Q8.7 format.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	ThresholdType is an enum which indicates the type of threshold
*
* @return	OT Temperature threshold in signed Q8.7 format.
*
* @note		None.
*
******************************************************************************/
u16 XSysMonPsv_ReadOTTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType)
{
	u32 Offset = XSYSMONPSV_OT_TEMP_TH + ((u32)ThresholdType * 4U);
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return (u16)XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset);
}

/*****************************************************************************/
/**
*
* This function sets OT Temperature Threshold values.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	ThresholdType is an enum which indicates the type of threshold.
* @param	Value is the raw ADC threshold value.
*
* @return	None.
*
* @note		To get the raw ADC value, use XSysMonPsv_FloatToFixed.
*
******************************************************************************/
void XSysMonPsv_SetOTTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType,
				   u16 Value)
{
	u32 Offset = XSYSMONPSV_OT_TEMP_TH + ((u32)ThresholdType * 4U);
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Value);
}

/*****************************************************************************/
/**
*
* This function returns the temperature values for the device in signed Q8.7
* format.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Value is an enum which indicates the typde of temperature value
*		to be read
* @param	Temp is a pointer to which the device temperature is written to.
*
* @return	Temperature value requested
*		XSYSMONPSV_FAILURE if invalid value requested
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_ReadDeviceTemp(XSysMonPsv *InstancePtr, XSysMonPsv_Val Value)
{
	u32 Offset;
	u32 Temperature;
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	switch(Value) {
		case XSYSMONPSV_VAL_VREF_MIN:
			Offset = XSYSMONPSV_DEVICE_TEMP_MIN;
			break;

		case XSYSMONPSV_VAL_VREF_MAX:
			Offset = XSYSMONPSV_DEVICE_TEMP_MAX;
			break;

		case XSYSMONPSV_VAL_MIN:
			Offset = XSYSMONPSV_DEVICE_TEMP_MIN_MIN;
			break;

		case XSYSMONPSV_VAL_MAX:
			Offset = XSYSMONPSV_DEVICE_TEMP_MAX_MAX;
			break;

		default:
			Temperature = XSYSMONPSV_INVALID;
			goto END;
			break;
	}

	Temperature = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
					 Offset);
END:
	return Temperature;
}

/*****************************************************************************/
/**
*
* This function reads the raw value for Supply Threshold.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply to be read
* @param	ThresholdType is an enum which indicates the type of threshold
*
* @return	The raw Upper or Lower threshold of the Supply
*		XSYSMONPSV_INVALID if the Supply hasn't been configured
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_ReadSupplyThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply,
				   XSysMonPsv_Threshold ThresholdType)
{
	u32 Offset;
	u8 SupplyReg;
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if(InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	if(ThresholdType ==  XSYSMONPSV_TH_LOWER) {
		Offset = XSYSMONPSV_SUPPLY_TH_LOWER;
	} else {
		Offset = XSYSMONPSV_SUPPLY_TH_UPPER;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);
	return XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset);
}

/*****************************************************************************/
/**
*
* This function reads the raw Supply Value for requested.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply to be read
* @param	Value is the type of reading for the Supply
*
* @return	The raw values for Min, Max or the prevailing Supply Value.
*		Invalid if the Supply hasn't been configured
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_ReadSupplyValue(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply, XSysMonPsv_Val Value)
{
	u32 Offset;
	u8 SupplyReg;
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	if (Value == XSYSMONPSV_VAL) {
		Offset = XSYSMONPSV_SUPPLY;
	} else if (Value == XSYSMONPSV_VAL_MIN) {
		Offset = XSYSMONPSV_SUPPLY_MIN;
	} else {
		Offset = XSYSMONPSV_SUPPLY_MAX;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);

	return XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset);
}

/*****************************************************************************/
/**
*
* This function is to be used to check if new data is available for a supply.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply.
*
* @return	True if new data available
*		False if new data isn't available
*		Invalid if the Supply hasn't been configured
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_IsNewData(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply)
{
	u8 Offset;
	u8 Shift;
	u8 SupplyReg;
	u32 Status;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the New data flag */
	Status = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset +
				    XSYSMONPSV_NEW_DATA_FLAG0);
        Status &= ((u32)1U << Shift);

	/* Clear the New data flag if its set */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset +
			    XSYSMONPSV_NEW_DATA_FLAG0, Status);

	return ((Status > 0U) ? 1U : 0U);

}

/*****************************************************************************/
/**
*
* This function is to be used to check if the supply value has exceeded the set
* threshold values.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply.
*
* @return	True if new data available
*		False if new data isn't available
*		Invalid if the Supply hasn't been configured
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_IsAlarmCondition(XSysMonPsv *InstancePtr,
				XSysMonPsv_Supply Supply)
{
	u8 Offset;
	u8 Shift;
	u8 SupplyReg;
	u32 Status;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the New data flag */
	Status = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset +
				    XSYSMONPSV_ALARM_FLAG0);
        Status &= ((u32)1U << Shift);

	/* Clear the New data flag if its set */
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset +
			    XSYSMONPSV_ALARM_FLAG0, Status);

	return ((Status > 0U) ? 1U : 0U);

}
/*****************************************************************************/
/**
*
* This function sets the raw value for Upper Supply Threshold.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply to be
*		configured
* @param	ThresholdType is an enum which indicates the type of threshold
*
* @return	XSYSMONPSV_INVALID if the Supply hasn't been configured
*		XST_SUCCESS otherwise
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_SetSupplyUpperThreshold(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Supply Supply, u32 Value)
{
	u32 Offset;
	u8 SupplyReg;
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	Offset = XSYSMONPSV_SUPPLY_TH_UPPER;

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Value);

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function sets the raw value for Lower Supply Threshold.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is an enum which indicates the desired supply to be
*		configured
* @param	ThresholdType is an enum which indicates the type of threshold
*
* @return	XSYSMONPSV_INVALID if the Supply hasn't been configured
*               XST_SUCCESS otherwise
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_SetSupplyLowerThreshold(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Supply Supply, u32 Value)
{
	u32 Offset;
	u8 SupplyReg;
	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	Offset = XSYSMONPSV_SUPPLY_TH_LOWER;

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset += ((u32)SupplyReg * 4U);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset, Value);

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function sets the alarm mode for Temperature alarms.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Mode sets the Hysteresis or Window mode.
*		Mode = 1 Hysteresis Mode
*		Mode = 0 Window Mode
*
* @return	None.
* @note		None.
*
******************************************************************************/
void XSysMonPsv_SetTempMode(XSysMonPsv *InstancePtr, u32 Mode)
{
	u32 RegVal;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mode < 2U);

	RegVal = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				    XSYSMONPSV_ALARM_CONFIG);
	RegVal &= ~(XSYSMONPSV_ALARM_CONFIG_DEV_ALARM_MODE_MASK);
	RegVal |= (Mode << XSYSMONPSV_ALARM_CONFIG_DEV_ALARM_MODE_SHIFT);
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_ALARM_CONFIG, RegVal);

}

/*****************************************************************************/
/**
*
* This function sets the alarm mode for OT alarm.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Mode sets the Hysteresis or Window mode.
*               Mode = XSYSMONPSV_HYSTERESIS Hysteresis Mode
*               Mode = XSYSMONPSV_WINDOW Window Mode
*
* @return	None.
* @note		None.
*
******************************************************************************/
void XSysMonPsv_SetOTMode(XSysMonPsv *InstancePtr, u32 Mode)
{
	u32 RegVal;
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Mode < 2U);

	RegVal = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress +
				    XSYSMONPSV_ALARM_CONFIG);
	RegVal &= ~(XSYSMONPSV_ALARM_CONFIG_OT_ALARM_MODE_MASK);
	RegVal |= (Mode << XSYSMONPSV_ALARM_CONFIG_OT_ALARM_MODE_SHIFT);
	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress +
			    XSYSMONPSV_ALARM_CONFIG, RegVal);
}

/*****************************************************************************/
/**
*
* This function reads the current Supply Alarm Config.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is the supply for which config is to be read.
*
* @return	XSYSMONPSV_ENABLE if enabled
*		XSYSMONPSV_DISABLE if disbaled
*		XSYSMONPSV_INVALID if invalid SupplyValue
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_ReadAlarmConfig(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply)
{
	u32 Status;
        u32 Shift;
        u32 SupplyReg;
        u32 Offset;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the Alarm flag */
	Status = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset +
				    XSYSMONPSV_ALARM_REG0) >> Shift;

	return (Status & 1U);

}

/*****************************************************************************/
/**
*
* This function sets Alarm config for a supply.
*
* @param	InstancePtr is a pointer to the driver instance.
* @param	Supply is the supply for which config is to be set.
* @param	Config is the alarm config value.
*               XSYSMONPSV_ENABLE to enable
*		XSYSMONPSV_DISABLE to disable
*
* @return	XST_SUCCESS if successful
*		XSYSMONPSV_INVALID if invalid SupplyValue
*
* @note		None.
*
******************************************************************************/
u32 XSysMonPsv_SetAlarmConfig(XSysMonPsv *InstancePtr,
			      XSysMonPsv_Supply Supply, u32 Config)
{
	u32 Status;
        u32 Shift;
        u32 Offset;
        u32 SupplyReg;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	if (InstancePtr->Config.Supply_List[Supply] == XSYSMONPSV_INVALID_SUPPLY) {
		return XSYSMONPSV_INVALID;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Offset = 4U * (SupplyReg / 32U);
	Shift = SupplyReg % 32U;

	/* Read the Alarm flag */
	Status = XSysMonPsv_ReadReg(InstancePtr->Config.BaseAddress + Offset +
				    XSYSMONPSV_ALARM_REG0);
	Status &= ~((u32)1U << Shift);
	Status |= (Config << Shift);

	XSysMonPsv_WriteReg(InstancePtr->Config.BaseAddress + Offset +
			    XSYSMONPSV_ALARM_REG0, Status);

	return XST_SUCCESS;

}

/** @} */
