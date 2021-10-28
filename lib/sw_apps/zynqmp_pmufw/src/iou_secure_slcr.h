/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef IOU_SECURE_SLCR_H_
#define IOU_SECURE_SLCR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IOU_SECURE_SLCR Base Address
 */
#define IOU_SECURE_SLCR_BASEADDR	((u32)0xFF240000U)

/**
 * Register: IOU_AXI_WPRTCN
 */
#define IOU_AXI_WPRTCN		( ( IOU_SLCR_BASEADDR ) + ((u32)0X00000000U) )

#define GEM0_AXI_AWPROT_SHIFT		0U
#define GEM0_AXI_AWPROT_MASK		((u32)0x00000007U)
#define GEM1_AXI_AWPROT_SHIFT		3U
#define GEM1_AXI_AWPROT_MASK		((u32)0x00000038U)
#define GEM2_AXI_AWPROT_SHIFT		6U
#define GEM2_AXI_AWPROT_MASK		((u32)0x000001C0U)
#define GEM3_AXI_AWPROT_SHIFT		9U
#define GEM3_AXI_AWPROT_MASK		((u32)0x00000E00U)
#define SD0_AXI_AWPROT_SHIFT		16U
#define SD0_AXI_AWPROT_MASK		((u32)0x00070000U)
#define SD1_AXI_AWPROT_SHIFT		19U
#define SD1_AXI_AWPROT_MASK		((u32)0x00380000U)

/**
 * Register: IOU_AXI_RPRTCN
 */
#define IOU_AXI_RPRTCN		( ( IOU_SLCR_BASEADDR ) + ((u32)0X00000004U) )

#define GEM0_AXI_ARPROT_SHIFT		0U
#define GEM0_AXI_ARPROT_MASK		((u32)0x00000007U)
#define GEM1_AXI_ARPROT_SHIFT		3U
#define GEM1_AXI_ARPROT_MASK		((u32)0x00000038U)
#define GEM2_AXI_ARPROT_SHIFT		6U
#define GEM2_AXI_ARPROT_MASK		((u32)0x000001C0U)
#define GEM3_AXI_ARPROT_SHIFT		9U
#define GEM3_AXI_ARPROT_MASK		((u32)0x00000E00U)
#define SD0_AXI_ARPROT_SHIFT		16U
#define SD0_AXI_ARPROT_MASK		((u32)0x00070000U)
#define SD1_AXI_ARPROT_SHIFT		19U
#define SD1_AXI_ARPROT_MASK		((u32)0x00380000U)

#ifdef __cplusplus
}
#endif


#endif /* _IOU_SECURE_SLCR_H_ */
