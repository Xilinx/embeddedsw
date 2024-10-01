/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_qspi.h
 *
 * This is the header file which contains qspi declarations.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

#ifndef XPLM_QSPI_H
#define XPLM_QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_status.h"
#include "xplm_load.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define XPLM_READ_ID_CMD			(0x9FU)
#define XPLM_FAST_READ_CMD_24BIT		(0x0BU)
#define	XPLM_DUAL_READ_CMD_24BIT		(0x3BU)
#define XPLM_QUAD_READ_CMD_24BIT		(0x6BU)
#define XPLM_FAST_READ_CMD_32BIT		(0x0CU)
#define XPLM_DUAL_READ_CMD_32BIT		(0x3CU)
#define XPLM_QUAD_READ_CMD_32BIT		(0x6CU)
#define XPLM_QUAD_READ_CMD_24BIT2		(0xEBU)

#define XPLM_WRITE_ENABLE_CMD	(0x06U)
#define XPLM_BANK_REG_RD_CMD		(0x16U)
#define XPLM_BANK_REG_WR_CMD		(0x17U)
/* Bank register is called Extended Addr Reg in Micron */
#define XPLM_EXTADD_REG_RD_CMD	(0xC8U)
#define XPLM_EXTADD_REG_WR_CMD	(0xC5U)

#define XPLM_DUMMY_CLOCKS	(8U) /* Number of dummy bytes for fast, dual and
				     quad reads */

/*
 * Max limit of single DMA transfer is 512MB
 */
#define XPLM_DMA_DATA_TRAN_SIZE	(0x20000000U)

/*
 * Macros related to Qspi Bank Size
 */
#define XPLM_FLASH_SIZE_16MB			(0x1000000U)
#define XPLM_BANKSIZE			XPLM_FLASH_SIZE_16MB
#define XPLM_FLASH_SIZE_64MB			(0x4000000U)
#define XPLM_WINBOND_BANKSIZE		XPLM_FLASH_SIZE_64MB
/*
 * Bank mask
 */
#define XPLM_BANKMASK		(~(XPLM_BANKSIZE - 1U))
#define XPLM_WINBOND_BANKMASK		(~(XPLM_WINBOND_BANKSIZE - 1U))

/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0xBB or 0xBA
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 * Spansion:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is Device ID - Memory Interface type - 0x20 or 0x02
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 */
#define XPLM_MICRON_ID		(0x20U)
#define XPLM_MICRON_QEMU_ID		(0x2CU) // TODO Fix once qemu is fixed
#define XPLM_SPANSION_ID		(0x01U)
#define XPLM_WINBOND_ID		(0xEFU)
#define XPLM_MACRONIX_ID		(0xC2U)
#define XPLM_ISSI_ID			(0x9DU)

/* Macronix size constants for 1.8V and 3.3 parts. */
#define XPLM_MACRONIX_FLASH_SIZE_ID_512M		(0x1AU)
#define XPLM_MACRONIX_FLASH_SIZE_ID_1G		(0x1BU)
#define XPLM_MACRONIX_FLASH_SIZE_ID_2G		(0x1CU)
#define XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_128M       (0x38U)
#define XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_512M       (0x3AU)
#define XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_1G         (0x3BU)
#define XPLM_MACRONIX_FLASH_1_8_V_SIZE_ID_2G		(0x3CU)

#define XPLM_READ_ID_CMD_TX_BYTE_CNT		(1U)
#define XPLM_READ_ID_CMD_RX_BYTE_CNT		(4U)
#define XPLM_QSPI24_COPY_DISCARD_BYTE_CNT	(4U)
#define XPLM_QSPI32_COPY_DISCARD_BYTE_CNT	(5U)
#define XPLM_QSPI_WRITE_ENABLE_CMD_BYTE_CNT	(1U)
#define XPLM_EXTADD_REG_WR_CMD_BYTE_CNT	(2U)
#define XPLM_EXTADD_REG_RD_CMD_BYTE_CNT	(1U)
#define XPLM_BANK_REG_WR_CMD_BYTE_CNT	(2U)
#define XPLM_BANK_REG_RD_CMD_BYTE_CNT	(1U)


#define XOSPIPSV_READ_1_1_2	1U
#define XOSPIPSV_READ_1_1_4	2U
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_QspiInit(XPlm_BootModes Mode);
u32 XPlm_QspiCopy(u64 SrcAddr, u32 DestAddr, u32 Length, u32 Flags);

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPLM_QSPI_H */
