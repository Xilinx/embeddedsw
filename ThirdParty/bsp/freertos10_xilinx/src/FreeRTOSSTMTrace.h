/*
    Copyright (C) 2016 - 2018 Xilinx, Inc. All rights reserved.

    This file is part of the FreeRTOS distribution.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software. If you wish to use our Amazon
    FreeRTOS name, please do so in a fair use way that does not cause confusion.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    http://www.FreeRTOS.org
    http://aws.amazon.com/freertos

    1 tab == 4 spaces!
 */

/*****************************************************************************/
/**
*
* @file FreeRTOSSTMTrace.h
*
* Contains FreeRTOS trace macros to write trace data to STM address space on
* ZU+. STM generates STPI packets, which are consumed by SDK to generate trace
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sdm   07/15/16 Initial version
* </pre>
*
******************************************************************************/

#ifndef _XFREERTOS_STM_TRACE_H_
#define _XFREERTOS_STM_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"

#ifdef FREERTOS_ENABLE_TRACE

#if (!defined EXEC_MODE32) && (! defined EXEC_MODE64)
 #error "Unsupported Processor Type"
#endif

enum stm_trace_events {
    FREERTOS_TASK_SWITCHED_IN,
    FREERTOS_INCREASE_TICK_COUNT,
    FREERTOS_LOW_POWER_IDLE_BEGIN,
    FREERTOS_LOW_POWER_IDLE_END,
    FREERTOS_TASK_SWITCHED_OUT,
    FREERTOS_TASK_PRIORITY_INHERIT,
    FREERTOS_TASK_PRIORITY_DISINHERIT,
    FREERTOS_BLOCKING_ON_QUEUE_RECEIVE,
    FREERTOS_BLOCKING_ON_QUEUE_SEND,
    FREERTOS_MOVED_TASK_TO_READY_STATE,
    FREERTOS_QUEUE_CREATE,
    FREERTOS_QUEUE_CREATE_FAILED,
    FREERTOS_CREATE_MUTEX,
    FREERTOS_CREATE_MUTEX_FAILED,
    FREERTOS_GIVE_MUTEX_RECURSIVE,
    FREERTOS_GIVE_MUTEX_RECURSIVE_FAILED,
    FREERTOS_TAKE_MUTEX_RECURSIVE,
    FREERTOS_TAKE_MUTEX_RECURSIVE_FAILED,
    FREERTOS_CREATE_COUNTING_SEMAPHORE,
    FREERTOS_CREATE_COUNTING_SEMAPHORE_FAILED,
    FREERTOS_QUEUE_SEND,
    FREERTOS_QUEUE_SEND_FAILED,
    FREERTOS_QUEUE_RECEIVE,
    FREERTOS_QUEUE_PEEK,
    FREERTOS_QUEUE_PEEK_FROM_ISR,
    FREERTOS_QUEUE_RECEIVE_FAILED,
    FREERTOS_QUEUE_SEND_FROM_ISR,
    FREERTOS_QUEUE_SEND_FROM_ISR_FAILED,
    FREERTOS_QUEUE_RECEIVE_FROM_ISR,
    FREERTOS_QUEUE_RECEIVE_FROM_ISR_FAILED,
    FREERTOS_QUEUE_PEEK_FROM_ISR_FAILED,
    FREERTOS_QUEUE_DELETE,
    FREERTOS_TASK_CREATE,
    FREERTOS_TASK_CREATE_FAILED,
    FREERTOS_TASK_DELETE,
    FREERTOS_TASK_DELAY_UNTIL,
    FREERTOS_TASK_DELAY,
    FREERTOS_TASK_PRIORITY_SET,
    FREERTOS_TASK_SUSPEND,
    FREERTOS_TASK_RESUME,
    FREERTOS_TASK_RESUME_FROM_ISR,
    FREERTOS_TASK_INCREMENT_TICK,
    FREERTOS_TIMER_CREATE,
    FREERTOS_TIMER_CREATE_FAILED,
    FREERTOS_TIMER_COMMAND_SEND,
    FREERTOS_TIMER_EXPIRED,
    FREERTOS_TIMER_COMMAND_RECEIVED,
    FREERTOS_MALLOC,
    FREERTOS_FREE,
    FREERTOS_EVENT_GROUP_CREATE,
    FREERTOS_EVENT_GROUP_CREATE_FAILED,
    FREERTOS_EVENT_GROUP_SYNC_BLOCK,
    FREERTOS_EVENT_GROUP_SYNC_END,
    FREERTOS_EVENT_GROUP_WAIT_BITS_BLOCK,
    FREERTOS_EVENT_GROUP_WAIT_BITS_END,
    FREERTOS_EVENT_GROUP_CLEAR_BITS,
    FREERTOS_EVENT_GROUP_CLEAR_BITS_FROM_ISR,
    FREERTOS_EVENT_GROUP_SET_BITS,
    FREERTOS_EVENT_GROUP_SET_BITS_FROM_ISR,
    FREERTOS_EVENT_GROUP_DELETE,
    FREERTOS_PEND_FUNC_CALL,
    FREERTOS_PEND_FUNC_CALL_FROM_ISR,
    FREERTOS_QUEUE_REGISTRY_ADD,
    FREERTOS_TASK_NOTIFY_TAKE_BLOCK,
    FREERTOS_TASK_NOTIFY_TAKE,
    FREERTOS_TASK_NOTIFY_WAIT_BLOCK,
    FREERTOS_TASK_NOTIFY_WAIT,
    FREERTOS_TASK_NOTIFY,
    FREERTOS_TASK_NOTIFY_FROM_ISR,
    FREERTOS_TASK_NOTIFY_GIVE_FROM_ISR,
};

