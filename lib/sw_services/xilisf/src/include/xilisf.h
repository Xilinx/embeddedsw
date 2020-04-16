/******************************************************************************
* Copyright (c) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilisf.h
 * @addtogroup xilisf_apis Library-APIs
 * @{
 * @cond xilisf_internal
 * @{
 *
 * The Xilinx In-system and Serial Flash (XilIsf) Library supports the Xilinx
 * In-system Flash and, external Serial Flash Memories from Atmel (AT45XXXD),
 * Intel (S33), ST Microelectronics (STM) (M25PXX), Winbond (W25QXX/W25XX),
 * Numonyx (N25QXX) devices. Intel (S33) and STM (M25PXX) Serial Flash devices
 * are now a part of Serial Flash devices provided by Numonyx. Flash parts
 * N25QXX are now known as Micron MT25QXX keeping all the functionality same.
 *
 * <b>This library also supports the Spansion (S25FLXX) devices, but this family
 * hasn't been tested. The support for this family of devices is limited to the
 * common commands supported by the other flash families</b>
 *
 * The following instructions are not supported in WinBond (W25QXX/W25XX) -
 * Block Erase 32KB, Erase Suspend/Resume, Mode Bit Reset, Read Unique ID,
 * and Read Manufacturer/Device ID.
 *
 * The following instructions are not supported in Numonyx (N25QXX)- Erase
 * Suspend/Resume, Read/Write Volatile/Non-volatile configuration register.
 *
 * <b>Library Description</b>
 *
 * The library enables higher layer software (e.g. an application) to
 * communicate with the Serial Flash.
 *
 * The library allows the user to Write, Read and Erase the Serial Flash.
 * The user can also protect the data stored in Serial Flash from unwarranted
 * modification by enabling the Sector Protection feature. User can also perform
 * different Control operations on Intel, STM (Numonyx), Winbond and Spansion
 * Serial Flash devices.
 *
 * The library supports interrupt driven mode and polled mode based on the
 * mode in which the Spi driver is configured by the user.
 *
 * - Polled Mode of operation: All the APIs are blocking in this mode.
 * - Interrupt mode of operation: It is the application's responsibility to
 * acknowledge the associated Interrupt Controller's interrupts.
 * The transfer is initiated and the control is given back to the user
 * application. The user application has to keep track of whether the initiated
 * operation is completed successfully.
 *
 * This library can support multiple instances of Serial Flash at a time,
 * provided they are of the same device family (either Atmel, Intel, STM or
 * Spansion) as the device family is selected at compile time.
 *
 * <b>Device Operation</b>
 *
 * The Serial Flash operates as a slave device on the SPI bus, with Xilinx SPI
 * core operating as the Master. The library uses XSpi driver for communicating
 * with the Serial Flash.
 *
 * <b>Device Geometry</b>
 *
 * - Atmel (AT45XXXD)/Xilinx ISF:
 * The Atmel Serial Flash is divided into Sectors. Each Sector consists of
 * multiple Blocks. Each Block contains multiple Pages. Each Page contains
 * multiple Bytes. The Number of Sectors, Blocks Per Sector, Pages Per Block and
 * Bytes Per Page vary for different devices within this family.
 * There are two addressing modes supported by the Atmel Serial Flash:
 *	- Default-Addressing Mode.
 *	- Power-Of-2 Addressing mode.
 *
 * In Default Addressing mode the Atmel Serial Flash contains approximately 3%
 * more Addressing memory than the Power-Of-2 addressing mode. The addressing
 * mode of the Atmel Serial Flash can be known by reading the Device Status
 * Register The ISF in the Xilinx devices is in Default-Addressing mode by
 * default.
 *
 * The following Atmel flash memories are supported by this library.
 * AT45DB011D		AT45DB021D		AT45DB041D
 * AT45DB081D		AT45DB161D		AT45DB321D
 * AT45DB642D
 *
 * - Intel (Numonyx) (S33) and STM (Numonyx)(M25PXX):
 * The Intel and STM Serial Flash is divided into Sectors. Each Sector consists
 * of multiple pages. Each Page contains 256 Bytes. The Number of Sectors
 * and Pages Per Sectors vary for different devices within this family.
 *
 * The following Intel and STM flash memories are supported by this library.
 * S3316MBIT		S3332MBIT		S3364MBIT
 *
 * M25P05_A		M25P10_A		M25P20
 * M25P40		M25P80			M25P16
 * M25P32		M25P64			M25P128
 *
 * - Winbond W25QXX/W25XX:
 * The Winbond W25QXX/W25XX Serial Flash is divided into Blocks of 64 KB and
 * the blocks are divided into sectors. Each Sector consists of multiple pages.
 * Each Page contains 256 Bytes. The Number of Blocks and Sectors vary for
 * different devices within this family.
 * The following instructions are not supported - Block Erase 32KB, Erase
 * Suspend/Resume, Mode Bit Reset, Read Unique ID, and Read Manufacturer/Device
 * ID.
 *
 * The following Winbond flash memories are supported by this library.
 * W25Q80		W25Q16			W25Q32
 * W25Q64		W25Q128			W25X10
 * W25X20		W25X40			W25X80
 * W25X16		W25X32			W25X64
 *
 * - Numonyx N25QXX
 * The Numonyx N25QXX Serial Flash is divided into sectors of 64 KB and
 * the sectors are divided into sub-sectors. Each Sector consists of multiple
 * pages. Each Page contains 256 Bytes. The Number of Blocks vary for different
 * devices within this family.
 * The following instructions are not supported - Erase Suspend/Resume,
 * Read/Write Volatile/Non-volatile configuration register.
 *
 * The following Numonyx flash memories are supported by this library.
 * N25Q32		N25Q64			N25Q128   N25Q256A
 *
 * - Spansion S25FL
 * The Spansion S25FL Serial Flash is divided into sectors of 64 KB and
 * in devices like S25FL128/129, the sectors are divided into sub-sectors.
 * Each Sector consists of multiple pages. Each Page contains 256 Bytes. The
 * Number of Blocks vary for different devices within this family.
 *
 * The following Spansion flash memories are supported by this library.
 * S25FL004		S25FL008		S25FL016
 * S25FL032		S25FL064		S25FL128/129
 *
 * - Silicon Storage Technology (SST) SST25WF080
 * The SST25WF080 Serial Flash is divided into sectors of 4KB. This flash
 * doesn't support Page Write commands. Supports only Byte-Write Command.
 *
 * Support for new parts can be easily added, when they are available from
 * vendors.
 *
 * <b>Library Initialization</b>
 *
 * The function call XIsf_Initialize() should be called by the application
 * before using  any other function in the library. This function will fetch the
 * Manufacturer code and Device code and determine the geometry of the Serial
 * Flash used.
 *
 * <b>Write Operations</b>
 *
 * The XIsf_Write() API is used to write data to the Serial Flash. A maximum
 * of a Page of data can be written using this API. Once the user initiates a
 * write operation, the Serial Flash takes time to complete the write operation
 * internally. The user has to read the Status Register (XIsf_GetStatus) to
 * know if the Serial Flash is still busy with a previously initiated operation
 * before initiating a new one.
 *
 * Using the XIsf_Write() API the user can perform several different types of
 * write operations as mentioned below:
 *
 * - Normal Write:
 *	Write data to the specified locations in the Serial Flash.
 *	This operation is supported in Atmel, Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Dual Input Fast Program:
 *	This operation is similar to the Normal Write operation, except that
 *	the data is transmitted on two lines (DQ0 and DQ1) instead of one.
 *	This operation is supported in Numonyx (N25QXX) Quad Serial Flash.
 *
 * - Dual Input Extended Fast Program:
 *	This operation is similar to the Dual Input Fast Program, except that
 *	the address is transmitted on two lines (DQ0 and DQ1) instead of one.
 *	This operation is supported in Numonyx (N25QXX) Quad Serial Flash.
 *
 * - Quad Input Fast Program:
 *	This operation is similar to the Dual Input Fast Program, except that
 *	the data is transmitted on four lines (DQ0 - DQ3) instead of two.
 *	This operation is supported in Numonyx (N25QXX), Winbond (W25QXX) and
 *	Spansion (S25FL129) Quad Serial Flash.
 *
 * - Quad Input Extended Fast Program:
 *	This operation is similar to the Quad Input Fast Program, except that
 *	the address is transmitted on four lines (DQ0 - DQ3) instead of one.
 *	This operation is supported in Numonyx (N25QXX) Quad Serial Flash.
 *
 * - Auto Page Write:
 *	Auto rewrite the contents of the page.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Buffer Write:
 *	Write data to the internal SRAM buffer of the Serial Flash.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Buffer To Memory Write With Erase:
 *	Write data from the specified SRAM buffer to a page in the Serial Flash
 *	after erasing the page.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Buffer To Memory Write Without Erase:
 *	Write data from the specified SRAM buffer to a Page in the Serial Flash
 *	without erasing the page.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Write Status Register:
 *	Write to the Status Register of the Serial Flash.
 *	This operation is supported only for Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Write 2 byte Status Register:
 *	Write to the 16-bit Status Register of the Serial Flash.
 *	This operation is supported only for Winbond Serial Flash.
 *
 * - One Time Programmable Area Write:
 *	Write one byte of data in to One Time Programmable area.
 *	This operation is supported only for Intel Serial Flash.
 *
 * For Intel, STM (Numonyx), Winbond and Spansion Serial Flash devices, the user
 * application must call the XIsf_WriteEnable() API by passing XISF_WRITE_ENABLE
 * as an argument before calling the Isf_Write() API.
 *
 *
 * <b>Read Operations</b>
 *
 * The XIsf_Read() API can be used to read a minimum of one byte
 * and a maximum of an entire array of the Serial Flash depending
 * on the type of read operation.
 *
 * Using the XIsf_Read() API the user can perform several
 * different types of read operations as mentioned below:
 *
 * - Normal Read:
 *	Read data from the specified locations in the Serial Flash .
 *	This operation is supported for Atmel, Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Fast Read:
 *	Read a large block of contiguous data from the specified locations of
 *	the Serial Flash at a higher speed than the Normal Read.
 *	This operation is supported for Atmel, Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Dual Output Fast Read:
 *	This operation is similar to the Fast Read, except that the data is
 *	transmitted on two lines (DQ0 and DQ1) instead of one.
 *	This operation is supported in Numonyx (N25QXX), Winbond (W25QXX) and
 *	Spansion (S25FL129) Quad Serial Flash.
 *
 * - Dual Input/Output Fast Read:
 *	This operation is similar to the Dual Output Fast Read, except that
 *	the address is transmitted on two lines (DQ0 and DQ1) instead of one.
 *	This operation is supported in Numonyx (N25QXX), Winbond (W25QXX) and
 *	Spansion (S25FL129) Quad Serial Flash.
 *
 * - Quad Output Fast Read:
 *	This operation is similar to the Dual Output Fast Read, except that
 *	the data is transmitted on four lines (DQ0 - DQ3) instead of two.
 *	This operation is supported in Numonyx (N25QXX), Winbond (W25QXX) and
 *	Spansion (S25FL129) Quad Serial Flash.
 *
 * - Quad Input/Output Fast Read:
 *	This operation is similar to the Quad Output Fast Program, except that
 *	the address is transmitted on four lines (DQ0 - DQ3) instead of one.
 *	This operation is supported in Numonyx (N25QXX), Winbond (W25QXX) and
 *	Spansion (S25FL129) Quad Serial Flash.
 *
 * - Memory To Buffer Transfer:
 *	Transfer a page of data from the Serial Flash to the specified
 *	internal SRAM buffer of the Serial Flash.
 *	This operation is supported only in Atmel Serial Flash.
 *
 * - Buffer Read:
 *	Read data from the specified SRAM internal buffer of the Serial Flash.
 *	This operation is supported only in Atmel Serial Flash.
 *
 * - Fast Buffer Read:
 *	Read multiple contiguous bytes from the internal SRAM page buffer of
 *	Serial Flash at a higher speed than normal Buffer Read.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - One Time Programmable Area Read:
 *	Read One Time Programmable area.
 *	This operation is supported only for Intel Serial Flash.
 *
 * <b>Erase Operations</b>
 *
 * The XIsf_Erase() API can be used to Erase the contents of the Serial Flash.
 * Once the user initiates an Erase operation, the Serial Flash takes time to
 * complete the Erase operation internally. The user has to read the Status
 * Register to know if Serial Flash is still busy with a previously initiated
 * operation before initiating a new one.
 *
 * Using the XIsf_Erase() API the user can perform four different types of Erase
 * operations as mentioned below :
 *
 * - Page Erase:
 *	Erase one Page of the Serial Flash.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Block Erase:
 *	Erase one Block of the Serial Flash.
 *	This operation is supported for Atmel, Intel, Winbond Serial Flash.
 *
 * - Sector Erase:
 *	Erase one Sector of the Serial Flash.
 *	This operation is supported for Atmel, Intel, STM, Spansion and Winbond
 *	Serial Flash.
 *
 * - Bulk Erase:
 *	Erase an entire Serial Flash.
 *	This operation is supported for Intel, STM, Winbond and Spansion Serial
 *	Flash.
 *
 * For Intel, STM, Winbond and Spansion Serial Flash the user application must
 * call the XIsf_WriteEnable() API by passing XISF_WRITE_ENABLE as an argument
 * before calling the XIsf_Erase() API.
 *
 * <b>Sector Protection Operations</b>
 *
 * The XIsf_SectorProtect() API can be used to perform Sector Protection related
 * operations. The Serial Flash is divided into Sectors.
 * Each Sector or number of Sectors can be protected from
 * unwarranted writing/erasing.
 *
 * Using the XIsf_SectorProtect() API the user can perform
 * five different type of operations as given below:
 *
 * - Sector Protect Register Read:
 *	Read Sector Protect Register/Bits in to the buffer provided by user.
 *	This operation is supported for Atmel, Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Sector Protect Register Write:
 *	Write data to the Sector Protect Register/Bits.
 *	This operation is supported for Atmel, Intel, STM, Winbond and Spansion
 *	Serial Flash.
 *
 * - Sector Protect Register Erase:
 *	Erase the Sector Protect Register.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Sector Protect Enable:
 *	Enable Sector Protect mode of Serial Flash.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * - Sector Protect Disable:
 *	Disable Sector Protect mode of Serial Flash.
 *	This operation is supported only for Atmel Serial Flash.
 *
 * For Intel, STM, Winbond and Spansion Serial Flash the user application must
 * call the XIsf_WriteEnable() API by passing XISF_WRITE_ENABLE as an argument
 * before calling the XIsf_SectorProtect() API for Sector Protect Register
 * Write operation.
 *
 * <b>Device Control Operations</b>
 *
 * The XIsf_Ioctl() API can be used to perform control operations on the
 * Intel, STM, Winbond and Spansion Serial Flash.
 *
 * Using the XIsf_Ioctl() API the user can perform several different types of
 * operations as given below:
 *
 * - Release From Deep Power-Down Mode:
 *	This operation releases the Serial Flash from Deep Power-Down Mode.
 *	This operation is supported for Intel, STM, Winbond and Spansion Serial
 *	Flash.
 *
 * - Enter to Deep Power-Down Mode:
 *	This operation puts the Serial Flash in to Deep Power-Down Mode.
 *	In this mode all commands except the release from Deep Power-Down Mode
 *	command will be ignored.
 *	This operation is supported for Intel, STM, Winbond and Spansion Serial
 *	Flash.
 *
 * - Clear Status Register Fail Flag:
 *	This operation clears all the fail flags in the Status Register
 *	of the Serial Flash.
 *	This operation is only supported for Intel Serial Flash.
 *
 * - High Performance Mode:
 *	This instruction must be executed before the dual/quad I/O instructions
 *	in Winbond Flash. This instruction is supported only in Winbond (W25QXX)
 *	Serial Flash.
 *
 * <b>Serial Flash Information Read</b>
 *
 * XIsf_GetDeviceInfo() API is used to read the Joint Electron Device
 * Engineering Council (JEDEC) compatible device information.
 * This JEDEC information consists of Manufacturer ID, Vendor-Specific Device
 * family identifier, Vendor-Specific device identifier
 * for the specified family, number of bits stored per memory
 * cell, product version and number of additional Extended
 * Device Information bytes.
 *
 * Read the Spartan-3AN In-system Flash User Guide and the data sheets of
 * Atmel-AT45XXXD/STM-M25PXX/Intel-S33/Winbond-W25QXX/W25XX/Spansion-S25FLXX for
 * more information.
 *
 * XIsf_GetStatus() API is used to read the Status Register of the Serial Flash.
 * Winbond devices have a Status Register 2 which can be read using the
 * XIsf_GetStatusReg2() API.
 *
 * <b>Write Enable/Disable Operations</b>
 *
 * For Intel, STM, Winbond and Spansion Serial Flash the user application must
 * enable the Write to the Serial Flash by calling XIsf_WriteEnable
 * (XISF_WRITE_ENABLE) API before doing any Write operations to Serial Flash.
 * Writing to the Serial Flash is disabled by calling XIsf_WriteEnable
 * (XISF_WRITE_DISABLE) API.
 *
 * @note
 *
 * This library is intended to be RTOS and processor independent. It works with
 * physical addresses only. Any needs for dynamic memory management, threads,
 * mutual exclusion, virtual memory or cache control must be satisfied by the
 * layer above this driver.
 *
 * This library supports the Spansion (S25FLXX) devices, but this family hasn't
 * been tested. The support for this family of devices is limited to the common
 * commands supported by the other flash families.
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who      Date     Changes
 * ----- -------  -------- -----------------------------------------------
 * 1.00a ksu/sdm  03/03/08 First release
 * 2.00a ktn	 11/27/09 Updated to use HAL processor APIs/definitions
 * 2.01a sdm	 01/04/10 Added Support for Winbond W25Q80/16/32 devices
 * 2.01a sdm	 06/17/10 Updated the Tcl to support axi_spi
 * 2.03a sdm	 04/17/10 Updated to support Winbond memory W25Q128 and added
 *			  a list of supported flash memories
 *			  Updated the Tcl to support axi_quad_spi
 * 2.04a sdm	 08/17/10 Updated to support Numonyx (N25QXX) and Spansion
 *			  flash memories
 * 3.00a srt	 06/20/12 Updated to support interfaces SPI PS and QSPI PS.
 *			  New API:
 *				XIsf_RegisterInterface()
 *				XIsf_SetSpiConfiguration()
 *				XIsf_SetOperatingMode()
 *			  Changed API:
 *				XIsf_Initialize()
 *				XIsf_Transfer()
 *			  Added support to SST flash.
 * 3.01a srt	 02/06/13 Updated for changes made in QSPIPS driver (CR 698107).
 *				      APIs changed:
 *					  XQspiPs_PolledTransfer()
 *					  XQspiPs_Transfer()
 *					  XQspiPs_SetSlaveSelect().
 *			  Modified the examples xilisf_spips_sst_intr_example.c
 *			  and xilisf_spips_sst_polled_example.c to correct
 *			  the flash write, erase and read logic. (CR 703816)
 * 3.02a srt	 04/25/13 - Added Bulk Erase command support for SST and
 *			    Spansion flashes (CR 703816 & 711003).
 *			  - Modified SECTOR and BLOCK Erase commands for
 *			    SST flash and updated spips examples.
 *			    (CR 703816)
 *			  - Updated spips and qspips examples to perform
 *			    Write enable operation in each sector
 *			  - Removed compiler errors when not selecting proper
 *			    interface for Zynq. (CR 716451)
 * 5.0   sb	 08/05/14 - Updated for support for > 128 MB flash for PSQSPI
 *			    Interface.
 *			  - Added Library Handler API which will
 *			    register to driver interrupts, based upon the
 *			    interface selected.
 *			  New API:
 *				GetRealAddr()
 *				SendBankSelect()
 *				XIsf_SetStatusHandler()
 *				XIsf_IfaceHandler()
 * 5.1   sb	 12/23/14 Added check for flash interface for Winbond, Spansion
 *			  and Micron flash family for PSQSPI.
 * 5.2   asa  5/12/15 Added support for Micron N25Q256A (>16MB) flash devices.
 *               This meant adding necessary support for 4 byte addressing mode.
 *               APIs were added to enter and exit from 4 byte mode.
 *               Changes were made in read, erase and write APIs to
 *               support 4 byte mode. These were done to fix CR#858950.
 * 5.3  sk    06/01/15 Used Half of Actual byte count for calculating
 *                     Real Byte count in parallel mode. CR# 859979.
 * 5.3  sk   08/07/17 Added QSPIPSU flash interface support for ZynqMP.
 * 5.4  nsk  09/14/15 Updated IntelStmDevices list in xilisf.c to support
 *                    Micron N25Q256A.CR#881478.
 * 5.5  sk   11/07/15 Removed Compilation warnings with SPI interface.
 *                    CR# 868893.
 * 5.7  rk   27/07/16 Added the SubSector erase option.
 * 5.8  nsk  03/02/17 Update WriteBuffer index to 10 in FastReadData, CR#968476
 * 5.9  nsk  07/11/17 Add 4Byte addressing support for Micron, CR#980169
 *	   tjs	08/09/17 Updated the calculation of RealByteCount for
 *                   reading data from flash with TESTADDRESS being
 *                   0x00000000, CR#981795
 *      ms   08/03/17 Added doxygen tags.
 * 5.10 tjs  11/30/17 Added S25FL-L series flash parts support. CR# 987566
 * 5.10 tjs	03/11/17 Added MT25Q512 3V and 1.8V flash part
 *					support. CR# 995477
 * 5.11 tjs	03/16/18 Added support for ISSI flash part.
 * 5.12	tjs  05/02/18 Added support for IS25LP064A and IS25WP064A.
 * 5.12 tjs	05/21/18 Removed the check for address to be non zero.
 *                    Added check for Spansion flash before proceeding
 *                    to quad mode read CR#1002769
 * 5.12  tjs  06/05/18 Added support for Macronix 1G flash parts. CR#978447
 * 5.12 tjs	 06/18/18 Removed checkpatch and gcc warnings.
 * 5.12 tjs	 07/16/18 Updated notes for Micron flash parts. CR#973229
 * 5.12 tjs	 06/18/18 Added support for low density ISSI
 *			  flash parts. PR#9237
 * 5.12 tjs	 08/13/18 Fixed the compilation warnings for ARMCC (CR#1008307)
 * 5.13 nsk  01/22/19 Make variable declaration to XQspiPsu_Msg as global
 *                    CR#1015808.
 * 5.13 akm	 01/30/19 Fixed multiple definition error in C++ project
 *   			  CR#1019773
 *      sk   02/11/19 Added support for OSPI flash interface.
 * 5.13	akm  02/26/19 Added support for ISSI serial NOR Flash Devices.
 * 		       PR# 11442
 *      sk   02/28/19 Added support for SST26WF016B flash.
 *      sk   02/28/19 Used 3B Sector erase and write commands for QSPI Micron
 *                    flashes.
 * 5.14 akm  08/01/19 Initialized Status variable to XST_FAILURE.
 * 5.14	akm  09/09/19 Added message regarding deprecation of Xilisf.
 *
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef XILISF_H /* prevent circular inclusions */
#define XILISF_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xparameters.h"

