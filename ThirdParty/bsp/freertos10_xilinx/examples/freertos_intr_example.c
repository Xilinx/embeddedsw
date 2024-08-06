/*
 * FreeRTOS Kernel V10.6.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2020-2022 Xilinx, Inc. All Rights Reserved.
 * Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/**
*
* @file freertos_intr_example.c
*
* Implements example that demonstrates interrupt handling in FreeRTOS
* based applications. This example can be used on Cortex-A9,
* Cortex-A53 (64 bit mode), Cortex-A72 and Cortex-R5, Microblaze based
* platforms. For Microblaze based platforms interrupt handling would
* be demonstrated through AXI Timer IP, and for other processors, it
* would be demonstrated through TTC1.
*
* This appplication consist of only 1 user defined task "prvTimerTask".
* prvTimerTask task would configure timer device, register it's interrupt
* handler, with interrupt controller initialized by porting layer.
* Once timer is kicked off, prvTimerTask would wait for semaphore.
* Semaphore would be released by interrupt handler of timer device.
* Once semaphore is obtained, task would display success message and
* delete itself.
*
* Follownig are HW design dependencies for defferent platforms,

* --------------------------------------------------
* Processor          |       HW design dependency  |
*--------------------|-----------------------------|
*  Microblaze        |  2 AXI TIMER IP's, interrupt
*                    |  output of both of them     |
*                    |  should be connected to     |
*..                  |  AXI INTC                   |
* -------------------------------------------------|
* Other supported    |  TTC0 and TTC1 instances of |
* processors         |  ttcps IP should be enabled |
*                    |  in HW design               |
*                    |                             |
*--------------------------------------------------
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.3   mus  08/24/20  First release of example which demonstrates interrupt
*                      handling in FreeRTOS based applications.
* 1.14  asa  06/23/23  Update the timer interrupt id to support use
*                      cases where xiltimer or SDT is enabled.
* 1.15  dp   01/30/24  Update the example to support SDT flow.
* 1.16  ml   07/26/24  Add support for SDT flow.
* </pre>
******************************************************************************/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "portmacro.h"

/* Xilinx includes. */
#include "xil_exception.h"
#include "xil_printf.h"
#include "xparameters.h"

#ifdef SDT

#ifdef XPAR_XTTCPS_3_BASEADDR
#include "xttcps.h"
/* Instance for ttcps */
static XTtcPs xTimerInstance;

/* Interrupt handler for ttcps */
void TtcHandler(XTtcPs *InstancePtr);

#elif defined (XPAR_XTMRCTR_1_BASEADDR)
#include "xtmrctr.h"

/* AXI timer instance */
static XTmrCtr xTickTimerInstance;

/* AXI timer instance Handler */
void TmrctrHandler(XTmrCtr *InstancePtr);
#else
#error "This example needs 2 AXI timer IP's in HW design for Microblaze case, and TTC0, TTC1 in case of other ARM processors, \
Please make sure that, you have enabled required IP's in HW design"
#endif

#else
#if defined (XPAR_XTTCPS_3_DEVICE_ID)
#include "xttcps.h"

#define	TIMER_DEVICE_ID		XPAR_XTTCPS_3_DEVICE_ID

#if !defined(XPAR_XILTIMER_ENABLED) && !defined(SDT)
#define TIMER_INTR_ID		XPAR_XTTCPS_3_INTR
#else
#define TIMER_INTR_ID		XPAR_PSU_TTC_3_INTERRUPT_ID
#endif

/* Instance for ttcps */
static XTtcPs xTimerInstance;

/* Interrupt handler for ttcps */
void TtcHandler(XTtcPs *InstancePtr);

#elif defined (XPAR_TMRCTR_1_DEVICE_ID)
#include "xtmrctr.h"

#define	TIMER_DEVICE_ID		XPAR_TMRCTR_1_DEVICE_ID
#define	TIMER_INTR_ID		XPAR_INTC_0_TMRCTR_1_VEC_ID

/* AXI timer instance */
static XTmrCtr xTickTimerInstance;

