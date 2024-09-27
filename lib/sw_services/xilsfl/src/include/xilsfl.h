/******************************************************************************
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl.h
 * @addtogroup xilsfl overview
 * @{
 * @details
 *
 * This file should be included in the example files and compiled along with
 * the examples (*.c).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Add XSfl_FlashRead API and callback for non blocking transfer
 *                    in XSfl_CntrlInfo.
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XILSFL_H
#define XILSFL_H

/***************************** Include Files *********************************/
#include "xparameters.h"	/* SDK generated parameters */
#ifdef XPAR_XOSPIPSV_NUM_INSTANCES
#include "xospipsv.h"		/* OSPIPSV device driver */
#endif
#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/*
 * Define DEBUG macro to enable debug prints
 */
#undef XSFL_DEBUG

/*
 * following macro contains the number of interfaces drivers
 */
#define XSFL_NUM_INSTANCES    1

#define XSFL_RXADDR_OVER_32BIT	0x100000000U /**< Rx address over 32 bit */

/*
 * SDR max supported frequency in Mz
 */
#define XSFL_SDR_NON_PHY_MAX_FREQ    50000000

/*
 * The following constants define the commands which may be sent to the Flash
 * device.
 */
#define XSFL_WRITE_STATUS_CMD        0x01
#define XSFL_WRITE_DISABLE_CMD       0x04
#define XSFL_WRITE_ENABLE_CMD        0x06
#define XSFL_BULK_ERASE_CMD          0xC7
#define XSFL_DIE_ERASE_CMD           0xC4
#define XSFL_READ_ID                 0x9F
#define XSFL_READ_CONFIG_CMD         0x35
#define XSFL_WRITE_CONFIG_CMD        0x01
#define XSFL_READ_STATUS_CMD         0x05
#define XSFL_READ_FLAG_STATUS_CMD    0x70
#define XSFL_READ_CMD_4B             0x13
#define XSFL_WRITE_CMD_4B            0x12
#define XSFL_SEC_ERASE_CMD_4B        0xDC
#define XSFL_READ_CMD_OCTAL_4B       0x7C
#define XSFL_READ_CMD_OCTAL_IO_4B    0xCC
#define XSFL_READ_CMD_OCTAL_DDR      0x9D
#define XSFL_WRITE_CMD_OCTAL_4B      0x84
#define XSFL_ENTER_4B_ADDR_MODE      0xB7
#define XSFL_EXIT_4B_ADDR_MODE       0xE9
#define XSFL_WRITE_CONFIG_REG        0x81
#define XSFL_READ_CONFIG_REG         0x85
#define XSFL_DUAL_READ_CMD           0x3B
#define XSFL_QUAD_READ_CMD           0x6B
#define XSFL_DUAL_READ_CMD_4B        0x3C
#define XSFL_QUAD_READ_CMD_4B        0x6C
#define XSFL_QUAD_WRITE_CMD_4B       0x34

/*
 * Sixteen MB
 */
#define XSFL_SIXTEENMB 0x1000000

#define XSFL_FLASH_PAGE_SIZE_256           256
#define XSFL_FLASH_SECTOR_SIZE_4KB         0x1000
#define XSFL_FLASH_SECTOR_SIZE_256KB       0x40000
#define XSFL_FLASH_SECTOR_SIZE_64KB        0x10000
#define XSFL_FLASH_SECTOR_SIZE_128KB       0x20000
#define XSFL_FLASH_DEVICE_SIZE_256M        0x2000000
#define XSFL_FLASH_DEVICE_SIZE_512M        0x4000000
#define XSFL_FLASH_DEVICE_SIZE_1G          0x8000000
#define XSFL_FLASH_DEVICE_SIZE_2G          0x10000000

#define XSFL_MICRON_OCTAL_ID_BYTE0          0x2c

#define XSFL_MICRON_BP_BITS_MASK		0x7C
#define XSFL_CONFIG_REG2_VOLATILE_ADDR_MX 	0x00000300