#ifdef XPAR_XISF_INTERFACE_AXISPI
#include "xspi.h"
#elif XPAR_XISF_INTERFACE_PSSPI
#include "xspips.h"
#elif XPAR_XISF_INTERFACE_PSQSPI
#include "xqspips.h"
#elif XPAR_XISF_INTERFACE_QSPIPSU
#include "xqspipsu.h"
#elif XPAR_XISF_INTERFACE_OSPIPSV
#include "xospipsv.h"
#endif

/**
 * The following definitions specify the type of Serial Flash family.
 * Based on the Serial Flash family selected, some part of the code is included
 * or excluded in the In-system and Serial Flash Library.
 */
#define ATMEL		1 /**< Atmel family device */
#define INTEL		2 /**< Intel family device */
#define STM		3 /**< STM family device */
#define WINBOND		4 /**< Winbond family device */
#define SPANSION	5 /**< Spansion family device */
#define SST		6 /**< SST family device */

#if (XPAR_XISF_FLASH_FAMILY == ATMEL)
#include "xilisf_atmel.h"
#endif

#if ((XPAR_XISF_FLASH_FAMILY == INTEL) || \
	(XPAR_XISF_FLASH_FAMILY == STM) || \
	(XPAR_XISF_FLASH_FAMILY == WINBOND) ||  \
	(XPAR_XISF_FLASH_FAMILY == SPANSION) || \
	(XPAR_XISF_FLASH_FAMILY == SST))
