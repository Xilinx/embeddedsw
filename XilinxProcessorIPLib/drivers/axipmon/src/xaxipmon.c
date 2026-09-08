/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xaxipmon.c
* @addtogroup axipmon Overview
* @{
*
* @brief Driver API functions for the AXI Performance Monitor device.
*
* Provides functions to access and configure the AXI Performance Monitor.
*
* Refer to the xaxipmon.h header file for more information about this driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss   02/27/12  First release
* 2.00a bss   06/23/12  Updated to support v2_00a version of IP.
* 3.00a bss   09/03/12  Deleted XAxiPmon_SetAgent, XAxiPmon_GetAgent APIs and
*			modified XAxiPmon_SetMetrics, XAxiPmon_GetMetrics APIs
*			to support v2_01a version of IP.
* 3.01a bss   10/25/12  Added XAxiPmon_StartCounters and XAxiPmon_StopCounters
*			APIs (CR #683799).
*			Added XAxiPmon_StartEventLog and XAxiPmon_StopEventLog
*			APIs (CR #683801).
*			Added XAxiPmon_GetMetricName API (CR #683803).
*			Modified XAxiPmon_SetMetrics and XAxiPmon_GetMetrics
*			(CR #683746)
*			Added XAxiPmon_EnableEventLog,
*			XAxiPmon_DisableMetricsCounter,
*			XAxiPmon_EnableMetricsCounter APIs to replace macros.
*			Added XAxiPmon_SetMetricCounterCutOff,
*			XAxiPmon_GetMetricCounterCutOff,
*			XAxiPmon_EnableExternalTrigger and
*			XAxiPmon_DisableExternalTrigger APIs to support new
*			version of IP.
* 4.00a bss   01/17/13  To support new version of IP:
* 			Added XAxiPmon_SetLogEnableRanges,
*	  		XAxiPmon_GetLogEnableRanges,
*			XAxiPmon_EnableMetricCounterTrigger,
*			XAxiPmon_DisableMetricCounterTrigger,
*			XAxiPmon_EnableEventLogTrigger,
*			XAxiPmon_DisableEventLogTrigger,
*			XAxiPmon_SetWriteLatencyId,
*			XAxiPmon_SetReadLatencyId,
*			XAxiPmon_GetWriteLatencyId,
*			XAxiPmon_GetReadLatencyId APIs and removed
*			XAxiPmon_SetMetricCounterCutOff,
*			XAxiPmon_GetMetricCounterCutOff,
*			XAxiPmon_EnableExternalTrigger and
*			XAxiPmon_DisableExternalTrigger APIs
* 5.00a bss   08/26/13  To support new version of IP:
*			Modified XAxiPmon_CfgInitialize to add Mode of APM and
*			ScaleFactor parameter.
*			Modified Assert functions depending on Mode.
*			Modified XAxiPmon_GetMetricCounter and
*			XAxiPmon_GetSampledMetricCounter to include
*			new Counters.
*			Modified XAxiPmon_SetSampleInterval and
*			XAxiPmon_GetSampleInterval to remove higher 32 bit
*			value of SampleInterval since Sample Interval Register
*			is only 32 bit.
*			Added XAxiPmon_SetWrLatencyStart,
*			XAxiPmon_SetWrLatencyEnd, XAxiPmon_SetRdLatencyStart
*			XAxiPmon_SetRdLatencyEnd, XAxiPmon_GetWrLatencyStart,
*			XAxiPmon_GetWrLatencyEnd, XAxiPmon_GetRdLatencyStart,
*			XAxiPmon_GetRdLatencyEnd, XAxiPmon_SetWriteIdMask,
*			XAxiPmon_SetReadIdMask,
*			XAxiPmon_GetWriteIdMask and
*			XAxiPmon_GetReadIdMask APIs.
*			Renamed:
*			XAxiPmon_SetWriteLatencyId to XAxiPmon_SetWriteId
*			XAxiPmon_SetReadLatencyId to XAxiPmon_SetReadId
*			XAxiPmon_GetWriteLatencyId to XAxiPmon_GetWriteId
*			XAxiPmon_SetReadLatencyId to XAxiPmon_GetReadId.
* 6.2   bss  04/21/14   Updated XAxiPmon_CfgInitialize to Reset counters
*			and FIFOs based on Modes(CR#782671). And if both
*			profile and trace modes are present set mode as
*			Advanced.
* 6.2	bss  03/02/15	Updated XAxiPmon_SetWriteId, XAxiPmon_SetReadId,
*						XAxiPmon_GetWriteId, XAxiPmon_GetReadId
*						XAxiPmon_SetWriteIdMask, XAxiPmon_SetReadIdMask,
*						XAxiPmon_GetWriteIdMask, XAxiPmon_GetReadIdMask
*						functions to support Zynq MP APM.
* 6.3	kvn  07/02/15	Modified code according to MISRA-C:2012 guidelines.
* 6.4   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XAxiPmon_CfgInitialize API.
* 6.7   sd   17/11/18 Fix Doxygen reported warnings
* 6.10  ht   06/23/23 Added support for system device-tree flow.
* 6.12  ht   12/01/24 Fix GCC warnings
* 6.15  vn   07/30/26 Fix Doxygen warnings
* 6.15  vn   09/08/26 Align Doxygen comments with AMD standards
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xaxipmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* @brief Initializes a specific XAxiPmon device/instance.
*
* Call this function prior to using the AXI Performance Monitor device.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   ConfigPtr points to the XAxiPmon device configuration structure.
* @param[in]   EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		First call the XAxiPmon_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XAxiPmon_CfgInitialize() API.
*
******************************************************************************/
s32 XAxiPmon_CfgInitialize(XAxiPmon *InstancePtr, XAxiPmon_Config *ConfigPtr,
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
#ifndef SDT
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.GlobalClkCounterWidth =
		ConfigPtr->GlobalClkCounterWidth;
	InstancePtr->Config.MetricSampleCounterWidth =
		ConfigPtr->MetricSampleCounterWidth;
	InstancePtr->Config.IsEventCount =
		ConfigPtr->IsEventCount;
	InstancePtr->Config.NumberofSlots =
		ConfigPtr->NumberofSlots;
	InstancePtr->Config.NumberofCounters =
		ConfigPtr->NumberofCounters;
	InstancePtr->Config.HaveSampledCounters =
		ConfigPtr->HaveSampledCounters;
	InstancePtr->Config.IsEventLog =
		ConfigPtr->IsEventLog;
	InstancePtr->Config.FifoDepth =
		ConfigPtr->FifoDepth;
	InstancePtr->Config.FifoWidth =
		ConfigPtr->FifoWidth;
	InstancePtr->Config.TidWidth =
		ConfigPtr->TidWidth;
	InstancePtr->Config.Is32BitFiltering = ConfigPtr->Is32BitFiltering;

	InstancePtr->Config.ScaleFactor = ConfigPtr->ScaleFactor;
#ifdef SDT
	InstancePtr->Config.IntId = ConfigPtr->IntId;
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif

	if ((ConfigPtr->ModeProfile == ConfigPtr->ModeTrace)
	    || (ConfigPtr->ModeAdvanced == 1U)) {
		InstancePtr->Mode = XAPM_MODE_ADVANCED;
	} else if (ConfigPtr->ModeTrace == 1U) {
		InstancePtr->Mode = XAPM_MODE_TRACE;
	} else {
		InstancePtr->Mode = XAPM_MODE_PROFILE;
	}

	/*
	 * Indicate the instance is now ready to use, initialized without error.
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/*
	 * Reset the Counters and FIFO based on Modes.
	 */

	/* Advanced and Profile */
	if ((InstancePtr->Mode == XAPM_MODE_ADVANCED) ||
	    (InstancePtr->Mode == XAPM_MODE_PROFILE)) {
		(void)XAxiPmon_ResetMetricCounter(InstancePtr);
	}
	/* Advanced */
	if (InstancePtr->Mode == XAPM_MODE_ADVANCED) {
		XAxiPmon_ResetGlobalClkCounter(InstancePtr);
	}
	/* Advanced and Trace */
	if ((InstancePtr->Mode == XAPM_MODE_ADVANCED) ||
	    (InstancePtr->Mode == XAPM_MODE_TRACE)) {
		(void)XAxiPmon_ResetFifo(InstancePtr);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Resets all Metric Counters and Sampled Metric Counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	XST_SUCCESS
*
******************************************************************************/
s32 XAxiPmon_ResetMetricCounter(XAxiPmon *InstancePtr)
{

	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode != XAPM_MODE_TRACE);

	/*
	 * Write the reset value to the Control register to reset
	 * Metric counters
	 */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue | XAPM_CR_MCNTR_RESET_MASK));
	/*
	 * Release from Reset
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue & ~(XAPM_CR_MCNTR_RESET_MASK)));
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* @brief Resets the Global Clock Counter of the AXI Performance Monitor.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
******************************************************************************/
void XAxiPmon_ResetGlobalClkCounter(XAxiPmon *InstancePtr)
{

	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);

	/*
	 * Write the reset value to the Control register to reset
	 * Global Clock Counter
	 */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue | XAPM_CR_GCC_RESET_MASK));

	/*
	 * Release from Reset
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue & ~(XAPM_CR_GCC_RESET_MASK)));

}

/*****************************************************************************/
/**
*
* @brief Resets the Streaming FIFO of the AXI Performance Monitor.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	XST_SUCCESS
*
******************************************************************************/
s32 XAxiPmon_ResetFifo(XAxiPmon *InstancePtr)
{

	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode != XAPM_MODE_PROFILE);

	/* Check Event Logging is enabled in Hardware */
	if ((InstancePtr->Config.IsEventLog == 0U) &&
	    (InstancePtr->Mode == XAPM_MODE_ADVANCED)) {
		/*Event logging not enabled in Hardware*/
		return XST_SUCCESS;
	}
	/*
	 * Write the reset value to the Control register to reset
	 * FIFO
	 */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue | XAPM_CR_FIFO_RESET_MASK));
	/*
	 * Release from Reset
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  (RegValue & ~(XAPM_CR_FIFO_RESET_MASK)));

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* @brief Sets ranges for the specified Incrementer.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   IncrementerNum specifies the Incrementer for which Ranges
*		need to be set
* @param[in]   RangeUpper specifies the Upper limit in 32 bit Register
* @param[in]   RangeLower specifies the Lower limit in 32 bit Register
*
*****************************************************************************/
void XAxiPmon_SetIncrementerRange(XAxiPmon *InstancePtr, u8 IncrementerNum,
				  u16 RangeUpper,	u16 RangeLower)
{

	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);
	Xil_AssertVoid(IncrementerNum < XAPM_MAX_COUNTERS);

	/*
	 * Write to the specified Range register
	 */
	RegValue = (u32)RangeUpper << 16;
	RegValue |= RangeLower;
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
			  ((u32)XAPM_RANGE0_OFFSET + ((u32)IncrementerNum * (u32)16)),
			  RegValue);
}

