/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2019 Xilinx, Inc. All Rights Reserved.
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

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#if configSTREAM_BUFFER
#include "stream_buffer.h"
#elif configMESSAGE_BUFFER
#include "message_buffer.h"
#endif
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"


#define TIMER_ID	1
#define DELAY_10_SECONDS	10000UL
#define DELAY_1_SECOND		1000UL
#define TIMER_CHECK_THRESHOLD	9
/*-----------------------------------------------------------*/

static void prvTxTask( void *pvParameters );
static void prvRxTask( void *pvParameters );
static void vTimerCallback( TimerHandle_t pxTimer );
/*-----------------------------------------------------------*/

static TaskHandle_t xTxTask;
static TaskHandle_t xRxTask;
static TimerHandle_t xTimer = NULL;
char HWstring[15] = "Hello World\n";
long RxtaskCntr = 0;
#if configSTREAM_BUFFER
StreamBufferHandle_t xStreamBuffer;
const size_t xStreamBufferSizeBytes = 100, xTriggerLevel = 3;
#elif configMESSAGE_BUFFER
MessageBufferHandle_t xMessageBuffer;
const size_t xMessageBufferSizeBytes = 100;
#endif

int main( void )
{
	const TickType_t x10seconds = pdMS_TO_TICKS( DELAY_10_SECONDS );
	/* Create the buffer used by the tasks.  The Rx task has a higher priority
	than the Tx task, so will preempt the Tx task and remove values from the
	buffer as soon as the number of bytes in the stream buffer will be greater
	than or equal to xTriggerlevel */

#if (configSTREAM_BUFFER == 1)
	xil_printf( "Creating stream buffer\r\n" );
	xStreamBuffer = xStreamBufferCreate( xStreamBufferSizeBytes, xTriggerLevel );
	configASSERT( xStreamBuffer );
#elif (configMESSAGE_BUFFER == 1)
	xil_printf("Creating message buffer\n");
	xMessageBuffer = xMessageBufferCreate( xMessageBufferSizeBytes );
	configASSERT( xMessageBuffer );
#else
	xil_printf("stream/message buffer option is not enabled \n");
	xil_printf("Please provide valid option\n");
	return 0;
#endif
	/* Create the two tasks.  The Tx task is given a lower priority than the
	Rx task, so the Rx task will leave the Blocked state and pre-empt the Tx
	task as soon as the Tx task places an item in the buffer. */
	xTaskCreate( 	prvTxTask, 					/* The function that implements the task. */
					( const char * ) "Tx", 		/* Text name for the task, provided to assist debugging only. */
					configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
					NULL, 						/* The task parameter is not used, so set to NULL. */
					tskIDLE_PRIORITY ,			/* The task runs at the idle priority. */
					&xTxTask );

	xTaskCreate( prvRxTask,
				 ( const char * ) "GB",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 1 ,
				 &xRxTask );

	/* Create a timer with a timer expiry of 10 seconds. The timer would expire
	 after 10 seconds and the timer call back would get called. In the timer call back
	 checks are done to ensure that the tasks have been running properly till then.
	 The tasks are deleted in the timer call back and a message is printed to convey that
	 the example has run successfully.
	 The timer expiry is set to 10 seconds and the timer set to not auto reload. */
	xTimer = xTimerCreate( (const char *) "Timer",
							x10seconds,
							pdFALSE,
							(void *) TIMER_ID,
							vTimerCallback);
	/* Check the timer was created. */
	configASSERT( xTimer );

	/* start the timer with a block time of 0 ticks. This means as soon
	   as the schedule starts the timer will start running and will expire after
	   10 seconds */
	xTimerStart( xTimer, 0 );

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}


/*-----------------------------------------------------------*/
static void prvTxTask( void *pvParameters )
{
const TickType_t x1second = pdMS_TO_TICKS( DELAY_1_SECOND );

	for( ;; )
	{
		vTaskDelay( x1second );
#if (configSTREAM_BUFFER == 1)
		/*
		 * writing to stream buffer byte-wise
		 */
		for(int i = 0; i< strlen(HWstring); i++)
{
			xStreamBufferSend( xStreamBuffer,
					( void * ) (HWstring + i),
					sizeof( char ),
					portMAX_DELAY);
}
#elif (configMESSAGE_BUFFER == 1)
		xMessageBufferSend( xMessageBuffer,
				( void * ) HWstring,
				sizeof( HWstring ),
				portMAX_DELAY );
#endif

	}
}

/*-----------------------------------------------------------*/
static void prvRxTask( void *pvParameters )
{
char Recdstring[20] = "";

	for( ;; )
	{

#if (configSTREAM_BUFFER == 1)
		/*
		 * This Rx task will be blocked until the number of bytes in stream
		 * buffer is greater than or equal to xTriggerLevel.
		 */
		for(int i = 0; i < 12; i = i + xTriggerLevel )
		{
		xStreamBufferReceive( xStreamBuffer,
				( void * ) ( Recdstring + i ),
				sizeof ( Recdstring ),
				portMAX_DELAY);
		}

#elif (configMESSAGE_BUFFER ==1)
		xMessageBufferReceive( xMessageBuffer,
				( void * ) Recdstring,
				sizeof( Recdstring ),
				portMAX_DELAY);

#endif
		/* Print the received data. */
		xil_printf( "Rx task received string from Tx task: %s\r\n", Recdstring );
		RxtaskCntr++;
	}
}

/*-----------------------------------------------------------*/
static void vTimerCallback( TimerHandle_t pxTimer )
{
	long lTimerId;
	configASSERT( pxTimer );

	lTimerId = ( long ) pvTimerGetTimerID( pxTimer );

	if (lTimerId != TIMER_ID) {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	/* If the RxtaskCntr is updated every time the Rx task is called. The
	 Rx task is called every time the Tx task sends a message. The Tx task
	 sends a message every 1 second.
	 The timer expires after 10 seconds. We expect the RxtaskCntr to at least
	 have a value of 9 (TIMER_CHECK_THRESHOLD) when the timer expires. */
	if (RxtaskCntr >= TIMER_CHECK_THRESHOLD) {
		xil_printf("FreeRTOS Hello World Example PASSED");
	} else {
		xil_printf("FreeRTOS Hello World Example FAILED");
	}

	vTaskDelete( xRxTask );
	vTaskDelete( xTxTask );
}
