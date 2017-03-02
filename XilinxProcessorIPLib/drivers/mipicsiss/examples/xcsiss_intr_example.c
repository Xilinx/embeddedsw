/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* @file xcsiss_intr_example.c
*
* This file contains a design example using the XCsiSs driver with interrupts.
* On receiving a frame received interrupt, it will print frames received count.
* On receiving a short packet FIFO not empty interrupt, it will print the
* contents of the short packet received.
* On receiving DPHY, protocol or Packet level error, it will print the same
* On receiving any type of error interrupt the sub-system will reset
*
* @note		This example requires an interrupt controller connected to the
*		processor and the MIPI CSI Rx Subsystem in the system.
*		The Camera Sensor may need some programming before via the IIC
*		before it starts to send any video stream.
*
*		For this example to display output, the user need to implement
*		initialization of the system (CsiSs_PlatformInit) and after
*		MIPI CSI Rx subsystem start (XCsiSs_Start) is complete,
*		implement configuration of the video stream source in order to
*		provide the MIPI CSI Rx Subsystem HIP input.
*		The functions CsiSs_PlatformInit and CsiSs_StreamSrc are
*		declared and are left up to the user implement.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/21/15 Initial release
* 1.1 ms  01/23/17 Modified xil_printf statement in main function to
*                  ensure that "Successfully ran" and "Failed" strings
*                  are available in all examples. This is a fix for
*                  CR-965028.
* 1.2 vsa 03/02/17 Added Word Count corruption interrupt
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsiss.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"

/************************** Constant Definitions *****************************/

/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
*/
#define XINTC_CSISS_CSI_INTERRUPT_ID	XPAR_INTC_0_MIPICSISS_0_VEC_ID
#define XINTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define XINTC				XIntc
#define XINTC_HANDLER			XIntc_InterruptHandler

/* The unique device ID of the MIPI CSI Rx Subsystem instance to be used
 */
#define XCSISS_DEVICE_ID		XPAR_CSISS_0_DEVICE_ID
#define MAX_INTERRUPT_COUNT 	1

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 CsiSs_IntrExample(u32 DeviceId);
u32 CsiSs_PlatformInit(void);
u32 CsiSs_CciInit(void);
u32 CsiSs_CciSetupIntrSystem(void);

/* Interrupt helper functions */
u32 CsiSs_SetupIntrSystem(void);
void CsiSs_DphyEventHandler(void *InstancePtr, u32 Mask);
void CsiSs_PktLvlEventHandler(void *InstancePtr, u32 Mask);
void CsiSs_ProtLvlEventHandler(void *InstancePtr, u32 Mask);
void CsiSs_SPktEventHandler(void *InstancePtr, u32 Mask);
void CsiSs_ErrEventHandler(void *InstancePtr, u32 Mask);
void CsiSs_FrameRcvdEventHandler(void *InstancePtr, u32 Mask);

/************************** Variable Definitions *****************************/

XCsiSs CsiSsInst;	/* The MIPI CSI Rx Subsystem instance.*/
XINTC IntcInst;		/* The interrupt controller instance. */

#if (XPAR_XIIC_NUM_INSTANCES > 0)
XIic *IicInstPtr;
#endif
volatile u32 interrupt_counts;

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
*
* For Microblaze we use an assembly loop that is roughly the same regardless of
* optimization level, although caches and memory access time can make the delay
* vary.  Just keep in mind that after resetting or updating the PHY modes,
* the PHY typically needs time to recover.
*
* @param	Number of seconds to sleep
*
* @return	None
*
* @note		None
*
******************************************************************************/
void Delay(u32 Seconds)
{
#if defined (__MICROBLAZE__) || defined(__PPC__)
	static s32 WarningFlag = 0;

	/* If MB caches are disabled or do not exist, this delay loop could
	 * take minutes instead of seconds (e.g., 30x longer).  Print a warning
	 * message for the user (once).  If only MB had a built-in timer!
	 */
	if (((mfmsr() & 0x20) == 0) && (!WarningFlag)) {
		WarningFlag = 1;
	}

#define ITERS_PER_SEC   (XPAR_CPU_CORE_CLOCK_FREQ_HZ / 6)
    asm volatile ("\n"
			"1:               \n\t"
			"addik r7, r0, %0 \n\t"
			"2:               \n\t"
			"addik r7, r7, -1 \n\t"
			"bneid  r7, 2b    \n\t"
			"or  r0, r0, r0   \n\t"
			"bneid %1, 1b     \n\t"
			"addik %1, %1, -1 \n\t"
			:: "i"(ITERS_PER_SEC), "d" (Seconds));
#else
    sleep(Seconds);
#endif
}

