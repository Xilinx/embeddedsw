/******************************************************************************
* Copyright (C) 2012 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxivdma_example_intr.c
 *
 * This example demonstrates how to use the AXI Video DMA in loopback mode
 * to do video frame transfers. This example reads video frames from memory,
 * using Memory Map to Stream (MM2S) interface, and then video frames are
 * written to memory using  Stream to Memory Map (S2MM) AXI4 interface.
 * At the end of transfer it does sanity check and report pass/fail status.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * @note
 * The values of DDR_BASE_ADDR and DDR_HIGH_ADDR should be as per the HW system.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a jz   07/26/10 First release
 * 1.01a jz   09/26/10 Updated callback function signature
 * 2.00a jz   12/10/10 Added support for direct register access mode, v3 core
 * 2.01a rvp  01/22/11 Renamed the example file to be consistent
 * 		       Added support to the example to use SCU GIC interrupt
 *		       controller for ARM, some functions in this example have
 *		       changed.
 *       rkv  03/28/11 Updated to support for frame store register.
 * 3.00a srt  08/26/11 Added support for Flush on Frame Sync Feature.
 * 4.00a srt  03/06/12 Modified interrupt support for Zynq.
 * 4.02a srt  09/25/12 Fixed CR 677704
 *		       Description - Arguments misused in function
 *                     XAxiVdma_IntrEnable().
 * 4.03a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 6.2   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * 6.5   rsp  12/01/17 Set TX/RX framebuffer count to IP default. CR-990409
 * 6.6   rsp  07/02/18 Set Vertical Flip state to IP default. CR-989453
 * 6.7   sk   05/06/20 Fix optimization level 2 failure in release mode.
 * 6.8   sk   07/07/20 Add frame data check support
 * 6.9	 sk   05/25/21 Modify the ReadSetup buffer initialization call and
 *		       CheckFrame to correct the example logic.
 * 6.9	 sk   05/25/21 Fix data comparison failure wtih optimization level 2.
 * 6.10  rsp  09/09/21 Fix read/write done count check in while loop.
 *                     Remove unused variable GCC warning in ReadSetup().
 * 6.11  rsp  03/16/21 After Wr/Rd channel reset ensure it's completed
 *                     and then do data comparison.
 * 6.12  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * 6.13	 sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 * ***************************************************************************
 */

#include "xaxivdma.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xil_util.h"

#include "xil_cache.h"
#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#else
#include "xscugic.h"
#endif

#ifndef __MICROBLAZE__
#include "xpseudo_asm_gcc.h"
#endif

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/******************** Constant Definitions **********************************/

/*
 * Device related constants. These need to defined as per the HW system.
 */
#define DMA_DEVICE_ID		XPAR_AXIVDMA_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define WRITE_INTR_ID		XPAR_INTC_0_AXIVDMA_0_S2MM_INTROUT_VEC_ID
#define READ_INTR_ID		XPAR_INTC_0_AXIVDMA_0_MM2S_INTROUT_VEC_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define WRITE_INTR_ID		XPAR_FABRIC_AXIVDMA_0_S2MM_INTROUT_VEC_ID
#define READ_INTR_ID		XPAR_FABRIC_AXIVDMA_0_MM2S_INTROUT_VEC_ID
#endif

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_HIGH_ADDR		XPAR_AXI_7SDDR_0_S_AXI_HIGHADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR		XPAR_MIG7SERIES_0_BASEADDR
#define DDR_HIGH_ADDR	 	XPAR_MIG7SERIES_0_HIGHADDR
#elif XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define DDR_BASE_ADDR		XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#define DDR_HIGH_ADDR	 	XPAR_MIG_0_C0_DDR4_MEMORY_MAP_HIGHADDR
#else
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
			DEFAULT SET TO 0x01000000
#define DDR_BASE_ADDR		0x10000000
#define DDR_HIGH_ADDR		0x20000000
#endif

/* Memory space for the frame buffers
 *
 * This example only needs one set of frame buffers, because one video IP is
 * to write to the frame buffers, and the other video IP is to read from the
 * frame buffers.
 *
 * For 16 frames of 1080p, it needs 0x07E90000 memory for frame buffers
 */
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x01000000)
#define MEM_HIGH_ADDR		DDR_HIGH_ADDR
#define MEM_SPACE		(MEM_HIGH_ADDR - MEM_BASE_ADDR)

/* Read channel and write channel addresses
 */
#define READ_ADDRESS_BASE	MEM_BASE_ADDR
#define WRITE_ADDRESS_BASE	(MEM_BASE_ADDR + 0x02000000)

/* Frame size related constants
 */
#define FRAME_HORIZONTAL_LEN  0x1E00   /* 1920 pixels, each pixel 4 bytes */
#define FRAME_VERTICAL_LEN    0x438    /* 1080 pixels */

/* Subframe to be transferred by Video DMA
 *
 *|<----------------- FRAME_HORIZONTAL_LEN ---------------------->|
 * --------------------------------------------------------------------
 *|                                                                | ^
 *|                                                                | |
 *|               |<-SUBFRAME_HORIZONTAL_SIZE ->|                  | FRAME_
 *|               -----------------------------------              | VERTICAL_
 *|               |/////////////////////////////|  ^               | LEN
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////| SUBFRAME_        | |
 *|               |/////////////////////////////| VERTICAL_        | |
 *|               |/////////////////////////////| SIZE             | |
 *|               |/////////////////////////////|  |               | |
 *|               |/////////////////////////////|  v               | |
 *|                ----------------------------------              | |
 *|                                                                | v
 *--------------------------------------------------------------------
 *
 * Note that SUBFRAME_HORIZONTAL_SIZE and SUBFRAME_VERTICAL_SIZE must ensure
 * to be inside the frame.
 */
#define SUBFRAME_START_OFFSET    (FRAME_HORIZONTAL_LEN * 5 + 64)
#define SUBFRAME_HORIZONTAL_SIZE 0x100
#define SUBFRAME_VERTICAL_SIZE   0x100

/* Number of frames to transfer
 *
 * This is used to monitor the progress of the test, test purpose only
 */
#define NUM_TEST_FRAME_SETS	10

#define TEST_START_VALUE        0xC

/* Delay timer counter
 *
 * WARNING: If you are using fsync, please increase the delay counter value
 * to be 255. Because with fsync, the inter-frame delay is long. If you do not
 * care about inactivity of the hardware, set this counter to be 0, which
 * disables delay interrupt.
 */
#define DELAY_TIMER_COUNTER	10

/* Default reset timeout
 */
#define XAXIVDMA_RESET_TIMEOUT_USEC	500
#define POLL_TIMEOUT_COUNTER            1100000U
#define NUMBER_OF_EVENTS		1
/*
 * Device instance definitions
 */
XAxiVdma AxiVdma;

#ifdef XPAR_INTC_0_DEVICE_ID
static XIntc Intc;	/* Instance of the Interrupt Controller */
#else
static XScuGic Intc;	/* Instance of the Interrupt Controller */
#endif

/* Data address
 *
 * Read and write sub-frame use the same settings
 */
static UINTPTR ReadFrameAddr;
static UINTPTR WriteFrameAddr;
static UINTPTR BlockStartOffset;
static UINTPTR BlockHoriz;
static UINTPTR BlockVert;

/* Frame-buffer count i.e Number of frames to work on
 */
volatile static u16 ReadCount;
volatile static u16 WriteCount;

/* DMA channel setup
 */
static XAxiVdma_DmaSetup ReadCfg;
static XAxiVdma_DmaSetup WriteCfg;

/* Transfer statics
 */
volatile static u32 ReadDone;
volatile static u32 ReadError;
volatile static u32 WriteDone;
volatile static u32 WriteError;

/******************* Function Prototypes ************************************/



