/*
 * FreeRTOS Kernel V10.6.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/* BSP includes. */
#include <mb_interface.h>
#include <xparameters.h>
#include <xintc_l.h>

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#ifdef __arch64__
#define portSTACK_TYPE	size_t
typedef uint64_t UBaseType_t;
#else
#define portSTACK_TYPE  uint32_t
typedef unsigned long UBaseType_t;
#endif
#define portBASE_TYPE   long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
typedef uint16_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

#endif
/*-----------------------------------------------------------*/

/* Interrupt control macros and functions. */
void microblaze_disable_interrupts( void );
void microblaze_enable_interrupts( void );
#define portDISABLE_INTERRUPTS()    microblaze_disable_interrupts()
#define portENABLE_INTERRUPTS()     microblaze_enable_interrupts()
/*-----------------------------------------------------------*/

/* Critical section macros. */
extern volatile UBaseType_t uxCriticalNesting;
void vPortEnterCritical( void );
void vPortExitCritical( void );
#define portENTER_CRITICAL()        {                                                               \
		extern volatile UBaseType_t uxCriticalNesting;              \
		microblaze_disable_interrupts();                            \
		uxCriticalNesting++;                                        \
	}

#define portEXIT_CRITICAL()         {                                                               \
		extern volatile UBaseType_t uxCriticalNesting;              \
		/* Interrupts are disabled, so we can */                    \
		/* access the variable directly. */                         \
		uxCriticalNesting--;                                        \
		if( uxCriticalNesting == 0 )                                \
		{                                                           \
			/* The nesting has unwound and we                       \
			can enable interrupts again. */                         \
			portENABLE_INTERRUPTS();                                \
		}                                                           \
	}

/*-----------------------------------------------------------*/

/* The yield macro maps directly to the vPortYield() function. */
void vPortYield( void );
#define portYIELD() vPortYield()

/* portYIELD_FROM_ISR() does not directly call vTaskSwitchContext(), but instead
sets a flag to say that a yield has been requested.  The interrupt exit code
then checks this flag, and calls vTaskSwitchContext() before restoring a task
context, if the flag is not false.  This is done to prevent multiple calls to
vTaskSwitchContext() being made from a single interrupt, as a single interrupt
can result in multiple peripherals being serviced. */
extern volatile uint32_t ulTaskSwitchRequested;
#define portYIELD_FROM_ISR( x ) if( ( x ) != pdFALSE ) ulTaskSwitchRequested = 1

#if( configUSE_PORT_OPTIMISED_TASK_SELECTION == 1 )

/* Generic helper function. */
__attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros(
	uint32_t ulBitmap )
{
	uint8_t ucReturn;

	__asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) );
	return ucReturn;
}

/* Check the configuration. */
#if( configMAX_PRIORITIES > 32 )
#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
#endif

/* Store/clear the ready priorities in a bit map. */
#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

/*-----------------------------------------------------------*/

#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t ) ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/*-----------------------------------------------------------*/

/* Hardware specifics. */
#ifdef __arch64__
#define portBYTE_ALIGNMENT          8
#else
#define portBYTE_ALIGNMENT			4
#endif
#define portSTACK_GROWTH			( -1 )
#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portNOP()					asm volatile ( "NOP" )
#define portMEMORY_BARRIER() 				asm volatile( "" ::: "memory" )
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* The following structure is used by the FreeRTOS exception handler.  It is
filled with the MicroBlaze context as it was at the time the exception occurred.
This is done as an aid to debugging exception occurrences. */
typedef struct PORT_REGISTER_DUMP {
	/* The following structure members hold the values of the MicroBlaze
	registers at the time the exception was raised. */
	UINTPTR ulR1_SP;
	UINTPTR ulR2_small_data_area;
	UINTPTR ulR3;
	UINTPTR ulR4;
	UINTPTR ulR5;
	UINTPTR ulR6;
	UINTPTR ulR7;
	UINTPTR ulR8;
	UINTPTR ulR9;
	UINTPTR ulR10;
	UINTPTR ulR11;
	UINTPTR ulR12;
	UINTPTR ulR13_read_write_small_data_area;
	UINTPTR ulR14_return_address_from_interrupt;
	UINTPTR ulR15_return_address_from_subroutine;
	UINTPTR ulR16_return_address_from_trap;
	UINTPTR ulR17_return_address_from_exceptions; /* The exception entry code will copy the BTR into R17 if the exception occurred in the delay slot of a branch instruction. */
	UINTPTR ulR18;
	UINTPTR ulR19;
	UINTPTR ulR20;
	UINTPTR ulR21;
	UINTPTR ulR22;
	UINTPTR ulR23;
	UINTPTR ulR24;
	UINTPTR ulR25;
	UINTPTR ulR26;
	UINTPTR ulR27;
	UINTPTR ulR28;
	UINTPTR ulR29;
	UINTPTR ulR30;
	UINTPTR ulR31;
	UINTPTR ulPC;
	UINTPTR ulESR;
	UINTPTR ulMSR;
	UINTPTR ulEAR;
	UINTPTR ulFSR;
	UINTPTR ulEDR;

	/* A human readable description of the exception cause.  The strings used
	are the same as the #define constant names found in the
	microblaze_exceptions_i.h header file */
	int8_t *pcExceptionCause;

	/* The human readable name of the task that was running at the time the
	exception occurred.  This is the name that was given to the task when the
	task was created using the FreeRTOS xTaskCreate() API function. */
	char *pcCurrentTaskName;

	/* The handle of the task that was running a the time the exception
	occurred. */
	void *xCurrentTaskHandle;

} xPortRegisterDump;