#define STM_BASE                        0xf8000000

#define FREERTOS_EMIT_EVENT(id)         Xil_Out8(STM_BASE + (FREERTOS_STM_CHAN * 0x100), id)
#ifdef EXEC_MODE32
        #define FREERTOS_EMIT_DATA(data) Xil_Out32((u32) (STM_BASE + (FREERTOS_STM_CHAN * 0x100) + 0x18), (u32) data)
#else
        #define FREERTOS_EMIT_DATA(data) Xil_Out64((u64) (STM_BASE + (FREERTOS_STM_CHAN * 0x100) + 0x18), (u64) data)
#endif

/* Remove any unused trace macros. */
#ifndef traceSTART
    /* Used to perform any necessary initialisation - for example, open a file
    into which trace is to be written. */
    #define traceSTART()
#else
    #error "FreeRTOS Trace is already enabled"
#endif

#ifndef traceEND
    /* Use to close a trace, for example close a file into which trace has been
    written. */
    #define traceEND()
#endif

#ifndef traceTASK_SWITCHED_IN
    /* Called after a task has been selected to run.  pxCurrentTCB holds a pointer
    to the task control block of the selected task. */
    #define traceTASK_SWITCHED_IN() {                               \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_SWITCHED_IN);             \
        FREERTOS_EMIT_DATA(pxCurrentTCB);                           \
    }
#endif

#ifndef traceINCREASE_TICK_COUNT
    /* Called before stepping the tick count after waking from tickless idle
    sleep. */
    #define traceINCREASE_TICK_COUNT( x ) {                         \
        FREERTOS_EMIT_EVENT(FREERTOS_INCREASE_TICK_COUNT);          \
        FREERTOS_EMIT_DATA(x);                                      \
    }
#endif

#ifndef traceLOW_POWER_IDLE_BEGIN
    /* Called immediately before entering tickless idle. */
    #define traceLOW_POWER_IDLE_BEGIN() {                           \
        FREERTOS_EMIT_EVENT(FREERTOS_LOW_POWER_IDLE_BEGIN);         \
    }
#endif