/****************************************************************************/
/**
*
* @brief Returns the ranges of the specified Incrementer Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   IncrementerNum specifies the Incrementer for which Ranges
*		need to be returned.
* @param[out]  RangeUpper specifies the reference variable which returns
*		the Upper Range Value of the specified Incrementer.
* @param[out]  RangeLower specifies the reference variable which returns
*		the Lower Range Value of the specified Incrementer.
*
*****************************************************************************/
void XAxiPmon_GetIncrementerRange(XAxiPmon *InstancePtr, u8 IncrementerNum,
				  u16 *RangeUpper, u16 *RangeLower)
{

	u32 RegValue;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);
	Xil_AssertVoid(IncrementerNum < XAPM_MAX_COUNTERS);

	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    ((u32)XAPM_RANGE0_OFFSET + ((u32)IncrementerNum * (u32)16)));

	*RangeLower = (u16)(RegValue & 0x0000FFFFU);
	*RangeUpper = (u16)((RegValue >> 16) & 0x0000FFFFU);
}

/****************************************************************************/
/**
*
* @brief Sets the Sample Interval Register value.
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   SampleInterval is the Sample Interval value to be set
*
*****************************************************************************/
void XAxiPmon_SetSampleInterval(XAxiPmon *InstancePtr, u32 SampleInterval)
{

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_TRACE);

	/*
	 * Set Sample Interval Lower
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
			  XAPM_SI_LOW_OFFSET, SampleInterval);

}

/****************************************************************************/
/**
*
* @brief Returns the contents of the Sample Interval Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[out]  SampleInterval is a pointer where the Sample Interval
*		Counter value is returned.
*
******************************************************************************/
void XAxiPmon_GetSampleInterval(XAxiPmon *InstancePtr, u32 *SampleInterval)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_TRACE);

	/*
	 * Set Sample Interval Lower
	 */
	*SampleInterval = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					   XAPM_SI_LOW_OFFSET);

}

/****************************************************************************/
/**
*
* @brief Sets metrics for the specified Counter in the Metric Selector Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   Slot is the slot ID for which specified counter has to
*		be connected.
* @param[in]   Metrics is one of the Metric Sets. User has to use
*		XAPM_METRIC_SET_* macros in xaxipmon.h for this parameter
* @param[in]   CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return	XST_SUCCESS if Success
*		XST_FAILURE if Failure
*
*****************************************************************************/
s32 XAxiPmon_SetMetrics(XAxiPmon *InstancePtr, u8 Slot, u8 Metrics,
			u8 CounterNum)
{
	u32 RegValue;
	u32 Mask;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);
	Xil_AssertNonvoid(Slot < XAPM_MAX_AGENTS);
	Xil_AssertNonvoid((Metrics <= XAPM_METRIC_SET_22) ||
			  (Metrics == XAPM_METRIC_SET_30));
	Xil_AssertNonvoid(CounterNum < XAPM_MAX_COUNTERS);

	/* Find Mask value to force zero in counternum byte range */
	if ((CounterNum == 0U) || (CounterNum == 4U) || (CounterNum == 8U)) {
		Mask = 0xFFFFFF00U;
	} else if ((CounterNum == 1U) || (CounterNum == 5U) || (CounterNum == 9U)) {
		Mask = 0xFFFF00FFU;
	} else if ((CounterNum == 2U) || (CounterNum == 6U)) {
		Mask = 0xFF00FFFFU;
	} else {
		Mask = 0x00FFFFFFU;
	}

	if (CounterNum <= 3U) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    XAPM_MSR0_OFFSET);

		RegValue = RegValue & Mask;
		RegValue = RegValue | ((u32)Metrics << (CounterNum * (u8)8));
		RegValue = RegValue | ((u32)Slot << ((CounterNum * (u8)8) + (u8)5));
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  (u32)XAPM_MSR0_OFFSET, RegValue);
	} else if ((CounterNum >= 4U) && (CounterNum <= 7U)) {
		CounterNum = CounterNum - 4U;
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    (u32)XAPM_MSR1_OFFSET);

		RegValue = RegValue & Mask;
		RegValue = RegValue | ((u32)Metrics << (CounterNum * (u8)8));
		RegValue = RegValue | ((u32)Slot << ((CounterNum * (u8)8) + (u8)5));
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_MSR1_OFFSET, RegValue);
	} else {
		CounterNum = CounterNum - 8U;
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    XAPM_MSR2_OFFSET);

		RegValue = RegValue & Mask;
		RegValue = RegValue | ((u32)Metrics << (CounterNum * (u8)8));
		RegValue = RegValue | ((u32)Slot << ((CounterNum * (u8)8) + (u8)5));
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_MSR2_OFFSET, RegValue);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Returns metrics from the specified Counter in the
*        Metric Selector Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   CounterNum is the Counter Number.
*		The valid values are 0 to 9.
* @param[out]  Metrics is a reference parameter from application where metrics
*		of specified counter is filled.
* @param[out]  Slot is a reference parameter in which slot Id of
*		specified counter is filled
* @return	XST_SUCCESS if Success
*		XST_FAILURE if Failure
*
*****************************************************************************/
s32 XAxiPmon_GetMetrics(XAxiPmon *InstancePtr, u8 CounterNum, u8 *Metrics,
			u8 *Slot)
{
	u32 RegValue;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);
	Xil_AssertNonvoid(CounterNum <= XAPM_MAX_COUNTERS);

	if (CounterNum <= 3U) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    XAPM_MSR0_OFFSET);
		*Metrics = (u8)(RegValue >> (CounterNum * (u8)8)) & 0x1FU;
		*Slot 	= (u8)(RegValue >> ((CounterNum * (u8)8) + (u8)5)) & 0x07U;

	} else if ((CounterNum >= 4U) && (CounterNum <= 7U)) {
		CounterNum = CounterNum - 4U;
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    XAPM_MSR1_OFFSET);
		*Metrics = (u8)(RegValue >> (CounterNum * (u8)8)) & 0x1FU;
		*Slot 	= (u8)(RegValue >> ((CounterNum * (u8)8) + (u8)5)) & 0x07U;
	} else {
		CounterNum = CounterNum - 8U;
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    XAPM_MSR2_OFFSET);
		*Metrics = (u8)(RegValue >> (CounterNum * (u8)8)) & 0x1FU;
		*Slot 	= (u8)(RegValue >> ((CounterNum * (u8)8) + (u8)5)) & 0x07U;
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the Global Clock Counter Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[out]  CntHighValue is the pointer with which upper 32 bits
*		of Global Clock Counter has to be filled
* @param[out]  CntLowValue is the pointer with which lower 32 bits
*		of Global Clock Counter has to be filled
*
*****************************************************************************/
void XAxiPmon_GetGlobalClkCounter(XAxiPmon *InstancePtr, u32 *CntHighValue,
				  u32 *CntLowValue)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode == XAPM_MODE_ADVANCED);

	*CntHighValue = 0x0U;
	*CntLowValue  = 0x0U;

	/*
	 * If Counter width is 64 bit then Counter Value has to be
	 * filled at CntHighValue address also.
	 */
	if (InstancePtr->Config.GlobalClkCounterWidth == 64) {

		/* Bits[63:32] exists at XAPM_GCC_HIGH_OFFSET */
		*CntHighValue = XAxiPmon_ReadReg(InstancePtr->
						 Config.BaseAddress, XAPM_GCC_HIGH_OFFSET);
	}
	/* Bits[31:0] exists at XAPM_GCC_LOW_OFFSET */
	*CntLowValue = XAxiPmon_ReadReg(InstancePtr->
					Config.BaseAddress, XAPM_GCC_LOW_OFFSET);
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the specified Metric Counter Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   CounterNum is the number of the Metric Counter to be read.
*		Use the XAPM_METRIC_COUNTER* defines for the counter number in
*		xaxipmon.h. The valid values are 0 (XAPM_METRIC_COUNTER_0) to
*		47(XAPM_METRIC_COUNTER_47).
* @return	RegValue is the content of specified Metric Counter.
*
*****************************************************************************/
u32 XAxiPmon_GetMetricCounter(XAxiPmon *InstancePtr, u32 CounterNum)
{

	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode != XAPM_MODE_TRACE);
	Xil_AssertNonvoid(CounterNum < XAPM_MAX_COUNTERS_PROFILE);

	if (CounterNum < 10U ) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_MC0_OFFSET + (CounterNum * (u32)16)));
	} else if ((CounterNum >= 10U) && (CounterNum < 12U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_MC10_OFFSET + ((CounterNum - (u32)10) * (u32)16)));
	} else if ((CounterNum >= 12U) && (CounterNum < 24U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_MC12_OFFSET + ((CounterNum - (u32)12) * (u32)16)));
	} else if ((CounterNum >= 24U) && (CounterNum < 36U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_MC24_OFFSET + ((CounterNum - (u32)24) * (u32)16)));
	} else {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_MC36_OFFSET + ((CounterNum - (u32)36) * (u32)16)));
	}

	return RegValue;
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the specified Sampled Metric Counter Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   CounterNum is the number of the Sampled Metric Counter to read.
*		Use the XAPM_METRIC_COUNTER* defines for the counter number in
*		xaxipmon.h. The valid values are 0 (XAPM_METRIC_COUNTER_0) to
*		47(XAPM_METRIC_COUNTER_47).
*
* @return	RegValue is the content of specified Sampled Metric Counter.
*
*****************************************************************************/
u32 XAxiPmon_GetSampledMetricCounter(XAxiPmon *InstancePtr, u32 CounterNum)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Mode != XAPM_MODE_TRACE);
	Xil_AssertNonvoid(CounterNum < XAPM_MAX_COUNTERS_PROFILE);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_PROFILE) ||
			  ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			   (InstancePtr->Config.HaveSampledCounters == 1U)));

	if (CounterNum < 10U ) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_SMC0_OFFSET + (CounterNum * (u32)16)));
	} else if ((CounterNum >= 10U) && (CounterNum < 12U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_SMC10_OFFSET + ((CounterNum - (u32)10) * (u32)16)));
	} else if ((CounterNum >= 12U) && (CounterNum < 24U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_SMC12_OFFSET + ((CounterNum - (u32)12) * (u32)16)));
	} else if ((CounterNum >= 24U) && (CounterNum < 36U)) {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_SMC24_OFFSET + ((CounterNum - (u32)24) * (u32)16)));
	} else {
		RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					    ((u32)XAPM_SMC36_OFFSET + ((CounterNum - (u32)36) * (u32)16)));
	}

	return RegValue;
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the specified Incrementer Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   IncrementerNum is the number of the Incrementer register to
*		read.Use the XAPM_INCREMENTER_* defines for the Incrementer
*		number.The valid values are 0 (XAPM_INCREMENTER_0) to
*		9 (XAPM_INCREMENTER_9).
* @return	RegValue is content of specified Metric Incrementer register.
*
*****************************************************************************/
u32 XAxiPmon_GetIncrementer(XAxiPmon *InstancePtr, u32 IncrementerNum)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			  (InstancePtr->Config.IsEventCount == 1U));
	Xil_AssertNonvoid(IncrementerNum < XAPM_MAX_COUNTERS);

	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    ((u32)XAPM_INC0_OFFSET + (IncrementerNum * (u32)16)));

	return RegValue;
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the specified Sampled Incrementer Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   IncrementerNum is the number of the Sampled Incrementer
*		register to read.Use the XAPM_INCREMENTER_* defines for the
*		Incrementer number.The valid values are 0 (XAPM_INCREMENTER_0)
*		to 9 (XAPM_INCREMENTER_9).
* @return	RegValue is content of specified Sampled Incrementer register.
*
*****************************************************************************/
u32 XAxiPmon_GetSampledIncrementer(XAxiPmon *InstancePtr, u32 IncrementerNum)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			  (InstancePtr->Config.IsEventCount == 1U) &&
			  (InstancePtr->Config.HaveSampledCounters == 1U));
	Xil_AssertNonvoid(IncrementerNum < XAPM_MAX_COUNTERS);

	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    ((u32)XAPM_SINC0_OFFSET + (IncrementerNum * (u32)16)));
	return RegValue;
}

/****************************************************************************/
/**
*
* @brief Sets the Software-written Data Register value.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   SwData is the Software written Data.
*
*****************************************************************************/
void XAxiPmon_SetSwDataReg(XAxiPmon *InstancePtr, u32 SwData)
{
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set Software-written Data Register
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_SWD_OFFSET,
			  SwData);
}

/****************************************************************************/
/**
*
* @brief Returns the contents of the Software-written Data Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	SwData.
*
*****************************************************************************/
u32 XAxiPmon_GetSwDataReg(XAxiPmon *InstancePtr)
{
	u32 SwData;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set Metric Selector Register
	 */
	SwData = (u32)XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				       XAPM_SWD_OFFSET);
	return SwData;
}

/*****************************************************************************/
/**
*
* @brief Starts event logging in the AXI Performance Monitor.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   FlagEnables is a value to write to the flag enables
*               register defined by XAPM_FEC_OFFSET. It is recommended
*               to use the XAPM_FEC_*_MASK mask bits to generate.
*               A value of 0x0 will disable all events to the event
*               log streaming FIFO.
*
* @return       XST_SUCCESS
*
******************************************************************************/
s32 XAxiPmon_StartEventLog(XAxiPmon *InstancePtr, u32 FlagEnables)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_TRACE) ||
			  ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			   (InstancePtr->Config.IsEventLog == 1U)));

	/* Read current register value */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    (u32)XAPM_CTL_OFFSET);
	/* Flag Enable register is present only in Advanced Mode */
	if (InstancePtr->Mode == XAPM_MODE_ADVANCED) {
		/* Now write to flag enables register */
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  (u32)XAPM_FEC_OFFSET, FlagEnables);
	}

	/* Write the new value to the Control register to
	 *	enable event logging */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_CTL_OFFSET,
			  RegValue | XAPM_CR_EVENTLOG_ENABLE_MASK);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Stops event logging in the AXI Performance Monitor.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return       XST_SUCCESS
*
******************************************************************************/
s32 XAxiPmon_StopEventLog(XAxiPmon *InstancePtr)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_TRACE) ||
			  ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			   (InstancePtr->Config.IsEventLog == 1U)));

	/* Read current register value */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    (u32)XAPM_CTL_OFFSET);

	/* Write the new value to the Control register to disable
	 * event logging */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_CTL_OFFSET,
			  RegValue & ~XAPM_CR_EVENTLOG_ENABLE_MASK);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Starts all counters in the AXI Performance Monitor.
*
* Enables the global clock counter, all metric counters, and all sampled
* metric counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   SampleInterval is the sample interval for the sampled metric
*              counters
*
* @return   XST_SUCCESS
******************************************************************************/
s32 XAxiPmon_StartCounters(XAxiPmon *InstancePtr, u32 SampleInterval)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_PROFILE) ||
			  ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			   (InstancePtr->Config.IsEventCount == 1U)));

	/* Read current register value */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    (u32)XAPM_CTL_OFFSET);
	/* Globlal Clock Counter is present in Advanced mode only */
	if (InstancePtr->Mode == XAPM_MODE_ADVANCED) {
		RegValue = RegValue | XAPM_CR_GCC_ENABLE_MASK;
	}

	/*
	 * Write the new value to the Control register to enable
	 * global clock counter and metric counters
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_CTL_OFFSET,
			  RegValue | XAPM_CR_MCNTR_ENABLE_MASK);

	/* Set, enable, and load sampled counters */
	XAxiPmon_SetSampleInterval(InstancePtr, SampleInterval);
	XAxiPmon_LoadSampleIntervalCounter(InstancePtr);
	XAxiPmon_EnableSampleIntervalCounter(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Stops all counters in the AXI Performance Monitor.
*
* Disables the global clock counter and all metric counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return       XST_SUCCESS
*
******************************************************************************/
s32 XAxiPmon_StopCounters(XAxiPmon *InstancePtr)
{
	u32 RegValue;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((InstancePtr->Mode == XAPM_MODE_PROFILE) ||
			  ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			   (InstancePtr->Config.IsEventCount == 1U)));

	/* Read current register value */
	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    (u32)XAPM_CTL_OFFSET);
	/* Globlal Clock Counter is present in Advanced mode only */
	if (InstancePtr->Mode == XAPM_MODE_ADVANCED) {
		RegValue = RegValue & ~XAPM_CR_GCC_ENABLE_MASK;
	}

	/*
	 * Write the new value to the Control register to disable
	 * global clock counter and metric counters
	 */
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_CTL_OFFSET,
			  RegValue & ~XAPM_CR_MCNTR_ENABLE_MASK);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Enables Metric Counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*******************************************************************************/
void XAxiPmon_EnableMetricsCounter(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((InstancePtr->Mode == XAPM_MODE_PROFILE) ||
		       ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			(InstancePtr->Config.IsEventCount == 1U)));

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal | XAPM_CR_MCNTR_ENABLE_MASK);
}
/****************************************************************************/
/**
*
* @brief Disables the Metric Counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*****************************************************************************/
void XAxiPmon_DisableMetricsCounter(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((InstancePtr->Mode == XAPM_MODE_PROFILE) ||
		       ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			(InstancePtr->Config.IsEventCount == 1U)));

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  (u32)XAPM_CTL_OFFSET);

	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, (u32)XAPM_CTL_OFFSET,
			  RegVal & ~(XAPM_CR_MCNTR_ENABLE_MASK));
}

/****************************************************************************/
/**
*
* @brief Sets the ranges for the specified Metric Counter Log Enable Register.
*
* Event Logging starts when the corresponding Metric Counter value falls
* in between these ranges.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   CounterNum is the Metric Counter number for which
*		Ranges are to be assigned.Use the XAPM_METRIC_COUNTER*
*		defines for the counter number in xaxipmon.h.
*		The valid values are 0 (XAPM_METRIC_COUNTER_0) to
*		9 (XAPM_METRIC_COUNTER_9).
* @param[in]   RangeUpper specifies the Upper limit in 32 bit Register
* @param[in]   RangeLower specifies the Lower limit in 32 bit Register
*
*****************************************************************************/
void XAxiPmon_SetLogEnableRanges(XAxiPmon *InstancePtr, u32 CounterNum,
				 u16 RangeUpper, u16 RangeLower)
{
	u32 RegValue;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CounterNum < XAPM_MAX_COUNTERS);
	Xil_AssertVoid((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
		       (InstancePtr->Config.IsEventCount == 1U));


	/*
	 * Write the specified Ranges to corresponding Metric Counter Log
	 * Enable Register
	 */
	RegValue = (u32)RangeUpper << 16;
	RegValue |= RangeLower;
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
			  ((u32)XAPM_MC0LOGEN_OFFSET + (CounterNum * (u32)16)), RegValue);

}

/****************************************************************************/
/**
*
* @brief Returns the ranges of the specified Metric Counter Log Enable Register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   CounterNum is the Metric Counter number for which
*		Ranges are to be returned.Use the XAPM_METRIC_COUNTER*
*		defines for the counter number in xaxipmon.h.
*		The valid values are 0 (XAPM_METRIC_COUNTER_0) to
*		9 (XAPM_METRIC_COUNTER_9).
*
* @param[in]   RangeUpper specifies the user reference variable which returns
*		the Upper Range Value of the specified Metric Counter
*		Log Enable Register.
* @param[in]   RangeLower specifies the user reference variable which returns
*		the Lower Range Value of the specified Metric Counter
*		Log Enable Register.
*
*****************************************************************************/
void XAxiPmon_GetLogEnableRanges(XAxiPmon *InstancePtr, u32 CounterNum,
				 u16 *RangeUpper, u16 *RangeLower)
{
	u32 RegValue;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(CounterNum < XAPM_MAX_COUNTERS);
	Xil_AssertVoid((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
		       (InstancePtr->Config.IsEventCount == 1U));


	RegValue = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				    ((u32)XAPM_MC0LOGEN_OFFSET + (CounterNum * (u32)16)));

	*RangeLower = (u16)RegValue & 0xFFFFU;
	*RangeUpper = (u16)(RegValue >> 16) & 0xFFFFU;
}

