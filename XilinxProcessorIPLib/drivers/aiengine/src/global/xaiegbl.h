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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiegbl.h
* @{
*
* @details
*
* Math Engine (ME) Run Time Software (RTS) Driver is a congregation of
* software based APIs for the PS application to configure/initialize/program
* the AIE array and shim.
*
* The layered software stack of the AIE RTS driver is shown below :
*
*        -------------------------------------------------------
*        |   Application layer (Cardano interface/PS app call) |
*        -------------------------------------------------------
*                                   |
*                                   |
*        -------------------------------------------------------
*        |                    AIE RTS driver                    |
*        -------------------------------------------------------
*                                   |
*                                   |
*        -------------------------------------------------------
*        |               AIE simulator / LibMetal               |
*        -------------------------------------------------------
*
* The Driver APIs are typically invoked by the Cardano tool flow as part of the
* PS application generation. The APIs can also be directly invoked by the user
* application for performing any runtime configuration like partial
* reconfiguration etc.
*
* AIE driver is partitioned into the following sub-components :
*	- AIE Global Driver
*		- APIs for device/configuration lookup, global driver
*		  instantiation and initialization and generic APIs commonly
*		  used by other components
*	- AIE Tile Driver
*		- APIs for configuring the Tile resources: Core, Program memory,
*		  Data memory, Locks, Stream switch, Events
*		- APIs for configuring the Shim resources: NoC and PL
*		  interfaces, Stream switch, Events
*	- AIE DMA Driver
*		- APIs for configuring the Tile DMA S2MM and MM2S channels
*		- APIs for configuring Shim DMA S2MM,MM2S channels and Locks
*       - AIE low level library layer
*               - APIs to interface with the underlying target platform
*
* AIE driver will be common for all the Math Engine devices. So in the similar
* lines of xparameters.h for other standalone drivers, AIE driver also needs
* the following device related parameters :
*	- AIE Array base address
*	- Number of Rows
*	- Number of Columns
*	- Interrupt IDs
*
* These parameters can be passed to the AIE driver in the similar lines of any
* other standalone driver i.e., in the form of compile time parameters from
* xparameters.h or in the form of a runtime configuration object etc.
*
* AIE RTS driver provides a suite of APIs to be invoked by the application layer
* and also a low level interface abstraction for the driver APIs to interface
* with the low level target specific APIs. These low level target APIs are in
* turn used to talk to the underlying target platform with typical AXI read and
* write transactions.
*
* Following are the features of the AIE RTS driver APIs :
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
* 1.5  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

#ifndef XAIEGBL_H /* prevent circular inclusions */
#define XAIEGBL_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xparameters_aie.h"
#include "xaiegbl_params.h"
#include "xaiegbl_defs.h"

/************************** Constant Definitions *****************************/
#define XAIEGBL_TILE_ADDR_ARR_SHIFT	        30U
#define XAIEGBL_TILE_ADDR_ROW_SHIFT	        18U
#define XAIEGBL_TILE_ADDR_COL_SHIFT	        23U
#define XAIEGBL_TILE_BASE_ADDRMASK	        0x3FFFFFFFU

#define XAIEGBL_TILE_ADDR_MEMMODOFF	        0U
#define XAIEGBL_TILE_ADDR_COREMODOFF	        0x20000U
#define XAIEGBL_TILE_ADDR_NOCMODOFF	        0U
#define XAIEGBL_TILE_ADDR_PLMODOFF	        0x31000U

#define XAIEGBL_TILE_ADDR_MEMLOCKOFF	        0x1E000U
#define XAIEGBL_TILE_ADDR_NOCLOCKOFF	        0x14000U

#define XAIEGBL_TILE_ADDR_CORESTRMOFF	        0x1F000U
#define XAIEGBL_TILE_ADDR_PLSTRMOFF	        0xE000U

#define XAIEGBL_TILE_TYPE_AIETILE		        0U
#define XAIEGBL_TILE_TYPE_SHIMNOC	        1U
#define XAIEGBL_TILE_TYPE_SHIMPL		        2U

#define XAIEGBL_TILE_LOCK_NUM_MAX	        16U

#define XAIEGBL_TILE_PLIF_AIE2PL_MAX_STRMS	6U
#define XAIEGBL_TILE_PLIF_AIE2PL_MAX_STRMS128	3U
#define XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS	8U
#define XAIEGBL_TILE_PLIF_PL2AIE_MAX_STRMS128	4U

#define XAIEGBL_TILE_PLIF_PL2AIE_MAX_BYPASS_STRMS	6U

#define XAIEGBL_HWCFG_SET_CONFIG(cfgptr, rows, cols, arrayoff)  cfgptr->NumRows = rows;\
                                                                cfgptr->NumCols = cols;\
                                                                cfgptr->ArrayOff = arrayoff

/**************************** Type Definitions *******************************/
/**
 * This typedef contains configuration information of the tiles.
 */
typedef struct XAieGbl_Tile
{
	u16 RowId;		/**< Row index of the tile in the AIE array */
	u16 ColId;		/**< Column index of the tile  in the AIE array */
	u8 TileType;		/**< AIE tile or Shim tile */
	u64 TileAddr;		/**< 48-bit Tile base address */
	u64 MemModAddr;		/**< 48-bit Memory module base address */
	u64 CoreModAddr;	/**< 48-bit Core module base address */
	u64 NocModAddr;		/**< 48-bit NoC module base address */
	u64 PlModAddr;		/**< 48-bit PL module base address */
	u64 LockAddr;		/**< 48-bit Lock config base address */
	u64 StrmSwAddr;		/**< 48-bit Stream switch config base address */
	u8 IsReady;		/**< Tile is initialized and ready */
	void *Private;		/**< Private data */
} XAieGbl_Tile;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
	u16 DeviceId;		/**< Unique ID of the AIE device */
	u32 ArrOffset;		/**< AIE array address offset in the system address map */
	u16 NumRows;		/**< Total number of rows including shim */
	u16 NumCols;		/**< Total number of columns */
} XAieGbl_Config;

/**
 * The XAie driver instance data. The user is required to allocate a
 * variable of this type for the AIE instance.
 */
typedef struct
{
	XAieGbl_Config *Config;  /**< Configuration table entry for the AIE device */
	u32 IsReady;		/**< Device is initialized and ready */
} XAieGbl;

/**
 * This typedef contains the HW configuration data for the AIE array.
 */
typedef struct {
        u8 NumRows;             /**< Number of rows */
        u8 NumCols;             /**< Number of columns */
        u32 ArrayOff;           /**< AIE array address offset in the system address map */
} XAieGbl_HwCfg;

/**************************** Macro Definitions *****************************/

/**************************** Function prototypes ***************************/
void XAieGbl_HwInit(XAieGbl_HwCfg *CfgPtr);
void XAieGbl_CfgInitialize(XAieGbl *InstancePtr, XAieGbl_Tile *TileInstPtr, XAieGbl_Config *ConfigPtr);
XAieGbl_Config *XAieGbl_LookupConfig(u16 DeviceId);

#endif            /* end of protection macro */
/** @} */
