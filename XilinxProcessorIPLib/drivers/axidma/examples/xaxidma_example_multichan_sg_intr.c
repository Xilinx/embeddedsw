/******************************************************************************
*
* Copyright (C) 2010 - 2018 Xilinx, Inc.  All rights reserved.
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
 * @file xaxidma_example_multichan_sg_intr.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core v6_00_a (AXIDMA) to transfer packets in interrupt mode for
 * Multiple Channel capability.
 * Thi example is designed to work only when AXIDMA core is configured
 * in Scatter Gather Mode and Multiple Channel mode.
 *
 * We show how to do multiple packets transfers, as well as how to do multiple
 * BDs per packet transfers.
 *
 * This code is tested only with two channels on both Tx and Rx.
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback. The loopback widget is configured in a way
 * that, when a packet is transmitted on "Tx Channel 0" it will be received
 * on the "Rx channel 1" and if "Tx Channel 1" it is on Rx Channel 0.
 * Both the cases are included in this example.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system. The
 * h/w system built in Area mode has a maximum DDR memory limit of 64MB. In
 * throughput mode, it is 512MB.  These limits are need to ensured for
 * proper operation of this code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a srt  03/27/12 First release
 * 2.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 2.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Added tabspace for return statements in functions
 *                     for proper documentation while generating doxygen.
 * 9.6   rsp  02/14/18 Support data buffers above 4GB. Use UINTPTR for storing
 *                     and typecasting buffer address(CR-992638).
 * </pre>
 *
 * ***************************************************************************
 */
/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xdebug.h"
#include "xaxidma_bd.h"

#ifdef __aarch64__
#include "xil_mmu.h"
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

#ifndef DEBUG
extern void xil_printf(const char *format, ...);
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/******************** Constant Definitions **********************************/
/*
 * Device hardware build related constants.
 */

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif XPAR_MIG_0_BASEADDR
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif XPAR_PSU_DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define RX_INTR_ID		XPAR_INTC_0_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_INTC_0_AXIDMA_0_MM2S_INTROUT_VEC_ID
#else
#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_S2MM_INTROUT_VEC_ID
#define TX_INTR_ID		XPAR_FABRIC_AXIDMA_0_MM2S_INTROUT_VEC_ID
#endif

#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x0000FFFF)
#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00040000)
#define TX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x0004FFFF)
#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif

/* Timeout loop counter for reset
 */
#define RESET_TIMEOUT_COUNTER	10000

/*
 * Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 * The receive side gets one completion per transfer.
 */
#define NUMBER_OF_BDS_PER_PKT		2
#define NUMBER_OF_PKTS_TO_TRANSFER 	1
#define NUMBER_OF_BDS_TO_TRANSFER	(NUMBER_OF_PKTS_TO_TRANSFER * \
						NUMBER_OF_BDS_PER_PKT)
#define MAX_BD_COUNT		1024
#define MARK_UNCACHEABLE        0x701

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1
 *
 * Note: Coalescing threshold configured for more than one is not
 * 	 supported by the AXIDMA IP v6_00_a.
 */
#define COALESCING_COUNT		1
#define DELAY_TIMER_COUNT		100

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC		XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC		XScuGic
 #define INTC_HANDLER	XScuGic_InterruptHandler
#endif

/*
 * Packet Data Values
 */
#define PACKET0_DATA	0x1
#define PACKET1_DATA	0xC

/*
 * MCDMA Values
 */
#define TID0	0x0		/* Stream identifier 0 */
#define TDEST0 	0x0		/* Coarse Routing info for stream 0 */
#define TID1 	0x1		/* Stream identifier 1 */
#define TDEST1 	0x1		/* Coarse Routing info for stream 1 */
#define TUSER	0x0		/* User defined sideband signaling */
#define ARCACHE 0x3		/* Cache type */
#define ARUSER 	0x0		/* Sideband signals */
#define VSIZE	0x4		/* Vsize */
#define STRIDE	0x20		/* Stride control */
#define HSIZE   0x100  		/* Hsize */

/*
 * Buffer and Buffer Descriptor related constant definition
 */
