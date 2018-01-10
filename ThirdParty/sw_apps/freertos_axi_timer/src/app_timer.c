/*
 * A AXI timer is used to print the message "hello, world"
 * every 500ms.
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* using FreeRTOS drivers */
#include "freertos_axitimer.h"
#include "freertos_gpio.h"

/*-----------------------------------------------------------*/

/* Handles of the test tasks that must be accessed from other test tasks. */
static TaskHandle_t xSlaveHandle;

/* generate a TIMER */
XTmrCtr xTimer;
static SemaphoreHandle_t xTimerSemaphore = NULL;
void TimerISR(void *CallBackRef, u8 TmrCtrNumber);
static void TimerTask(void *pvTask);

const u16 TimerID              = XPAR_AXI_TIMER_0_DEVICE_ID;
const u16 TimerINTR            = XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR;
const BaseType_t TimerPRIORITY = ( configMAX_API_CALL_INTERRUPT_PRIORITY + 1 );
const BaseType_t intsemTIMER_PRIORITY = ( tskIDLE_PRIORITY + 1 );
const u32 TimerCnt             = 50000000UL;

/*-----------------------------------------------------------*/

void app_timer( void )
{

    /* initialise GPIO */
    GPIO_Init();

    /* Create the semaphores that are given from an interrupt. */
    xTimerSemaphore = xSemaphoreCreateBinary();
    configASSERT( xTimerSemaphore );

    /* create the task */
    xTaskCreate(TimerTask,
                "A-Timer",
                configMINIMAL_STACK_SIZE,
                NULL,
                intsemTIMER_PRIORITY,
                &xSlaveHandle );

    /* initialise AXI Timer in RTOS way */
    AXITimer_Init(&xTimer, TimerID);

    /* set options */
    AXITimer_OptSet(&xTimer, 0, XTC_DOWN_COUNT_OPTION  |
                                XTC_AUTO_RELOAD_OPTION |
                                XTC_INT_MODE_OPTION);

    /* load counter */
    AXITimer_LoadSet(&xTimer, 0, TimerCnt);

    /* register callback functions */
    AXITimer_CallbackSet(&xTimer, 0, TimerINTR, TimerISR);

    /* start */
    AXITimer_Start(&xTimer, 0);

}
/*-----------------------------------------------------------*/

/* pending for Timer0 semaphore */
static void TimerTask(void *pvTask)
{
const TickType_t TimerBlock = 100 * portTICK_PERIOD_MS;

    while(1) {
        if( xSemaphoreTake( xTimerSemaphore, ( TickType_t ) TimerBlock ) == pdPASS ) {
            GPIO_Toggle( partstLED_OUTPUT );
        }
    }

}
/*-----------------------------------------------------------*/

/* post semaphore when INIT */
void TimerISR(void *CallBackRef, u8 TmrCtrNumber)
{
u32 ControlStatusReg;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* read/mask interrupt */
    ControlStatusReg = XTmrCtr_ReadReg(xTimer.BaseAddress,
                                       0,
                                       XTC_TCSR_OFFSET);

    /* mask the interrupt */
    XTmrCtr_WriteReg( xTimer.BaseAddress,
                      0,
                      XTC_TCSR_OFFSET,
                      ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);

    /* post semaphore */
    xSemaphoreGiveFromISR( xTimerSemaphore, &xHigherPriorityTaskWoken );

    /* reset timer */
    XTmrCtr_Reset(CallBackRef,TmrCtrNumber);

    /* we yield to previous tasks (you may skip this step) */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

}
/*-----------------------------------------------------------*/

