/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp_example.c
*
* This file provides the implementation of the HDCP example
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#if (XPAR_XHDCP_NUM_INSTANCES > 0)
#include "xhdcp1x.h"
#include "xhdcp1x_port.h"
#include "xhdcp1x_example.h"
#include "xintc.h"

#include "xtmrctr.h"
#include "dppt.h"

/************************** Example Configuration ****************************/
#if defined(XPAR_XHDMI_RX_NUM_INSTANCES) || defined(XPAR_XHDMI_TX_NUM_INSTANCES)
	#define HDMI_EXAMPLE
#endif
#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_1_FLOW_DIRECTION == 0)
	#define DPTX_EXAMPLE
#endif
#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_0_FLOW_DIRECTION != 0)
	#define DPRX_EXAMPLE
#endif

/************************** Constant Definitions *****************************/
#if defined(HDMI_EXAMPLE)

#include "xhdmi_rx.h"
#include "xhdmi_tx.h"

#define HDCP_TIMER_ID			(XPAR_TMRCTR_0_DEVICE_ID)
#define HDCP_TIMER_VEC_ID		(XPAR_INTC_0_TMRCTR_0_VEC_ID)
#define HDCP_TIMER_FREQ			(XPAR_TMRCTR_0_CLOCK_FREQ_HZ)

#define HDCP_RX_DEV_ID			(XPAR_XHDCP_0_DEVICE_ID)
#define HDCP_RX_VEC_ID			(XPAR_INTC_0_HDCP1X_0_VEC_ID)

#define HDCP_TX_DEV_ID			(XPAR_XHDCP_1_DEVICE_ID)
#define HDCP_TX_VEC_ID			(XPAR_INTC_0_HDCP1X_1_VEC_ID)

#define INTC				(Intc)

extern XHdmi_Tx HdmiTx;
extern XHdmi_Rx HdmiRx;
extern XIntc INTC;

#endif // HDMI Definitions done

#if defined(DPTX_EXAMPLE)

#include "xdp.h"


#define HDCP_TIMER_ID			\
	XPAR_PROCESSOR_SUBSYSTEM_INTERCONNECT_AXI_TIMER_2_DEVICE_ID
#define HDCP_TIMER_VEC_ID		\
	XPAR_INTC_0_DPTXSS_0_DPTXSS_TIMER_IRQ_VEC_ID//(XPAR_INTC_0_TMRCTR_2_VEC_ID)
#define HDCP_TIMER_FREQ			(XPAR_TMRCTR_2_CLOCK_FREQ_HZ)

//#undef  HDCP_RX_DEV_ID
//#undef  HDCP_RX_VEC_ID

#define HDCP_TX_DEV_ID			\
	(XPAR_DP_TX_HIER_DP_TX_SUBSYSTEM_0_DP_TX_HDCP_DEVICE_ID)
#define HDCP_TX_VEC_ID			\
	(XPAR_INTC_0_DPTXSS_0_DPTXSS_HDCP_IRQ_VEC_ID)

//#define INTC				(IntcInstance)


//extern XDptx DptxInstance;
//extern XIntc INTC;

#endif	/* ******** DPTX definitions done ******** */

#if defined(DPRX_EXAMPLE)

#include "xdp.h"

//#undef  HDCP_TIMER_ID
//#undef  HDCP_TIMER_VEC_ID
//#undef  HDCP_TIMER_FREQ

#define HDCP_RX_DEV_ID			\
	(XPAR_DP_RX_HIER_DP_RX_SUBSYSTEM_0_DP_RX_HDCP_DEVICE_ID)
#define HDCP_RX_VEC_ID			\
	(XPAR_INTC_0_DPRXSS_0_DPRXSS_HDCP_IRQ_VEC_ID)

//#undef  HDCP_TX_DEV_ID
//#undef  HDCP_TX_VEC_ID

