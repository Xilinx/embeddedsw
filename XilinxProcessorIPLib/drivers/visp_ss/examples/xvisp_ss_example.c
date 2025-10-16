/******************************************************************************
 * @file xvisp_ss_example.c
 * @brief Xilinx Video Image Signal Processing Subsystem (VISP SS) Example
 *
 * This file contains a comprehensive example demonstrating the use of the
 * Xilinx VISP SS driver. The example includes:
 * - VISP SS initialization and configuration
 * - Frame buffer writer setup and management
 * - Interrupt system configuration
 * - Memory management with aligned buffer allocation
 * - Hardware reset and power management
 *
 * The VISP SS is designed for real-time video processing applications
 * including camera interfaces, video streaming, and image processing pipelines.
 *
 * @note This example is designed for non-MCM (Multi-Channel Mode) operation
 * @note Frame buffer writers support multiple output formats including RGB888
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (C) 2025 Xilinx Inc. All rights reserved.
 ******************************************************************************/

#include <xvisp_ss_example.h>
#include <xil_types.h>
#include "xparameters.h"        /* Contains hardware platform parameters */
#include "xscugic.h"           /* Generic Interrupt Controller driver */
#include "xvisp_ss.h"          /* VISP SS driver header */
#include "xil_exception.h"     /* Exception handling utilities */
#include "limo.h"
#include "lilo.h"
#include "mimo.h"
#include "image.h"

#define LOGTAG "MAIN"

/*ISP_ID to configure pipeline*/
#define ISP_ID 0
/* *
 * If custom_json = 0  internal json will be configured
 * custom_json = 1 external json will be configured
 * */
int custom_json = 0;
int image_len = image_raw_len;

#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
int init_lilo = 1;
extern int dequeue_call_count;
#include "xv_frmbufwr_l2.h"    /* Frame Buffer Writer Level 2 driver */
#include "xintc.h"             /* Interrupt Controller driver */
#include "xvidc.h"             /* Video Common driver for format definitions */

/**
 * @brief Output format enumeration
 *
 * Defines the supported output formats for the frame buffer writer.
 * These formats determine how video data is stored in memory.
 */
enum outputformat {
	RGB888_8bit,    /* 24-bit RGB format, 8 bits per component */
	YUV420_8bit,    /* YUV 4:2:0 format, 8 bits per component */
	YUV422_8bit,    /* YUV 4:2:2 format, 8 bits per component */
	RGB888_10bit,   /* 30-bit RGB format, 10 bits per component */
	YUV420_10bit,   /* YUV 4:2:0 format, 10 bits per component */
	YUV422_10bit,   /* YUV 4:2:2 format, 10 bits per component */
	YUV444,         /* YUV 4:4:4 format */
	YUV400_8bit,    /* Luma-only format, 8 bits */
	YUV400_10bit    /* Luma-only format, 10 bits */
};

#define NUM_FBWR XPAR_XV_FRMBUF_WR_NUM_INSTANCES  /* Number of frame buffer writers */

enum outputformat outputformat_selection;  /* Currently selected output format */

XVidC_ColorFormat FBWR_Cfmt[NUM_FBWR];    /* Color format for each frame buffer writer */
#define Buffer_Count 3                      /* Triple buffering for smooth video processing */

/* Aligned buffer structure for hardware buffers */
struct aligned_buf {
	void *original_addr;    /* Original malloc address for freeing */
	void *aligned_addr;     /* Aligned address for hardware */
	uint32_t size;
	uint32_t alignment;
};

/* Updated aligned buffer structure to use new memory manager */
struct aligned_buf Frame_Array_p[4][Buffer_Count];  /* Frame buffer arrays */

u32 Frame_Count[NUM_FBWR];                  /* Frame counter for each FBWR */
u32 chroma_offset[NUM_FBWR];               /* Chroma buffer offset for semi-planar formats */
u32 Wr_Ptr[NUM_FBWR], Rd_Ptr[NUM_FBWR];    /* Write and read pointers for buffer management */
XV_FrmbufWr_l2 frmbufwr[NUM_FBWR];     /* Frame buffer writer instances */
int fbwr_enable[NUM_FBWR]; // to enable FBWRs                /* Enable flags for each FBWR */
int fbwr_layerID[NUM_FBWR];                                   /* Layer ID mapping for each FBWR */
static int fbcb_cnt[NUM_FBWR] = {0};                         /* Callback counter for each FBWR */
XVidC_VideoStream StreamOut;                                /* Video stream configuration */
XIntc InterruptController;                                   /* AXI Interrupt Controller instance */
#endif

/* Global VISP SS instance - main video processing subsystem handle */
#define VISP_INST XPAR_XVISP_SS_NUM_INSTANCES
XVisp_Ss VispSsInst[VISP_INST];

/* Global interrupt controller instance - manages all system interrupts */
XScuGic Intc;

/**
 * @brief Main application entry point
 *
 * This function demonstrates a complete VISP SS application workflow:
 * 1. Initialize memory management system
 * 2. Perform hardware reset sequence
 * 3. Setup interrupt system
 * 4. Configure VISP SS subsystem
 * 5. Initialize ISP to RPU mapping
 * 6. Setup frame buffer writers (if available)
 * 7. Initialize mailbox communication
 * 8. Run the main video benchmark application
 * 9. Cleanup resources
 *
 * @return 0 on success, negative value on error
 *
 * @note The function follows Xilinx recommended initialization sequence
 * @note Memory manager must be initialized before any buffer allocations
 */