/* AXI timer instance Handler */
void TmrctrHandler(XTmrCtr *InstancePtr);
#else
#error "This example needs 2 AXI timer IP's in HW design for Microblaze case, and TTC0, TTC1 in case of other ARM processors, \
Please make sure that, you have enabled required IP's in HW design"
#endif
#endif

#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9
#define DELAY_100_MILISEC		100UL
/*-----------------------------------------------------------*/

static void prvTimerTask( void *pvParameters );

/*-----------------------------------------------------------*/

static TaskHandle_t xTimerTask;
static SemaphoreHandle_t xSemaphore;
int main( void )
{
	const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_10_SECONDS );
	/* Create the timer tasks. This task would register timer interrupt with
	interrupt controller initialized by FreeRTOS porting layer. Customized
	interrupt handler, registered by this task would be triggered, once
	interrupt condition for given timer peripheral occurs.*/
	xTaskCreate( 	prvTimerTask, 					/* The function that implements the task. */
			( const char * ) "Timer", 		/* Text name for the task, provided to assist debugging only. */
			configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
			NULL, 						/* The task parameter is not used, so set to NULL. */
			tskIDLE_PRIORITY,			/* The task runs at the idle priority. */
			&xTimerTask );


	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for ( ;; );
}

/*-----------------------------------------------------------*/
static void prvTimerTask( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );


	const TickType_t x100mseconds = pdMS_TO_TICKS( DELAY_100_MILISEC );
	int xStatus;

	xSemaphore = xSemaphoreCreateBinary();
	if ( xSemaphore == NULL ) {
		xil_printf("Failed to create semaphore\n");
		return XST_FAILURE;
	}

#if defined (XPAR_XTTCPS_3_DEVICE_ID) || defined(XPAR_XTTCPS_3_BASEADDR)
	XTtcPs_Config *pxTimerConfig;
	XInterval usInterval;
	uint8_t ucPrescaler;
#ifdef SDT
	pxTimerConfig = XTtcPs_LookupConfig( XPAR_XTTCPS_3_BASEADDR );
#else
   pxTimerConfig = XTtcPs_LookupConfig( TIMER_DEVICE_ID );
#endif

	xStatus = XTtcPs_CfgInitialize( &xTimerInstance, pxTimerConfig, pxTimerConfig->BaseAddress );

	if ( xStatus != XST_SUCCESS ) {
		XTtcPs_Stop(&xTimerInstance);
		xStatus = XTtcPs_CfgInitialize( &xTimerInstance, pxTimerConfig, pxTimerConfig->BaseAddress );
		if ( xStatus != XST_SUCCESS ) {
			xil_printf( "In %s: Timer Cfg initialization failed...\r\n", __func__ );
			return xStatus;
		}
	}
	XTtcPs_SetOptions( &xTimerInstance, XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE );


	XTtcPs_CalcIntervalFromFreq( &xTimerInstance, configTICK_RATE_HZ * 2, &usInterval, &ucPrescaler );
	XTtcPs_SetInterval( &xTimerInstance, usInterval );
	XTtcPs_SetPrescaler( &xTimerInstance, ucPrescaler );
	XTtcPs_EnableInterrupts( &xTimerInstance, XTTCPS_IXR_INTERVAL_MASK );
	/* Register the ttcps Timer interrupt handler with interrupt controller */
#ifdef SDT
	xPortInstallInterruptHandler( pxTimerConfig->IntrId[0], (Xil_ExceptionHandler)TtcHandler, &xTimerInstance );
	/* Enable interrupt for TTC1 instance */
	vPortEnableInterrupt(pxTimerConfig->IntrId[0]);

#else
	xPortInstallInterruptHandler( TIMER_INTR_ID, (Xil_ExceptionHandler)TtcHandler, &xTimerInstance );
	/* Enable interrupt for TTC1 instance */
	vPortEnableInterrupt(TIMER_INTR_ID);
#endif
	XTtcPs_Start( &xTimerInstance );

