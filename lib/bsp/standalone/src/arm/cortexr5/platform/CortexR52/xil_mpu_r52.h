/******************************************************************************
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xil_mpu.h
*
* @addtogroup r52_mpu_apis Cortex R5 Processor MPU specific APIs
*
* MPU functions provides access to MPU operations such as enable MPU, disable
* MPU and set attribute for section of memory.
* Boot code invokes Init_MPU function to configure the MPU. A total of 10 MPU
* regions are allocated with another 6 being free for users. Overview of the
* memory attributes for different MPU regions is as given below,
*
*|                       | Memory Range            | Attributes of MPURegion        |
*|-----------------------|-------------------------|--------------------------------|
*| DDR                   | 0x00000000 - 0x7FFFFFFF | Normal write-through Cacheable |
*| PL                    | 0x80000000 - 0x9FFFFFFF | Device Memory                  |
*| PCIE + FPD_AFI_FS     | 0xA0000000 - 0xA7FFFFFF | Device Memory                  |
*| OCM                   | 0xBBF00000 - 0xBBFFFFFF | Normal write-through Cacheable |
*| xSPI + Coresight      | 0xC0000000 - 0xE0FFFFFF | Device Memory                  |
*| RPU_R52_GIC           | 0xE2000000 - 0xE21FFFFF | Device memory                  |
*| CPM                   | 0xE4000000 - 0xE4FFFFFF | Device Memory                  |
*| FPD + LPD             | 0xEA000000 - 0xECFFFFFF | Device Memory                  |
*| PMC                   | 0xF0000000 - 0xF7FFFFFF | Device Memory                  |
*| FPD_CMN               | 0xF8000000 - 0xFBFFFFFF | Device Memory                  |
*
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 9.0   mus  04/19/23 Initial version
* 9.1   asa  03/04/24 Added a new structure "XMpuConfig_Init" that will be
*                     used to pass MPU initialization data.
* </pre>
*
*
******************************************************************************/
/**
 *@cond nocomments
 */

#ifndef XIL_MPU_H
#define XIL_MPU_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/***************************** Include Files *********************************/
#include "xil_types.h"
/***************** Macros (Inline Functions) Definitions *********************/
#define MPU_REG_DISABLED		0U
#define MPU_REG_ENABLED			1U
#define MAX_POSSIBLE_MPU_REGS	16U


#define XMPU_LIMIT_REG_ATTRIBUTE_SHIFT 7U
#define XMPU_LIMIT_REG_ATTRIBUTE_MASK 0xEU
#define XMPU_64BYTE_ALIGNMENT_MASK	0xFFFFFFC0U

#define XMPU_PRLAR_REG_ATTRIBUTE_MASK	0xEU
#define XMPU_PRLAR_REG_ENDADDR_MASK	0xFFFFFFC0U

#define XMPU_PRLAR_ATTRIBUTE_SHIFT 7U

#define XMPU_PBAR_REG_ATTRIBUTE_MASK	0x1FU
#define XMPU_PBAR_REG_BASEADDR_MASK	0xFFFFFFC0U

#define XMPU_REGION_CAN_MODIFY	0x1U
#define XMPU_VALID_REGION		0x2U

#define OCM_START_ADDR			0xBBF00000U
#define OCM_END_ADDR			0xBBFFFFFFU

/**************************** Type Definitions *******************************/
struct XMpuConfig{
	u32 RegionStatus; /* Enabled or disabled */
	UINTPTR BaseAddress;/* MPU region base address */
	u64 Size; /* MPU region size address */
	u32 Attribute; /* MPU region size attribute */
	u32 flags; /* MPU region flags */
};

struct XMpuConfig_Init{
	UINTPTR BaseAddress;/* MPU region base address */
	u64 Size; /* MPU region size address */
	u32 Attribute; /* MPU region size attribute */
};


typedef struct XMpuConfig XMpu_Config[MAX_POSSIBLE_MPU_REGS];
typedef struct XMpuConfig_Init XMpuConfig_Initial[MAX_POSSIBLE_MPU_REGS];

extern XMpu_Config Mpu_Config;
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/**
 *@endcond
 */
u32 Xil_SetTlbAttributes(UINTPTR Addr, u32 attrib);
void Xil_EnableMPU(void) __attribute__((__section__(".boot")));
void Xil_DisableMPU(void) __attribute__((__section__(".boot")));
void Xil_DisableMPURegions(void) __attribute__((__section__(".boot")));
u32 Xil_SetMPURegion(UINTPTR addr, u64 size, u32 attrib);
void Xil_GetMPUConfig (XMpu_Config mpuconfig);
u32 Xil_GetNumOfFreeRegions (void);
u32 Xil_GetNextMPURegion(void);
u32 Xil_DisableMPURegionByRegNum (u32 reg_num) __attribute__((__section__(".boot")));
u16 Xil_GetMPUFreeRegMask (void);
u32 Xil_SetMPURegionByRegNum (u32 reg_num, UINTPTR addr, u64 size, u32 attrib);
void* Xil_MemMap(UINTPTR PhysAddr, size_t size, u32 flags);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XIL_MPU_H */
/**
* @} End of "addtogroup r5_mpu_apis".
*/
