/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga.h
 * @addtogroup xilfpga_zynq_versal XilFPGA APIs for Versal Adaptive SoC and Zynq UltraScale+ MPSoC
 *
 * @{
 * @details
 *
 * <pre>
 * Xilfpga Error format:
 * Lower level Errors + Interface specific Errors + Xilfpga top layer Errors
 *----------------------------------------------------------------------------
 * Lower level Errors | Interface Specific Errors | Xilfpga top layer Errors
 * (other libarier    |     (PCAP Interface)      |
 * or drivers         |                           |
 * Used by xilfpga)   |                           |
 * ----------------------------------------------------------------------------
 * 31 - 16 bits       |     15 - 8 bits           |         7 - 0 bits
 * ----------------------------------------------------------------------------
 * Xilfpga Top Layer:
 *	The functionality exist in this layers is completely Interface agnostic.
 * It provides a unique interface to load the Bitstream  across multiple
 * platforms.(ie; ZynqMP)
 *
 * Interface Specific layer:
 * 	This layer is responsible for providing the interface specific related
 * errors.
 *	-->In Case of ZynqMp, it provides the errors related to PCAP Interface.
 *
 * Xilfpga lower layer:
 *	This layer is responsible for providing the Error related  to the lower
 * level drivers used by Interface layer.
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.2   Nava  06/08/16 Refactor the xilfpga library to support
 *                      different PL programming Interfaces.
 * 4.2   adk   07/11/18 Added support for readback of PL configuration data.
 * 4.2   Nava  08/16/18 Modified the PL data handling Logic to support
 *                      different PL programming interfaces.
 * 4.2   adk   08/28/18 Fixed misra-c required standard violations.
 * 4.2   Nava  09/15/18 Fixed global function call-backs issue.
 * 5.0   Nava  05/11/18 Added full bitstream loading support for versal Platform.
 * 5.0   Div   01/21/19 Fixed misra-c required standard violations.
 * 5.0   Nava  02/06/19 Remove redundant API's from the interface agnostic layer
 *                      and make the existing API's generic to support both
 *                      ZynqMP and versal platforms.
 * 5.0  Nava  02/26/19  Update the data handling logic to avoid the code
 *                      duplication
 * 5.0  sne   03/27/19  Fixed misra-c violations.
 * 5.0  Nava  03/29/19  Removed vesal platform related changes.As per the new
 *                      design, the Bitstream loading for versal platform is
 *                      done by PLM based on the CDO's data exists in the PDI
 *                      images. So there is no need of xilfpga API's for versal
 *                      platform to configure the PL.
 * 5.2  Nava  12/05/19  Added Versal platform support.
 * 5.2  Nava  02/14/20  Added Bitstream loading support by using IPI services
 *                      for ZynqMP platform.
 * 5.3  Nava  06/16/20  Modified the date format from dd/mm to mm/dd.
 * 5.3  Nava  09/09/20  Added a new error code (XFPGA_INVALID_PARAM)
 *                      for user API's input validation parameters.
 * 6.0  Nava  12/14/20  In XFpga_PL_BitStream_Load() API the argument
 *                      AddrPtr_Size is being used for multiple purposes.
 *                      Use of the same variable for multiple purposes can
 *                      make it more difficult for a person to read (or)
 *                      understand the code and also it leads to a safety
 *                      violation. fixes this  issue by adding a separate
 *                      function arguments to read KeyAddr and
 *                      Size(Bitstream size).
 * 6.0  Nava  05/17/21  Added platform specific ifdef checks to optimize the code.
 * 6.2  Nava  01/10/22  Removed duplicated legacy API's to reduce the xilfpga
 *                      memory footprint.
 * 6.2  Nava  01/10/22  Adds XFpga_GetVersion() and XFpga_GetFeatureList() API's
 *                      to provide the access to the xilfpga library to get the
 *                      xilfpga version and supported feature list info.
 * 6.2  Nava  03/11/22  Fixed an "implicit declaration of function" warning.
 * 6.3  Nava  08/05/22  Added doxygen tags.
 * 6.5  Nava  08/18/23  Resolved the doxygen issues.
 * 6.5  Nava  08/02/23  Updated version info macro to align with the library mld version.
 * 6.5  Nava  09/04/23  Added proper ifdef platform checks for user-accessible APIs.
 * 6.6  AC	  04/04/24  Resolved the doxygen issues.
 * </pre>
 *
 *
 ******************************************************************************/
#ifndef XILFPGA_H
#define XILFPGA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_printf.h"
#include "xil_util.h"
#include "xparameters.h"
#include "xfpga_config.h"
/**************************** Type Definitions *******************************/
/**
 * Structure to the XFpga instance.
 *
 */
typedef struct XFpgatag{
	u32 (*XFpga_ValidateBitstream)(struct XFpgatag *InstancePtr); /**< Validate the bitstream header before programming the PL */
	u32 (*XFpga_PreConfig)(struct XFpgatag *InstancePtr); /**<Prepare the FPGA to receive confuration data */
	u32 (*XFpga_WriteToPl)(struct XFpgatag *InstancePtr); /**< Write count bytes of configuration data to the FPGA */
	u32 (*XFpga_PostConfig)(struct XFpgatag *InstancePtr); /**< Set FPGA to operating state after writing is done */
	u32 (*XFpga_GetFeatureList)(struct XFpgatag *InstancePtr); /**< Gets the feature list that xilfpga library supports */
#ifndef versal
	u32 (*XFpga_GetInterfaceStatus)(void); /**< Provides the STATUS of PL programming interface */
	u32 (*XFpga_GetConfigReg)(const struct XFpgatag *InstancePtr); /**< Returns the value of the specified configuration register */
	u32 (*XFpga_GetConfigData)(const struct XFpgatag *InstancePtr); /**< Provides the FPGA readback data */
#ifndef XFPGA_SECURE_IPI_MODE_EN
	XFpga_Info	PLInfo; /**< Structure which is used to store the secure image data */
#endif
	XFpga_Read	ReadInfo; /**< Structure which is used to store the PL Image readback details */
#endif
	XFpga_Write	WriteInfo; /**< Structure which is used to store the PL Write Image details */
#ifdef XFPGA_GET_FEATURE_LIST
	u32 FeatureList;
#endif
}XFpga;
/************************** Variable Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/
/*
@cond internal

*/

#define XFPGA_SUCCESS			(0x0U)
#define XFPGA_FAILURE			(0x1U)
#define XFPGA_VALIDATE_ERROR		(0x2U)
#define XFPGA_PRE_CONFIG_ERROR		(0x3U)
#define XFPGA_WRITE_BITSTREAM_ERROR	(0x4U)
#define XFPGA_POST_CONFIG_ERROR		(0x5U)
#define XFPGA_OPS_NOT_IMPLEMENTED	(0x6U)
#define XFPGA_INVALID_PARAM		(0x8U)

#ifndef versal
#define XFPGA_FULLBIT_EN			(0x00000000U)
#define XFPGA_PARTIAL_EN			(0x00000001U)
#define XFPGA_AUTHENTICATION_DDR_EN		(0x00000002U)
#define XFPGA_AUTHENTICATION_OCM_EN		(0x00000004U)
#endif
#define XFPGA_ENCRYPTION_USERKEY_EN		(0x00000008U)
#ifndef versal
#define XFPGA_ENCRYPTION_DEVKEY_EN		(0x00000010U)
#define XFPGA_ONLY_BIN_EN			(0x00000020U)
#define XFPGA_READ_BACK_EN			(0x00000040U)
#define XFPGA_REG_READ_BACK_EN			(0x00000080U)

/* FPGA invalid interface status */
#define XFPGA_INVALID_INTERFACE_STATUS		(0xFFFFFFFFU)
#endif

/* XILFPGA Component version info */
#define XFPGA_MAJOR_VERSION		6U
#define XFPGA_MINOR_VERSION		5U

#ifndef versal
#define XFPGA_SECURE_FLAGS	(				\
				XFPGA_AUTHENTICATION_DDR_EN	\
				| XFPGA_AUTHENTICATION_OCM_EN	\
				| XFPGA_ENCRYPTION_USERKEY_EN	\
				| XFPGA_ENCRYPTION_DEVKEY_EN	\
				)

