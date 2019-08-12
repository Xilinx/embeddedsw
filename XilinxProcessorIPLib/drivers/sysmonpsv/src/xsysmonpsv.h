/******************************************************************************
*
* Copyright (C) 2016 - 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xsysmonpsv.h
* @addtogroup sysmonpsv_v1_1
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
* 1.00  aad    08/02/18 First release
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
#define XSYSMONPSV_MAX_SUPPLIES		185
#define XSYSMONPSV_MAX_ACTIVE_MEAS	160
#define XSYSMONPSV_INVALID_SUPPLY	160
#define XSYSMONPSV_PMBUS_INTERFACE	0
#define XSYSMONPSV_I2C_INTERFACE	1
#define XSYSMONPSV_INVALID		0x80000000
#define XSYSMONPSV_MAX_SUPPLY_REG	0xA0
#define XSYSMONPSV_EXPONENT_RANGE_16	16
#define XSYSMONPSV_QFMT_SIGN		15
#define XSYSMONPSV_QFMT_FRACTION	7

/**************************** Type Definitions *******************************/

/******************************************************************************/
/**
 * This data type defines a handler that an application defines to communicate
 * with interrupt system to retrieve state information about an application.
 *
 * @param	CallBackRef is a callback reference passed in by the upper layer
 *		when setting the handler, and is passed back to the upper layer
 *		when the handler is called. It is used to find the device driver
 *		instance.
 *
 ******************************************************************************/
typedef void (*XSysMonPsv_Handler) (void *CallBackRef);


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
 * @name This typedef contains configuration information for a device.
 * @{
 */
typedef struct {
	u32 BaseAddress;	/**< Register base address */
	u8 Supply_List[XSYSMONPSV_MAX_SUPPLIES];/**< Maps voltage supplies in
						  use to the Supply registers */
} XSysMonPsv_Config;

/*@}*/


/**
 * @name The XSysmonPsv driver instance data. The user is required to allocate a
 * variable of this type for the SYSMON device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 * @{
 */
typedef struct {
	XSysMonPsv_Config Config;	/**< Device configuration */
	XSysMonPsv_Handler Handler;	/**< Event handler */
	void *CallBackRef;		/**< Callback reference for
					  event handler */
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
	Mask = 1 << (InstancePtr->Supply_List[Supply]/32)

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
	int Mantissa, Scale, Format, Exponent;

	Mantissa = AdcData & XSYSMONPSV_SUPPLY_MANTISSA_MASK;
	Exponent = (AdcData & XSYSMONPSV_SUPPLY_MODE_MASK) >>
		XSYSMONPSV_SUPPLY_MODE_SHIFT;
	Format = (AdcData & XSYSMONPSV_SUPPLY_FMT_MASK) >>
	       XSYSMONPSV_SUPPLY_FMT_SHIFT;

	/* Calculate the exponent
	 * 2^(16-Exponent)
	 */
	Scale = (1 << (XSYSMONPSV_EXPONENT_RANGE_16 - Exponent));
	if(Format & (Mantissa  >> XSYSMONPSV_SUPPLY_MANTISSA_SIGN)) {
		return	((float)Mantissa/(float)Scale) - 1;
	}
	else {
		return (float)Mantissa/(float)Scale;
	}
}
/****************************************************************************/
/**
*
* This function converts the fixed point to degree celsius
*
* @param	Q8.7 representation of temperature value.
*
* @return	The Temperature in degree celsisus
*
* @note		None.
*
*****************************************************************************/
static inline float XSysMonPsv_FixedToFloat(u32 FixedQFmt)
{
	if(FixedQFmt >> XSYSMONPSV_QFMT_SIGN) {
		return (float)(~(FixedQFmt) + 1) /
			((float)(1 << XSYSMONPSV_QFMT_FRACTION)) * (-1.0);
	}
	else {
		return	(float)FixedQFmt /
			(float)(1 << XSYSMONPSV_QFMT_FRACTION);
	}
}

/************************** Function Prototypes ******************************/

/* Functions in xsysmonpsv.c */
s32 XSysMonPsv_CfgInitialize(XSysMonPsv *InstancePtr, XSysMonPsv_Config *CfgPtr);
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
u16 XSysMonPsv_ReadOTTempThreshold(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Threshold ThresholdType);
u32 XSysMonPsv_ReadDeviceTemp(XSysMonPsv *InstancePtr, XSysMonPsv_Val Value);
u32 XSysMonPsv_ReadSupplyThreshold(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Supply Supply,
				   XSysMonPsv_Threshold ThresholdType);
u32 XSysMonPsv_ReadSupplyValue(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply, XSysMonPsv_Val Value);
u32 XSysMonPsv_IsNewData(XSysMonPsv *InstancePtr, XSysMonPsv_Supply Supply);
u32 XSysMonPsv_IsAlarmCondition(XSysMonPsv *InstancePtr,
			       XSysMonPsv_Supply Supply);

/* Interrupt functions in xsysmonpsv_intr.c */
void XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
void XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
u32 XSysMonPsv_IntrGetEnabled(XSysMonPsv *InstancePtr, u8 IntrNum);
u32 XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr);
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask);
void XSysMonPsv_SetNewDataIntSrc(XSysMonPsv *InstancePtr,
				XSysMonPsv_Supply Supply, u32 Mask);

/* Functions in xsysmonpsv_selftest.c */
s32 XSysMonPsv_SelfTest(XSysMonPsv *InstancePtr);

/* Functions in xsysmonpsv_sinit.c */
XSysMonPsv_Config *XSysMonPsv_LookupConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* XSYSMONPSV_H_ */
