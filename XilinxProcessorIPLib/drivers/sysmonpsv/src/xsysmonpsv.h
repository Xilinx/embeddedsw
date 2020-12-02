/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xsysmonpsv.h
* @addtogroup sysmonpsv_v2_0
*
* The XSysMon driver supports the Xilinx System Monitor device on Versal
*
* The System Monitor device has the following features:
*		- Measure and monitor up to 160 voltages across the chip
*		- Automatic alarms based on user defined limis for the
*		  on-chip supply voltages and temperature.
*		- Optional interrupt request generation
*
*
* The user should refer to the hardware device specification for detailed
* information about the device.
*
* This header file contains the prototypes of driver functions that can
* be used to access the System Monitor device.
*
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the System Monitor device.
*
* XSysMonPsv_CfgInitialize() API is used to initialize the System Monitor
* device. The user needs to first call the XSysMonPsv_LookupConfig() API which
* returns the Configuration structure pointer which is passed as a parameter to
* the XSysMonPsv_CfgInitialize() API.
*
*
* <b>Interrupts</b>
*
* The System Monitor device supports interrupt driven mode and the default
* operation mode is polling mode.
*
* This driver does not provide a Interrupt Service Routine (ISR) for the device.
* It is the responsibility of the application to provide one if needed. Refer to
* the interrupt example provided with this driver for details on using the
* device in interrupt mode.
*
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
*
* <b> Building the driver </b>
*
* The XSysMonPsv driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
*
* <b> Limitations of the driver </b>
*
* System Monitor device can be accessed through the JTAG port and the AXI
* interface. The driver implementation does not support the simultaneous access
* of the device by both these interfaces. The user has to take care of this
* situation in the user application code.
*
*
*
* <br><br>
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    02/08/18 First release
* 1.2	aad    06/14/20 Fixed temperature calculation for negative temp
* 2.0   aad    07/31/20 Added new APIs to set threshold values, alarm
*                       config and modes for temperature and voltages.
*                       Added new interrupt handling structure.
*       aad    10/12/20 Fixed MISRAC violations
*
* </pre>
*
******************************************************************************/


#ifndef XSYSMONPSV_H_			/* prevent circular inclusions */
#define XSYSMONPSV_H_			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xstatus.h"
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv_supplylist.h"
/************************** Constant Definitions *****************************/
#define XSYSMONPSV_MAX_SUPPLIES		160U
#define XSYSMONPSV_INVALID_SUPPLY	160U
#define XSYSMONPSV_PMBUS_INTERFACE	0U
#define XSYSMONPSV_I2C_INTERFACE	1U
#define XSYSMONPSV_INVALID		0x80000000U
#define XSYSMONPSV_MAX_SUPPLY_REG	0xA0U
#define XSYSMONPSV_EXPONENT_RANGE_16	16U
#define XSYSMONPSV_QFMT_SIGN		15U
#define XSYSMONPSV_QFMT_FRACTION	128
#define XSYSMONPSV_UP_SAT_SIGNED	32767
#define XSYSMONPSV_UP_SAT		65535
#define XSYSMONPSV_LOW_SAT_SIGNED	-32767
#define XSYSMONPSV_LOW_SAT		0
#define XSYSMONPSV_BIPOLAR_UP_SAT	0x7FFF
#define XSYSMONPSV_BIPOLAR_LOW_SAT	0x8000
#define XSYSMONPSV_UNIPOLAR_UP_SAT	0xFFFF
#define XSYSMONPSV_UNIPOLAR_LOW_SAT	0x0000
#define XSYSMONPSV_ENABLE		1U
#define XSYSMONPSV_DISABLE		0U
#define XSYSMONPSV_HYSTERESIS		1U
#define XSYSMONPSV_WINDOW		0U

/**************************** Type Definitions *******************************/

/******************************************************************************/
/**
 * This data type defines a handler that an application defines to communicate
 * with interrupt system to retrieve state information about an application.
 *
 * @param	CallbackRef is a callback reference passed in by the upper layer
 *		when setting the handler, and is passed back to the upper layer
 *		when the handler is called. It is used to find the device driver
 *		instance.
 * @param	Value is the which the event occured
 *
 ******************************************************************************/
typedef void (*XSysMonPsv_Handler) (void *CallbackRef, u32 *Value);


/*@}*/

/**
 * @name This typedef defines Threshold types.
 * @{
 */
typedef enum {
	XSYSMONPSV_TH_LOWER,
	XSYSMONPSV_TH_UPPER,
} XSysMonPsv_Threshold;

/*@}*/

/**
 * @name This typedef defines value types for voltage readings.
 * @{
 */
typedef enum {
	XSYSMONPSV_VAL,			/**< Supply Value */
	XSYSMONPSV_VAL_MIN,		/**< Minimum Value reached since
					  reset */
	XSYSMONPSV_VAL_MAX,		/**< Maximum Value reached since
					  reset */
	XSYSMONPSV_VAL_VREF_MIN,	/**< Minimum value reached for a
					  temperature since last VRef */
	XSYSMONPSV_VAL_VREF_MAX,	/**< Maximum value reached for a
					  temperature since last VRef */
} XSysMonPsv_Val;