int main()
{

	RESULT ret = RET_SUCCESS;


	/* Initialize the memory manager early - required for all buffer allocations */
	if (mm_init() != 0) {
		xil_printf("ERROR: Memory manager initialization failed.\r\n");
		return -1;
	}
	xil_printf("Memory manager initialized successfully.\r\n");

	/* Initialize IRQ - setup interrupt handling infrastructure */
	u32 Status = SetupInterruptSystem();
	if (Status == XST_FAILURE)
		xil_printf("\r\n\r\n IRQ Configuration failed.\r\n\r\n");

	/* Configure VISP SS - main video processing subsystem */
	Status = config_visp_ss(XPAR_XVISP_SS_0_BASEADDR);
	if (Status == XST_INVALID_VERSION) {
			return Status;
	}
	else if (Status != XST_SUCCESS) {
		xil_printf("ERROR: VISP SS configuration failed.\r\n");
		return Status;
	}

	/* Initialize ISP to RPU mapping - connects ISP outputs to processing units */
	reset_isp2rpu_mapping();
	for(int i=0;i<VISP_INST;i++)
	{
		init_isp2rpu_mapping(VispSsInst[0].Config.Rpu, VispSsInst[i].Config.IspId);
	}
	/* Setup frame buffer writers if available - handles video output buffering */
#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
	setup_frmbuf_wr();
#endif

	/* Initialize mailbox communication - enables inter-processor communication */
	mailbox_wrapper();

	/* Run main video benchmark application - core video processing workflow */
	ret = main_vvbench(&VispSsInst[ISP_ID]);

	/* Cleanup mailbox resources */
	mailbox_close();

	return 0;
}

#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
/**
 * @brief Setup AXI Interrupt Controller for Frame Buffer Writers
 *
 * This function configures the AXI Interrupt Controller to handle interrupts
 * from multiple frame buffer writer instances. It performs the following:
 * 1. Initialize the interrupt controller
 * 2. Perform self-test to verify functionality
 * 3. Connect interrupt handlers for all frame buffer writers
 * 4. Enable individual interrupt sources
 * 5. Start the interrupt controller
 * 6. Setup exception handling system
 *
 * @param BaseAddr - Base address of the AXI Interrupt Controller
 * @return XST_SUCCESS on success, XST_FAILURE on error
 *
 * @note This function is only compiled when frame buffer writers are present
 * @note Each frame buffer writer gets its own interrupt handler
 */
