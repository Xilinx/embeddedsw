/*
 * A software timer is used to toggle the LED on Microzed board and
 * polling the status of button every 500ms (soft timer).
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* XILINX includes. */
#include "xparameters.h"

/* GPIO */
#include "freertos_gpio.h"

/*-----------------------------------------------------------*/

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

void app_gpio( void )
{

    /* initialise GPIO */
    GPIO_Init();

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
    GPIO_Toggle( partstLED_OUTPUT );

    /* read from button, you may press or not the button */
    unsigned int val = GPIO_Read( partstBTN_INPUT );
    xil_printf("button status = %d\n", val);

}
/*-----------------------------------------------------------*/