/**< Controller Type */
#define XSFL_OSPI_CNTRL 0x01

/* Flash Device Type */
#define XSFL_QSPI_FLASH  0x01
#define XSFL_OSPI_FLASH  0x02

/**
 * @name Dual Byte opcode selection
 * @{
 */
/**
 * Macros to enable/disable Dual Byte opcode.
 */
#define XSFL_DUAL_BYTE_OP_DISABLE  0x00
#define XSFL_DUAL_BYTE_OP_INVERT   0x01
#define XSFL_DUAL_BYTE_OP_SAME     0x02
/** @} */

/**
 * @name Edge mode selection
 * @{
 */
/**
 * Macros to select different edge modes like
 * SDR+NON-PHY, SDR+PHY and DDR+PHY.
 */
#define XSFL_EDGE_MODE_SDR_PHY        0x0U
#define XSFL_EDGE_MODE_SDR_NON_PHY    0x1U
#define XSFL_EDGE_MODE_DDR_PHY        0x2U
/** @} */

/**
 * @name Chip Select selection
 * @{
 */
/**
 * Macros to select CS0 and CS1.
 */
#define XSFL_SELECT_FLASH_CS0	0
#define XSFL_SELECT_FLASH_CS1	1
/** @} */

/**
 * @name Connection mode selection
 * @{
 */
/**
 * Macros to select SINGLE, STACKED and PARALLEL connection modes.
 */
#define XSFL_CONNECTION_MODE_SINGLE      0x0U
#define XSFL_CONNECTION_MODE_STACKED     0x1U
/** @} */

/**
 * @name Prototype(Bus width: Cmd_Addr_Data) selection
 * @{
 */
/**
 * Macros to select Read and Write prototype.
 */
#define XSFL_FLASH_PROTO_1_1_1    0U
#define XSFL_FLASH_PROTO_1_0_1    1U
#define XSFL_FLASH_PROTO_1_1_2    2U
#define XSFL_FLASH_PROTO_1_1_8    3U
#define XSFL_FLASH_PROTO_1_8_8    4U
#define XSFL_FLASH_PROTO_8_8_8    5U
#define XSFL_FLASH_PROTO_8_0_0    6U
#define XSFL_FLASH_PROTO_8_8_0    7U
#define XSFL_FLASH_PROTO_8_0_8    8U
#define XSFL_FLASH_PROTO_1_1_4    9U
#define XSFL_FLASH_PROTO_4_4_4    10U

/*
 * following macros contains flash info
 */
#define XSFL_FLASH_ID      0U /* Flash Device jdec_id */
#define XSFL_DEVICE_SIZE   1U /* Individual device size or combined combined size
				 in case of Stacked/Parrellel config */
#define XSFL_SECT_SIZE     2U /* Individual sector size or combined sector size
                                 in case of Parrellel config */
#define XSFL_PAGE_SIZE     3U /* Individual sector size or combined sector size
                                 in case of Parrellel config */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for a flash message.
 */
typedef struct {
	u64 RxAddr64bit;  /**< Provide 64-bit read address for 32-bit platform */
	u32 ByteCount;    /**< Number of bytes to read or write */
	u32 Addr;         /**< Device Address */

	u8 *TxBfrPtr;     /**< Write buffer pointer */
	u8 *RxBfrPtr;     /**< Read buffer pointer */
	u8 Opcode;        /**< Opcode/Command */
	u8 Dummy;         /**< Number of dummy cycles for opcode */
	u8 Xfer64bit;     /**< Set to 1 when reading from 64-bit addr otherwise 0 */

	u8 DualByteOpCode; /**< Extended opcode in dual-byte opcode mode */
	u8 Addrsize;     /**< Size of address in bytes */
	u8 Addrvalid;    /**< 1 if Address is required for opcode, 0 otherwise */
	u8 Proto;        /**< Indicate number of Cmd-Addr-Data lines */
} XSfl_Msg;

/**
 * This typedef contains configuration information for the flash device.
 */
