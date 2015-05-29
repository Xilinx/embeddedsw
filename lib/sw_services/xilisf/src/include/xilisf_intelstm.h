/******************************************************************************
*
* Copyright (C) 2012 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilisf_intelstm.h
*
* This file contains the definitions to be used when accessing the Intel, STM,
* Winbond and Spansion Serial Flash.
* If any new definitions are added to this file, check if they need to be
* added to the xilisf_atmel.h file too.
*
* @note	None
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------  -------- -----------------------------------------------
* 1.00a ksu/sdm  03/03/08 First release
* 2.01a sdm      01/04/10 Added Support for Winbond W25QXX/W25XX devices
* 2.04a sdm      08/17/10 Updated to support Numonyx (N25QXX) and Spansion
*			  flash memories
* 3.00a srt	 06/20/12 Updated to support interfaces SPI PS and QSPI PS.
*			  Added support to SST flash on SPI PS interface.
* 3.02a srt	 04/26/13 Modified SECTOR and BLOCK Erase commands for
*			  SST flash (CR 703816).
* 5.2   asa  05/12/15 Added macros for 4 byte commands.
* </pre>
*
******************************************************************************/
#ifndef XILISF_INTELSTM_H /* prevent circular inclusions */
#define XILISF_INTELSTM_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**
 * The following definitions specify the Device Id for the different
 * Intel (Numonyx) S33 Serial Flash devices.
 */
#define XISF_INTEL_DEV_S3316MBIT	0x11	/**< Device ID for 16Mbit */
#define XISF_INTEL_DEV_S3332MBIT	0x12	/**< Device ID for 32Mbit */
#define XISF_INTEL_DEV_S3364MBIT	0x13	/**< Device ID for 64Mbit */

/**
 * The following definitions specify the Device Id for the different
 * STM (Numonyx) M25PXX Serial Flash devices.
 */
#define XISF_STM_DEV_M25P05_A		0x2010	/**< Device ID for M25P05-A */
#define XISF_STM_DEV_M25P10_A		0x2011	/**< Device ID for M25P10-A */
#define XISF_STM_DEV_M25P20		0x2012	/**< Device ID for M25P20 */
#define XISF_STM_DEV_M25P40		0x2013	/**< Device ID for M25P40 */
#define XISF_STM_DEV_M25P80		0x2014	/**< Device ID for M25P80 */
#define XISF_STM_DEV_M25P16		0x2015	/**< Device ID for M25P16 */
#define XISF_STM_DEV_M25P32		0x2016	/**< Device ID for M25P32 */
#define XISF_STM_DEV_M25P64		0x2017	/**< Device ID for M25P64 */
#define XISF_STM_DEV_M25P128		0x2018	/**< Device ID for M25P128 */

/**
 * The following definitions specify the Device Id (memory type/capacity)
 * for the different Winbond W25QX/W25XX Serial Flash devices.
 */
#define XISF_WB_DEV_W25Q80		0x4014	/**< Device ID for W25Q80 */
#define XISF_WB_DEV_W25Q16		0x4015	/**< Device ID for W25Q16 */
#define XISF_WB_DEV_W25Q32		0x4016	/**< Device ID for W25Q32 */
#define XISF_WB_DEV_W25Q64		0x4017	/**< Device ID for W25Q64 */
#define XISF_WB_DEV_W25Q128		0x4018	/**< Device ID for W25Q128 */
#define XISF_WB_DEV_W25X10		0x3011	/**< Device ID for W25X10 */
#define XISF_WB_DEV_W25X20		0x3012	/**< Device ID for W25X20 */
#define XISF_WB_DEV_W25X40		0x3013	/**< Device ID for W25X40 */
#define XISF_WB_DEV_W25X80		0x3014	/**< Device ID for W25X80 */
#define XISF_WB_DEV_W25X16		0x3015	/**< Device ID for W25X16 */
#define XISF_WB_DEV_W25X32		0x3016	/**< Device ID for W25X32 */
#define XISF_WB_DEV_W25X64		0x3017	/**< Device ID for W25X64 */

/**
 * The following definitions specify the Device Id (memory type/capacity)
 * for the different STM (Numonyx) N25QXX Serial Flash devices.
 */
