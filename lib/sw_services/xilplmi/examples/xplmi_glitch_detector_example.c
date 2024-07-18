/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_glitch_detector_example.c
 *
 * This file illustrates glitch detector usage
 * To build this application, xilmailbox and xilpm libraries must be included in BSP and
 * xilplmi library must be in client mode
 *
 * This example is supported for Versal and Versal Net devices.
 *
 * Procedure to run the example.
 * ------------------------------------------------------------------------------------------------
 * Generate pdi with glitch_detection_overlay cdo.
 * Load the Pdi.
 * Select the target.
 * Download the example elf into the target.
 *
 * Procedure to test glitch on hardware
 * ------------------------------------------------------------------------------------------------
 * Refer versal plm wiki to configure the glitch detector
 *
 * This code intends to provide an example to test the glitch detector functionality by generating
 * the test glitch. User must disable that and have a programmable power supply for Vcc_PMC and
 * generate the actual glitch by varying the power supply
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- ---------- --------------------------------------------------------------------------
 * 1.00  pre  06/09/2024 Initial release
 *       pre  07/15/2024 Added support for SDT flow and fixed misrac warnings
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xplmi_client_example_apis XilPlmi Client Example APIs
 * @{
 */

/*************************************** Include Files *******************************************/
#include "xplmi_glitchdetector.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "sleep.h"
#include "xscugic.h"
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
#include "pm_api_sys.h"
#include "xipipsu.h"
#else
#include "xil_io.h"
#endif

/************************************ Constant Definitions ***************************************/
#define GD_STATUS_OFFSET         0x00004 /**< GD_STATUS register offset */
#define RESET_GD_STATUS_VAL      0x02000200U /**< Value to reset glitch detectors */
#define GD_IRQ_STATUS_CLEAR      0x80000000U /**< Value to clear IRQ status of glitch detector */
#define GD0_TEST_GLITCH_GENVALUE 0x00F800FEU /**< Value to generate test glitch on
                                             glitch detector0*/
#define GD1_TEST_GLITCH_GENVALUE 0x00FE00F8U /**< Value to generate test glitch on
                                             glitch detector1*/
#define GD_TEST_GLITCH_STOPVALUE 0x00F800F8U /**< Value to stop test glitch generation on
                                             both the glitch detectors */

#define GLITCH_DETECTOR0         (0U)  /**< Glitch detector0 number */
#define GLITCH_DETECTOR1         (1U) /**< Glitch detector1 number */
#define DEFAULT_DEPTH_VAL        (0U) /**< Default depth value */
#define DEFAULT_WIDTH_VAL        (0U) /**< Default width value */
#define DEFAULT_REFVOL_VAL       (0U) /**< Default reference voltage value */
#define DEFAULT_USERREG_VAL      (0U) /**< Default user reg value */
#define XPLMI_SET                (1U) /**< Set value */
#define XPLMI_RESET              (0U) /**< Reset value */

#define GICP4_IRQ_STATUS_ADDR    0xF1140050 /**< GICP4_IRQ_STATUS register address */
#define DEVICE_INT_ID            191U /* when using for PL,change interrupt ID here */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void GlitchDetectorISR(void);

/************************************ Variable Definitions ***************************************/
static u8 GlitchDetected0; /* Flag to indicate glitch detection on glitch detector0 */
static u8 GlitchDetected1; /* Flag to indicate glitch detection on glitch detector1 */

static XScuGic InterruptController; /* Instance of the Interrupt Controller */
static XScuGic_Config *GicConfig; /* The configuration parameters of the controller */

/*************************************************************************************************/
/**
 * @brief	This is interrupt service routine of glitch detector. User code
 *          can be written here. Some default actions like reading the glitch detector status and
 *          clearing IRQ status, disabling interrupt on entry and enabling at exit is done here.
 *
  ************************************************************************************************/
static void GlitchDetectorISR(void)
{
	u32 RegVal;

	XScuGic_Disable(&InterruptController, DEVICE_INT_ID);

	/* Reading glitch detector status */
	XPlmi_ReadReg32(GD_STATUS_OFFSET, &RegVal);

	/* Reset glitch detectors */
	XPlmi_WriteReg32(GD_CTRL_OFFSET, RESET_GD_STATUS_VAL);

	/* Clear IRQ status of glitch detector */
	Xil_Out32(GICP4_IRQ_STATUS_ADDR, GD_IRQ_STATUS_CLEAR);

	xil_printf("\n\rGlitchDetectorISR(): GD0 = %d, GD1 = %d", (RegVal & 1U), ((RegVal >> 1)& 1U));

	if(RegVal & 1U) {
		GlitchDetected0 = XPLMI_SET;
	}
	if(((RegVal >> 1)& 1U)) {
		GlitchDetected1 = XPLMI_SET;
	}

	XScuGic_Enable(&InterruptController, DEVICE_INT_ID);
}

/*************************************************************************************************/
/**
 * @brief	This function generates glitch on given glitch detector
 *
 * @param    GdNum is the glitch detector number
*
**************************************************************************************************/
void GenerateTestGlitch(u8 GdNum)
{
	u32 value;

	if (GdNum == GLITCH_DETECTOR0) {
		value = GD0_TEST_GLITCH_GENVALUE;
	} else if (GdNum == GLITCH_DETECTOR1) {
		value = GD1_TEST_GLITCH_GENVALUE;
	}

	xil_printf("GenerateTestGlitch(): Generate test glitch (0x%08x: 0x%08x)\n\r",
           (PMC_ANALOG_BASE_ADDR + GD_CTRL_OFFSET), value);

	/* Generate glitch */
	XPlmi_WriteReg32(GD_CTRL_OFFSET, value);

	XPlmi_WriteReg32(GD_CTRL_OFFSET, GD_TEST_GLITCH_STOPVALUE);
}

int main()
{
    int Status = XST_FAILURE;
	/**
	 * To generate glitch using test mode, configure glitch detectors with default values.
	 * To configure glitch detector to detect glitch of at least 5% depth, 1ns width with 0.7V
	 * reference voltage, provide Depth = 0x0E, Width = 0x01, RefVoltage = 0x04.To select the
	 * register values instead of efuse values, provide UserRegVal = 0x01
	 */
    u8 Depth = DEFAULT_DEPTH_VAL;
    u8 Width = DEFAULT_WIDTH_VAL;
    u8 RefVoltage = DEFAULT_REFVOL_VAL;
    u8 UserRegVal = DEFAULT_USERREG_VAL;
    u8 GlitchDetectorNum = GLITCH_DETECTOR1;

#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	XIpiPsu IpiInst;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
#ifndef SDT
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);
#else
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		xil_printf("IPI lookup config failed\n");
		goto END;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
				       IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("ERROR #%d in configuring IPI\n", Status);
		goto END;
	}

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(&IpiInst, XIPIPSU_ALL_MASK);

	/* XilPM Initialize */
	Status = XPm_InitXilpm(&IpiInst);
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_InitXilpm() failed with error: %d\r\n", Status);
		goto END;
	}

	/* Finalize Initialization */
	Status = XPm_InitFinalize();
	if (XST_SUCCESS != Status) {
		xil_printf("XPm_initfinalize() failed\r\n");
		goto END;
	}