/*
 * Installs pxHandler as the interrupt handler for the peripheral specified by
 * the ucInterruptID parameter.
 *
 * ucInterruptID:
 *
 * The ID of the peripheral that will have pxHandler assigned as its interrupt
 * handler.  Peripheral IDs are defined in the xparameters.h header file, which
 * is itself part of the BSP project.  For example, in the official demo
 * application for this port, xparameters.h defines the following IDs for the
 * four possible interrupt sources:
 *
 * XPAR_INTC_0_UARTLITE_1_VEC_ID  -  for the UARTlite peripheral.
 * XPAR_INTC_0_TMRCTR_0_VEC_ID    -  for the AXI Timer 0 peripheral.
 * XPAR_INTC_0_EMACLITE_0_VEC_ID  -  for the Ethernet lite peripheral.
 * XPAR_INTC_0_GPIO_1_VEC_ID      -  for the button inputs.
 *
 *
 * pxHandler:
 *
 * A pointer to the interrupt handler function itself.  This must be a void
 * function that takes a (void *) parameter.
 *
 *
 * pvCallBackRef:
 *
 * The parameter passed into the handler function.  In many cases this will not
 * be used and can be NULL.  Some times it is used to pass in a reference to
 * the peripheral instance variable, so it can be accessed from inside the
 * handler function.
 *
 *
 * pdPASS is returned if the function executes successfully.  Any other value
 * being returned indicates that the function did not execute correctly.
 */
#if !defined(XPAR_XILTIMER_ENABLED) && !defined(SDT)
BaseType_t xPortInstallInterruptHandler( uint8_t ucInterruptID, XInterruptHandler pxHandler,
	void *pvCallBackRef );
#else
BaseType_t xPortInstallInterruptHandler( uint32_t ucInterruptID, XInterruptHandler pxHandler,
	void *pvCallBackRef );
#endif

/*
 * Installs pxHandler as the fast interrupt handler for the peripheral
 * specified by the ucInterruptID parameter.
 *
 * ucInterruptID:
 *
 * The ID of the peripheral that will have pxHandler assigned as its interrupt
 * handler.  Peripheral IDs are defined in the xparameters.h header file, which
 * is itself part of the BSP project.  For example, in the official demo
 * application for this port, xparameters.h defines the following IDs for the
 * four possible interrupt sources:
 *
 * XPAR_INTC_0_UARTLITE_1_VEC_ID  -  for the UARTlite peripheral.
 * XPAR_INTC_0_TMRCTR_0_VEC_ID    -  for the AXI Timer 0 peripheral.
 * XPAR_INTC_0_EMACLITE_0_VEC_ID  -  for the Ethernet lite peripheral.
 * XPAR_INTC_0_GPIO_1_VEC_ID      -  for the button inputs.
 *
 *
 * pxHandler:
 *
 * A pointer to the interrupt handler function itself.  This must be a void
 * function that takes a (void *) parameter and must have
 * __attribute__ ((fast_interrupt)) in the function declaration.
 *
 * pdPASS is returned if the function executes successfully.  Any other value
 * being returned indicates that the function did not execute correctly.
 */
#if !defined(XPAR_XILTIMER_ENABLED) && !defined(SDT)
BaseType_t xPortInstallFastInterruptHandler( uint8_t ucInterruptID,
	XFastInterruptHandler pxHandler);
#else
BaseType_t xPortInstallFastInterruptHandler( uint32_t ucInterruptID,
	XFastInterruptHandler pxHandler);
#endif

/*
 * Enables the interrupt, within the interrupt controller, for the peripheral
 * specified by the ucInterruptID parameter.
 *
 * ucInterruptID:
 *
 * The ID of the peripheral that will have its interrupt enabled in the
 * interrupt controller.  Peripheral IDs are defined in the xparameters.h header
 * file, which is itself part of the BSP project.  For example, in the official
 * demo application for this port, xparameters.h defines the following IDs for
 * the four possible interrupt sources:
 *
 * XPAR_INTC_0_UARTLITE_1_VEC_ID  -  for the UARTlite peripheral.
 * XPAR_INTC_0_TMRCTR_0_VEC_ID    -  for the AXI Timer 0 peripheral.
 * XPAR_INTC_0_EMACLITE_0_VEC_ID  -  for the Ethernet lite peripheral.
 * XPAR_INTC_0_GPIO_1_VEC_ID      -  for the button inputs.
 *
 */