int setup_axi_with_device(UINTPTR BaseAddr)
{
	int Status;
	extern XScuGic Intc;

	/* Initialize the AXI Interrupt Controller */
	Status = XIntc_Initialize(&InterruptController, BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Perform self-test to verify interrupt controller functionality */
	Status = XIntc_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Connect interrupt handlers for all frame buffer writer instances */
	for (int i = 0; i < 4; i++) {
		// Connect interrupt handlers for all 4 frame buffer writers
		Status = XIntc_Connect(&InterruptController, frmbufwr[i].FrmbufWr.Config.IntrId,
				       (XInterruptHandler) XVFrmbufWr_InterruptHandler,
				       (void *) &frmbufwr[i]);
		if (Status != XST_SUCCESS) {
			xil_printf("Intc Example Failed\r\n");
			return XST_FAILURE;
		}

		/* Enable interrupt for this frame buffer writer */
		XIntc_Enable(&InterruptController, frmbufwr[i].FrmbufWr.Config.IntrId);
	}

	/* Start the interrupt controller in real mode */
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Initialize exception handling */
	Xil_ExceptionInit();

	Status = XSetupInterruptSystem(&InterruptController, (Xil_ExceptionHandler)XIntc_InterruptHandler,
				       InterruptController.CfgPtr->IntrId, InterruptController.CfgPtr->IntrParent, 0xA0);
	if (Status == XST_FAILURE) {
		xil_printf("Intc Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Intc Example\r\n");
	return XST_SUCCESS;
}
#endif

/**
 * @brief Configure VISP SS (Video Image Signal Processing Subsystem)
 *
 * This function performs the complete initialization and configuration of the
 * VISP SS driver. The VISP SS is the main video processing pipeline that
 * handles camera input, ISP processing, and video output formatting.
 *
 * Configuration steps:
 * 1. Lookup device configuration based on device ID or base address
 * 2. Initialize the driver with the configuration
 * 3. Verify single-stream operation (non-MCM mode)
 *
 * @param baseaddress - Base address of the VISP SS hardware block
 * @return XST_SUCCESS on successful configuration
 *         XST_FAILURE on configuration lookup or initialization failure
 *         XST_INVALID_VERSION if MCM mode is detected (not supported)
 *
 * @note This example only supports non-MCM (Multi-Channel Mode) operation
 * @note The function uses either device ID (non-SDT) or base address (SDT) lookup
 */
int config_visp_ss(u32 baseaddress)
{
	xvisp_ss_Config *CfgPtr;
	int Status;

	/* Lookup the device configuration */
	for(int i=0;i<VISP_INST;i++)
	{
		MEMSET(CfgPtr,0,sizeof(xvisp_ss_Config));
#ifndef SDT
		CfgPtr = XVisp_Ss_LookupConfig(XPAR_VISP_SS_0_DEVICE_ID);
#else
		CfgPtr = XVisp_Ss_LookupConfig(baseaddress + (i * 0x800));
#endif
		if (!CfgPtr) {
			xil_printf("ERROR: Lookup of VISP SS configuration failed.\r\n");
			return XST_FAILURE;
		}

		/* Initialize the driver */
		Status = XVisp_Ss_CfgInitialize(&VispSsInst[i], CfgPtr, CfgPtr->BaseAddress);
		if (Status != XST_SUCCESS) {
			xil_printf("ERROR: Could not initialize VISP SS driver.\r\n");
			return XST_FAILURE;
		}
		xil_printf("base address 0x%x %d\n",baseaddress,__LINE__);
	}
	/* Verify single-stream operation - this example doesn't support MCM mode */
	if (VispSsInst[ISP_ID].Config.NumStreams > 1) {
		xil_printf("This ISP example only works for NON-MCM mode.\r\n");
		return XST_INVALID_VERSION;
	}

	xil_printf("VISP SS driver initialized successfully.\r\n");

	return XST_SUCCESS;
}

/**
 * @brief Setup interrupt system for the entire application
 *
 * This function initializes the interrupt controller and sets up the interrupt system
 * for the entire video processing application. It configures the main system
 * interrupt controller (GIC for ARM-based systems).
 *
 * Initialization steps:
 * 1. Lookup interrupt controller configuration
 * 2. Initialize the interrupt controller with configuration
 * 3. Initialize exception handling system
 * 4. Register interrupt controller handler with exception table
 *
 * @return XST_SUCCESS if successful, error code otherwise
 *
 * @note Uses static count to ensure initialization happens only once
 * @note Supports both SDT and non-SDT build systems
 * @note Critical for frame buffer writer interrupt handling
 */
int SetupInterruptSystem(void)
{
	int Status;
	static int count = 0;

	/* Ensure initialization happens only once */
	if (count == 0) {
		count++;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
		XScuGic *IntcInstPtr = &Intc;
#endif

		/*
		 * Initialize the interrupt controller driver so that it's ready to
		 * use, specify the device ID that was generated in xparameters.h
		 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
		XScuGic_Config *IntcCfgPtr;
#ifndef SDT
		/* Lookup configuration by device ID (traditional method) */
		IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
#else
		/* Lookup configuration by base address (SDT method) */
		IntcCfgPtr = XScuGic_LookupConfig(XPAR_GIC_BASEADDR);
#endif
		if (IntcCfgPtr == NULL) {
			xil_printf("ERR:: Interrupt Controller not found");
			return (XST_DEVICE_NOT_FOUND);
		}

		/* Initialize the interrupt controller with found configuration */
		Status = XScuGic_CfgInitialize(IntcInstPtr,
					       IntcCfgPtr,
					       IntcCfgPtr->CpuBaseAddress);
#endif
		if (Status != XST_SUCCESS) {
			xil_printf("Intc initialization failed!\r\n");
			return XST_FAILURE;
		}

		/* Initialize exception handling system */
		Xil_ExceptionInit();

		/*
		 * Register the interrupt controller handler with the exception table.
		 * This enables the interrupt controller to handle all interrupts
		 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
					     (XScuGic *)IntcInstPtr);
#endif
	}
	return (XST_SUCCESS);
}


#ifdef XPAR_XV_FRMBUF_WR_NUM_INSTANCES
/**
 * @brief Calculate memory stride for different video formats
 *
 * This function calculates the memory stride (bytes per line) required for
 * different video color formats. The stride must be aligned to the AXI MM
 * data width for optimal memory access performance.
 *
 * Supported formats and their byte requirements:
 * - Y_UV10, Y_UV10_420, Y10: 4 bytes per 3 pixels (10-bit formats)
 * - Y_UV8, Y_UV8_420, Y8: 1 byte per pixel (8-bit formats)
 * - RGB8, YUV8, BGR8: 3 bytes per pixel (24-bit formats)
 * - Others: 4 bytes per pixel (32-bit formats)
 *
 * @param Cfmt - Video color format (XVidC_ColorFormat)
 * @param AXIMMDataWidth - AXI Memory Mapped data width in bits
 * @param StreamPtr - Pointer to video stream structure containing resolution
 * @return Calculated stride in bytes, aligned to AXI MM data width
 *
 * @note Stride calculation ensures proper memory alignment for DMA transfers
 * @note Different formats have different pixel packing requirements
 */
u32 CalcStride(XVidC_ColorFormat Cfmt,
	       u16 AXIMMDataWidth,
	       XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive;
	u16 MMWidthBytes = AXIMMDataWidth / 8; /* Convert bits to bytes */

	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y10)) {
		/* 10-bit formats: 4 bytes per 3 pixels due to 10-bit packing */
		stride = ((((width * 4) / 3) + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	}	else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
		    || (Cfmt == XVIDC_CSF_MEM_Y8)) {
		/* 8-bit formats: 1 byte per pixel */
		stride = ((width + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;

	}	else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)
		    || (Cfmt == XVIDC_CSF_MEM_BGR8)) {
		/* 24-bit RGB formats: 3 bytes per pixel */
		stride = (((width * 3) + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	}	else {
		/* 32-bit formats and others: 4 bytes per pixel */
		stride = (((width * 4) + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	}
	xil_printf("FBWR STRIDE %d \n", stride);
	return (stride);
}
/**
 * @brief Configure Frame Buffer Writer for video output
 *
 * This function configures a single frame buffer writer instance with the
 * specified parameters. Frame buffer writers are responsible for storing
 * processed video frames in memory for display or further processing.
 *
 * Configuration steps:
 * 1. Wait for frame buffer writer to become idle
 * 2. Get aligned buffer address for current write pointer
 * 3. Configure memory format and video stream parameters
 * 4. Set buffer addresses for luma and chroma (if applicable)
 * 5. Enable completion interrupts
 *
 * @param StrideInBytes - Memory stride (bytes per line) for the video format
 * @param Cfmt - Video color format (XVidC_ColorFormat)
 * @param StreamPtr - Pointer to video stream configuration
 * @param chroma_offset - Offset for chroma buffer in semi-planar formats
 * @param fbwr_id - Frame buffer writer instance ID (0 to NUM_FBWR-1)
 * @return XST_SUCCESS on successful configuration, XST_FAILURE on error
 *
 * @note Semi-planar formats (YUV) require separate chroma buffer setup
 * @note Buffer addresses must be aligned for optimal DMA performance
 * @note Interrupts are enabled to signal frame completion
 */
int ConfigFrmbuf(u32 StrideInBytes,
		 XVidC_ColorFormat Cfmt,
		 XVidC_VideoStream *StreamPtr,
		 u32 chroma_offset,
		 u32 fbwr_id
		)
{
	int Status;
	u64 XvFrmBufFWr_Buffer_Baseaddr = 0;

	/* Print allocated buffers for debugging */
	for (int i = 0; i < Buffer_Count; i++) {
		xil_printf("Baseaddr:%p\n", (uintptr_t)Frame_Array_p[fbwr_id][i].aligned_addr);
		xil_printf("Data :%x\n", Xil_In32((uintptr_t)(uintptr_t)Frame_Array_p[fbwr_id][i].aligned_addr));
	}

	/* Wait for frame buffer writer to become idle before configuration */
	XVFrmbufWr_WaitForIdle(&frmbufwr[fbwr_id]);

	/* Get current buffer address from the write pointer */
	XvFrmBufFWr_Buffer_Baseaddr = (uintptr_t)Frame_Array_p[fbwr_id][Wr_Ptr[fbwr_id]].aligned_addr;

	/* Configure Frame Buffer memory format and video stream */
	Status = XVFrmbufWr_SetMemFormat(&frmbufwr[fbwr_id], StrideInBytes, Cfmt, StreamPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write %d\r\n", Status);
		return (XST_FAILURE);
	}

	/* Set the luma buffer address */
	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr[fbwr_id], XvFrmBufFWr_Buffer_Baseaddr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame \ Buffer Write buffer address\r\n");
		return (XST_FAILURE);
	}

	/* Set Chroma Buffer Address for semi-planar color formats (YUV) */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {

		Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr[fbwr_id],
							XvFrmBufFWr_Buffer_Baseaddr + chroma_offset);
		if (Status != XST_SUCCESS) {
			xil_printf("ERROR::Unable to configure Frame Buffer \ Write chroma buffer address\r\n");
			return (XST_FAILURE);
		}
	}

	/* Enable Interrupt to signal frame completion */
	XVFrmbufWr_InterruptEnable(&frmbufwr[fbwr_id], XVFRMBUFWR_IRQ_DONE_MASK);
	xil_printf("Frame buffer write config done\n");
	return (Status);
}


/**
 * @brief Setup and initialize all Frame Buffer Writer instances
 *
 * This function performs comprehensive setup of all frame buffer writer instances
 * in the system. Frame buffer writers are essential for video output buffering,
 * allowing smooth video display and processing.
 *
 * Initialization sequence:
 * 1. Reset each frame buffer writer IP block
 * 2. Initialize frame buffer writer drivers
 * 3. Setup AXI Interrupt Controller for frame buffer interrupts
 * 4. Register completion callbacks for each frame buffer writer
 *
 * The function sets up separate interrupt handlers for each frame buffer writer
 * to handle frame completion events independently.
 *
 * @note Only compiled when XPAR_XV_FRMBUF_WR_NUM_INSTANCES is defined
 * @note Each frame buffer writer gets its own completion callback
 * @note Supports up to 4 frame buffer writer instances
 */
void setup_frmbuf_wr()
{
	int Status;
#ifdef SDT
	UINTPTR Baseaddress = XPAR_FRMBUF_WR_SS_V_FRMBUF_WR_0_BASEADDR;
#endif

	/* Initialize Frame Buffer Write instances */
	for (int i = 0; i < 4; i++) {
		/* Reset each frame buffer writer IP block */
		Reset_IP(i);

#ifndef SDT
		/* Traditional initialization using base address offset */
		Status = XVFrmbufWr_Initialize(&frmbufwr[i], XPAR_FRMBUF_WR_SS_V_FRMBUF_WR_0_BASEADDR + i);
#else
		/* SDT-based initialization using incremental base addresses */
		Status = XVFrmbufWr_Initialize(&frmbufwr[i], Baseaddress);
		Baseaddress += 0x10000; /* Increment to next frame buffer writer base address */
#endif
		if (Status != XST_SUCCESS) {
			xil_printf(TXT_RED "Frame Buffer Write Init failed status = %x.\r\n" TXT_RST, Status);
			return XST_FAILURE;
		} else
			xil_printf("Framebuffer write[%d] initilaized\n", i);
	}

	/* Setup AXI Interrupt Controller for frame buffer writer interrupts */
	setup_axi_with_device(XPAR_FRMBUF_WR_SS_AXI_INTC_FRMBUF_BASEADDR);

	/* Register completion callback for Frame Buffer Writer 0 */
	Status = XVFrmbufWr_SetCallback(&frmbufwr[0],
					XVFRMBUFWR_HANDLER_DONE,
					(void *)FrmbufwrDoneCallback_0,
					(void *) &frmbufwr[0]);
	if (Status != XST_SUCCESS)	{
		xil_printf("Frame Buffer Write Call back  failed status = %x.\r\n" TXT_RST, Status);
		return XST_FAILURE;
	}

	/* Register completion callback for Frame Buffer Writer 1 */
	Status = XVFrmbufWr_SetCallback(&frmbufwr[1],
					XVFRMBUFWR_HANDLER_DONE,
					(void *)FrmbufwrDoneCallback_1,
					(void *) &frmbufwr[1]);
	if (Status != XST_SUCCESS)	{
		xil_printf("Frame Buffer Write Call back  failed status = %x.\r\n" TXT_RST, Status);
		return XST_FAILURE;
	}

	/* Register completion callback for Frame Buffer Writer 2 */
	Status = XVFrmbufWr_SetCallback(&frmbufwr[2],
					XVFRMBUFWR_HANDLER_DONE,
					(void *)FrmbufwrDoneCallback_2,
					(void *) &frmbufwr[2]);
	if (Status != XST_SUCCESS)	{
		xil_printf("Frame Buffer Write Call back  failed status = %x.\r\n" TXT_RST, Status);
		return XST_FAILURE;
	}

	/* Register completion callback for Frame Buffer Writer 3 */
	Status = XVFrmbufWr_SetCallback(&frmbufwr[3],
					XVFRMBUFWR_HANDLER_DONE,
					(void *)FrmbufwrDoneCallback_3,
					(void *) &frmbufwr[3]);
	if (Status != XST_SUCCESS)	{
		xil_printf("Frame Buffer Write Call back  failed status = %x.\r\n" TXT_RST, Status);
		return XST_FAILURE;
	}

}


/**
 * @brief Frame Buffer Writer 0 Done Callback Handler
 *
 * This interrupt callback function is invoked when Frame Buffer Writer 0
 * completes writing a frame to memory. It implements a triple-buffering
 * scheme to ensure smooth video processing without frame drops.
 *
 * Callback operations:
 * 1. Increment callback and dequeue counters
 * 2. Implement circular buffer management (triple buffering)
 * 3. Setup next buffer addresses for continuous operation
 * 4. Handle chroma buffer addresses for semi-planar formats
 * 5. Increment frame counter for statistics
 *
 * Triple buffering scheme:
 * - Buffer 0: Currently being written to
 * - Buffer 1: Available for reading/display
 * - Buffer 2: Next buffer to be written to
 *
 * @param CallbackRef - Reference to the frame buffer writer instance
 *
 * @note This function runs in interrupt context - keep processing minimal
 * @note Buffer pointer management ensures continuous video processing
 * @note Semi-planar formats require separate chroma buffer setup
 */
void FrmbufwrDoneCallback_0(void *CallbackRef)
{
	static const int fbwr_id = 0; /* Frame buffer writer ID for this callback */

	XVidC_ColorFormat Cfmt = FBWR_Cfmt[fbwr_id];  /* Get color format for this FBWR */

	/* Increment callback counters */
	fbcb_cnt[fbwr_id]++;
	dequeue_call_count++;

	int Status;
	u64 XvFrmBufFWr_Buffer_Baseaddr = 0, XvFrmBufRd_Buffer_Baseaddr = 0;

	/* Triple buffer management xil_printfc - rotate buffer pointers */
	Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id];
	if (Wr_Ptr[fbwr_id] == 0)
		Rd_Ptr[fbwr_id] = (Buffer_Count - 1) ; /* Wrap to last buffer */
	else
		Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] - 1 ; /* Move to previous buffer */

	/* Update write pointer for next frame */
	if (Wr_Ptr[fbwr_id] == (Buffer_Count - 1)) {
		Wr_Ptr[fbwr_id] = 0; /* Wrap to first buffer */
	} else	{
		Wr_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] + 1; /* Move to next buffer */
	}

	/* Get buffer addresses for read and write operations */
	XvFrmBufRd_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Rd_Ptr[fbwr_id]].aligned_addr;
	XvFrmBufFWr_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Wr_Ptr[fbwr_id]].aligned_addr;

	/* Setup next buffer address for continuous operation */
	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr[fbwr_id], XvFrmBufFWr_Buffer_Baseaddr);
	if (Status != XST_SUCCESS)
		xil_printf("ERROR:: Unable to configure Frame Buffer \ Write buffer address\r\n");

	/* Set Chroma Buffer Address for semi-planar color formats (YUV) */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr[fbwr_id],
							XvFrmBufFWr_Buffer_Baseaddr + chroma_offset[fbwr_id]);
		if (Status != XST_SUCCESS)
			xil_printf("ERROR:: Unable to configure Frame Buffer \ Write chroma buffer address\r\n");
	}

	/* Increment frame counter for statistics tracking */
	Frame_Count[fbwr_id]++;
}