extern XDpRxSs DpRxSsInst;		//DprxInstance;
//shadul for hdcp
extern XDpTxSs DpTxSsInst;

#endif	/* ******** DP RX definitions done ******** */

extern XIntc IntcInst;	//INTC;

#define INTC				(IntcInst)

//extern XDp DprxInstance;
//extern XIntc INTC;

#define TX_KEYSELECT			(0x00u)
#define RX_KEYSELECT			(0x01u)
//#define RX_KEYSELECT			(0x00u)

/***************** Macros (Inline Functions) Definitions *********************/

/**************************** Type Definitions *******************************/

/****************************** Local Globals ********************************/

/**************************** Exported Globals *******************************/
XHdcp1x HdcpIf[XPAR_XHDCP_NUM_INSTANCES];
//XHdcp1x HdcpIf[1];		//for Rx only
#if defined(HDCP_TIMER_ID)
XTmrCtr HdcpTimer;
#endif

/************************** Function Prototypes ******************************/

#if defined(HDCP_TIMER_ID)

#if 0
/*****************************************************************************/
/**
*
* This function serves as the timer callback function
*
* @param CallBackRef  the callback reference value
* @param TimerChannel  the channel within the timer that expired
*
* @return
*   void
*
* @note
*   None
*
******************************************************************************/
static void TimerCallback(void* CallBackRef, u8 TimerChannel)
{
	XHdcp1x* HdcpPtr = CallBackRef;

	XHdcp1x_HandleTimeout(HdcpPtr);
	return;
}
#endif

#if 0
/******************************************************************************/
/**
*
* This function converts from microseconds to timer ticks
*
* @param TimeoutInUs  the timeout to convert
* @param ClockFrequency  the clock frequency to use in the conversion
*
* @return
*   The number of "ticks"
*
* @note
*   None.
*
******************************************************************************/
static u32 ConvertUsToTicks(u32 TimeoutInUs, u32 ClockFrequency)
{
	u32 TimeoutFreq = 0;
	u32 NumTicks = 0;

	/* Check for greater than one second */
	if (TimeoutInUs > 1000000ul) {
		u32 NumSeconds = 0;

		/* Determine theNumSeconds */
		NumSeconds = (TimeoutInUs/1000000ul);

		/* Update theNumTicks */
		NumTicks = (NumSeconds*ClockFrequency);

		/* Adjust theTimeoutInUs */
		TimeoutInUs -= (NumSeconds*1000000ul);
	}

	/* Convert TimeoutFreq to a frequency */
	TimeoutFreq  = 1000;
	TimeoutFreq *= 1000;
	TimeoutFreq /= TimeoutInUs;

	/* Update NumTicks */
	NumTicks += ((ClockFrequency / TimeoutFreq) + 1);

	return (NumTicks);
}
#endif

#if 0
/******************************************************************************/
/**
*
* This function starts a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
* @param TimeoutInMs  the timer duration in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
static int StartTimer(const XHdcp1x* InstancePtr, u16 TimeoutInMs)
{
//	xil_printf("Starting HDCP timer \r\n");
	XTmrCtr *TimerPtr = &HdcpTimer;
	u8 TimerChannel = 0;
	u32 TimerOptions = 0;
	u32 NumTicks = 0;

	/* Determine NumTicks */
	NumTicks = ConvertUsToTicks((TimeoutInMs*1000ul), HDCP_TIMER_FREQ);

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	/* Configure the callback */
	XTmrCtr_SetHandler(TimerPtr, &TimerCallback, (void*) InstancePtr);

	/* Configure the timer options */
	TimerOptions  = XTmrCtr_GetOptions(TimerPtr, TimerChannel);
	TimerOptions |=  XTC_DOWN_COUNT_OPTION;
	TimerOptions |=  XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TimerPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TimerPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TimerPtr, TimerChannel);

	return (XST_SUCCESS);
}
#endif

