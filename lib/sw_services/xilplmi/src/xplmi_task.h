/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_task.h
*
* This is the file which contains command execution code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/06/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_TASK_H
#define XPLMI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include <xil_assert.h>
#include "mb_interface.h"
#include "xil_types.h"
#include "xstatus.h"
#include "list.h"
#include "xplmi_proc.h"

/************************** Constant Definitions *****************************/
#define XPLMI_TASK_MAX			(32U)
#define XPLMI_TASK_PRIORITIES		(2U)

enum PRIORITY {
        XPLM_TASK_PRIORITY_0 = 0U,
        XPLM_TASK_PRIORITY_1,
};

/**************************** Type Definitions *******************************/
typedef struct XPlmi_TaskNode XPlmi_TaskNode;

struct XPlmi_TaskNode {
    u32 Priority;
    u32 Delay;
    struct metal_list TaskNode;
    int (*Handler)(void * PrivData);
    void * PrivData;
};

/***************** Macros (Inline Functions) Definitions *********************/
/** Compute offset of a field within a structure. */
#define metal_offset_of(structure, member)		\
	((uintptr_t) &(((structure *) 0)->member))

/** Compute pointer to a structure given a pointer to one of its fields. */
#define metal_container_of(ptr, structure, member)	\
	(void *)((uintptr_t)(ptr) - metal_offset_of(structure, member))

/************************** Function Prototypes ******************************/
XPlmi_TaskNode * XPlmi_TaskCreate(u32 Priority,
	int (*Handler)(void * PrivData), void * PrivData);
void XPlmi_TaskDelete(XPlmi_TaskNode * Task);
void XPlmi_TaskTriggerNow(XPlmi_TaskNode * Task);
void XPlmi_TaskInit(void );
void XPlmi_TaskDispatchLoop(void );
/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLMI_TASK_H */