/**
 * @brief Frame Buffer Writer 1 Done Callback Handler
 *
 * Interrupt callback for Frame Buffer Writer 1. Implements the same triple-buffering
 * xil_printfc as FrmbufwrDoneCallback_0 but for FBWR instance 1.
 *
 * @param CallbackRef - Reference to the frame buffer writer instance
 * @see FrmbufwrDoneCallback_0 for detailed callback operation description
 */
void FrmbufwrDoneCallback_1(void *CallbackRef)
{
	static const int fbwr_id = 1;

	XVidC_ColorFormat Cfmt = FBWR_Cfmt[fbwr_id];

	fbcb_cnt[fbwr_id]++;
	dequeue_call_count++;

	int Status;
	u64 XvFrmBufFWr_Buffer_Baseaddr = 0, XvFrmBufRd_Buffer_Baseaddr = 0;

	/**********buffer swapping xil_printfc ***********/
	Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id];
	if (Wr_Ptr[fbwr_id] == 0)
		Rd_Ptr[fbwr_id] = (Buffer_Count - 1) ;
	else
		Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] - 1 ;

	if (Wr_Ptr[fbwr_id] == (Buffer_Count - 1))
		Wr_Ptr[fbwr_id] = 0;
	else
		Wr_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] + 1;

	XvFrmBufRd_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Rd_Ptr[fbwr_id]].aligned_addr;
	XvFrmBufFWr_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Wr_Ptr[fbwr_id]].aligned_addr;
	/******************************************/

	/*************************Set buffer for next interrupt******************/
	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr[fbwr_id], XvFrmBufFWr_Buffer_Baseaddr);
	if (Status != XST_SUCCESS)
		xil_printf("ERROR:: Unable to configure Frame Buffer \ Write buffer address\r\n");
	/* Set Chroma Buffer Address for semi-planar color formats */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr[fbwr_id],
							XvFrmBufFWr_Buffer_Baseaddr + chroma_offset[fbwr_id]);
		if (Status != XST_SUCCESS)
			xil_printf("ERROR:: Unable to configure Frame Buffer \ Write chroma buffer address\r\n");
	}
	/*************************************************************************/
	Frame_Count[fbwr_id]++;
}

