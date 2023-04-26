/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xpmonpsv.c
* @addtogroup pmonpsv Overview
* @{
*
* This file contains the driver API functions that can be used to access
* the Performance Monitor device.
*
* Refer to the xpmonpsv.h header file for more information about this driver.
*
* @note 	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd   01/20/19  First release
*     sd   03/05/19  Fix the counter check
* 2.0 sd   04/22/20  Rename the APIs
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xpmonpsv.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function initializes a specific XPmonPsv device/instance. This function
* must be called prior to using the Performance Monitor device.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	ConfigPtr points to the XPmonPsv device configuration structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_IS_STARTED if the device is already started.
*
* @note		The user needs to first call the XPmonPsv_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XPmonPsv_CfgInitialize() API.
*
******************************************************************************/
u32 XPmonPsv_CfgInitialize(XPmonPsv *InstancePtr, const XPmonPsv_Config *ConfigPtr,
						UINTPTR EffectiveAddr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if (InstancePtr->IsReady == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	/* Set the values read from the device config and the base address. */
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Indicate the instance is now ready to use, initialized without error */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->RequestedCounters[0] = 0xFF;
	InstancePtr->RequestedCounters[1] = 0XFF;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function resets the specified counter
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	Domain is one of the counter like lpd_main or r5_domain
* 		eg XPMONPSV_R5_DOMAIN, XPMONPSV_LPD_MAIN_DOMAIN
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return   XST_SUCCESS on success
*	    XST_FAILURE on failure
*
*
* @note		None.
*
******************************************************************************/
u32 XPmonPsv_ResetCounter(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	/*
	 *  Reset the counters
	 *   All statistics counters are cleared when the StatEn
	 *   bit goes from 0 to 1
	 */
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL, 0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL, 0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL, 0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL, 0x0U);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL,
		MAINCTRL_STATEN_MASK);

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function sets Metrics for specified Counter in the corresponding
* Metric Selector Register.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	StatPeriod is the period for which specified counter has to
*		be connected.
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return   XST_SUCCESS on success
*	    XST_FAILURE on failure
*
* @note		None.
*
*****************************************************************************/
u32 XPmonPsv_SetMetrics(const XPmonPsv *InstancePtr, u32 StatPeriod, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_STATPERIOD, StatPeriod);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_STATPERIOD, StatPeriod);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_STATPERIOD, StatPeriod);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_STATPERIOD, StatPeriod);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns Metrics in the specified Counter from the corresponding
* Metric Selector Register.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
* @param	MainCtl is a reference parameter from application where mainctrl
*		of specified counter is filled.
* @param	StatPeriod is the reference parameter from application where
* 		the StatPeriod is filled.
* @param	Domain is one of the counter like lpd_main or r5_domain
* @return	XST_SUCCESS if Success
*
*****************************************************************************/
u32 XPmonPsv_GetMetrics(const XPmonPsv *InstancePtr, u32 CounterNum, u8 *MainCtl,
						u8 *StatPeriod ,u32 Domain)
{
	u32 RegValue;
	u32 Offset=0;
	/*
	 * Assert the arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(MainCtl != NULL);
	Xil_AssertNonvoid(StatPeriod != NULL);

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] <= CounterNum) {
		return XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	RegValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_PROBE2_MAINCTRL);
	*MainCtl = (u8)(RegValue & 0xffU);
	RegValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_PROBE2_STATPERIOD);
	*StatPeriod = (u8)(RegValue & STATPERIOD_PERIOD_MASK);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns the contents of the Write response and request
* Counter Register.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	WriteRequestValue is the user space pointer with
* *		which the value of Write Request Count has to be filled
* @param	WriteRespValue is the user space pointer with
* *		which the value of Write Response Count has to be filled
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return   XST_SUCCESS on success
*	    XST_FAILURE on failure
*
* @note		None.
*
*****************************************************************************/
u32 XPmonPsv_GetWriteCounter(const XPmonPsv *InstancePtr,u32 *WriteRequestValue,
				u32 *WriteRespValue, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(WriteRespValue != NULL);
	Xil_AssertNonvoid(WriteRequestValue != NULL);

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] <= CounterNum) {
		return XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	*WriteRequestValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_WR_REQ_COUNTER0_VAL);
	*WriteRespValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_WR_RESP_COUNTER0_VAL);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns success if a free counter was found and request
