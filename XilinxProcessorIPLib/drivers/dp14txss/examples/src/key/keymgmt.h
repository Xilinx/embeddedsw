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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file keymgmt.h
*
* This file contains the interface for the key management module.
*
******************************************************************************/


#ifndef KEYMGMT_H
#define KEYMGMT_H


/* Include Files */
#include <stdint.h>
#include "xil_types.h"
#include "xstatus.h"

#define		DEBUG_HDCP_INIT				0
/* Constant Definitions */
#define KEYMGMT_ERROR_NONE             ( 0)

#define KEYMGMT_DEVID_0                ( 0)
#define KEYMGMT_DEVID_1                ( 1)
#define KEYMGMT_DEVID_UNKNOWN          (99)

#define KEYMGMT_TABLEID_0              ( 0)
#define KEYMGMT_TABLEID_1              ( 1)
#define KEYMGMT_TABLEID_2              ( 2)
#define KEYMGMT_TABLEID_3              ( 3)
#define KEYMGMT_TABLEID_4              ( 4)
#define KEYMGMT_TABLEID_5              ( 5)
#define KEYMGMT_TABLEID_6              ( 6)
#define KEYMGMT_TABLEID_7              ( 7)
#define KEYMGMT_TABLEID_UNKNOWN        (99)

#define KEYMGMT_ROWID_0                ( 0)
#define KEYMGMT_ROWID_UNKNOWN          (99)

/* DEBUG CONSTANTS */
#define DEBUG_KEYMGMT_INIT				1

/* EEPROM Constant */
#define WRITE_KEYS_IN_EEPROM			1

#define IIC_EEPROM_WRITE				1

#define IIC_EEPROM_READ					1


/* Type Definitions */
typedef int      KEYMGMT_tError;
typedef uint8_t  KEYMGMT_tDevID;
typedef uint8_t  KEYMGMT_tTableID;
typedef uint8_t  KEYMGMT_tRowID;
extern const uint8_t USER_KEYMGMT_ENCDATA[];
extern const uint32_t USER_KEYMGMT_ENCDATA_SZ;
extern int gIsKeyWrittenInEeeprom;
/* Function Prototypes */
KEYMGMT_tError
KEYMGMT_Init(void);


void
KEYMGMT_Poll(uint32_t theUptime);


int
KEYMGMT_Debug(int argc, const char* argv[]);

KEYMGMT_tError KEYMGMT_WriteKeysToEeprom(uint8_t *keyBuf, unsigned int keySize);
//KEYMGMT_tError KEYMGMT_WriteKeysToEeprom();

uint32_t iicEepromWriteKeys(uint8_t *userKeyBuf, unsigned int userKeySize,
								int userKeysIsTrue);
//uint32_t iicEepromWriteKeys();

#endif /* KEYMGMT_H */