#define XFPGA_AUTH_ENC_USERKEY_DDR	(				\
					XFPGA_AUTHENTICATION_DDR_EN	\
					| XFPGA_ENCRYPTION_USERKEY_EN	\
					)

#define XFPGA_AUTH_ENC_DEVKEY_DDR	(				\
					XFPGA_AUTHENTICATION_DDR_EN	\
					| XFPGA_ENCRYPTION_DEVKEY_EN	\
					)

#define XFPGA_AUTH_ENC_USERKEY_OCM	(				\
					XFPGA_AUTHENTICATION_OCM_EN	\
					| XFPGA_ENCRYPTION_USERKEY_EN	\
					)

#define XFPGA_AUTH_ENC_DEVKEY_OCM	(				\
					XFPGA_AUTHENTICATION_OCM_EN	\
					| XFPGA_ENCRYPTION_DEVKEY_EN	\
					)
/* FPGA Configuration Registers Offsets */
#define CRC             0U  /* Status Register */
#define FAR1            1U  /* Frame Address Register */
#define FDRI            2U  /* FDRI Register */
#define FDRO            3U  /* FDRO Register */
#define CMD             4U  /* Command Register */
#define CTL0            5U  /* Control Register 0 */
#define MASK            6U  /* MASK Register */
#define STAT            7U  /* Status Register */
#define LOUT            8U  /* LOUT Register */
#define COR0            9U  /* Configuration Options Register 0 */
#define MFWR            10U /* MFWR Register */
#define CBC             11U /* CBC Register */
#define IDCODE          12U /* IDCODE Register */
#define AXSS            13U /* AXSS Register */
#define COR1            14U /* Configuration Options Register 1 */
#define WBSTAR          16U /* Warm Boot Start Address Register */
#define TIMER           17U /* Watchdog Timer Register */
#define BOOTSTS         22U /* Boot History Status Register */
#define CTL1            24U /* Control Register 1 */
#else
#define XFPGA_PDI_LOAD			(0x00000000U)
#define XFPGA_DELAYED_PDI_LOAD		(0x00000001U)
#endif