/**
 * @brief Frame Buffer Writer 2 Done Callback Handler
 *
 * Interrupt callback for Frame Buffer Writer 2. Implements the same triple-buffering
 * xil_printfc as FrmbufwrDoneCallback_0 but for FBWR instance 2.
 *
 * @param CallbackRef - Reference to the frame buffer writer instance
 * @see FrmbufwrDoneCallback_0 for detailed callback operation description
 */
void FrmbufwrDoneCallback_2(void *CallbackRef)
{
	static const int fbwr_id = 2;

	XVidC_ColorFormat Cfmt = FBWR_Cfmt[fbwr_id];

	fbcb_cnt[fbwr_id]++;
	dequeue_call_count++;

	int Status;

	u64 XvFrmBufFWr_Buffer_Baseaddr = 0, XvFrmBufRd_Buffer_Baseaddr = 0;

	/**********buffer swapping xil_printfc ***********/
	Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id];
	if (Wr_Ptr[fbwr_id] == 0)
		Rd_Ptr[fbwr_id] = (Buffer_Count - 1) ;
	else
		Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] - 1 ;

	if (Wr_Ptr[fbwr_id] == (Buffer_Count - 1))
		Wr_Ptr[fbwr_id] = 0;
	else
		Wr_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] + 1;

	XvFrmBufRd_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Rd_Ptr[fbwr_id]].aligned_addr;
	XvFrmBufFWr_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Wr_Ptr[fbwr_id]].aligned_addr;
	/******************************************/

	/*************************Set buffer for next interrupt******************/
	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr[fbwr_id], XvFrmBufFWr_Buffer_Baseaddr);
	if (Status != XST_SUCCESS)
		xil_printf("ERROR:: Unable to configure Frame Buffer \ Write buffer address\r\n");
	/* Set Chroma Buffer Address for semi-planar color formats */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr[fbwr_id],
							XvFrmBufFWr_Buffer_Baseaddr + chroma_offset[fbwr_id]);
		if (Status != XST_SUCCESS)
			xil_printf("ERROR:: Unable to configure Frame Buffer \ Write chroma buffer address\r\n");
	}
	/*************************************************************************/
	Frame_Count[fbwr_id]++;
}