#include "xilisf_intelstm.h"
#endif

/************************** Constant Definitions *****************************/

/**
 * The following definitions specify the Manufacturer Code for the different
 * families of Serial Flash supported by this library.
 */
#define XISF_MANUFACTURER_ID_ATMEL	0x1F	/**< Atmel device */
#define XISF_MANUFACTURER_ID_INTEL	0x89	/**< Intel device */
#define XISF_MANUFACTURER_ID_STM	0x20	/**< STM device */
#define XISF_MANUFACTURER_ID_WINBOND	0xEF	/**< Winbond device */
#define XISF_MANUFACTURER_ID_SPANSION	0x01	/**< Spansion device */
#define XISF_MANUFACTURER_ID_SST	0xBF	/**< SST device */
#define XISF_MANUFACTURER_ID_MICRON	0x20	/**< Micron device */
#define XISF_MANUFACTURER_ID_ISSI	0x9D	/**< ISSI device */
#define XISF_MANUFACTURER_ID_MACRONIX	0xC2	/**< Macronix device */

#define XISF_MANUFACTURER_ID_MICRON_OCTAL	0x2C	/**< Micron Octal device */

#define XISF_SPANSION_ID_BYTE2_128	0x18
#define XISF_SPANSION_ID_BYTE2_256	0x19
#define XISF_SPANSION_ID_BYTE2_512	0x20
/*Micron*/
#define XISF_MICRON_ID_BYTE2_128	0x18
#define XISF_MICRON_ID_BYTE2_256	0x19
#define XISF_MICRON_ID_BYTE2_512	0x20
#define XISF_MICRON_ID_BYTE2_1G		0x21
#define XISF_MICRON_OCTAL_ID_BYTE2_512	0x1A
/*Winbond*/
#define XISF_WINBOND_ID_BYTE2_128	0x18
/*ISSI*/
#define XISF_ISSI_ID_BYTE2_08		0x14
#define XISF_ISSI_ID_BYTE2_16		0x15
#define XISF_ISSI_ID_BYTE2_32		0x16
#define XISF_ISSI_ID_BYTE2_64		0x17
#define XISF_ISSI_ID_BYTE2_128		0x18
#define XISF_ISSI_ID_BYTE2_256		0x19
#define XISF_ISSI_ID_BYTE2_512		0x1a
/*Macronix*/
#define XISF_MACRONIX_ID_BYTE2_1G	0x1B
#define XISF_MACRONIX_ID_BYTE2_1GU	0x3B