#define XISF_NM_DEV_N25Q32		0xBA16	/**< Device ID for N25Q32 */
#define XISF_NM_DEV_N25Q64		0xBA17	/**< Device ID for N25Q64 */
#define XISF_NM_DEV_N25Q128		0xBA18	/**< Device ID for N25Q128 */
#define XISF_MIC_DEV_N25Q128		0xBB18	/**< Device ID for N25Q128 */
#define XISF_MIC_DEV_N25Q256_3V0	0xBA19
#define XISF_MIC_DEV_N25Q256_1V8	0xBB19

/**
 * The following definitions specify the Device Id for the different
 * Spansion S25FLXX Serial Flash devices.
 */
#define XISF_SPANSION_DEV_S25FL004	0x0212	/**< Device ID for S25FL004 */
#define XISF_SPANSION_DEV_S25FL008	0x0213	/**< Device ID for S25FL008 */
#define XISF_SPANSION_DEV_S25FL016	0x0214	/**< Device ID for S25FL016 */
#define XISF_SPANSION_DEV_S25FL032	0x0215	/**< Device ID for S25FL032 */
#define XISF_SPANSION_DEV_S25FL064	0x0216	/**< Device ID for S25FL064 */
#define XISF_SPANSION_DEV_S25FL128	0x2018	/**< Device ID for S25FL128
						  *  and S25FL129 */

/**
 * The following definitions specify the Device Id for the different
 * SST Serial Flash device.
 */
#define XISF_SST_DEV_SST25WF080		0x2505	/**< Device ID for SST25WF080 */

/**
 * Definitions for Intel, STM, Winbond and Spansion Serial Flash Device
 * geometry.
 */
#define XISF_BYTES256_PER_PAGE		256	/**< 256 Bytes per Page */
#define XISF_PAGES16_PER_SECTOR		16	/**< 16 Pages per Sector */
#define XISF_PAGES128_PER_SECTOR	128	/**< 128 Pages per Sector */
#define XISF_PAGES256_PER_SECTOR	256	/**< 256 Pages per Sector */
#define XISF_PAGES1024_PER_SECTOR	1024	/**< 1024 Pages per Sector */
#define XISF_NUM_OF_SECTORS2		2	/**< 2 Sectors */
#define XISF_NUM_OF_SECTORS4		4	/**< 4 Sectors */
#define XISF_NUM_OF_SECTORS8		8	/**< 8 Sector */
#define XISF_NUM_OF_SECTORS16		16	/**< 16 Sectors */
#define XISF_NUM_OF_SECTORS32		32	/**< 32 Sectors */
#define XISF_NUM_OF_SECTORS64		64	/**< 64 Sectors */
#define XISF_NUM_OF_SECTORS128		128	/**< 128 Sectors */
#define XISF_NUM_OF_SECTORS256		256	/**< 256 Sectors */
#define XISF_NUM_OF_SECTORS512		512	/**< 512 Sectors */
#define XISF_NUM_OF_SECTORS1024		1024	/**< 1024 Sectors */
#define XISF_NUM_OF_SECTORS2048		2048	/**< 2048 Sectors */
#define XISF_NUM_OF_SECTORS4096		4096	/**< 4096 Sectors */

/**
 * Definitions of Read commands.
 */
#define XISF_CMD_RANDOM_READ		0x03	/**< Random Read command  */
#define XISF_CMD_RANDOM_READ_4BYTE  0x13    /**< Random 4 byte Read command  */
#define XISF_CMD_FAST_READ		0x0B	/**< Fast Read command */
#define XISF_CMD_FAST_READ_4BYTE	0x0C /**< 4 byte Fast Read command */
#define XISF_CMD_ISFINFO_READ		0x9F	/**< Device Info command */
#define XISF_CMD_STATUSREG_READ		0x05	/**< Status Reg Read command */
#define XISF_CMD_STATUSREG2_READ	0x35	/**< Status Reg2 Read command */
#define XISF_CMD_DUAL_OP_FAST_READ	0x3B	/**< Dual output fast read */
#define XISF_CMD_DUAL_OP_FAST_READ_4B	0x3C /**< 4 byte Dual output fast read */
#define XISF_CMD_DUAL_IO_FAST_READ	0xBB	/**< Dual i/o fast read */
#define XISF_CMD_DUAL_IO_FAST_READ_4B	0xBC /**< 4 byte Dual i/o fast read */
#define XISF_CMD_QUAD_OP_FAST_READ	0x6B	/**< Quad output fast read */
#define XISF_CMD_QUAD_OP_FAST_READ_4B	0x6C /**< 4 byte Quad output fast read */
#define XISF_CMD_QUAD_IO_FAST_READ	0xEB	/**< Quad i/o fast read */
#define XISF_CMD_QUAD_IO_FAST_READ_4B	0xEC /**< 4 byte Quad i/o fast read */