/**
 * @brief Frame Buffer Writer 3 Done Callback Handler
 *
 * Interrupt callback for Frame Buffer Writer 3. Implements the same triple-buffering
 * xil_printfc as FrmbufwrDoneCallback_0 but for FBWR instance 3.
 *
 * @param CallbackRef - Reference to the frame buffer writer instance
 * @see FrmbufwrDoneCallback_0 for detailed callback operation description
 */
void FrmbufwrDoneCallback_3(void *CallbackRef)
{
	static const int fbwr_id = 3;

	XVidC_ColorFormat Cfmt = FBWR_Cfmt[fbwr_id];

	fbcb_cnt[fbwr_id]++;
	dequeue_call_count++;
	int Status;
	u64 XvFrmBufFWr_Buffer_Baseaddr = 0, XvFrmBufRd_Buffer_Baseaddr = 0;

	/**********buffer swapping xil_printfc ***********/
	Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id];
	if (Wr_Ptr[fbwr_id] == 0)
		Rd_Ptr[fbwr_id] = (Buffer_Count - 1) ;
	else
		Rd_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] - 1 ;

	if (Wr_Ptr[fbwr_id] == (Buffer_Count - 1))
		Wr_Ptr[fbwr_id] = 0;
	else
		Wr_Ptr[fbwr_id] = Wr_Ptr[fbwr_id] + 1;

	XvFrmBufRd_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Rd_Ptr[fbwr_id]].aligned_addr;
	XvFrmBufFWr_Buffer_Baseaddr = Frame_Array_p[fbwr_id][Wr_Ptr[fbwr_id]].aligned_addr;
	/******************************************/

	/*************************Set buffer for next interrupt******************/
	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr[fbwr_id], XvFrmBufFWr_Buffer_Baseaddr);
	if (Status != XST_SUCCESS)
		xil_printf("ERROR:: Unable to configure Frame Buffer \ Write buffer address\r\n");
	/* Set Chroma Buffer Address for semi-planar color formats */
	if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
		Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr[fbwr_id],
							XvFrmBufFWr_Buffer_Baseaddr + chroma_offset[fbwr_id]);
		if (Status != XST_SUCCESS)
			xil_printf("ERROR:: Unable to configure Frame Buffer \ Write chroma buffer address\r\n");
	}
	/*************************************************************************/

	/***Find Vmix Layer assigned for this FBWR & update latest buffer with the layer********/
	/************************************************************************************/
	Frame_Count[fbwr_id]++;
}


