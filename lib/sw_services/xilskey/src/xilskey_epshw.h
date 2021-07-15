/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xilskey_epshw.h
 *
 *
 * @note	None.
 *
 *
 * MODIFICATION HISTORY:
 *
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 4.00  vns     09/10/15 Added DFT control bits addresses
* 7.2   am      07/13/21 Fixed doxygen warnings
*
* </pre>
*
 *****************************************************************************/

#ifndef XILSKEY_EPSHW_H
#define XILSKEY_EPSHW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xilskey_utils.h"

/**
 * Rsa Key hash length in bits
 */
#define XSK_EFUSEPS_RSA_KEY_HASH_LEN_BITS		(256)
/**
 * Rsa Key hash length calculation
 */
#define XSK_EFUSEPS_RSA_HASH_LEN_ECC_CALC		(260)

/**
 * @name  Hamming information
 * @{
 */
/**< Hamming loops, data and length */
#define XSK_EFUSEPS_HAMMING_LOOPS				(10)
#define XSK_EFUSEPS_HAMMING_LENGTH 				(31)
#define XSK_EFUSEPS_HAMMING_DATA				(26)
/** @} */

/**
 * @name  Mode types
 * @{
 */
/**< Mode types and definitions */
#define XSK_EFUSEPS_SINGLE_MODE                 (0x0)
#define XSK_EFUSEPS_REDUNDANCY_MODE             (0x1)
/** @} */

/**
 * @name  ReadMode
 * @{
 */
/**< ReadModes and definitions */
#define XSK_EFUSEPS_READ_MODE_NORMAL			(0x1)
#define XSK_EFUSEPS_READ_MODE_MARGIN_1			(0x2)
#define XSK_EFUSEPS_READ_MODE_MARGIN_2			(0x3)
/** @} */

/**
 * @name  EFUSE operation modes
 * @{
 */
/**< EFUSE operation modes and definitions */
#define XSK_EFUSEPS_ENABLE_PROGRAMMING			(0x1)
#define XSK_EFUSEPS_ENABLE_READ					(0x2)
#define XSK_EFUSEPS_ENABLE_WRITE				(0x4)
/** @} */

/**
 * Strobe width calculation
 */
#define XSK_EFUSEPS_PRGM_STROBE_WIDTH(RefClk)	((12 * (RefClk))/1000000)
/**
 *  Modified to have max of 32 bit value
 */
#define XSK_EFUSEPS_RD_STROBE_WIDTH(RefClk)		((15 * (RefClk))/100000000)

/**
 * @name  EFUSE Reference Clock frequency
 * @{
 */
/**< EFUSE Reference Clock frequency definitions */
#define XSK_EFUSEPS_REFCLK_LOW_FREQ				(20000000)
#define XSK_EFUSEPS_REFCLK_HIGH_FREQ			(60000000)
/** @} */

/**
 * PSS eFUSE Register addresses
 */
/**
 * eFuse base address
 */
#define XSK_EFUSEPS_BASE_ADDRESS                (0xF800D000)
	/**
	 * WR_LOCK    Write lock offset
	 */
#define XSK_EFUSEPS_WR_LOCK_REG_OFFSET   		(0x0)
	/**
	 * WR_UNLOCK    Write 0xDF0D to allow write offset
	 */
#define XSK_EFUSEPS_WR_UNLOCK_REG_OFFSET  		(0x4)
	/**
	 * WR_LOCKSTA   Write protection status offset
	 */
#define XSK_EFUSEPS_WR_LOCK_STATUS_REG_OFFSET	(0x8)
	/**
	 * CFG  Configuration register offset
	 */
#define XSK_EFUSEPS_CONFIG_REG_OFFSET    		(0xC)
	/**
	 * STATUS       Status register offset
	 */
#define XSK_EFUSEPS_STATUS_REG_OFFSET  			(0x10)
	/**
	 * CONTROL      Control register offset
	 */
#define XSK_EFUSEPS_CONTROL_REG_OFFSET    		(0x14)
	/**
	 * PGM_STBW     eFuse program strobe width register offset
	 */
#define XSK_EFUSEPS_PGM_STBW_REG_OFFSET 		(0x18)
	/**
	 * RD_STBW      eFuse read strobe width register offset
	 */
#define XSK_EFUSEPS_RD_STBW_REG_OFFSET    		(0x1C)
	/**
	 * WR_LOCK      Write 0x767B to disallow write
	 */
#define XSK_EFUSEPS_WR_LOCK_REG   		   (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_WR_LOCK_REG_OFFSET)
	/**
	 * WR_UNLOCK    Write 0xDF0D to allow write
	 */
#define XSK_EFUSEPS_WR_UNLOCK_REG         (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_WR_UNLOCK_REG_OFFSET)
	/**
	 *  WR_LOCKSTA   Write protection status
	 */
#define XSK_EFUSEPS_WR_LOCK_STATUS_REG    (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_WR_LOCK_STATUS_REG_OFFSET)
	/**
	 *  CFG  Configuration register
	 */
#define XSK_EFUSEPS_CONFIG_REG            (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_CONFIG_REG_OFFSET)
	/**
	 *  STATUS       Status register
	 */
#define XSK_EFUSEPS_STATUS_REG            (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_STATUS_REG_OFFSET)
	/**
	 *  CONTROL      Control register
	 */
#define XSK_EFUSEPS_CONTROL_REG           (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_CONTROL_REG_OFFSET)
	/**
	 * PGM_STBW     eFuse program strobe width register
	 */
#define XSK_EFUSEPS_PGM_STBW_REG          (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_PGM_STBW_REG_OFFSET)
	/**
	 *  RD_STBW      eFuse read strobe width register
	 */
#define XSK_EFUSEPS_RD_STBW_REG           (XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_RD_STBW_REG_OFFSET)


/**< PSS eFUSE register bit defines & description */

/**< XSK_EFUSEPS_WR_LOCK_STATUS_REG (Write Protection Status Register) */
/** Current state of write protection mode of eFuse subsystem:-
* 0 Region is writable
* 1 Region is not writable. Any attempted writes are ignored, but reads will complete as normal.
*/
#define XSK_EFUSEPS_WR_LOCK_STATUS_BIT		(0x1)
/**< XSK_EFUSEPS_CONFIG_REG (Configuration Register) */
/** Redundancy mode, if set, else single mode.
*  This bit only applies to APB access.
*  BISR and eFuse reader always work in redundancy mode.
*/
#define XSK_EFUSEPS_CONFIG_REDUNDANCY		(0x00010000)
/** eFuse read/program setup/hold control between address and strobe assert
*  1 b0  1 ref clock cycle
*  1 b1  2 ref clock cycles
*/
#define XSK_EFUSEPS_CONFIG_TSU_H_A		(0x00002000)
/** eFuse read/program setup/hold control between csb and strobe assert
*  1 b0  1 ref clock cycle
*  1 b1  2 ref clock cycles
*/
#define XSK_EFUSEPS_CONFIG_TSU_H_CS		(0x00001000)
	/**
	 *  eFuse program setup/hold control between ps and csb active
	 */
#define XSK_EFUSEPS_CONFIG_TSU_H_PS		(0x00000F00)

/**
 * @name  eFuse read margin control
 * @{
 */
/**< eFuse read margin control:
 * 00  normal, 01  margin 1, 10  margin 2, 11 - undefined
 */
#define XSK_EFUSEPS_CONFIG_MARGIN_RD		(0x00000030)
#define XSK_EFUSEPS_CONFIG_RD_NORMAL		(0x00000000)
#define XSK_EFUSEPS_CONFIG_RD_MARGIN_1		(0x00000010)
#define XSK_EFUSEPS_CONFIG_RD_MARGIN_2		(0x00000020)
/** @} */

/** Reference clock scaler
*  2 b00  bypass clock divider
*  2 b01  div 2
*  2 b10  div 4
*  2 h11  div 8
*/
#define XSK_EFUSEPS_CONFIG_CLK_DIV		(0x00000003)


/**< XSK_EFUSEPS_STATUS_REG (Status Register)*/
/** Status Register containing BISR Controller status, trim value,
*  and security debug info.
*/

/**
 *  Build in self test finished at boot time
 */
#define XSK_EFUSEPS_STATUS_BISR_DONE		(0x80000000)
/**
 *  Build in self test finished successfully
 */
#define XSK_EFUSEPS_STATUS_BISR_GO			(0x40000000)
/**
 *  eFuse box is blank, i.e., not yet been written to, if set
 */
