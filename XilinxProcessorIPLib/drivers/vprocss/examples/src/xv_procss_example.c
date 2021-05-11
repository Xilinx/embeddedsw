/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file main.c
*
* This is main file for the Video Processing Subsystem example design.
*
* The VPSS HW configuration is detected and several use cases are selected to
* test it.  These VPSS HW characteristics are configurable:
*  Topology:             6 cases - Full fledged, Scaler-only, ...
*  Pixels/clock:         3 cases - 1, 2, 4
*  Component/Pixel:      1 case - 3
*  Data Width/Component: 4 cases - 8, 10, 12, 16
*  Interlaced input:     2 cases - Allowed, Not allowed
*  Allow color formats:  3 cases - (RGB,444,422,420), (RGB,444,422), (RGB,444)
*  Max Width:  range is  64...3840
*  Max Height: range is  64...2160
*
* The video pipeline in the Example Design HW consists of the Test Pattern
* Generator driving the VPSS input.  The VPSS output is checked for video lock.
*
* On start-up the program reads the HW config and initializes its internal
* data structures.  Based on the HW capabilities, 2 use cases are selected.
* Testing a use case is done by:
*  1) Select an appropriate video input format, and program the Test Pattern
*     Generator and the VPSS input stage for this format.
*  2) Select an appropriate video output format, and program the VPSS output
*     stage and the Video Timing Controller for that format.
*  3) Start the HW, and poll the Lock status bit waiting for video Lock.
*     If you get Lock, the test reports "PASSED".  If there is no Lock
*     after several seconds the test reports "FAILED"
*  4) Optionally, go back and set up the next use case, repeating steps 1,2,3.
*
******************************************************************************/

#include <stdio.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "system.h"
#include "xvprocss_vdma.h"

#if XPAR_XCLK_WIZ_NUM_INSTANCES
#include "xclk_wiz.h"
#endif

/************************** Local Constants *********************************/
#define XVPROCSS_SW_VER "v2.00"
#define VERBOSE_MODE 0
#define TOPOLOGY_COUNT 6
#define USECASE_COUNT 2
#define VIDEO_MONITOR_LOCK_TIMEOUT (2000000)

#define PROC_DELAY 2

#ifdef XPAR_GPIO_0_BASEADDR
#define GPIO_BASE XPAR_GPIO_0_BASEADDR
#endif

#ifdef XPAR_AXI_VDMA_0_BASEADDR
#define VDMA_BASE XPAR_AXI_VDMA_0_BASEADDR
#endif

#if XPAR_XCLK_WIZ_NUM_INSTANCES
/*
* The following constants map to the names of the hardware instances.
* They are only defined here such that a user can easily change all the
* needed device IDs in one place.
*/
#define XCLK_WIZ_DEVICE_ID		XPAR_CLK_WIZ_0_DEVICE_ID


/*
* change the XCLK_WIZ_DYN_DEVICE_ID value as per the Clock wizard
* whihc is setting as dynamic reconfiguration. In the present
* example clokc wizard 1 configured as clock wizard 1 as dynamic
* reconfigurable parameter
*/
#define XCLK_WIZ_DYN_DEVICE_ID		XPAR_CLK_WIZ_0_DEVICE_ID

/*
* The following constants are part of clock dynamic reconfiguration
* They are only defined here such that a user can easily change
* needed parameters
*/

#define CLK_LOCK			1

/*FIXED Value */
#define VCO_FREQ			1200
#define CLK_WIZ_VCO_FACTOR		(VCO_FREQ * 10000)

 /*Input frequency in MHz */
#define DYNAMIC_INPUT_FREQ		100
#define DYNAMIC_INPUT_FREQ_FACTOR	(DYNAMIC_INPUT_FREQ * 10000)

/*
 * Output frequency in MHz. User need to change this value to
 * generate grater/lesser interrupt as per input frequency
 */
#define DYNAMIC_OUTPUT_FREQ		175
#define DYNAMIC_OUTPUT_FREQFACTOR	(DYNAMIC_OUTPUT_FREQ * 10000)

#define CLK_WIZ_RECONFIG_OUTPUT		DYNAMIC_OUTPUT_FREQ
#define CLK_FRAC_EN			1





/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/

