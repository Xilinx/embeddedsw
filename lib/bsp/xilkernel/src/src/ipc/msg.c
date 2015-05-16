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
//! @file msg.c
//! This contains functions handling the Message Queue functionality
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <sys/process.h>
#include <sys/ktypes.h>
#include <sys/kmsg.h>
#include <sys/decls.h>
#include <sys/bufmalloc.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ksemaphore.h>

#ifdef CONFIG_MSGQ
//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//
// The statically allocated message queues in the system
msgid_ds msgq_heap[NUM_MSGQS] ;

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//
msgid_ds* get_msgid_by_key( key_t key);
extern void  sys_buffree   (membuf_t, void* mem);
extern void* sys_bufmalloc (membuf_t, size_t size);
extern void  sys_buffree   (membuf_t, void* mem);

#ifdef CONFIG_ENHANCED_MSGQ
#define MSGQ_MALLOC(siz)        malloc(siz)
#define MSGQ_FREE(ptr)          free(ptr)
#else
#define MSGQ_MALLOC(siz)        sys_bufmalloc(MEMBUF_ANY, siz)
#define MSGQ_FREE(ptr)          sys_buffree(MEMBUF_ANY, ptr)
#endif

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
msgid_ds* get_msgid_by_key (key_t key)
{
    int i;
    msgid_ds *msgds = msgq_heap;

    for (i=0; i<NUM_MSGQS; i++) {
	if ((msgds->msgid != -1) &&  // Structure valid
	   (msgds->key == key))      // Key match
	    return msgds;
	msgds++;
    }

    return NULL;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_msgget
//! @desc
//!   Get a Message Queue Identifier
//!   - Create a message queue identifier with key as the key, if IPC_CREAT is specified
//!   - If IPC_EXCL is also specified, then return the already existing message queue referenced by key
//!   - The message queue size is initialized based on the configuration specified
//! @param
//!   - key is the unique key identifying this message queue
//!   - msgflg is the message queue creation flags ( IPC_CREAT, IPC_EXCL )
//! @return
//!   - Return -1 on error. msgid of the created/retrieved message queue on success.
//!     errno set to,
//!     EEXIST - If a message queue identifier exists for the argument key but ((msgflg & IPC_CREAT) &&
//!              (msgflg & IPC_EXCL)) is non-zero.
//!     ENOENT - A message queue identifier does not exist for the argument key and (msgflg & IPC_CREAT)
//!              is 0.
//!     ENOSPC - If the message queue resources are exhausted
//! @note
//!   - IPC_PRIVATE not supported yet.
//----------------------------------------------------------------------------------------------------//
int sys_msgget (key_t key, int msgflg)
{
    msgid_ds *msgds ;
    int i;

    msgds = get_msgid_by_key (key);
    if (msgds != NULL) {
	if ((msgflg & IPC_CREAT) && (msgflg & IPC_EXCL)) {
            kerrno = EEXIST;
	    return -1;                                                                  // Create requested, but msgq already exists
        }
	else if ((msgflg & IPC_CREAT))
	    return msgds->msgid;                                                        // Return already existing msgq
	else return -1;                                                                 // Unsupported msgflg
    }
    else if (key == IPC_PRIVATE)
	return -1;
    else {                                                                              // msgds == NULL && key != IPC_PRIVATE
	if (!(msgflg & IPC_CREAT)) {
            kerrno = ENOENT;
	    return -1;
        }
    }

    msgds = msgq_heap;
    for (i=0; i<NUM_MSGQS; i++) {
	if (msgds->msgid == -1)
	    break;
	msgds++ ;
    }

    if (i == NUM_MSGQS) {                                                                // Message Queues exhausted
        kerrno = ENOSPC;
	return -1;
    }

    msgds->msgid = i;
    msgds->key = key;
    if (sys_sem_init (&msgds->full, 0, msgds->msgq_len) < 0)
	return -1;
    if (sys_sem_init (&msgds->empty, 0, 0) < 0)
	return -1;

    qinit (&msgds->msg_q);
    msgds->stats.msg_qnum = 0;
    msgds->stats.msg_lspid =  0;
    msgds->stats.msg_lrpid =  0;

    return i;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_msgctl
//! @desc
//!   Control actions on a message queue
//!   - Check if operation requested on a valid message queue. Return error if not valid.
//!   - If cmd is IPC_STAT, return statistics in user level structure
//!   - If cmd is IPC_RMID, destroy this message queue. Destroy also forcibly terminates
//!   - all currently blocked processes. Also deallocates currently stored messages.
//! @param
//!   - msgid is the msgid of the message queue
//!   - cmd is the operation requested
//!   - msqid_ds is the user level structure for storing stats
//! @return
//!   - Return 0 on success. -1 on failure.
//!     errno set to,
//!     EINVAL - If msgid parameter refers to an invalid message queue or if cmd is invalid or if buf
//!              contains invalid parameters.
//! @note
//!   - IPC_SET command not supported.
//----------------------------------------------------------------------------------------------------//
int sys_msgctl (int msgid, int cmd, struct msqid_ds *buf)
{
    msgid_ds *msgds ;
    msg_t k_msg;
    int i;

    if ((msgid < 0) || (msgid >= NUM_MSGQS) || (msgq_heap[msgid].msgid == -1)) {
        kerrno = EINVAL;
	return -1;
    }

    msgds = &msgq_heap[msgid];

    if (cmd == IPC_STAT) {
	if (buf == NULL) {
            kerrno = EINVAL;
	    return -1;
        }
	buf->msg_qnum = msgds->stats.msg_qnum;                                  // Number of messages currently in the queue
	buf->msg_qbytes = msgds->stats.msg_qbytes;                              // Maximum number of bytes allowed in the queue
	buf->msg_lspid = msgds->stats.msg_lspid;
	buf->msg_lrpid = msgds->stats.msg_lrpid;
    }
    else if (cmd == IPC_RMID) {
	if (sem_force_destroy (&msgds->full) < 0)
	    return -1;
	if (sem_force_destroy (&msgds->empty) < 0)
	    return -1;

	for (i=0; i < msgds->msg_q.item_count; i++) {
	    deq (&msgds->msg_q,&k_msg,0);
	    MSGQ_FREE (k_msg.msg_buf);
	    msgds->msg_q.item_count--;
	}
	msgds->stats.msg_lspid = -1;
	msgds->stats.msg_lrpid = -1;
	msgds->msgid = -1 ;
    }
    else if (cmd == IPC_SET) {
	// Do nothing.
	// We are not supporting any of the fields that IPC_SET is capable of setting.
	// Also not flagging an error.
    }
    else {
        kerrno = EINVAL;
        return -1;
    }

    return 0 ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_msgsnd
//! @desc
//!   Send a message through a message queue
//!   - Allocate memory for the message inside the queue.
//!   - Copy the message and store information about the message in the queue.
//!   - If IPC_NOWAIT is specified and the queue is currently full then return -1.
//!     Wake up any process that is blocked on a msgrcv.
//! @param
//!   - msgid -> msg ID of the message queue
//!   - msgp  -> pointer to the message
//!   - msgsz -> size of the message
//!   - msgflg -> msgsnd control flags
//! @return
//!   - Return 0 on success, -1 on failure. Fail if unable to allocate memory for message
//!     errno set to,
//!     EINVAL - The value of msgid is not a valid message queue identifier
//!     ENOSPC - The system could not allocate space for the message
//!     EIDRM  - The message queue was removed from the system during the send operation
//! @note
//!   - Messages are not stored based on POSIX specified message structure
//!   - (i.e. No assumption or use is made about/of a type field in the message)
//----------------------------------------------------------------------------------------------------//
int sys_msgsnd (int msgid, const void *msgp, size_t msgsz, int msgflg)
{
    msgid_ds *msgds ;
    msg_t k_msg;

    if ((msgid < 0) || (msgid >= NUM_MSGQS) || (msgq_heap[msgid].msgid == -1)) {
        kerrno = EINVAL;
	return -1;
    }

    k_msg.msg_buf = MSGQ_MALLOC (msgsz);
    k_msg.msg_len = msgsz;
    if (k_msg.msg_buf == NULL) {                                        // Unable to allocate mem for message
        kerrno = ENOSPC;
	return -1;
    }

    msgds = &msgq_heap[msgid];
    if ((msgflg & IPC_NOWAIT)) {
	if (sys_sem_trywait (&msgds->full) < 0) {
	    MSGQ_FREE (k_msg.msg_buf);
            kerrno = EAGAIN;
	    return -1;                                                  // Can't wait. Return and indicate unable to wait
	}
    } else {
	if (sys_sem_wait_x (&msgds->full) < 0) {
	    MSGQ_FREE (k_msg.msg_buf);
	    return -1;                                                  // sem_wait error
	}

        if (msgq_heap[msgid].msgid == -1) {                             // The message queue was removed from the system during the send operation
	    MSGQ_FREE (k_msg.msg_buf);
            kerrno = EIDRM;
	    return -1;
	}
    }

    memcpy (k_msg.msg_buf, msgp, msgsz);                                // Save the message
    enq (&msgds->msg_q, &k_msg, 0);                                     // Enqueue the msg_t structure in the queue
    msgds->stats.msg_lspid = sys_get_currentPID ();
    msgds->stats.msg_qnum++;
    sys_sem_post (&msgds->empty);                                       // Signal the consumer of mesg in Q
    return 0;
}


//----------------------------------------------------------------------------------------------------//
//  @func - sys_msgrcv
//! @desc
//!   Receive a message through the message queue
//!   - If IPC_NOWAIT is specified, return immediately signalling error if no message is currently
//!     present
//!   - If the message in the queue is larger than the message size requested, then flag an error
//!     if MSG_NOERROR is not specified.
//!   - Copy the message into the user level buffer and deallocate the kernel message memory.
//!   - Wakeup any process blocked on a full message queue
//! @param
//!   - msgid is the msg ID of the message queue
//!   - msgp is the pointer to the user level buffer
//!   - msgsz is the size requested in bytes
//!   - msgtyp is unused
//!   - msgflg is the flag controlling msgrcv
//! @return
//!   - Return number of bytes actually copied. Return -1 on error
//!     errno set to,
//!     EINVAL - If msgid is not a valid message queue identifier
//!     EIDRM  - If the message queue was removed from the system
//!     ENOMSG - msgsz is smaller than the size of the message in the queue
//! @note
//!   - msgtyp IGNORED. Support only FIFO dequeueing of messages
//!   - i.e. messages not assumed to be of the following structure
//!     struct mymsg {
//!       long    mtype;            /* Message type. */
//!       char    mtext[somesize];  /* Message text. */
//!     }
//----------------------------------------------------------------------------------------------------//
ssize_t sys_msgrcv (int msgid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    msgid_ds *msgds;
    msg_t k_msg;
    ssize_t nbytes;

    if ((msgid < 0) || (msgid >= NUM_MSGQS) || (msgq_heap[msgid].msgid == -1)) {
        kerrno = EINVAL;
	return -1;
    }

    msgds = &msgq_heap[msgid] ;
    if (msgflg == IPC_NOWAIT) {
	if (sys_sem_trywait (&msgds->empty) < 0)
	    return (ssize_t)-1 ;
    } else {
	if (sys_sem_wait_x (&msgds->empty) < 0)
	    return (ssize_t)-1;

	// Return here on unblock
	if (msgq_heap[msgid].msgid == -1) {                             // The message queue was removed from the system during the recv operation
            kerrno = EIDRM;
	    return (ssize_t)-1;
        }
    }

    deq (&msgds->msg_q, &k_msg, 0);                                     // Get the msg_t structure from the Queue
    if (k_msg.msg_len > msgsz) {
	if (!(msgflg & MSG_NOERROR)) {
	    MSGQ_FREE (k_msg.msg_buf);                                  // Release message stored in the kernel
            kerrno = E2BIG;                                             // user buffer too small to hold message
	    return -1;
	}
	else
	    nbytes = msgsz;
    } else nbytes = k_msg.msg_len;

    memcpy (msgp, k_msg.msg_buf, nbytes);
    msgds->stats.msg_lrpid = sys_get_currentPID ();
    msgds->stats.msg_qnum--;
    MSGQ_FREE (k_msg.msg_buf);
    sys_sem_post (&msgds->full);                                        // Decrement the message count
    return (ssize_t)nbytes;                                             // Return number of bytes saved
}
#endif /* CONFIG_MSGQ */
