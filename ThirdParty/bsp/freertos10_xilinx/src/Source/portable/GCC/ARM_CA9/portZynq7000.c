/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
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

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Xilinx includes. */
#include "xscutimer.h"
#include "xscugic.h"

#define XSCUTIMER_CLOCK_HZ ( XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2UL )

/*
 * Some FreeRTOSConfig.h settings require the application writer to provide the
 * implementation of a callback function that has a specific name, and a linker
 * error will result if the application does not provide the required function.
 * To avoid the risk of a configuration file setting resulting in a linker error
 * this file provides default implementations of each callback that might be
 * required.  The default implementations are declared as weak symbols to allow
 * the application writer to override the default implementation by providing
 * their own implementation in the application itself.
 */
void vApplicationAssert( const char *pcFileName, uint32_t ulLine ) __attribute__((weak));
void vApplicationTickHook( void ) __attribute__((weak));
void vApplicationIdleHook( void ) __attribute__((weak));
void vApplicationMallocFailedHook( void ) __attribute((weak));
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) __attribute__((weak));

/* Timer used to generate the tick interrupt. */
static XScuTimer xTimer;
XScuGic xInterruptController; 	/* Interrupt controller instance */
/*-----------------------------------------------------------*/

void FreeRTOS_SetupTickInterrupt( void )
{
BaseType_t xStatus;
extern void FreeRTOS_Tick_Handler( void );
XScuTimer_Config *pxTimerConfig;
XScuGic_Config *pxGICConfig;
const uint8_t ucRisingEdge = 3;

	/* This function is called with the IRQ interrupt disabled, and the IRQ
	interrupt should be left disabled.  It is enabled automatically when the
	scheduler is started. */

	/* Ensure XScuGic_CfgInitialize() has been called.  In this demo it has
	already been called from prvSetupHardware() in main(). */
	pxGICConfig = XScuGic_LookupConfig( XPAR_SCUGIC_SINGLE_DEVICE_ID );
	xStatus = XScuGic_CfgInitialize( &xInterruptController, pxGICConfig, pxGICConfig->CpuBaseAddress );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */

	/* The priority must be the lowest possible. */
	XScuGic_SetPriorityTriggerType( &xInterruptController, XPAR_SCUTIMER_INTR, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, ucRisingEdge );

	/* Install the FreeRTOS tick handler. */
	xStatus = XScuGic_Connect( &xInterruptController, XPAR_SCUTIMER_INTR, (Xil_ExceptionHandler) FreeRTOS_Tick_Handler, ( void * ) &xTimer );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */

	/* Initialise the timer. */
	pxTimerConfig = XScuTimer_LookupConfig( XPAR_SCUTIMER_DEVICE_ID );
	xStatus = XScuTimer_CfgInitialize( &xTimer, pxTimerConfig, pxTimerConfig->BaseAddr );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */

	/* Enable Auto reload mode. */
	XScuTimer_EnableAutoReload( &xTimer );

	/* Ensure there is no prescale. */
	XScuTimer_SetPrescaler( &xTimer, 0 );

	/* Load the timer counter register. */
	XScuTimer_LoadTimer( &xTimer, XSCUTIMER_CLOCK_HZ / configTICK_RATE_HZ );

	/* Start the timer counter and then wait for it to timeout a number of
	times. */
	XScuTimer_Start( &xTimer );

	/* Enable the interrupt for the xTimer in the interrupt controller. */
	XScuGic_Enable( &xInterruptController, XPAR_SCUTIMER_INTR );

	/* Enable the interrupt in the xTimer itself. */
	FreeRTOS_ClearTickInterrupt();
	XScuTimer_EnableInterrupt( &xTimer );
}
/*-----------------------------------------------------------*/

void FreeRTOS_ClearTickInterrupt( void )
{
	XScuTimer_ClearInterruptStatus( &xTimer );
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler( uint32_t ulICCIAR )
{
extern const XScuGic_Config XScuGic_ConfigTable[];
static const XScuGic_VectorTableEntry *pxVectorTable = XScuGic_ConfigTable[ XPAR_SCUGIC_SINGLE_DEVICE_ID ].HandlerTable;
uint32_t ulInterruptID;
const XScuGic_VectorTableEntry *pxVectorEntry;

	/* The ID of the interrupt is obtained by bitwise anding the ICCIAR value
	with 0x3FF. */
	ulInterruptID = ulICCIAR & 0x3FFUL;
	if( ulInterruptID < XSCUGIC_MAX_NUM_INTR_INPUTS )
	{
		/* Call the function installed in the array of installed handler functions. */
		pxVectorEntry = &( pxVectorTable[ ulInterruptID ] );
		pxVectorEntry->Handler( pxVectorEntry->CallBackRef );
	}
}
/*-----------------------------------------------------------*/

/* This version of vApplicationAssert() is declared as a weak symbol to allow it
to be overridden by a version implemented within the application that is using
this BSP. */
void vApplicationAssert( const char *pcFileName, uint32_t ulLine )
{
volatile uint32_t ul = 0;
volatile const char *pcLocalFileName = pcFileName; /* To prevent pcFileName being optimized away. */
volatile uint32_t ulLocalLine = ulLine; /* To prevent ulLine being optimized away. */

	/* Prevent compile warnings about the following two variables being set but
	not referenced.  They are intended for viewing in the debugger. */
	( void ) pcLocalFileName;
	( void ) ulLocalLine;

	xil_printf( "Assert failed in file %s, line %lu\r\n", pcLocalFileName, ulLocalLine );

	/* If this function is entered then a call to configASSERT() failed in the
	FreeRTOS code because of a fatal error.  The pcFileName and ulLine
	parameters hold the file name and line number in that file of the assert
	that failed.  Additionally, if using the debugger, the function call stack
	can be viewed to find which line failed its configASSERT() test.  Finally,
	the debugger can be used to set ul to a non-zero value, then step out of
	this function to find where the assert function was entered. */
	taskENTER_CRITICAL();
	{
		while( ul == 0 )
		{
			__asm volatile( "NOP" );
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* This default tick hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

/* This default idle hook does nothing and is declared as a weak symbol to allow
the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationIdleHook( void )
{
}
/*-----------------------------------------------------------*/

/* This default malloc failed hook does nothing and is declared as a weak symbol
to allow the application writer to override this default by providing their own
implementation in the application code. */
void vApplicationMallocFailedHook( void )
{
	xil_printf( "vApplicationMallocFailedHook() called\n" );
}
/*-----------------------------------------------------------*/

/* This default stack overflow hook will stop the application for executing.  It
is declared as a weak symbol to allow the application writer to override this
default by providing their own implementation in the application code. */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
/* Attempt to prevent the handle and name of the task that overflowed its stack
from being optimised away because they are not used. */
volatile TaskHandle_t xOverflowingTaskHandle = xTask;
volatile char *pcOverflowingTaskName = pcTaskName;

	( void ) xOverflowingTaskHandle;
	( void ) pcOverflowingTaskName;

	xil_printf( "HALT: Task %s overflowed its stack.", pcOverflowingTaskName );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
