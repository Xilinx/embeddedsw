/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file keymgmt_loader.h
*
* This file contains the interface for the key management loader module.
*
******************************************************************************/


#ifndef KEYMGMT_LOADER_H
#define KEYMGMT_LOADER_H


/* Include Files */
#include "keymgmt.h"
#include <stdint.h>


/* Constant Definitions */


/* Type Definitions */


/* Function Prototypes */
KEYMGMT_tError
KEYMGMTLDR_Init(void);


void
KEYMGMTLDR_Poll(uint32_t theUpTime);


int
KEYMGMTLDR_Debug(int argc, const char* argv[]);


#endif /* KEYMGMT_LOADER_H */
