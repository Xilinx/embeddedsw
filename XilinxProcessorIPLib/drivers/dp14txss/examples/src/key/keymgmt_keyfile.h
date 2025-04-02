/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



#ifndef KEYMGMT_KEYFILE_H
#define KEYMGMT_KEYFILE_H
#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
#endif  /* KEYMGMT_KEYFILE_H */