/**
 * The following definitions specify the size of the Serial Flash
 * supported by this library.
 */
#define FLASH_SIZE_128				0x1000000
#define FLASH_SIZE_256				0x2000000
#define FLASH_SIZE_512				0x4000000
#define FLASH_SIZE_1G				0x8000000


#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define READ_FLAG_STATUS_CMD	0x70

#define RD_ID_SIZE		4
#define DIE_ERASE_SIZE	4
#define	DIE_ERASE_CMD	0xC4
#define READ_ID			0x9F

/*
 * QSPI Flash Connection Mode
 */
#define XISF_QSPIPS_CONNECTION_MODE_SINGLE	0
#define XISF_QSPIPS_CONNECTION_MODE_STACKED	1
#define XISF_QSPIPS_CONNECTION_MODE_PARALLEL	2

/*
 * The index for Flash config table
 */
/* Spansion*/
#define SPANSION_INDEX_START		0
#define FLASH_CFG_TBL_SINGLE_128_SP	SPANSION_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_SP	(SPANSION_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_SP	(SPANSION_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_SP	(SPANSION_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_SP	(SPANSION_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_SP	(SPANSION_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_SP	(SPANSION_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_SP	(SPANSION_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_SP	(SPANSION_INDEX_START + 8)

/* Micron */
#define MICRON_INDEX_START		(FLASH_CFG_TBL_PARALLEL_512_SP + 1)
#define FLASH_CFG_TBL_SINGLE_128_MC	MICRON_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_MC	(MICRON_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_MC	(MICRON_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_256_MC	(MICRON_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_256_MC	(MICRON_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_256_MC	(MICRON_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_512_MC	(MICRON_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_512_MC	(MICRON_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_512_MC	(MICRON_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_1GB_MC	(MICRON_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_1GB_MC	(MICRON_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_1GB_MC	(MICRON_INDEX_START + 11)

