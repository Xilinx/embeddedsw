/******************************************************************************
* Copyright (C) 2020 - 2021  Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef KEYGEN_CONFIG_H_
#define KEYGEN_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Base address of the IIC that is used to read from EEPROM.
 * The SDT flow locates drivers by base address, not by device ID. */
#define		EEPROM_IIC_1_BASEADDR				XPAR_XIIC_0_BASEADDR

/* Please put the size of the key block array in EEPROM. Current implementation
 * is tested for 736 on KC705 */
#define KEYMGMT_ENCDATA_SZ_EEPROM				736

// 0x54 is the IIC address on KC705
#define EEPROM_ADDR 0x54

#ifdef __cplusplus
}
#endif
#endif /* KEYGEN_CONFIG_H_ */
