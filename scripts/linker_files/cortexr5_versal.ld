/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

MEMORY
{
   psv_r5_0_atcm_MEM_0 : ORIGIN = 0x0, LENGTH = 0x10000
   psv_r5_0_atcm_lockstep_MEM_0 : ORIGIN = 0xFFE10000, LENGTH = 0x10000
   psv_r5_0_btcm_MEM_0 : ORIGIN = 0x20000, LENGTH = 0x10000
   psv_r5_0_btcm_lockstep_MEM_0 : ORIGIN = 0xFFE30000, LENGTH = 0x10000
   psv_r5_0_data_cache_MEM_0 : ORIGIN = 0xFFE50000, LENGTH = 0x10000
   psv_r5_0_instruction_cache_MEM_0 : ORIGIN = 0xFFE40000, LENGTH = 0x10000
   psv_r5_1_data_cache_MEM_0 : ORIGIN = 0xFFED0000, LENGTH = 0x10000
   psv_r5_1_instruction_cache_MEM_0 : ORIGIN = 0xFFEC0000, LENGTH = 0x10000
   psv_r5_0_atcm_global_MEM_0 : ORIGIN = 0xFFE00000, LENGTH = 0x10000
   psv_r5_1_atcm_global_MEM_0 : ORIGIN = 0xFFE90000, LENGTH = 0x10000
   psv_r5_0_btcm_global_MEM_0 : ORIGIN = 0xFFE20000, LENGTH = 0x10000
   psv_r5_1_btcm_global_MEM_0 : ORIGIN = 0xFFEB0000, LENGTH = 0x10000
}