static int ReadSetup(XAxiVdma *InstancePtr);
static int WriteSetup(XAxiVdma * InstancePtr);
static int StartTransfer(XAxiVdma *InstancePtr);
static int CheckFrame(int FrameIndex);
static void BufferInit(UINTPTR BaseAddr, int Length, u8 StartValue);

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId,
				u16 WriteIntrId);

static void DisableIntrSystem(u16 ReadIntrId, u16 WriteIntrId);

/* Interrupt call back functions
 */
static void ReadCallBack(void *CallbackRef, u32 Mask);
static void ReadErrorCallBack(void *CallbackRef, u32 Mask);
static void WriteCallBack(void *CallbackRef, u32 Mask);
static void WriteErrorCallBack(void *CallbackRef, u32 Mask);


#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
*
******************************************************************************/
static void Uart550_Setup(void)
{

	/* Set the baudrate to be predictable
	 */
	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);

}
#endif

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry point of the example on DMA core. It sets up
* DMA engine to be ready to receive and send frames, and start the transfers.
* It waits for the transfer of the specified number of frame sets, and check
* for transfer errors.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status,Index;
	XAxiVdma_Config *Config;
	XAxiVdma_FrameCounter FrameCfg;
	int Polls;

#if defined(XPAR_UARTNS550_0_BASEADDR)
	Uart550_Setup();
