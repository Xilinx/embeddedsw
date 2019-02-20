/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_mb.c
* @addtogroup rfdc_v8_1
* @{
*
* Contains the interface functions of the Mixer Settings in XRFdc driver.
* See xrfdc.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 6.0   cog    02/17/18 Initial release/handle alternate bound out.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    08/02/19 Formatting changes.
* 7.1   cog    12/20/19 Metal log messages are now more descriptive.
*       cog    01/29/20 Fixed metal log typos.
* 8.0   cog    02/10/20 Updated addtogroup.
*       cog    03/20/20 Clock enables for new bondout.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Support for Dual Band IQ for new bondout.
*       cog    06/24/20 MB config is now read from bitstream.
*       cog    08/04/20 Refactor multiband for Dual DAC tiles.
*       cog    08/28/20 Prevent datapaths in bypass mode from being
*                       configured for multiband.
*       cog    10/12/20 Check generation before cheching datapath mode.
*       cog    10/14/20 Get I and Q data now supports warm bitstream swap.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static void XRFdc_SetSignalFlow(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Mode, u32 DigitalDataPathId,
				u32 MixerInOutDataType, int ConnectIData, int ConnectQData);
static void XRFdc_MB_R2C_C2R(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 NoOfDataPaths, u32 MixerInOutDataType,
			     u32 Mode, u32 DataPathIndex[], u32 BlockIndex[]);
static void XRFdc_MB_C2C(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 NoOfDataPaths, u32 MixerInOutDataType, u32 Mode,
			 u32 DataPathIndex[], u32 BlockIndex[]);
static void XRFdc_SB_R2C_C2R(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 MixerInOutDataType, u32 Mode,
			     u32 DataPathIndex[], u32 BlockIndex[]);
static void XRFdc_SB_C2C(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 MixerInOutDataType, u32 Mode,
			 u32 DataPathIndex[], u32 BlockIndex[]);
/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Static API to setup Singleband configuration for C2C MixerInOutDataType
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DataPathIndex is the array that represents the blocks enabled in
*           DigitalData Path.
* @param    BlockIndex is the array that represents the blocks enabled in
*           Analog Path(Data Converters).
*
* @return
*           - None
*
* @note     Static API for ADC/DAC blocks
*
******************************************************************************/
static void XRFdc_SB_C2C(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 MixerInOutDataType, u32 Mode,
			 u32 DataPathIndex[], u32 BlockIndex[])
{
	u32 Block_Id;

	if ((Type == XRFDC_ADC_TILE) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
		/* Update ConnectedIData and ConnectedQData for ADC 4GSPS */
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
			BlockIndex[1U];
		Block_Id = (DataPathIndex[0] == 0U ? 1U : 0U);
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].ConnectedIData = -1;
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;

		if (DataPathIndex[0] == XRFDC_BLK_ID1) {
			DataPathIndex[0] = XRFDC_BLK_ID2;
		}
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0U], BlockIndex[0U] + 1U);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 1U, MixerInOutDataType,
				    BlockIndex[1U] + 1U, BlockIndex[1U] + 2U);
		Block_Id = (DataPathIndex[0] == XRFDC_BLK_ID2 ? XRFDC_BLK_ID0 : XRFDC_BLK_ID2);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, Block_Id, MixerInOutDataType, -1, -1);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, Block_Id + 1U, MixerInOutDataType, -1, -1);
	} else {
		DataPathIndex[1] = BlockIndex[0] + BlockIndex[1] - DataPathIndex[0];
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0], BlockIndex[1]);
		/* Update ConnectedIData and ConnectedQData for DAC and ADC 2GSPS */
		if (Type == XRFDC_ADC_TILE) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType, -1,
					    -1);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1];

			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
		} else {
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				/* rerouting & configuration for alternative bonding. */
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_CLK_EN_OFFSET, XRFDC_ALT_BOND_CLKDP_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_CLKDP_SHIFT);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, XRFDC_BLK_ID1, MixerInOutDataType,
						    -1, -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, XRFDC_BLK_ID3, MixerInOutDataType,
						    -1, -1);
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID1].ConnectedIData =
					-1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID3].ConnectedQData =
					-1;
			} else {
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
						    MixerInOutDataType, -1, -1);
			}
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1];

			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
		}
	}
}