#define MAX_PKT_LEN		VSIZE * STRIDE

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

static int CheckData(int Length, u8 *RxPacket, u8 StartValue);
static void TxCallBack(XAxiDma * AxiDmaInstPtr);
static void TxIntrHandler(void *Callback);
static void RxCallBack(XAxiDma * AxiDmaInstPtr);
static void RxIntrHandler(void *Callback);
extern void XaxiDma_DumpBd(XAxiDma_Bd *BdPtr);

static int SetupIntrSystem(INTC * IntcInstancePtr,
			   XAxiDma * AxiDmaPtr, u16 TxIntrId,
			   u16 RxIntrId, int RingIndex);
static void DisableIntrSystem(INTC * IntcInstancePtr,
					u16 TxIntrId, u16 RxIntrId);

static int RxSetup(XAxiDma * AxiDmaInstPtr);
static int TxSetup(XAxiDma * AxiDmaInstPtr);
static int SendPacket(XAxiDma * AxiDmaInstPtr,
		u8 TDest, u8 TId, u8 Value);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;


static INTC Intc;	/* Instance of the Interrupt Controller */

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile int TxDone;
volatile int RxDone;
volatile int Error;

/*
 * Buffer for transmit packet. Must be 32-bit aligned to be used by DMA.
 */
u32 *Packet = (u32 *) TX_BUFFER_BASE;

/*
 * Buffer for Receive packet. Must be 32-bit aligned to be used by DMA.
 */
u8 *RxPacket0 = (u8 *) RX_BUFFER_BASE;
u8 *RxPacket1 = (u8 *) (RX_BUFFER_BASE + MAX_BD_COUNT
				* MAX_PKT_LEN);

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the interrupt test. It does the following:
*	- Set up the output terminal if UART16550 is in the hardware build
*	- Initialize the DMA engine
*	- Set up Tx and Rx channels
*	- Set up the interrupt system for the Tx and Rx interrupts
*	- Submit a transfer
*	- Wait for the transfer to finish
*	- Check transfer status
*	- Disable Tx and Rx interrupts
*	- Print test status and exit
*
* @param	None
*
* @return
*		- XST_SUCCESS if tests pass
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	XAxiDma_Config *Config;

	/* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif

	xil_printf("\r\n--- Entering main() --- \r\n");
#ifdef __aarch64__
	Xil_SetTlbAttributes(TX_BD_SPACE_BASE, MARK_UNCACHEABLE);
	Xil_SetTlbAttributes(RX_BD_SPACE_BASE, MARK_UNCACHEABLE);
#endif

	Config = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);

		return XST_FAILURE;
	}

	/* Initialize DMA engine */
	XAxiDma_CfgInitialize(&AxiDma, Config);

	if(!XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as Simple mode \r\n");
		return XST_FAILURE;
	}

	/* Set up TX/RX channels to be ready to transmit and receive packets */
	Status = TxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed TX setup\r\n");
		return XST_FAILURE;
	}

	Status = RxSetup(&AxiDma);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed RX setup\r\n");
		return XST_FAILURE;
	}

	/* Set up Interrupt system  */
	Status = SetupIntrSystem(&Intc, &AxiDma, TX_INTR_ID,
					RX_INTR_ID, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed intr setup\r\n");
		return XST_FAILURE;
	}

	/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error = 0;

	/* Send a packet */
	Status = SendPacket(&AxiDma, TDEST0, TID0, PACKET0_DATA);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	/*
	 * Wait TX done and RX done
	 */
	while (((TxDone < NUMBER_OF_BDS_TO_TRANSFER) ||
			(RxDone < NUMBER_OF_BDS_TO_TRANSFER)) && !Error) {
		/* NOP */
	}

	if (Error) {
		xil_printf("Failed test transmit%s done, "
			"receive%s done\r\n", TxDone? "":" not",
				RxDone? "":" not");
		goto Done;
	}else {
		/*
		 * Test finished, check data
		 */
		Status = CheckData(MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER,
							RxPacket1, PACKET0_DATA);
		if (Status != XST_SUCCESS) {
			xil_printf("Data check failed\r\n");
			goto Done;
		}
	}

	xil_printf("Sent Packet with Tdest 0 Successfully\n\r");

	/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error = 0;

	/* Send a packet */
	Status = SendPacket(&AxiDma, TDEST1, TID1, PACKET1_DATA);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed send packet\r\n");
		return XST_FAILURE;
	}

	/*
	 * Wait TX done and RX done
	 */
	while (((TxDone < NUMBER_OF_BDS_TO_TRANSFER) ||
		(RxDone < NUMBER_OF_BDS_TO_TRANSFER)) && !Error) {
		/* NOP */
	}


	if (Error) {
		xil_printf("Failed test transmit%s done, "
				"receive%s done\r\n", TxDone? "":" not",
				RxDone? "":" not");
		goto Done;
	}else {
		/*
		 * Test finished, check data
		 */
		Status = CheckData(MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER,
					RxPacket0, PACKET1_DATA);
		if (Status != XST_SUCCESS) {
			xil_printf("Data check failed\r\n");
			goto Done;
		}
	}

	/* Disable TX and RX Ring interrupts and return success */
	DisableIntrSystem(&Intc, TX_INTR_ID, RX_INTR_ID);

	xil_printf("Sent Packet with Tdest 1 Successfully\n\r");

	xil_printf("Successfully ran AXI DMA SG interrupt Example\r\n");

