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
//! @file shm.c
//! POSIX Shared memory implementation
//----------------------------------------------------------------------------------------------------//
#include <stdio.h>
#include <os_config.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/process.h>
#include <sys/kshm.h>
#include <sys/decls.h>
#include <errno.h>
#include <string.h>

#ifdef CONFIG_SHM

//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------// 
//! Array of Shared Memory structures. They are used to keep track of allocated
//! Shared mem's. Also used for other bookkeeping operations on shared mem.
shm_info_t shm_heap[N_SHM] ;

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------// 
shm_info_t* get_shm_by_key (key_t key);

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
shm_info_t* get_shm_by_key (key_t key)
{
    int i;
    shm_info_t* shmds = shm_heap;

    for (i=0; i<N_SHM; i++) {
	if ((shmds->shm_id != -1) &&                                    // Structure valid
	   (shmds->shm_key == key))                                     // Key match
	    return shmds;
	shmds++;
    }

    return NULL;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_shmget
//! @desc
//!   Create a new shared memory segment based on key
//!   - If creation flags are IPC_CREAT then the shm_id of the shared memory segment created/retrieved
//!     is returned.
//!   - If the shared memory already exists and IPC_EXCL is specified, -1 is returned
//!   - Atleast one of the shared memory segments available in the system must EXACTLY
//!     match the requested size.
//!   - If creating a new segment, initialize memory to zero.
//! @param
//!   - key is the unique key representing the segment
//!   - size is the requested shared memory size
//!   - shmflg is the creation flag
//! @return
//!   - Returns 0 on success, -1 on error
//!     errno set to,
//!     EEXIST - A shared memory identifier exists for the argument key but 
//!              (shmflg &IPC_CREAT) && (shmflg &IPC_EXCL) is non-zero
//!     ENOTSUP - Unsupported shmflg
//!     ENOENT - A shared memory identifier does not exist for the argument key and (shmflg &IPC_CREAT) 
//!              is 0
//! @note
//!   - IPC_PRIVATE is not supported. Size restrictions exist based on system configuration.
//----------------------------------------------------------------------------------------------------//
int sys_shmget (key_t key, size_t size, int shmflg) 
{
    unsigned int i = 0;
    shm_info_t *shmds ;

    shmds = get_shm_by_key (key);
    
    if (shmds != NULL) {
	if ((shmflg & IPC_CREAT) && (shmflg & IPC_EXCL)) {
            kerrno = EEXIST;                                            // Create requested, but shm already exists
	    return -1;                 
        }
	else 
            return shmds->shm_id;                                       // Return already existing shm
    } else if (key == IPC_PRIVATE) {
        kerrno = ENOTSUP;
	return -1;       
    }            
    else {                                                              // shmds == NULL && key != IPC_PRIVATE
	if (!(shmflg & IPC_CREAT)) {   
            kerrno = ENOENT;
	    return -1;               
	}
    }

    shmds = shm_heap;
    for (i=0; i<N_SHM; i++) {
	if ((shmds->shm_id == -1) && (shmds->shm_segsz == size))        // Use only matching shm allocations
	    break;
	shmds++ ;
    }

    if (i == N_SHM) {                                                   // Out of resources
        kerrno = ENOSPC;
	return -1;  
    }

    shmds->shm_id = i;
    shmds->shm_key = key;
    shmds->shm_nattch = 0;
    shmds->shm_lpid = -1;
    shmds->shm_cpid = current_pid;
    bzero ((void *)shmds->shm_addr, shmds->shm_segsz);                         // POSIX requires mem to be initialized to zero.

    return i;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_shmctl
//! @desc
//!   Control the shared memory
//!   - if cmd is IPC_STAT store the statistics for this shared memory segment in buf
//!   - if cmd is IPC_RMID remove this shared memory segment from the system
//! @param
//!   - shmid is the ID of the shared memory segment
//!   - cmd is the shm command
//!   - buf is the store area for shm statistics
//! @return
//!   - Returns 0 on success and -1 on error
//!     errno set to,
//!     EINVAL if shmid refers to an invalid shared memory segment or cmd or other params are invalid. 
//! @note
//!   - IPC_SET is not supported.
//!   - On an IPC_RMID operation, no notification is sent to processes still attached to the segment
//----------------------------------------------------------------------------------------------------//
int sys_shmctl (int shmid, int cmd, struct shmid_ds* buf) 
{
  shm_info_t* shmds ;

  if ((shmid < 0) || (shmid >= N_SHM) || (shm_heap[shmid].shm_id == -1)) {
      kerrno = EINVAL;
      return -1;
  }

  shmds = &shm_heap[shmid] ;
  if (cmd == IPC_STAT) {
      if (buf == NULL) {
          kerrno = EINVAL;
	  return -1;
      }
      buf->shm_segsz = shmds->shm_segsz;
      buf->shm_lpid  = shmds->shm_lpid;
      buf->shm_cpid  = shmds->shm_cpid;
      buf->shm_nattch = shmds->shm_nattch;
  } else if (cmd == IPC_SET) {
                                                                                // Do nothing. We don't support it, but we don't flag an error either.
  } else if (cmd == IPC_RMID) {
      shmds->shm_id = -1;                                                       // Attached processes can still carry on
      shmds->shm_nattch = 0;
  } else {
      kerrno = EINVAL;
      return -1;
  }
  
  return 0;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_shmat
//! @desc
//!   Attach to shared memory segment
//!   - Lookup shared memory segment based on shmid and return pointer to start of the segment
//! @param
//!   - shmid is the ID of the segment
//!   - shmaddr is the address specification of the segment (ignored)
//!   - shmflg is the attach flag (ignored)
//! @return
//!   - Return the address of the segment on success and NULL on error.
//!     errno set to,
//!     EINVAL if shmid refers to an invalid shared memory segment
//! @note
//!   - The shmaddr specification is discarded. We support attaches only to the start of the segment.
//!     No memory mapping of any kind is done.
//!   - shmflg is ignored because it applies to shmaddr
//----------------------------------------------------------------------------------------------------//
void* sys_shmat (int shmid, const void *shmaddr, int flag) 
{
    if ((shmid < 0) || (shmid >= N_SHM) || (shm_heap[shmid].shm_id == -1)) {
        kerrno = EINVAL;
        return NULL;
    }
    
    shm_heap[shmid].shm_nattch++ ; 	
    shm_heap[shmid].shm_lpid = sys_get_currentPID();
    return (void*)(shm_heap[shmid].shm_addr) ;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_shmdt
//! @desc
//!   Detach from shared memory segment
//! @param
//!   - shmaddr is the address of the shared memory segment that is to be detached from
//! @return
//!   - Return's 0 on success and -1 on error.
//!     errno set to,
//!     EINVAL if shmaddr is not within any of the available shared memory segments.
//! @note
//!   - None.
//----------------------------------------------------------------------------------------------------//
int sys_shmdt (const void *shmaddr) 
{
    shm_info_t* shmds;
    unsigned int i = 0;

    shmds = shm_heap;
    for (; i < N_SHM; i++) {
	if ((char *)shmds->shm_addr == (char *)shmaddr)
	    break ;
	shmds++ ;
    }
    if (i == N_SHM) {
        kerrno = EINVAL;
	return -1;
    }
    else 
	shmds->shm_nattch--;

    return 0;
}
#endif	/* CONFIG_SHM */