u32 ClkWiz_IntrExample(u32 DeviceId, u32 outfreq);
//int SetupInterruptSystem(INTC *IntcInstancePtr, XClk_Wiz *ClkWizPtr);
void XClk_Wiz_IntrHandler(void *InstancePtr);
void XClk_Wiz_InterruptEnable(XClk_Wiz *InstancePtr, u32 Mask);
int Clk_Wiz_Reconfig(XClk_Wiz_Config *CfgPtr_Dynamic, u32 outfreq);
int Wait_For_Lock(XClk_Wiz_Config *CfgPtr_Dynamic);

/* Interrupt helper functions */
void ClkWiz_ClkOutOfRangeEventHandler(void *CallBackRef, u32 Mask);
void ClkWiz_ClkGlitchEventHandler(void *CallBackRef, u32 Mask);
void ClkWiz_ClkStopEventHandler(void *CallBackRef, u32 Mask);

/************************** Variable Definitions *****************************/
XClk_Wiz ClkWiz_Mon;   /* The instance of the ClkWiz_Mon */
XClk_Wiz ClkWiz_Dynamic; /* The instance of the ClkWiz_Dynamic */
//XIntc InterruptController;  /* The instance of the Interrupt Controller */

volatile u8 Clk_Outof_Range_Flag = 1;
volatile u8 Clk_Glitch_Flag = 1;
volatile u8 Clk_Stop_Flag = 1;
#endif

/************************** Local Typedefs **********************************/
typedef struct {
  u16 width_in;
  u16 height_in;
  XVidC_ColorFormat Cformat_in;
  u16 Pattern;
  u16 IsInterlaced;

  u16 width_out;
  u16 height_out;
  XVidC_ColorFormat Cformat_out;
} vpssVideo;

/************************** Local Routines **********************************/
static void check_usecase(XVprocSs *VpssPtr, vpssVideo *useCase);

static int setup_video_io(
                 XPeriph *PeriphPtr, XVprocSs *VpssPtr, vpssVideo *useCase);

static int start_system(XPeriph *PeriphPtr, XVprocSs *VpssPtr);

/************************** Variable Definitions *****************************/
XPeriph  PeriphInst;
XVprocSs VprocInst;
const char topo_name[XVPROCSS_TOPOLOGY_NUM_SUPPORTED][32]={
        "Scaler-only",
        "Full",
        "Deint-only",
        "Csc-only",
        "Vcr-only",
        "Hcr-only"};

vpssVideo useCase[TOPOLOGY_COUNT][USECASE_COUNT] =
  //scaler only
  {{{1920, 1080, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1280, 720, XVIDC_CSF_YCRCB_444                             },
    {640,  480, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
      3840,  2160, XVIDC_CSF_YCRCB_420                             }},
  //full fledged
   {{3840, 2160, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1280, 720, XVIDC_CSF_YCRCB_420                             },
	{ 720,  240, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, TRUE,
	1920, 1080, XVIDC_CSF_RGB                                   }},
  //deinterlacer only
   {{1920,  540, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, TRUE,
     1920, 1080, XVIDC_CSF_YCRCB_444                             },
    { 720,  240, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, TRUE,
      720,  480, XVIDC_CSF_YCRCB_444                             }},
   //color space conversion only
   {{1920, 1080, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_RGB                                   },
    {1280,  720, XVIDC_CSF_RGB, XTPG_BKGND_COLOR_BARS,       FALSE,
     1280,  720, XVIDC_CSF_YCRCB_444                             }},
   //vertical chroma resample only
   {{1920, 1080, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_422                             },
    {1920, 1080, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_420                             }},
   //horizontal chroma resample only
   {{1920, 1080, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_444                             },
    {1920, 1080, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_422                             }}};

void reset_video_ips(void)
{
	u32 count;
	*(u32 *)(GPIO_BASE + 0x00) = 0xF;
	for (count = 0; count < 1000; count++);
	*(u32 *)(GPIO_BASE + 0x00) = 0x0;
	for (count = 0; count < 1000; count++);
	*(u32 *)(GPIO_BASE + 0x00) = 0xF;
	for (count = 0; count < 1000; count++);
	xil_printf("\n\r Video IPs reset done \n\r");
}


#if XPAR_XCLK_WIZ_NUM_INSTANCES

int Wait_For_Lock(XClk_Wiz_Config *CfgPtr_Dynamic)
{
	u32 Count = 0;
	u32 Error = 0;

	while(!(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK)) {
		if(Count == 10000) {
			Error++;
			break;
		}
		Count++;
        }
    return Error;
}

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

