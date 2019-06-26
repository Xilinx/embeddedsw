/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* @file xplmi_generic.h
*
* This is the file which contains .
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_GENERIC_H
#define XPLMI_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xplmi_debug.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xplmi_modules.h"
#include "xplmi_dma.h"
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
enum {
	XPLMI_ERR_MASKPOLL = 0x10,
	XPLMI_ERR_MASKPOLL64,
	XPLMI_ERR_CMD_NOT_SUPPORTED,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define LL_HEADER(LENGTH, COMMAND)                              \
    (((LENGTH) << 16) | (LL_MODULE_ID << 8) | (COMMAND))
#define LL_ADDR32(ADDR) ((uint32_t)(ADDR))
#define LL_ADDR64(ADDR) ((uint32_t)(ADDR)), ((uint32_t)((ptrdiff_t)(ADDR) >> 32))

#define LL_MASK_POLL(ADDR, MASK, VALUE, TIME)                   \
    LL_HEADER(4*4, 1), LL_ADDR32(ADDR), (MASK), (VALUE), (TIME)

#define LL_MASK_WRITE(ADDR, MASK, VALUE)                \
    LL_HEADER(3*4, 2), LL_ADDR32(ADDR), (MASK), (VALUE)

#define LL_WRITE(ADDR, MASK, VALUE)             \
    LL_HEADER(2*4, 3), LL_ADDR32(ADDR), (VALUE)

#define LL_DELAY(TIME)                          \
    LL_HEADER(1*4, 4), (TIME)

#define LL_DMA_WRITE(ADDR, LEN, ...)                            \
    LL_HEADER((2 + (LEN))*4, 5), LL_ADDR32(ADDR), __VA_ARGS__

#define LL_MASK_POLL64(ADDR, MASK, VALUE, TIME)                 \
    LL_HEADER(5*4, 6), LL_ADDR64(ADDR), (MASK), (VALUE), (TIME)

#define LL_MASK_WRITE64(ADDR, MASK, VALUE)              \
    LL_HEADER(4*4, 7), LL_ADDR64(ADDR), (MASK), (VALUE)

#define LL_WRITE64(ADDR, MASK, VALUE)           \
    LL_HEADER(3*4, 8), LL_ADDR64(ADDR), (VALUE)

#define LL_DMA_XFER(SRC, DST, LEN, PARAM)                      \
    LL_HEADER(6*4, 9), LL_ADDR64(SRC), LL_ADDR64(DST), (LEN), (PARAM)

#define XPLMI_SBI_DEST_ADDR				(0xFFFFFFFFFFFFFFFFL)
#define XPLMI_READBK_INTF_TYPE_SMAP		(0x0U)
#define XPLMI_READBK_INTF_TYPE_JTAG		(0x1U)
#define XPLMI_READBK_INTF_TYPE_DDR		(0x2U)
/************************** Function Prototypes ******************************/
void XPlmi_GenericInit(void);
int XPlmi_CfiRead(XPlmi_Cmd * Cmd);
int XPlmi_DmaWriteKeyHole(XPlmi_Cmd * Cmd);


/************************** Variable Definitions *****************************/

/*****************************************************************************/




#ifdef __cplusplus
}
#endif

#endif /* XPLMI_GENERIC_H */
