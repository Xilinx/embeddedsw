/*
 * a simple AXITimer driver
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

/*
 * INIT -> CONFIG -> ISR -> START
 */

#include "freertos_axitimer.h"

extern XScuGic xInterruptController;

/* initialize AXITIMER */
void AXITimer_Init(XTmrCtr *InstancePtr, u16 DeviceId)
{
BaseType_t xStatus;

    xStatus = XTmrCtr_Initialize(InstancePtr, DeviceId);
    configASSERT( xStatus == XST_SUCCESS );
}

/* one-line function */
void AXITimer_OptSet(XTmrCtr *InstancePtr, u8 TmrCtrNumber, u32 options)
{
    XTmrCtr_SetOptions(InstancePtr,
                       TmrCtrNumber,
                       options);
}

/* one-line function */
void AXITimer_LoadSet(XTmrCtr *InstancePtr, u8 TmrCtrNumber, u32 ResetValue)
{
    XTmrCtr_SetResetValue(InstancePtr, TmrCtrNumber, ResetValue);
}

/* build callback functions when AXITIMER ISR triggered */
void AXITimer_CallbackSet(XTmrCtr *InstancePtr,
                          u8 TmrCtrNumber,
                          u32 IntID,
                          XTmrCtr_Handler TimerISR)
{
/* use global GIC controller */
extern XScuGic xInterruptController;

/* timer ISR triggered at rising edge of INTR */
BaseType_t xStatus;
const uint8_t ucRisingEdge = 0x3;

/* lowest possible = highest PIN ISR priority */
const BaseType_t TimerPRIORITY = ( configMAX_API_CALL_INTERRUPT_PRIORITY + 1 );

    /* connect */
    XTmrCtr_SetHandler(InstancePtr, TimerISR, InstancePtr);

    /* The priority trigger types */
    XScuGic_SetPriorityTriggerType(&xInterruptController,
                                   IntID,
                                   TimerPRIORITY << portPRIORITY_SHIFT,
                                   ucRisingEdge );

    /* Connect to the interrupt controller. */
    xStatus = XScuGic_Connect(&xInterruptController,
                              IntID,
                              ( Xil_InterruptHandler ) XTmrCtr_InterruptHandler,
                              InstancePtr );
    configASSERT( xStatus == XST_SUCCESS);

    /* Enable the interrupt in the GIC. */
    XScuGic_Enable( &xInterruptController, IntID );

}

/* one-line function */
void AXITimer_Start(XTmrCtr *InstancePtr, u8 TmrCtrNumber)
{
    XTmrCtr_Start(InstancePtr, TmrCtrNumber);
}


