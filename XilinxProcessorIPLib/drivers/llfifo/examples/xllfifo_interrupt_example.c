/******************************************************************************
*
* Copyright (C) 2013 - 2018 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
 *
 * @file xllfifo_interrupt_example.c
 * This file demonstrates how to use the Streaming fifo driver on the xilinx AXI
 * Streaming FIFO IP.The AXI4-Stream FIFO core allows memory mapped access to a
 * AXI-Stream interface. The core can be used to interface to AXI Streaming IPs
 * similar to the LogiCORE IP AXI Ethernet core, without having to use full DMA
 * solution.
 *
 * This is the interrupt example for the FIFO it assumes that at the
 * h/w level FIFO is connected in loopback.In these we write known amount of
 * data to the FIFO and wait for interrupts and after completely receiving the
 * data compares it with the data transmitted.
 *
 * Note: The TDEST Must be enabled in the H/W design inorder to get
 * correct RDR value.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 3.00a adk 08/10/2013 initial release CR:727787
 * 5.1   ms  01/23/17   Modified xil_printf statement in main function to
 *                      ensure that "Successfully ran" and "Failed" strings
 *                      are available in all examples. This is a fix for
 *                      CR-965028.
 *       ms  04/05/17   Added tabspace for return statements in functions for
 *                      proper documentation and Modified Comment lines
 *                      to consider it as a documentation block while
 *                      generating doxygen.
 * 5.3  rsp 11/08/18    Modified TxSend to fill SourceBuffer with non-zero
 *                      data otherwise the test can return a false positive
 *                      because DestinationBuffer is initialized with zeros.
 *                      In fact, fixing this exposed a bug in FifoRecvHandler
 *                      and caused the test to start failing. According to the
 *                      product guide (pg080) for the AXI4-Stream FIFO, the
 *                      RDFO should be read before reading RLR. Reading RLR
 *                      first will result in the RDFO being reset to zero and
 *                      no data being received.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_exception.h"
#include "xstreamer.h"
#include "xil_cache.h"
#include "xllfifo.h"
#include "xstatus.h"

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define FIFO_DEV_ID	   	XPAR_AXI_FIFO_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define FIFO_INTR_ID		XPAR_INTC_0_LLFIFO_0_VEC_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define FIFO_INTR_ID            XPAR_FABRIC_LLFIFO_0_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC           XIntc
 #define INTC_HANDLER   XIntc_InterruptHandler
#else
 #define INTC           XScuGic
 #define INTC_HANDLER   XScuGic_InterruptHandler
#endif


#define WORD_SIZE 4			/* Size of words in bytes */

#define MAX_PACKET_LEN 4

#define NO_OF_PACKETS 64

#define MAX_DATA_BUFFER_SIZE NO_OF_PACKETS*MAX_PACKET_LEN

#undef DEBUG

/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

int XLlFifoInterruptExample(XLlFifo *InstancePtr, u16 DeviceId);
int TxSend(XLlFifo *InstancePtr, u32 *SourceAddr);
static void FifoHandler(XLlFifo *Fifo);
static void FifoRecvHandler(XLlFifo *Fifo);
static void FifoSendHandler(XLlFifo *Fifo);
static void FifoErrorHandler(XLlFifo *InstancePtr, u32 Pending);
int SetupInterruptSystem(INTC *IntcInstancePtr, XLlFifo *InstancePtr,
				u16 FifoIntrId);
static void DisableIntrSystem(INTC *IntcInstancePtr, u16 FifoIntrId);

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile int Done;
volatile int Error;

/************************** Variable Definitions *****************************/

/*
 * Device instance definitions
 */
XLlFifo FifoInstance;

/*
 * Instance of the Interrupt Controller
 */
static INTC Intc;