#ifndef TESTAPP_GEN
/*****************************************************************************/
/**
*
* This is the main function for XCsiSs interrupt example. If the
* CsiSs_IntrExample function which sets up the system succeeds, this function
* will wait for the interrupts. Once a connection event or pulse is detected,
* CsiSs will RX device capabilities and re-start the subsystem.
*
* @param	None.
*
* @return
*		- XST_FAILURE if the interrupt example was unsuccessful.
*
* @note		Unless setup failed, main will never return since
*		CsiSs_IntrExample is blocking (it is waiting on interrupts
*		for Hot-Plug-Detect (HPD) events.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("------------------------------------------\n\r");
	xil_printf("MIPI CSI Rx Subsystem interrupt example\n\r");
	xil_printf("-------------------------------------------\n\r");

	Status = CsiSs_IntrExample(XCSISS_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("MIPI CSI Rx Subsystem interrupt example failed.");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran MIPI CSI Rx Subsystem interrupt example\n\r");

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* XCsiSs driver. This function will set up the system with interrupts handlers.
*
* @param	DeviceId is the unique device ID of the MIPI CSI Rx
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		  was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling.
*
******************************************************************************/
u32 CsiSs_IntrExample(u32 DeviceId)
{
	u32 Status;
	XCsiSs_Config *ConfigPtr;
	u8 ActiveLanes = 3;
	u32 IntrRequest = XCSISS_ISR_ALLINTR_MASK;
	u8 Exit_Count = 0;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = CsiSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

	/* Obtain the device configuration for the MIPI CSI Rx Subsystem */
	ConfigPtr = XCsiSs_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into the CsiSsInst's Config
	 * structure. */
	Status = XCsiSs_CfgInitialize(&CsiSsInst, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("CSISS config initialization "
		"failed.\n\r");
		return XST_FAILURE;
	}

	/* Dump the configuration */
	XCsiSs_ReportCoreInfo(&CsiSsInst);

	/* Reset the subsystem */
	XCsiSs_Reset(&CsiSsInst);

	/* Disable the subsystem till the camera
	 * and interrupts are configured
	 */
	XCsiSs_Activate(&CsiSsInst, 0);

	/* Configure the subsystem for ActiveLanes and Interrupts
	 * The minimum value of ActiveLanes is 0 and max value is
	 * maximum lanes set in the design (max 3).
	 * The interrupt mask can be selected from the bitmasks in
	 * xcsiss_hw.h
	 */
	ActiveLanes = 0;
	IntrRequest = XCSISS_ISR_ALLINTR_MASK;

	Status = XCsiSs_Configure(&CsiSsInst, ActiveLanes, IntrRequest);
	if (Status == XST_FAILURE) {
		xil_printf("CSISS Configure failed. \n\r");
		return XST_FAILURE;
	}


#if (XPAR_XIIC_NUM_INSTANCES > 0)
	/* Initialise the IIC (inside subsys or external) separately */
	IicInstPtr = XCsiSs_GetIicInstance(&CsiSsInst);

	if (!IicInstPtr) {
		xil_printf("IIC not present in "
		" the subsystem \n\r");
		xil_printf("Need to have external "
		"IIC in design \n\r");
	}
	else {
		/* Initialise the IIC in the system design */
	}
#endif

	/* Setup the camera to send some stream */
	xil_printf("CCI Init \n\r");
	Status = CsiSs_CciInit();
	if (Status != XST_SUCCESS) {
		xil_printf("CCI init failed!\n\r");
	}
	xil_printf("Camera Control Interface "
	"initialization done.\n\r");

	/* Setup the interrupts and call back handlers */
	Status = CsiSs_SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Interrupt system "
		"setup failed.\n\r");
		return XST_FAILURE;
	}

	/* Enable the cores */
	XCsiSs_Activate(&CsiSsInst, 1);

	/* Wait up to MAX_INTERRUPT_COUNT */
	do {
		Delay(1);
		Exit_Count++;
		if(Exit_Count > 3) {
			xil_printf("CSISS Interrupt "
			" test failed, Please check design\n\r");
			return XST_FAILURE;
		}

	}
	while (interrupt_counts < MAX_INTERRUPT_COUNT);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required platform-specifc peripherals.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required peripherals are initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 CsiSs_PlatformInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function initialize required IIC instance to configure and control the
