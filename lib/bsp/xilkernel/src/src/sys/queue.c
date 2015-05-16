/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
//----------------------------------------------------------------------------------------------------//
//! @file queue.c
//! This file contains the Queue Handling routines.
//----------------------------------------------------------------------------------------------------//

#include <stdio.h>
#include <string.h>
#include <os_config.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/queue.h>
#include <sys/mem.h>
#include <sys/process.h>

#define LEFT_CHILD(index)  ((index << 1) + 1)
#define RIGHT_CHILD(index) ((index << 1) + 2)
#define PARENT(index)      ((int)((index - 1) >> 1))
#define PROC_PRIO(index)   (ptable[index].priority)

extern process_struct ptable[MAX_PROCESS_CONTEXTS];

//----------------------------------------------------------------------------------------------------//
//  @func alloc_q
//! @desc
//!   Initialize a Queue - Allocate array of memory to the Queue. Called
//!   during system initialization.
//! @param
//!   - queue is the queue of items.
//!   - max_items is the maximum queue length.
//!   - size is size of the elements in the queue.
//!   - qtype is the type of Q. Each Q is allocated a static memory
//!   - qno is the queue number, In case of multiple queue's of
//!     the same type.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void alloc_q (queuep queue, unsigned char max_items,
	      unsigned char qtype, unsigned short size, unsigned char qno)
{
    queue->item_count = queue->qfront = queue->qend = 0;
    queue->max_items = max_items;
    queue->item_size = size;
    if (qtype != MSG_Q)
	alloc_pidq_mem (queue, qtype, qno);
#ifdef CONFIG_MSGQ
    else
	alloc_msgq_mem (queue, qno);
#endif
   // Check for NULL
}