* was granted.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* 		which the value of Write Request Count has to be filled
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum  pointer to get Counter Number.
*
*
* @return	Granted counter number.
* 		XST_FAILURE otherwise.
*
*****************************************************************************/
u32 XPmonPsv_RequestCounter(XPmonPsv *InstancePtr,u32 Domain, u32 *CounterNum)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CounterNum != NULL);

	/* Unrequested give the 0th counter */
	if ( InstancePtr->RequestedCounters[Domain] == 0xFF )
		InstancePtr->RequestedCounters[Domain] = 0U;

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return XST_FAILURE;
	}

	*CounterNum = InstancePtr->RequestedCounters[Domain];
	InstancePtr->RequestedCounters[Domain] += 1U;
	return XST_SUCCESS;

}
/****************************************************************************/
/**
*
* This function returns the contents of the Read response and request
* Counter Register.
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	ReadRequestValue is the user space pointer with
* 		which the value of Write Request Count has to be filled
* @param	ReadRespValue is the user space pointer with
* 		which the value of Write Response Count has to be filled
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return   XST_SUCCESS on success
*	    XST_FAILURE on failure
*
*****************************************************************************/
u32 XPmonPsv_GetReadCounter(const XPmonPsv *InstancePtr,u32 *ReadRequestValue,
				u32 *ReadRespValue, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ReadRequestValue != NULL);
	Xil_AssertNonvoid(ReadRespValue != NULL);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	*ReadRequestValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_RD_REQ_COUNTER0_VAL);
	*ReadRespValue = XPmonPsv_ReadReg(InstancePtr,
				Offset + PMONPSV_RD_RESP_COUNTER0_VAL);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function enables the following in the Performance Monitor:
*   - Global clock counter
*
* @param    InstancePtr is a pointer to the XPmonPsv instance.
*           SampleInterval is the sample interval for the sampled metric
*           counters
* @param    Domain is one of the counter like lpd_main or r5_domain
* @param    CounterNum is the Counter Number to be enabled.
*
* @return   XST_SUCCESS on success
*	    XST_FAILURE on failure
*
* @note	    None
******************************************************************************/
u32 XPmonPsv_EnableCounters(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	/**
	 *Setting register field GlobalEn to 1 enables the tracing and
	 *statistics collection subsystems of the packet probe.
	 */
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function unlocks the PmonPsv
*
* @param        InstancePtr is a pointer to the XPmonPsv instance.
*
* @return       None
*
*
******************************************************************************/
void XPmonPsv_Unlock(const XPmonPsv *InstancePtr)
{
	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XPmonPsv_WriteReg(InstancePtr, (u32)PMONPSV_APM0_LAR, 0xC5ACCE55U);
}
/*****************************************************************************/
/**
*
* This function locks the PmonPsv
*
* @param        InstancePtr is a pointer to the XPmonPsv instance.
*
* @return       None
*
*
******************************************************************************/
void XPmonPsv_Lock(const XPmonPsv *InstancePtr)
{
	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XPmonPsv_WriteReg(InstancePtr, (u32)PMONPSV_APM0_LAR, 0x0U);
}
/*****************************************************************************/
/**
*
* This function disables the following in the Performance Monitor:
*   - Global clock counter
*
* @param        InstancePtr is a pointer to the XPmonPsv instance.
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure
*
******************************************************************************/
u32 XPmonPsv_StopCounter(const XPmonPsv *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (u32)XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_CFGCTRL,
              0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_CFGCTRL,
              0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_CFGCTRL,
              0x0U);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_CFGCTRL,
              0x0U);
	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function selects the Source to be monitored
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	SrcSel is the value of the sourceselect.
*
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure
*
*****************************************************************************/
u32 XPmonPsv_SetSrc(const XPmonPsv *InstancePtr, u32 SrcSel , u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);
	/**
	 *Set the source
	 *	000 - OFF  Counter disabled.
	 *	001 - CYCLE8  Probe clock cycles.
	 *	002 - IDLE Idle cycles during which no packet data is observed.
	 *	003 - XFER Transfer cycles during which packet data is transferred.
	 *	004 - BUSY Busy cycles during which the packet data is made available by the
	 *		transmitting agent but the receiving agent is not ready to receive it.
	 *	005 - WAIT Wait cycles during a packet in which the transmitting agent suspends
	 *		the transfer of packet data.
	 *	006 -  PKT Packets.
	 *	007 - LUT1  Packets selected by the LUT.
	 *	008 - BYTE2  Total number of payload bytes.
	 */
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_WR_REQ_COUNTER0_SRC, SrcSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_WR_RESP_COUNTER0_SRC, SrcSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_RD_REQ_COUNTER0_SRC, SrcSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_RD_RESP_COUNTER0_SRC, SrcSel);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function selects the Port to be monitored
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
* @param	PortSel is the value of the portselect.
* 		eg:for lpd_main
* 			0: lpd_fpd_axi
* 			1: prot_xppu
* 		for r5_domain
* 			0 : lpd_afifs_axi
* 			1 : lpd_ocm
*              		2 : lpd_ocmext
*               	3 : lpd_pmc_rpu_axi0
*
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure
*
*****************************************************************************/
u32 XPmonPsv_SetPort(const XPmonPsv *InstancePtr, u32 PortSel, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_WR_REQ_COUNTER0_PORTSEL, PortSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_WR_RESP_COUNTER0_PORTSEL, PortSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_RD_REQ_COUNTER0_PORTSEL, PortSel);
	XPmonPsv_WriteReg(InstancePtr, Offset + PMONPSV_RD_RESP_COUNTER0_PORTSEL, PortSel);
	return XST_SUCCESS;
}
/** @} */