/*****************************************************************************/
/**
*
* Static API to setup Singleband configuration for C2R and R2C MultiBandDataTypes
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DataPathIndex is the array that represents the blocks enabled in
*           DigitalData Path.
* @param    BlockIndex is the array that represents the blocks enabled in
*           Analog Path(Data Converters).
*
* @return
*           - None
*
* @note     Static API for ADC/DAC blocks
*
******************************************************************************/
static void XRFdc_SB_R2C_C2R(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 MixerInOutDataType, u32 Mode,
			     u32 DataPathIndex[], u32 BlockIndex[])
{
	if (Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
	} else {
		if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
			/* rerouting & configuration for alternative bonding. */
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
					XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
					XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
			XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
					XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
					XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
		}
		InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
	}
	if ((Type == XRFDC_ADC_TILE) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
		if (DataPathIndex[0] == XRFDC_BLK_ID1) {
			DataPathIndex[0] = XRFDC_BLK_ID2;
		}
		if (BlockIndex[0] == XRFDC_BLK_ID1) {
			BlockIndex[0] = XRFDC_BLK_ID2;
		}
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 1U, MixerInOutDataType,
				    BlockIndex[0U] + 1U, -1);
	}
	XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType, BlockIndex[0U], -1);
}

/*****************************************************************************/
/**
*
* Static API to setup Multiband configuration for C2C MixerInOutDataType
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DataPathIndex is the array that represents the blocks enabled in
*           DigitalData Path.
* @param    BlockIndex is the array that represents the blocks enabled in
*           Analog Path(Data Converters).
*
* @return
*           - None
*
* @note     Static API for ADC/DAC blocks
*
******************************************************************************/
static void XRFdc_MB_C2C(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 NoOfDataPaths, u32 MixerInOutDataType, u32 Mode,
			 u32 DataPathIndex[], u32 BlockIndex[])
{
	if ((Type == XRFDC_ADC_TILE) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0U], BlockIndex[0U] + 1U);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 1U, MixerInOutDataType,
				    BlockIndex[0U] + 2U, BlockIndex[0U] + 3U);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 2U, MixerInOutDataType,
				    BlockIndex[0U], BlockIndex[0U] + 1U);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 3U, MixerInOutDataType,
				    BlockIndex[0U] + 2U, BlockIndex[0U] + 3U);

		/* Update ConnectedIData and ConnectedQData for ADC 4GSPS */
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
			BlockIndex[1U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData =
			BlockIndex[1U];
	} else if (NoOfDataPaths == XRFDC_MB_DUAL_BAND) {
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0U], BlockIndex[1U]);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
				    BlockIndex[0U], BlockIndex[1U]);

		/* Update ConnectedIData and ConnectedQData for DAC and ADC 2GSPS */
		if (Type == XRFDC_ADC_TILE) {
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData =
				BlockIndex[1U];
		} else {
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				/* rerouting & configuration for alternative bonding. */
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_CLK_EN_OFFSET, XRFDC_ALT_BOND_CLKDP_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_CLKDP_SHIFT);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, XRFDC_BLK_ID2, MixerInOutDataType,
						    -1, -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, XRFDC_BLK_ID3, MixerInOutDataType,
						    -1, -1);
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID2].ConnectedIData =
					-1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID2].ConnectedQData =
					-1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID3].ConnectedIData =
					-1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[XRFDC_BLK_ID3].ConnectedQData =
					-1;
			}
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData =
				BlockIndex[1U];
		}
	}
	if (NoOfDataPaths == XRFDC_MB_QUAD_BAND) {
		if (Type == XRFDC_ADC_TILE) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
					    BlockIndex[0U], BlockIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
					    BlockIndex[0U], BlockIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2], MixerInOutDataType,
					    BlockIndex[0U], BlockIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3], MixerInOutDataType,
					    BlockIndex[0U], BlockIndex[1U]);

			/* Update ConnectedIData and ConnectedQData for ADC 4GSPS */
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].ConnectedQData =
				BlockIndex[1U];
		} else {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
					    DataPathIndex[0], DataPathIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
					    DataPathIndex[0U], DataPathIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2], MixerInOutDataType,
					    DataPathIndex[2U], DataPathIndex[3U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3], MixerInOutDataType,
					    DataPathIndex[2U], DataPathIndex[3U]);

			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				/* rerouting & configuration for alternative bonding. */
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_CLK_EN_OFFSET, XRFDC_ALT_BOND_CLKDP_MASK,
						XRFDC_ENABLED << XRFDC_ALT_BOND_CLKDP_SHIFT);
			}

			/* Update ConnectedIData and ConnectedQData for DAC and ADC 2GSPS */
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].ConnectedQData =
				BlockIndex[1U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].ConnectedQData =
				BlockIndex[1U];
		}
	}
}