#ifndef    traceLOW_POWER_IDLE_END
    /* Called when returning to the Idle task after a tickless idle. */
    #define traceLOW_POWER_IDLE_END() {                             \
        FREERTOS_EMIT_EVENT(FREERTOS_LOW_POWER_IDLE_END);           \
    }
#endif

#ifndef traceTASK_SWITCHED_OUT
    /* Called before a task has been selected to run.  pxCurrentTCB holds a pointer
    to the task control block of the task being switched out. */
    #define traceTASK_SWITCHED_OUT() {                              \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_SWITCHED_OUT);            \
        FREERTOS_EMIT_DATA(pxCurrentTCB);                           \
    }
#endif

#ifndef traceTASK_PRIORITY_INHERIT
    /* Called when a task attempts to take a mutex that is already held by a
    lower priority task.  pxTCBOfMutexHolder is a pointer to the TCB of the task
    that holds the mutex.  uxInheritedPriority is the priority the mutex holder
    will inherit (the priority of the task that is attempting to obtain the
    muted. */
    #define traceTASK_PRIORITY_INHERIT( pxTCBOfMutexHolder, uxInheritedPriority ) {	\
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_PRIORITY_INHERIT);        \
        FREERTOS_EMIT_DATA(pxTCBOfMutexHolder);                     \
        FREERTOS_EMIT_DATA(uxInheritedPriority);                    \
    }
#endif

#ifndef traceTASK_PRIORITY_DISINHERIT
    /* Called when a task releases a mutex, the holding of which had resulted in
    the task inheriting the priority of a higher priority task.
    pxTCBOfMutexHolder is a pointer to the TCB of the task that is releasing the
    mutex.  uxOriginalPriority is the task's configured (base) priority. */
    #define traceTASK_PRIORITY_DISINHERIT( pxTCBOfMutexHolder, uxOriginalPriority ) {	\
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_PRIORITY_DISINHERIT);     \
        FREERTOS_EMIT_DATA(pxTCBOfMutexHolder);                     \
        FREERTOS_EMIT_DATA(uxOriginalPriority);                     \
    }
#endif

#ifndef traceBLOCKING_ON_QUEUE_RECEIVE
    /* Task is about to block because it cannot read from a
    queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
    upon which the read was attempted.  pxCurrentTCB points to the TCB of the
    task that attempted the read. */
    #define traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue ) {             \
        FREERTOS_EMIT_EVENT(FREERTOS_BLOCKING_ON_QUEUE_RECEIVE);    \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceBLOCKING_ON_QUEUE_SEND
    /* Task is about to block because it cannot write to a
    queue/mutex/semaphore.  pxQueue is a pointer to the queue/mutex/semaphore
    upon which the write was attempted.  pxCurrentTCB points to the TCB of the
    task that attempted the write. */
    #define traceBLOCKING_ON_QUEUE_SEND( pxQueue ) {                \
        FREERTOS_EMIT_EVENT(FREERTOS_BLOCKING_ON_QUEUE_SEND);       \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

/* The following event macros are embedded in the kernel API calls. */

#ifndef traceMOVED_TASK_TO_READY_STATE
    #define traceMOVED_TASK_TO_READY_STATE( pxTCB ) {               \
        FREERTOS_EMIT_EVENT(FREERTOS_MOVED_TASK_TO_READY_STATE);    \
        FREERTOS_EMIT_DATA(pxTCB);                                  \
    }
#endif

#ifndef traceQUEUE_CREATE
    #define traceQUEUE_CREATE( pxNewQueue ) {                       \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_CREATE);                 \
        FREERTOS_EMIT_DATA(pxNewQueue);                             \
        FREERTOS_EMIT_DATA(pxNewQueue->uxLength);                   \
    }
#endif

#ifndef traceQUEUE_CREATE_FAILED
    #define traceQUEUE_CREATE_FAILED( ucQueueType ) {               \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_CREATE_FAILED);          \
        FREERTOS_EMIT_DATA(ucQueueType);                            \
    }
#endif

