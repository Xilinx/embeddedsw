/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2014 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file scaler_intgtest.h
*
* This header file contains the integration test functions prototypes,
* constants and variable definitions used in the test. This test runs on a
* Zynq702 system.
*
* @note		None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------------
* 7.0   adk   22/08/14 First release.
* </pre>
*
******************************************************************************/

#ifndef SCALER_INTEGTEST_H
#define SCALER_INTEGTEST_H	/**< Prevent circular inclusions  by using
				  *  protection macros. */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/*
 * Includes that define system hardware constants.
 */
#include "xparameters.h"

#include "xscaler.h"
#include "xscugic.h"
#include "xstatus.h"
#include "xil_testlib.h"

/*
 * Includes that define test utility APIs.
 */
#include "ct.h"
#include "xil_exception.h"

/************************** Constant Definitions *****************************/

#define LOOP_TIME_OUT		0x000FFFFF	/**< Loop time out macro */

/*
 * Constants indicating INTC device ID and interrupt ID. They are defined in
 * xparameters.h.
 */
#ifndef SCALER_XSCALER_0_INTR
#define SCALER_XSCALER_0_INTR	92	/**<SCALER Interrupt */
#endif
#define SCALER_0_DEVICE_ID	XPAR_XSCALER_0_DEVICE_ID /**<SCALER Device
							   * Id */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID /**<INTC Device
							       * Id */
#define SCALER_INT_IRQ_ID	SCALER_XSCALER_0_INTR	/**<Scaler Interrupt
							  * IRQ Id */
#define XSCALAR_DEVICEID	5			/**< Random device id*/

/* Pele Regression tests */
#define AUTOMATIC_TEST_MODE		/**<Automatic Test Mode */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/

/* Instance of the Scaler driver. */
extern XScaler ScalerInst;

/* Instance of the XScuGic driver. */
extern XScuGic InterruptController;

/* User command storage */
extern char CmdLine[132];

/************************** Function Prototypes ******************************/

/* Basic self test implemented in scaler_intg_basic.c. */
int Scaler_Intg_SelfTest(int TestLoops);

/* Polled mode test implemented in scaler_intg_polled.c. */
/*int Scaler_Intg_PolledTest(int TestLoops);*/

/* Interrupt mode test implemented in scaler_intg_intr.c. */
int Scaler_Intg_InterruptTest(int TestLoops);

/* Scaler initialization that restores the format and operation mode in
 * scaler_intg_util.c.
 */
int Scaler_Initialize(XScaler *InstancePtr, u16 DeviceID);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro. */
