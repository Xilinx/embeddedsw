/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef AFI_H_
#define AFI_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FPD SLCR Base Address
 */
#define FPD_SLCR_BASEADDR                       ((u32)0xFD610000U)

/*
 * FPD SLCR Ctrl
 */
#define FPD_SLCR_CTRL							( ( FPD_SLCR_BASEADDR ) + ((u32)0x4U) )

/*
 * Register: AFI_FS
 */
#define FPD_SLCR_AFI_FS_REG                     ( ( FPD_SLCR_BASEADDR ) + ((u32)0X00005000U) )

/*
 * AFI FM0 Base Address
 */
#define AFI_FM0_BASEADDR                        ((u32)0xFD360000U)

/*
 * AFI FM1 Base Address
 */
#define AFI_FM1_BASEADDR                        ((u32)0xFD370000U)

/*
 * AFI FM2 Base Address
 */
#define AFI_FM2_BASEADDR                        ((u32)0xFD380000U)

/*
 * AFI FM3 Base Address
 */
#define AFI_FM3_BASEADDR                        ((u32)0xFD390000U)

/*
 * AFI FM4 Base Address
 */
#define AFI_FM4_BASEADDR                        ((u32)0xFD3A0000U)

/*
 * AFI FM5 Base Address
 */
#define AFI_FM5_BASEADDR                        ((u32)0xFD3B0000U)

/*
 * AFI FM6 Base Address
 */
#define AFI_FM6_BASEADDR                        ((u32)0xFF9B0000U)

#ifdef __cplusplus
}
#endif

#endif /* _AFI_H_ */