/* Winbond */
#define WINBOND_INDEX_START		(FLASH_CFG_TBL_PARALLEL_1GB_MC + 1)
#define FLASH_CFG_TBL_SINGLE_128_WB	WINBOND_INDEX_START
#define FLASH_CFG_TBL_STACKED_128_WB	(WINBOND_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_128_WB	(WINBOND_INDEX_START + 2)

/*ISSI*/
#define ISSI_INDEX_START		(FLASH_CFG_TBL_PARALLEL_128_WB + 1)
#define FLASH_CFG_TBL_SINGLE_08_ISSI	ISSI_INDEX_START
#define FLASH_CFG_TBL_STACKED_08_ISSI	(ISSI_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_08_ISSI	(ISSI_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_16_ISSI	(ISSI_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_16_ISSI	(ISSI_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_16_ISSI	(ISSI_INDEX_START + 5)
#define FLASH_CFG_TBL_SINGLE_32_ISSI	(ISSI_INDEX_START + 6)
#define FLASH_CFG_TBL_STACKED_32_ISSI	(ISSI_INDEX_START + 7)
#define FLASH_CFG_TBL_PARALLEL_32_ISSI	(ISSI_INDEX_START + 8)
#define FLASH_CFG_TBL_SINGLE_64_ISSI	(ISSI_INDEX_START + 9)
#define FLASH_CFG_TBL_STACKED_64_ISSI	(ISSI_INDEX_START + 10)
#define FLASH_CFG_TBL_PARALLEL_64_ISSI	(ISSI_INDEX_START + 11)
#define FLASH_CFG_TBL_SINGLE_128_ISSI	(ISSI_INDEX_START + 12)
#define FLASH_CFG_TBL_STACKED_128_ISSI	(ISSI_INDEX_START + 13)
#define FLASH_CFG_TBL_PARALLEL_128_ISSI	(ISSI_INDEX_START + 14)
#define FLASH_CFG_TBL_SINGLE_256_ISSI	(ISSI_INDEX_START + 15)
#define FLASH_CFG_TBL_STACKED_256_ISSI	(ISSI_INDEX_START + 16)
#define FLASH_CFG_TBL_PARALLEL_256_ISSI	(ISSI_INDEX_START + 17)
#define FLASH_CFG_TBL_SINGLE_512_ISSI	(ISSI_INDEX_START + 18)
#define FLASH_CFG_TBL_STACKED_512_ISSI	(ISSI_INDEX_START + 19)
#define FLASH_CFG_TBL_PARALLEL_512_ISSI	(ISSI_INDEX_START + 20)

/* Macronix */
#define MACRONIX_INDEX_START	(FLASH_CFG_TBL_PARALLEL_256_ISSI + 1)
#define FLASH_CFG_TBL_SINGLE_1G_MX		MACRONIX_INDEX_START
#define FLASH_CFG_TBL_STACKED_1G_MX		(MACRONIX_INDEX_START + 1)
#define FLASH_CFG_TBL_PARALLEL_1G_MX	(MACRONIX_INDEX_START + 2)
#define FLASH_CFG_TBL_SINGLE_1GU_MX		(MACRONIX_INDEX_START + 3)
#define FLASH_CFG_TBL_STACKED_1GU_MX	(MACRONIX_INDEX_START + 4)
#define FLASH_CFG_TBL_PARALLEL_1GU_MX	(MACRONIX_INDEX_START + 5)

/*
 * Interrupt or Polling mode of Operation Flags
 */
#define XISF_POLLING_MODE	0
#define XISF_INTERRUPT_MODE	1
/*
 * SPI Options flags
 */
#ifdef XPAR_XISF_INTERFACE_AXISPI
#define XISF_SPI_OPTIONS	(XSP_MASTER_OPTION | \
					XSP_MANUAL_SSELECT_OPTION | \
					XSP_CLK_PHASE_1_OPTION | \
					XSP_CLK_ACTIVE_LOW_OPTION)
#elif XPAR_XISF_INTERFACE_PSQSPI
#define XISF_SPI_OPTIONS	(XQSPIPS_MANUAL_START_OPTION | \
					XQSPIPS_FORCE_SSELECT_OPTION)
#elif XPAR_XISF_INTERFACE_QSPIPSU
#define XISF_SPI_OPTIONS	XQSPIPSU_MANUAL_START_OPTION
#elif XPAR_XISF_INTERFACE_OSPIPSV
#define XISF_SPI_OPTIONS	XOSPIPSV_IDAC_EN_OPTION
#elif XPAR_XISF_INTERFACE_PSSPI
#define XISF_SPI_OPTIONS	(XSPIPS_MASTER_OPTION | \
					XSPIPS_FORCE_SSELECT_OPTION)
#endif

/*
 * PS SPI/QSPI PreScaler Settings
 */