/*@}*/

/**
 * @name This typedef defines types of supply values.
 * @{
 */
typedef enum {
	XSYSMONPSV_1V_UNIPOLAR,
	XSYSMONPSV_2V_UNIPOLAR,
	XSYSMONPSV_4V_UNIPOLAR,
	XSYSMONPSV_1V_BIPOLAR,
} XSysMonPsv_VoltageScale;

/*@}*/

/**
 * @name This typedef contains configuration information for a device.
 * @{
 */
typedef struct {
	UINTPTR BaseAddress;	/**< Register base address */
	u8 Supply_List[XSYSMONPSV_MAX_SUPPLIES];/**< Maps voltage supplies in
                                                  use to the Supply registers */
} XSysMonPsv_Config;

/*@}*/

/**
 * @name This is an interrupt callback structure where callbacks and the
 * callback reference is stored.
 * @{
 */
typedef struct {
	XSysMonPsv_Handler Handler;	/**< Event handler */
	void *CallbackRef;		/**< Callback reference for
					  event handler */
	XSysMonPsv_Supply Supply;	/**<Supply for which event is set */
	u32 IsCallbackSet;
} XSysMonPsv_EventHandler;

/*@}*/

/**
 * @name The XSysmonPsv driver instance data. The user is required to allocate a
 * variable of this type for the SYSMON device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 * @{
 */
typedef struct {
	XSysMonPsv_Config Config;	/**< Device configuration */
	XSysMonPsv_EventHandler
		SupplyEvent[XSYSMONPSV_MAX_SUPPLIES];	/**< EventList will
							  have callbacks for
							  events supported
							  by sysmon */
	XSysMonPsv_EventHandler TempEvent; /**< Device Temperature event
					    handler information */
	XSysMonPsv_EventHandler OTEvent; /**< OT event handler information */
	u32 IsReady;
} XSysMonPsv;

/*@}*/

/************************* Variable Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* This macro returns the XSYSMONPSV_NEW_ALARMn_MASK for a configured supply.
*
* @param	Supply is a type enum of supply enabled
*
* @return	A 32 bit mask to be used for configuring interrupts
*
* @note		None
*
*****************************************************************************/
#define XSysMonPsv_GetAlarmMask(InstancePtr, Supply)		\
	Mask = 1U << (InstancePtr->Supply_List[Supply]/32U)

/****************************************************************************/
/**
*
* This function converts raw AdcData into Voltage value.
*
* @param	AdcData is the System Monitor ADC Raw Data.
*
* @return	The Voltage in volts.
*
* @note		None.
*
*****************************************************************************/
static inline float XSysMonPsv_RawToVoltage(u32 AdcData)
{
	u32 Mantissa, Scale, Format, Exponent;

	Mantissa = AdcData & XSYSMONPSV_SUPPLY_MANTISSA_MASK;
	Exponent = (AdcData & XSYSMONPSV_SUPPLY_MODE_MASK) >>
		XSYSMONPSV_SUPPLY_MODE_SHIFT;
	Format = (AdcData & XSYSMONPSV_SUPPLY_FMT_MASK) >>
	       XSYSMONPSV_SUPPLY_FMT_SHIFT;

	/* Calculate the exponent
	 * 2^(16-Exponent)
	 */
	Scale = ((u32)1U << (XSYSMONPSV_EXPONENT_RANGE_16 - Exponent));
	if ((Format & (Mantissa  >> XSYSMONPSV_SUPPLY_MANTISSA_SIGN)) == 1U) {
		return	((float)Mantissa/(float)Scale) - 1.0f;
	}
	else {
		return (float)Mantissa/(float)Scale;
	}
}

/****************************************************************************/
/**
*
* This function converts raw AdcData into Voltage value.
*
* @param	Volts is the voltage value to be converted
* @param	Type is the type of supply,
*		Type = 0, Unipolar
*		Type = 1, Bipolar
*
* @return	The Voltage in Raw ADC format.
*
* @note		None.
*
*****************************************************************************/
static inline u16 XSysMonPsv_VoltageToRaw(float Volts,
					  XSysMonPsv_VoltageScale Type)
{
	u32 Format = 0;
	u32 Exponent = 16U;
	u32 Scale;
	int TmpVal;
	float TmpFloat;

	if (Type != XSYSMONPSV_1V_BIPOLAR) {
		Exponent -= (u32)Type;
	} else {
		Format = 1U;
	}

	Scale = ((u32)1U << (16U - Exponent));
	TmpFloat = (Volts * (float)Scale);

	TmpVal = (int) TmpFloat;

	if(Format == 1U) {
		if (TmpVal > XSYSMONPSV_UP_SAT_SIGNED) {
			TmpVal = XSYSMONPSV_BIPOLAR_UP_SAT;
		} else if (TmpVal < XSYSMONPSV_LOW_SAT_SIGNED) {
			TmpVal = XSYSMONPSV_BIPOLAR_LOW_SAT;
		}
	} else {
		if (TmpVal > XSYSMONPSV_UP_SAT) {
			TmpVal = XSYSMONPSV_UNIPOLAR_UP_SAT;
		} else if (TmpVal < XSYSMONPSV_LOW_SAT) {
			TmpVal = XSYSMONPSV_UNIPOLAR_LOW_SAT;
		}
	}

	return (u16)((u32)TmpVal & 0xFFFFU);
}

