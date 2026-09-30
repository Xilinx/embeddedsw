/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XILSFL_H
#define XILSFL_H

/*****************************************************************************/
/**
 * @file xilsfl.h
 * @addtogroup xilsfl overview
 * @{
 * @brief Defines the interface of the serial flash library (SFL).
 *
 * @details
 *
 * Include this file in the example files and compile it along with the
 * examples (*.c).
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.0   sb  8/20/24  Initial release
 * 1.0   sb  9/25/24  Add XSfl_FlashRead API and callback for non blocking transfer
 *                    in XSfl_CntrlInfo.
 * 1.1   sb  01/28/25  Add support to read in stig when DMA is not available.
 * 1.1   sb  02/11/25  Add support for x2/x4 operation.
 * 1.3   zm  03/09/26  Add support for W25Q128JW Winbond flash.
 *       zm  07/17/26  Add support for Macronix OSPI flashes mx66um2g45g and mx66uw2g345gxrix0.
 *       zm  08/03/26  Add support for GD25LR512MF GigaDevice flash.
 *       zm  08/03/26  Add support for MX25U6432F Macronix flash.
 * 1.3   sb  07/20/26  Add support for Advanced Sector Protection (ASP).
 * 1.4   vn  08/26/26  Fix Doxygen warnings.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"	/* SDK generated parameters */
#include "xilsfl_config.h"
#ifdef XPAR_XOSPIPSV_NUM_INSTANCES
#include "xospipsv.h"		/* OSPIPSV device driver */
#endif
#include "xil_printf.h"
#include "xil_cache.h"
#include "xstatus.h"
#if CONFIG_ADVANCED_SECTOR_PROTECTION
#include "xilsfl_asp.h"
#endif

/************************** Constant Definitions *****************************/

/*
 * Define DEBUG macro to enable debug prints
 */
#undef XSFL_DEBUG

#define XSFL_NUM_INSTANCES    1 /**< Number of interface drivers. */

/** Rx address above the 32-bit range. */
#define XSFL_RXADDR_OVER_32BIT	0x100000000U

/** SDR maximum supported frequency, in Hz. */
#define XSFL_SDR_NON_PHY_MAX_FREQ    50000000

/**
 * @name Flash commands
 *
 * Commands that the library can send to the flash device.
 * @{
 */
#define XSFL_WRITE_STATUS_CMD        0x01 /**< Write status command. */
#define XSFL_WRITE_DISABLE_CMD       0x04 /**< Write disable command. */
#define XSFL_WRITE_ENABLE_CMD        0x06 /**< Write enable command. */
#define XSFL_BULK_ERASE_CMD          0xC7 /**< Bulk erase command. */
#define XSFL_DIE_ERASE_CMD           0xC4 /**< Die erase command. */
#define XSFL_READ_ID                 0x9F /**< Read ID command. */
#define XSFL_READ_CONFIG_CMD         0x35 /**< Read configuration command. */
#define XSFL_WRITE_CONFIG_CMD        0x01 /**< Write configuration command. */
#define XSFL_READ_STATUS_CMD         0x05 /**< Read status command. */
#define XSFL_READ_FLAG_STATUS_CMD    0x70 /**< Read flag status command. */
#define XSFL_READ_CMD_4B             0x13 /**< 4-byte address read command. */
/** 4-byte address single-I/O fast read command. */
#define XSFL_FAST_READ_CMD_4B        0x0C
#define XSFL_WRITE_CMD_4B            0x12 /**< 4-byte address write command. */
/** 4-byte address sector erase command. */
#define XSFL_SEC_ERASE_CMD_4B        0xDC
/** 4-byte address octal read command. */
#define XSFL_READ_CMD_OCTAL_4B       0x7C
/** 4-byte address octal I/O read command. */
#define XSFL_READ_CMD_OCTAL_IO_4B    0xCC
#define XSFL_READ_CMD_OCTAL_DDR      0x9D /**< Octal DDR read command. */
/** 4-byte address octal write command. */
#define XSFL_WRITE_CMD_OCTAL_4B      0x84
/** Enter 4-byte address mode command. */
#define XSFL_ENTER_4B_ADDR_MODE      0xB7
/** Exit 4-byte address mode command. */
#define XSFL_EXIT_4B_ADDR_MODE       0xE9
/** Write configuration register command. */
#define XSFL_WRITE_CONFIG_REG        0x81
/** Read configuration register command. */
#define XSFL_READ_CONFIG_REG         0x85
#define XSFL_DUAL_READ_CMD           0x3B /**< Dual read command. */
#define XSFL_QUAD_READ_CMD           0x6B /**< Quad read command. */
/** 4-byte address dual read command. */
#define XSFL_DUAL_READ_CMD_4B        0x3C
/** 4-byte address quad read command. */
#define XSFL_QUAD_READ_CMD_4B        0x6C
#define XSFL_QUAD_WRITE_CMD          0x32 /**< Quad write command. */
#define XSFL_DUAL_WRITE_CMD          0xA2 /**< Dual write command. */
#define XSFL_QUAD_SEC_ERASE_CMD      0xD8 /**< Quad sector erase command. */
#define XSFL_WRITE_CMD               0x02 /**< Write command. */
#define XSFL_READ_CMD                0x03 /**< Read command. */
/**
 * @def XSFL_NO_CMD
 * @brief No-command or dummy command value (0x00).
 */
