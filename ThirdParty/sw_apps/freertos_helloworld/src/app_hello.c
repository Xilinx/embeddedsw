/*
 * A software timer is used to print the message "hello, world"
 * every 500ms.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*
 * The rate at which data is sent to the queue. (software timer)
 * The 500ms value is converted to ticks using the
 * portTICK_PERIOD_MS constant.
 */
#define mainTIMER_PERIOD_MS         ( 500 / portTICK_PERIOD_MS )

/* The LED toggled by the RX task. */
#define mainTIMER_LED               ( 0 )

/* A block time of zero just means "don't block". */
#define mainDONT_BLOCK              ( 0 )
/*-----------------------------------------------------------*/

/*
 * The callback for the timer that just toggles an LED to show the system is
 * running.
 */
static void prvLEDToggleTimer( TimerHandle_t pxTimer );

/*-----------------------------------------------------------*/

void app_hello( void )
{

    /* A timer is used to toggle an LED just to show the application is
    executing. */
    TimerHandle_t xTimer;
    xTimer = xTimerCreate( "LED",                     /* Text name to make debugging easier. */
                           mainTIMER_PERIOD_MS,       /* The timer's period. */
                           pdTRUE,                    /* This is an auto reload timer. */
                           NULL,                      /* ID is not used. */
                           prvLEDToggleTimer );       /* The callback function. */

    /*
     * Start the timer. (NOTE : this timer is a software timer)
     */
    configASSERT( xTimer );
    xTimerStart( xTimer, mainDONT_BLOCK );

}
/*-----------------------------------------------------------*/

static void prvLEDToggleTimer( TimerHandle_t pxTimer )
{
    /* Prevent compiler warnings. */
    ( void ) pxTimer;

    /*
     * Just do something (i.e., toggle an LED)
     * to show the application is running.
     */
    xil_printf("hello, world\n");

}
/*-----------------------------------------------------------*/