/*****************************************************************************/
/**
*
* @brief Enables Event Logging.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*******************************************************************************/
void XAxiPmon_EnableEventLog(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((InstancePtr->Mode == XAPM_MODE_TRACE) ||
		       ((InstancePtr->Mode == XAPM_MODE_ADVANCED) &&
			(InstancePtr->Config.IsEventLog == 1U)));

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal | XAPM_CR_EVENTLOG_ENABLE_MASK);
}

/*****************************************************************************/
/**
*
* @brief Enables external trigger for Metric Counters.
*
* Enables external trigger pulse so that Metric Counters can be started
* on external trigger pulse for a Slot.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*******************************************************************************/
void XAxiPmon_EnableMetricCounterTrigger(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_TRACE);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal | XAPM_CR_MCNTR_EXTTRIGGER_MASK);
}

/****************************************************************************/
/**
*
* @brief Disables external trigger for Metric Counters.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*****************************************************************************/
void XAxiPmon_DisableMetricCounterTrigger(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_TRACE);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);

	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal & ~(XAPM_CR_MCNTR_EXTTRIGGER_MASK));
}

/*****************************************************************************/
/**
*
* @brief Enables external trigger for Event Log.
*
* Enables external trigger pulse so that Event Logging can be started
* on external trigger pulse for a Slot.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*******************************************************************************/
void XAxiPmon_EnableEventLogTrigger(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_PROFILE);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal | XAPM_CR_EVTLOG_EXTTRIGGER_MASK);
}

/****************************************************************************/
/**
*
* @brief Disables external trigger for Event Log.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
*****************************************************************************/
void XAxiPmon_DisableEventLogTrigger(XAxiPmon *InstancePtr)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Mode != XAPM_MODE_PROFILE);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);

	XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress, XAPM_CTL_OFFSET,
			  RegVal & ~(XAPM_CR_EVTLOG_EXTTRIGGER_MASK));
}

/****************************************************************************/
/**
*
* @brief Returns the name string for a given Metric.
*
* @param[in]   Metrics is one of the Metric Sets. User has to use
*              XAPM_METRIC_SET_* macros in xaxipmon.h for this parameter
*
* @return       const char *
*
*****************************************************************************/
const char *XAxiPmon_GetMetricName(u8 Metrics)
{
	if (Metrics == XAPM_METRIC_SET_0 ) {
		return "Write Transaction Count";
	}
	if (Metrics == XAPM_METRIC_SET_1 ) {
		return "Read Transaction Count";
	}
	if (Metrics == XAPM_METRIC_SET_2 ) {
		return "Write Byte Count";
	}
	if (Metrics == XAPM_METRIC_SET_3 ) {
		return "Read Byte Count";
	}
	if (Metrics == XAPM_METRIC_SET_4 ) {
		return "Write Beat Count";
	}
	if (Metrics == XAPM_METRIC_SET_5 ) {
		return "Total Read Latency";
	}
	if (Metrics == XAPM_METRIC_SET_6 ) {
		return "Total Write Latency";
	}
	if (Metrics == XAPM_METRIC_SET_7 ) {
		return "Slv_Wr_Idle_Cnt";
	}
	if (Metrics == XAPM_METRIC_SET_8 ) {
		return "Mst_Rd_Idle_Cnt";
	}
	if (Metrics == XAPM_METRIC_SET_9 ) {
		return "Num_BValids";
	}
	if (Metrics == XAPM_METRIC_SET_10) {
		return "Num_WLasts";
	}
	if (Metrics == XAPM_METRIC_SET_11) {
		return "Num_RLasts";
	}
	if (Metrics == XAPM_METRIC_SET_12) {
		return "Minimum Write Latency";
	}
	if (Metrics == XAPM_METRIC_SET_13) {
		return "Maximum Write Latency";
	}
	if (Metrics == XAPM_METRIC_SET_14) {
		return "Minimum Read Latency";
	}
	if (Metrics == XAPM_METRIC_SET_15) {
		return "Maximum Read Latency";
	}
	if (Metrics == XAPM_METRIC_SET_16) {
		return "Transfer Cycle Count";
	}
	if (Metrics == XAPM_METRIC_SET_17) {
		return "Packet Count";
	}
	if (Metrics == XAPM_METRIC_SET_18) {
		return "Data Byte Count";
	}
	if (Metrics == XAPM_METRIC_SET_19) {
		return "Position Byte Count";
	}
	if (Metrics == XAPM_METRIC_SET_20) {
		return "Null Byte Count";
	}
	if (Metrics == XAPM_METRIC_SET_21) {
		return "Slv_Idle_Cnt";
	}
	if (Metrics == XAPM_METRIC_SET_22) {
		return "Mst_Idle_Cnt";
	}
	if (Metrics == XAPM_METRIC_SET_30) {
		return "External event count";
	}
	return "Unsupported";
}

/****************************************************************************/
/**
*
* @brief Sets the Write ID in the ID register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   WriteId is the Write ID to be written in ID register.
*
* @note
*			If ID filtering for write is of 32 bits(for Zynq MP APM) width then
*			WriteID is written to XAPM_ID_OFFSET or if it is 16 bit width
*			then lower 16 bits of WriteID are written to XAPM_ID_OFFSET.
*
*****************************************************************************/
void XAxiPmon_SetWriteId(XAxiPmon *InstancePtr, u32 WriteId)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_ID_OFFSET);
		RegVal = RegVal & ~(XAPM_ID_WID_MASK);
		RegVal = RegVal | WriteId;
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_ID_OFFSET, RegVal);
	} else {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_ID_OFFSET, WriteId);
	}
}

/****************************************************************************/
/**
*
* @brief Sets the Read ID in the ID register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   ReadId is the Read ID to be written in ID register.
*
* @note
*			If ID filtering for read is of 32 bits(for Zynq MP APM) width then
*			ReadId is written to XAPM_RID_OFFSET or if it is 16 bit width
*			then lower 16 bits of ReadId are written to XAPM_ID_OFFSET.
*
*****************************************************************************/
void XAxiPmon_SetReadId(XAxiPmon *InstancePtr, u32 ReadId)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_ID_OFFSET);
		RegVal = RegVal & ~(XAPM_ID_RID_MASK);
		RegVal = RegVal | (ReadId << 16);
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_ID_OFFSET, RegVal);
	} else {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_RID_OFFSET, ReadId);
	}
}