#if !defined(XPAR_XILTIMER_ENABLED) && !defined(SDT)
void vPortEnableInterrupt( uint8_t ucInterruptID );
#else
void vPortEnableInterrupt( uint32_t ucInterruptID );
#endif

/*
 * Disables the interrupt, within the interrupt controller, for the peripheral
 * specified by the ucInterruptID parameter.
 *
 * ucInterruptID:
 *
 * The ID of the peripheral that will have its interrupt disabled in the
 * interrupt controller.  Peripheral IDs are defined in the xparameters.h header
 * file, which is itself part of the BSP project.  For example, in the official
 * demo application for this port, xparameters.h defines the following IDs for
 * the four possible interrupt sources:
 *
 * XPAR_INTC_0_UARTLITE_1_VEC_ID  -  for the UARTlite peripheral.
 * XPAR_INTC_0_TMRCTR_0_VEC_ID    -  for the AXI Timer 0 peripheral.
 * XPAR_INTC_0_EMACLITE_0_VEC_ID  -  for the Ethernet lite peripheral.
 * XPAR_INTC_0_GPIO_1_VEC_ID      -  for the button inputs.
 *
 */
#if !defined(XPAR_XILTIMER_ENABLED) && !defined(SDT)
void vPortDisableInterrupt( uint8_t ucInterruptID );
#else
void vPortDisableInterrupt( uint32_t ucInterruptID );
#endif

/*
 * This is an application defined callback function used to install the tick
 * interrupt handler.  It is provided as an application callback because the
 * kernel will run on lots of different MicroBlaze and FPGA configurations - not
 * all of which will have the same timer peripherals defined or available.  This
 * example uses the AXI Timer 0.  If that is available on your hardware platform
 * then this example callback implementation should not require modification.
 * The name of the interrupt handler that should be installed is vPortTickISR(),
 * which the function below declares as an extern.
 */
void vApplicationSetupTimerInterrupt( void );

/*
 * This is an application defined callback function used to clear whichever
 * interrupt was installed by the the vApplicationSetupTimerInterrupt() callback
 * function - in this case the interrupt generated by the AXI timer.  It is
 * provided as an application callback because the kernel will run on lots of
 * different MicroBlaze and FPGA configurations - not all of which will have the
 * same timer peripherals defined or available.  This example uses the AXI Timer 0.
 * If that is available on your hardware platform then this example callback
 * implementation should not require modification provided the example definition
 * of vApplicationSetupTimerInterrupt() is also not modified.
 */
void vApplicationClearTimerInterrupt( void );

/*
 * vPortExceptionsInstallHandlers() is only available when the MicroBlaze
 * is configured to include exception functionality, and
 * configINSTALL_EXCEPTION_HANDLERS is set to 1 in FreeRTOSConfig.h.
 *
 * vPortExceptionsInstallHandlers() installs the FreeRTOS exception handler
 * for every possible exception cause.
 *
 * vPortExceptionsInstallHandlers() can be called explicitly from application
 * code.  After that is done, the default FreeRTOS exception handler that will
 * have been installed can be replaced for any specific exception cause by using
 * the standard Xilinx library function microblaze_register_exception_handler().
 *
 * If vPortExceptionsInstallHandlers() is not called explicitly by the
 * application, it will be called automatically by the kernel the first time
 * xPortInstallInterruptHandler() is called.  At that time, any exception
 * handlers that may have already been installed will be replaced.
 *
 * See the description of vApplicationExceptionRegisterDump() for information
 * on the processing performed by the FreeRTOS exception handler.
 */
void vPortExceptionsInstallHandlers( void );

/*
 * The FreeRTOS exception handler fills an xPortRegisterDump structure (defined
 * in portmacro.h) with the MicroBlaze context, as it was at the time the
 * exception occurred.  The exception handler then calls
 * vApplicationExceptionRegisterDump(), passing in the completed
 * xPortRegisterDump structure as its parameter.
 *
 * The FreeRTOS kernel provides its own implementation of
 * vApplicationExceptionRegisterDump(), but the kernel provided implementation
 * is declared as being 'weak'.  The weak definition allows the application
 * writer to provide their own implementation, should they wish to use the
 * register dump information.  For example, an implementation could be provided
 * that wrote the register dump data to a display, or a UART port.
 */
void vApplicationExceptionRegisterDump( xPortRegisterDump *xRegisterDump );

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */
