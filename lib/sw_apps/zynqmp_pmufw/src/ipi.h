/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef IPI_H_
#define IPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IPI Base Address
 */
#define IPI_BASEADDR      ((u32)0XFF300000U)

#define IPI_PMU_0_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00030010U) )
#define IPI_PMU_1_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00031010U) )
#define IPI_PMU_2_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00032010U) )
#define IPI_PMU_3_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00033010U) )

#define IPI_CTRL				( ( IPI_BASEADDR ) + ((u32)0x00080000U) )

#ifdef __cplusplus
}
#endif


#endif /* _IPI_H_ */
