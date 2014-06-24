////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Xilinx, Inc.  All rights reserved.
//
// Xilinx, Inc.
// XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
// STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
// XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Id: intr.h,v 1.1.2.1 2011/08/25 12:12:51 anirudh Exp $
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------------------//
//! @file mem.h
//! Kernel level memory allocation definitions and declarations
//----------------------------------------------------------------------------------------------------//

#ifndef _INTR_H
#define _INTR_H

#include <config/config_param.h>
#include <config/config_cparam.h>
#include <sys/ktypes.h>
#include <sys/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char int_id_t;

unsigned int register_int_handler (int_id_t intr_id, void (*handler)(void*), void *callback);
void unregister_int_handler (int_id_t intr_id);
void enable_interrupt (int_id_t intr_id);
void disable_interrupt (int_id_t intr_id);
void acknowledge_interrupt (int_id_t intr_id);


#ifdef __cplusplus
}       
#endif 

#endif /* _INTR_H */