#endif

#ifndef SDT
	GicConfig = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
#else
	GicConfig = XScuGic_LookupConfig(XPAR_XSCUGIC_0_BASEADDR);
#endif
	if (NULL == GicConfig) {
		xil_printf("SCU GIC lookup configuration failed\r\n");
		goto END;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
				       GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("GIC configuration initialization failed\r\n");
		goto END;
	}

    XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
    XScuGic_Connect(&InterruptController, DEVICE_INT_ID, (Xil_InterruptHandler) GlitchDetectorISR,
	               0);

    XScuGic_Enable(&InterruptController, DEVICE_INT_ID);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
	                            (Xil_ExceptionHandler) XScuGic_InterruptHandler,
	                            (void *) &InterruptController);
    Xil_ExceptionEnable();

	Status = XPlmi_ConfigureGlitchDetector(Depth, Width, RefVoltage, UserRegVal,
	                                       GlitchDetectorNum);
    if (Status != XST_SUCCESS) {
	xil_printf("Configuration reset failed\n\r");
		goto END;
    }

    GenerateTestGlitch(GLITCH_DETECTOR0);
	sleep(10);

    GenerateTestGlitch(GLITCH_DETECTOR1);
	sleep(10);

END:
	if((GlitchDetected0 == XPLMI_SET) && (GlitchDetected1 == XPLMI_SET) &&
	  (Status == XST_SUCCESS)) {
		xil_printf("Successfully ran glitch detector example\n\r");
	}
	else {
		xil_printf("Glitch detector example failed with error code:%08x\n\r",Status);
	}
    return 0;
}
