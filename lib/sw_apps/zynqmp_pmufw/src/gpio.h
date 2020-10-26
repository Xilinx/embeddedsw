/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef GPIO_H_
#define GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * APU Base Address
 */
#define GPIO_BASEADDR      (0XFF0A0000U)

/**
 * Register: GPIO_MASK_DATA_5_MSW_REG
 */
#define GPIO_MASK_DATA_5_MSW_REG    ( GPIO_BASEADDR  + 0X0000002CU )

/**
 * Register: GPIO_DATA_5_RO_REG
 */
#define GPIO_DATA_5_RO_REG    ( GPIO_BASEADDR  + 0X00000074U )

/**
 * Register: GPIO_DIRM_5
 */
#define GPIO_DIRM_5    ( GPIO_BASEADDR + 0X00000344U )

#define MAX_REG_BITS               32

/*
 * GPIO5 EMIO[95:92] are the PS-PL reset lines
 */
#define GPIO5_EMIO92_MSW_DATA_BIT    12
#define GPIO5_EMIO93_MSW_DATA_BIT    13
#define GPIO5_EMIO94_MSW_DATA_BIT    14
#define GPIO5_EMIO95_MSW_DATA_BIT    15

#define GPIO_PIN_MASK_BITS         0xFFFF0000U

#ifdef __cplusplus
}
#endif


#endif /* _GPIO_H_ */