#define XSK_EFUSEPS_STATUS_BISR_BLANK		(0x00100000)
/** Security debug status, with authentication
*  0  security debug enabled
*  1  security debug disabled
*/
#define XSK_EFUSEPS_STATUS_SDEBUG_DIS		(0x00010000)
/** eFuse write protection, if either bit is set,
 * writes to the eFuse box are disabled
 */
#define XSK_EFUSEPS_STATUS_WR_PROTECT		(0x00003000)
/**
 *  Analog trim value
 */
#define XSK_EFUSEPS_STATUS_TRIM			(0x000000FC)


/**
 *  XSK_EFUSEPS_CONTROL_REG (Control register for eFuse program,
 *  read and write control)
 *  eFuse ps control, enable programming if set.
 */
#define XSK_EFUSEPS_CONTROL_PS_EN		(0x00000010)
/**
 *  eFuse write disable, if set.
 */
#define XSK_EFUSEPS_CONTROL_WR_DIS		(0x00000002)
/**
 *  eFuse read disable, if set
 */
#define XSK_EFUSEPS_CONTROL_RD_DIS		(0x00000001)

/**
 * eFuse memory APB Customer key start address offset
 */
#define XSK_EFUSEPS_APB_START_ADDR_OFFSET      		(0x1000)
/**
 * eFuse memory APB Customer key first half start address offset
 */
#define XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_1_OFFSET 	(0x20)
/**
 * eFuse memory APB Customer key second half start address offset
 */
#define XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_2_OFFSET 	(0x24)
/**
 * eFUSE APB address for ROM 128k CRC enable offset
 */
#define XSK_EFUSEPS_APB_ROM_128K_CRC_ENABLE_OFFSET 		(0x28)
/**
 * eFUSE APB address for RSA authentication enable offset
 */
#define XSK_EFUSEPS_APB_RSA_AUTH_ENABLE_OFFSET 			(0x2C)
/**
 * eFUSE DFT JTAG disable
 */
#define XSK_EFUSEPS_APB_DFT_JTAG_DISABLE_OFFSET 			(0x30)
/**
 * eFUSE DFT mode disable
 */
#define XSK_EFUSEPS_APB_DFT_MODE_DISABLE_OFFSET 			(0x34)
/**
 * eFUSE APB address for RSA uart status enable on MIO48 offset
 */
#define XSK_EFUSEPS_APB_ROM_UART_STATUS_ENABLE_OFFSET 		(0x5C0)
/**
 * eFUSE APB address for non-secure INIT_B signaling offset
 */
#define XSK_EFUSEPS_APB_ROM_NONSECURE_INITB_ENABLE_OFFSET (0x5C4)

/** eFUSE bits from 0 to 0x1F and 0x180 to 0x1FF in the First half,
*  and bits from 0x200 to 0x21F and 0x380 to 0x3FF in the
*  Second half(if Single mode is enabled)
*/
/**
 *  If Redundant mode is enabled only First half addresses are valid.
 *  eFuse memory APB Customer key first half start address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR_OFFSET	(0x80)
/**
 *  eFuse memory APB Customer key first half end address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_END_ADDR_OFFSET	(0x580)
/**
 * If Single mode is enabled both First and Second half addresses are valid.
 * eFuse memory APB Customer key second half start address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_START_ADDR_OFFSET	(0x880)
/**
 *  eFuse memory APB Customer key second half end address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_END_ADDR_OFFSET	(0xE00)
/**
 * Mirror Address = addr + 2nd half start address + mirror offset
 */
#define XSK_EFUSEPS_APB_MIRROR_ADDRESS(Addr)	(Addr + 0x87C - (2*(Addr%128)))

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the First half of the eFUSE block offsets
 * and definitions*/
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x20_OFFSET		(0x80)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x41_OFFSET		(0x104)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x62_OFFSET		(0x188)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x83_OFFSET		(0x20C)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xA4_OFFSET		(0x290)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xC5_OFFSET		(0x314)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xE6_OFFSET		(0x398)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x107_OFFSET	(0x41C)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x128_OFFSET	(0x4A0)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x149_OFFSET	(0x524)

#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x23F_OFFSET	(0x8FC)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x25E_OFFSET	(0x978)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x27D_OFFSET	(0x9F4)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x29C_OFFSET	(0xA70)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2BB_OFFSET	(0xAEC)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2DA_OFFSET	(0xB68)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2F9_OFFSET	(0xBE4)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x318_OFFSET	(0xC60)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x337_OFFSET	(0xCDC)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x356_OFFSET	(0xD58)