#elif defined (XPAR_TMRCTR_1_DEVICE_ID) || defined (XPAR_XTMRCTR_1_BASEADDR)

	const unsigned char ucTickTimerCounterNumber = ( unsigned char ) 0U;
	const unsigned char ucRunTimeStatsCounterNumber = ( unsigned char ) 1U;
#ifdef SDT
	const unsigned long ulCounterValue = ( ( XPAR_XTMRCTR_1_CLOCK_FREQUENCY / configTICK_RATE_HZ / 5) - 1UL );
#else
	const unsigned long ulCounterValue = ( ( XPAR_TMRCTR_1_CLOCK_FREQ_HZ / configTICK_RATE_HZ / 5) - 1UL );
#endif
	extern void vPortTickISR( void *pvUnused );

	/* Initialise the timer/counter. */
#ifdef SDT
	xStatus = XTmrCtr_Initialize( &xTickTimerInstance, XPAR_XTMRCTR_1_BASEADDR);
#else
	xStatus = XTmrCtr_Initialize( &xTickTimerInstance, TIMER_DEVICE_ID );
#endif

	if ( xStatus == XST_SUCCESS ) {

#ifdef SDT
		/* Register the AXI Timer interrupt handler with interrupt controller */
		xStatus = xPortInstallInterruptHandler(xTickTimerInstance.Config.IntrId,
                                             (XInterruptHandler)XTmrCtr_InterruptHandler,
                                             &xTickTimerInstance );
#else
		xStatus = xPortInstallInterruptHandler(TIMER_INTR_ID,
                                             (XInterruptHandler)XTmrCtr_InterruptHandler,
                                             &xTickTimerInstance );

#endif
		XTmrCtr_SetHandler(&xTickTimerInstance, TmrctrHandler,
				   &xTickTimerInstance);
	}

	if ( xStatus == pdPASS ) {
		/* Enable interrupt for timer peripheral */
#ifdef SDT
		vPortEnableInterrupt(xTickTimerInstance.Config.IntrId);
#else
		vPortEnableInterrupt( TIMER_INTR_ID );
#endif

		/* Set the correct period for the timer. */
		XTmrCtr_SetResetValue( &xTickTimerInstance, ucTickTimerCounterNumber, ulCounterValue );

		XTmrCtr_SetOptions( &xTickTimerInstance, ucTickTimerCounterNumber,
				    ( XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION ) );

		/* Start the timer. */
		XTmrCtr_Start( &xTickTimerInstance, ucTickTimerCounterNumber );
	}
#endif

	xil_printf("Waiting for semaphore, FreeRTOS tick count is %x\n", xTaskGetTickCount());
	if ( xSemaphoreTake( xSemaphore, x100mseconds ) != pdTRUE ) {
		xil_printf("FreeRTOS interrupt example FAILED \n");
		vTaskDelete( xTimerTask );
		return XST_FAILURE;
	}

	xil_printf("Successfully ran FreeRTOS interrupt example, FreeRTOS tick count is %x \n", xTaskGetTickCount());
	vTaskDelete( xTimerTask );

}

#if defined (XPAR_XTTCPS_3_DEVICE_ID) || defined (XPAR_XTTCPS_3_BASEADDR)
void TtcHandler(XTtcPs *InstancePtr)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	u32 XTtcPsStatusReg;
	Xil_AssertNonvoid(InstancePtr != NULL);

	XTtcPsStatusReg = XTtcPs_GetInterruptStatus(InstancePtr);
	XTtcPs_ClearInterruptStatus(InstancePtr, XTtcPsStatusReg);
	XTtcPs_Stop(InstancePtr);

	if ( xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken ) != pdFALSE) {
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

#elif defined (XPAR_TMRCTR_1_DEVICE_ID) || defined (XPAR_XTMRCTR_1_BASEADDR)
void TmrctrHandler(XTmrCtr *InstancePtr)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	XTmrCtr_Stop( InstancePtr, 0);
	if ( xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken ) != pdFALSE) {
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}
#endif
