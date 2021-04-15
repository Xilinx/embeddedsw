/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