#ifdef XPAR_XISF_INTERFACE_PSQSPI
#define XISF_SPI_PRESCALER	XQSPIPS_CLK_PRESCALE_4
#elif XPAR_XISF_INTERFACE_QSPIPSU
#define XISF_SPI_PRESCALER	XQSPIPSU_CLK_PRESCALE_4
#elif XPAR_XISF_INTERFACE_PSSPI
#define XISF_SPI_PRESCALER	XSPIPS_CLK_PRESCALE_8
#elif XPAR_XISF_INTERFACE_OSPIPSV
#define XISF_SPI_PRESCALER	XOSPIPSV_CLK_PRESCALE_12
#elif XPAR_XISF_INTERFACE_AXISPI
#define XISF_SPI_PRESCALER	0
#endif

/**
 * The following definitions determines the type of Write operation to be
 * performed on the Serial Flash.
 */
typedef enum {
	XISF_WRITE,				/**< Normal write operation */
#ifdef XPAR_XISF_INTERFACE_OSPIPSV
	XISF_WRITE_VOLATILE_CONFIG_REG,	/**< Write volatile config Register */
#else
	XISF_AUTO_PAGE_WRITE,			/**< Auto rewrite the contents
						  *  of the page
						  */
	XISF_BUFFER_WRITE,			/**< Write data to the internal
						  *  SRAM buffer of Flash
						  */
	XISF_BUF_TO_PAGE_WRITE_WITH_ERASE,	/**< Erase the specified Page
						  *  then Write data to Flash
						  *  from the internal SRAM
						  *  buffer
						  */
	XISF_BUF_TO_PAGE_WRITE_WITHOUT_ERASE,	/**< Write data to the Flash
						  *  from the internal SRAM
						  *  buffer
						  */
	XISF_WRITE_STATUS_REG,	/**< Write to the Status Register */
	XISF_OTP_WRITE,				/**< Write one byte of data in
						  *  to the One Time
						  *  Programmable area
						  */
	XISF_WRITE_STATUS_REG2,			/**< Write to the 2 byte Status
						  * Register in W25QXX flash
						  */
#if (((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == STM) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION)) && \
	(!defined(XPAR_XISF_INTERFACE_OSPIPSV)))
	XISF_QUAD_IP_PAGE_WRITE, /**< Quad input fast page write */
#endif
#endif
/**
 * ((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
 * (XPAR_XISF_FLASH_FAMILY == STM) || \
 * (XPAR_XISF_FLASH_FAMILY == SPANSION))
 */

#if (XPAR_XISF_FLASH_FAMILY == STM)
	XISF_DUAL_IP_PAGE_WRITE,	/**< Dual input fast page write*/
	XISF_DUAL_IP_EXT_PAGE_WRITE,	/**< Dual input extended fast
					  *  page write
					  */
	XISF_QUAD_IP_EXT_PAGE_WRITE,	/**< Dual input extended fast
					  *  page write
					  */
#endif /* (XPAR_XISF_FLASH_FAMILY == STM) */
} XIsf_WriteOperation;

/**
 * The following definitions determines the type of Read operations to be
 * performed on the Serial Flash.
 */
typedef enum {
#ifdef XPAR_XISF_INTERFACE_OSPIPSV
	XISF_OCTAL_IO_FAST_READ,
	XISF_READ_VCR,
#else
	XISF_READ,			/**< Normal Read operation */
	XISF_FAST_READ,			/**< Fast Read operation */
	XISF_PAGE_TO_BUF_TRANS,		/**< Transfer data from Flash memory to
					  *  internal SRAM buffer of Flash
					  */
	XISF_BUFFER_READ,		/**< Read data from SRAM internal
					  * buffer of the Flash
					  */
	XISF_FAST_BUFFER_READ,	/**< Fast SRAM buffer read operation on Flash */
	XISF_OTP_READ,		/**< Read One Time Programmable area */

#if ((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
	(XPAR_XISF_FLASH_FAMILY == STM) || \
	(XPAR_XISF_FLASH_FAMILY == SPANSION))
	XISF_DUAL_OP_FAST_READ,	/**< Dual output fast read */
	XISF_DUAL_IO_FAST_READ,	/**< Dual input/output fast read */
	XISF_QUAD_OP_FAST_READ,	/**< Quad output fast read */
	XISF_QUAD_IO_FAST_READ,	/**< Quad input/output fast read */
#endif
#endif
/**
 * ((XPAR_XISF_FLASH_FAMILY == WINBOND) || \
 * (XPAR_XISF_FLASH_FAMILY == STM)) || \
 * (XPAR_XISF_FLASH_FAMILY == SPANSION)
 */
} XIsf_ReadOperation;

/**
 * The following definitions determines the type of Erase operation to be
 * performed on the Serial Flash.
 */
typedef enum {
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
	XISF_PAGE_ERASE,	/**< Page Erase operation */
	XISF_BLOCK_ERASE,	/**< Block Erase operation */
	XISF_SUB_SECTOR_ERASE,	/**< Sub Sector Erase operation */
#endif
	XISF_SECTOR_ERASE,	/**< Sector Erase operation */
	XISF_BULK_ERASE		/**< Erase an entire Flash */
} XIsf_EraseOperation;

/**
 * The following definitions determines the type of Sector protection
 * operations to be performed on the Serial Flash.
 */
typedef enum {
	XISF_SPR_READ,		/**< Sector protect register read */
	XISF_SPR_WRITE,		/**< Sector protect register write */
	XISF_SPR_ERASE,		/**< Sector protect register erase */
	XISF_SP_ENABLE,		/**< Sector protect enable */
	XISF_SP_DISABLE		/**< Sector protect disable */
} XIsf_SpOperation;

/**
 * The following definitions determines the type of control operations to be
 * performed on the Serial Flash.
 */
typedef enum  {
	XISF_IOCTL_RELEASE_DPD,		/**< Release from Deep Power-down */
	XISF_IOCTL_ENTER_DPD,		/**< Enter in to Deep Power-down mode */
	XISF_IOCTL_CLEAR_SR_FAIL_FLAGS,	/**< Clear Status Register Fail Flags */
	XISF_IOCTL_ENABLE_HI_PERF_MODE
	/**< Enable high performance mode
	 *  (available in Winbond quad flash
	 *  (W25Q))
	 */
} XIsf_IoctlOperation;

/**************************** Type Definitions *******************************/

#ifdef XPAR_XISF_INTERFACE_AXISPI
typedef XSpi XIsf_Iface;
#elif XPAR_XISF_INTERFACE_PSSPI
typedef XSpiPs XIsf_Iface;
#elif XPAR_XISF_INTERFACE_PSQSPI
typedef XQspiPs XIsf_Iface;
#elif XPAR_XISF_INTERFACE_QSPIPSU
typedef XQspiPsu XIsf_Iface;
#elif XPAR_XISF_INTERFACE_OSPIPSV
typedef XOspiPsv XIsf_Iface;
#endif
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
typedef void (*XIsf_StatusHandler) (void *CallBackRef, u32 StatusEvent,
					unsigned int ByteCount);
