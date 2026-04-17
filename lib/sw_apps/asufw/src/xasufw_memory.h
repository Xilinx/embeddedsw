/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memory.h
 *
 * This file contains defines related to ASUFW internal memory.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   07/23/23 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       rmv  09/12/25 Added shared memory related macros
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_MEMORY_H_
#define XASUFW_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/

/**
 * ASUFW Memory Address Map (0xEBE00000 - 0xEBE5FFFF):
 *
 * 0xEBE00000 - 0xEBE3FFFF : Instruction RAM		(256KB)
 * 0xEBE40000 - 0xEBE40FFF : RTCA Configuration Memory	(4KB)
 * 0xEBE41000 - 0xEBE48FFF : IPI Shared Memory		(32KB)
 * 0xEBE49000 - 0xEBE5BBFF : Data RAM			(75KB)
 * 0xEBE5BC00 - 0xEBE5DBFF : OCP Memory			(8KB)
 * 0xEBE5DC00 - 0xEBE5EBFF : Debug Log Buffer		(4KB)
 * 0xEBE5EC00 - 0xEBE5FFFF : RSA Data Buffer		(5KB)
 */

/*  ASUFW Data Ram Start and end address related defines */
#define XASUFW_RAM_START_ADDR		(0xEBE00000U)	/**< Start address of ASUFW RAM */

#define XASUFW_RAM_END_ADDR		(0xEBE5FFFFU)	/**< End address of ASUFW RAM */

/*  ASUFW Run Time Configuration Area related register defines */
#define	XASUFW_RTCA_BASEADDR			(0xEBE40000U) /**< ASUFW RTCA Base Address */
#define XASUFW_RTCA_ACCESS_PERM_ADDR	(XASUFW_RTCA_BASEADDR + 0xC00U)
							/**< ASUFW RTCA commands access permissions data address */
#define XASUFW_RTCA_IDENTIFICATION_ADDR		(XASUFW_RTCA_BASEADDR + 0x0U)
							/**< RTCA identification address */
#define XASUFW_RTCA_VERSION_ADDR		(XASUFW_RTCA_BASEADDR + 0x4U)
							/**< RTCA version address */
#define XASUFW_RTCA_SIZE_ADDR			(XASUFW_RTCA_BASEADDR + 0x8U)
							/**< RTCA size address */
#define XASUFW_RTCA_COMM_CHANNEL_INFO_ADDR	(XASUFW_RTCA_BASEADDR + 0x10U)
							/**< RTCA channel information address */

/* Default values of ASUFW Run Time Configuration Area registers */
#define XASUFW_RTCA_IDENTIFICATION_STRING	(0x41435452U)
							/**< RTCA identification string */
#define XASUFW_RTCA_VERSION			(0x1U)
							/**< RTCA version */
#define XASUFW_RTCA_SIZE			(0x1000U)
							/**< RTCA size */
/* Shared memory related macros */
#define XASUFW_SHARED_MEMORY_ADDRESS		(0xEBE41000U)	/**< Reserved address in ASU DATA
								RAM for channel buffers shared
								memory */
#define XASUFW_SHARED_MEMORY_SIZE		(0x8000U)	/**< 32KB size for shared memory */

#define XASUFW_DEBUG_LOG_BUFFER_SIZE	(0x1000U)	/**< Size of debug log buffer in bytes */

#define XASUFW_DEBUG_LOG_BUFFER_ADDR	(0xEBE5DC00U)	/**< Address of debug log buffer in
								data RAM */

/* PLM RunTime Configuration Area Base Address */
#define XASUFW_PLMI_RTCFG_BASEADDR	(0xF2014000U) /**< PLM runtime configuration base address */
#define XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR	(XASUFW_PLMI_RTCFG_BASEADDR + 0x384U)
						/**< Address of efuse 0 IV of size 12 bytes */

/* DDR Reserved Area Configuration. */
#define XASUFW_RTCFG_RSVD_DDR_ADDR	(XASUFW_PLMI_RTCFG_BASEADDR + 0x2A8U)
					/**< Reserved DDR address register */
#define XASUFW_RTCFG_RSVD_DDR_SIZE	(XASUFW_PLMI_RTCFG_BASEADDR + 0x2ACU)
					/**< Reserved DDR size register. */
#define XASUFW_2GB_END_ADDR		(0x7FFFFFFFU)	/**< 2GB end address */

#define XASUFW_DDR_RSVD_SIZE		(1024U)	/**< Size of DDR reserved area for ASUFW in bytes. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MEMORY_H_ */
/** @} */