/*****************************************************************************/
/**
*
* Static API to setup Multiband configuration for C2C MixerInOutDataType
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DataPathIndex is the array that represents the blocks enabled in
*           DigitalData Path.
* @param    BlockIndex is the array that represents the blocks enabled in
*           Analog Path(Data Converters).
*
* @return
*           - None
*
* @note     Static API for ADC/DAC blocks
*
******************************************************************************/
static void XRFdc_MB_R2C_C2R(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 NoOfDataPaths, u32 MixerInOutDataType,
			     u32 Mode, u32 DataPathIndex[], u32 BlockIndex[])
{
	if ((Type == XRFDC_ADC_TILE) && (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
		/* Update ConnectedIData and ConnectedQData */
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
			BlockIndex[0U];
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
		if (BlockIndex[0] == XRFDC_BLK_ID1) {
			BlockIndex[0] = XRFDC_BLK_ID2;
		}
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0U], -1);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
				    BlockIndex[0U] + 1U, -1);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0] + 2U, MixerInOutDataType,
				    BlockIndex[0U], -1);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1] + 2U, MixerInOutDataType,
				    BlockIndex[0U] + 1U, -1);
	} else if (NoOfDataPaths == XRFDC_MB_DUAL_BAND) {
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
				    BlockIndex[0], -1);
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
				    BlockIndex[0], -1);

		/* Update ConnectedIData and ConnectedQData */
		if (Type == XRFDC_ADC_TILE) {
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
		} else {
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				/* rerouting & configuration for alternative bonding. */
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
			}
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
		}
	}
	if (NoOfDataPaths == XRFDC_MB_QUAD_BAND) {
		if (Type == XRFDC_ADC_TILE) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
					    BlockIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
					    BlockIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2], MixerInOutDataType,
					    BlockIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3], MixerInOutDataType,
					    BlockIndex[0], -1);

			/* Update ConnectedIData and ConnectedQData */
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].ConnectedIData =
				BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].ConnectedQData = -1;

		} else {
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				/* rerouting & configuration for alternative bonding. */
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID1),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
				XRFdc_ClrSetReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, XRFDC_BLK_ID2),
						XRFDC_DAC_MB_CFG_OFFSET, XRFDC_ALT_BOND_MASK,
						XRFDC_DISABLED << XRFDC_ALT_BOND_SHIFT);
			}
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0], MixerInOutDataType,
					    DataPathIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1], MixerInOutDataType,
					    DataPathIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2], MixerInOutDataType,
					    DataPathIndex[2], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3], MixerInOutDataType,
					    DataPathIndex[2], -1);

			/* Update ConnectedIData and ConnectedQData */
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedIData =
				DataPathIndex[0];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].ConnectedQData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedIData =
				DataPathIndex[0];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].ConnectedQData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].ConnectedIData =
				DataPathIndex[0];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].ConnectedQData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].ConnectedIData =
				DataPathIndex[0];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].ConnectedQData = -1;
		}
	}
}