/**
 * Definitions of Write commands.
 */
#define XISF_CMD_PAGEPROG_WRITE		0x02	/**< Page Program command */
#define XISF_CMD_PAGEPROG_WRITE_4BYTE	0x12	/**< 4 byte Page Program command */
#define XISF_CMD_STATUSREG_WRITE	0x01	/**< Status Reg Write Command */
#define XISF_CMD_DUAL_IP_PAGE_WRITE	0xA2	/**< Dual input fast page write
						  */
#define XISF_CMD_DUAL_IP_EXT_PAGE_WRITE	0xD2	/**< Dual input extended fast
						  *  page write */
#define XISF_CMD_QUAD_IP_PAGE_WRITE	0x32	/**< Quad input fast page write
						  */
#define XISF_CMD_QUAD_IP_EXT_PAGE_WRITE	0x12	/**< Dual input extended fast
						  *  page write */

/**
 * Definitions of Erase commands.
 */
#define XISF_CMD_BULK_ERASE		0xC7	/**< Bulk Erase command */


#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || (XPAR_XISF_FLASH_FAMILY == STM) || \
    (XPAR_XISF_FLASH_FAMILY == SPANSION))
#define XISF_CMD_SECTOR_ERASE		0xD8	/**< Sector Erase command */
#define XISF_CMD_SUB_SECTOR_ERASE	0x20	/**< Sub-sector Erase command.
						  *  only for N25QXX */
#define XISF_CMD_4BYTE_SECTOR_ERASE 0xDC
#endif /* ((XPAR_XISF_FLASH_FAMILY==INTEL)||(XPAR_XISF_FLASH_FAMILY == STM) ||
	   (XPAR_XISF_FLASH_FAMILY == SPANSION)) */

#if ((XPAR_XISF_FLASH_FAMILY == WINBOND) || (XPAR_XISF_FLASH_FAMILY == SST))
#define XISF_CMD_BLOCK_ERASE		0xD8	/**< Block Erase command */
#define XISF_CMD_SECTOR_ERASE		0x20	/**< Sector Erase command */
#endif

/**
 * Definitions of commands used for
 *  - Write Enable/Disable.
 *  - Deep Power Down mode Enter/Release.
 *  - Switch to 4 byte addressing
 */
#define XISF_CMD_ENABLE_WRITE		0x06	/**< Write enable command */
#define XISF_CMD_DISABLE_WRITE		0x04	/**< Write disable command */
#define XISF_CMD_DEEP_POWER_DOWN	0xB9	/**< Enter DPD mode command */
#define XISF_CMD_RELEASE_FROM_DPD	0xAB	/**< Release DPD mode command */

#define XISF_CMD_ENABLE_HPM		0xA3	/**< Enable high performance
						  *  mode */

#if (XPAR_XISF_FLASH_FAMILY == SPANSION)
#define XISF_CMD_ENTER_4BYTE_ADDR_MODE	0xB7
#define XISF_CMD_EXIT_4BYTE_ADDR_MODE	0xE9
#endif
#if (XPAR_XISF_FLASH_FAMILY == INTEL)
/**
 * Definitions of commands which are only supported in Intel Serial Flash.
 */
#define XISF_CMD_OTP_READ		0x4B	/**< OTP data read command */
#define XISF_CMD_OTP_WRITE		0x42	/**< OTP write command */
#define XISF_CMD_PARAM_BLOCK_ERASE	0x40	/**< Parameter Block Erase
						  *  command */
#define XISF_CMD_CLEAR_SRFAIL_FLAGS	0x30	/**< Clear SR fail bits Cmd */
#define XISF_OTP_RDWR_EXTRA_BYTES	0x05 	/**< OTP Read/Write
						  *  extra bytes */
#endif /* INTEL */


/**
 * The following definitions specify the Status Register bit definitions of
 * Intel, STM, Winbond and Spansion Serial Flash.
 */
#define XISF_SR_IS_READY_MASK		0x01	/**< Ready mask */
#define XISF_SR_WRITE_ENABLE_MASK 	0x02	/**< Write Enable latch mask */
#define XISF_SR_BLOCK_PROTECT_MASK	0x1C	/**< Block Protect mask */
#define XISF_SR_WRITE_PROTECT_MASK	0x80	/**< Status Reg write
						   * protect mask */
#define XISF_SR_BLOCK_PROTECT_SHIFT	2	/**< Block protect bits shift */