#define XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_OFFSET  		(0x1C)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_OFFSET  	(0x40)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_OFFSET  	(0x7C)
#define XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_OFFSET  		(0x600)
#define XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_OFFSET  		(0x7FC)
/** @} */

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the Second half of the eFUSE block offsets */
#define XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR_2ND_HALF_OFFSET  			(0x860)
#define XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_2ND_HALF_OFFSET  			(0x87C)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_2ND_HALF_OFFSET  	(0x800)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_2ND_HALF_OFFSET		(0x83C)
#define XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_2ND_HALF_OFFSET  			(0xE00)
#define XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_2ND_HALF_OFFSET  			(0xFFC)
/** @} */

/**
 * eFuse memory APB start address
 */
#define XSK_EFUSEPS_APB_START_ADDR      			(XSK_EFUSEPS_BASE_ADDRESS + XSK_EFUSEPS_APB_START_ADDR_OFFSET)
/**
 * eFuse memory APB Customer key second half start address
 */
#define XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_1 	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_1_OFFSET)
/**
 * eFuse memory APB Customer key second half start address
 */
#define XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_2 	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_WRITE_PROTECTION_ADDR_2_OFFSET)
/**
 * eFUSE APB address for ROM 128k CRC enable
 */
#define XSK_EFUSEPS_APB_ROM_128K_CRC_ENABLE 		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_ROM_128K_CRC_ENABLE_OFFSET)
/**
 * eFUSE APB address for RSA authentication enable
 */
#define XSK_EFUSEPS_APB_RSA_AUTH_ENABLE 			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_RSA_AUTH_ENABLE_OFFSET)
/**
 * eFuse DFT JTAG disable
 */
#define XSK_EFUSEPS_APB_DFT_JTAG_DISABLE				(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_DFT_JTAG_DISABLE_OFFSET)
/**
 * eFuse DFT mode disable
 */
#define XSK_EFUSEPS_APB_DFT_MODE_DISABLE				(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_DFT_MODE_DISABLE_OFFSET)
/**
 * eFUSE APB address for RSA uart status enable on MIO48
 */
#define XSK_EFUSEPS_APB_ROM_UART_STATUS_ENABLE 	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_ROM_UART_STATUS_ENABLE_OFFSET)
/**
 * eFUSE APB address for non-secure INIT_B signaling
 */
#define XSK_EFUSEPS_APB_ROM_NONSECURE_INITB_ENABLE (XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_ROM_NONSECURE_INITB_ENABLE_OFFSET)

/** eFUSE bits from 0 to 0x1F and 0x180 to 0x1FF in the First half,
*  and bits from 0x200 to 0x21F and 0x380 to 0x3FF in the
*  Second half(if Single mode is enabled)
*/
/**
 *  If Redundant mode is enabled only First half addresses are valid.
 *
 *  eFuse memory APB Customer key first half start address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_START_ADDR_OFFSET)
/**
 *  eFuse memory APB Customer key first half end address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_END_ADDR	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_CUSTOMER_KEY_FIRST_HALF_END_ADDR_OFFSET)
/**
 *  If Single mode is enabled both First and Second half addresses are valid.
 *
 *  eFuse memory APB Customer key second half start address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_START_ADDR	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_START_ADDR_OFFSET)
/**
 * eFuse memory APB Customer key second half end address
 */
#define XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_END_ADDR	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_CUSTOMER_KEY_SECND_HALF_END_ADDR_OFFSET)
/**
 * Mirror Address = addr + 2nd half start address + mirror offset
 */
#define XSK_EFUSEPS_APB_MIRROR_ADDRESS(Addr)	(Addr + 0x87C - (2*(Addr%128)))

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the First half of the eFUSE block address */
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x20		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x20_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x41		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x41_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x62		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x62_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x83		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x83_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xA4		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xA4_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xC5		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xC5_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xE6		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_xE6_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x107		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x107_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x128		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x128_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x149		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x149_OFFSET)
/** @} */

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the First half of the eFUSE block address */
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x23F		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x23F_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x25E		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x25E_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x27D		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x27D_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x29C		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x29C_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2BB		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2BB_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2DA		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2DA_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2F9		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x2F9_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x318		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x318_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x337		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x337_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x356		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_TEST_BIT_x356_OFFSET)
/** @} */

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the First half of the eFUSE block address */
#define XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR  			(XSK_EFUSEPS_APB_START_ADDR)
#define XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR  			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR  	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR  	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_OFFSET)
#define XSK_EFUSEPS_APB_BISR_BITS_START_ADDR  			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_OFFSET)
#define XSK_EFUSEPS_APB_BISR_BITS_END_ADDR  			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_OFFSET)
/** @} */