#ifndef traceCREATE_MUTEX
    #define traceCREATE_MUTEX( pxNewQueue ) {                       \
        FREERTOS_EMIT_EVENT(FREERTOS_CREATE_MUTEX);                 \
        FREERTOS_EMIT_DATA(pxNewQueue);                             \
        FREERTOS_EMIT_DATA(pxNewQueue->uxQueueNumber);              \
    }
#endif

#ifndef traceCREATE_MUTEX_FAILED
    #define traceCREATE_MUTEX_FAILED() {                            \
        FREERTOS_EMIT_EVENT(FREERTOS_CREATE_MUTEX_FAILED);          \
    }
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE
    #define traceGIVE_MUTEX_RECURSIVE( pxMutex ) {                  \
        FREERTOS_EMIT_EVENT(FREERTOS_GIVE_MUTEX_RECURSIVE);         \
        FREERTOS_EMIT_DATA(pxMutex);                                \
    }
#endif

#ifndef traceGIVE_MUTEX_RECURSIVE_FAILED
    #define traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex ) {           \
        FREERTOS_EMIT_EVENT(FREERTOS_GIVE_MUTEX_RECURSIVE_FAILED);  \
        FREERTOS_EMIT_DATA(pxMutex);                                \
    }
#endif

#ifndef traceTAKE_MUTEX_RECURSIVE
    #define traceTAKE_MUTEX_RECURSIVE( pxMutex ) {                  \
        FREERTOS_EMIT_EVENT(FREERTOS_TAKE_MUTEX_RECURSIVE);         \
        FREERTOS_EMIT_DATA(pxMutex);                                \
    }
#endif

#ifndef traceTAKE_MUTEX_RECURSIVE_FAILED
    #define traceTAKE_MUTEX_RECURSIVE_FAILED( pxMutex ) {           \
        FREERTOS_EMIT_EVENT(FREERTOS_TAKE_MUTEX_RECURSIVE_FAILED);  \
        FREERTOS_EMIT_DATA(pxMutex);                                \
    }
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE
    #define traceCREATE_COUNTING_SEMAPHORE() {                      \
        FREERTOS_EMIT_EVENT(FREERTOS_CREATE_COUNTING_SEMAPHORE);    \
    }
#endif

#ifndef traceCREATE_COUNTING_SEMAPHORE_FAILED
    #define traceCREATE_COUNTING_SEMAPHORE_FAILED() {                   \
        FREERTOS_EMIT_EVENT(FREERTOS_CREATE_COUNTING_SEMAPHORE_FAILED); \
    }
#endif

#ifndef traceQUEUE_SEND
    #define traceQUEUE_SEND( pxQueue ) {                            \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_SEND);                   \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_SEND_FAILED
    #define traceQUEUE_SEND_FAILED( pxQueue ) {                     \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_SEND_FAILED);            \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_RECEIVE
    #define traceQUEUE_RECEIVE( pxQueue ) {                         \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_RECEIVE);                \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_PEEK
    #define traceQUEUE_PEEK( pxQueue ) {                            \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_PEEK);                   \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_PEEK_FROM_ISR
    #define traceQUEUE_PEEK_FROM_ISR( pxQueue ) {                   \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_PEEK_FROM_ISR);          \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_RECEIVE_FAILED
    #define traceQUEUE_RECEIVE_FAILED( pxQueue ) {                  \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_RECEIVE_FAILED);         \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_SEND_FROM_ISR
    #define traceQUEUE_SEND_FROM_ISR( pxQueue ) {                   \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_SEND_FROM_ISR);          \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_SEND_FROM_ISR_FAILED
    #define traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue ) {            \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_SEND_FROM_ISR_FAILED);   \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR
    #define traceQUEUE_RECEIVE_FROM_ISR( pxQueue ) {                \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_RECEIVE_FROM_ISR);       \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_RECEIVE_FROM_ISR_FAILED
    #define traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue ) {         \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_RECEIVE_FROM_ISR_FAILED);\
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_PEEK_FROM_ISR_FAILED
    #define traceQUEUE_PEEK_FROM_ISR_FAILED( pxQueue ) {            \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_PEEK_FROM_ISR_FAILED);   \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceQUEUE_DELETE
    #define traceQUEUE_DELETE( pxQueue ) {                          \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_DELETE);                 \
        FREERTOS_EMIT_DATA(pxQueue);                                \
    }