/****************************************************************************/
/**
*
* This function converts the fixed point to degree celsius
*
* @param	FixedQFmt is Q8.7 representation of temperature value.
*
* @return	The Temperature in degree celsisus
*
* @note		None.
*
*****************************************************************************/
static inline float XSysMonPsv_FixedToFloat(u32 FixedQFmt)
{
	u32 TwosComp;
	float Temperature;
	if((FixedQFmt >> XSYSMONPSV_QFMT_SIGN) > 0U) {
		TwosComp = (~(FixedQFmt) + 1U) & (0x000FFFFU);
		Temperature = (float)(TwosComp) /
			((float)XSYSMONPSV_QFMT_FRACTION * (-1.0f));
	}
	else {
		Temperature = (float)FixedQFmt /
			((float)XSYSMONPSV_QFMT_FRACTION * (1.0f));
	}
	return Temperature;
}

/****************************************************************************/
/**
*
* This function converts the floating point to Fixed Q8.7 format
*
* @param	Temp is temperature value in Deg Celsius
*
* @return	temperature value in fixed Q8.7 format.
*
* @note		None.
*
*****************************************************************************/
static inline u16 XSysMonPsv_FloatToFixed(float Temp)
{
	float ScaledDown;
	int RawAdc;

	ScaledDown = (float)(Temp * 128.0f);
	RawAdc = (int)ScaledDown;

	return (u16)RawAdc;
}

/************************** Function Prototypes ******************************/

/* Functions in xsysmonpsv.c */
s64 XSysMonPsv_CfgInitialize(XSysMonPsv *InstancePtr, XSysMonPsv_Config *CfgPtr);
void XSysMonPsv_SystemReset(XSysMonPsv *InstancePtr);
void XSysMonPsv_EnRegGate(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_SetPMBusAddress(XSysMonPsv *InstancePtr, u8 Address);
void XSysMonPsv_PMBusEnable(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_PMBusEnableCmd(XSysMonPsv *InstancePtr, u8 Enable);
void XSysMonPsv_SelectExtInterface(XSysMonPsv *InstancePtr, u8 Interface);
void XSysMonPsv_StatusReset(XSysMonPsv *InstancePtr,
			    u8 ResetSupply, u8 ResetTemperature);
u16 XSysMonPsv_ReadDevTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType);
void XSysMonPsv_SetDevTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType,
				    u16 Value);
u16 XSysMonPsv_ReadOTTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType);
void XSysMonPsv_SetOTTempThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Threshold ThresholdType,
				   u16 Value);
u32 XSysMonPsv_ReadDeviceTemp(XSysMonPsv *InstancePtr, XSysMonPsv_Val Value);
u32 XSysMonPsv_ReadSupplyThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply,
				   XSysMonPsv_Threshold ThresholdType);
u32 XSysMonPsv_ReadSupplyValue(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply, XSysMonPsv_Val Value);
u32 XSysMonPsv_IsNewData(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply);
u32 XSysMonPsv_IsAlarmCondition(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply);
u32 XSysMonPsv_SetSupplyUpperThreshold(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Supply Supply, u32 Value);
u32 XSysMonPsv_SetSupplyLowerThreshold(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Supply Supply, u32 Value);
void XSysMonPsv_SetTempMode(XSysMonPsv *InstancePtr, u32 Mode);
void XSysMonPsv_SetOTMode(XSysMonPsv *InstancePtr, u32 Mode);
u32 XSysMonPsv_ReadAlarmConfig(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply);
u32 XSysMonPsv_SetAlarmConfig(XSysMonPsv *InstancePtr,
			      XSysMonPsv_Supply Supply, u32 Config);

/* Interrupt functions in xsysmonpsv_intr.c */
void XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
void XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
u32 XSysMonPsv_IntrGetEnabled(XSysMonPsv *InstancePtr, u8 IntrNum);
u32 XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr);
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask);
void XSysMonPsv_SetNewDataIntSrc(XSysMonPsv *InstancePtr,
				XSysMonPsv_Supply Supply, u32 Mask);
void XSysMonPsv_SetTempEventHandler(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Handler CallbackFunc,
				    void *CallbackRef);
void XSysMonPsv_SetOTEventHandler(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Handler CallbackFunc,
				  void *CallbackRef);
void XSysMonPsv_SetSupplyEventHandler(XSysMonPsv *InstancePtr,
				      XSysMonPsv_Supply Supply,
				      XSysMonPsv_Handler CallbackFunc,
				      void *CallbackRef);
void XSysMonPsv_AlarmEventHandler(XSysMonPsv *InstancePtr);

/* Functions in xsysmonpsv_sinit.c */
XSysMonPsv_Config *XSysMonPsv_LookupConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* XSYSMONPSV_H_ */
