/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* @file xmegbl.h
* @{
*
* @details
*
* Math Engine (ME) Run Time Software (RTS) Driver is a congregation of
* software based APIs for the PS application to configure/initialize/program
* the ME array and shim.
*
* The layered software stack of the ME RTS driver is shown below :
*
*        -------------------------------------------------------
*        |   Application layer (Cardano interface/PS app call) |
*        -------------------------------------------------------
*                                   |
*                                   |
*        -------------------------------------------------------
*        |                    ME RTS driver                    |
*        -------------------------------------------------------
*                                   |
*                                   |
*        -------------------------------------------------------
*        |               ME simulator / LibMetal               |
*        -------------------------------------------------------
*
* The Driver APIs are typically invoked by the Cardano tool flow as part of the
* PS application generation. The APIs can also be directly invoked by the user
* application for performing any runtime configuration like partial
* reconfiguration etc.
*
* ME driver is partitioned into the following sub-components :
*	- ME Global Driver
*		- APIs for device/configuration lookup, global driver
*		  instantiation and initialization and generic APIs commonly
*		  used by other components
*	- ME Tile Driver
*		- APIs for configuring the Tile resources: Core, Program memory,
*		  Data memory, Locks, Stream switch, Events
*		- APIs for configuring the Shim resources: NoC and PL
*		  interfaces, Stream switch, Events
*	- ME DMA Driver
*		- APIs for configuring the Tile DMA S2MM and MM2S channels
*		- APIs for configuring Shim DMA S2MM,MM2S channels and Locks
*       - ME low level library layer
*               - APIs to interface with the underlying target platform
*
* ME driver will be common for all the Math Engine devices. So in the similar
* lines of xparameters.h for other standalone drivers, ME driver also needs
* the following device related parameters :
*	- ME Array base address
*	- Number of Rows
*	- Number of Columns
*	- Interrupt IDs
*
* These parameters can be passed to the ME driver in the similar lines of any
* other standalone driver i.e., in the form of compile time parameters from
* xparameters.h or in the form of a runtime configuration object etc.
*
* ME RTS driver provides a suite of APIs to be invoked by the application layer
* and also a low level interface abstraction for the driver APIs to interface
* with the low level target specific APIs. These low level target APIs are in
* turn used to talk to the underlying target platform with typical AXI read and
* write transactions.
*
* Following are the features of the ME RTS driver APIs :
*	- Non re-entrant and not thread safe
*	- Implemented as bare-metal
*	- Implemented as blocking calls
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  04/12/2018  Code changed to fix CR#999685
* 1.2  Naresh  05/23/2018  Updated code to fix CR#999693
* 1.3  Naresh  06/18/2018  Updated code as per standalone driver framework
* 1.4  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/

#ifndef XMEGBL_H /* prevent circular inclusions */
#define XMEGBL_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xparameters_me.h"
#include "xmegbl_params.h"
#include "xmegbl_defs.h"

/************************** Constant Definitions *****************************/
#define XMEGBL_TILE_ADDR_ARR_SHIFT	        30U
#define XMEGBL_TILE_ADDR_ROW_SHIFT	        18U
#define XMEGBL_TILE_ADDR_COL_SHIFT	        23U
#define XMEGBL_TILE_BASE_ADDRMASK	        0x3FFFFFFFU

#define XMEGBL_TILE_ADDR_MEMMODOFF	        0U
#define XMEGBL_TILE_ADDR_COREMODOFF	        0x20000U
#define XMEGBL_TILE_ADDR_NOCMODOFF	        0U
#define XMEGBL_TILE_ADDR_PLMODOFF	        0x31000U

#define XMEGBL_TILE_ADDR_MEMLOCKOFF	        0x1E000U
#define XMEGBL_TILE_ADDR_NOCLOCKOFF	        0x14000U

#define XMEGBL_TILE_ADDR_CORESTRMOFF	        0x1F000U
#define XMEGBL_TILE_ADDR_PLSTRMOFF	        0xE000U

#define XMEGBL_TILE_TYPE_METILE		        0U
#define XMEGBL_TILE_TYPE_SHIMNOC	        1U
#define XMEGBL_TILE_TYPE_SHIMPL		        2U

#define XMEGBL_TILE_LOCK_NUM_MAX	        16U

#define XMEGBL_TILE_PLIF_ME2PL_MAX_STRMS	6U
#define XMEGBL_TILE_PLIF_ME2PL_MAX_STRMS128	3U
#define XMEGBL_TILE_PLIF_PL2ME_MAX_STRMS	8U
#define XMEGBL_TILE_PLIF_PL2ME_MAX_STRMS128	4U

#define XMEGBL_TILE_PLIF_PL2ME_MAX_BYPASS_STRMS	6U

#define XMEGBL_HWCFG_SET_CONFIG(cfgptr, rows, cols, arrayoff)  cfgptr->NumRows = rows;\
                                                                cfgptr->NumCols = cols;\
                                                                cfgptr->ArrayOff = arrayoff

/**************************** Type Definitions *******************************/
/**
 * This typedef contains configuration information of the tiles.
 */
typedef struct XMeGbl_Tile
{
	u16 RowId;		/**< Row index of the tile in the ME array */
	u16 ColId;		/**< Column index of the tile  in the ME array */
	u8 TileType;		/**< ME tile or Shim tile */
	u64 TileAddr;		/**< 48-bit Tile base address */
	u64 MemModAddr;		/**< 48-bit Memory module base address */
	u64 CoreModAddr;	/**< 48-bit Core module base address */
	u64 NocModAddr;		/**< 48-bit NoC module base address */
	u64 PlModAddr;		/**< 48-bit PL module base address */
	u64 LockAddr;		/**< 48-bit Lock config base address */
	u64 StrmSwAddr;		/**< 48-bit Stream switch config base address */
	u8 IsReady;		/**< Tile is initialized and ready */
	void *Private;		/**< Private data */
} XMeGbl_Tile;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
	u16 DeviceId;		/**< Unique ID of the ME device */
	u32 ArrOffset;		/**< ME array address offset in the system address map */
	u16 NumRows;		/**< Total number of rows including shim */
	u16 NumCols;		/**< Total number of columns */
} XMeGbl_Config;

/**
 * The XMe driver instance data. The user is required to allocate a
 * variable of this type for the ME instance.
 */
typedef struct
{
	XMeGbl_Config *Config;  /**< Configuration table entry for the ME device */
	u32 IsReady;		/**< Device is initialized and ready */
} XMeGbl;

/**
 * This typedef contains the HW configuration data for the ME array.
 */
typedef struct {
        u8 NumRows;             /**< Number of rows */
        u8 NumCols;             /**< Number of columns */
        u32 ArrayOff;           /**< ME array address offset in the system address map */
} XMeGbl_HwCfg;

/**************************** Macro Definitions *****************************/

/**************************** Function prototypes ***************************/
void XMeGbl_HwInit(XMeGbl_HwCfg *CfgPtr);
void XMeGbl_CfgInitialize(XMeGbl *InstancePtr, XMeGbl_Tile *TileInstPtr, XMeGbl_Config *ConfigPtr);
XMeGbl_Config *XMeGbl_LookupConfig(u16 DeviceId);

#endif            /* end of protection macro */
/** @} */