#endif

#ifndef traceTASK_CREATE
    #define traceTASK_CREATE( pxNewTCB )  {                         \
        int i, len;                                                 \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_CREATE);                  \
        FREERTOS_EMIT_DATA(pxNewTCB);                               \
        FREERTOS_EMIT_DATA(pxNewTCB->uxPriority);                   \
        len = strlen(pxNewTCB->pcTaskName);                         \
        for (i = 0; i < len; i++) {                                 \
            Xil_Out8(0xf8000018, pxNewTCB->pcTaskName[i]);          \
        }                                                           \
    }
#endif

#ifndef traceTASK_CREATE_FAILED
    #define traceTASK_CREATE_FAILED() {                             \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_CREATE_FAILED);           \
    }
#endif

#ifndef traceTASK_DELETE
    #define traceTASK_DELETE( pxTaskToDelete ) {                    \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_DELETE);                  \
        FREERTOS_EMIT_DATA(pxTaskToDelete);                         \
    }
#endif

#ifndef traceTASK_DELAY_UNTIL
    #define traceTASK_DELAY_UNTIL( xTimeToWake ) {                               \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_DELAY_UNTIL);             \
        FREERTOS_EMIT_DATA(xTimeToWake);                            \
    }
#endif

#ifndef traceTASK_DELAY
    #define traceTASK_DELAY() {                                     \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_DELAY);                   \
    }
#endif

#ifndef traceTASK_PRIORITY_SET
    #define traceTASK_PRIORITY_SET( pxTask, uxNewPriority ) {       \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_PRIORITY_SET);            \
        FREERTOS_EMIT_DATA(pxTask);                                 \
        FREERTOS_EMIT_DATA(uxNewPriority);                          \
    }
#endif

#ifndef traceTASK_SUSPEND
    #define traceTASK_SUSPEND( pxTaskToSuspend ) {                  \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_SUSPEND);                 \
        FREERTOS_EMIT_DATA(pxTaskToSuspend);                        \
    }
#endif

#ifndef traceTASK_RESUME
    #define traceTASK_RESUME( pxTaskToResume ) {                    \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_RESUME);                  \
        FREERTOS_EMIT_DATA(pxTaskToResume);                         \
    }
#endif

#ifndef traceTASK_RESUME_FROM_ISR
    #define traceTASK_RESUME_FROM_ISR( pxTaskToResume ) {           \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_RESUME_FROM_ISR);         \
        FREERTOS_EMIT_DATA(pxTaskToResume);                         \
    }
#endif

#ifdef FREERTOS_ENABLE_TIMER_TICK_TRACE
#ifndef traceTASK_INCREMENT_TICK
    #define traceTASK_INCREMENT_TICK( xTickCount ) {                \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_INCREMENT_TICK);          \
        FREERTOS_EMIT_DATA(xTickCount);                             \
    }
#endif
#endif

#ifndef traceTIMER_CREATE
    #define traceTIMER_CREATE( pxNewTimer ) {                       \
        FREERTOS_EMIT_EVENT(FREERTOS_TIMER_CREATE);                 \
        FREERTOS_EMIT_DATA(pxNewTimer);                             \
        FREERTOS_EMIT_DATA(pxNewTimer->uxTimerNumber);              \
    }
#endif

#ifndef traceTIMER_CREATE_FAILED
    #define traceTIMER_CREATE_FAILED() {                            \
        FREERTOS_EMIT_EVENT(FREERTOS_TIMER_CREATE_FAILED);          \
    }
#endif