#if 0
/******************************************************************************/
/**
*
* This function stops a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
static int StopTimer(const XHdcp1x* InstancePtr)
{
	XTmrCtr *TimerPtr = &HdcpTimer;
	u8 TimerChannel = 0;

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	return (XST_SUCCESS);
}
#endif

#if 0
/******************************************************************************/
/**
*
* This function busy waits for an interval on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
* @param DelayInMs  the delay duration in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/

static int BusyDelay(const XHdcp1x* InstancePtr, u16 DelayInMs)
{
	XTmrCtr *TimerPtr = &HdcpTimer;
	u8 TimerChannel = 0;
	u32 TimerOptions = 0;
	u32 NumTicks = 0;

	/* Determine NumTicks */
	NumTicks = ConvertUsToTicks((DelayInMs*1000ul), HDCP_TIMER_FREQ);

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	/* Configure the timer options */
	TimerOptions  = XTmrCtr_GetOptions(TimerPtr, TimerChannel);
	TimerOptions |=  XTC_DOWN_COUNT_OPTION;
	TimerOptions &= ~XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TimerPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TimerPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TimerPtr, TimerChannel);

	/* Wait until done */
	while (!XTmrCtr_IsExpired(TimerPtr, TimerChannel));

	return (XST_SUCCESS);
}
#endif

#endif

/*****************************************************************************/
/**
*
* This function retrieves the physical interface for a hdcp device id
*
* @param DeviceID  the device id
*
* @return
*   Pointer to the corresponding physical interface.  NULL if none found.
*
* @note
*   None.
*
******************************************************************************/
//static void *GetPhyIfPtr(u16 DeviceID)
//{
//	void *PhyIfPtr = NULL;
//
//	/* Which device? */
//	switch (DeviceID) {
//
//#if defined(HDMI_EXAMPLE)
//	/* Rx Device */
//	case HDCP_RX_DEV_ID:
//		PhyIfPtr = &HdmiRx;
//		break;
//	/* Tx Device */
//	case HDCP_TX_DEV_ID:
//		PhyIfPtr = &HdmiTx;
//		break;
//#endif
//#if defined(DPTX_EXAMPLE)
//	/* Tx Device */
//	case HDCP_TX_DEV_ID:
//		PhyIfPtr = DpTxSsInst.DpPtr;	//&DptxInstance;
//		break;
//#endif
//#if defined(DPRX_EXAMPLE)
//	/* Rx Device */
//	case HDCP_RX_DEV_ID:
//		//->RxInstance;		BUG IN THE CODE AS GIVEN BY ANDREI
//		PhyIfPtr = DpRxSsInst.DpPtr;
//		break;
//#endif
//
//	/* Otherwise */
//	default:
//		break;
//	}
//
//	return (PhyIfPtr);
//}

/*****************************************************************************/
/**
*
* This function configures and initializes the interrupts associated with the
* hdcp example module
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None
*
******************************************************************************/
static int SetupInterrupts(void)
{
	int Status = XST_SUCCESS;

#if defined(HDCP_TIMER_VEC_ID)
	/* Connect and enable the hardware timer interrupt */
//	XIntc_Connect(&INTC, HDCP_TIMER_VEC_ID, XTmrCtr_InterruptHandler,
//			&HdcpTimer);
	XIntc_Connect(&INTC, HDCP_TIMER_VEC_ID, XTmrCtr_InterruptHandler,
			DpTxSsInst.TmrCtrPtr);
	XIntc_Enable(&INTC, HDCP_TIMER_VEC_ID);
#endif

#if defined(HDCP_RX_VEC_ID)
	/* Connect and enable the receive cipher interrupt */
//	XIntc_Connect(&INTC, HDCP_RX_VEC_ID, XHdcp1x_CipherIntrHandler,
//			&HdcpIf[HDCP_RX_DEV_ID]);
	XIntc_Connect(&INTC, HDCP_RX_VEC_ID, XHdcp1x_CipherIntrHandler,
			DpRxSsInst.Hdcp1xPtr);
	XIntc_Enable(&INTC, HDCP_RX_VEC_ID);
#endif

#if defined(HDCP_TX_VEC_ID)
	/* Connect and enable the transmit cipher interrupt */
//	XIntc_Connect(&INTC, HDCP_TX_VEC_ID, XHdcp1x_CipherIntrHandler,
//			&HdcpIf[HDCP_TX_DEV_ID]);
	XIntc_Connect(&INTC, HDCP_TX_VEC_ID, XHdcp1x_CipherIntrHandler,
			DpTxSsInst.Hdcp1xPtr);
	XIntc_Enable(&INTC, HDCP_TX_VEC_ID);
#endif

	return (Status);
}