/*****************************************************************************/
/**
*
* Static API to update mode and MultibandConfig
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    NoOfDataPaths is number of DataPaths enabled.
* @param    ModePtr is a pointer to connection mode SB/MB_2X/MB_4X.
* @param    DataPathIndex is the array that represents the blocks enabled in
*           DigitalData Path.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
*
* @return
*           - None
*
* @note     Static API for ADC/DAC blocks
*
******************************************************************************/
static u32 XRFdc_UpdateMBConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 NoOfDataPaths, u32 *ModePtr,
				u32 DataPathIndex[], u32 MixerInOutDataType)
{
	u8 MultibandConfig;
	u32 Status;

	MultibandConfig = XRFdc_GetMultibandConfig(InstancePtr, Type, Tile_Id);

	if (NoOfDataPaths == 1U) {
		*ModePtr = XRFDC_SINGLEBAND_MODE;
		if (((DataPathIndex[0] == XRFDC_BLK_ID2) || (DataPathIndex[0] == XRFDC_BLK_ID3)) &&
		    ((MultibandConfig == XRFDC_MB_MODE_2X_BLK01_BLK23) ||
		     (MultibandConfig == XRFDC_MB_MODE_2X_BLK01_BLK23_ALT) || (MultibandConfig == XRFDC_MB_MODE_4X))) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK01;
		} else if (((DataPathIndex[0] == XRFDC_BLK_ID0) || (DataPathIndex[0] == XRFDC_BLK_ID1)) &&
			   ((MultibandConfig == XRFDC_MB_MODE_2X_BLK01_BLK23) ||
			    (MultibandConfig == XRFDC_MB_MODE_2X_BLK01_BLK23_ALT) ||
			    (MultibandConfig == XRFDC_MB_MODE_4X))) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK23;
		} else if ((MultibandConfig == XRFDC_MB_MODE_2X_BLK01) &&
			   ((DataPathIndex[0] == XRFDC_BLK_ID0) || (DataPathIndex[0] == XRFDC_BLK_ID1))) {
			MultibandConfig = XRFDC_MB_MODE_SB;
		} else if ((MultibandConfig == XRFDC_MB_MODE_2X_BLK23) &&
			   ((DataPathIndex[0] == XRFDC_BLK_ID2) || (DataPathIndex[0] == XRFDC_BLK_ID3))) {
			MultibandConfig = XRFDC_MB_MODE_SB;
		}
	} else if (NoOfDataPaths == XRFDC_MB_DUAL_BAND) {
		*ModePtr = XRFDC_MULTIBAND_MODE_2X;
		if ((Type == XRFDC_DAC_TILE) &&
		    (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) &&
		    (MixerInOutDataType == XRFDC_MB_DATATYPE_C2C)) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK01_BLK23_ALT;
		} else if (((MultibandConfig == XRFDC_MB_MODE_2X_BLK01) && (DataPathIndex[0] == XRFDC_BLK_ID2) &&
			    (DataPathIndex[1] == XRFDC_BLK_ID3)) ||
			   ((MultibandConfig == XRFDC_MB_MODE_2X_BLK23) && (DataPathIndex[0] == XRFDC_BLK_ID0) &&
			    (DataPathIndex[1] == XRFDC_BLK_ID1)) ||
			   (MultibandConfig == XRFDC_MB_MODE_4X)) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK01_BLK23;
		} else if (((DataPathIndex[0] == XRFDC_BLK_ID2) && (DataPathIndex[1] == XRFDC_BLK_ID3)) &&
			   (MultibandConfig == XRFDC_MB_MODE_SB)) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK23;
		} else if (((DataPathIndex[0] == XRFDC_BLK_ID0) && (DataPathIndex[1] == XRFDC_BLK_ID1)) &&
			   (MultibandConfig == XRFDC_MB_MODE_SB)) {
			MultibandConfig = XRFDC_MB_MODE_2X_BLK01;
		}
	} else if (NoOfDataPaths == XRFDC_MB_QUAD_BAND) {
		*ModePtr = XRFDC_MULTIBAND_MODE_4X;
		MultibandConfig = XRFDC_MB_MODE_4X;
	} else {
		metal_log(METAL_LOG_ERROR, "\n Invalid Number of Datapaths (%u) for %s %u in %s\r\n", NoOfDataPaths,
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	/* Update Multiband Config member */
	XRFdc_WriteReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_MB_CONFIG_OFFSET, MultibandConfig);
	if (Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].MultibandConfig = MultibandConfig;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].MultibandConfig = MultibandConfig;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* User-level API to setup multiband configuration.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    DigitalDataPathMask is the DataPath mask. First 4 bits represent