/****************************************************************************/
/**
*
* @brief Returns the Write ID from the ID register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	WriteId is the required Write ID in ID register.
*			If ID filtering for write is of 32 bits(for Zynq MP APM) width then
*			32 bit XAPM_ID_OFFSET contents are returned or if it is 16 bit
*			width then lower 16 bits of XAPM_ID_OFFSET register are returned.
*
*****************************************************************************/
u32 XAxiPmon_GetWriteId(XAxiPmon *InstancePtr)
{

	u32 WriteId;
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_ID_OFFSET);
		WriteId = RegVal & XAPM_ID_WID_MASK;
	} else {
		WriteId = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					   XAPM_ID_OFFSET);
	}

	return WriteId;
}

/****************************************************************************/
/**
*
* @brief Returns the Read ID from the ID register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	ReadId is the required Read ID in ID register.
*			If ID filtering for write is of 32 bits(for Zynq MP APM) width then
*			32 bit XAPM_RID_OFFSET contents are returned or if it is 16 bit
*			width then higher 16 bits of XAPM_ID_OFFSET register are returned.
*
*****************************************************************************/
u32 XAxiPmon_GetReadId(XAxiPmon *InstancePtr)
{

	u32 ReadId;
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_ID_OFFSET);
		RegVal = RegVal & XAPM_ID_RID_MASK;
		ReadId = RegVal >> 16;
	} else {
		ReadId = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_RID_OFFSET);
	}

	return ReadId;
}

/*****************************************************************************/
/**
*
* @brief Sets the write latency start point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   Param is XAPM_LATENCY_ADDR_ISSUE or XAPM_LATENCY_ADDR_ACCEPT
*		in xaxipmon.h.
*
* @note		Param can be 0 - XAPM_LATENCY_ADDR_ISSUE
*		or 1 - XAPM_LATENCY_ADDR_ACCEPT
*
*******************************************************************************/
void XAxiPmon_SetWrLatencyStart(XAxiPmon *InstancePtr, u8 Param)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	if (Param == XAPM_LATENCY_ADDR_ACCEPT) {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET, RegVal | XAPM_CR_WRLATENCY_START_MASK);
	} else {
		XAxiPmon_WriteReg((InstancePtr)->Config.BaseAddress,
				  XAPM_CTL_OFFSET,
				  XAxiPmon_ReadReg((InstancePtr)->Config.BaseAddress,
						   XAPM_CTL_OFFSET) & ~(XAPM_CR_WRLATENCY_START_MASK));
	}
}

/*****************************************************************************/
/**
*
* @brief Sets the write latency end point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   Param is XAPM_LATENCY_LASTWR or XAPM_LATENCY_FIRSTWR
* 		in xaxipmon.h.
*
* @note		Param can be 0 - XAPM_LATENCY_LASTWR
*		or 1 - XAPM_LATENCY_FIRSTWR
*
*******************************************************************************/
void XAxiPmon_SetWrLatencyEnd(XAxiPmon *InstancePtr, u8 Param)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	if (Param == XAPM_LATENCY_FIRSTWR) {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET, RegVal | XAPM_CR_WRLATENCY_END_MASK);
	} else {
		XAxiPmon_WriteReg((InstancePtr)->Config.BaseAddress,
				  XAPM_CTL_OFFSET,
				  XAxiPmon_ReadReg((InstancePtr)->Config.BaseAddress,
						   XAPM_CTL_OFFSET) & ~(XAPM_CR_WRLATENCY_END_MASK));
	}
}

/*****************************************************************************/
/**
*
* @brief Sets the read latency start point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   Param is XAPM_LATENCY_ADDR_ISSUE or XAPM_LATENCY_ADDR_ACCEPT
*		in xaxipmon.h.
*
* @note		Param can be 0 - XAPM_LATENCY_ADDR_ISSUE
*		or 1 - XAPM_LATENCY_ADDR_ACCEPT
*
*******************************************************************************/
void XAxiPmon_SetRdLatencyStart(XAxiPmon *InstancePtr, u8 Param)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	if (Param == XAPM_LATENCY_ADDR_ACCEPT) {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET, RegVal | XAPM_CR_RDLATENCY_START_MASK);
	} else {
		XAxiPmon_WriteReg((InstancePtr)->Config.BaseAddress,
				  XAPM_CTL_OFFSET,
				  XAxiPmon_ReadReg((InstancePtr)->Config.BaseAddress,
						   XAPM_CTL_OFFSET) & ~(XAPM_CR_RDLATENCY_START_MASK));
	}
}

/*****************************************************************************/
/**
*
* @brief Sets the read latency end point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   Param is XAPM_LATENCY_LASTRD or XAPM_LATENCY_FIRSTRD
* 		in xaxipmon.h.
*
* @note		Param can be 0 - XAPM_LATENCY_LASTRD
*		or 1 - XAPM_LATENCY_FIRSTRD
*
*******************************************************************************/
void XAxiPmon_SetRdLatencyEnd(XAxiPmon *InstancePtr, u8 Param)
{
	u32 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET);
	if (Param == XAPM_LATENCY_FIRSTRD) {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_CTL_OFFSET, RegVal | XAPM_CR_RDLATENCY_END_MASK);
	} else {
		XAxiPmon_WriteReg((InstancePtr)->Config.BaseAddress,
				  XAPM_CTL_OFFSET,
				  XAxiPmon_ReadReg((InstancePtr)->Config.BaseAddress,
						   XAPM_CTL_OFFSET) & ~(XAPM_CR_RDLATENCY_END_MASK));
	}
}

/*****************************************************************************/
/**
*
* @brief Returns the write latency start point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	Returns 0 - XAPM_LATENCY_ADDR_ISSUE or
*			1 - XAPM_LATENCY_ADDR_ACCEPT
*
*******************************************************************************/
u8 XAxiPmon_GetWrLatencyStart(XAxiPmon *InstancePtr)
{
	u8 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = (u8)XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				      (u32)XAPM_CTL_OFFSET);
	RegVal = RegVal & XAPM_CR_WRLATENCY_START_MASK;
	if (RegVal != XAPM_LATENCY_ADDR_ISSUE) {
		return (u8)XAPM_LATENCY_ADDR_ACCEPT;
	} else {
		return (u8)XAPM_LATENCY_ADDR_ISSUE;
	}
}