/*****************************************************************************/
/**
*
* This initializes the hdcp example module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_Init(void)
{
//	u16 DeviceID = 0;
	int Status = XST_SUCCESS;

#if defined(HDCP_TIMER_ID)
	/* Initialize the timer for hdcp */
//	XTmrCtr_Initialize(&HdcpTimer, HDCP_TIMER_ID);

	/* Initialize the hdcp timer functions */
//	XHdcp1x_SetTimerStart(&StartTimer);
//	XHdcp1x_SetTimerStop(&StopTimer);
//	XHdcp1x_SetTimerDelay(&BusyDelay);
#endif


	/* Initialize the debug printf function */
	XHdcp1x_SetDebugPrintf(xil_printf);
//shadul
	#if 0
	/* Initialize each hdcp interface */
//	for (DeviceID = 0; DeviceID < XPAR_XHDCP_NUM_INSTANCES; DeviceID++) {
		for (DeviceID = 0; DeviceID < 1; DeviceID++) {
	//for (DeviceID = 0; DeviceID < 1; DeviceID++) {

		/* Loop variables */
		XHdcp1x_Config* CfgPtr = XHdcp1x_LookupConfig(DeviceID);
				//CfgPtr->IsRx, CfgPtr->IsHDMI);
		void *PhyIfPtr = GetPhyIfPtr(DeviceID);

		/* Initialize the hdcp interface */
		Status = XHdcp1x_CfgInitialize(&HdcpIf[DeviceID],
										CfgPtr, PhyIfPtr, CfgPtr->BaseAddress);
		if (Status != XST_SUCCESS) {
			xil_printf("XHdcp1x_CfgInitialize failed.\n");
			while(1);
		}

		/* Self-test the hdcp interface */
		if (XHdcp1x_SelfTest(&HdcpIf[DeviceID]) != XST_SUCCESS) {
			xil_printf("HDCP %d self-test failed \r\n",DeviceID);
			Status = XST_FAILURE;
		}
	}

	/* Configure the key selection vectors */
#if defined(HDCP_RX_DEV_ID)
	XHdcp1x_SetKeySelect(&HdcpIf[HDCP_RX_DEV_ID], RX_KEYSELECT);
#endif
#if defined(HDCP_TX_DEV_ID)

//Commeted by shadul
//	XHdcp1x_SetKeySelect(&HdcpIf[HDCP_TX_DEV_ID], TX_KEYSELECT);

#endif
#endif

	/* Setup interrupts */
	SetupInterrupts();

	return (Status);
}

/*****************************************************************************/
/**
*
* This function enables the hdcp example module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_Enable(void)
{
//	u16 DeviceID = 0;
	int Status = XST_SUCCESS;

	/* Enable each hdcp interface */
//	for (DeviceID = 0; DeviceID < XPAR_XHDCP_NUM_INSTANCES; DeviceID++) {

//shadul-->RX dev id is zero. Enabling only RX HDCP.
//	for (DeviceID = 0; DeviceID < 1; DeviceID++) {

//	for (DeviceID = 0; DeviceID < 1; DeviceID++) {
		XDpRxSs_HdcpEnable(&DpRxSsInst);
		XDpTxSs_HdcpEnable(&DpTxSsInst);
