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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*
******************************************************************************/

/*****************************************************************************/
/**
* @file keymgmt_device.h
*
* This file contains the interface for the key management module.
*
******************************************************************************/


#ifndef KEYMGMT_DEVICE_H
#define KEYMGMT_DEVICE_H


/* Include Files */
#include "keymgmt.h"
#include <stdint.h>


/* Constant Definitions */


/* Function Prototypes */
KEYMGMT_tError
KEYMGMTDEV_Init(void);


KEYMGMT_tError
KEYMGMTDEV_Probe(KEYMGMT_tDevID theDevID, KEYMGMT_tTableID* theNumTables,
		KEYMGMT_tRowID* theNumRowsPerTable);


KEYMGMT_tError
KEYMGMTDEV_Enable(KEYMGMT_tDevID theDevID);

KEYMGMT_tError
KEYMGMTDEV_Disable(KEYMGMT_tDevID theDevID);


KEYMGMT_tError
KEYMGMTDEV_Load(KEYMGMT_tDevID theDevID, KEYMGMT_tTableID theTableID,
		const uint64_t* theBuf, int theBufSize);

KEYMGMT_tError
KEYMGMTDEV_Verify(KEYMGMT_tDevID theDevID, KEYMGMT_tTableID theTableID,
		const uint64_t* theBuf, int theBufSize);


uint32_t
KEYMGMTDEV_GetVersion(KEYMGMT_tDevID theDevID);


int
KEYMGMTDEV_Debug(int argc, const char* argv[]);


#endif /* KEYMGMT_DEVICE_H */
