/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
#endif /* KEYMGMT_LOADER_H */