//	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function polls the hdcp example module
*
* @return
*   void
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
void XHdcp1xExample_Poll(void)
{
	/* Locals */
	u16 DeviceID = 0;

	/* Poll each hdcp interface */
	for (DeviceID = 0; DeviceID < 1; DeviceID++) {
//	for (DeviceID = 0; DeviceID < 1; DeviceID++) {
//		XHdcp1x_Poll(&HdcpIf[DeviceID]);
		XDpRxSs_Poll(&DpRxSsInst);
		XDpTxSs_Poll(&DpTxSsInst);
	}

	/* Return */
	return;
}

/*****************************************************************************/
/**
*
* This function retrieves a specific XHdcp instance
*
* @param DeviceId  the device id to retrieve
*
* @return
*   Pointer to the specific XHdcp instance. NULL if not found.
*
* @note
*   None
*
******************************************************************************/
XHdcp1x* XHdcp1xExample_Get(u16 DeviceId)
{
	XHdcp1x* HdcpPtr = NULL;

	if (DeviceId < XPAR_XHDCP_NUM_INSTANCES) {
//	if (DeviceId < 1) {
		if (DeviceId == 0) //added by shadul
			HdcpPtr = DpRxSsInst.Hdcp1xPtr; //added by shadul
		else //added by shadul
			HdcpPtr = DpTxSsInst.Hdcp1xPtr; //added by shadul
	}

	return (HdcpPtr);
}

/*****************************************************************************/
/**
*
* This function authenticates the hdcp tx example module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_TxAuthenticate(void)
{
	int Status;

	/* Enable hdcp tx interface */
//	Status = XHdcp1x_Authenticate(&HdcpIf[HDCP_TX_DEV_ID]);
	Status = XDpTxSs_Authenticate(&DpTxSsInst);
	if (Status!=XST_SUCCESS){
		xil_printf("HDCP_TX authentication failed \r\n");
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function encrypts the hdcp tx example module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_TxEncrypt(void)
{
	int Status;

	/*
	 * Encrypt the hdcp tx interface
	 * Here the stream Map will always be one in our test case
	 * as there is only one receiver attached to the HDCP downstream
	 * connection. In case of REPEATER please update this.
	 */
#if ENABLE_HDCP_DEBUG_LOG
//	xil_printf("Encrypting Tx ... , CurrentState = %d \r\n",
//		HdcpIf[1].Tx.CurrentState);
#endif
//	Status = XHdcp1x_EnableEncryption(&HdcpIf[HDCP_TX_DEV_ID],0x1);
		Status = XDpTxSs_EnableEncryption(&DpTxSsInst,0x1);
	if (Status!=XST_SUCCESS){
		xil_printf("HDCP_TX encryption failed \r\n");
		Status = XST_FAILURE;
	}
#if ENABLE_HDCP_DEBUG_LOG
	xil_printf(" ... done Encrypting Tx, CurrentState = %d \r\n",
		HdcpIf[1].Tx.CurrentState);
#endif
	return (Status);
}

/*****************************************************************************/
/**
*
* This function encrypts the hdcp tx example module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_TxIsCapable(void)
{
	int Status;

	/* Check the BCAPS register */
//	Status = XHdcp1x_PortIsCapable(&HdcpIf[HDCP_TX_DEV_ID]);
	Status = XDpTxSs_IsHdcpCapable(&DpTxSsInst);
	if (Status==FALSE){
		xil_printf("HDCP_TX is not HDCP capable \r\n");
		Status = XST_FAILURE;
	}

	return (Status);
}