#endif

	WriteDone = 0;
	ReadDone = 0;
	WriteError = 0;
	ReadError = 0;

	ReadFrameAddr = READ_ADDRESS_BASE;
	WriteFrameAddr = WRITE_ADDRESS_BASE;
	BlockStartOffset = SUBFRAME_START_OFFSET;
	BlockHoriz = SUBFRAME_HORIZONTAL_SIZE;
	BlockVert = SUBFRAME_VERTICAL_SIZE;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* The information of the XAxiVdma_Config comes from hardware build.
	 * The user IP should pass this information to the AXI DMA core.
	 */
	Config = XAxiVdma_LookupConfig(DMA_DEVICE_ID);
	if (!Config) {
		xil_printf(
		    "No video DMA found for ID %d\r\n", DMA_DEVICE_ID);

		return XST_FAILURE;
	}

	/* Set default read and write count based on HW config*/
	ReadCount = Config->MaxFrameStoreNum;
	WriteCount = Config->MaxFrameStoreNum;

	/* Initialize DMA engine */
	Status = XAxiVdma_CfgInitialize(&AxiVdma, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Configuration Initialization failed %d\r\n", Status);

		return XST_FAILURE;
	}

	Status = XAxiVdma_SetFrmStore(&AxiVdma, ReadCount,
							XAXIVDMA_READ);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Setting Frame Store Number Failed in Read Channel"
							" %d\r\n", Status);

		return XST_FAILURE;
	}

	Status = XAxiVdma_SetFrmStore(&AxiVdma, WriteCount,
							XAXIVDMA_WRITE);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Setting Frame Store Number Failed in Write Channel"
							" %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Setup frame counter and delay counter for both channels
	 *
	 * This is to monitor the progress of the test only
	 *
	 * WARNING: In free-run mode, interrupts may overwhelm the system.
	 * In that case, it is better to disable interrupts.
	 */
	FrameCfg.ReadFrameCount = ReadCount;
	FrameCfg.WriteFrameCount = WriteCount;
	FrameCfg.ReadDelayTimerCount = DELAY_TIMER_COUNTER;
	FrameCfg.WriteDelayTimerCount = DELAY_TIMER_COUNTER;

	Status = XAxiVdma_SetFrameCounter(&AxiVdma, &FrameCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Set frame counter failed %d\r\n", Status);

		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");

		return XST_FAILURE;
	}

	/*
	 * Setup your video IP that writes to the memory
	 */


	/* Setup the write channel
	 */
	Status = WriteSetup(&AxiVdma);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Write channel setup failed %d\r\n", Status);
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");

		return XST_FAILURE;
	}


	/*
	 * Setup your video IP that reads from the memory
	 */

	/* Setup the read channel
	 */
	Status = ReadSetup(&AxiVdma);
	if (Status != XST_SUCCESS) {
		xil_printf(
			"Read channel setup failed %d\r\n", Status);
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");

		return XST_FAILURE;
	}

	Status = SetupIntrSystem(&AxiVdma, READ_INTR_ID, WRITE_INTR_ID);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Setup interrupt system failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Register callback functions
	 */
	XAxiVdma_SetCallBack(&AxiVdma, XAXIVDMA_HANDLER_GENERAL, ReadCallBack,
	    (void *)&AxiVdma, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(&AxiVdma, XAXIVDMA_HANDLER_ERROR,
	    ReadErrorCallBack, (void *)&AxiVdma, XAXIVDMA_READ);

	XAxiVdma_SetCallBack(&AxiVdma, XAXIVDMA_HANDLER_GENERAL,
	    WriteCallBack, (void *)&AxiVdma, XAXIVDMA_WRITE);

	XAxiVdma_SetCallBack(&AxiVdma, XAXIVDMA_HANDLER_ERROR,
	    WriteErrorCallBack, (void *)&AxiVdma, XAXIVDMA_WRITE);

	/* Enable your video IP interrupts if needed
	 */

	/* Enable DMA read and write channel interrupts
	 *
	 * If interrupts overwhelms the system, please do not enable interrupt
	 */
	XAxiVdma_IntrEnable(&AxiVdma, XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_WRITE);
	XAxiVdma_IntrEnable(&AxiVdma, XAXIVDMA_IXR_ALL_MASK, XAXIVDMA_READ);

	/* Start the DMA engine to transfer
	 */
	Status = StartTransfer(&AxiVdma);
	if (Status != XST_SUCCESS) {
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		return XST_FAILURE;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &ReadError);
	if (Status == XST_SUCCESS) {
                xil_printf("Test has read error %d\r\n", ReadError);
                Status = XST_FAILURE;
                goto Done;
	}

	/* Wait for dma tranfer to complete or timeout */
	Status = Xil_WaitForEvent((UINTPTR)&ReadDone, NUM_TEST_FRAME_SETS, NUM_TEST_FRAME_SETS, POLL_TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		xil_printf("DMA read failed %d\r\n", Status);
                goto Done;
	}

	/* Check for any error events to occur */
	Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &WriteError);
	if (Status == XST_SUCCESS) {
                xil_printf("Test has write error %d\r\n", WriteError);
                Status = XST_FAILURE;
                goto Done;
	}

	/* Wait for dma tranfer to complete or timeout */
	Status = Xil_WaitForEvent((UINTPTR)&WriteDone, NUM_TEST_FRAME_SETS, NUM_TEST_FRAME_SETS, POLL_TIMEOUT_COUNTER);
	if (Status != XST_SUCCESS) {
		xil_printf("DMA write failed %d\r\n", Status);
                goto Done;
	}

	/* Soft reset for AXI VDMA channels which causes the AXI VDMA
	 * channels to be reset
	 */

	Polls = XAXIVDMA_RESET_TIMEOUT_USEC;
	XAxiVdma_Reset(&AxiVdma,XAXIVDMA_READ);

	while (Polls && XAxiVdma_ResetNotDone(&AxiVdma,XAXIVDMA_READ)) {
		usleep(1);
		Polls -= 1;
	}

	if (!Polls) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		            "Read channel reset failed %x\n\r",
		            (unsigned int)XAxiVdma_GetStatus(&AxiVdma,XAXIVDMA_READ));
	}

	Polls = XAXIVDMA_RESET_TIMEOUT_USEC;
	XAxiVdma_Reset(&AxiVdma,XAXIVDMA_WRITE);
	while (Polls && XAxiVdma_ResetNotDone(&AxiVdma,XAXIVDMA_WRITE)) {
		usleep(1);
		Polls -= 1;
	}

	if (!Polls) {
		xdbg_printf(XDBG_DEBUG_ERROR,
		            "Write channel reset failed %x\n\r",
		            (unsigned int)XAxiVdma_GetStatus(&AxiVdma,XAXIVDMA_WRITE));
	}

	for(Index = 0; Index < ReadCount; Index++) {
		Status = CheckFrame(Index);
		if (Status != XST_SUCCESS) {
			xil_printf("Check frame %d failed %d\n\r", Index, Status);
			goto Done;
		}
	}
	xil_printf("Successfully ran axivdma intr Example\r\n");

