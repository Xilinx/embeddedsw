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
//! @file bufmalloc.c
//! Block memory allocation support
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <stdio.h>
#include <config/config_cparam.h>
#include <config/config_param.h>
#include <sys/arch.h>
#include <sys/types.h>
#include <sys/decls.h>
#include <sys/init.h>
#include <errno.h>
#include <sys/bufmalloc.h>

#ifdef CONFIG_BUFMALLOC
//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//

#define    MEM_TO_BLK(membufp, mem)             (((unsigned int)mem - (unsigned int)membufp->memptr)/(membufp->blksiz))
#define    MEM_WITHIN_BUF(membufp, mem)         (((mem >= membufp->memptr) && (mem < membufp->limit))?1:0)

typedef struct membuf_info_s {
    char        active;
    void        *memptr;
    void        *limit;
    void        *freep;
    int         nblks;
    int         nfree;
    size_t      blksiz;
} membuf_info_t;

void*   get_mbufblk    (membuf_info_t *mbufptr);
extern  void bufmalloc_mem_init (void);
//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//
membuf_info_t   mbufheap[N_MBUFS];

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//
void bufmalloc_init (void)
{
    int i;
    for (i = 0; i < N_MBUFS; i++)
        mbufheap[i].active = 0;

    bufmalloc_mem_init ();
}

void* get_mbufblk (membuf_info_t *mbufptr)
{
    void *ret;

    if (!mbufptr->nfree)
        return NULL;

    ret = mbufptr->freep;
    mbufptr->freep = (*(void**)ret);
    mbufptr->nfree--;

    return ret;
}

//----------------------------------------------------------------------------------------------------//
//  @func - sys_bufcreate
//! @desc
//!   Free the memory allocated by bufmalloc
//! @param
//!   - mem is the address of the memory block, that was got from bufmalloc
//! @return
//!   - Nothing
//! @note
//!   - WARNING. ALIGNMENT requirements have to be met or exception handlers required.
//----------------------------------------------------------------------------------------------------//
int sys_bufcreate (membuf_t *mbuf, void *memptr, int nblks, size_t blksiz)
{
    int i;
    membuf_info_t *mbufptr;
    void **cur, **next;

    if ((mbuf == NULL) || (memptr == NULL) || (nblks <= 0) || (blksiz < sizeof (void*))) {
        kerrno = EINVAL;
        return -1;
    }

    mbufptr = &mbufheap[0];
    for (i = 0; i < N_MBUFS; i++) {                                                     // Find first free membuf descriptor
        if (!mbufptr->active)
            break;
        mbufptr++;
    }

    if (i == N_MBUFS) {
        kerrno = EAGAIN;
        return -1;
    }

    mbufptr->active  = 1;
    mbufptr->memptr  = memptr;
    mbufptr->nblks   = nblks;
    mbufptr->blksiz  = blksiz;
    mbufptr->nfree   = nblks;
    mbufptr->freep   = memptr;
    mbufptr->limit   = (void*)((unsigned int)memptr + (nblks * blksiz));
    *mbuf = (membuf_t)i;                                                                // Return membuf identifier

    cur  = (void**)memptr;
    next = (void**)((unsigned int)memptr + blksiz);
    for (i = 0; i < (nblks - 1); i++) {                                                 // Initialize free list in the the memory block
        *cur = (void*)next;                                                             // Alignment constraints should be met here
        cur  = next;                                                                    // Otherwise, unaligned exceptions will occur
        next = (void**)((unsigned int)next + blksiz);
    }
    *cur = (void*) NULL;
    return 0;
}

int sys_bufdestroy (membuf_t mbuf)
{
    if (mbuf >= 0 && mbuf < N_MBUFS)
        mbufheap[mbuf].active = 0;
    else {
        kerrno = EINVAL;
        return -1;
    }

    return 0;
}

void* sys_bufmalloc (membuf_t mbuf, size_t siz)
{
    membuf_info_t   *mbufptr;
    void* ret = NULL;
    int i;

    if ((mbuf != MEMBUF_ANY) && ((mbuf < 0 || mbuf > N_MBUFS)))
        return NULL;

    if (mbuf == MEMBUF_ANY) {
        mbufptr = &mbufheap[0];
        for (i = 0; i < N_MBUFS; i++) {
            if (mbufptr->active && mbufptr->blksiz >= siz) {                            // Get first pool that can fit the request
                ret = get_mbufblk (mbufptr);
                if (ret)
                    break;
            }
            mbufptr++;
        }
    } else {
        mbufptr = &mbufheap[mbuf];

        if (!mbufptr->active) {
            kerrno = EINVAL;
            return NULL;
        }
        ret = get_mbufblk (mbufptr);
    }

    if (ret == NULL)
        kerrno = EAGAIN;
    return ret;
}

void sys_buffree (membuf_t mbuf, void *mem)
{
    membuf_info_t   *mbufptr;
    void** newblk;
    int blk, i;

    if ((mbuf != MEMBUF_ANY) && ((mbuf < 0 || mbuf > N_MBUFS)))
        return;

    if (mbuf == MEMBUF_ANY) {
        mbufptr = &mbufheap[0];
        for (i = 0; i < N_MBUFS; i++) {
            if (mbufptr->active && MEM_WITHIN_BUF (mbufptr, mem))
                break;
            mbufptr++;
        }
        if (i == N_MBUFS)
            return;
    } else
        mbufptr = &mbufheap[mbuf];

    blk = MEM_TO_BLK (mbufptr, mem);
    if (blk < 0 || blk > mbufptr->nblks)
        return;

    newblk = (void**)((unsigned int)mbufptr->memptr +
                      (mbufptr->blksiz * blk));                                         // Forcing it to be a valid chain offset within the block

    *newblk = mbufptr->freep;
    mbufptr->freep = (void*)newblk;
    mbufptr->nfree++;
}
#endif /* CONFIG_BUFMALLOC */