*           4 data paths, 1 means enabled and 0 means disabled.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    DataConverterMask is block enabled mask (input/output driving
*           blocks). 1 means enabled and 0 means disabled.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_MultiBand(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 DigitalDataPathMask, u32 MixerInOutDataType,
		    u32 DataConverterMask)
{
	u32 Status;
	u32 Block_Id;
	u8 NoOfDataPaths = 0U;
	u32 BlockIndex[XRFDC_NUM_OF_BLKS4] = { XRFDC_BLK_ID4 };
	u32 DataPathIndex[XRFDC_NUM_OF_BLKS4] = { XRFDC_BLK_ID4 };
	u32 NoOfDataConverters = 0U;
	u32 Mode = 0x0;
	u32 NoOfBlocks = XRFDC_BLK_ID4;
	u32 DatapathMode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((DigitalDataPathMask == 0U) || (DigitalDataPathMask > 0xFU)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid DigitalDataPathMask value (0x%x) for %s %u in %s\r\n",
			  DigitalDataPathMask, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((Type == XRFDC_DAC_TILE) &&
	    (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) &&
	    (DataConverterMask & 0xa)) {
		metal_log(METAL_LOG_ERROR,
			  "\n Invalid DigitalDataPathMask value (0x%x) for alternate bondout DAC %u in %s\r\n",
			  DigitalDataPathMask, Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((DataConverterMask == 0U) || (DataConverterMask > 0xFU)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid DataConverterMask value (0x%x) for %s %u in %s\r\n",
			  DataConverterMask, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((MixerInOutDataType != XRFDC_MB_DATATYPE_C2C) && (MixerInOutDataType != XRFDC_MB_DATATYPE_R2C) &&
	    (MixerInOutDataType != XRFDC_MB_DATATYPE_C2R)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid MixerInOutDataType value (%u) for %s %u in %s\r\n",
			  MixerInOutDataType, (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_BLK_ID2;
	}
	/* Identify DataPathIndex and BlockIndex */
	for (Block_Id = XRFDC_BLK_ID0; Block_Id < NoOfBlocks; Block_Id++) {
		if ((DataConverterMask & (1U << Block_Id)) != 0U) {
			BlockIndex[NoOfDataConverters] = Block_Id;
			NoOfDataConverters += 1U;
			Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
			if (Status != XRFDC_SUCCESS) {
				metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
				goto RETURN_PATH;
			}
			if ((InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) && (Type == XRFDC_DAC_TILE)) {
				DatapathMode =
					XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id),
						    XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
				if (DatapathMode == XRFDC_DAC_INT_MODE_FULL_BW_BYPASS) {
					metal_log(METAL_LOG_ERROR, "\n DAC %u block %u in bypass mode in %s\r\n",
						  Tile_Id, Block_Id, __func__);
					Status = XRFDC_FAILURE;
					goto RETURN_PATH;
				}
			}
		}
		if ((DigitalDataPathMask & (1U << Block_Id)) != 0U) {
			DataPathIndex[NoOfDataPaths] = Block_Id;
			NoOfDataPaths += 1U;
			Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
			if (Status != XRFDC_SUCCESS) {
				metal_log(METAL_LOG_ERROR, "\n %s %u digital path %u not enabled in %s\r\n",
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
				goto RETURN_PATH;
			}
			if ((InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) && (Type == XRFDC_DAC_TILE)) {
				DatapathMode =
					XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id),
						    XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
				if (DatapathMode == XRFDC_DAC_INT_MODE_FULL_BW_BYPASS) {
					metal_log(METAL_LOG_ERROR, "\n DAC %u block %u in bypass mode in %s\r\n",
						  Tile_Id, Block_Id, __func__);
					Status = XRFDC_FAILURE;
					goto RETURN_PATH;
				}
			}
		}
	}

	if (BlockIndex[0] != DataPathIndex[0]) {
		metal_log(METAL_LOG_ERROR, "\n Not a valid MB/SB combination for %s %u block %u in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	/* UPdate MultibandConfig in driver instance */
	Status = XRFdc_UpdateMBConfig(InstancePtr, Type, Tile_Id, NoOfDataPaths, &Mode, DataPathIndex,
				      MixerInOutDataType);
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}

	if ((MixerInOutDataType == XRFDC_MB_DATATYPE_C2C) && (Mode == XRFDC_SINGLEBAND_MODE)) {
		/* Singleband C2C */
		XRFdc_SB_C2C(InstancePtr, Type, Tile_Id, MixerInOutDataType, Mode, DataPathIndex, BlockIndex);
	} else if (((MixerInOutDataType == XRFDC_MB_DATATYPE_R2C) || (MixerInOutDataType == XRFDC_MB_DATATYPE_C2R)) &&
		   (Mode == XRFDC_SINGLEBAND_MODE)) {
		/* Singleband R2C and C2R */
		XRFdc_SB_R2C_C2R(InstancePtr, Type, Tile_Id, MixerInOutDataType, Mode, DataPathIndex, BlockIndex);
	}
	if ((MixerInOutDataType == XRFDC_MB_DATATYPE_C2C) &&
	    ((Mode == XRFDC_MULTIBAND_MODE_2X) || (Mode == XRFDC_MULTIBAND_MODE_4X))) {
		/* Multiband C2C */
		XRFdc_MB_C2C(InstancePtr, Type, Tile_Id, NoOfDataPaths, MixerInOutDataType, Mode, DataPathIndex,
			     BlockIndex);
	} else if (((MixerInOutDataType == XRFDC_MB_DATATYPE_R2C) || (MixerInOutDataType == XRFDC_MB_DATATYPE_C2R)) &&
		   ((Mode == XRFDC_MULTIBAND_MODE_2X) || (Mode == XRFDC_MULTIBAND_MODE_4X))) {
		/* Multiband C2R and R2C */
		XRFdc_MB_R2C_C2R(InstancePtr, Type, Tile_Id, NoOfDataPaths, MixerInOutDataType, Mode, DataPathIndex,
				 BlockIndex);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Sets up signal flow configuration.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DigitalDataPathId for the requested I or Q data.
* @param    MixerInOutDataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    ConnectIData is analog blocks that are connected to
*           DigitalDataPath I.
* @param    ConnectQData is analog blocks that are connected to
*           DigitalDataPath Q.
*
* @note     None.
*
* @note     static API used internally.
*
******************************************************************************/
static void XRFdc_SetSignalFlow(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Mode, u32 DigitalDataPathId,
				u32 MixerInOutDataType, int ConnectIData, int ConnectQData)
{
	u16 ReadReg;
	u32 BaseAddr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, DigitalDataPathId);
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_ADC_SWITCH_MATRX_OFFSET);
		ReadReg &= ~XRFDC_SWITCH_MTRX_MASK;
		if (ConnectIData != -1) {
			ReadReg |= ((u16)ConnectIData) << XRFDC_SEL_CB_TO_MIX0_SHIFT;
		}
		if (ConnectQData != -1) {
			ReadReg |= (u16)ConnectQData;
		}
		if ((MixerInOutDataType == XRFDC_MB_DATATYPE_C2C) &&
		    (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1)) {
			ReadReg |= XRFDC_SEL_CB_TO_QMC_MASK;
		}
		if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
			ReadReg |= XRFDC_SEL_CB_TO_DECI_MASK;
		}

		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_ADC_SWITCH_MATRX_OFFSET, ReadReg);
	} else {
		/* DAC */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_DAC_MB_CFG_OFFSET);
		ReadReg &= ~XRFDC_MB_CFG_MASK;
		if (Mode == XRFDC_SINGLEBAND_MODE) {
			if ((u32)ConnectIData == DigitalDataPathId) {
				if (ConnectQData != -1) {
					ReadReg |= XRFDC_SB_C2C_BLK0;
				} else {
					ReadReg |= XRFDC_SB_C2R;
				}
			}
			if ((ConnectIData == -1) && (ConnectQData == -1)) {
				ReadReg |= XRFDC_SB_C2C_BLK1;
			}
		} else {
			if (Mode == XRFDC_MULTIBAND_MODE_4X) {
				ReadReg |= XRFDC_MB_EN_4X_MASK;
			}
			if ((u32)ConnectIData == DigitalDataPathId) {
				if (ConnectQData != -1) {
					ReadReg |= XRFDC_MB_C2C_BLK0;
				} else {
					ReadReg |= XRFDC_MB_C2R_BLK0;
				}
			} else {
				if (ConnectQData != -1) {
					ReadReg |= XRFDC_MB_C2C_BLK1;
				} else {
					ReadReg |= XRFDC_MB_C2R_BLK1;
				}
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_MB_CFG_OFFSET, ReadReg);
	}
}
/*****************************************************************************/
/**
*
* Get Data Converter connected for digital data paths I & Q.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is Digital Data Path number.
* @param    ConnectedData is Digital Data Path number.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
******************************************************************************/
u32 XRFdc_GetConnectedIQData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, int *ConnectedIData,
			     int *ConnectedQData)
{
	u32 Status;
	u32 Index;
	u32 MBConfig;
	u32 XBarReg;
	u32 IQ;
	u32 NumConverters;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConnectedIData != NULL);
	Xil_AssertNonvoid(ConnectedQData != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	*ConnectedIData = -1;
	*ConnectedQData = -1;
	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u digital path %u not enabled in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	MBConfig = XRFdc_ReadReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_MB_CONFIG_OFFSET);
	Index = Block_Id;
	if ((Block_Id == XRFDC_BLK_ID1) || (Block_Id == XRFDC_BLK_ID3)) {
		Index--;
	}
	if (Type == XRFDC_ADC_TILE) {
		NumConverters = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].NumSlices;
		XBarReg = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_ADC_TILE, Tile_Id, Index),
				      XRFDC_ADC_SWITCH_MATRX_OFFSET, XRFDC_SEL_CB_TO_MIX1_MASK);
	} else {
		NumConverters = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices;
		XBarReg = XRFdc_RDReg(InstancePtr, XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Index),
				      XRFDC_DAC_MB_CFG_OFFSET, (XRFDC_DAC_MB_SEL_MASK | XRFDC_ALT_BOND_MASK));
	}
	IQ = (XBarReg == XRFDC_DISABLED) ? XRFDC_DISABLED : XRFDC_ENABLED;

	switch (MBConfig) {
	case XRFDC_MB_MODE_4X:
		*ConnectedIData = XRFDC_BLK_ID0;
		if (IQ == XRFDC_ENABLED) {
			*ConnectedQData = XRFDC_BLK_ID1;
		}
		break;
	case XRFDC_MB_MODE_SB:
		if (IQ == XRFDC_ENABLED) {
			if ((Block_Id == XRFDC_BLK_ID0) ||
			    ((Block_Id == XRFDC_BLK_ID2) && (NumConverters == XRFDC_QUAD_TILE))) {
				*ConnectedIData = Block_Id;
				*ConnectedQData = Block_Id + 1;
			}
		} else {
			if ((Block_Id == XRFDC_BLK_ID0) ||
			    ((Block_Id == XRFDC_BLK_ID1) && (NumConverters == XRFDC_DUAL_TILE) &&
			     (Type == XRFDC_ADC_TILE)) ||
			    ((Block_Id == XRFDC_BLK_ID2) && (NumConverters == XRFDC_DUAL_TILE)) ||
			    (NumConverters == XRFDC_QUAD_TILE)) {
				*ConnectedIData = Block_Id;
			}
		}
		break;
	case XRFDC_MB_MODE_2X_BLK01_BLK23_ALT:
		if (Block_Id < XRFDC_BLK_ID2) {
			*ConnectedIData = XRFDC_BLK_ID0;
			*ConnectedQData = XRFDC_BLK_ID1;
		}
		break;
	case XRFDC_MB_MODE_2X_BLK01_BLK23:
		if (Block_Id < XRFDC_BLK_ID2) {
			*ConnectedIData = XRFDC_BLK_ID0;
		} else {
			*ConnectedIData = XRFDC_BLK_ID2;
		}
		if (IQ == XRFDC_ENABLED) {
			*ConnectedQData = *ConnectedIData + 1;
		}
		break;
	case XRFDC_MB_MODE_2X_BLK01:
		if (Block_Id < XRFDC_BLK_ID2) {
			*ConnectedIData = XRFDC_BLK_ID0;
			if (IQ == XRFDC_ENABLED) {
				*ConnectedQData = XRFDC_BLK_ID1;
			}
		} else {
			if (IQ == XRFDC_ENABLED) {
				if ((Block_Id == XRFDC_BLK_ID2) && (NumConverters == XRFDC_QUAD_TILE)) {
					*ConnectedIData = XRFDC_BLK_ID2;
					*ConnectedQData = XRFDC_BLK_ID3;
				}
			} else {
				if (((Block_Id == XRFDC_BLK_ID2) && (NumConverters == XRFDC_DUAL_TILE)) ||
				    (NumConverters == XRFDC_QUAD_TILE)) {
					*ConnectedIData = Block_Id;
				}
			}
		}
		break;
	case XRFDC_MB_MODE_2X_BLK23:
		if (Block_Id < XRFDC_BLK_ID2) {
			if (IQ == XRFDC_ENABLED) {
				if (Block_Id == XRFDC_BLK_ID0) {
					*ConnectedIData = XRFDC_BLK_ID0;
					*ConnectedQData = XRFDC_BLK_ID1;
				}
			} else {
				if ((Block_Id != XRFDC_BLK_ID1) || (NumConverters == XRFDC_QUAD_TILE)) {
					*ConnectedIData = Block_Id;
				}
			}
		} else {
			if ((NumConverters == XRFDC_QUAD_TILE) || (IQ == XRFDC_DISABLED)) {
				*ConnectedIData = XRFDC_BLK_ID2;
				if (IQ == XRFDC_ENABLED) {
					*ConnectedQData = XRFDC_BLK_ID3;
				}
			}
		}
		break;
	default:
		metal_log(METAL_LOG_ERROR, "\nUndefined Multiband Mode (%u) in %s\r\n", MBConfig, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if ((Type == XRFDC_DAC_TILE) && (NumConverters == XRFDC_DUAL_TILE) && (*ConnectedQData == XRFDC_BLK_ID1)) {
		*ConnectedQData = XRFDC_BLK_ID2;
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Get Data Converter connected for digital data path I
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is Digital Data Path number.
*
* @return
*           - Return Data converter Id.
*
******************************************************************************/
int XRFdc_GetConnectedIData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	int ConnectedIData;
	int ConnectedQData;
	(void)XRFdc_GetConnectedIQData(InstancePtr, Type, Tile_Id, Block_Id, &ConnectedIData, &ConnectedQData);
	return ConnectedIData;
}
/*****************************************************************************/
/**
*
* Get Data Converter connected for digital data path Q
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is Digital Data Path number.
*
* @return
*           - Return Data converter Id.
*
******************************************************************************/
int XRFdc_GetConnectedQData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	int ConnectedIData;
	int ConnectedQData;
	(void)XRFdc_GetConnectedIQData(InstancePtr, Type, Tile_Id, Block_Id, &ConnectedIData, &ConnectedQData);
	return ConnectedQData;
}
/** @} */