* camera.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if required IIC is initialized and
*		configured successfully.
*		- XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 CsiSs_CciInit(void)
{
	/* User is responsible to setup platform specific initialization */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function setup interrupts of the IIC instance if used for the
* MIPI CSI Rx Subsystem.
*
* @param	None
*
* @return
*		- XST_SUCCESS if IIC interrupt is configured successfully.
*		- XST_FAILURE,if IIC interrupt is failed to configure
*
* @note		None.
*
******************************************************************************/
u32 CsiSs_CciSetupIntrSystem(void)
{
	/* User is responsible to setup stream source to input CSISS */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* MIPI CSI Rx Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The MIPI CSI
* Subsystem core could be directly connected to a processor without an
* interrupt controller. The user should modify this function to fit the
* application.
*
* @param	None
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 CsiSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	/* Set the HPD interrupt handlers. */
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_DPHY,
				CsiSs_DphyEventHandler, &CsiSsInst);
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_PKTLVL,
				CsiSs_PktLvlEventHandler, &CsiSsInst);
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_PROTLVL,
				CsiSs_ProtLvlEventHandler, &CsiSsInst);
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_SHORTPACKET,
				CsiSs_SPktEventHandler, &CsiSsInst);
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_FRAMERECVD,
				CsiSs_FrameRcvdEventHandler, &CsiSsInst);
	XCsiSs_SetCallBack(&CsiSsInst, XCSISS_HANDLER_OTHERERROR,
				CsiSs_ErrEventHandler, &CsiSsInst);

	/* Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed\n\r");
		return XST_FAILURE;
	}

	/* Hook up interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_CSISS_CSI_INTERRUPT_ID,
			(XInterruptHandler)XCsiSs_IntrHandler,
				&CsiSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: MIPI CSI RX SS CSI "
		"interrupt connect  failed!\n\r");
		return XST_FAILURE;
	}

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstPtr, XINTC_CSISS_CSI_INTERRUPT_ID);

	/* Need to hookup the ISR for IIC. This is user implemented system
	 * in case application needs to use IIC in interrupt mode instead
	 * of polling mode. */
	CsiSs_CciSetupIntrSystem();

	/* Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XINTC_HANDLER, IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is called when a DPHY level error event is received by
* the MIPI CSI Rx Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for DPHY level error event.
*
******************************************************************************/
void CsiSs_DphyEventHandler(void *InstancePtr, u32 Mask)
{
	xil_printf("+===> DPHY Level Error detected.\n\r");
	interrupt_counts++;

	if (Mask & XCSISS_ISR_SOTERR_MASK) {
		xil_printf("Start of Transmission Error\n\r");
	}

	if (Mask & XCSISS_ISR_SOTSYNCERR_MASK) {
		xil_printf("Start of Transmission "
		"Sync Error \n\r");
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Packet level error event is received by
* the MIPI CSI Rx Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for Packet level error event.
*
******************************************************************************/
void CsiSs_PktLvlEventHandler(void *InstancePtr, u32 Mask)
{
	xil_printf("+===> Packet Level Error detected.\n\r");
	interrupt_counts++;

	if (Mask & XCSISS_ISR_ECC2BERR_MASK) {
		xil_printf("2 bit ECC Error \n\r");
	}

	if (Mask & XCSISS_ISR_ECC1BERR_MASK) {
		xil_printf("1 bit ECC Error \n\r");
	}

	if (Mask & XCSISS_ISR_CRCERR_MASK) {
		xil_printf("Frame CRC Error \n\r");
	}

	if (Mask & XCSISS_ISR_DATAIDERR_MASK) {
		xil_printf("Data Id Error \n\r");
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Protocol decoding level error event is
* received by the MIPI CSI Rx Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for Protocol Decoding level error
*
******************************************************************************/
void CsiSs_ProtLvlEventHandler(void *InstancePtr, u32 Mask)
{
	xil_printf("+===> Packet Level Error detected.\n\r");
	interrupt_counts++;

	if (Mask & XCSISS_ISR_VC3FSYNCERR_MASK) {
		xil_printf("VC3 Frame Sync Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC2FSYNCERR_MASK) {
		xil_printf("VC2 Frame Sync Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC1FSYNCERR_MASK) {
		xil_printf("VC1 Frame Sync Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC0FSYNCERR_MASK) {
		xil_printf("VC0 Frame Sync Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC3FLVLERR_MASK) {
		xil_printf("VC3 Frame Level Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC2FLVLERR_MASK) {
		xil_printf("VC2 Frame Level Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC1FLVLERR_MASK) {
		xil_printf("VC1 Frame Level Error \n\r");
	}

	if (Mask & XCSISS_ISR_VC0FLVLERR_MASK) {
		xil_printf("VC0 Frame Level Error \n\r");
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Other errors event is received by the
* MIPI CSI Rx Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for Other error event.
*
******************************************************************************/
void CsiSs_ErrEventHandler(void *InstancePtr, u32 Mask)
{
	XCsiSs *CsiSsInstance = (XCsiSs *)InstancePtr;

	xil_printf("+===> Other Errors detected.\n\r");
	interrupt_counts++;

	if (Mask & XCSISS_ISR_WC_MASK) {
		xil_printf("Word count corruption Error\n\r");
	}

	if (Mask & XCSISS_ISR_ILC_MASK) {
		xil_printf("Incorrect Lane Count Error \n\r");
	}

	if (Mask & XCSISS_ISR_SLBF_MASK) {
		xil_printf("Stream line buffer full Error \n\r");
	}

	if (Mask & XCSISS_ISR_STOP_MASK) {
		xil_printf("Stop Error \n\r");
		XCsiSs_IntrDisable(CsiSsInstance, XCSISS_ISR_STOP_MASK);
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Short Packet FIFO event is received by
* the MIPI CSI Rx Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for Short Packet FIFO error event.
*
******************************************************************************/
void CsiSs_SPktEventHandler(void *InstancePtr, u32 Mask)
{
	XCsiSs *CsiSsInstance = (XCsiSs *)InstancePtr;
	u32 IntrMask;

	xil_printf("+===> Short Packet Event detected.\n\r");
	interrupt_counts++;

	if (Mask & XCSISS_ISR_SPFIFONE_MASK) {
		xil_printf("Fifo not empty \n\r");
		XCsiSs_GetShortPacket(InstancePtr);
		xil_printf("Data Type = 0x%x,"
		"Virtual Channel = 0x%x, Data = 0x%x",
			CsiSsInstance->SpktData.DataType,
			CsiSsInstance->SpktData.VirtualChannel,
			CsiSsInstance->SpktData.Data);
	}

	if (Mask & XCSISS_ISR_SPFIFOF_MASK) {
		xil_printf("Fifo Full \n\r");
	}
}

/*****************************************************************************/
/**
*
* This function is called when a Frame is received by the MIPI CSI Rx
* Subsystem core.
*
* @param	InstancePtr is a pointer to the XCsiSs instance.
*
* @param	Mask of interrupt which caused this event
*
* @return	None.
*
* @note		Use the XCsiSs_SetCallback driver function to set this
*		function as the handler for Frame Receieved event.
*
******************************************************************************/
void CsiSs_FrameRcvdEventHandler(void *InstancePtr, u32 Mask)
{
	xil_printf("+=> Frame Receieved Event detected.\n\r");
	interrupt_counts++;
}