#define XSFL_NO_CMD                  0x00
/** @} */

/* Commands for Macronix Octal (OPI) Flash */
/**
 * @def XSFL_MX_WRITE_CONFIG_REG2_CMD
 * @brief Macronix Write Configuration Register 2 (WRCR2) command.
 */
#define XSFL_MX_WRITE_CONFIG_REG2_CMD   0x72

/**
 * @def XSFL_MX_READ_CMD_OCTAL_DDR
 * @brief Macronix Octal DDR read (8DTRD) command.
 */
#define XSFL_MX_READ_CMD_OCTAL_DDR      0xEE

/**
 * @def XSFL_MX_SEC_ERASE_CMD_4B
 * @brief Macronix 4-byte 4KB sector erase (SE4B) command.
 */
#define XSFL_MX_SEC_ERASE_CMD_4B        0x21

/* Commands for Macronix Quad SPI Flash */
/**
 * @def XSFL_MX_READ_CONFIG_REG
 * @brief Macronix Read Configuration Register (RDCR) command.
 */
#define XSFL_MX_READ_CONFIG_REG         0x15

/* Commands for Winbond Flash */
/**
 * @def XSFL_WB_READ_STATUS_REG2_CMD
 * @brief Command for reading the status register 2.
 *
 * This command retrieves the status of the second status register.
 * Winbond Read SR2 (0x35) coincides with XSFL_READ_CONFIG_CMD.
 */
#define XSFL_WB_READ_STATUS_REG2_CMD    XSFL_READ_CONFIG_CMD

/**
 * @def XSFL_WB_WRITE_STATUS_REG2_CMD
 * @brief Write Status Register 2 command.
 *
 * This command writes status register 2.
 */
#define XSFL_WB_WRITE_STATUS_REG2_CMD   0x31

/* Manufacturer IDs */
/**
 * @def XSFL_WINBOND_ID_BYTE0
 * @brief First byte of the Winbond ID.
 */
#define XSFL_WINBOND_ID_BYTE0 0xEF

#define XSFL_SIXTEENMB 0x1000000 /**< Sixteen MB. */

/**
 * @name Flash page, sector, and device sizes
 * @{
 */
#define XSFL_FLASH_PAGE_SIZE_256           256 /**< Page size of 256 bytes. */
#define XSFL_FLASH_SECTOR_SIZE_4KB         0x1000 /**< Sector size of 4 KB. */
/** Sector size of 256 KB. */
#define XSFL_FLASH_SECTOR_SIZE_256KB       0x40000
#define XSFL_FLASH_SECTOR_SIZE_64KB        0x10000 /**< Sector size of 64 KB. */
/** Sector size of 128 KB. */
#define XSFL_FLASH_SECTOR_SIZE_128KB       0x20000
/** Flash device size of 128 Mb. */
#define XSFL_FLASH_DEVICE_SIZE_128M        0x1000000
/** Flash device size of 256 Mb. */
#define XSFL_FLASH_DEVICE_SIZE_256M        0x2000000
/** Flash device size of 512 Mb. */
#define XSFL_FLASH_DEVICE_SIZE_512M        0x4000000
/** Flash device size of 1 Gb. */
#define XSFL_FLASH_DEVICE_SIZE_1G          0x8000000
/** Flash device size of 2 Gb. */
#define XSFL_FLASH_DEVICE_SIZE_2G          0x10000000
/** @} */

/**
 * @name Micron manufacturer IDs
 * @{
 */