/*****************************************************************************/
/**
*
* This is the Clk_Wiz_Reconfig function, it will reconfigure frequencies as
* per input array
*
* @param	CfgPtr_Dynamic provides pointer to clock wizard dynamic config
* @param	Findex provides the index for Frequency divide register
* @param	Sindex provides the index for Frequency phase register
*
* @return
*		-  Error 0 for pass scenario
*		-  Error > 0 for failure scenario
*
* @note	 None
*
******************************************************************************/
int Clk_Wiz_Reconfig(XClk_Wiz_Config *CfgPtr_Dynamic, u32 outfreq)
{
    u32 Count = 0;
    u32 Error = 0;
    u32 Fail  = 0;
    u32 Frac_en = 0;
    u32 Frac_divide = 0;
    u32 Divide = 0;
    float Freq = 0.0;

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	Error++;
        xil_printf("\n ERROR: Clock is not locked for default frequency" \
	" : 0x%x\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* SW reset applied */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x00) = 0xA;

    if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
	Error++;
        xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "\
	  "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* Wait cycles after SW reset */
    for(Count = 0; Count < 2000; Count++);

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	  Error++;
          xil_printf("\n ERROR: Clock is not locked after SW reset :"
	      "0x%x \t Expected  : 0x1\n\r",
	      *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* Calculation of Input Freq and Divide factors*/
    Freq = ((float) CLK_WIZ_VCO_FACTOR/ DYNAMIC_INPUT_FREQ_FACTOR);

    Divide = Freq;
    Freq = (float)(Freq - Divide);

    Frac_divide = Freq * 10000;

    if(Frac_divide % 10 > 5) {
	   Frac_divide = Frac_divide + 10;
    }
    Frac_divide = Frac_divide/10;

    if(Frac_divide > 1023 ) {
	   Frac_divide = Frac_divide / 10;
    }

    if(Frac_divide) {
	   /* if fraction part exists, Frac_en is shifted to 26
	    * for input Freq */
	   Frac_en = (CLK_FRAC_EN << 26);
    }
    else {
	   Frac_en = 0;
    }

    /* Configuring Multiply and Divide values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x200) = \
	Frac_en | (Frac_divide << 16) | (Divide << 8) | 0x01;
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x204) = 0x00;

    /* Calculation of Output Freq and Divide factors*/
    Freq = ((float) CLK_WIZ_VCO_FACTOR / outfreq);

    Divide = Freq;
    Freq = (float)(Freq - Divide);

    Frac_divide = Freq * 10000;

    if(Frac_divide%10 > 5) {
	Frac_divide = Frac_divide + 10;
    }
    Frac_divide = Frac_divide / 10;

    if(Frac_divide > 1023 ) {
        Frac_divide = Frac_divide / 10;
    }

    if(Frac_divide) {
	/* if fraction part exists, Frac_en is shifted to 18 for output Freq */
	Frac_en = (CLK_FRAC_EN << 18);
    }
    else {
	Frac_en = 0;
    }

    /* Configuring Multiply and Divide values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x208) =
	    Frac_en | (Frac_divide << 8) | (Divide);
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x20C) = 0x00;

    /* Load Clock Configuration Register values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x07;

    if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
	Error++;
        xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "
	    "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

    /* Clock Configuration Registers are used for dynamic reconfiguration */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x02;

    Fail = Wait_For_Lock(CfgPtr_Dynamic);
    if(Fail) {
	Error++;
        xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
	": 0x1\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    } else {
	xil_printf("\n Clock is configured for %d MHz \n\r",(outfreq/10000));
    }

    return Error;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the interrupt example using the
* XClk_Wiz driver. This function will set up the system with interrupts
* handlers.
*
* @param	DeviceId is the unique device ID of the CLK_WIZ
*		Subsystem core.
*
* @return
*		- XST_FAILURE if the system setup failed.
*		- XST_SUCCESS should never return since this function, if setup
*		was successful, is blocking.
*
* @note		If system setup was successful, this function is blocking in
*		order to illustrate interrupt handling taking place for HPD
*		events.
*
******************************************************************************/
u32 ClkWiz_IntrExample(u32 DeviceId, u32 outfreq)
{
	XClk_Wiz_Config *CfgPtr_Mon;
	XClk_Wiz_Config *CfgPtr_Dynamic;
	ULONG Exit_Count = 0;
	u32 Status = XST_SUCCESS;

	CfgPtr_Mon = XClk_Wiz_LookupConfig(DeviceId);
	if (!CfgPtr_Mon) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ driver so that it is ready to use.
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Mon, CfgPtr_Mon,
					CfgPtr_Mon->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Check the given clock wizard is enabled with clock monitor
	 * This test applicable only for clock monitor
	 */
//	if(CfgPtr_Mon->EnableClkMon == 0) {
//		xil_printf("Interrupt test only applicable for "
//			"clock monitor\r\n");
//		return XST_SUCCESS;
//	}

	/*
	 * Get the CLK_WIZ Dynamic reconfiguration driver instance
	 */
	CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XCLK_WIZ_DYN_DEVICE_ID);
	if (!CfgPtr_Dynamic) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ Dynamic reconfiguration driver
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
		 CfgPtr_Dynamic->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the CLK_WIZ to the interrupt subsystem such that interrupts can
	 * occur. This function is application specific.
	 */

//	Status = SetupInterruptSystem(IntcInstancePtr, &ClkWiz_Mon);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}

	/* Calling Clock wizard dynamic reconfig */
	Clk_Wiz_Reconfig(CfgPtr_Dynamic, outfreq);

	/* Enable interrupts after setup interrupt */
//	XClk_Wiz_InterruptEnable(&ClkWiz_Mon, XCLK_WIZ_IER_ALLINTR_MASK);
//
//	do {
//		Delay(1);
//		Exit_Count++;
//		if(Exit_Count > 3) {
//			xil_printf("ClKMon Interrupt test failed, " \
//				"Please check design\r\n");
//			return XST_FAILURE;
//		}
//	}
//	while((Clk_Outof_Range_Flag == 1) && (Clk_Glitch_Flag == 1) \
//		&& (Clk_Stop_Flag == 1));
	return XST_SUCCESS;
}

#endif

int main(void)
{
  XPeriph *PeriphPtr;
  XVprocSs *VpssPtr;

  vpssVideo *thisCase;
  int status, cnt;
  u32 Timeout;
  static int Lock = FALSE;

  /* Bind instance pointer with definition */
  PeriphPtr = &PeriphInst;
  VpssPtr   = &VprocInst;

  /* Initialize ICache */
  Xil_ICacheInvalidate();
  Xil_ICacheEnable();

  /* Initialize DCache */
  Xil_DCacheInvalidate();
  Xil_DCacheEnable();

  xil_printf("\r\n--------------------------------------------------------\r\n");
  xil_printf("  Video Processing Subsystem Example Design %s\r\n", XVPROCSS_SW_VER);
  xil_printf("  (c) 2015, 2016 by Xilinx Inc.\r\n");

  status = XSys_Init(PeriphPtr, VpssPtr);
  if(status != XST_SUCCESS) {
     xil_printf("CRITICAL ERROR:: System Init Failed. Cannot recover from this error. Check HW\n\r");
  }

  /* Based on the customized Video Processing Subsystem functionality
   * the video input and output formats are chosen.
   */

  status = 0;
  cnt = 0;
  while (cnt < USECASE_COUNT) {
    xil_printf("--------------------------------------------------------\r\n");
    printf("Topology is %s, case %d\r\n",topo_name[VpssPtr->Config.Topology],cnt+1);

    reset_video_ips();

    thisCase = &useCase[VpssPtr->Config.Topology][cnt];

    switch (VpssPtr->Config.Topology) {
      case XVPROCSS_TOPOLOGY_SCALER_ONLY:
        // Choose video format based on the "422, 420, and CSC Enabled" option
        // Video In: 720P Video Out: 1080P
        thisCase->Cformat_in = XV_HscalerIs420Enabled(VpssPtr->HscalerPtr)?
                     XVIDC_CSF_YCRCB_420 :
                     (XV_HscalerIs422Enabled(VpssPtr->HscalerPtr) ? XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444);
        thisCase->Cformat_out = XV_HscalerIsCscEnabled(VpssPtr->HscalerPtr)? XVIDC_CSF_RGB : XVIDC_CSF_YCRCB_444;
        break;

      case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
        // Full Fledged mode may deinterlace, change picture size and/or color format.
        // In the Full Fledged configuration, the presence of a sub-core
        //   is indicated by a non-NULL pointer to the sub-core driver instance.
	//Check if FULL topology has any sub-cores excluded
	if((VpssPtr->DeintPtr      == NULL) ||  //No interlaced support
	   (VpssPtr->VcrsmplrInPtr == NULL) ||  //No 420 support
		   (VpssPtr->HcrsmplrPtr   == NULL)) {  //No 422 support

          //Overwrite default test case with specific ones
          // If there is no Deinterlacer AND 420 input is supported (Vcr present),
          //   choose progressive 420 input format
          if ((VpssPtr->DeintPtr      == NULL) &&
              (VpssPtr->VcrsmplrInPtr != NULL)) {
          // Video In: 720P 420  Video Out: 1080P RGB
            thisCase->width_in = 1280;
            thisCase->height_in = 720;
            thisCase->Cformat_in = XVIDC_CSF_YCRCB_420;
            thisCase->IsInterlaced = FALSE;

          // If the Deinterlacer is present,
          //   choose 480i interlaced input 422 (Hcr present) or 444 (Hcr absent)
          } else {
            if (VpssPtr->DeintPtr != NULL) {
            // Video In: 480i YUV  Video Out: 1080P RGB
              thisCase->width_in = 720;
              thisCase->height_in = 240;
              thisCase->Cformat_in = (VpssPtr->HcrsmplrPtr != NULL)?
                XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444;
              thisCase->IsInterlaced = TRUE;
            }
          }
	} else {
	  //NOP - use default test cases

	}
        break;

      default:
        break;
    }

    xil_printf("Set up Video Input and Output streams.\r\n");
    status = setup_video_io(PeriphPtr, VpssPtr, thisCase);
    if (status != XST_SUCCESS) {
	xil_printf ("Failed to setup io\r\n");
	goto INFINITE_LOOP;
    }

#ifndef XPS_BOARD_VCK190
#if XPAR_XCLK_WIZ_NUM_INSTANCES
    u32 ClockFreq = 0;
    ClockFreq = (XVidC_GetPixelClockHzByVmId(VpssPtr->VidOut.VmId) / 1000000);
    if (ClockFreq > 150)
	ClockFreq = 150;
    xil_printf ("Start VPSS --> %d MHz\r\n", ClockFreq);

    ClockFreq = ClockFreq*10000;

	ClkWiz_IntrExample(XCLK_WIZ_DEVICE_ID, ClockFreq);
#endif
#endif

    //Configure and start VTC with output timing
    xil_printf("\n\rStart VTC.\r\n");
    XPeriph_ConfigVtc(PeriphPtr,
                    &VpssPtr->VidOut,
                    VprocInst.Config.PixPerClock);

    //Configure and start the TPG
    printf ("Start TPG.\r\n");
    XPeriph_ConfigTpg(PeriphPtr);

    xil_printf("\n\rStart VPSS...");
    status = start_system(PeriphPtr, VpssPtr);

    if(status != XST_SUCCESS)
    {
        xil_printf("\r\nERROR:: Test Failed\r\n");
        xil_printf("    ->VProcss Configuration Failed. \r\n");
    }

    xil_printf("Done...\n\r");

    XVprocSs_ReportSubsystemConfig(VpssPtr);

    /* check for output lock */
    xil_printf("Waiting for lock... ");
    Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;
    while(!Lock && Timeout) {
      status = XPeriph_IsVideoLocked(PeriphPtr);
	if (status & 0x1) {
		xil_printf("Locked.\r\n");
		Lock = TRUE;
	} else if (status & 0x2) {
		xil_printf("Underflow.\r\n");
		Lock = TRUE;
		Timeout = 0;
		break;
	} else if (status & 0x4) {
		xil_printf("Underflow.\r\n");
		Lock = TRUE;
		Timeout = 0;
		break;
	} else
		--Timeout;
    }

    if(!Timeout) {
      xil_printf("\r\nERROR:: Test Failed\r\n");
    } else {
      xil_printf("\r\nTest Completed Successfully\r\n\r\n");
    }

    xil_printf("Stop... ");
    XVprocSs_Stop(VpssPtr);

    // In the Deint-only configuration, it is necessary to allow
    // some time for aximm traffic to stop, and the core to become idle
    if (XVprocSs_IsConfigModeDeinterlaceOnly(VpssPtr)) {
      if (XV_DeintWaitForIdle(VpssPtr->DeintPtr) == XST_SUCCESS)
        xil_printf ("Deint subcore IDLE.\r\n");
    else
        xil_printf ("ERROR:: Deint subcore NOT IDLE.\r\n");
    }


#if VERBOSE_MODE
    XVprocSs_LogDisplay(VpssPtr);
#endif

    xil_printf ("End testing this use case.\r\n");
    Lock = FALSE;
    cnt++;
  }
  xil_printf ("VPSS Exdes Test completed\r\n");

INFINITE_LOOP:
  while(1) {
    //NOP
  }

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @local routine setup_video_io()
*
*  1) Program the TPG and the VPSS input stage to the input video format.
*  2) Program the VPSS output stage to the output video format.
*
*  @return Returns XST_SUCCESS, if stream configured properly,
*  	   otherwise XST_FAILURE.
*
******************************************************************************/

static int setup_video_io(XPeriph *PeriphPtr, XVprocSs *VpssPtr,
			   vpssVideo *useCase)
{
	int status = XST_FAILURE;

	/* depending on HW config, optionally modify the in/out formats */
	check_usecase(VpssPtr, useCase);

	/*
	 * Test Pattern Generator is the video source for the example design
	 * Set Test Pattern Generator parameters
	 */
	XPeriph_SetTpgParams(PeriphPtr, useCase->width_in, useCase->height_in,
			     useCase->Cformat_in, useCase->Pattern,
			     useCase->IsInterlaced);

	/*
	 * Set VPSS Video Input AXI Stream to match the TPG
	 * Note that framerate is hardwired to 60Hz in the example design
	 */
	status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_IN,
				     PeriphInst.TpgConfig.Width,
				     PeriphInst.TpgConfig.Height, XVIDC_FR_60HZ,
				     PeriphInst.TpgConfig.ColorFmt,
				     PeriphInst.TpgConfig.IsInterlaced);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Set VPSS Video Output AXI Stream
	 * Note that output video is always progressive
	 */
	status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_OUT,
				     useCase->width_out, useCase->height_out,
				     XVIDC_FR_60HZ, useCase->Cformat_out,
				     FALSE);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	return status;
}