#else
typedef void (*XIsf_StatusHandler) (void *CallBackRef, u32 StatusEvent);
#endif

/**
 * The following definition specifies the instance structure of the Serial
 * Flash.
 */
typedef struct {
	u8 IsReady;		/**< Is Device is ready for operation */

	u16 BytesPerPage;	/**< Number of Bytes per Page */
	u16 PagesPerBlock;	/**< This is Number of Pages per block,
				  * for Atmel and it is Number of
				  * Pages per sector for
				  * Intel/STM/Winbond/Spansion
				  */
	u16 BlocksPerSector;	/**< Number of Blocks per Sector, this is valid
				  *  for ATMEL devices
				  */
	u16 NumOfSectors;	/**< Number of Sectors in Serial Flash */
	u8 AddrMode;		/**< Type of addressing in Atmel Serial Flash
				  *  0 - Default/Normal Addressing Mode
				  *  1 - Power-Of-2 Addressing Mode
				  */
	u16 DeviceCode;		/**< The Serial Flash Device Code */
#if defined(XPAR_XISF_INTERFACE_PSQSPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU) || \
	defined(XPAR_XISF_INTERFACE_OSPIPSV)
	u8 DeviceIDMemSize;	/**< Byte of device ID indicating the memory
				  *   size
				  */
	u8 NumDie;		/**< No. of die forming a single flash */
	u32 SectorSize;		/**< Size of the Sector */
	u32 NumSectors;		/**< No. of sectors */
#endif
	u32 ManufacturerID;	/**< Serial Flash Manufacturer ID */
	XIsf_Iface *SpiInstPtr;	/**< SPI Device Instance pointer */
	u32 SpiSlaveSelect;	/**< SPI Slave select for the Serial Flash */
	u8 *WriteBufPtr;	/**< Pointer to Write Buffer */

	u16 ByteMask;
	/**< Mask used for Address translation in Atmel
	 *  devices
	 */
	u8 RegDone;		/**< Registration Done flag */
	u8 IntrMode;		/**< Operating Mode flag Interrupt/Polling */
	u8 FourByteAddrMode; /**< In four byte address mode flag */

#ifdef XPAR_XISF_INTERFACE_OSPIPSV
	u32 (*XIsf_Iface_SetOptions)
			(XIsf_Iface *InstancePtr, u32 Options);
#else
	s32 (*XIsf_Iface_SetOptions)
		(XIsf_Iface *InstancePtr, u32 Options);
#endif
#if defined(XPAR_XISF_INTERFACE_OSPIPSV)
	u32 (*XIsf_Iface_SetSlaveSelect)
		(XIsf_Iface *InstancePtr, u8 ChipSelect);
#elif (!defined(XPAR_XISF_INTERFACE_PSQSPI)) && \
	(!defined(XPAR_XISF_INTERFACE_QSPIPSU))
	s32 (*XIsf_Iface_SetSlaveSelect)
		(XIsf_Iface *InstancePtr, u8 SlaveMask);
#else
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	void (*XIsf_Iface_SetSlaveSelect)
		(XIsf_Iface *InstancePtr, u8 FlashCS, u8 FlashBus);
#else
	int (*XIsf_Iface_SetSlaveSelect)
		(XIsf_Iface *InstancePtr);
#endif
#endif
	int (*XIsf_Iface_Start)
		(XIsf_Iface *InstancePtr);
#ifdef XPAR_XISF_INTERFACE_QSPIPSU
	int (*XIsf_Iface_Transfer)
		(XIsf_Iface *InstancePtr, XQspiPsu_Msg * Msg,
			u32 NumMsg);
	int (*XIsf_Iface_PolledTransfer)
		(XIsf_Iface *InstancePtr, XQspiPsu_Msg *Msg,
			u32 NumMsg);
#elif defined(XPAR_XISF_INTERFACE_OSPIPSV)
	u32 (*XIsf_Iface_Transfer)
		(XIsf_Iface *InstancePtr, XOspiPsv_Msg * Msg);
	u32 (*XIsf_Iface_PolledTransfer)
		(XIsf_Iface *InstancePtr, XOspiPsv_Msg *Msg);
#else
	s32 (*XIsf_Iface_Transfer)
		(XIsf_Iface *InstancePtr, u8 *SendBufPtr,
			u8 *RecvBufPtr, u32 ByteCount);
	s32 (*XIsf_Iface_PolledTransfer)
		(XIsf_Iface *InstancePtr, u8 *SendBufPtr,
			u8 *RecvBufPtr, u32 ByteCount);
#endif
#ifdef XPAR_XISF_INTERFACE_OSPIPSV
	u32 (*XIsf_Iface_SetClkPrescaler)
			(const XIsf_Iface *InstancePtr, u8 PreScaler);
#else
	s32 (*XIsf_Iface_SetClkPrescaler)
		(XIsf_Iface *InstancePtr, u8 PreScaler);
#endif
	XIsf_StatusHandler StatusHandler;
} XIsf;

/**
 * The following structure definition specifies the operational parameters to be
 * passed to the XIsf_Write API while performing Normal write (XISF_WRITE) and
 * OTP write (XISF_OTP_WRITE) operations.
 */
typedef struct {
	u32 Address;		/**< Address in the Serial Flash */
	u8 *WritePtr;		/**< Pointer to the data to be written to the
				  *  Serial Flash
				  */
	u32 NumBytes;		/**< Number of bytes to be written to the Serial
				  *  Flash
				  */
} XIsf_WriteParam;


/**
 * The following structure definition specifies the operational parameters to be
 * passed to the XIsf_Read API while performing Normal Read (XISF_READ),
 * Fast Read (XISF_FAST_READ) and OTP Read (XISF_OTP_READ) operations .
 */
typedef struct {
	u32 Address;		/**< Start address in the Serial Flash */
	u8 *ReadPtr;		/**< Read buffer pointer where data needs to be
				  * stored
				  */
	u32 NumBytes;		/**< Number of bytes to read */
	int NumDummyBytes;	/**< Number of dummy bytes associated with the
				  * fast read command. Valid only for dual o/p
				  * fast read, dual i/o fast read, quad o/p
				  * fast read and quad i/o fast read
				  */
} XIsf_ReadParam;

/**
 * The following structure definition specifies the operational parameters to
 * be passed to the XIsf_Write API while writing data to the internal SRAM
 * buffer of the Atmel Serial Flash (XISF_BUFFER_WRITE).
 */
typedef struct {
	u8 BufferNum;		/**< SRAM buffer number of Serial Flash */
	u8 *WritePtr;		/**< Pointer to the data to be written into the
				  *  Serial Flash SRAM Buffer
				  */
	u32 ByteOffset;		/**< Byte offset in the buffer from which the
				  *  data is written
				  */
	u32 NumBytes;		/**< Number of bytes to be written into the
				  *  buffer
				  */
} XIsf_BufferWriteParam;

