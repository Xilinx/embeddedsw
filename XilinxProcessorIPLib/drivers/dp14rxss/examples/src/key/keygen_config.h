/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef KEYGEN_CONFIG_H_
#define KEYGEN_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif

/* Please update the ID of IIC that is being used to read from EEPROM */
#define		EEPROM_IIC_1_DEVICE_ID				XPAR_IIC_0_DEVICE_ID

/* Please put the size of the key block array in EEPROM. Current implementation
 * is tested for 736 on KC705 */
#define KEYMGMT_ENCDATA_SZ_EEPROM				736

// 0x54 is the IIC address on KC705
#define EEPROM_ADDR 0x54

#ifdef __cplusplus
}
#endif
#endif /* KEYGNE_CONFIG_H_ */