u32 SourceBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];
u32 DestinationBuffer[MAX_DATA_BUFFER_SIZE * WORD_SIZE];

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the AXI FIFO interrupt test.
*
* @param	None
*
* @return
*		- XST_SUCCESS if tests pass
* 		- XST_FAILURE if fails.
*
* @note		None
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("--- Entering main() ---\n\r");

	Status = XLlFifoInterruptExample(&FifoInstance, FIFO_DEV_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Axi Streaming FIFO Interrupt Example Test Failed");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Axi Streaming FIFO Interrupt Example\n\r");
	xil_printf("--- Exiting main() ---\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the usage of AXI FIFO
* It does the following:
*       - Set up the output terminal if UART16550 is in the hardware build
*       - Initialize the Axi FIFO Device.
*	- Set up the interrupt handler for fifo
*	- Transmit the data
*	- Compare the data
*	- Return the result
*
* @param	InstancePtr is a pointer to the instance of the
*		XLlFifo instance.
* @param	DeviceId is Device ID of the Axi Fifo Device instance,
*		typically XPAR_<AXI_FIFO_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
int XLlFifoInterruptExample(XLlFifo *InstancePtr, u16 DeviceId)
{
	XLlFifo_Config *Config;
	int Status;
	int i;
	int err;
	Status = XST_SUCCESS;

	/* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif

	/* Initialize the Device Configuration Interface driver */
	Config = XLlFfio_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XLlFifo_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(InstancePtr);
	XLlFifo_IntClear(InstancePtr,0xffffffff);
	Status = XLlFifo_Status(InstancePtr);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
			    "Expected : 0x0\n\r",
			    XLlFifo_Status(InstancePtr));
		return XST_FAILURE;
	}

	/*
	 * Connect the Axi Streaming FIFO to the interrupt subsystem such
	 * that interrupts can occur. This function is application specific.
	 */
	Status = SetupInterruptSystem(&Intc, InstancePtr, FIFO_INTR_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed intr setup\r\n");
		return XST_FAILURE;
	}

	XLlFifo_IntEnable(InstancePtr, XLLF_INT_ALL_MASK);

	Done = 0;
	/* Transmit the Data Stream */
	Status = TxSend(InstancePtr, SourceBuffer);
	if (Status != XST_SUCCESS){
		xil_printf("Transmission of Data failed\n\r");
		return XST_FAILURE;
	}
	while(!Done);

	/* Check for errors */
	if(Error) {
		xil_printf("Errors in the FIFO\n\r");
		return XST_FAILURE;
	}

	err = 0;

	/* Compare the data sent with the data received */
	xil_printf("Comparing data...\n\r");
	for( i=0 ; i<MAX_DATA_BUFFER_SIZE ; i++ ){
		if ( *(SourceBuffer + i) != *(DestinationBuffer + i) ){
			err = 1;
			break;
		}

	}

	if (err != 0){
		return XST_FAILURE;
	}

	DisableIntrSystem(&Intc, FIFO_INTR_ID);
	return Status;
}

/*****************************************************************************/
/**
*
* TxSend routine, It will send the requested amount of data at the
* specified addr.
*
* @param	InstancePtr is a pointer to the instance of the
*		XLlFifo component.
*
* @param	SourceAddr is the address of the memory
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
* @note		None
*
******************************************************************************/
int TxSend(XLlFifo *InstancePtr, u32  *SourceAddr)
{
	int i;
	int j;
	xil_printf("Transmitting Data ... \r\n");

	/* Fill the transmit buffer with incremental pattern */
	for (i=0; i < MAX_DATA_BUFFER_SIZE; i++)
		*(SourceAddr + i) = i;

	for(i=0; i < NO_OF_PACKETS; i++){
		/* Writing into the FIFO Transmit Port Buffer */
		for (j=0; j < MAX_PACKET_LEN; j++){
			if( XLlFifo_iTxVacancy(InstancePtr) ){
				XLlFifo_TxPutWord(InstancePtr,
					*(SourceAddr+(i*MAX_PACKET_LEN)+j));
			}
		}

	}

	/* Start Transmission by writing transmission length into the TLR */
	XLlFifo_iTxSetLen(InstancePtr, (MAX_DATA_BUFFER_SIZE * WORD_SIZE));


	/* Transmission Complete */
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the interrupt handler for the fifo it checks for the type of interrupt
* and proceeds according to it.
*
* @param	InstancePtr is a reference to the Fifo device instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoHandler(XLlFifo *InstancePtr)
{
	u32 Pending;

	Pending = XLlFifo_IntPending(InstancePtr);
	while (Pending) {
		if (Pending & XLLF_INT_RC_MASK) {
			FifoRecvHandler(InstancePtr);
			XLlFifo_IntClear(InstancePtr, XLLF_INT_RC_MASK);
		}
		else if (Pending & XLLF_INT_TC_MASK) {
			FifoSendHandler(InstancePtr);
		}
		else if (Pending & XLLF_INT_ERROR_MASK){
			FifoErrorHandler(InstancePtr, Pending);
			XLlFifo_IntClear(InstancePtr, XLLF_INT_ERROR_MASK);
		} else {
			XLlFifo_IntClear(InstancePtr, Pending);
		}
		Pending = XLlFifo_IntPending(InstancePtr);
	}
}

/*****************************************************************************/
/**
*
* This is the Receive handler callback function.
*
* @param	InstancePtr is a reference to the Fifo device instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoRecvHandler(XLlFifo *InstancePtr)
{
	int i;
	u32 RxWord;
	static u32 ReceiveLength;

	xil_printf("Receiving Data... \n\r");

	while(XLlFifo_iRxOccupancy(InstancePtr)) {
		/* Read Receive Length */
		ReceiveLength = (XLlFifo_iRxGetLen(InstancePtr))/WORD_SIZE;
		for (i=0; i < ReceiveLength; i++) {
				RxWord = XLlFifo_RxGetWord(InstancePtr);
				*(DestinationBuffer+i) = RxWord;
		}
	}
}