/*****************************************************************************/
/**
*
* @local routine start_system()
*
*  Configure and Start the video system.
*
*  @return Returns XST_SUCCESS for successful configuration, else XST_FAILURE.
*
******************************************************************************/
static int start_system(XPeriph *PeriphPtr, XVprocSs *VpssPtr)
{
    int status;
      // For single-IP VPSS cases only, reset is handled outside vpss
      if (XVprocSs_IsConfigModeCscOnly(VpssPtr)          ||
          XVprocSs_IsConfigModeDeinterlaceOnly (VpssPtr) ||
          XVprocSs_IsConfigModeHCResampleOnly(VpssPtr)   ||
          XVprocSs_IsConfigModeVCResampleOnly(VpssPtr)
         ) {
        XPeriph_ResetHlsIp(PeriphPtr);
      }

      // Configure and Start the VPSS IP
      // (reset logic for multi-IP VPSS cases is done here)
      status = XVprocSs_SetSubsystemConfig(VpssPtr);

      return status;
}

/*****************************************************************************/
/**
*
* @local routine check_usecase()
*
*  Confine the useCase to the Height and Width restrictions of the HW.
*  Height and Width data in proposed useCase are altered if necessary.
*
*  @return Returns void.
*
******************************************************************************/
static void check_usecase(XVprocSs *VpssPtr, vpssVideo *useCase)
{
  u16 *width_in   = &useCase->width_in;
  u16 *width_out  = &useCase->width_out;
  u16 *height_in  = &useCase->height_in;
  u16 *height_out = &useCase->height_out;

    // check/correct Max Width
    if(*width_in > VpssPtr->Config.MaxWidth)
      *width_in = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_in % VpssPtr->Config.PixPerClock) != 0)
    *width_in -= (*width_in % VpssPtr->Config.PixPerClock);

  // check/correct Max Width
  if(*width_out > VpssPtr->Config.MaxWidth)
    *width_out = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_out % VpssPtr->Config.PixPerClock) != 0)
    *width_out -= (*width_out % VpssPtr->Config.PixPerClock);

  // check/correct Max Height
  if(*height_in > VpssPtr->Config.MaxHeight)
    *height_in = VpssPtr->Config.MaxHeight;

  // check/correct Max Height
  if(*height_out > VpssPtr->Config.MaxHeight)
    *height_out = VpssPtr->Config.MaxHeight;
}
