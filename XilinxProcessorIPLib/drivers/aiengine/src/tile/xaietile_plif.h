/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_plif.h
* @{
*
* Header file for PL interface configuration functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Jubaer  03/07/2019  Add pl if downsizer disable API
* </pre>
*
******************************************************************************/
#ifndef XAIETILE_PLIF_H
#define XAIETILE_PLIF_H

/***************************** Include Files *********************************/

/***************************** Constant Definitions **************************/
#define XAIETILE_PLIF_PLTOAIE_STRMS_ENABLE	1U

#define XAIETILE_PLIF_STRM_WIDTH32		32U
#define XAIETILE_PLIF_STRM_WIDTH64		64U
#define XAIETILE_PLIF_STRM_WIDTH128		128U

#define XAIETILE_PLIF_STRM_WIDTH128_IDDIV	2U
#define XAIETILE_PLIF_STRM_WIDTH128_BITOFF	2U
#define XAIETILE_PLIF_STRM_WIDTH64_SHIFT         6U

/***************************** Type Definitions ******************************/

/***************************** Macro Definitions *****************************/

/************************** Function Prototypes  *****************************/
void XAieTile_PlIntfStrmWidCfg(XAieGbl_Tile *TileInstPtr, u8 MeToPl, u8 StreamId, u8 Width);
void XAieTile_PlIntfDownszrEnable(XAieGbl_Tile *TileInstPtr, u8 StreamId);
void XAieTile_PlIntfDownszrDisable(XAieGbl_Tile *TileInstPtr, u8 StreamId);
void XAieTile_PlIntfDownszrSetBypass(XAieGbl_Tile *TileInstPtr, u8 StreamId, u8 Enable);

#endif		/* end of protection macro */
/** @} */