/**
 * @brief Initialize Frame Buffer Writer for specific video pipeline
 *
 * This function initializes a specific frame buffer writer instance for a
 * video processing pipeline. It handles buffer allocation, format configuration,
 * and prepares the frame buffer writer for video output.
 *
 * Initialization steps:
 * 1. Allocate aligned memory buffers using memory manager
 * 2. Determine output format based on input parameters
 * 3. Configure video stream parameters
 * 4. Calculate memory stride and chroma offset
 * 5. Configure frame buffer writer with all parameters
 * 6. Enable auto-restart for continuous operation
 *
 * @param hpId - Hardware pipeline ID (identifies ISP instance)
 * @param bufIo - Buffer chain ID (MP/SP1/SP2 output selection)
 * @param outFormat - Output format specification (resolution, format, bit depth)
 * @param bufsize - Required buffer size in bytes
 * @return 0 on success, negative value on error
 *
 * @note Triple buffering is implemented for smooth video processing
 * @note Memory alignment is critical for DMA performance
 * @note Different output formats require different stride calculations
 */
int init_fbwr(u8 hpId, CamDeviceBufChainId_t bufIo, CamDevicePipeOutFmt_t outFormat, u64 bufsize)
{
	/* Calculate frame buffer writer ID based on hardware pipeline and buffer chain */
	XVidC_VideoMode resIdOut;
	u32 stride;
	u32 fbwr_id = bufIo + hpId * 2; /* Max 2 FBWR per ISP (MP & SP1) */
	u32 alignment = 32; /* Frame buffer alignment requirement for DMA */

	xil_printf("init FBWR[%d]\n", fbwr_id);

	/* Mark this FBWR as enabled for later processing */
	fbwr_enable[fbwr_id] = 1;

	/* Allocate triple buffers using memory manager */
	for (int i = 0; i < Buffer_Count; i++) {
		void *aligned_ptr;

		/* Use the new memory manager for aligned allocation */
		aligned_ptr = mm_aligned_malloc(bufsize, alignment, &Frame_Array_p[fbwr_id][i]);
		if (aligned_ptr == NULL) {
			xil_printf("Memory allocation failed for FBWR[%d] buffer[%d], size=%llu\n", fbwr_id, i, bufsize);
			mm_print_stats(); /* Print stats to help debug */
			return 1;
		}

		xil_printf("FBWR[%d] Buffer[%d]: allocated %llu bytes, aligned_addr=%p\n",
		     fbwr_id, i, bufsize, aligned_ptr);
	}

	/* Determine output format based on input format specification */
	if (outFormat.outFormat == CAMDEV_PIX_FMT_YUV422SP) {
		if (outFormat.dataBits == 8)
			outputformat_selection = YUV422_8bit;
		else if (outFormat.dataBits == 10)
			outputformat_selection = YUV422_10bit;

	} else if (outFormat.outFormat == CAMDEV_PIX_FMT_YUV420SP) {
		if (outFormat.dataBits == 8)
			outputformat_selection = YUV420_8bit ;
		else if (outFormat.dataBits == 10)
			outputformat_selection = YUV420_10bit;
	} else if (outFormat.outFormat == CAMDEV_PIX_FMT_YUV400) {
		if (outFormat.dataBits == 8)
			outputformat_selection = YUV400_8bit ;
		else if (outFormat.dataBits == 10)
			outputformat_selection = YUV400_10bit;
	} else if (outFormat.outFormat == CAMDEV_PIX_FMT_RGB888P
		   || outFormat.outFormat == CAMDEV_PIX_FMT_RGB888)
		outputformat_selection = RGB888_8bit;

	/* Map output format to Xilinx video common format definitions */
	switch (outputformat_selection) {
		case RGB888_8bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_RGB8;
			StreamOut.ColorFormatId = XVIDC_CSF_RGB ;
			break;

		case YUV420_8bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_Y_UV8_420 ;
			StreamOut.ColorFormatId = XVIDC_CSF_YCRCB_420 ;
			break;
		case YUV422_8bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_Y_UV8 ;
			StreamOut.ColorFormatId = XVIDC_CSF_YCRCB_422 ;
			break;
		case YUV422_10bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_Y_UV10 ;
			StreamOut.ColorFormatId = XVIDC_CSF_YCRCB_422 ;
			break;

		case YUV420_10bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_Y_UV10_420 ;
			StreamOut.ColorFormatId = XVIDC_CSF_YCRCB_420 ;
			break;

		case YUV400_10bit:
			FBWR_Cfmt[fbwr_id] = XVIDC_CSF_MEM_Y10 ;
			StreamOut.ColorFormatId = XVIDC_CSF_YONLY ;
			break;

		default :
			xil_printf("Invalid output format \n");
			return -1;

	}

	/* Get video mode ID for the specified resolution */
	resIdOut = XVidC_GetVideoModeId(outFormat.outWidth, outFormat.outHeight, XVIDC_FR_30HZ, FALSE);

	xil_printf("Display Format width: %d height: %d\n", outFormat.outWidth, outFormat.outHeight);

	/* Configure video stream parameters */
	StreamOut.VmId = resIdOut;
	StreamOut.Timing.HActive = outFormat.outWidth;
	StreamOut.Timing.VActive = outFormat.outHeight;
	StreamOut.FrameRate = XVIDC_FR_30HZ ;
	StreamOut.IsInterlaced = 0;

	xil_printf("Display Format width: %d height: %d\n", StreamOut.Timing.HActive, StreamOut.Timing.VActive);

	/* Setup stream parameters based on frame buffer writer capabilities */
	StreamOut.ColorDepth =
		(XVidC_ColorDepth) frmbufwr[fbwr_id].FrmbufWr.Config.MaxDataWidth;
	StreamOut.PixPerClk =
		(XVidC_PixelsPerClock) frmbufwr[fbwr_id].FrmbufWr.Config.PixPerClk;

	/* Calculate memory stride for the selected format */
	stride = CalcStride(FBWR_Cfmt[fbwr_id], frmbufwr[fbwr_id].FrmbufWr.Config.AXIMMDataWidth,
			    &StreamOut);

	/* Calculate chroma offset for semi-planar formats */
	if ((FBWR_Cfmt[fbwr_id] == XVIDC_CSF_MEM_Y_UV8)
	    || (FBWR_Cfmt[fbwr_id] == XVIDC_CSF_MEM_Y_UV8_420)
	    || (FBWR_Cfmt[fbwr_id] == XVIDC_CSF_MEM_Y_UV10)
	    || (FBWR_Cfmt[fbwr_id] == XVIDC_CSF_MEM_Y_UV10_420))
		chroma_offset[fbwr_id] = stride * StreamOut.Timing.VActive;

	/* Configure frame buffer writer with all parameters */
	int status = ConfigFrmbuf(stride, FBWR_Cfmt[fbwr_id], &StreamOut, chroma_offset[fbwr_id], fbwr_id);
	if (status < 0) {
		xil_printf("FBWR Config Failed\n\n\n\n");
		return -1;
	}

	usleep(20000);  /* Allow configuration to settle */

	/* Enable auto-restart for continuous operation */
	XV_frmbufwr_EnableAutoRestart(&frmbufwr[fbwr_id].FrmbufWr);

	xil_printf("FrmBufWr setup done .. \r\n");
	return 0;
}