/**
 * @def XSFL_MICRON_OCTAL_ID_BYTE0
 * @brief First byte of the Micron octal flash ID.
 */
#define XSFL_MICRON_OCTAL_ID_BYTE0          0x2c
#define XSFL_MICRON_QUAD_ID_BYTE0           0x20 /**< Micron QUAD flash. */
/** @} */

/**
 * @def XSFL_MACRONIX_ID_BYTE0
 * @brief First byte of the Macronix ID.
 */
#define XSFL_MACRONIX_ID_BYTE0              0xc2

/**
 * @def XSFL_GIGADEVICE_ID_BYTE0
 * @brief First byte of the GigaDevice ID.
 */
#define XSFL_GIGADEVICE_ID_BYTE0            0xc8

/**
 * @def XSFL_FLASH_DEVICE_SIZE_64M
 * @brief Flash device size of 64 MB (512 Mb).
 */
#define XSFL_FLASH_DEVICE_SIZE_64M         0x800000

/* Macronix Configuration Register 2 (CR2) mode address and values */
/**
 * @def XSFL_MX_CONFIG_REG2_MODE_ADDR
 * @brief Macronix CR2 address selecting SPI/STR-OPI/DTR-OPI mode.
 */
#define XSFL_MX_CONFIG_REG2_MODE_ADDR       0x00000000

/**
 * @def XSFL_MX_CONFIG_REG2_DTR_OPI
 * @brief CR2 value to enable DTR Octal (DOPI) mode.
 */
#define XSFL_MX_CONFIG_REG2_DTR_OPI         0x02

/**
 * @def XSFL_MX_CONFIG_REG2_SPI
 * @brief CR2 value to return the device to Extended SPI mode.
 */
#define XSFL_MX_CONFIG_REG2_SPI             0x00

/**
 * @name Micron and Macronix register definitions
 * @{
 */
/** Micron block protection bits mask. */
#define XSFL_MICRON_BP_BITS_MASK		0x7C
/** Macronix Configuration Register 2 (CR2) volatile address. */
#define XSFL_CONFIG_REG2_VOLATILE_ADDR_MX 	0x00000300
/** @} */

#define XSFL_OSPI_CNTRL 0x01 /**< Controller type. */

#define	XSFL_BROKEN_DMA 0x01 /**< Flag to use DMA or not. */

/** Maximum byte count in STIG mode. */
#define	XSFL_MAX_STIG_BYTE_CNT  8

/**
 * @name Flash device type
 * @{
 */
#define XSFL_QSPI_FLASH  0x01 /**< QSPI flash device. */
#define XSFL_OSPI_FLASH  0x02 /**< OSPI flash device. */
/** @} */

/**
 * @name Dual Byte opcode selection
 *
 * Macros to enable or disable the dual-byte opcode.
 * @{
 */
#define XSFL_DUAL_BYTE_OP_DISABLE  0x00 /**< Dual-byte opcode disabled. */
/** Extended opcode is the inverted opcode. */
#define XSFL_DUAL_BYTE_OP_INVERT   0x01
/** Extended opcode is the same as the opcode. */
#define XSFL_DUAL_BYTE_OP_SAME     0x02
/** @} */

/**
 * @name Edge mode selection
 *
 * Macros to select the edge mode: SDR+NON-PHY, SDR+PHY, or DDR+PHY.
 * @{
 */
#define XSFL_EDGE_MODE_SDR_PHY        0x0U /**< SDR+PHY edge mode. */
#define XSFL_EDGE_MODE_SDR_NON_PHY    0x1U /**< SDR+NON-PHY edge mode. */
#define XSFL_EDGE_MODE_DDR_PHY        0x2U /**< DDR+PHY edge mode. */
/** @} */

/**
 * @name Chip Select selection
 *
 * Macros to select CS0 and CS1.
 * @{
 */
#define XSFL_SELECT_FLASH_CS0	0 /**< Select the flash on CS0. */
#define XSFL_SELECT_FLASH_CS1	1 /**< Select the flash on CS1. */
/** @} */

/**
 * @name Connection mode selection
 *
 * Macros to select SINGLE, STACKED, and PARALLEL connection modes.
 * @{
 */
#define XSFL_CONNECTION_MODE_SINGLE      0x0U /**< Single connection mode. */
#define XSFL_CONNECTION_MODE_STACKED     0x1U /**< Stacked connection mode. */
/** @} */

/**
 * @name Prototype (Bus width: Cmd_Addr_Data) selection
 *
 * Macros to select the read and write prototype. Each digit gives the bus
 * width of the command, address, and data phases.
 * @{
 */