#ifndef traceTIMER_COMMAND_SEND
    #define traceTIMER_COMMAND_SEND( xTimer, xMessageID, xMessageValueValue, xReturn ) {    \
        FREERTOS_EMIT_EVENT(FREERTOS_TIMER_COMMAND_SEND);           \
        FREERTOS_EMIT_DATA(xTimer);                                 \
        FREERTOS_EMIT_DATA(xMessageID);                             \
    }
#endif

#ifndef traceTIMER_EXPIRED
    #define traceTIMER_EXPIRED( pxTimer ) {                         \
        FREERTOS_EMIT_EVENT(FREERTOS_TIMER_EXPIRED);                \
        FREERTOS_EMIT_DATA(pxTimer);                                \
    }
#endif

#ifndef traceTIMER_COMMAND_RECEIVED
    #define traceTIMER_COMMAND_RECEIVED( pxTimer, xMessageID, xMessageValue ) { \
        FREERTOS_EMIT_EVENT(FREERTOS_TIMER_COMMAND_RECEIVED);       \
        FREERTOS_EMIT_DATA(pxTimer);                                \
        FREERTOS_EMIT_DATA(xMessageID);                             \
    }
#endif

#if 0
#ifndef traceMALLOC
    #define traceMALLOC( pvAddress, uiSize ) {                      \
        FREERTOS_EMIT_EVENT(FREERTOS_MALLOC);                       \
        FREERTOS_EMIT_DATA(pvAddress);                              \
        FREERTOS_EMIT_DATA(uiSize);                                 \
    }
#endif

#ifndef traceFREE
    #define traceFREE( pvAddress, uiSize ) {                        \
        FREERTOS_EMIT_EVENT(FREERTOS_FREE);                         \
        FREERTOS_EMIT_DATA(pvAddress);                              \
        FREERTOS_EMIT_DATA(uiSize);                                 \
    }
#endif
#endif

#ifndef traceEVENT_GROUP_CREATE
    #define traceEVENT_GROUP_CREATE( xEventGroup ) {                \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_CREATE);           \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef traceEVENT_GROUP_CREATE_FAILED
    #define traceEVENT_GROUP_CREATE_FAILED() {                      \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_CREATE_FAILED);    \
    }
#endif

#ifndef traceEVENT_GROUP_SYNC_BLOCK
    #define traceEVENT_GROUP_SYNC_BLOCK( xEventGroup, uxBitsToSet, uxBitsToWaitFor ) {  \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_SYNC_BLOCK);       \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef traceEVENT_GROUP_SYNC_END
    #define traceEVENT_GROUP_SYNC_END( xEventGroup, uxBitsToSet, uxBitsToWaitFor, xTimeoutOccurred ) {  \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_SYNC_END);         \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef traceEVENT_GROUP_WAIT_BITS_BLOCK
    #define traceEVENT_GROUP_WAIT_BITS_BLOCK( xEventGroup, uxBitsToWaitFor ) {  \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_WAIT_BITS_BLOCK);  \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef traceEVENT_GROUP_WAIT_BITS_END
    #define traceEVENT_GROUP_WAIT_BITS_END( xEventGroup, uxBitsToWaitFor, xTimeoutOccurred ) {  \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_WAIT_BITS_END);    \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef traceEVENT_GROUP_CLEAR_BITS
    #define traceEVENT_GROUP_CLEAR_BITS( xEventGroup, uxBitsToClear ) { \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_CLEAR_BITS);       \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
        FREERTOS_EMIT_DATA(uxBitsToClear);                          \
    }
#endif

#ifndef traceEVENT_GROUP_CLEAR_BITS_FROM_ISR
    #define traceEVENT_GROUP_CLEAR_BITS_FROM_ISR( xEventGroup, uxBitsToClear ) {    \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_CLEAR_BITS_FROM_ISR);              \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
        FREERTOS_EMIT_DATA(uxBitsToClear);                          \
    }
#endif