/*****************************************************************************/
/*
*
* This is the transfer Complete Interrupt handler function.
*
* This clears the transmit complete interrupt and set the done flag.
*
* @param	InstancePtr is a pointer to Instance of AXI FIFO device.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void FifoSendHandler(XLlFifo *InstancePtr)
{
	XLlFifo_IntClear(InstancePtr, XLLF_INT_TC_MASK);

	Done = 1;
}

/*****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments the
* the error counter so that the main thread knows the number of errors.
*
* @param	InstancePtr is a pointer to Instance of AXI FIFO device.
*
* @param	Pending is a bitmask of the pending interrupts.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void FifoErrorHandler(XLlFifo *InstancePtr, u32 Pending)
{
	if (Pending & XLLF_INT_RPURE_MASK) {
		XLlFifo_RxReset(InstancePtr);
	} else if (Pending & XLLF_INT_RPORE_MASK) {
		XLlFifo_RxReset(InstancePtr);
	} else if(Pending & XLLF_INT_RPUE_MASK) {
		XLlFifo_RxReset(InstancePtr);
	} else if (Pending & XLLF_INT_TPOE_MASK) {
		XLlFifo_TxReset(InstancePtr);
	} else if (Pending & XLLF_INT_TSE_MASK) {
	}
	Error++;
}

/****************************************************************************/
/**
*
* This function setups the interrupt system such that interrupts can occur
* for the FIFO device. This function is application specific since the
* actual system may or may not have an interrupt controller. The FIFO
* could be directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param    InstancePtr contains a pointer to the instance of the FIFO
*           component which is going to be connected to the interrupt
*           controller.
*
* @return   XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
int SetupInterruptSystem(INTC *IntcInstancePtr, XLlFifo *InstancePtr,
				u16 FifoIntrId)
{

	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr, FifoIntrId,
			   (XInterruptHandler)FifoHandler,
			   (void *)InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the FIFO can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the AXI FIFO device.
	 */
	XIntc_Enable(IntcInstancePtr, FifoIntrId);
#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, FifoIntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, FifoIntrId,
				(Xil_InterruptHandler)FifoHandler,
				InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, FifoIntrId);
#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)INTC_HANDLER,
		(void *)IntcInstancePtr);;

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
*
* This function disables the interrupts for the AXI FIFO device.
*
* @param	IntcInstancePtr is the pointer to the INTC component instance
* @param	FifoIntrId is interrupt ID associated for the FIFO component
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void DisableIntrSystem(INTC *IntcInstancePtr, u16 FifoIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts */
	XIntc_Disconnect(IntcInstancePtr, FifoIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, FifoIntrId);
#endif
}

#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void Uart550_Setup(void)
{

	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);
}
#endif
