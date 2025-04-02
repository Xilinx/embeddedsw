/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#ifdef __cplusplus
extern "C" {
#endif

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
#ifdef __cplusplus
}
#endif
#endif /* KEYMGMT_H */
