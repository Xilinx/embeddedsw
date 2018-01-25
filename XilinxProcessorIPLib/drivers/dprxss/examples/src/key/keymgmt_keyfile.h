/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/


#ifndef KEYMGMT_KEYFILE_H
#define KEYMGMT_KEYFILE_H


/* Include Files */
#include "keymgmt.h"
#include <stdint.h>


/* Constant Definitions */
#define KEYFILE_HANDLE_NULL            (-1)


/* Type Definitions */
typedef struct
{
  union
  {
    uint8_t  fU8[336];
    uint64_t fU64[42];

  } fData;

} KEYFILE_tDevKeyTable;

typedef int  KEYFILE_tHandle;

#if WRITE_KEYS_IN_EEPROM
typedef u8 iicAddressType;
#endif

/* Function Prototypes */
void
KEYFILE_Init(void);


KEYFILE_tHandle
KEYFILE_Validate(const char* theFileName, void* theKey);

int
KEYFILE_Close(KEYFILE_tHandle* theHandlePtr);


int
KEYFILE_Read(KEYFILE_tHandle theHandle, KEYFILE_tDevKeyTable* theTable);


#endif  /* KEYMGMT_KEYFILE_H */
