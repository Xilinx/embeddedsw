/*
    FreeRTOS V9.0.1 - Copyright (C) 2017 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Xilinx includes. */
#include "xttcps.h"
#include "xscugic.h"

/* Timer used to generate the tick interrupt. */
static XTtcPs xTimerInstance;
XScuGic xInterruptController;
/*-----------------------------------------------------------*/

void FreeRTOS_SetupTickInterrupt( void )
{
BaseType_t xStatus;
XTtcPs_Config *pxTimerConfiguration;
XInterval usInterval;
uint8_t ucPrescale;
const uint8_t ucLevelSensitive = 1;
XScuGic_Config *pxInterruptControllerConfig;

	/* Initialize the interrupt controller driver. */
	pxInterruptControllerConfig = XScuGic_LookupConfig( configINTERRUPT_CONTROLLER_DEVICE_ID );
	XScuGic_CfgInitialize( &xInterruptController,
						   pxInterruptControllerConfig,
						   pxInterruptControllerConfig->CpuBaseAddress );

	/* Connect the interrupt controller interrupt handler to the hardware
	interrupt handling logic in the ARM processor. */
	Xil_ExceptionRegisterHandler( XIL_EXCEPTION_ID_IRQ_INT,
								( Xil_ExceptionHandler ) XScuGic_InterruptHandler,
								&xInterruptController);

	/* Enable interrupts in the ARM. */
	Xil_ExceptionEnable();

	pxTimerConfiguration = XTtcPs_LookupConfig( configTIMER_ID );

	/* Initialise the device. */
	xStatus = XTtcPs_CfgInitialize( &xTimerInstance, pxTimerConfiguration, pxTimerConfiguration->BaseAddress );

	if( xStatus != XST_SUCCESS )
	{
		/* Not sure how to do this before XTtcPs_CfgInitialize is called as
		*xRTOSTickTimerInstance is set within XTtcPs_CfgInitialize(). */
		XTtcPs_Stop( &xTimerInstance );
		xStatus = XTtcPs_CfgInitialize( &xTimerInstance, pxTimerConfiguration, pxTimerConfiguration->BaseAddress );
		configASSERT( xStatus == XST_SUCCESS );
	}

	/* Set the options. */
	XTtcPs_SetOptions( &xTimerInstance, ( XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE ) );

	/* Derive values from the tick rate. */
	XTtcPs_CalcIntervalFromFreq( &xTimerInstance, configTICK_RATE_HZ, &( usInterval ), &( ucPrescale ) );

	/* Set the interval and prescale. */
	XTtcPs_SetInterval( &xTimerInstance, usInterval );
	XTtcPs_SetPrescaler( &xTimerInstance, ucPrescale );

	/* The priority must be the lowest possible. */
	XScuGic_SetPriorityTriggerType( &xInterruptController, configTIMER_INTERRUPT_ID, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, ucLevelSensitive );

	/* Connect to the interrupt controller. */
	XScuGic_Connect( &xInterruptController,
					 configTIMER_INTERRUPT_ID,
					( Xil_InterruptHandler ) FreeRTOS_Tick_Handler,
					( void * ) &xTimerInstance );

	/* Enable the interrupt in the GIC. */
	XScuGic_Enable( &xInterruptController, configTIMER_INTERRUPT_ID );

	/* Enable the interrupts in the timer. */
	XTtcPs_EnableInterrupts( &xTimerInstance, XTTCPS_IXR_INTERVAL_MASK );

	/* Start the timer. */
	XTtcPs_Start( &xTimerInstance );
}
/*-----------------------------------------------------------*/

void FreeRTOS_ClearTickInterrupt( void )
{
volatile uint32_t ulInterruptStatus;

	/* Read the interrupt status, then write it back to clear the interrupt. */
	ulInterruptStatus = XTtcPs_GetInterruptStatus( &xTimerInstance );
	XTtcPs_ClearInterruptStatus( &xTimerInstance, ulInterruptStatus );
	__asm volatile( "DSB SY" );
	__asm volatile( "ISB SY" );
}
/*-----------------------------------------------------------*/

void FreeRTOS_ApplicationIRQHandler( uint32_t ulICCIAR )
{
extern const XScuGic_Config XScuGic_ConfigTable[];
static const XScuGic_VectorTableEntry *pxVectorTable = XScuGic_ConfigTable[ XPAR_SCUGIC_SINGLE_DEVICE_ID ].HandlerTable;
uint32_t ulInterruptID;
const XScuGic_VectorTableEntry *pxVectorEntry;

	/* Interrupts cannot be re-enabled until the source of the interrupt is
	cleared. The ID of the interrupt is obtained by bitwise ANDing the ICCIAR
	value with 0x3FF. */
	ulInterruptID = ulICCIAR & 0x3FFUL;
	if( ulInterruptID < XSCUGIC_MAX_NUM_INTR_INPUTS )
	{
		/* Call the function installed in the array of installed handler
		functions. */
		pxVectorEntry = &( pxVectorTable[ ulInterruptID ] );
		configASSERT( pxVectorEntry );
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