#ifndef traceEVENT_GROUP_SET_BITS
    #define traceEVENT_GROUP_SET_BITS( xEventGroup, uxBitsToSet ) { \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_SET_BITS);         \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
        FREERTOS_EMIT_DATA(uxBitsToSet);                            \
    }
#endif

#ifndef traceEVENT_GROUP_SET_BITS_FROM_ISR
    #define traceEVENT_GROUP_SET_BITS_FROM_ISR( xEventGroup, uxBitsToSet ) {    \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_SET_BITS_FROM_ISR);            \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
        FREERTOS_EMIT_DATA(uxBitsToSet);                            \
    }
#endif

#ifndef traceEVENT_GROUP_DELETE
    #define traceEVENT_GROUP_DELETE( xEventGroup ) {                \
        FREERTOS_EMIT_EVENT(FREERTOS_EVENT_GROUP_DELETE);           \
        FREERTOS_EMIT_DATA(xEventGroup);                            \
    }
#endif

#ifndef tracePEND_FUNC_CALL
    #define tracePEND_FUNC_CALL(xFunctionToPend, pvParameter1, ulParameter2, ret) { \
        FREERTOS_EMIT_EVENT(FREERTOS_PEND_FUNC_CALL);                               \
        FREERTOS_EMIT_DATA(xFunctionToPend);                        \
        FREERTOS_EMIT_DATA(pvParameter1);                           \
    }
#endif

#ifndef tracePEND_FUNC_CALL_FROM_ISR
    #define tracePEND_FUNC_CALL_FROM_ISR(xFunctionToPend, pvParameter1, ulParameter2, ret) {    \
        FREERTOS_EMIT_EVENT(FREERTOS_PEND_FUNC_CALL_FROM_ISR);      \
        FREERTOS_EMIT_DATA(xFunctionToPend);                        \
        FREERTOS_EMIT_DATA(pvParameter1);                           \
    }
#endif

#ifndef traceQUEUE_REGISTRY_ADD
    #define traceQUEUE_REGISTRY_ADD(xQueue, pcQueueName) {          \
        int i, len;                                                 \
        FREERTOS_EMIT_EVENT(FREERTOS_QUEUE_REGISTRY_ADD);           \
        FREERTOS_EMIT_DATA(xQueue);                                 \
        len = strlen(pcQueueName);                                  \
        for (i = 0; i < len; i++) {                                 \
            Xil_Out8(0xf8000018, pcQueueName[i]);                   \
        }                                                           \
    }
#endif

#ifndef traceTASK_NOTIFY_TAKE_BLOCK
    #define traceTASK_NOTIFY_TAKE_BLOCK() {                         \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_TAKE_BLOCK);       \
    }
#endif

#ifndef traceTASK_NOTIFY_TAKE
    #define traceTASK_NOTIFY_TAKE() {                               \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_TAKE);             \
    }
#endif

#ifndef traceTASK_NOTIFY_WAIT_BLOCK
    #define traceTASK_NOTIFY_WAIT_BLOCK() {                         \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_WAIT_BLOCK);       \
    }
#endif

#ifndef traceTASK_NOTIFY_WAIT
    #define traceTASK_NOTIFY_WAIT() {                               \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_WAIT);             \
    }
#endif

#ifndef traceTASK_NOTIFY
    #define traceTASK_NOTIFY() {                                    \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY);                  \
    }
#endif

#ifndef traceTASK_NOTIFY_FROM_ISR
    #define traceTASK_NOTIFY_FROM_ISR() {                           \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_FROM_ISR);         \
    }
#endif

#ifndef traceTASK_NOTIFY_GIVE_FROM_ISR
    #define traceTASK_NOTIFY_GIVE_FROM_ISR() {                      \
        FREERTOS_EMIT_EVENT(FREERTOS_TASK_NOTIFY_GIVE_FROM_ISR);    \
    }
#endif

#endif /* ENABLE_FREERTOS_TRACE */

#ifdef __cplusplus
}
#endif

#endif /* _XFREERTOS_STM_TRACE_H_ */