//InstancePtr->Rx.CurrentState check the HDCP Rx state and
//	if authenticated authenticate Tx as well
/*****************************************************************************/
/**
*
* This function checks if HDCP Rx has been authenticated,
*	and if HDCP Rx has been
* authenticated, and HDCP encrypted content is coming to Tx
*	then it authenticates
* HDCP Tx as well.
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_TxEncryptIfRxIsUp(void)
{
	int Status;
	unsigned int CurrentRxState;
	unsigned int CurrentTxState;

	Status = XST_FAILURE;
//	CurrentRxState = HdcpIf[HDCP_RX_DEV_ID].Rx.CurrentState;
	CurrentRxState = DpRxSsInst.Hdcp1xPtr->Rx.CurrentState;
//	CurrentTxState = HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState;
	CurrentTxState = DpTxSsInst.Hdcp1xPtr->Tx.CurrentState;
	if(CurrentRxState == 3 && CurrentTxState == 5){
		// 3 = XHDCP1X_STATE_AUTHENTICATED
			//in the XHdcp1x_StateType in xhdcp1x_rx.c
		// 11 = XHDCP1X_STATE_PHYDOWN
			//in the XHdcp1x_StateType in xhdcp1x_tx.c
		// 5 = XHDCP1X_STATE_AUTHENTICATED
			//in the XHdcp1x_StateType in xhdcp1x_tx.c
		//We will now enable, authenticate and encrypt the HDCP Tx
//		xil_printf("Rx is authenticated (and Tx is phydown)!!
//			Rx State = %d\r\n",HdcpIf[HDCP_RX_DEV_ID].Rx.CurrentState);
		Status = XST_SUCCESS;
	}
	else{
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function checks if HDCP Tx has been authenticated and is
*	sending encrypted data.
*
* @return
*   XST_SUCCESS if successful.
*   XST_FAILURE if unsuccessful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1xExample_TxIsAuthenticated(void)
{
	int Status;
	unsigned int CurrentTxState;

	Status = XST_FAILURE;
//	CurrentTxState = HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState;
	CurrentTxState = DpTxSsInst.Hdcp1xPtr->Tx.CurrentState;
//	xil_printf("\r\n%s : Tx State = %d\r\n",__func__,
		//HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState);
	while(CurrentTxState!=5)
	{
//		CurrentTxState = HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState;
		CurrentTxState = DpTxSsInst.Hdcp1xPtr->Tx.CurrentState;

		if(CurrentTxState == 5){
			/*
			 *	5 = XHDCP1X_STATE_AUTHENTICATED
				in the XHdcp1x_StateType in xhdcp1x_tx.c
			 */
//			xil_printf("Tx is authenticated!! Tx State =
//				%d\r\n",HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState);
			Status = XST_SUCCESS;
			break;
		}
		else if( (CurrentTxState==10) || (CurrentTxState==11) )
		{
			/*
			 *	10 = XHDCP1X_STATE_UNAUTHENTICATED
				in the XHdcp1x_StateType in xhdcp1x_tx.c
			 *	11 = XHDCP1X_STATE_PHYDOWN
				in the XHdcp1x_StateType in xhdcp1x_tx.c
			 */
#if ENABLE_HDCP_DEBUG_LOG
	xil_printf("\r\n %s :DP_HDCP_TX Authentication failure !\r\n",__func__);
			xil_printf("State(Prev) %d -> State(Current) %d \r\n",
					HdcpIf[HDCP_TX_DEV_ID].Tx.PreviousState,
					HdcpIf[HDCP_TX_DEV_ID].Tx.CurrentState);
#endif
			Status = XST_FAILURE;
			break;
		}
		else if (CurrentTxState==4)
		{
			/*
			 * 4 = XHDCP1X_STATE_VALIDATE_RX
			 in the XHdcp1x_StateType in xhdcp1x_tx.c
			 */
			XHdcp1xExample_Poll();
		}
		else{
//			xil_printf("CurrentTxState = %d \r\n",CurrentTxState);
		}

	}

	if(CurrentTxState == 5){
		Status = XST_SUCCESS;
	}

	return (Status);
}
#endif