Done:
	DisableIntrSystem(READ_INTR_ID, WRITE_INTR_ID);

	if (Status != XST_SUCCESS) {
		if(Status == XST_VDMA_MISMATCH_ERROR)
			xil_printf("DMA Mismatch Error\r\n");
		xil_printf("axivdma intr Example Failed\r\n");
		Status = XST_FAILURE;
	}

	xil_printf("--- Exiting main() --- \r\n");

	return Status;
}


/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	InstancePtr is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int ReadSetup(XAxiVdma *InstancePtr)
{
	int Index;
	UINTPTR Addr;
	int Status;

	ReadCfg.VertSizeInput = SUBFRAME_VERTICAL_SIZE;
	ReadCfg.HoriSizeInput = SUBFRAME_HORIZONTAL_SIZE;

	ReadCfg.Stride = FRAME_HORIZONTAL_LEN;
	ReadCfg.FrameDelay = 0;  /* This example does not test frame delay */

	ReadCfg.EnableCircularBuf = 1;
	ReadCfg.EnableSync = 0;  /* No Gen-Lock */

	ReadCfg.PointNum = 0;    /* No Gen-Lock */
	ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

	ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

	Status = XAxiVdma_DmaConfig(InstancePtr, XAXIVDMA_READ, &ReadCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Read channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * These addresses are physical addresses
	 */
	Addr = READ_ADDRESS_BASE + BlockStartOffset;
	for(Index = 0; Index < ReadCount; Index++) {
		ReadCfg.FrameStoreStartAddr[Index] = Addr;

		BufferInit(Addr,FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN,TEST_START_VALUE);
		Xil_DCacheFlushRange(Addr, FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN);
		Addr += FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 * The buffer addresses are physical addresses
	 */
	Status = XAxiVdma_DmaSetBufferAddr(InstancePtr, XAXIVDMA_READ,
			ReadCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Read channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	InstancePtr is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int WriteSetup(XAxiVdma * InstancePtr)
{
	int Index;
	UINTPTR Addr;
	int Status;

	WriteCfg.VertSizeInput = SUBFRAME_VERTICAL_SIZE;
	WriteCfg.HoriSizeInput = SUBFRAME_HORIZONTAL_SIZE;

	WriteCfg.Stride = FRAME_HORIZONTAL_LEN;
	WriteCfg.FrameDelay = 0;  /* This example does not test frame delay */

	WriteCfg.EnableCircularBuf = 1;
	WriteCfg.EnableSync = 0;  /* No Gen-Lock */

	WriteCfg.PointNum = 0;    /* No Gen-Lock */
	WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

	WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

	WriteCfg.EnableVFlip = 1; /* Enable vertical flip */

	Status = XAxiVdma_DmaConfig(InstancePtr, XAXIVDMA_WRITE, &WriteCfg);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Write channel config failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Initialize buffer addresses
	 *
	 * Use physical addresses
	 */
	Addr = WRITE_ADDRESS_BASE + BlockStartOffset;
	for(Index = 0; Index < WriteCount; Index++) {
		WriteCfg.FrameStoreStartAddr[Index] = Addr;

		Addr += FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN;
	}

	/* Set the buffer addresses for transfer in the DMA engine
	 */
	Status = XAxiVdma_DmaSetBufferAddr(InstancePtr, XAXIVDMA_WRITE,
	        WriteCfg.FrameStoreStartAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Write channel set buffer address failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Clear data buffer
	 */
	memset((void *)WriteFrameAddr, 0,
	    FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN * WriteCount);
	Xil_DCacheFlushRange(WriteFrameAddr, FRAME_HORIZONTAL_LEN * FRAME_VERTICAL_LEN * WriteCount);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function starts the DMA transfers. Since the DMA engine is operating
* in circular buffer mode, video frames will be transferred continuously.
*
* @param	InstancePtr points to the DMA engine instance
*
* @return	XST_SUCCESS if both read and write start successfully
*		XST_FAILURE if one or both directions cannot be started
*
* @note		None.
*
******************************************************************************/
static int StartTransfer(XAxiVdma *InstancePtr)
{
	int Status;

	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_WRITE);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Start Write transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	Status = XAxiVdma_DmaStart(InstancePtr, XAXIVDMA_READ);
	if (Status != XST_SUCCESS) {
		xil_printf(
		    "Start read transfer failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function setups the interrupt system so interrupts can occur for the
* DMA.  This function assumes INTC component exists in the hardware system.
*
* @param	AxiDmaPtr is a pointer to the instance of the DMA engine
* @param	ReadIntrId is the read channel Interrupt ID.
* @param	WriteIntrId is the write channel Interrupt ID.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId,
				u16 WriteIntrId)
{
	int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc *IntcInstancePtr =&Intc;


	/* Initialize the interrupt controller and connect the ISRs */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {

		xil_printf( "Failed init intc\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, ReadIntrId,
	         (XInterruptHandler)XAxiVdma_ReadIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Failed read channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XIntc_Connect(IntcInstancePtr, WriteIntrId,
	         (XInterruptHandler)XAxiVdma_WriteIntrHandler, AxiVdmaPtr);
	if (Status != XST_SUCCESS) {

		xil_printf(
		    "Failed write channel connect intc %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Start the interrupt controller */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {

		xil_printf( "Failed to start intc\r\n");
		return XST_FAILURE;
	}

	/* Enable interrupts from the hardware */
	XIntc_Enable(IntcInstancePtr, ReadIntrId);
	XIntc_Enable(IntcInstancePtr, WriteIntrId);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_InterruptHandler,
			(void *)IntcInstancePtr);

	Xil_ExceptionEnable();

#else

	XScuGic *IntcInstancePtr = &Intc;	/* Instance of the Interrupt Controller */
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

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, ReadIntrId, 0xA0, 0x3);
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, WriteIntrId, 0xA0, 0x3);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, ReadIntrId,
				(Xil_InterruptHandler)XAxiVdma_ReadIntrHandler,
				AxiVdmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XScuGic_Connect(IntcInstancePtr, WriteIntrId,
				(Xil_InterruptHandler)XAxiVdma_WriteIntrHandler,
				AxiVdmaPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the DMA device.
	 */
	XScuGic_Enable(IntcInstancePtr, ReadIntrId);
	XScuGic_Enable(IntcInstancePtr, WriteIntrId);

	Xil_ExceptionInit();

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				IntcInstancePtr);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();


#endif

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts
*
* @param	ReadIntrId is interrupt ID associated w/ DMA read channel
* @param	WriteIntrId is interrupt ID associated w/ DMA write channel
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void DisableIntrSystem(u16 ReadIntrId, u16 WriteIntrId)
{

#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc *IntcInstancePtr =&Intc;

	/* Disconnect the interrupts for the DMA TX and RX channels */
	XIntc_Disconnect(IntcInstancePtr, ReadIntrId);
	XIntc_Disconnect(IntcInstancePtr, WriteIntrId);
#else
	XScuGic *IntcInstancePtr = &Intc;

	XScuGic_Disable(IntcInstancePtr, ReadIntrId);
	XScuGic_Disable(IntcInstancePtr, WriteIntrId);

	XScuGic_Disconnect(IntcInstancePtr, ReadIntrId);
	XScuGic_Disconnect(IntcInstancePtr, WriteIntrId);
#endif
}

/*****************************************************************************/
/*
 * Call back function for read channel
 *
 * This callback only clears the interrupts and updates the transfer status.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadCallBack(void *CallbackRef, u32 Mask)
{

	if (Mask & XAXIVDMA_IXR_FRMCNT_MASK) {
		ReadDone += 1;
	}
}

/*****************************************************************************/
/*
 * Call back function for read channel error interrupt
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{

	if (Mask & XAXIVDMA_IXR_ERROR_MASK) {
		ReadError += 1;
	}
}

/*****************************************************************************/
/*
 * Call back function for write channel
 *
 * This callback only clears the interrupts and updates the transfer status.
 *
 * @param	CallbackRef is the call back reference pointer
 * @param	Mask is the interrupt mask passed in from the driver
 *
 * @return	None
*
******************************************************************************/
static void WriteCallBack(void *CallbackRef, u32 Mask)
{

	if (Mask & XAXIVDMA_IXR_FRMCNT_MASK) {
		WriteDone += 1;
	}
}

/*****************************************************************************/
/*
* Call back function for write channel error interrupt
*
* @param	CallbackRef is the call back reference pointer
* @param	Mask is the interrupt mask passed in from the driver
*
* @return	None
*
******************************************************************************/
static void WriteErrorCallBack(void *CallbackRef, u32 Mask)
{

	if (Mask & XAXIVDMA_IXR_ERROR_MASK) {
		WriteError += 1;
	}
}

/*****************************************************************************/
/**
 * Compare a range of data from one frame with data from another frame
 *
 * @param FrameIndex, frame index for read and write frames
 *
 * @return
 *      - XST_SUCCESS if frames are identical
 *      - XST_FAILURE otherwise
 *******************************************************************************/
static int CheckFrame(int FrameIndex)
{
	u8 *RdAddr;
	u8 *WrAddr;
	int Hsize_Max;
	int Vsize_Max;
	int Vsize;
	int Hsize;
	int Index;

	RdAddr = (u8 *)ReadCfg.FrameStoreStartAddr[FrameIndex];
	WrAddr = (u8 *)WriteCfg.FrameStoreStartAddr[FrameIndex];

	Hsize_Max = ReadCfg.HoriSizeInput;
	Vsize_Max = ReadCfg.VertSizeInput;

	Xil_DCacheInvalidateRange((UINTPTR)RdAddr, Vsize_Max*Hsize_Max*ReadCount);

	for (Vsize = 0; Vsize < Vsize_Max; Vsize++) {
		for (Hsize = 0; Hsize < Hsize_Max; Hsize++) {
			Index = Hsize + Vsize * FRAME_HORIZONTAL_LEN;
			if (RdAddr[Index] != WrAddr[Index]) {

				xil_printf("Check frame data error (maybe "
				"expected) %d/%d, %x/%x, %x/%x: %x/%x\n\r",
				Hsize, Vsize, (UINTPTR)RdAddr, Index,
				(UINTPTR)WrAddr, Index,
				(UINTPTR)RdAddr[Index],
				(UINTPTR)WrAddr[Index]);
				return XST_FAILURE;
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * Buffer initialization
 *
 * Initialize a buffer from a specified value. Value increases in the buffer,
 * and wraps around at 0xFF.
 *
 * @param BaseAddr, starting address for buffer
 * @param Length, length of the buffer in bytes
 * @param StartValue, value for first byte
 *
 * @return
 *      None
 ******************************************************************************/
static void BufferInit(UINTPTR BaseAddr, int Length, u8 StartValue)
{
        int Tmp;
        u8 *Addr;
        u8 Value;

        Addr = (u8 *)BaseAddr;
        Value = StartValue;

        for (Tmp = 0; Tmp < Length; Tmp ++) {
                *Addr = Value;
                Addr += 1;
                Value = (Value + 1) & 0xFF;
        }

        return;
}