#define XSFL_FLASH_PROTO_1_1_1    0U	/**< 1-1-1 prototype. */
#define XSFL_FLASH_PROTO_1_0_1    1U	/**< 1-0-1 prototype. */
#define XSFL_FLASH_PROTO_1_1_2    2U	/**< 1-1-2 prototype. */
#define XSFL_FLASH_PROTO_1_1_8    3U	/**< 1-1-8 prototype. */
#define XSFL_FLASH_PROTO_1_8_8    4U	/**< 1-8-8 prototype. */
#define XSFL_FLASH_PROTO_8_8_8    5U	/**< 8-8-8 prototype. */
#define XSFL_FLASH_PROTO_8_0_0    6U	/**< 8-0-0 prototype. */
#define XSFL_FLASH_PROTO_8_8_0    7U	/**< 8-8-0 prototype. */
#define XSFL_FLASH_PROTO_8_0_8    8U	/**< 8-0-8 prototype. */
#define XSFL_FLASH_PROTO_1_1_4    9U	/**< 1-1-4 prototype. */
#define XSFL_FLASH_PROTO_4_4_4    10U	/**< 4-4-4 prototype. */
/** @} */

/**
 * @name Flash information options
 *
 * Options that select the flash information to read.
 * @{
 */
#define XSFL_FLASH_ID      0U /**< Flash device JEDEC ID. */
/**
 * Individual device size, or combined size in case of stacked or parallel
 * configuration.
 */
#define XSFL_DEVICE_SIZE   1U
/**
 * Individual sector size, or combined sector size in case of parallel
 * configuration.
 */
#define XSFL_SECT_SIZE     2U
/**
 * Individual sector size, or combined sector size in case of parallel
 * configuration.
 */
#define XSFL_PAGE_SIZE     3U
#define XSFL_FLASH_TYPE    4U /**< Flash type (QSPI/OSPI). */
/** @} */

/**
 * @name Bus width
 * @{
 */
#define XSFL_X1_BUS_WIDTH  0x01 /**< x1 bus width. */
#define XSFL_X2_BUS_WIDTH  0x02 /**< x2 bus width. */
#define XSFL_X4_BUS_WIDTH  0x04 /**< x4 bus width. */
#define XSFL_X8_BUS_WIDTH  0x08 /**< x8 bus width. */
/** @} */

/* Register BitFields */

/* Winbond */
/**
 * @def XSFL_WB_QUAD_ENABLE_BIT
 * @brief Winbond QE bit: Status Register-2 bit[1] (SR2[1]).
 *
 * Setting this bit enables Quad SPI transfers.
 */
#define XSFL_WB_QUAD_ENABLE_BIT 0x02U

/* Macronix */
/**
 * @def XSFL_MX_QUAD_ENABLE_BIT
 * @brief Macronix QE bit: Status Register bit[6] (SR[6]).
 *
 * Setting this bit enables Quad SPI transfers.
 */
#define XSFL_MX_QUAD_ENABLE_BIT 0x40U

/**************************** Type Definitions *******************************/

/**
 * @brief Holds the configuration information for a flash message.
 */
typedef struct {
	u64 RxAddr64bit;  /**< 64-bit read address for a 32-bit platform. */
	u32 ByteCount;	  /**< Number of bytes to read or write. */
	u32 Addr;	  /**< Device address. */

	u8 *TxBfrPtr;	  /**< Write buffer pointer. */
	u8 *RxBfrPtr;	  /**< Read buffer pointer. */
	u8 Opcode;	  /**< Opcode/command. */
	u8 Dummy;	  /**< Number of dummy cycles for the opcode. */
	u8 Xfer64bit;	  /**< Set to 1 when reading from a 64-bit address,
			   * otherwise 0. */

	u8 DualByteOpCode;	/**< Extended opcode in dual-byte opcode
				 * mode. */
	u8 Addrsize;	  /**< Size of address in bytes. */
	u8 Addrvalid;	  /**< 1 if the opcode requires an address, else 0. */
	u8 Proto;	  /**< Indicates the number of Cmd-Addr-Data lines. */
} XSfl_Msg;

/**
 * @brief Holds the configuration information for the flash device.
 */