typedef struct {
	u32 FlashIndex;   /**< Index in the flash_info structure */
	u32 DeviceSize;   /* This is the size of one flash device
			   * or combination of both devices, in case of stocked/parallel
			   * connection.
			   */
	u32 SectSize;     /* Individual sector size or
			   * combined sector size in case of parallel config
			   */
	u32 SectCount;    /* Individual sector count or
			   * combined sector count in case of stocked config
			   */
	u32 PageSize;     /* Individual page size or
			   * combined page size in case of parallel config
			   */
	u32 PageCount;    /* Individual page count or
			   * combined page count in case of stocked config
			   */
	u8 ConnectionMode; /**< Type of connection (single/stacked/parallel) */
	u8 FlashMake;      /**< vendor info */
} XSfl_FlashInfo;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u32 *DeviceIdData;	/**< Contains Device Id Data information
				 * - Is used for configuring SDR/DDR mode */
	u32 OpMode;             /**< Operating Mode DAC or INDAC */
	u32 RefClockHz;         /**< Input clock frequency */
	u8 DeviceId; 		/**< ID  of controller device */
	u8 ChipSelectNum;	/**< Chip select information */
	u8 SdrDdrMode;		/**< Edge mode can be SDR or DDR */
	u8 CntrlType;		/**< Type of the interface controller */
	u32 (*RxTunning)();	/**< Callback reference for Rx tuning */

	u32 (*SelectFlash)(u8 ChipSelNum);  /**< Callback reference for chip select */
	u32 (*Transfer)(u8 Index, XSfl_Msg *SflMsg); /**< Callback reference for interface
						      * transfer function */
	u32 (*NonBlockingTransfer)(u8 Index, XSfl_Msg *SflMsg); /**< Callback reference for interface
								 * non blocking transfer function */
	u32 (*SetSdrDdr)(u8 Mode, u8 DualByteOpCode);	/**< Callback reference for controller configs */
	u32 (*TransferDone)(u8 Index);		/**< Callback reference for flash read status */
	u32 (*DeviceReset)(u8 Type);  /**< Callback reference for Device reset */

} XSfl_CntrlInfo;

/**
 * This typedef contains interface information for a sfl library.
 */
typedef struct {
	XSfl_CntrlInfo CntrlInfo;		/**< Controller specific information */
	XSfl_FlashInfo SflFlashInfo;	/**< Flash Specific information */

} XSfl_Interface;

/**
 * This typedef contains information of the sfl library.
 */
typedef struct {
	int Index;
	XSfl_Interface Instance[XSFL_NUM_INSTANCES];
} XSfl;

/**
 * This typedef contains configuration information for a flash/controller device.
 * The user is required to allocate a variable of this type. A variable of this type
 * is then passed to the library API functions.
 */
typedef union XSfl_UserConfig{
	struct Ospi_Conig{
		u32 BaseAddress;	/**< Base address of the device */
		u8 ReadMode;		/**< Operating Mode DAC or INDAC */
		u8 ChipSelect;		/**< Chip select information */
	} Ospi_Config;
} XSfl_UserConfig;

/************************** Function Prototypes ******************************/
u32 XSfl_FlashInit(u8 *SflHandler, XSfl_UserConfig SflUserOptions, u8 ControllerInfo);
u32 XSfl_FlashErase(u8 SflHandler, u32 Address, u32 ByteCount);
u32 XSfl_FlashWrite(u8 SflHandler, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr);
u32 XSfl_FlashReadStart(u8 SflHandler, u32 Address, u32 ByteCount,
		u8 *ReadBfrPtr, u64 RxAddr64bit);
u32 XSfl_FlashReadDone(u8 SflHandler);
u32 XSfl_FlashGetInfo(u8 SflHandler, u8 Option, u32 *DataPtr);
u32 XSfl_FlashRead(u8 SflHandler, u32 Address, u32 ByteCount,
                u8 *ReadBfrPtr, u64 RxAddr64bit);

/************************** Variable Definitions *****************************/
#endif /* XILSFL_H */
