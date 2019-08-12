/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xpmonpsv.c
* @addtogroup  pmonpsv_v1_0
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
* This function initializes a specific XpsvPmon device/instance. This function
* must be called prior to using the Performance Monitor device.
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
* @param	ConfigPtr points to the XpsvPmon device configuration structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address is passed.
*		Unexpected errors may occur if the address mapping is changed
*		after this function is invoked.
*
* @return
*		- XST_SUCCESS on initialization completion
*
* @note		The user needs to first call the XpsvPmon_LookupConfig() API
*		which returns the Configuration structure pointer which is
*		passed as a parameter to the XpsvPmon_CfgInitialize() API.
*
******************************************************************************/
s32 XpsvPmon_CfgInitialize(XpsvPmon *InstancePtr, const XPmonpsv_Config *ConfigPtr,
						UINTPTR EffectiveAddr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

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
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
u32 XpsvPmon_ResetCounter(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return (s32)XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	/*
	 *  Reset the counters
	 *   All statistics counters are cleared when the StatEn
	 *   bit goes from 0 to 1
	 */
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL, 0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL, 0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL, 0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL, 0x0U);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL,
		MAINCTRL_STATEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL,
		MAINCTRL_STATEN_MASK);

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function sets Metrics for specified Counter in the corresponding
* Metric Selector Register.
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
u32 XpsvPmon_SetMetrics(const XpsvPmon *InstancePtr, u32 StatPeriod, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return (s32)XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_STATPERIOD, StatPeriod);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_STATPERIOD, StatPeriod);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_STATPERIOD, StatPeriod);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_MAINCTRL,
			MAINCTRL_STATEN_MASK | MAINCTRL_STATCONDDUMP_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_STATPERIOD, StatPeriod);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns Metrics in the specified Counter from the corresponding
* Metric Selector Register.
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_GetMetrics(const XpsvPmon *InstancePtr, u32 CounterNum, u8 *MainCtl,
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
		return (s32)XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	RegValue = XpsvPmon_ReadReg(InstancePtr,
				Offset + PMONPSV_PROBE2_MAINCTRL);
	*MainCtl = (u8)(RegValue & 0xffU);
	RegValue = XpsvPmon_ReadReg(InstancePtr,
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
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_GetWriteCounter(const XpsvPmon *InstancePtr,u32 *WriteRequestValue,
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
		return (s32)XST_FAILURE;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	*WriteRequestValue = XpsvPmon_ReadReg(InstancePtr,
				Offset + PMONPSV_WR_REQ_COUNTER0_VAL);
	*WriteRespValue = XpsvPmon_ReadReg(InstancePtr,
				Offset + PMONPSV_WR_RESP_COUNTER0_VAL);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns success if a free counter was found and request
* was granted.
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
* 		which the value of Write Request Count has to be filled
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum  pointer to get Counter Number.
*
*
* @return	Granted counter number.
* 		XST_FAILURE otherwise.
*
*****************************************************************************/
s32 XpsvPmon_RequestCounter(XpsvPmon *InstancePtr,u32 Domain, u32 *CounterNum)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(CounterNum != NULL);

	/* Unrequested give the 0th counter */
	if ( InstancePtr->RequestedCounters[Domain] == 0xFF )
		InstancePtr->RequestedCounters[Domain] = 0U;

	if (InstancePtr->RequestedCounters[Domain] > XPMONPSV_MAX_COUNTERS) {
		return (s32)XST_FAILURE;
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
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_GetReadCounter(const XpsvPmon *InstancePtr,u32 *ReadRequestValue,
				u32 *ReadRespValue, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ReadRequestValue != NULL);
	Xil_AssertNonvoid(ReadRespValue != NULL);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (s32)XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	*ReadRequestValue = XpsvPmon_ReadReg(InstancePtr,
				Offset + PMONPSV_RD_REQ_COUNTER0_VAL);
	*ReadRespValue = XpsvPmon_ReadReg(InstancePtr,
				Offset + PMONPSV_RD_RESP_COUNTER0_VAL);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function enables the following in the Performance Monitor:
*   - Global clock counter
*
* @param    InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_EnableCounters(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (s32)XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	/**
	 *Setting register field GlobalEn to 1 enables the tracing and
	 *statistics collection subsystems of the packet probe.
	 */
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_CFGCTRL,
		CFGCTRL_GLOBALEN_MASK);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function unlocks the Pmonpsv
*
* @param        InstancePtr is a pointer to the XpsvPmon instance.
*
* @return       XST_SUCCESS
*
*
******************************************************************************/
s32 XpsvPmon_Unlock(const XpsvPmon *InstancePtr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XpsvPmon_WriteReg(InstancePtr, (u32)PMONPSV_APM0_LAR, 0xC5ACCE55U);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function locks the Pmonpsv
*
* @param        InstancePtr is a pointer to the XpsvPmon instance.
*
* @return       XST_SUCCESS
*
*
******************************************************************************/
s32 XpsvPmon_Lock(const XpsvPmon *InstancePtr)
{
	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	XpsvPmon_WriteReg(InstancePtr, (u32)PMONPSV_APM0_LAR, 0x0U);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function disables the following in the Performance Monitor:
*   - Global clock counter
*
* @param        InstancePtr is a pointer to the XpsvPmon instance.
* @param	Domain is one of the counter like lpd_main or r5_domain
* @param	CounterNum is the Counter Number.
*		The valid values are 0 to 9.
*
* @return	XST_SUCCESS on success
*		XST_FAILURE on failure
*
******************************************************************************/
s32 XpsvPmon_StopCounter(const XpsvPmon *InstancePtr, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (s32)XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	Offset = Offset +  (CounterNum* XPMONPSV_COUNTER_OFFSET);

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE1_CFGCTRL,
              0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE2_CFGCTRL,
              0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE3_CFGCTRL,
              0x0U);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_PROBE4_CFGCTRL,
              0x0U);
	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function selects the Source to be monitored
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_SetSrc(const XpsvPmon *InstancePtr, u32 SrcSel , u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (s32)XST_FAILURE;
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
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_WR_REQ_COUNTER0_SRC, SrcSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_WR_RESP_COUNTER0_SRC, SrcSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_RD_REQ_COUNTER0_SRC, SrcSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_RD_RESP_COUNTER0_SRC, SrcSel);
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function selects the Port to be monitored
*
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
s32 XpsvPmon_SetPort(const XpsvPmon *InstancePtr, u32 PortSel, u32 Domain, u32 CounterNum)
{
	u32 Offset=0;

	/* Assert the arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->RequestedCounters[Domain] < CounterNum) {
		return (s32)XST_FAILURE;
	}

	if (Domain == XPMONPSV_LPD_MAIN_DOMAIN) {
		Offset = LPD_MAIN_OFFSET;
	}

	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_WR_REQ_COUNTER0_PORTSEL, PortSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_WR_RESP_COUNTER0_PORTSEL, PortSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_RD_REQ_COUNTER0_PORTSEL, PortSel);
	XpsvPmon_WriteReg(InstancePtr, Offset + PMONPSV_RD_RESP_COUNTER0_PORTSEL, PortSel);
	return XST_SUCCESS;
}
/*****************************************************************************/
/** @} */