typedef struct {
	u32 FlashIndex;	  /**< Index in the flash_info structure. */
	u32 DeviceSize;	  /**< Size of one flash device, or the combined size of
			   * both devices in case of stacked/parallel
			   * connection. */
	u32 SectSize;	  /**< Individual sector size, or combined sector size
			   * in case of parallel configuration. */
	u32 SectCount;	  /**< Individual sector count, or combined sector count
			   * in case of stacked configuration. */
	u32 PageSize;	  /**< Individual page size, or combined page size in
			   * case of parallel configuration. */
	u32 PageCount;	  /**< Individual page count, or combined page count in
			   * case of stacked configuration. */
	u8 ConnectionMode;	/**< Connection type
				 * (single/stacked/parallel). */
	u8 FlashMake;	  /**< Vendor information. */
	u8 AddrSize;	  /**< Size of address in bytes. */
#if CONFIG_ADVANCED_SECTOR_PROTECTION
	const XSfl_AspVendorOps *AspOps;	/**< Advanced sector-protection
						 * and password operations. */
#endif
} XSfl_FlashInfo;

/**
 * @brief Holds the configuration information for the controller device.
 */
typedef struct {
	u32 *DeviceIdData;	/**< Device ID data, used for configuring
				 * SDR/DDR mode. */
	u32 OpMode;		/**< Operating mode, DAC or INDAC. */
	u32 RefClockHz;		/**< Input clock frequency. */
	u8 DeviceId;		/**< ID of the controller device. */
	u8 ChipSelectNum;	/**< Chip select information. */
	u8 SdrDdrMode;		/**< Edge mode, which can be SDR or DDR. */
	u8 CntrlType;		/**< Type of the interface controller. */
	u8  BusWidth;		/**< Bus width available on the board. */
	u32 (*RxTunning)();	/**< Callback reference for Rx tuning. */

	u32 (*SelectFlash)(u8 ChipSelNum);	/**< Callback reference for chip
						 * select. */
	u32 (*Transfer)(u8 Index, XSfl_Msg *SflMsg);
				/**< Callback reference for the interface
				 * transfer function. */
	u32 (*NonBlockingTransfer)(u8 Index, XSfl_Msg *SflMsg);
				/**< Callback reference for the interface
				 * nonblocking transfer function. */
	u32 (*SetSdrDdr)(u8 Mode, u8 DualByteOpCode);
				/**< Callback reference for the controller
				 * configuration. */
	u32 (*TransferDone)(u8 Index);	/**< Callback reference for the flash
					 * read status. */
	u32 (*DeviceReset)(u8 Type);	/**< Callback reference for device
					 * reset. */

} XSfl_CntrlInfo;

/**
 * @brief Holds the interface information for the SFL library.
 */
typedef struct {
	XSfl_CntrlInfo CntrlInfo;	/**< Controller-specific information. */
	XSfl_FlashInfo SflFlashInfo;	/**< Flash-specific information. */
	u32 Quirks;			/**< Quirks to handle specific use
					 * cases. */

} XSfl_Interface;

/**
 * @brief Holds the information of the SFL library.
 */
typedef struct {
	int Index;				/**< Index of the next available
						 * instance. */
	XSfl_Interface Instance[XSFL_NUM_INSTANCES];	/**< Interface instances
							 * of the SFL
							 * library. */
} XSfl;

/**
 * @brief Holds the configuration information for a flash/controller device.
 *
 * Allocate a variable of this type and pass it to the library API functions.
 */
typedef union XSfl_UserConfig{
	/**
	 * @brief Holds the OSPI controller configuration.
	 */
	struct Ospi_Conig{
		u32 BaseAddress;	/**< Base address of the device. */
		u8 ReadMode;		/**< Operating mode, DAC or INDAC. */
		u8 ChipSelect;		/**< Chip select information. */
		u32 Quirks;		/**< Quirks to handle specific use
					 * cases. */
	} Ospi_Config;            /**< OSPI controller configuration. */
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

#if CONFIG_ADVANCED_SECTOR_PROTECTION
void XSfl_FlashAspAssignVendorOps(XSfl_Interface *IfPtr);
u32 XSfl_AspSetPassword(u8 SflHandler, const XSfl_AspParams *Params);
u32 XSfl_AspLockSectors(u8 SflHandler, const XSfl_AspParams *Params);
u32 XSfl_AspUnlockSectors(u8 SflHandler, const XSfl_AspParams *Params);
u32 XSfl_AspReadSectorLock(u8 SflHandler, const XSfl_AspParams *Params);
#endif
/************************** Variable Definitions *****************************/

/** @} */
#endif /* XILSFL_H */
