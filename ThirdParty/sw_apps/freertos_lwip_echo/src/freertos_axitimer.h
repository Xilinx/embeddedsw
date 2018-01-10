/*
 * a simple AXI Timer driver
 *
 * liu_benyuan <liubenyuan@gmail.com>
 */

#ifndef FREERTOS_AXITIMER_H_
#define FREERTOS_AXITIMER_H_

/* freeRTOS */
#include "FreeRTOS.h"

/* Xilinx */
#include <xtmrctr.h>
#include <xscugic.h>
#include <xil_types.h>

void AXITimer_Init( XTmrCtr *InstancePtr, u16 DeviceId );

void AXITimer_OptSet( XTmrCtr *InstancePtr,
                      u8 TmrCtrNumber,
                      u32 options );

void AXITimer_LoadSet( XTmrCtr *InstancePtr,
                       u8 TmrCtrNumber,
                       u32 ResetValue );

void AXITimer_CallbackSet( XTmrCtr *InstancePtr,
                           u8 TmrCtrNumber,
                           u32 IntID,
                           XTmrCtr_Handler TimerISR );

void AXITimer_Start( XTmrCtr *InstancePtr,
                     u8 TmrCtrNumber );

#endif /* FREERTOS_AXITIMER_H_ */