/**
 * The following structure definition specifies the operational parameters to be
 * passed to the XIsf_Write API while writing data from the internal SRAM buffer
 * to a Page in the Atmel Serial Flash using XISF_BUF_TO_PAGE_WRITE_WITH_ERASE /
 * XISF_BUF_TO_PAGE_WRITE_WITHOUT_ERASE commands in Atmel Serial Flash.
 */
typedef struct {
	u8 BufferNum;		/**< SRAM buffer number of Serial Flash */
	u32 Address;		/**< Starting address in the Serial Flash */
} XIsf_BufferToFlashWriteParam;

/**
 * The following structure definition specifies the operational parameters to be
 * passed to the XIsf_Read API while performing Page to Buffer Transfer
 * operation (XISF_PAGE_TO_BUF_TRANS) in Atmel Serial Flash.
 */
typedef struct {
	u8 BufferNum;		/**< SRAM buffer number of Serial Flash */
	u32 Address;		/**< Start address in the Serial Flash */
} XIsf_FlashToBufTransferParam;

/**
 * The following structure definition specifies operational parameters to be
 * passed to the XIsf_Read API while reading data from the Internal SRAM buffer
 * of Flash using XISF_BUFFER_READ or XISF_FAST_BUFFER_READ commands in Atmel
 * Serial Flash.
 */
typedef struct {
	u8 BufferNum;		/**< SRAM buffer number of Serial Flash */
	u8 *ReadPtr;		/**< Read buffer pointer where data read from
				  *  the SRAM buffer is stored
				  */
	u32 ByteOffset;		/**< Byte offset in the SRAM buffer from where
				  *  the first byte is read
				  */
	u32 NumBytes;		/**< Number of bytes to be read from the
				  *  buffer
				  */
} XIsf_BufferReadParam;


/************************** Variable Declaration *****************************/

extern u32 XIsf_StatusEventInfo;
extern unsigned int XIsf_ByteCountInfo;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This API sets the interrupt/polling mode of transfer.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 * @param	Mode is the value to be set.
 *
 * @note		By default, the xilisf library is designed to operate in
 *		polling mode.  User needs to call this API to set in interrupt
 *		mode, if operating in Interrupt Mode.
 ******************************************************************************/
static INLINE void XIsf_SetTransferMode(XIsf *InstancePtr, u8 Mode)
{
	InstancePtr->IntrMode = Mode;
}

/*****************************************************************************/
/**
 *
 * This API gets the interrupt/polling mode of transfer.
 *
 * @param	InstancePtr is a pointer to the XIsf instance.
 *
 * @note		(shakti)
 ******************************************************************************/
static INLINE u8 XIsf_GetTransferMode(XIsf *InstancePtr)
{
	return InstancePtr->IntrMode;
}
/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/

/*
 * Initialization Function.
 */
int XIsf_Initialize(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr, u8 SlaveSelect,
				u8 *WritePtr);

/*
 * Function to read the Status Registers.
 */
int XIsf_GetStatus(XIsf *InstancePtr, u8 *ReadPtr);

#if (XPAR_XISF_FLASH_FAMILY == WINBOND)
int XIsf_GetStatusReg2(XIsf *InstancePtr, u8 *ReadPtr);
#endif


/*
 * Function to read the Serial Flash information.
 */
int XIsf_GetDeviceInfo(XIsf *InstancePtr, u8 *ReadPtr);

/*
 * Function to transfer data to and fro flash devices
 * interfaced.
 */

int XIsf_Transfer(XIsf *InstancePtr, u8 *WritePtr,
		u8 *ReadPtr, u32 ByteCount);

/**
 * Function to get the real address of flash
 * in case dual parallel and stacked configuration.
 */
u32 GetRealAddr(XIsf_Iface *QspiPtr, u32 Address);

/*
 * Function for Writing to the Serial Flash.
 */
int XIsf_Write(XIsf *InstancePtr, XIsf_WriteOperation Operation,
		void *OpParamPtr);

/*
 * Function for Reading from the Serial Flash.
 */
int XIsf_Read(XIsf *InstancePtr, XIsf_ReadOperation Operation,
		void *OpParamPtr);

/*
 * Function for Erasing the Serial Flash.
 */
int XIsf_Erase(XIsf *InstancePtr, XIsf_EraseOperation Operation, u32 Address);

#if ((XPAR_XISF_FLASH_FAMILY == SPANSION) && \
	(defined(XPAR_XISF_INTERFACE_AXISPI) || \
	defined(XPAR_XISF_INTERFACE_QSPIPSU) || \
	defined(XPAR_XISF_INTERFACE_OSPIPSV)))
/*
 * Function for entering into 4 byte mode for Micron flash.
 */
int XIsf_MicronFlashEnter4BAddMode(XIsf *InstancePtr);

/*
 * Function for exiting from 4 byte mode for Micron flash.
 */
int XIsf_MicronFlashExit4BAddMode(XIsf *InstancePtr);
#endif

#ifdef XPAR_XISF_INTERFACE_OSPIPSV
u32 XIsf_Get_ProtoType(XIsf *InstancePtr, int Read);
#endif

/*
 * Function related to Sector protection.
 */
int XIsf_SectorProtect(XIsf *InstancePtr, XIsf_SpOperation Operation,
			u8 *BufferPtr);

/*
 * Function to perform different control operations.
 */
int XIsf_Ioctl(XIsf *InstancePtr, XIsf_IoctlOperation Operation);

/*
 * Function for Enabling/Disabling Write to Intel, STM, Winbond and Spansion
 * Serial Flash.
 */
int XIsf_WriteEnable(XIsf *InstancePtr, u8 WriteEnable);

/*
 * Function for Registering Interfaces SPI, PSQSPI, PSSPI
 */
void XIsf_RegisterInterface(XIsf *InstancePtr);

/*
 * Function for setting SPI/PSQSPI/PSSPI Configuration
 */
int XIsf_SetSpiConfiguration(XIsf *InstancePtr, XIsf_Iface *SpiInstPtr,
				u32 Options, u8 PreScaler);

/*
 *Interrupt Status Handler of XilIsf Lib
 */
void XIsf_SetStatusHandler(XIsf *InstancePtr, XIsf_Iface *XIfaceInstancePtr,
				XIsf_StatusHandler XilIsf_Handler);

/*
 *Interrupt Handler of XilISF Lib
 */
#ifndef XPAR_XISF_INTERFACE_OSPIPSV
void XIsf_IfaceHandler(void *CallBackRef, u32 StatusEvent,
		unsigned int ByteCount);
#else
void XIsf_IfaceHandler(void *CallBackRef, u32 StatusEvent);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* end of protection macro */
/* @} */