Done:

	xil_printf("--- Exiting main() --- \r\n");

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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
* @note		None.
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

/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* We use the static tx/rx buffers.
*
* @param	Length is the length to check
* @param	StartValue is the starting value of the first byte
*
* @return	- XST_SUCCESS if validation is successful
*		- XST_FAILURE if validation fails.
*
* @note		None.
*
******************************************************************************/
static int CheckData(int Length, u8 *RxPacket, u8 StartValue)
{
	int Index = 0;
	u8 Value;

	Value = StartValue;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
#ifndef __aarch64__
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, Length);
#endif

	for(Index = 0; Index < Length; Index++) {
		if (RxPacket[Index] != Value) {
			xil_printf("Data error %d: %x/%x\r\n",
			    Index, RxPacket[Index], Value);

			return XST_FAILURE;
		}
		Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This is the DMA TX callback function to be called by TX interrupt handler.
* This function handles BDs finished by hardware.
*
* @param	TxRingPtr is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxCallBack(XAxiDma *AxiDmaPtr)
{
	int BdCount;
	u32 BdSts;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	int Status;
	int Index;
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(AxiDmaPtr);

	/* Get all processed BDs from hardware */
	BdCount = XAxiDma_BdRingFromHw(TxRingPtr, XAXIDMA_ALL_BDS, &BdPtr);

	/* Handle the BDs */
	BdCurPtr = BdPtr;
	for (Index = 0; Index < BdCount; Index++) {

		/*
		 * Check the status in each BD
		 * If error happens, the DMA engine will be halted after this
		 * BD processing stops.
		 */
		BdSts = XAxiDma_BdGetSts(BdCurPtr);
		if ((BdSts & XAXIDMA_BD_STS_ALL_ERR_MASK) ||
		    (!(BdSts & XAXIDMA_BD_STS_COMPLETE_MASK))) {

			Error = 1;
			break;
		}

		/*
		 * Here we don't need to do anything. But if a RTOS is being
		 * used, we may need to free the packet buffer attached to
		 * the processed BD
		 */

		/* Find the next processed BD */
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(TxRingPtr, BdCurPtr);
	}

	/* Free all processed BDs for future transmission */
	Status = XAxiDma_BdRingFree(TxRingPtr, BdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		Error = 1;
	}

	if(!Error) {
		TxDone += BdCount;
	}
}

/*****************************************************************************/
/*
*
* This is the DMA TX Interrupt handler function.
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* presents, then it calls the callback function.
*
* @param	Callback is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxIntrHandler(void *Callback)
{
	XAxiDma *AxiDmaPtr = (XAxiDma *) Callback;
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(AxiDmaPtr);
	u32 IrqStatus;
	int TimeOut;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_BdRingGetIrq(TxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(TxRingPtr, IrqStatus);

	/* If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {

		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/*
		 * Reset should never fail for transmit channel
		 */
		XAxiDma_Reset(&AxiDma);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(&AxiDma)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If Transmit done interrupt is asserted, call TX call back function
	 * to handle the processed BDs and raise the according flag
	 */
	if ((IrqStatus & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		TxCallBack(AxiDmaPtr);
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX callback function called by the RX interrupt handler.
* This function handles finished BDs by hardware, attaches new buffers to those
* BDs, and give them back to hardware to receive more incoming packets
*
* @param	RxRingPtr is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxCallBack(XAxiDma *AxiDmaPtr)
{
	int BdCount;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdSts;
	int Index;
	int RingIndex;
	XAxiDma_BdRing *RxRingPtr;

	for (RingIndex = 0;
			RingIndex < AxiDmaPtr->RxNumChannels; RingIndex++) {

		RxRingPtr = XAxiDma_GetRxIndexRing(AxiDmaPtr, RingIndex);
		BdCount = XAxiDma_BdRingFromHw(RxRingPtr, XAXIDMA_ALL_BDS, &BdPtr);

		BdCurPtr = BdPtr;
		for (Index = 0; Index < BdCount; Index++) {

			/*
			 * Check the flags set by the hardware for status
			 * If error happens, processing stops, because the DMA engine
			 * is halted after this BD.
			 */
			BdSts = XAxiDma_BdGetSts(BdCurPtr);
			if ((BdSts & XAXIDMA_BD_STS_ALL_ERR_MASK) ||
			    (!(BdSts & XAXIDMA_BD_STS_COMPLETE_MASK))) {
				Error = 1;
				break;
			}

			/* Find the next processed BD */
			BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
			RxDone += 1;
		}
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* presents, then it calls the callback function.
*
* @param	Callback is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(void *Callback)
{
	XAxiDma *AxiDmaPtr = (XAxiDma *) Callback;

	XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxIndexRing(AxiDmaPtr, 0);

	u32 IrqStatus;
	int TimeOut;


	/* Read pending interrupts */
	IrqStatus = XAxiDma_BdRingGetIrq(RxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(RxRingPtr, IrqStatus);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(&AxiDma);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if(XAxiDma_ResetIsDone(&AxiDma)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If completion interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((IrqStatus & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		RxCallBack(AxiDmaPtr);
	}
}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param	IntcInstancePtr is a pointer to the instance of the INTC.
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	TxIntrId is the TX channel Interrupt ID.
* @param	RxIntrId is the RX channel Interrupt ID.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE.if not succesful
*
* @note		None.
*
******************************************************************************/

static int SetupIntrSystem(INTC * IntcInstancePtr,
			   XAxiDma * AxiDmaPtr, u16 TxIntrId,
			   u16 RxIntrId, int RingIndex)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID

	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, TxIntrId,
			       (XInterruptHandler) TxIntrHandler, AxiDmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed tx connect intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, RxIntrId,
			       (XInterruptHandler) RxIntrHandler, AxiDmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed rx connect intc\r\n");
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed to start intc\r\n");
		return XST_FAILURE;
	}

	XIntc_Enable(IntcInstancePtr, TxIntrId);
	XIntc_Enable(IntcInstancePtr, RxIntrId);

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


	XScuGic_SetPriorityTriggerType(IntcInstancePtr, TxIntrId, 0xA0, 0x3);

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, RxIntrId, 0xA0, 0x3);
	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, TxIntrId,
				(Xil_InterruptHandler)TxIntrHandler,
				AxiDmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, RxIntrId,
				(Xil_InterruptHandler)RxIntrHandler,
				AxiDmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	XScuGic_Enable(IntcInstancePtr, TxIntrId);
	XScuGic_Enable(IntcInstancePtr, RxIntrId);
#endif

	/* Enable interrupts from the hardware */

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)INTC_HANDLER,
			(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts for DMA engine.
*
* @param	IntcInstancePtr is the pointer to the INTC component instance
* @param	TxIntrId is interrupt ID associated w/ DMA TX channel
* @param	RxIntrId is interrupt ID associated w/ DMA RX channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(INTC * IntcInstancePtr,
					u16 TxIntrId, u16 RxIntrId)
{
#ifdef XPAR_INTC_0_DEVICE_ID
	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(IntcInstancePtr, TxIntrId);
	XIntc_Disconnect(IntcInstancePtr, RxIntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, TxIntrId);
	XScuGic_Disconnect(IntcInstancePtr, RxIntrId);
#endif
}

/*****************************************************************************/
/*
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	- XST_SUCCESS if the setup is successful.
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	int BdCount;
	int FreeBdCount;
	UINTPTR RxBufferPtr;
	UINTPTR RxBdSpacePtr;
	int Index;
	int RingIndex;

	RxBufferPtr = RX_BUFFER_BASE;
	RxBdSpacePtr = RX_BD_SPACE_BASE;

	for (RingIndex = 0;
			RingIndex < AxiDmaInstPtr->RxNumChannels; RingIndex++) {

		RxRingPtr = XAxiDma_GetRxIndexRing(&AxiDma, RingIndex);

		/* Disable all RX interrupts before RxBD space setup */
		XAxiDma_BdRingIntDisable(RxRingPtr,
						XAXIDMA_IRQ_ALL_MASK);

		/* Setup Rx BD space */
		BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
				RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

		Status = XAxiDma_BdRingCreate(RxRingPtr,
					RxBdSpacePtr,
					RxBdSpacePtr,
					XAXIDMA_BD_MINIMUM_ALIGNMENT,
					BdCount);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd create failed with %d\r\n",
				Status);
			return XST_FAILURE;
		}

		/*
		 * Setup a BD template for the Rx channel. Then copy it
		 * to every RX BD.
		 */
		XAxiDma_BdClear(&BdTemplate);
		Status = XAxiDma_BdRingClone(RxRingPtr,
						 &BdTemplate);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd clone failed with %d\r\n",
				Status);
			return XST_FAILURE;
		}

		/* Attach buffers to RxBD ring so we are ready to receive packets */
		FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);

		Status = XAxiDma_BdRingAlloc(RxRingPtr,
					FreeBdCount, &BdPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx bd alloc failed with %d\r\n",
				Status);
			return XST_FAILURE;
		}

		BdCurPtr = BdPtr;

		for (Index = 0; Index < FreeBdCount; Index++) {

			Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
			if (Status != XST_SUCCESS) {
				xil_printf("Rx set buffer addr %x on BD %x failed %d\r\n",
				(unsigned int)RxBufferPtr,
				(UINTPTR)BdCurPtr, Status);

				return XST_FAILURE;
			}

			Status = XAxiDma_BdSetLength(BdCurPtr, HSIZE,
						RxRingPtr->MaxTransferLen);
			if (Status != XST_SUCCESS) {
				xil_printf("Rx set length %d on BD %x failed %d\r\n",
				MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

				return XST_FAILURE;
			}

			/* Receive BDs do not need to set anything for the control
			 * The hardware will set the SOF/EOF bits per stream status
			 */
			XAxiDma_BdSetCtrl(BdCurPtr, 0);
			XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);
			XAxiDma_BdSetARCache(BdCurPtr, ARCACHE);
			XAxiDma_BdSetARUser(BdCurPtr, ARUSER);
			XAxiDma_BdSetVSize(BdCurPtr, VSIZE);
			XAxiDma_BdSetStride(BdCurPtr, STRIDE);

			RxBufferPtr += MAX_PKT_LEN;
			BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
		}

		/*
		 * Set the coalescing threshold, so only one receive interrupt
		 * occurs for this example
		 *
		 * If you would like to have multiple interrupts to happen, change
		 * the COALESCING_COUNT to be a smaller value
		 */
		Status = XAxiDma_BdRingSetCoalesce(RxRingPtr, COALESCING_COUNT,
				DELAY_TIMER_COUNT);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set coalesce failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx ToHw failed with %d\r\n", Status);
			return XST_FAILURE;
		}

		/* Enable all RX interrupts */
		XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

		/* Start RX DMA channel */
		Status = XAxiDma_UpdateBdRingCDesc(RxRingPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Failed bd start %x\r\n", Status);
			return XST_FAILURE;
		}

		RxBdSpacePtr += BdCount * sizeof(XAxiDma_Bd);

	}

	for (RingIndex = 0;
			RingIndex < AxiDmaInstPtr->RxNumChannels; RingIndex++) {
		RxRingPtr = XAxiDma_GetRxIndexRing(&AxiDma, RingIndex);
		Status = XAxiDma_StartBdRingHw(RxRingPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx start BD ring failed with %d\r\n", Status);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function sets up the TX channel of a DMA engine to be ready for packet
* transmission.
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	- XST_SUCCESS if the setup is successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int TxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(&AxiDma);
	XAxiDma_Bd BdTemplate;
	int Status;
	u32 BdCount;

	UINTPTR TxBdSpacePtr = TX_BD_SPACE_BASE;

	/* Disable all TX interrupts before TxBD space setup */
	XAxiDma_BdRingIntDisable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Setup TxBD space  */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
			(UINTPTR)TX_BD_SPACE_HIGH - (UINTPTR)TX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(TxRingPtr, TxBdSpacePtr,
				TxBdSpacePtr,
			     XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed create BD ring\r\n");
		return XST_FAILURE;
	}

	/*
	 * Like the RxBD space, we create a template and set all BDs to be the
	 * same as the template. The sender has to set up the BDs as needed.
	 */
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(TxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed clone BDs\r\n");
		return XST_FAILURE;
	}

	/*
	 * Set the coalescing threshold, so only one transmit interrupt
	 * occurs for this example
	 *
	 * If you would like to have multiple interrupts to happen, change
	 * the COALESCING_COUNT to be a smaller value
	 */
	Status = XAxiDma_BdRingSetCoalesce(TxRingPtr, COALESCING_COUNT,
			DELAY_TIMER_COUNT);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed set coalescing"
		" %d/%d\r\n",COALESCING_COUNT, DELAY_TIMER_COUNT);
		return XST_FAILURE;
	}

	/* Enable all TX interrupts */
	XAxiDma_BdRingIntEnable(TxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Start the TX channel */
	Status = XAxiDma_UpdateBdRingCDesc(TxRingPtr);
		if (Status != XST_SUCCESS) {

			xil_printf("Failed bd start %x\r\n", Status);
			return XST_FAILURE;
	}

	Status = XAxiDma_StartBdRingHw(TxRingPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed bd start %x\r\n", Status);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function non-blockingly transmits all packets through the DMA engine.
*
* @param	AxiDmaInstPtr points to the DMA engine instance
*
* @return
* 		- XST_SUCCESS if the DMA accepts all the packets successfully,
* 		- XST_FAILURE if error occurs
*
* @note		None.
*
******************************************************************************/
static int SendPacket(XAxiDma * AxiDmaInstPtr, u8 TDest, u8 TId, u8 Value)
{
	XAxiDma_BdRing *TxRingPtr = XAxiDma_GetTxRing(AxiDmaInstPtr);
	u8 *TxPacket;
	XAxiDma_Bd *BdPtr, *BdCurPtr;
	int Status;
	int Index, Pkts;
	UINTPTR BufferAddr;

	/*
	 * Each packet is limited to TxRingPtr->MaxTransferLen
	 *
	 * This will not be the case if hardware has store and forward built in
	 */
	if (MAX_PKT_LEN * NUMBER_OF_BDS_PER_PKT > TxRingPtr->MaxTransferLen) {

		xil_printf("Invalid total per packet transfer length for the "
		    "packet %d/%d\r\n",
		    MAX_PKT_LEN * NUMBER_OF_BDS_PER_PKT,
		    TxRingPtr->MaxTransferLen);

		return XST_INVALID_PARAM;
	}

	TxPacket = (u8 *) Packet;

	for(Index = 0; Index < MAX_PKT_LEN * NUMBER_OF_BDS_TO_TRANSFER;
					Index ++) {
		TxPacket[Index] = Value;

		Value = (Value + 1) & 0xFF;
	}

	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxPacket, MAX_PKT_LEN *
				NUMBER_OF_BDS_TO_TRANSFER);
#ifdef __aarch64__
	Xil_DCacheFlushRange((UINTPTR)RX_BUFFER_BASE, MAX_PKT_LEN *
			      NUMBER_OF_BDS_TO_TRANSFER);
#endif

	Status = XAxiDma_BdRingAlloc(TxRingPtr,
				NUMBER_OF_BDS_TO_TRANSFER,
				&BdPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed bd alloc\r\n");
		return XST_FAILURE;
	}

	BufferAddr = (UINTPTR) TxPacket;
	BdCurPtr = BdPtr;

	/*
	 * Set up the BD using the information of the packet to transmit
	 * Each transfer has NUMBER_OF_BDS_PER_PKT BDs
	 */
	for(Index = 0; Index < NUMBER_OF_PKTS_TO_TRANSFER; Index++) {

		for(Pkts = 0; Pkts < NUMBER_OF_BDS_PER_PKT; Pkts++) {
			u32 CrBits = 0;

			Status = XAxiDma_BdSetBufAddr(BdCurPtr, BufferAddr);
			if (Status != XST_SUCCESS) {
				xil_printf("Tx set buffer addr %x on BD %x failed %d\r\n",
				(unsigned int)BufferAddr,
				(UINTPTR)BdCurPtr, Status);

				return XST_FAILURE;
			}

			Status = XAxiDma_BdSetLength(BdCurPtr, HSIZE,
						TxRingPtr->MaxTransferLen);
			if (Status != XST_SUCCESS) {
				xil_printf("Tx set length %d on BD %x failed %d\r\n",
				MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

				return XST_FAILURE;
			}

			if (Pkts == 0) {
				/* The first BD has SOF set
				 */
				CrBits |= XAXIDMA_BD_CTRL_TXSOF_MASK;

#if (XPAR_AXIDMA_0_SG_INCLUDE_STSCNTRL_STRM == 1)
				/* The first BD has total transfer length set
				 * in the last APP word, this is for the
				 * loopback widget
				 */
				Status = XAxiDma_BdSetAppWord(BdCurPtr,
				    XAXIDMA_LAST_APPWORD,
				    MAX_PKT_LEN * NUMBER_OF_BDS_PER_PKT);

				if (Status != XST_SUCCESS) {
					xil_printf("Set app word failed with %d\r\n",
					Status);
				}
#endif
			}

			if(Pkts == (NUMBER_OF_BDS_PER_PKT - 1)) {
				/* The last BD should have EOF and IOC set
				 */
				CrBits |= XAXIDMA_BD_CTRL_TXEOF_MASK;
			}

			XAxiDma_BdSetCtrl(BdCurPtr, CrBits);
			XAxiDma_BdSetId(BdCurPtr, BufferAddr);
			XAxiDma_BdSetTId(BdCurPtr, TId);
			XAxiDma_BdSetTDest(BdCurPtr, TDest);
			XAxiDma_BdSetTUser(BdCurPtr, TUSER);
			XAxiDma_BdSetARCache(BdCurPtr, ARCACHE);
			XAxiDma_BdSetARUser(BdCurPtr, ARUSER);
			XAxiDma_BdSetVSize(BdCurPtr, VSIZE);
			XAxiDma_BdSetStride(BdCurPtr, STRIDE);

			BufferAddr += MAX_PKT_LEN;
			BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(TxRingPtr, BdCurPtr);
		}
	}

	/* Give the BD to hardware */
	Status = XAxiDma_BdRingToHw(TxRingPtr, NUMBER_OF_BDS_TO_TRANSFER,
					BdPtr);
	if (Status != XST_SUCCESS) {

		xil_printf("Failed to hw, length %d\r\n",
			(int)XAxiDma_BdGetLength(BdPtr,
					TxRingPtr->MaxTransferLen));

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