/** @name Xilinx reserved Tests bits registers
 * @{
 */
/**< Xilinx reserved Tests bits in the Second half of the eFUSE block address */
#define XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR_2ND_HALF  		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_TRIM_BITS_START_ADDR_2ND_HALF_OFFSET)
#define XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_2ND_HALF  			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_TRIM_BITS_END_ADDR_2ND_HALF_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_2ND_HALF  	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_BITS_START_ADDR_2ND_HALF_OFFSET)
#define XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_2ND_HALF  	(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_XILINX_RSVD_BITS_END_ADDR_2ND_HALF_OFFSET)
#define XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_2ND_HALF  		(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_BISR_BITS_START_ADDR_2ND_HALF_OFFSET)
#define XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_2ND_HALF  			(XSK_EFUSEPS_APB_START_ADDR + XSK_EFUSEPS_APB_BISR_BITS_END_ADDR_2ND_HALF_OFFSET)
/** @} */

/***************** Macros (Inline Functions) Definitions ********************/
/***************************************************************************/
/**
* This macro is used to lock the efuse controller
*
****************************************************************************/
#define XSK_EFUSEPS_CONTROLER_LOCK()     	Xil_Out32(XSK_EFUSEPS_WR_LOCK_REG,0x767B)

/***************************************************************************/
/**
* This macro is used to unlock the efuse controller
*
****************************************************************************/
#define XSK_EFUSEPS_CONTROLER_UNLOCK()   	Xil_Out32(XSK_EFUSEPS_WR_UNLOCK_REG,0xDF0D)

/***************************************************************************/
/**
* This macro is used to check the status whether eFuse controller is locked or not
*
* @return	- TRUE if the 32 bit Value read from the specified input address
*               - FALSE if the 32 bit Value not read from the specified input address
*
****************************************************************************/
#define XSK_EFUSEPS_CONTROLER_LOCK_STATUS() (Xil_In32(XSK_EFUSEPS_WR_LOCK_STATUS_REG) & 0x1)

/***************************************************************************/
/**
* This macro is used to determine operation mode of efuse controller
*
* @return
* 		- TRUE if eFuse controller mode is redundancy
*		- FALSE if eFuse controller mode is single
****************************************************************************/
#define XSK_EFUSEPS_CONTROLER_OP_MODE()     ((Xil_In32(XSK_EFUSEPS_CONFIG_REG) & XSK_EFUSEPS_CONFIG_REDUNDANCY)? 1 : 0)

/***************************************************************************/
/**
* This macro is used to check whether eFuse is write protected or not
*
* @return
* 		- TRUE if eFuse is write protected.
*		- FALSE is eFuse is not write protected.
****************************************************************************/

#define XilSKey_EfusePs_IsEfuseWriteProtected()     ((Xil_In32(XSK_EFUSEPS_STATUS_REG) & XSK_EFUSEPS_STATUS_WR_PROTECT)? TRUE : FALSE)
/************************** Function Prototypes ******************************/
void XilSKey_EfusePs_GenerateMatrixMap(void);
u8 XilSKey_EfusePs_EccDecode(const u8 *Corrupt, u8 *Syndrome);
void XilSKey_EfusePs_EccEncode(const u8 *InData, u8 *Ecc);
u32 XilSKey_EfusePs_ControllerConfig(u8 CtrlMode, u32 RefClk, u8 ReadMode);
u8 XilSKey_EfusePs_IsAddressXilRestricted (u32 Addr);
void XilSKey_EfusePs_ControllerSetReadWriteEnable(u32 ReadWriteEnable);
u32 XilSKey_EfusePs_ReadEfuseBit(u32 Addr, u8 *Data);
u32 XilSKey_EfusePs_WriteEfuseBit(u32 Addr);

#ifdef __cplusplus
}
#endif

#endif  /* End of XILSKEY_EPSHW_H */