/**
 * @brief Enable all configured Frame Buffer Writers
 *
 * This function enables all frame buffer writers that have been configured
 * and marked as enabled. It writes to the control register to start the
 * frame buffer writer operation.
 *
 * The enable sequence:
 * 1. Check if each FBWR is marked as enabled
 * 2. Write 0x81 to the control register (Enable + Start bits)
 * 3. Log the enable status for debugging
 *
 * @note Only frame buffer writers marked as enabled will be started
 * @note The value 0x81 = 0x80 (Enable) + 0x01 (Start) bits
 * @note This function should be called after all FBWR configuration is complete
 */
void enable_fbwr()
{
	for (int i = 0; i < NUM_FBWR; i++) {
		if (fbwr_enable[i] != 0) {
			/* Enable and start the frame buffer writer */
			Xil_Out32(frmbufwr[i].FrmbufWr.Config.BaseAddress, 0x81);
			xil_printf("Starting FBWR -%x %x \n", frmbufwr[i].FrmbufWr.Config.BaseAddress,
			     Xil_In32(frmbufwr[i].FrmbufWr.Config.BaseAddress));
		}
	}
}

/**
 * @brief Reset specific IP block using GPIO control
 *
 * This function resets a specific IP block using the LPD (Low Power Domain) GPIO
 * interface. It performs a controlled reset sequence to ensure proper IP initialization.
 *
 * Reset sequence:
 * 1. Configure GPIO direction as output
 * 2. Enable GPIO output
 * 3. Assert reset (low)
 * 4. Wait for reset propagation
 * 5. Deassert reset (high)
 * 6. Wait for IP to come out of reset
 *
 * @param Ip_ResetBit - Bit position for the IP reset in GPIO register
 *
 * @note Uses LPD GPIO registers at specific addresses
 * @note Reset timing is critical - 4ms delays ensure proper reset
 * @note Each IP block has its own reset bit position
 */
void Reset_IP(u8 Ip_ResetBit)
{
	/* LPD GPIO registers for IP reset control */
#define LPD_GPI0_data XPAR_XGPIO_0_BASEADDR+0x4c    /* GPIO data register */
#define LPD_GPI0_dir XPAR_XGPIO_0_BASEADDR+0x2c4    /* GPIO direction register */
#define LPD_GPI0_OE XPAR_XGPIO_0_BASEADDR+0x2c8     /* GPIO output enable register */

	/* Read current GPIO register values */
	int	Gpio_data = Xil_In32(LPD_GPI0_data);
	int	Gpio_dir = Xil_In32(LPD_GPI0_dir);
	int	Gpio_OE = Xil_In32(LPD_GPI0_OE);

	/* Configure GPIO direction as output for the reset bit */
	Gpio_dir |= (1 << Ip_ResetBit);
	Xil_Out32(LPD_GPI0_dir, Gpio_dir);
	usleep(4000);

	/* Enable GPIO output for the reset bit */
	Gpio_OE |= (1 << Ip_ResetBit);
	Xil_Out32(LPD_GPI0_OE, Gpio_OE);

	/* Assert reset (set bit low) */
	Gpio_data &= (~(1 << (Ip_ResetBit)));
	Xil_Out32(LPD_GPI0_data, Gpio_data);
	usleep(4000);

	/* Deassert reset (set bit high) */
	Gpio_data |= (1 << (Ip_ResetBit));
	Xil_Out32(LPD_GPI0_data, Gpio_data);
	usleep(4000);
}
#endif
