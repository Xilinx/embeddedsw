/******************************************************************************
*
* Copyright (C) 2004 - 2014 Xilinx, Inc.  All rights reserved.
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
//! @file shm.h
//! Standard POSIX Shared memory declarations and definitions
//----------------------------------------------------------------------------------------------------//
#ifndef _SHM_H
#define _SHM_H

#include <sys/types.h>
#include <sys/ipc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define SHM_RDONLY    1     // Attach read-only (else read-write). 
#define SHM_RND       2     // Round attach address to SHMLBA. 

#define SHMLBA        4     // Segment low boundary address multiple. 



typedef unsigned int shmatt_t;

// Each Shared Memory is associated with this structure.
// @Note: shmid_ds not fully posix compliant.
// Commented out fields unsupported.
struct shmid_ds {
    //struct ipc_perm shm_perm;          // Operation permission structure. 
    size_t          shm_segsz;           // Size of segment in bytes. 
    pid_t           shm_lpid;            // Process ID of last shared memory operation. 
    pid_t           shm_cpid;            // Process ID of creator. 
    shmatt_t        shm_nattch;          // Number of current attaches. 
    //time_t          shm_atime;         // Time of last shmat (). 
    //time_t          shm_dtime;         // Time of last shmdt (). 
    //time_t          shm_ctime ;        // Time of last change by shmctl ().
};

int   shmget (key_t key, size_t size, int shmflg);
int   shmctl (int shmid, int cmd, struct shmid_ds* buf);
void* shmat (int shmid, const void *shmaddr, int shmflg);
int   shmdt (const void *shmaddr);

#ifdef __cplusplus
}       
#endif 

#endif /* _SHM_H */