/*****************************************************************************/
/**
*
* @brief Returns the write latency end point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	Returns 0 - XAPM_LATENCY_LASTWR or
*			1 - XAPM_LATENCY_FIRSTWR.
*
*******************************************************************************/
u8 XAxiPmon_GetWrLatencyEnd(XAxiPmon *InstancePtr)
{
	u8 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = (u8)XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				      (u32)XAPM_CTL_OFFSET);
	RegVal = RegVal & XAPM_CR_WRLATENCY_END_MASK;
	if (RegVal != XAPM_LATENCY_LASTWR) {
		return (u8)XAPM_LATENCY_FIRSTWR;
	} else {
		return (u8)XAPM_LATENCY_LASTWR;
	}
}

/*****************************************************************************/
/**
*
* @brief Returns the read latency start point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	Returns 0 - XAPM_LATENCY_ADDR_ISSUE or
*			1 - XAPM_LATENCY_ADDR_ACCEPT
*
*******************************************************************************/
u8 XAxiPmon_GetRdLatencyStart(XAxiPmon *InstancePtr)
{
	u8 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = (u8)XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				      (u32)XAPM_CTL_OFFSET);
	RegVal = RegVal & XAPM_CR_RDLATENCY_START_MASK;

	if (RegVal != XAPM_LATENCY_ADDR_ISSUE) {
		return	(u8)XAPM_LATENCY_ADDR_ACCEPT;
	} else {
		return (u8)XAPM_LATENCY_ADDR_ISSUE;
	}
}

/*****************************************************************************/
/**
*
* @brief Returns the read latency end point.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	Returns 0 - XAPM_LATENCY_LASTRD or
*			1 - XAPM_LATENCY_FIRSTRD.
*
*******************************************************************************/
u8 XAxiPmon_GetRdLatencyEnd(XAxiPmon *InstancePtr)
{
	u8 RegVal;

	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = (u8)XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
				      (u32)XAPM_CTL_OFFSET);
	RegVal = RegVal & XAPM_CR_RDLATENCY_END_MASK;
	if (RegVal != XAPM_LATENCY_LASTRD) {
		return (u8)XAPM_LATENCY_FIRSTRD;
	} else {
		return (u8)XAPM_LATENCY_LASTRD;
	}

}

/****************************************************************************/
/**
*
* @brief Sets the Write ID Mask in the ID Mask register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   WrMask is the Write ID mask to be written in ID register.
*
* @note
*			If ID masking for write is of 32 bits(for Zynq MP APM) width then
*			WrMask is written to XAPM_IDMASK_OFFSET or if it is 16 bit width
*			then lower 16 bits of WrMask are written to XAPM_IDMASK_OFFSET.
*
*****************************************************************************/
void XAxiPmon_SetWriteIdMask(XAxiPmon *InstancePtr, u32 WrMask)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_IDMASK_OFFSET);
		RegVal = RegVal & ~(XAPM_MASKID_WID_MASK);
		RegVal = RegVal | WrMask;
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_IDMASK_OFFSET, RegVal);
	} else {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_IDMASK_OFFSET, WrMask);
	}
}

/****************************************************************************/
/**
*
* @brief Sets the Read ID Mask in the ID Mask register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
* @param[in]   RdMask is the Read ID mask to be written in ID Mask register.
*
* @note
*			If ID masking for read is of 32 bits(for Zynq MP APM) width then
*			RdMask is written to XAPM_RIDMASK_OFFSET or if it is 16 bit width
*			then lower 16 bits of RdMask are written to XAPM_IDMASK_OFFSET.
*
*****************************************************************************/
void XAxiPmon_SetReadIdMask(XAxiPmon *InstancePtr, u32 RdMask)
{
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_IDMASK_OFFSET);
		RegVal = RegVal & ~(XAPM_MASKID_RID_MASK);
		RegVal = RegVal | (RdMask << 16);
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_IDMASK_OFFSET, RegVal);
	} else {
		XAxiPmon_WriteReg(InstancePtr->Config.BaseAddress,
				  XAPM_RIDMASK_OFFSET, RdMask);
	}
}

/****************************************************************************/
/**
*
* @brief Returns the Write ID Mask from the ID Mask register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	WrMask is the required Write ID Mask in ID Mask register.
*
* @note
*			If ID masking for write is of 32 bits(for Zynq MP APM) width then
*			32 bit XAPM_IDMASK_OFFSET contents are returned or if it is 16 bit
*			width then lower 16 bits of XAPM_IDMASK_OFFSET register
*			are returned.
*
*****************************************************************************/
u32 XAxiPmon_GetWriteIdMask(XAxiPmon *InstancePtr)
{

	u32 WrMask;
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_IDMASK_OFFSET);
		WrMask = RegVal & XAPM_MASKID_WID_MASK;
	} else {
		WrMask = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_IDMASK_OFFSET);
	}

	return WrMask;
}

/****************************************************************************/
/**
*
* @brief Returns the Read ID Mask from the ID Mask register.
*
* @param[in]   InstancePtr is a pointer to the XAxiPmon instance.
*
* @return	RdMask is the required Read ID Mask in ID Mask register.
*
* @note
*			If ID masking for read is of 32 bits(for Zynq MP APM) width then
*			32 bit XAPM_RIDMASK_OFFSET contents are returned or if it is 16 bit
*			width then higher 16 bits of XAPM_IDMASK_OFFSET register
*			are returned.
*
*****************************************************************************/
u32 XAxiPmon_GetReadIdMask(XAxiPmon *InstancePtr)
{

	u32 RdMask;
	u32 RegVal;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->Config.Is32BitFiltering == 0U) {
		RegVal = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_IDMASK_OFFSET);
		RegVal = RegVal & XAPM_MASKID_RID_MASK;
		RdMask = RegVal >> 16;
	} else {
		RdMask = XAxiPmon_ReadReg(InstancePtr->Config.BaseAddress,
					  XAPM_RIDMASK_OFFSET);
	}

	return RdMask;
}
/** @} */