//----------------------------------------------------------------------------------------------------//
//  @func - qinit
//! @desc
//!   Intialize the Queue.
//! @param
//!   - queue is the queue of items.
//! @return
//!   - Nothing
//! @note
//!   - None
//----------------------------------------------------------------------------------------------------//
void qinit (queuep queue)
{
    queue->item_count = queue->qfront = queue->qend = 0 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - enq
//! @desc
//!   enqueue the item in the Queue.
//! @param
//!   - queue is the queue of items.
//!   - item is the queue element.
//!   - key for insertion into the Queue. Can be priority.
//! @return
//!   - Nothing
//! @note
//!   - key is not used currently
//!   - No error is flagged if queue is full
//----------------------------------------------------------------------------------------------------//
void enq (queuep queue, const void *item, unsigned short key)
{
    unsigned char qend = queue->qend;

    key = 0;
    // Queue full
    if (queue->item_count == queue->max_items) {
	return;
    }

    memcpy ((queue->items+(qend*queue->item_size)),item,queue->item_size);
    queue->qend = (qend+1) % (queue->max_items);
    queue->item_count++;
}

//----------------------------------------------------------------------------------------------------//
//  @func - deq
//! @desc
//!   Dequeue the process from the Queue, based on the schemes.
//! @param
//!   - queue is the queue of items.
//!   - item is buffer where queue element is returned.
//!     If queue is empty then NULL is returned.
//!   - key for removing from the Queue.
//! @return
//!   - Queue element is returned in item.
//!   - Null is assigned to item if Error.
//! @note
//!   - No error is flagged on queue being empty
//----------------------------------------------------------------------------------------------------//
void deq (queuep queue, void *item, unsigned short key)
{
    unsigned char qfront = queue->qfront ;

    key = 0;
    // Queue EMPTY
    if (queue->item_count == 0) {
	item = NULL ;
	return ;
    }

    memcpy (item,(queue->items+(qfront*queue->item_size)),queue->item_size);
    queue->qfront = (qfront+1)%(queue->max_items);
    queue->item_count--;
    return ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - penq
//! @desc
//!   Enqueue a process id in the Queue.
//! @param
//!   - queue is the queue of items.
//!   - item is the queue element.
//!   - key for insertion into the Queue. Can be priority.
//! @return
//!   - Nothing
//! @note
//!   - key is not used currently
//!   - No error is flagged if queue is full.
//----------------------------------------------------------------------------------------------------//
void penq (queuep queue, pid_t item, unsigned short key)
{
    unsigned char qend = queue->qend ;
    pid_t* citems = (pid_t*)(queue->items);

    key = 0;
    // Queue full
    if (queue->item_count == queue->max_items) {
	return ;
    }

    citems[qend] = item;
    queue->qend = (qend + 1) % (queue->max_items) ;
    queue->item_count++;
}

//----------------------------------------------------------------------------------------------------//
//  @func - pdeq
//! @desc
//!   Dequeue the process from the Queue, based on the schemes.
//! @param
//!   - queue is the queue of items.
//!   - item is buffer where queue element is returned.
//!     If queue is empty then 255 is returned.
//!   - key for removing from the Queue.
//! @return
//!   - Queue element is returned in item.
//!   - 255 (-1) is assigned to item if Error.
//! @note
//!   - No error is flagged if queue is empty
//----------------------------------------------------------------------------------------------------//
void pdeq (queuep queue, pid_t *item, unsigned short key)
{
    unsigned char qfront = queue->qfront ;
    pid_t *citems = (pid_t*) (queue->items);

    key = 0;
    // Queue EMPTY
    if (queue->item_count == 0) {
	*item = 255;
	return ;
    }
    *item = citems[qfront];
    queue->qfront = (qfront + 1) % (queue->max_items);
    queue->item_count--;
    return ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - pdelq
//! @desc
//!   Delete a pid from the Queue.
//! @param
//!   - queue is the queue of items.
//!   - item is the element to delete.
//!     If queue is empty then -1 is returned.
//! @return
//!   - Return 0 on success and -1 on errors
//! @note
//!   - Since the Queue implementation is array, this operation is very
//!     expensive as other entries needs to be copied.
//----------------------------------------------------------------------------------------------------//
int pdelq (queuep queue, pid_t item)
{
    unsigned char qfront = queue->qfront;
    unsigned char qend = queue->qend;
    unsigned char item_count = queue->item_count;
    unsigned char max_items = queue->max_items;
    pid_t *citems = (pid_t*) (queue->items);

    if (item_count == 0)
	return -1;	/* Queue EMPTY */

    while ((item_count) && (item != citems[qfront])) {
	item_count--;
	qfront = (qfront + 1) % max_items ;
    }

    if (!item_count)
	return -1;

    if (qfront == queue->qfront)
	queue->qfront = (qfront + 1) % max_items;
    else {
	unsigned int temp = qfront;
	while (qfront != qend) {
	    qfront = (qfront + 1) % max_items;
	    citems[temp] = citems[qfront];
	    temp = qfront;
	}
	queue->qend = (qend + max_items - 1) % max_items ;
    }
    queue->item_count--;
    return 0 ;
}

#if SCHED_TYPE == SCHED_PRIO
//----------------------------------------------------------------------------------------------------//
//  @func - prio_penq
//! @desc
//!   Enqueue a process in the Priority Queue.
//! @param
//!   - queue is the queue of items.
//!   - item is the queue element.
//!   - Key for insertion (priority).
//! @return
//!   - Nothing
//! @note
//!   - No error is flagged if queue is full.
//!   - Uses queue array structure as a binary heap for implementing priority queue.
//!     0 is highest priority
//!   - The priority key is not used. Instead the value is obtained directly from the ptable.
//----------------------------------------------------------------------------------------------------//
void prio_penq (queuep queue, pid_t item, unsigned short key)
{
    pid_t* citems = (pid_t*)(queue->items);
    unsigned char tmp, cur;

    if (queue->item_count == queue->max_items)
        return;

    citems[queue->qend] = item;                                                         // qend points to the next free space in the queue

    cur = queue->qend;                                                                  // Propagate new value up the heap
    while (cur != 0) {
        if (PROC_PRIO (citems[PARENT (cur)]) > PROC_PRIO (citems[cur])) {
            tmp = citems[cur];
            citems[cur] = citems[PARENT (cur)];                                         // Swap
            citems[PARENT (cur)] = tmp;
        }
        else break;
	cur = PARENT (cur);
    }

    queue->qend++;
    queue->item_count++;
}

//----------------------------------------------------------------------------------------------------//
//  @func - prio_pdeq
//! @desc
//!   Dequeue the highest priority process from the Queue.
//! @param
//!   - queue is the queue of items.
//!   - item is buffer where queue element is returned.
//!     If queue is empty then 255 is returned.
//!   - Key for removing from the Queue (priority)
//! @return
//!   - Queue element is returned in item.
//!   - 255 (-1) is assigned to item if Error.
//! @note
//!   - Uses queue array structure as a binary heap for implementing priority queue.
//!     0 is highest priority
//!   - The priority key is not used currently. Instead the value is obtained from the ptable directly.
//----------------------------------------------------------------------------------------------------//
void prio_pdeq (queuep queue, pid_t *item, unsigned short key)
{
    pid_t *citems = (pid_t*) (queue->items);
    unsigned char tmp, cur;
    pid_t slct, ret;

    // Queue EMPTY
    if (queue->item_count == 0) {
	*item = 255;
	return;
    }

    ret = citems[0];
                                                                        // Remove head (highest prio)
    citems[0] = citems[queue->qend - 1];                                // Reshape heap (Take lowest prio. Place it at head. Propagate it down the heap)

    cur = 0;
    while (LEFT_CHILD (cur) <= (queue->qend - 1)) {                     // While cur has children in the heap
	if ((RIGHT_CHILD (cur) > (queue->qend - 1)) ||
	    (PROC_PRIO (citems[LEFT_CHILD (cur)]) <= PROC_PRIO (citems[RIGHT_CHILD (cur)])))
	    slct = LEFT_CHILD (cur);
	else
	    slct = RIGHT_CHILD (cur);

	if (PROC_PRIO (citems[slct]) <= PROC_PRIO (citems[cur])) {
	    tmp = citems[cur];                                          // Swap
	    citems[cur] = citems[slct];
	    citems[slct] = tmp;
	}
	else
            break;
	cur = slct;
    }

    queue->qend--;
    queue->item_count--;
    *item = ret;
}

int prio_pdelq (queuep queue, pid_t item)
{
    unsigned char i;
    pid_t *citems = (pid_t*) (queue->items);
    unsigned char qsiz = queue->item_count;
    pid_t tmp;
    signed char priosave;

    if (qsiz == 0)
	return -1;                                                      // Queue EMPTY

    for (i = 0; i < qsiz; i++) {
        if (citems[i] == item)
            break;
    }

    if (i == qsiz)
	return -1;

    priosave = PROC_PRIO (citems[i]);                                   // Found our item. Now do a "decreaseKey" heap operation on it
    PROC_PRIO (citems[i]) = -1;

    while (i != 0) {                                                    // Propagate new value up the heap
        tmp = citems[i];
        citems[i] = citems[PARENT (i)];                                 // Swap
        citems[PARENT (i)] = tmp;
	i = PARENT (i);
    }

    prio_pdeq (queue, &tmp, 0);                                         // Now delete this element from the queue
    PROC_PRIO (tmp) = priosave;                                         // Restore original priority
    return 0;
}
#endif /* SCHED_TYPE == SCHED_PRIO */