#if (XPAR_XISF_FLASH_FAMILY == INTEL)
#define XISF_SR_PROG_FAIL_MASK		0x40	/**< Program Fail bit mask */
#define XISF_SR_ERASE_FAIL_MASK		0x20	/**< Erase Fail bit mask */
#endif /* (XPAR_XISF_FLASH_FAMILY == INTEL) */

#if (XPAR_XISF_FLASH_FAMILY == WINBOND)
#define XISF_SR_TB_PROTECT_MASK		0x20	/**< Top/Bottom Write Protect */
#define XISF_SR_SECTOR_PROTECT_MASK	0x40	/**< Sector Protect mask */
#endif /* (XPAR_XISF_FLASH_FAMILY == WINBOND) */


/**
 * The definition specifies the total bytes in Bulk Erase commands.
 *  This count includes Command byte and any don't care bytes needed.
 */
#define XISF_BULK_ERASE_BYTES		0x01	/**< Bulk erase extra bytes */

/**
 * The following definitions specify the Write Enable and Disable operation
 * arguments to be passed to the XIsf_WriteEnable API.
 */
#define XISF_WRITE_ENABLE		1	/**< Write enable */
#define XISF_WRITE_DISABLE		0	/**< Write disable */

/**
 * This definition specifies the extra bytes in each of the Write Enable/Disable
 * commands. This count includes Command byte, address bytes and any don't care
 * bytes needed.
 */
#define XISF_CMD_WRITE_ENABLE_DISABLE_BYTES	1  /**< Write enable/disable
						     *  command extra bytes */

/**
 * This definition specifies the extra bytes in 4 byte addr mode enter and exit
 * commands. This count refers to the Command byte.
 */
#define XISF_CMD_4BYTE_ADDR_ENTER_EXIT_BYTES	1 /**< Four byte addr mode
							 *  command extra bytes */
/**
 * This definition specifies the extra bytes in each of the Write/Read/Erase
 * commands, commands operating on SPR, auto page write, page to
 * buffer and buffer to page transfer commands. This count includes
 * Command byte, Address bytes and any don't care bytes needed.
 */
#define XISF_CMD_SEND_EXTRA_BYTES	4	/**< Command extra bytes */

/**
 * This definition specifies the extra bytes in each of the Write/Read/Erase
 * commands, commands operating on SPR, auto page write, page to
 * buffer and buffer to page transfer commands in 4 bytes addressing mode.
 * This count includes Command byte, Address bytes and any
 * don't care bytes needed.
 */
#define XISF_CMD_SEND_EXTRA_BYTES_4BYTE_MODE	5 /**< Command extra bytes */

/**
 * This definition specifies the extra bytes in Fast Read and Fast Buffer Read
 * commands. This count includes Command byte, address bytes and any don't care
 * bytes needed.
 */
#define XISF_CMD_FAST_READ_EXTRA_BYTES	5	/**< Fast read and Fast buffer
						  *  read extra bytes */

/**
 * The following definitions specify the total bytes in some of the commands.
 * This count includes Command byte and any don't care bytes needed.
 */
#define XISF_STATUS_RDWR_BYTES		2	/**< Status Read/Write bytes
						  *  count */
#define XISF_INFO_READ_BYTES		5	/**< Serial Flash Info read
						  *  bytes count */
#define XISF_INFO_EXTRA_BYTES		1	/**< Serial Flash Info extra
						  *  bytes */
#define XISF_IOCTL_BYTES		1	/**< Serial Flash IOCTL bytes */
#define XISF_HPM_BYTES			4	/**< Serial Flash HPM bytes */
#define XISF_CMD_MAX_EXTRA_BYTES	5	/**< Max extra bytes for
						  *  all commands  */
#define XISF_DUMMYBYTE			0xFF	/**< Dummy byte to fill */

/**
 * Address Shift Masks.
 */
#define XISF_ADDR_SHIFT24		24 /**< 24 bit Shift */
#define XISF_ADDR_SHIFT16		16 /**< 16 bit Shift */
#define XISF_ADDR_SHIFT8		8  /**< 8 bit Shift */

/**
 * Byte Positions.
 */
#define BYTE1				0 /**< Byte 1 position */
#define BYTE2				1 /**< Byte 2 position */
#define BYTE3				2 /**< Byte 3 position */
#define BYTE4				3 /**< Byte 4 position */
#define BYTE5				4 /**< Byte 5 position */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif  /* end of protection macro */