#define Xfpga_Printf(DebugType, ...) \
	if ((DebugType) != 0U) \
	{xil_printf (__VA_ARGS__); }

#define XFPGA_ERR_MASK				(0xFFU)
#define XFPGA_ERR_INTERFACE_MASK	(0xFFFFFF00U)
#define XFPGA_UPDATE_ERR(XfpgaErr, InterfaceErr) \
		(((InterfaceErr)&XFPGA_ERR_INTERFACE_MASK) + \
		 ((XfpgaErr)&XFPGA_ERR_MASK))

/** @endcond*/
/************************** Function Prototypes ******************************/


#ifdef XFPGA_GET_FEATURE_LIST
u32 XFpga_GetFeatureList(XFpga *InstancePtr, u32 *FeatureList);
#endif

#ifdef XFPGA_GET_VERSION_INFO
u32 XFpga_GetVersion(u32 *Version);
#endif

u32 XFpga_Initialize(XFpga *InstancePtr); /* This API, when called, initializes the XFPGA interface with default settings.*/
u32 XFpga_ValidateImage(XFpga *InstancePtr,
			UINTPTR BitstreamImageAddr,
			UINTPTR KeyAddr, u32 Size, u32 Flags);
u32 XFpga_BitStream_Load(XFpga *InstancePtr,
			 UINTPTR BitstreamImageAddr,
			 UINTPTR KeyAddr, u32 Size, u32 Flags);
u32 XFpga_PL_Preconfig(XFpga *InstancePtr);
u32 XFpga_Write_Pl(XFpga *InstancePtr,UINTPTR BitstreamImageAddr,
		   UINTPTR KeyAddr, u32 Size, u32 Flags);
u32 XFpga_PL_PostConfig(XFpga *InstancePtr);
#ifndef versal
u32 XFpga_GetPlConfigData(XFpga *InstancePtr, UINTPTR ReadbackAddr,
			  u32 NumFrames);
u32 XFpga_GetPlConfigReg(XFpga *InstancePtr, UINTPTR ReadbackAddr,
			 u32 ConfigRegAddr);
u32 XFpga_InterfaceStatus(XFpga *InstancePtr);
#pragma message ("From 2023.1 release onwards the XilFPGA BSP user configuration  flags ‘reg_readback_en’ and  ‘data_readback_en’ will be disabled by default but users can still be able to enable these flags as needed")
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_H */
/** @} */
