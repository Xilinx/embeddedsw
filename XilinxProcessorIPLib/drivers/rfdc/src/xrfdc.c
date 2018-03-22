/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* @file xrfdc.c
* @addtogroup xrfdc_v3_2
* @{
*
* Contains the interface functions of the XRFdc driver.
* See xrfdc.h for a detailed description of the device and driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   sk     05/16/17 Initial release
* 2.0   sk     08/09/17 Fixed coarse Mixer configuration settings
*                       CR# 977266, 977872.
*                       Return error for Slice Event on 4G ADC Block.
*              08/16/17 Add support for SYSREF and PL event sources.
*              08/18/17 Add API to enable and disable FIFO.
*              08/23/17 Add API to configure Nyquist zone.
*              08/30/17 Add additional info to BlockStatus.
*              08/30/17 Add support for Coarse Mixer BYPASS mode.
*              08/31/17 Removed Tile Reset Assert and Deassert.
*              09/07/17 Add support for negative NCO freq.
*              09/15/17 Fixed NCO freq precision issue.
*              09/15/17 Fixed Immediate Event source issue and also
*                       updated the Immediate Macro value to 0.
* 2.1   sk     09/15/17 Remove Libmetal library dependency for MB.
*       sk     09/25/17 Modified XRFdc_GetBlockStatus API to give
*                       correct information and also updates the
*                       description for Vector Param in intr handler
*                       Add API to get Output current and removed
*                       GetTermVoltage and GetOutputCurr inline functions.
* 2.2   sk     10/05/17 Fixed XRFdc_GetNoOfADCBlocks API for 4GSPS.
*                       Enable the decoder clock based on decoder mode.
*                       Add API to get the current FIFO status.
*                       Updated XRFdc_DumpRegs API for better readability
*                       of output register dump.
*                       Add support for 4GSPS CoarseMixer frequency.
*              10/11/17 Modify float types to double to increase precision.
*              10/12/17 Update BlockStatus API to give current status.
*                       In BYPASS mode, input datatype can be Real or IQ,
*                       hence checked both while reading the mixer mode.
*              10/17/17 Fixed Set Threshold API Issue.
* 2.3   sk     11/06/17 Fixed PhaseOffset truncation issue.
*                       Provide user configurability for FineMixerScale.
*              11/08/17 Return error for DAC R2C mode and ADC C2R mode.
*              11/20/17 Fixed StartUp, Shutdown and Reset API for Tile_Id -1.
*              11/20/17 Remove unwanted ADC block checks in 4GSPS mode.
* 3.0   sk     12/11/17 Added DDC and DUC support.
*              12/13/17 Add CoarseMixMode field in Mixer_Settings structure.
*              12/15/17 Add support to switch calibration modes.
*              12/15/17 Add support for mixer frequencies > Fs/2 and < -Fs/2.
*	sg     13/01/18 Added PLL and external clock switch support.
*			Added API to get PLL lock status.
*			Added API to get clock source. 
* 3.1   jm     01/24/18 Add Multi-tile sync support.
*       sk     01/25/18 Updated Set and Get Interpolation/Decimation factor
*                       API's to consider the actual factor value.
* 3.2   sk     02/02/18 Add API's to configure inverse-sinc.
*       sk     02/27/18 Add API's to configure Multiband.
*       sk     03/09/18 Update PLL structure in XRFdc_DynamicPLLConfig API.
*       sk     03/09/18 Update ADC and DAC datatypes in Mixer API and use
*                       input datatype for ADC in threshold and QMC APIs.
*       sk     03/09/18 Removed FIFO disable check in DDC and DUC APIs.
*       sk     03/09/18 Add support for Marker event source for DAC block.
*       sk     03/22/18 Updated PLL settings based on latest IP values.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/
u32 PllTuningMatrix [8][4][2] = {
		{{0x7F8A, 0x3FFF}, {0x7F9C, 0x3FFF}, {0x7FE2, 0x3FFF}},
		{{0x7FE9, 0xFFFF}, {0x7F8E, 0xFFFF}, {0x7F9C, 0xFFFF}},
		{{0x7F95, 0xFFFF}, {0x7F8E, 0xFFFF}, { 0x7F9A, 0xFFFF}, {0x7F8C, 0xFFFF}},
		{{0x7F95, 0x3FFF}, {0x7FEE, 0x3FFF}, { 0x7F9A, 0xFFFF}, {0x7F9C, 0xFFFF}},
		{{0x7F95, 0x3FFF}, {0x7FEE, 0x3FFF}, { 0x7F9A, 0xFFFF}, {0x7F9C, 0xFFFF}},
		{{0x7F95, 0xFFFF}, {0x7F8E, 0xFFFF}, { 0x7FEA, 0xFFFF}, {0x7F9C, 0xFFFF}},
		{{0x7FE9, 0xFFFF}, {0x7F8E, 0xFFFF}, { 0x7F9A, 0xFFFF}, {0x7F9C, 0xFFFF}},
		{{0x7FEC, 0xFFFF}, {0x7FEE, 0x3FFF}, { 0x7F9C, 0xFFFF}}
};


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static void XRFdc_RestartIPSM(XRFdc* InstancePtr, u32 BaseAddress, u32 Start,
								u32 End);
static void StubHandler(void *CallBackRef, u32 Type, int Tile_Id,
								u32 Block_Id, u32 StatusEvent);
static u32 XRFdc_SetPLLConfig(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
		double RefClkFreq, double SamplingRate);
static void XRFdc_SetSignalFlow(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
		u32 Mode, u32 DigitalDataPathId, u32 DataType,
		int ConnectIData, int ConnectQData);
/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Initializes a specific XRFdc instance such that the driver is ready to use.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Config is a reference to a structure containing information
*			about xrfdc. This function initializes an InstancePtr object
*			for a specific device specified by the contents of Config.
*
* @return
*		- XRFDC_SUCCESS if successful.
*
* @note		The user needs to first call the XRFdc_LookupConfig() API
*			which returns the Configuration structure pointer which is
*			passed as a parameter to the XRFdc_CfgInitialize() API.
*
******************************************************************************/
int XRFdc_CfgInitialize(XRFdc* InstancePtr, XRFdc_Config *Config)
{
	s32 Status;
	u32 Tile_Id;
	u32 Block_Id;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Config != NULL);
#endif
#ifndef __MICROBLAZE__
	InstancePtr->io = (struct metal_io_region *)
			metal_allocate_memory(sizeof(struct metal_io_region));
	metal_io_init(InstancePtr->io, (void *)(metal_phys_addr_t)Config->BaseAddr,
			&Config->BaseAddr, 0x40000, (unsigned)(-1), 0, NULL);
#endif
	/*
	 * Set the values read from the device config and the base address.
	 */
	InstancePtr->BaseAddr = Config->BaseAddr;
	InstancePtr->RFdc_Config = *Config;
	InstancePtr->ADC4GSPS = Config->ADCType;
	InstancePtr->StatusHandler = StubHandler;

	for (Tile_Id = 0; Tile_Id < 4U; Tile_Id++) {
		InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks = 0;
		InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks = 0;
		for (Block_Id = 0; Block_Id < 4U; Block_Id++) {
			if (XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U)
				InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks++;
			if (XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U)
				InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks++;

			if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
				ADCBlock_Analog_Config[Block_Id].MixMode == XRFDC_MIXER_MODE_BYPASS)
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].DataType =
					InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Digital_Config[Block_Id].DataType;
			else
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].DataType =
						InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
										ADCBlock_Analog_Config[Block_Id].MixMode;
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
				DACBlock_Analog_Config[Block_Id].MixMode == XRFDC_MIXER_MODE_BYPASS)
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].DataType =
					InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Digital_Config[Block_Id].DataType;
			else
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].DataType =
						XRFDC_DATA_TYPE_IQ;
		}

		/* Update PLL structure */
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate =
				InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].SamplingRate;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate =
				InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].SamplingRate;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq =
				InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].RefClkFreq;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq =
				InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].RefClkFreq;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.Enabled =
				InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].PLLEnable;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.Enabled =
				InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].PLLEnable;
	}

	for (Tile_Id = 0; Tile_Id < 4U; Tile_Id++) {
		for (Block_Id = 0; Block_Id < 4U; Block_Id++) {
			if (XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
						ADCBlock_Analog_Config[Block_Id].MixMode == 1U) {
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].ConnectedIData = Block_Id;
					Block_Id++;
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].ConnectedQData = Block_Id;
				} else {
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].ConnectedIData = Block_Id;
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;

				}
			} else {
				InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Block_Id].ConnectedIData = -1;
				InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;
			}
		}
	}
	for (Tile_Id = 0; Tile_Id < 4U; Tile_Id++) {
		for (Block_Id = 0; Block_Id < 4U; Block_Id++) {
			if (XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
						DACBlock_Analog_Config[Block_Id].MixMode == 1U) {
					InstancePtr->DAC_Tile[Tile_Id].
						DACBlock_Digital_Datapath[Block_Id].ConnectedIData = Block_Id;
					Block_Id++;
					InstancePtr->DAC_Tile[Tile_Id].
						DACBlock_Digital_Datapath[Block_Id].ConnectedQData = Block_Id;
				} else {
					InstancePtr->DAC_Tile[Tile_Id].
						DACBlock_Digital_Datapath[Block_Id].ConnectedIData = Block_Id;
					InstancePtr->DAC_Tile[Tile_Id].
						DACBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;

				}
			} else {
				InstancePtr->DAC_Tile[Tile_Id].
					DACBlock_Digital_Datapath[Block_Id].ConnectedIData = -1;
				InstancePtr->DAC_Tile[Tile_Id].
					DACBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;
			}
		}
	}
	/*
	 * Indicate the instance is now ready to use and
	 * initialized without error.
	 */
	InstancePtr->IsReady = XRFDC_COMPONENT_IS_READY;

	Status = XRFDC_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
*
* The API Restarts the requested tile. It can restart a single tile and
* alternatively can restart all the tiles. Existing register settings are not
* lost or altered in the process. It just starts the requested tile(s).
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if tile is not enabled or available
*
* @note		None
*
******************************************************************************/
int XRFdc_StartUp(XRFdc* InstancePtr, u32 Type, int Tile_Id)
{
	s32 Status;
	u32 IsTileEnable;
	u32 BaseAddr;
	u16 NoOfTiles;
	u16 Index;
	int Tile;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	/* An input tile if of -1 selects all tiles */
	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = 4;
	} else {
		NoOfTiles = 1;
	}

	Tile = Tile_Id;
	for (Index = 0U; Index < NoOfTiles; Index++) {

		if (Tile == XRFDC_SELECT_ALL_TILES) {
			Tile_Id = Index;
		}

		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
										Enable;
		}
		else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
										Enable;
		}

		if ((IsTileEnable == 0U) && (NoOfTiles == 1)) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else if (IsTileEnable == 0U) {
			continue;
		}

		XRFdc_RestartIPSM(InstancePtr, BaseAddr, 0x1, 0xF);
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* Restarts a requested the tile and ensures that starts from a defined start
* state and reaches the requested or defined end state.
*
*
* @param	BaseAddress is base address for tile control and status registers.
* @param	Start is start state of State Machine
* @param	End is end state of State Machine.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if tile is not enabled or available
*
* @note		None
*
******************************************************************************/
static void XRFdc_RestartIPSM(XRFdc* InstancePtr, u32 BaseAddress, u32 Start,
								u32 End)
{
	u32 ReadReg;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#endif

	/* Write Start and End states */
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddress,
								XRFDC_RESTART_STATE_OFFSET);
	ReadReg &= ~XRFDC_PWR_STATE_MASK;
	ReadReg |= Start << 8;
	ReadReg |= End;
	XRFdc_WriteReg16(InstancePtr, BaseAddress,
								XRFDC_RESTART_STATE_OFFSET, ReadReg);

	/* Trigger restart */
	XRFdc_WriteReg(InstancePtr, BaseAddress, XRFDC_RESTART_OFFSET, 0x1);

}

/*****************************************************************************/
/**
*
* The API stops the tile as requested. It can also stop all the tiles if
* asked for. It does not clear any of the existing register settings. It just
* stops the requested tile(s).
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if tile is not enabled or available
*
* @note		None
*
******************************************************************************/
int XRFdc_Shutdown(XRFdc* InstancePtr, u32 Type, int Tile_Id)
{
	u32 BaseAddrCtrl;
	u16 NoOfTiles;
	u16 Index;
	u32 IsTileEnable;
	u32 Status;
	int Tile;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = 4;
	} else {
		NoOfTiles = 1;
	}
	Tile = Tile_Id;
	for (Index = 0U; Index < NoOfTiles; Index++) {
		if (Tile == XRFDC_SELECT_ALL_TILES) {
			Tile_Id = Index;
		}
		if (Type == XRFDC_ADC_TILE) {
			BaseAddrCtrl = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
								Enable;
		} else {
			BaseAddrCtrl = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
								Enable;
		}
		if ((IsTileEnable == 0U) && (NoOfTiles == 1)) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested tile not "
                             "available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else if (IsTileEnable == 0U) {
			continue;
		}

		XRFdc_RestartIPSM(InstancePtr, BaseAddrCtrl, 0x1, 0x1);
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* The API resets the requested tile. It can reset all the tiles as well. In
* the process, all existing register settings are cleared and are replaced
* with the settings initially configured (through the GUI).
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if tile is not enabled or available
*
* @note		None
******************************************************************************/
int XRFdc_Reset(XRFdc* InstancePtr, u32 Type, int Tile_Id)
{
	u32 BaseAddr;
	u16 NoOfTiles;
	u16 Index;
	u32 IsTileEnable;
	u32 Status;
	int Tile;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#endif

	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = 4;
	} else {
		NoOfTiles = 1;
	}
	Tile = Tile_Id;
	for (Index = 0U; Index < NoOfTiles; Index++) {
		if (Tile == XRFDC_SELECT_ALL_TILES) {
			Tile_Id = Index;
		}
		if (Type == XRFDC_ADC_TILE) {
			IsTileEnable = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
										Enable;
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		} else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
								Enable;
		}
		if ((IsTileEnable == 0U) && (NoOfTiles == 1)) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested tile not "
                             "available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else if (IsTileEnable == 0U) {
			continue;
		}

		XRFdc_RestartIPSM(InstancePtr, BaseAddr, 0x0, 0xF);
	}

	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* The API returns the IP status.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	IPStatus Pointer to the XRFdc_IPStatus structure through
*           which the status is returned.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if IP not ready.
*
* @note		None.
*
******************************************************************************/
int XRFdc_GetIPStatus(XRFdc* InstancePtr, XRFdc_IPStatus* IPStatus)
{
	s32 Status;
	u32 Tile_Id;
	u32 Block_Id;
	u32 BaseAddr;
	u16 ReadReg;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (InstancePtr->IsReady != XRFDC_COMPONENT_IS_READY) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	for (Tile_Id = 0; Tile_Id < 4U; Tile_Id++) {
		IPStatus->ADCTileStatus[Tile_Id].IsEnabled = 0;
		IPStatus->ADCTileStatus[Tile_Id].TileState = 0;
		IPStatus->ADCTileStatus[Tile_Id].BlockStatusMask = 0x0;
		IPStatus->ADCTileStatus[Tile_Id].PLLState = 0x0;
		IPStatus->ADCTileStatus[Tile_Id].PowerUpState = 0x0;
		IPStatus->DACTileStatus[Tile_Id].IsEnabled = 0;
		IPStatus->DACTileStatus[Tile_Id].TileState = 0;
		IPStatus->DACTileStatus[Tile_Id].BlockStatusMask = 0x0;
		IPStatus->DACTileStatus[Tile_Id].PLLState = 0x0;
		IPStatus->DACTileStatus[Tile_Id].PowerUpState = 0x0;

		for (Block_Id = 0; Block_Id < 4U; Block_Id++) {
			if (XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
								Block_Id) != 0U) {
				IPStatus->ADCTileStatus[Tile_Id].IsEnabled = 1;
				IPStatus->ADCTileStatus[Tile_Id].BlockStatusMask |=
								(1 << Block_Id);
			}
			if (XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Block_Id) != 0U) {

				IPStatus->DACTileStatus[Tile_Id].IsEnabled = 1;
				IPStatus->DACTileStatus[Tile_Id].BlockStatusMask |=
								(1 << Block_Id);
			}
		}

		if (IPStatus->ADCTileStatus[Tile_Id].IsEnabled) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_STATUS_OFFSET);
			IPStatus->ADCTileStatus[Tile_Id].PowerUpState = (ReadReg &
							XRFDC_PWR_UP_STAT_MASK) >> 2;
			IPStatus->ADCTileStatus[Tile_Id].PLLState = (ReadReg &
							XRFDC_PLL_LOCKED_MASK) >> 3;
			IPStatus->ADCTileStatus[Tile_Id].TileState =
							XRFdc_ReadReg16(InstancePtr, BaseAddr,
											XRFDC_CURRENT_STATE_OFFSET);
		}
		if (IPStatus->DACTileStatus[Tile_Id].IsEnabled) {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IPStatus->DACTileStatus[Tile_Id].TileState =
					XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_CURRENT_STATE_OFFSET);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_STATUS_OFFSET);
			IPStatus->DACTileStatus[Tile_Id].PowerUpState = (ReadReg &
							XRFDC_PWR_UP_STAT_MASK) >> 2;
			IPStatus->DACTileStatus[Tile_Id].PLLState = (ReadReg &
							XRFDC_PLL_LOCKED_MASK) >> 3;
		}
	}
	(void)BaseAddr;

	//TODO IP state

	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* The API returns the requested block status.
*
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3. XRFdc_BlockStatus.
* @param	BlockStatus is Pointer to the XRFdc_BlockStatus structure through
*			which the ADC/DAC block status is returned.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Common API for ADC/DAC blocks.
*
******************************************************************************/
int XRFdc_GetBlockStatus(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, XRFdc_BlockStatus* BlockStatus)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 Block;
	u16 ReadReg;
	u32 BaseAddr;
	u8 FIFOEnable;
	u32 DecimationFactor;
	u32 InterpolationFactor;
	u8 AdderEnable;
	u32 DecoderMode;
	u8 MixerMode = 0x8;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
			(Type == XRFDC_ADC_TILE)) {
		Block_Id = 2U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
										XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			BlockStatus->SamplingFreq = InstancePtr->ADC_Tile[Tile_Id].
					PLL_Settings.SampleRate;
			BlockStatus->DigitalDataPathStatus = 0;
			XRFdc_GetFIFOStatus(InstancePtr, XRFDC_ADC_TILE,
						Tile_Id, &FIFOEnable);
			BlockStatus->DigitalDataPathStatus = FIFOEnable;
			XRFdc_GetDecimationFactor(InstancePtr, Tile_Id,
						Block, &DecimationFactor);
			BlockStatus->DigitalDataPathStatus |=
				(DecimationFactor << 4);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_MXR_MODE_OFFSET);
			ReadReg &= (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK);
			if (ReadReg == 0xF)
				MixerMode =
					XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX;
			else if (ReadReg == 0x5)
				MixerMode =
					XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX;
			else if (ReadReg == 0x0)
				MixerMode = XRFDC_FINE_MIXER_MOD_OFF;
			BlockStatus->DigitalDataPathStatus |=  (MixerMode << 8);

			/*
			 * Checking ADC block enable for ADC AnalogPath.
			 * This can be changed later,
			 */
			BlockStatus->AnalogDataPathStatus =
			XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
							Block);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_IMR_OFFSET);
			BlockStatus->IsFIFOFlagsEnabled =
						ReadReg & XRFDC_FAB_IMR_USRDAT_MASK;
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_ISR_OFFSET);
			BlockStatus->IsFIFOFlagsAsserted =
					ReadReg & XRFDC_FAB_ISR_USRDAT_MASK;
		} else {
			BlockStatus->SamplingFreq = InstancePtr->DAC_Tile[Tile_Id].
					PLL_Settings.SampleRate;
			BlockStatus->DigitalDataPathStatus = 0;
			XRFdc_GetFIFOStatus(InstancePtr, XRFDC_DAC_TILE,
					Tile_Id, &FIFOEnable);
			BlockStatus->DigitalDataPathStatus = FIFOEnable;
			XRFdc_GetInterpolationFactor(InstancePtr, Tile_Id,
					Block_Id, &InterpolationFactor);
			BlockStatus->DigitalDataPathStatus |=
						(InterpolationFactor << 4);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_MB_CFG_OFFSET);
			ReadReg &= XRFDC_EN_MB_MASK;
			AdderEnable = ReadReg >> 3;
			BlockStatus->DigitalDataPathStatus |=
						(AdderEnable << 8);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
							XRFDC_MXR_MODE_OFFSET);
			ReadReg &= (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK);
			if (ReadReg == 0xF)
				MixerMode =
					XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX;
			else if (ReadReg == 0x3)
				MixerMode =
					XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL;
			else if (ReadReg == 0x0)
				MixerMode = XRFDC_FINE_MIXER_MOD_OFF;
			BlockStatus->DigitalDataPathStatus |=
						(MixerMode << 12);
			BlockStatus->AnalogDataPathStatus = 0;
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_INVSINC_OFFSET);
			ReadReg &= XRFDC_EN_INVSINC_MASK;
			BlockStatus->AnalogDataPathStatus = ReadReg;
			XRFdc_GetDecoderMode(InstancePtr, Tile_Id, Block_Id,
						&DecoderMode);
			BlockStatus->AnalogDataPathStatus |= (DecoderMode << 4);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_FABRIC_IMR_OFFSET);
			BlockStatus->IsFIFOFlagsEnabled =
						ReadReg & XRFDC_FAB_IMR_USRDAT_MASK;
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_FABRIC_ISR_OFFSET);
			BlockStatus->IsFIFOFlagsAsserted =
					ReadReg &= XRFDC_FAB_ISR_USRDAT_MASK;
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_CLK_EN_OFFSET);
		ReadReg &= XRFDC_DAT_CLK_EN_MASK;
		if (ReadReg == XRFDC_DAT_CLK_EN_MASK)
			BlockStatus->DataPathClocksStatus = 0x1;
		else
			BlockStatus->DataPathClocksStatus = 0x0;
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* The API is used to update various mixer settings, fine, coarse, NCO etc.
* Mixer/NCO settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Mixer_Settings Pointer to the XRFdc_Mixer_Settings structure
*			in which the Mixer/NCO settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		FineMixerScale in Mixer_Settings structure can have 3 values.
*		XRFDC_MXR_SCALE_* represents the valid values.
*		XRFDC_MXR_SCALE_NO_CHANGE - If mixer mode R2C, Mixer Scale is
*		set to 1 and for other modes mixer scale is set to 0.7
*		XRFDC_MXR_SCALE_ONE - To set fine mixer scale to 1.
*		XRFDC_MXR_SCALE_ZERO_DOT_SEVEN - To set fine mixer scale to 0.7.
*
******************************************************************************/
int XRFdc_SetMixerSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, XRFdc_Mixer_Settings * Mixer_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	double SamplingRate;
	s64 Freq;
	s32 PhaseOffset;
	u16 NoOfBlocks;
	u16 Index;
	XRFdc_Mixer_Settings *Mixer_Config;
	u8 CalibrationMode;
	u32 CoarseMixFreq;
	double NCOFreq;
	u32 NyquistZone;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Mixer_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
							Block_Id);
			Mixer_Config = &InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Index].Mixer_Settings;
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
			SamplingRate = InstancePtr->ADC_Tile[Tile_Id].
						PLL_Settings.SampleRate;
		} else {
			/* DAC */
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			Mixer_Config = &InstancePtr->DAC_Tile[Tile_Id].
							DACBlock_Digital_Datapath[Index].Mixer_Settings;
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(Index);
			SamplingRate = InstancePtr->DAC_Tile[Tile_Id].
						PLL_Settings.SampleRate;
		}

		if (SamplingRate <= 0) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Incorrect Sampling rate "
					"in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Incorrect Sampling rate "
							"in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		}

		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if ((Mixer_Settings->PhaseOffset >
						XRFDC_MIXER_PHASE_OFFSET_UP_LIMIT) ||
						(Mixer_Settings->PhaseOffset <
						XRFDC_MIXER_PHASE_OFFSET_LOW_LIMIT))
			{
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid phase offset value "
											"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid phase offset value "
											"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((Mixer_Settings->EventSource != XRFDC_EVNT_SRC_SLICE) &&
						(Mixer_Settings->EventSource != XRFDC_EVNT_SRC_TILE) &&
						(Mixer_Settings->EventSource !=
						XRFDC_EVNT_SRC_IMMEDIATE) &&
						(Mixer_Settings->EventSource !=
						XRFDC_EVNT_SRC_SYSREF) &&
						(Mixer_Settings->EventSource != XRFDC_EVNT_SRC_PL) &&
						((Mixer_Settings->EventSource != XRFDC_EVNT_SRC_MARKER) ||
								(Type == XRFDC_ADC_TILE)))
			{
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if (Mixer_Settings->FineMixerMode > XRFDC_MIXER_MAX_SUPP_MIXER_MODE) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid fine mixer mode "
													"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid fine mixer mode "
													"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((Mixer_Settings->CoarseMixFreq != XRFDC_COARSE_MIX_OFF) &&
				(Mixer_Settings->CoarseMixFreq !=
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO) &&
				(Mixer_Settings->CoarseMixFreq !=
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR) &&
				(Mixer_Settings->CoarseMixFreq !=
					XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR) &&
				(Mixer_Settings->CoarseMixFreq != XRFDC_COARSE_MIX_BYPASS)) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid coarse mix frequency value "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid coarse mix frequency value "
												"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((Mixer_Settings->FineMixerScale !=
				XRFDC_MIXER_SCALE_AUTO) &&
				(Mixer_Settings->FineMixerScale !=
						XRFDC_MIXER_SCALE_1P0) &&
				(Mixer_Settings->FineMixerScale !=
					XRFDC_MIXER_SCALE_0P7)) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR,
				"\n Invalid Mixer Scale in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR,
				"\n Invalid Mixer Scale in %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}
			if (((Mixer_Settings->FineMixerMode ==
				XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX) &&
				(Type == XRFDC_DAC_TILE)) ||
				((Mixer_Settings->FineMixerMode ==
				XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL) &&
				(Type == XRFDC_ADC_TILE))) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR,
				"\n Invalid Mixer mode in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR,
				"\n Invalid Mixer mode in %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}
			if ((Mixer_Settings->CoarseMixMode ==
				XRFDC_COARSE_MIX_MODE_R2C) &&
				(Type == XRFDC_DAC_TILE)) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR,
				"\n Invalid Mixer mode in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR,
				"\n Invalid Mixer mode in %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}

			if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
					(Type == XRFDC_ADC_TILE) &&
					((Mixer_Settings->EventSource == XRFDC_EVNT_SRC_SLICE) ||
					(Mixer_Settings->EventSource ==
							XRFDC_EVNT_SRC_IMMEDIATE))) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}

			if (Type == XRFDC_DAC_TILE) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_ITERP_DATA_OFFSET);
				ReadReg &= ~XRFDC_DAC_INTERP_DATA_MASK;
				InstancePtr->DAC_Tile[Tile_Id].
					DACBlock_Digital_Datapath[Block_Id].DataType = XRFDC_DATA_TYPE_REAL;
				if ((Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) ||
					(Mixer_Settings->FineMixerMode ==
						XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX) ||
					(Mixer_Settings->FineMixerMode ==
							XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL)) {
					ReadReg |= XRFDC_DAC_INTERP_DATA_MASK;
					InstancePtr->DAC_Tile[Tile_Id].
						DACBlock_Digital_Datapath[Block_Id].DataType = XRFDC_DATA_TYPE_IQ;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_ITERP_DATA_OFFSET, ReadReg);
			} else {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_DECI_CONFIG_OFFSET);
				ReadReg &= ~XRFDC_DEC_CFG_MASK;
				if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)
					ReadReg |= XRFDC_DEC_CFG_4GSPS_MASK;
				else if ((Mixer_Settings->FineMixerMode ==
							XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX) ||
						(Mixer_Settings->FineMixerMode ==
							XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX) ||
						(Mixer_Settings->CoarseMixMode ==
								XRFDC_COARSE_MIX_MODE_C2C_C2R) ||
						(Mixer_Settings->CoarseMixMode ==
								XRFDC_COARSE_MIX_MODE_R2C))
					ReadReg |= XRFDC_DEC_CFG_IQ_MASK;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
							XRFDC_ADC_DECI_CONFIG_OFFSET, ReadReg);
				if ((Mixer_Settings->FineMixerMode ==
						XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX) ||
					(Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R))
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].DataType = XRFDC_DATA_TYPE_IQ;
				if ((Mixer_Settings->FineMixerMode ==
						XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX) ||
					(Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_R2C))
					InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Digital_Datapath[Block_Id].DataType = XRFDC_DATA_TYPE_REAL;
			}

			if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
						(Type == XRFDC_ADC_TILE)) {
				if ((Index == 0U) || (Index == 2U)) {
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_NCO_PHASE_MOD_OFFSET,
										XRFDC_NCO_PHASE_MOD_EVEN);
				} else {
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_NCO_PHASE_MOD_OFFSET,
										XRFDC_NCO_PHASE_MODE_ODD);
				}
			}

			CoarseMixFreq = Mixer_Settings->CoarseMixFreq;
			NCOFreq = Mixer_Settings->Freq;
			if (Type == XRFDC_ADC_TILE) {
				Status = XRFdc_GetCalibrationMode(InstancePtr,
					Tile_Id, Block_Id, &CalibrationMode);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (CalibrationMode == XRFDC_CALIB_MODE1) {
					if (CoarseMixFreq ==
						XRFDC_COARSE_MIX_BYPASS)
						CoarseMixFreq =
							XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
					else if (CoarseMixFreq ==
						XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR)
						CoarseMixFreq =
							XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
					else if (CoarseMixFreq ==
						XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO)
						CoarseMixFreq =
							XRFDC_COARSE_MIX_BYPASS;
					else if (CoarseMixFreq ==
						XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR)
						CoarseMixFreq =
							XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
					NCOFreq -= (SamplingRate * 1000) / 2.0;
				}
			}

			if ((NCOFreq < -((SamplingRate * 1000) / 2.0)) ||
				(NCOFreq > ((SamplingRate * 1000) / 2.0))) {
				if (Type == XRFDC_ADC_TILE)
					Status = XRFdc_GetNyquistZone(InstancePtr, XRFDC_ADC_TILE,
						Tile_Id, Block_Id, &NyquistZone);
				else
					Status = XRFdc_GetNyquistZone(InstancePtr, XRFDC_DAC_TILE,
						Tile_Id, Block_Id, &NyquistZone);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				do {
					if (NCOFreq < -((SamplingRate * 1000) / 2.0))
						NCOFreq +=  (SamplingRate * 1000);
					if (NCOFreq > ((SamplingRate * 1000) / 2.0))
						NCOFreq -= (SamplingRate * 1000);
				} while ((NCOFreq < -((SamplingRate * 1000) / 2.0)) ||
					(NCOFreq > ((SamplingRate * 1000) / 2.0)));

				if ((NyquistZone == XRFDC_EVEN_NYQUIST_ZONE) &&
						(NCOFreq != 0))
					NCOFreq *= -1;
			}

			if (NCOFreq < 0)
				Freq = ((NCOFreq * XRFDC_NCO_FREQ_MIN_MULTIPLIER) /
												(SamplingRate * 1000U));
			else
				Freq = ((NCOFreq * XRFDC_NCO_FREQ_MULTIPLIER) /
												(SamplingRate * 1000U));
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_NCO_FQWD_LOW_OFFSET, (u16)Freq);
			ReadReg = (Freq >> 16) & XRFDC_NCO_FQWD_MID_MASK;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_NCO_FQWD_MID_OFFSET, (u16)ReadReg);
			ReadReg = (Freq >> 32) & XRFDC_NCO_FQWD_UPP_MASK;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_NCO_FQWD_UPP_OFFSET, (u16)ReadReg);

			PhaseOffset = ((Mixer_Settings->PhaseOffset *
									XRFDC_NCO_PHASE_MULTIPLIER) / 180);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_LOW_OFFSET,
									(u16)PhaseOffset);

			ReadReg = (PhaseOffset >> 16) & XRFDC_NCO_PHASE_UPP_MASK;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_PHASE_UPP_OFFSET,
									ReadReg);

			if (CoarseMixFreq ==
					XRFDC_COARSE_MIX_BYPASS) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG0_MASK;
				ReadReg |= XRFDC_CRSE_MIX_BYPASS;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG1_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R)
					ReadReg |= XRFDC_CRSE_MIX_BYPASS;
				else
					ReadReg |= XRFDC_CRSE_MIX_OFF;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
			} else if (CoarseMixFreq ==
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG0_MASK;
				if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
					ReadReg |= XRFDC_CRSE_MIX_I_Q_FSBYTWO;
				else {
					if (Index % 2 == 0U)
						ReadReg |=
							XRFDC_CRSE_MIX_BYPASS;
					else
						ReadReg |=
						XRFDC_CRSE_4GSPS_ODD_FSBYTWO;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG1_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_I_Q_FSBYTWO;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_BYPASS :
						XRFDC_CRSE_4GSPS_ODD_FSBYTWO;
					}
				}
				else
					ReadReg |= XRFDC_CRSE_MIX_OFF;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
			} else if (CoarseMixFreq ==
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG0_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_I_FSBYFOUR;
					else{
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
					}
				} else {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_R_I_FSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_OFF;
					}
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG1_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_Q_FSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
					}
				} else {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_R_Q_FSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_OFF :
						XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
					}
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
			} else if (CoarseMixFreq ==
					XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG0_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_I_MINFSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR;
					}
				} else {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_R_I_MINFSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_OFF;
					}
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG1_MASK;
				if (Mixer_Settings->CoarseMixMode ==
						XRFDC_COARSE_MIX_MODE_C2C_C2R) {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_Q_MINFSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_I_Q_FSBYTWO :
						XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
					}
				} else {
					if ((InstancePtr->ADC4GSPS !=
						XRFDC_ADC_4GSPS) ||
						(Type == XRFDC_DAC_TILE))
						ReadReg |=
						XRFDC_CRSE_MIX_R_Q_MINFSBYFOUR;
					else {
						ReadReg =
						(Index % 2 == 0U) ?
						XRFDC_CRSE_MIX_OFF :
						XRFDC_CRSE_MIX_I_ODD_FSBYFOUR;
					}
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
			} else if (CoarseMixFreq ==
					XRFDC_COARSE_MIX_OFF) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG0_MASK;
				ReadReg |= XRFDC_CRSE_MIX_OFF;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG0_OFFSET, (u16)ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET);
				ReadReg &= ~XRFDC_MIX_CFG1_MASK;
				ReadReg |= XRFDC_CRSE_MIX_OFF;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_MXR_CFG1_OFFSET, (u16)ReadReg);
			}

			if (Mixer_Settings->FineMixerMode ==
					XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_MXR_MODE_OFFSET);
				ReadReg |= (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK);
				ReadReg &= ~XRFDC_SEL_I_IQ_MASK;
				ReadReg |= XRFDC_I_IQ_COS_MINSIN;
				ReadReg &= ~XRFDC_SEL_Q_IQ_MASK;
				ReadReg |= XRFDC_Q_IQ_SIN_COS;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
											ReadReg);
			} else if (Mixer_Settings->FineMixerMode ==
					XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_MXR_MODE_OFFSET);
				ReadReg &= ~XRFDC_EN_Q_IQ_MASK;
				ReadReg |= XRFDC_EN_I_IQ_MASK;
				ReadReg &= ~XRFDC_SEL_I_IQ_MASK;
				ReadReg |= XRFDC_I_IQ_COS_MINSIN;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
											ReadReg);
			} else if (Mixer_Settings->FineMixerMode ==
					XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_MXR_MODE_OFFSET);
				ReadReg &= ~(XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK);
				ReadReg |= (XRFDC_EN_I_IQ | XRFDC_EN_Q_IQ);
				ReadReg &= ~XRFDC_SEL_I_IQ_MASK;
				ReadReg |= XRFDC_I_IQ_COS_MINSIN;
				ReadReg &= ~XRFDC_SEL_Q_IQ_MASK;
				ReadReg |= XRFDC_Q_IQ_SIN_COS;
				ReadReg |= XRFDC_FINE_MIX_SCALE_MASK;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
											ReadReg);
			} else {
				/* Fine mixer mode is OFF */
				ReadReg = 0x0;
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET,
														ReadReg);
			}

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_MXR_MODE_OFFSET);
			if (Mixer_Settings->FineMixerScale ==
						XRFDC_MIXER_SCALE_1P0) {
				ReadReg |= XRFDC_FINE_MIX_SCALE_MASK;
				InstancePtr->UpdateMixerScale = 0x1U;
			} else if (Mixer_Settings->FineMixerScale ==
					XRFDC_MIXER_SCALE_0P7) {
				ReadReg &= ~XRFDC_FINE_MIX_SCALE_MASK;
				InstancePtr->UpdateMixerScale = 0x1U;
			} else
				InstancePtr->UpdateMixerScale = 0x0U;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_MXR_MODE_OFFSET, ReadReg);

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_NCO_UPDT_OFFSET);
			ReadReg &= ~XRFDC_NCO_UPDT_MODE_MASK;
			ReadReg |= Mixer_Settings->EventSource;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_UPDT_OFFSET,
										ReadReg);
			if (Mixer_Settings->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
				if (Type == XRFDC_ADC_TILE) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 1U); /* XRFDC_UPDT_EVNT_NCO_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET, ReadReg);
				} else {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 1U); /* XRFDC_UPDT_EVNT_NCO_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET, ReadReg);
				}
			}

			/* Update the instance with new values */
			Mixer_Config->EventSource = Mixer_Settings->EventSource;
			Mixer_Config->PhaseOffset = Mixer_Settings->PhaseOffset;
			Mixer_Config->FineMixerMode = Mixer_Settings->FineMixerMode;
			Mixer_Config->CoarseMixFreq = Mixer_Settings->CoarseMixFreq;
			Mixer_Config->Freq = Mixer_Settings->Freq;
			Mixer_Config->CoarseMixMode =
				Mixer_Settings->CoarseMixMode;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* The API returns back Mixer/NCO settings to the caller.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Mixer_Settings Pointer to the XRFdc_Mixer_Settings structure
*			in which the Mixer/NCO settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		FineMixerScale in Mixer_Settings structure can have 3 values.
*		XRFDC_MXR_SCALE_* represents the valid return values.
*		XRFDC_MXR_SCALE_NO_CHANGE - Default driver implementation.
*		XRFDC_MXR_SCALE_ONE - If fine Mixer Scale is set to 1 by user.
*		XRFDC_MXR_SCALE_ZERO_DOT_SEVEN - If fine mixer scale is set
*		to 0.7 by user.
*
******************************************************************************/
int XRFdc_GetMixerSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, XRFdc_Mixer_Settings * Mixer_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u64 ReadReg;
	u64 ReadReg_Mix1;
	double SamplingRate;
	s64 Freq;
	s32 PhaseOffset;
	u32 Block;
	u8 CalibrationMode;
	XRFdc_Mixer_Settings *Mixer_Config;
	u32 NyquistZone;
	double NCOFreq;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Mixer_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)  &&
				(Type == XRFDC_ADC_TILE)) {
		if (Block_Id == 1U)
			Block_Id = 3U;
		if (Block_Id == 0U)
			Block_Id = 1U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
		SamplingRate = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate;
		Mixer_Config = &InstancePtr->ADC_Tile[Tile_Id].
			ADCBlock_Digital_Datapath[Block_Id].Mixer_Settings;
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
		SamplingRate = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate;
		Mixer_Config = &InstancePtr->DAC_Tile[Tile_Id].
			DACBlock_Digital_Datapath[Block_Id].Mixer_Settings;
	}

	if (SamplingRate <= 0) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Incorrect Sampling rate "
						"in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Incorrect Sampling rate "
						"in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	}

	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	}
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_MXR_CFG0_OFFSET);
	ReadReg &= XRFDC_MIX_CFG0_MASK;
	ReadReg_Mix1 = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_MXR_CFG1_OFFSET);
	ReadReg_Mix1 &= XRFDC_MIX_CFG1_MASK;
	Mixer_Settings->CoarseMixFreq = 0x20;
	if (ReadReg == XRFDC_CRSE_MIX_BYPASS) {
		if (ReadReg_Mix1 == XRFDC_CRSE_MIX_BYPASS) {
			Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_BYPASS;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if (ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_BYPASS;
			Mixer_Settings->CoarseMixMode =
				XRFDC_COARSE_MIX_MODE_R2C;
		}
	}
	if ((InstancePtr->ADC4GSPS != XRFDC_ADC_4GSPS) ||
			(Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 ==
			XRFDC_CRSE_MIX_I_Q_FSBYTWO) &&
			(ReadReg ==
			XRFDC_CRSE_MIX_I_Q_FSBYTWO)) {
			Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if ((ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) &&
				(ReadReg ==
				XRFDC_CRSE_MIX_I_Q_FSBYTWO)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_R2C;
		}
	} else {
		if (ReadReg == XRFDC_CRSE_4GSPS_ODD_FSBYTWO) {
			if (ReadReg_Mix1 ==
					XRFDC_CRSE_4GSPS_ODD_FSBYTWO) {
				Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
				Mixer_Settings->CoarseMixMode =
						XRFDC_COARSE_MIX_MODE_C2C_C2R;
			} else if (ReadReg_Mix1 == XRFDC_CRSE_MIX_OFF) {
				Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
				Mixer_Settings->CoarseMixMode =
						XRFDC_COARSE_MIX_MODE_R2C;
			}
		}
	}

	if ((InstancePtr->ADC4GSPS != XRFDC_ADC_4GSPS) ||
			(Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 ==
			XRFDC_CRSE_MIX_Q_FSBYFOUR) &&
			(ReadReg ==
			XRFDC_CRSE_MIX_I_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if ((ReadReg_Mix1 ==
			XRFDC_CRSE_MIX_R_Q_FSBYFOUR) &&
			(ReadReg ==
					XRFDC_CRSE_MIX_R_I_MINFSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_R2C;
		}
	} else {
		if ((ReadReg ==
			XRFDC_CRSE_MIX_I_ODD_FSBYFOUR) &&
				(ReadReg_Mix1 ==
				XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if ((ReadReg ==
				XRFDC_CRSE_MIX_OFF) &&
					(ReadReg_Mix1 ==
					XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
				XRFDC_COARSE_MIX_MODE_R2C;
		}

	}

	if ((InstancePtr->ADC4GSPS != XRFDC_ADC_4GSPS) ||
			(Type == XRFDC_DAC_TILE)) {
		if ((ReadReg_Mix1 ==
				XRFDC_CRSE_MIX_I_FSBYFOUR) &&
			(ReadReg ==
					XRFDC_CRSE_MIX_Q_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if ((ReadReg_Mix1 ==
				XRFDC_CRSE_MIX_R_Q_MINFSBYFOUR) &&
			(ReadReg ==
					XRFDC_CRSE_MIX_R_I_MINFSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_R2C;
		}
	} else {
		if ((ReadReg ==
				XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR) &&
				(ReadReg_Mix1 ==
					XRFDC_CRSE_MIX_I_ODD_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
		} else if ((ReadReg ==
				XRFDC_CRSE_MIX_OFF) &&
				(ReadReg_Mix1 ==
					XRFDC_CRSE_MIX_I_ODD_FSBYFOUR)) {
			Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			Mixer_Settings->CoarseMixMode =
				XRFDC_COARSE_MIX_MODE_R2C;
		}

	}

	if ((ReadReg == XRFDC_CRSE_MIX_OFF) && (ReadReg_Mix1 ==
			XRFDC_CRSE_MIX_OFF)) {
		Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_OFF;
		Mixer_Settings->CoarseMixMode =
					XRFDC_COARSE_MIX_MODE_C2C_C2R;
	}
	if (Mixer_Settings->CoarseMixFreq == 0x20) {
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR,
		"\n Coarse mixer settings not match any of the modes %s\r\n",
			__func__);
#else
		metal_log(METAL_LOG_ERROR,
		"\n Coarse mixer settings not match any of the modes %s\r\n",
			__func__);
#endif
	}

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
							XRFDC_MXR_MODE_OFFSET);
	ReadReg &= (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK);
	if (ReadReg == 0xF)
		Mixer_Settings->FineMixerMode =
				XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX;
	else if (ReadReg == 0x3)
		Mixer_Settings->FineMixerMode =
				XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL;
	else if (ReadReg == 0x5)
		Mixer_Settings->FineMixerMode =
				XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX;
	else if (ReadReg == 0x0)
		Mixer_Settings->FineMixerMode = XRFDC_FINE_MIXER_MOD_OFF;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
				XRFDC_MXR_MODE_OFFSET);
	ReadReg &= XRFDC_FINE_MIX_SCALE_MASK;
	if (InstancePtr->UpdateMixerScale == 0x0U)
		Mixer_Settings->FineMixerScale =
				XRFDC_MIXER_SCALE_AUTO;
	else if ((ReadReg != 0U) &&
			(InstancePtr->UpdateMixerScale == 0x1U))
		Mixer_Settings->FineMixerScale =
				XRFDC_MIXER_SCALE_1P0;
	else if (InstancePtr->UpdateMixerScale == 0x1U)
		Mixer_Settings->FineMixerScale =
				XRFDC_MIXER_SCALE_0P7;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_NCO_PHASE_UPP_OFFSET);
	PhaseOffset = ReadReg << 16;
	PhaseOffset |= XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_NCO_PHASE_LOW_OFFSET);
	PhaseOffset &= XRFDC_NCO_PHASE_MASK;
	PhaseOffset = ((PhaseOffset << 14) >> 14);
	Mixer_Settings->PhaseOffset = ((PhaseOffset * 180.0) /
						XRFDC_NCO_PHASE_MULTIPLIER);

	Freq = 0;
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_NCO_FQWD_UPP_OFFSET);
	Freq = ReadReg << 32;
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_NCO_FQWD_MID_OFFSET);
	Freq |= ReadReg << 16;
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_NCO_FQWD_LOW_OFFSET);
	Freq |= ReadReg;
	Freq &= XRFDC_NCO_FQWD_MASK;
	Freq = (Freq << 16) >> 16;
	if (Freq < 0)
		Mixer_Settings->Freq = ((Freq * (SamplingRate * 1000.0)) /
					XRFDC_NCO_FREQ_MIN_MULTIPLIER);
	else
		Mixer_Settings->Freq = ((Freq * (SamplingRate * 1000.0)) /
					XRFDC_NCO_FREQ_MULTIPLIER);
	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
			XRFDC_NCO_UPDT_OFFSET);
	Mixer_Settings->EventSource = ReadReg & XRFDC_NCO_UPDT_MODE_MASK;

	NCOFreq = Mixer_Config->Freq;
	if (Type == XRFDC_ADC_TILE) {
		Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id,
						Block, &CalibrationMode);
		if (Status != XRFDC_SUCCESS)
			return XRFDC_FAILURE;

		if (CalibrationMode == XRFDC_CALIB_MODE1) {
			if (Mixer_Settings->CoarseMixFreq ==
					XRFDC_COARSE_MIX_BYPASS)
				Mixer_Settings->CoarseMixFreq =
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO;
			else if (Mixer_Settings->CoarseMixFreq ==
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR)
				Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
			else if (Mixer_Settings->CoarseMixFreq ==
					XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO)
				Mixer_Settings->CoarseMixFreq =
						XRFDC_COARSE_MIX_BYPASS;
			else if (Mixer_Settings->CoarseMixFreq ==
				XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR)
				Mixer_Settings->CoarseMixFreq =
				XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
			NCOFreq = (Mixer_Config->Freq -
					((SamplingRate * 1000) / 2.0));
		}
	}

	if ((NCOFreq > ((SamplingRate * 1000) / 2.0)) ||
				(NCOFreq < -((SamplingRate * 1000) / 2.0))) {
		if (Type == XRFDC_ADC_TILE)
			Status = XRFdc_GetNyquistZone(InstancePtr,
				XRFDC_ADC_TILE, Tile_Id, Block, &NyquistZone);
		else
			Status = XRFdc_GetNyquistZone(InstancePtr,
				XRFDC_DAC_TILE, Tile_Id, Block, &NyquistZone);
		if (Status != XRFDC_SUCCESS)
			return XRFDC_FAILURE;

		if ((NyquistZone == XRFDC_EVEN_NYQUIST_ZONE) &&
				(Mixer_Settings->Freq != 0))
			Mixer_Settings->Freq *= -1;

		do {
			if (NCOFreq < -((SamplingRate * 1000) / 2.0)) {
				NCOFreq +=  (SamplingRate * 1000);
				Mixer_Settings->Freq -= (SamplingRate * 1000);
			}
			if (NCOFreq > ((SamplingRate * 1000) / 2.0)) {
				NCOFreq -= (SamplingRate * 1000);
				Mixer_Settings->Freq += (SamplingRate * 1000);
			}
		} while ((NCOFreq > ((SamplingRate * 1000) / 2.0)) ||
				(NCOFreq < -((SamplingRate * 1000) / 2.0)));
	}
	if (CalibrationMode == XRFDC_CALIB_MODE1)
		Mixer_Settings->Freq += (SamplingRate * 1000) / 2.0;

	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
* This API is used to update various QMC settings, eg gain, phase, offset etc.
* QMC settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	QMC_Settings is Pointer to the XRFdc_QMC_Settings structure
*			in which the QMC settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		None
*
******************************************************************************/
int XRFdc_SetQMCSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, XRFdc_QMC_Settings * QMC_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	XRFdc_QMC_Settings *QMC_Config;
	u32 BaseAddr;
	s32 PhaseCorrectionFactor;
	u32 GainCorrectionFactor;
	u16 Index;
	u16 NoOfBlocks;


#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(QMC_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].
				DataType == XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = 3U;
			if (Block_Id == 1U) {
				NoOfBlocks = 4U;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
							Tile_Id, Block_Id);
			QMC_Config = &InstancePtr->ADC_Tile[Tile_Id].
							ADCBlock_Analog_Datapath[Index].QMC_Settings;
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			QMC_Config = &InstancePtr->DAC_Tile[Tile_Id].
							DACBlock_Analog_Datapath[Index].QMC_Settings;
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if ((QMC_Settings->EnableGain != 0) &&
						(QMC_Settings->EnableGain != 1)) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid QMC gain option "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid QMC gain option "
												"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((QMC_Settings->EnablePhase != 0) &&
						(QMC_Settings->EnablePhase != 1)) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid QMC phase option "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid QMC phase option "
											"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			if ((QMC_Settings->EventSource != XRFDC_EVNT_SRC_SLICE) &&
				(QMC_Settings->EventSource != XRFDC_EVNT_SRC_TILE) &&
				(QMC_Settings->EventSource != XRFDC_EVNT_SRC_IMMEDIATE) &&
				(QMC_Settings->EventSource != XRFDC_EVNT_SRC_SYSREF) &&
				(QMC_Settings->EventSource != XRFDC_EVNT_SRC_PL) &&
				((QMC_Settings->EventSource != XRFDC_EVNT_SRC_MARKER) ||
						(Type == XRFDC_ADC_TILE))) {
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
					(Type == XRFDC_ADC_TILE) &&
					((QMC_Settings->EventSource == XRFDC_EVNT_SRC_SLICE) ||
					(QMC_Settings->EventSource ==
							XRFDC_EVNT_SRC_IMMEDIATE))) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_QMC_CFG_OFFSET);
			ReadReg &= ~XRFDC_QMC_CFG_EN_GAIN_MASK;
			ReadReg |= QMC_Settings->EnableGain;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET,
								ReadReg);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_QMC_CFG_OFFSET);
			ReadReg &= ~XRFDC_QMC_CFG_EN_PHASE_MASK;
			ReadReg |= (QMC_Settings->EnablePhase << 1U);
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET,
								ReadReg);

			/* Phase Correction factor is applicable to IQ mode only */
			if (((Type == XRFDC_ADC_TILE) &&
				(InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Index].
						DataType == XRFDC_DATA_TYPE_IQ)) ||
				((Type == XRFDC_DAC_TILE) &&
				(InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Index].
						DataType == XRFDC_DATA_TYPE_IQ)))
				{
						ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_QMC_PHASE_OFFSET);
						ReadReg &= ~XRFDC_QMC_PHASE_CRCTN_MASK;
						PhaseCorrectionFactor =
						((QMC_Settings->PhaseCorrectionFactor / 26.5) *
								XRFDC_QMC_PHASE_MULT);

						ReadReg |= PhaseCorrectionFactor;
						XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_QMC_PHASE_OFFSET, ReadReg);
				}

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_QMC_GAIN_OFFSET);
			ReadReg &= ~XRFDC_QMC_GAIN_CRCTN_MASK;
			GainCorrectionFactor = ((QMC_Settings->GainCorrectionFactor *
									XRFDC_QMC_GAIN_MULT) / 2.0);
			ReadReg |= GainCorrectionFactor;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_QMC_GAIN_OFFSET,
														ReadReg);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_QMC_OFF_OFFSET);
			ReadReg &= ~XRFDC_QMC_OFFST_CRCTN_MASK;
			ReadReg |= QMC_Settings->OffsetCorrectionFactor;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_QMC_OFF_OFFSET,
														ReadReg);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_QMC_UPDT_OFFSET);
			ReadReg &= ~XRFDC_QMC_UPDT_MODE_MASK;
			ReadReg |= QMC_Settings->EventSource;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_QMC_UPDT_OFFSET,
										ReadReg);
			if (QMC_Settings->EventSource == XRFDC_EVNT_SRC_IMMEDIATE) {
				if (Type == XRFDC_ADC_TILE) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 2U); /* XRFDC_UPDT_EVNT_QMC_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET, ReadReg);
				} else {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 2U); /* XRFDC_UPDT_EVNT_QMC_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET, ReadReg);
				}
			}
			/* Update the instance with new values */
			QMC_Config->EventSource = QMC_Settings->EventSource;
			QMC_Config->PhaseCorrectionFactor =
									QMC_Settings->PhaseCorrectionFactor;
			QMC_Config->GainCorrectionFactor =
									QMC_Settings->GainCorrectionFactor;
			QMC_Config->OffsetCorrectionFactor =
									QMC_Settings->OffsetCorrectionFactor;
			QMC_Config->EnablePhase = QMC_Settings->EnablePhase;
			QMC_Config->EnableGain = QMC_Settings->EnableGain;
		}
		if ((Type == XRFDC_ADC_TILE) &&
			(InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
					XRFDC_DATA_TYPE_IQ) && (InstancePtr->ADC4GSPS ==
				XRFDC_ADC_4GSPS)) {
			Index += 2U;
		} else {
			Index += 1U;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* QMC settings are returned back to the caller through this API.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	QMC_Settings Pointer to the XRFdc_QMC_Settings structure
*			in which the QMC settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		None
*
******************************************************************************/
int XRFdc_GetQMCSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, XRFdc_QMC_Settings * QMC_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u16 ReadReg;
	s32 PhaseCorrectionFactor;
	u32 GainCorrectionFactor;
	s32 OffsetCorrectionFactor;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(QMC_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(Type == XRFDC_ADC_TILE) &&
				(InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Block_Id].DataType !=
						XRFDC_DATA_TYPE_IQ)) {
		Block_Id = 2U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET);
		QMC_Settings->EnableGain = ReadReg & XRFDC_QMC_CFG_EN_GAIN_MASK;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_QMC_CFG_OFFSET);
		QMC_Settings->EnablePhase = (ReadReg & XRFDC_QMC_CFG_EN_PHASE_MASK) >>
										1U;
		if (Type == XRFDC_ADC_TILE) {
			if (InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Block_Id].DataType ==
								XRFDC_DATA_TYPE_IQ) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_QMC_PHASE_OFFSET);
				PhaseCorrectionFactor = ReadReg & XRFDC_QMC_PHASE_CRCTN_MASK;
				PhaseCorrectionFactor = (PhaseCorrectionFactor >> 11) == 0 ?
										PhaseCorrectionFactor :
										((-1 ^ 0xFFF) | PhaseCorrectionFactor);
				QMC_Settings->PhaseCorrectionFactor = ((PhaseCorrectionFactor *
								26.5) / XRFDC_QMC_PHASE_MULT);
			} else {
				QMC_Settings->PhaseCorrectionFactor = 0U;
			}
		} else {
			if (InstancePtr->DAC_Tile[Tile_Id].
					DACBlock_Digital_Datapath[Block_Id].DataType ==
								XRFDC_DATA_TYPE_IQ) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_QMC_PHASE_OFFSET);
				PhaseCorrectionFactor = ReadReg & XRFDC_QMC_PHASE_CRCTN_MASK;
				PhaseCorrectionFactor = (PhaseCorrectionFactor >> 11) == 0 ?
										PhaseCorrectionFactor :
										((-1 ^ 0xFFF) | PhaseCorrectionFactor);
				QMC_Settings->PhaseCorrectionFactor = ((PhaseCorrectionFactor *
										26.5) / XRFDC_QMC_PHASE_MULT);
			} else {
				QMC_Settings->PhaseCorrectionFactor = 0U;
			}
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_QMC_GAIN_OFFSET);
		GainCorrectionFactor = ReadReg & XRFDC_QMC_GAIN_CRCTN_MASK;
		QMC_Settings->GainCorrectionFactor = ((GainCorrectionFactor * 2.0) /
								XRFDC_QMC_GAIN_MULT);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_QMC_OFF_OFFSET);
		OffsetCorrectionFactor = ReadReg & XRFDC_QMC_OFFST_CRCTN_MASK;
		QMC_Settings->OffsetCorrectionFactor =
				(OffsetCorrectionFactor >> 11) == 0 ? OffsetCorrectionFactor :
				((-1 ^ 0xFFF) | OffsetCorrectionFactor);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_QMC_UPDT_OFFSET);
		QMC_Settings->EventSource = ReadReg & XRFDC_QMC_UPDT_MODE_MASK;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Coarse delay settings passed are used to update the corresponding
* block level registers. Driver structure is updated with the new values.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	CoarseDelay_Settings is Pointer to the XRFdc_CoarseDelay_Settings
*			structure in which the CoarseDelay settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		None
*
******************************************************************************/
int XRFdc_SetCoarseDelaySettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
		u32 Block_Id, XRFdc_CoarseDelay_Settings * CoarseDelay_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;
	XRFdc_CoarseDelay_Settings *CoarseDelay_Config;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoarseDelay_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
							Tile_Id, Block_Id);
			CoarseDelay_Config = &InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Analog_Datapath[Index].CoarseDelay_Settings;
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			CoarseDelay_Config = &InstancePtr->DAC_Tile[Tile_Id].
					DACBlock_Analog_Datapath[Index].CoarseDelay_Settings;
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if (CoarseDelay_Settings->CoarseDelay > 7) {
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested coarse delay not valid "
										"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Requested coarse delay not valid "
										"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((CoarseDelay_Settings->EventSource != XRFDC_EVNT_SRC_SLICE) &&
				(CoarseDelay_Settings->EventSource != XRFDC_EVNT_SRC_TILE) &&
				(CoarseDelay_Settings->EventSource !=
						XRFDC_EVNT_SRC_IMMEDIATE) &&
						(CoarseDelay_Settings->EventSource !=
						XRFDC_EVNT_SRC_SYSREF) &&
						(CoarseDelay_Settings->EventSource != XRFDC_EVNT_SRC_PL) &&
						((CoarseDelay_Settings->EventSource !=
							XRFDC_EVNT_SRC_MARKER) ||(Type == XRFDC_ADC_TILE))) {
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid event source selection "
												"in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
					(Type == XRFDC_ADC_TILE) &&
					((CoarseDelay_Settings->EventSource == XRFDC_EVNT_SRC_SLICE) ||
					(CoarseDelay_Settings->EventSource ==
							XRFDC_EVNT_SRC_IMMEDIATE))) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid Event Source, "
						"event source is not supported in 4GSPS ADC %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}
			if (Type == XRFDC_ADC_TILE) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_CRSE_DLY_CFG_OFFSET);
				ReadReg &= ~XRFDC_CRSE_DLY_CFG_MASK;
				ReadReg |= CoarseDelay_Settings->CoarseDelay;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
							XRFDC_ADC_CRSE_DLY_CFG_OFFSET, ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_CRSE_DLY_UPDT_OFFSET);
				ReadReg &= ~XRFDC_QMC_UPDT_MODE_MASK;
				ReadReg |= CoarseDelay_Settings->EventSource;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_CRSE_DLY_UPDT_OFFSET, ReadReg);
				if (CoarseDelay_Settings->EventSource ==
									XRFDC_EVNT_SRC_IMMEDIATE) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 3U); /* XRFDC_ADC_UPDT_CRSE_DLY_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET, ReadReg);
				}
			} else {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_CRSE_DLY_CFG_OFFSET);
				ReadReg &= ~XRFDC_CRSE_DLY_CFG_MASK;
				ReadReg |= CoarseDelay_Settings->CoarseDelay;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_CRSE_DLY_CFG_OFFSET, ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_CRSE_DLY_UPDT_OFFSET);
				ReadReg &= ~XRFDC_QMC_UPDT_MODE_MASK;
				ReadReg |= CoarseDelay_Settings->EventSource;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_CRSE_DLY_UPDT_OFFSET, ReadReg);
				if (CoarseDelay_Settings->EventSource ==
									XRFDC_EVNT_SRC_IMMEDIATE) {
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET);
					ReadReg &= ~XRFDC_UPDT_EVNT_MASK;
					ReadReg |= (0x1 << 5U); /* XRFDC_DAC_UPDT_CRSE_DLY_MASK */
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_UPDATE_DYN_OFFSET, ReadReg);
				}
			}
			/* Update the instance with new values */
			CoarseDelay_Config->CoarseDelay = CoarseDelay_Settings->CoarseDelay;
			CoarseDelay_Config->EventSource = CoarseDelay_Settings->EventSource;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Coarse delay settings are returned back to the caller.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	CoarseDelay_Settings Pointer to the XRFdc_CoarseDelay_Settings
*			structure in which the Coarse Delay settings are passed.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		None.
*
******************************************************************************/
int XRFdc_GetCoarseDelaySettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
		u32 Block_Id, XRFdc_CoarseDelay_Settings * CoarseDelay_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u16 ReadReg;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CoarseDelay_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(Type == XRFDC_ADC_TILE)) {
		Block_Id = 2U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			CoarseDelay_Settings->CoarseDelay =
						XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_CRSE_DLY_CFG_OFFSET);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_CRSE_DLY_UPDT_OFFSET);
			CoarseDelay_Settings->EventSource = ReadReg &
								XRFDC_QMC_UPDT_MODE_MASK;
		} else {
			CoarseDelay_Settings->CoarseDelay =
						XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_CRSE_DLY_CFG_OFFSET);
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_CRSE_DLY_UPDT_OFFSET);
			CoarseDelay_Settings->EventSource = ReadReg &
								XRFDC_QMC_UPDT_MODE_MASK;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function will trigger the update event for an event.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Event is for which dynamic update event will trigger.
*			XRFDC_EVENT_* defines the different events.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
int XRFdc_UpdateEvent(XRFdc* InstancePtr, u32 Type, int Tile_Id, u32 Block_Id,
								u32 Event)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u32 EventSource = 0;
	u16 NoOfBlocks;
	u16 Index;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
							Tile_Id, Block_Id);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Index);
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if (Event == XRFDC_EVENT_MIXER) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_NCO_UPDT_OFFSET);
				EventSource = ReadReg & XRFDC_NCO_UPDT_MODE_MASK;
			} else if (Event == XRFDC_EVENT_CRSE_DLY) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_DAC_CRSE_DLY_UPDT_OFFSET);
				EventSource = ReadReg & XRFDC_QMC_UPDT_MODE_MASK;
			} else if (Event == XRFDC_EVENT_QMC) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_QMC_UPDT_OFFSET);
				EventSource = ReadReg & XRFDC_QMC_UPDT_MODE_MASK;
			}
			if ((EventSource == XRFDC_EVNT_SRC_SYSREF) ||
					(EventSource == XRFDC_EVNT_SRC_PL) ||
					(EventSource == XRFDC_EVNT_SRC_MARKER)) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid Event Source, this"
						" should be issued external to the driver"
						" %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Invalid Event Source, this"
						" should be issued external to the driver"
						" %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}
			if (Type == XRFDC_ADC_TILE) {
				if (EventSource == XRFDC_EVNT_SRC_SLICE)
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_UPDATE_DYN_OFFSET, 0x1);
				else {
					BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
										XRFDC_HSCOM_ADDR;
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_HSCOM_UPDT_DYN_OFFSET, 0x1);
				}
			} else {
				if (EventSource == XRFDC_EVNT_SRC_SLICE)
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
							XRFDC_DAC_UPDATE_DYN_OFFSET, ReadReg);
				else {
					BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
										XRFDC_HSCOM_ADDR;
					XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_HSCOM_UPDT_DYN_OFFSET, 0x1);
				}
			}
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the decimation factor and also update the FIFO write
* words w.r.t to decimation factor.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	DecimationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only ADC blocks
*
******************************************************************************/
u32 XRFdc_SetDecimationFactor(XRFdc *InstancePtr, int Tile_Id, u32 Block_Id,
					u32 DecimationFactor)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;
	u16 FabricRate;
	u8 DataType;
	u32 Factor;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
					XRFDC_BLOCK_ADDR_OFFSET(Index);
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
					"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
					"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			DataType = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_DECI_CONFIG_OFFSET) &
						XRFDC_DEC_CFG_MASK;
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_DECI_MODE_OFFSET) &
						~XRFDC_DEC_MOD_MASK;
			Factor = DecimationFactor;
			if (DecimationFactor == XRFDC_INTERP_DECIM_4X)
				Factor = 0x3;
			if (DecimationFactor == XRFDC_INTERP_DECIM_8X)
				Factor = 0x4;
			ReadReg |= Factor;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_DECI_MODE_OFFSET, ReadReg);
			FabricRate = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_FABRIC_RATE_OFFSET);
			FabricRate = (FabricRate & XRFDC_ADC_FAB_RATE_WR_MASK);
			if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) ||
					(DataType == 0x2) ||
						(DataType == 0x3)) {
				if (DecimationFactor == XRFDC_INTERP_DECIM_1X)
					FabricRate = 8;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_2X)
					FabricRate = 4;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_4X)
					FabricRate = 2;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_8X)
					FabricRate = 1;
			} else {
				if (DecimationFactor == XRFDC_INTERP_DECIM_1X)
					FabricRate = 4;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_2X)
					FabricRate = 2;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_4X)
					FabricRate = 1;
				else if (DecimationFactor ==
						XRFDC_INTERP_DECIM_8X)
					FabricRate = 1;
			}
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_FABRIC_RATE_OFFSET);
			ReadReg &= ~XRFDC_ADC_FAB_RATE_WR_MASK;
			ReadReg |= FabricRate;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_FABRIC_RATE_OFFSET, ReadReg);
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the divider for clock fabric out.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	FabClkDiv to be set for a tile.
*           XRFDC_FAB_CLK_* defines the valid divider values.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		ADC and DAC Tiles
*
******************************************************************************/
u32 XRFdc_SetFabClkOutDiv(XRFdc *InstancePtr, u32 Type, int Tile_Id,
								u16 FabClkDiv)
{
	s32 Status;
	u32 IsTileEnable;
	u16 ReadReg;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_HSCOM_ADDR;
		IsTileEnable = InstancePtr->RFdc_Config.
				ADCTile_Config[Tile_Id].Enable;
	} else {
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_HSCOM_ADDR;
		IsTileEnable = InstancePtr->RFdc_Config.
				DACTile_Config[Tile_Id].Enable;
	}

	if (IsTileEnable == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested tile not "
					"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested tile not "
					"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else if ((Type == XRFDC_ADC_TILE) &&
			(FabClkDiv == XRFDC_FAB_CLK_DIV1)) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid clock divider "
						"in %s \r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Invalid clock divider "
						"in %s \r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
				XRFDC_HSCOM_CLK_DIV_OFFSET) &
				~XRFDC_FAB_CLK_DIV_MASK;
		ReadReg |= FabClkDiv;
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
				XRFDC_HSCOM_CLK_DIV_OFFSET, ReadReg);
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the interpolation factor and also update the FIFO read
* words w.r.t to interpolation factor.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	InterpolationFactor to be set for DAC block.
*           XRFDC_INTERP_DECIM_* defines the valid values.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only DAC blocks
*
******************************************************************************/
u32 XRFdc_SetInterpolationFactor(XRFdc *InstancePtr, int Tile_Id, u32 Block_Id,
						u32 InterpolationFactor)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 FabricRate;
	u8 DataType;
	u32 Factor;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
					XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
					"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
					"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		DataType = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_ITERP_DATA_OFFSET);
		if ((DataType == XRFDC_ADC_MIXER_MODE_IQ) &&
				(InterpolationFactor ==
					XRFDC_INTERP_DECIM_1X)) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid interpolation "
				"factor in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Invalid interpolation "
				"factor in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
				XRFDC_DAC_INTERP_CTRL_OFFSET) &
				~XRFDC_INTERP_MODE_MASK;
		Factor = InterpolationFactor;
		if (InterpolationFactor == XRFDC_INTERP_DECIM_4X)
			Factor = 0x3;
		if (InterpolationFactor == XRFDC_INTERP_DECIM_8X)
			Factor = 0x4;
		ReadReg |= Factor;
		if (DataType == XRFDC_ADC_MIXER_MODE_IQ)
			ReadReg |= Factor << 4;
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
				XRFDC_DAC_INTERP_CTRL_OFFSET, ReadReg);
		FabricRate = XRFdc_ReadReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_FABRIC_RATE_OFFSET);
		FabricRate = (FabricRate & XRFDC_FAB_RATE_RD_MASK) >> 8;
		if (DataType == XRFDC_ADC_MIXER_MODE_IQ) {
			if (InterpolationFactor == XRFDC_INTERP_DECIM_2X)
				FabricRate = 8;
			else if (InterpolationFactor == XRFDC_INTERP_DECIM_4X)
				FabricRate = 4;
			else if (InterpolationFactor == XRFDC_INTERP_DECIM_8X)
				FabricRate = 2;
		} else {
			if (InterpolationFactor == XRFDC_INTERP_DECIM_1X)
				FabricRate = 8;
			else if (InterpolationFactor == XRFDC_INTERP_DECIM_2X)
				FabricRate = 4;
			else if (InterpolationFactor == XRFDC_INTERP_DECIM_4X)
				FabricRate = 2;
			else if (InterpolationFactor == XRFDC_INTERP_DECIM_8X)
				FabricRate = 1;
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_FABRIC_RATE_OFFSET);
		ReadReg &= ~XRFDC_FAB_RATE_RD_MASK;
		ReadReg |= FabricRate << 8;
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_FABRIC_RATE_OFFSET, ReadReg);
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Interpolation factor are returned back to the caller.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	InterpolationFactor Pointer to return the interpolation factor
*			for DAC blocks.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for DAC blocks
*
******************************************************************************/
int XRFdc_GetInterpolationFactor(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id, u32 * InterpolationFactor)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InterpolationFactor != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		*InterpolationFactor = XRFdc_ReadReg16(InstancePtr, BaseAddr,
				XRFDC_DAC_INTERP_CTRL_OFFSET) & XRFDC_INTERP_MODE_I_MASK;
		if (*InterpolationFactor == 0x3)
			*InterpolationFactor = XRFDC_INTERP_DECIM_4X;
		else if (*InterpolationFactor == 0x4)
			*InterpolationFactor = XRFDC_INTERP_DECIM_8X;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decimation factor are returned back to the caller.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	DecimationFactor Pointer to return the Decimation factor
*			for DAC blocks.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for ADC blocks
*
******************************************************************************/
int XRFdc_GetDecimationFactor(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
						u32 * DecimationFactor)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DecimationFactor != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U)) {
		Block_Id = 2U;
	}

	IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
					Tile_Id, Block);
	BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		*DecimationFactor = XRFdc_ReadReg16(InstancePtr, BaseAddr,
				XRFDC_ADC_DECI_MODE_OFFSET) & XRFDC_DEC_MOD_MASK;
		if (*DecimationFactor == 0x3)
			*DecimationFactor = XRFDC_INTERP_DECIM_4X;
		else if (*DecimationFactor == 0x4)
			*DecimationFactor = XRFDC_INTERP_DECIM_8X;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Fabric data rate for the requested DAC block is set by writing to the
* corresponding register. The function writes the number of valid write words
* for the requested DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	FabricWrVldWords is write fabric rate to be set for DAC block.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for DAC blocks
*
******************************************************************************/
int XRFdc_SetFabWrVldWords(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 FabricWrVldWords)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_RATE_OFFSET);
		if (FabricWrVldWords > 16) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested write valid words is Invalid "
										"in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested write valid words is Invalid "
										"in %s\r\n", __func__);
#endif
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		ReadReg &= ~XRFDC_DAC_FAB_RATE_WR_MASK;
		ReadReg |= FabricWrVldWords;
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_RATE_OFFSET, ReadReg);
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* Fabric data rate for the requested ADC block is set by writing to the
* corresponding register. The function writes the number of valid read words
* for the requested ADC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC block number inside the tile. Valid values
*			are 0-3.
* @param	FabricRdVldWords is Read fabric rate to be set for ADC block.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for ADC blocks
*
******************************************************************************/
int XRFdc_SetFabRdVldWords(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 FabricRdVldWords)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);

		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_FABRIC_RATE_OFFSET);
			if (FabricRdVldWords > 8) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested read valid words is "
										"Invalid in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Requested read valid words is "
										"Invalid in %s\r\n", __func__);
#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
			ReadReg &= ~XRFDC_FAB_RATE_RD_MASK;
			ReadReg |= FabricRdVldWords << 8;
			XRFdc_WriteReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_FABRIC_RATE_OFFSET, ReadReg);
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* This API returns the the number of fabric write valid words requested
* for the block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	FabricWrVldWords Pointer to return the fabric data rate for
*			DAC block
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		ADC/DAC blocks
*
******************************************************************************/
int XRFdc_GetFabWrVldWords(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, u32 * FabricWrVldWords)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FabricWrVldWords != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(Type == XRFDC_ADC_TILE)) {
		Block_Id = 2U;
	}
	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		*FabricWrVldWords = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_RATE_OFFSET);
		if (Type == XRFDC_ADC_TILE)
			*FabricWrVldWords &= XRFDC_ADC_FAB_RATE_WR_MASK;
		else
			*FabricWrVldWords &= XRFDC_DAC_FAB_RATE_WR_MASK;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API returns the the number of fabric read valid words requested
* for the block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	FabricRdVldWords Pointer to return the fabric data rate for
*			ADC/DAC block
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		ADC/DAC blocks
*
******************************************************************************/
int XRFdc_GetFabRdVldWords(XRFdc* InstancePtr, u32 Type, int Tile_Id,
						u32 Block_Id, u32 * FabricRdVldWords)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(FabricRdVldWords != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(Type == XRFDC_ADC_TILE)) {
		Block_Id = 2U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		*FabricRdVldWords = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_FABRIC_RATE_OFFSET);
		*FabricRdVldWords = (*FabricRdVldWords & XRFDC_FAB_RATE_RD_MASK) >> 8;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to clear the Sticky bit in threshold config registers.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	ThresholdToUpdate Select which Threshold (Threshold0 or
*           Threshold1 or both) to update.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only ADC blocks
*
******************************************************************************/
int XRFdc_ThresholdStickyClear(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
													u32 ThresholdToUpdate)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = 3U;
			if (Block_Id == 1U) {
				NoOfBlocks = 4U;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Index);
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_0) ||
					(ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD0_CFG_OFFSET);
				ReadReg |= XRFDC_TRSHD0_STIKY_CLR_MASK;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD0_CFG_OFFSET, ReadReg);
			}
			if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_1) ||
					(ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD1_CFG_OFFSET);
				ReadReg |= XRFDC_TRSHD1_STIKY_CLR_MASK;
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD1_CFG_OFFSET, ReadReg);
			}

		}
		if ((InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) && (InstancePtr->ADC4GSPS ==
				XRFDC_ADC_4GSPS)) {
			Index += 2U;
		} else {
			Index += 1U;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API sets the threshold clear mode. The clear mode can be through
* explicit DRP access (manual) or auto clear (QMC gain update event).
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADCC block number inside the tile. Valid values
*			are 0-3.
* @param	ThresholdToUpdate Select which Threshold (Threshold0 or
*           Threshold1 or both) to update.
* @param	ClrMode can be DRP access (manual) or auto clear (QMC gain
*           update event).
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only ADC blocks
*
******************************************************************************/
int XRFdc_SetThresholdClrMode(XRFdc* InstancePtr, int Tile_Id,
							u32 Block_Id, u32 ThresholdToUpdate, u32 ClrMode)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = 3U;
			if (Block_Id == 1U) {
				NoOfBlocks = 4U;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
									XRFDC_BLOCK_ADDR_OFFSET(Index);
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_0) ||
					(ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD0_CFG_OFFSET);
				if (ClrMode == XRFDC_THRESHOLD_CLRMD_MANUAL_CLR) {
					ReadReg &= ~XRFDC_TRSHD0_CLR_MOD_MASK;
				} else {
					ReadReg |= XRFDC_TRSHD0_CLR_MOD_MASK;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_TRSHD0_CFG_OFFSET, ReadReg);
			}
			if ((ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_1) ||
					(ThresholdToUpdate == XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD1_CFG_OFFSET);
				if (ClrMode == XRFDC_THRESHOLD_CLRMD_MANUAL_CLR) {
					ReadReg &= ~XRFDC_TRSHD1_CLR_MOD_MASK;
				} else {
					ReadReg |= XRFDC_TRSHD1_CLR_MOD_MASK;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_TRSHD1_CFG_OFFSET, ReadReg);
			}
		}
		if ((InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) && (InstancePtr->ADC4GSPS ==
				XRFDC_ADC_4GSPS)) {
			Index += 2U;
		} else {
			Index += 1U;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Threshold settings are updated into the relevant registers. Driver structure
* is updated with the new values. There can be two threshold settings:
* threshold0 and threshold1. Both of them are independent of each other.
* The function returns the requested threshold (which can be threshold0,
* threshold1, or both.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Threshold_Settings Pointer through which the register settings for
*			thresholds are passed to the API.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only ADC blocks
*
******************************************************************************/
int XRFdc_SetThresholdSettings(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
						XRFdc_Threshold_Settings * Threshold_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;
	XRFdc_Threshold_Settings * Threshold_Config;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Threshold_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
		if (InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) {
			Index = Block_Id;
			NoOfBlocks = 3U;
			if (Block_Id == 1U) {
				NoOfBlocks = 4U;
			}
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks;) {
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block_Id);
		Threshold_Config = &InstancePtr->ADC_Tile[Tile_Id].
						ADCBlock_Analog_Datapath[Index].Threshold_Settings;
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
						XRFDC_BLOCK_ADDR_OFFSET(Index);
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if ((Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_0) ||
					(Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_BOTH)) {
				if (Threshold_Settings->ThresholdMode[0] > 3) {
#ifdef __MICROBLAZE__
					xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested threshold mode for "
							"threshold0 is invalid in %s\r\n", __func__);
#else
					metal_log(METAL_LOG_ERROR, "\n Requested threshold mode for "
							"threshold0 is invalid in %s\r\n", __func__);
#endif
					Status = XRFDC_FAILURE;
					goto RETURN_PATH;
				}
			}
			if ((Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_1) ||
					(Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_BOTH)) {
				if (Threshold_Settings->ThresholdMode[1] > 3) {
#ifdef __MICROBLAZE__
					xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested threshold mode for "
							"threshold1 is invalid in %s\r\n", __func__);
#else
					metal_log(METAL_LOG_ERROR, "\n Requested threshold mode for "
							"threshold1 is invalid in %s\r\n", __func__);
#endif
					Status = XRFDC_FAILURE;
					goto RETURN_PATH;
				}
			}

			if ((Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_0) ||
					(Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD0_CFG_OFFSET);
				ReadReg &= ~XRFDC_TRSHD0_EN_MOD_MASK;
				ReadReg |= Threshold_Settings->ThresholdMode[0];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD0_CFG_OFFSET, ReadReg);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_AVG_LO_OFFSET,
								(u16)Threshold_Settings->ThresholdAvgVal[0]);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_TRSHD0_AVG_UP_OFFSET,
						(u16)(Threshold_Settings->ThresholdAvgVal[0] >> 16));
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD0_UNDER_OFFSET);
				ReadReg &= ~XRFDC_TRSHD0_UNDER_MASK;
				ReadReg |= Threshold_Settings->ThresholdUnderVal[0];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_UNDER_OFFSET, ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
												XRFDC_ADC_TRSHD0_OVER_OFFSET);
				ReadReg &= ~XRFDC_TRSHD0_OVER_MASK;
				ReadReg |= Threshold_Settings->ThresholdOverVal[0];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_OVER_OFFSET, ReadReg);
				Threshold_Config->ThresholdMode[0] =
								Threshold_Settings->ThresholdMode[0];
				Threshold_Config->ThresholdAvgVal[0] =
								Threshold_Settings->ThresholdAvgVal[0];
				Threshold_Config->ThresholdUnderVal[0] =
								Threshold_Settings->ThresholdUnderVal[0];
				Threshold_Config->ThresholdOverVal[0] =
								Threshold_Settings->ThresholdOverVal[0];
			}

			if ((Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_1) ||
					(Threshold_Settings->UpdateThreshold ==
					XRFDC_UPDATE_THRESHOLD_BOTH)) {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD1_CFG_OFFSET);
				ReadReg &= ~XRFDC_TRSHD1_EN_MOD_MASK;
				ReadReg |= Threshold_Settings->ThresholdMode[1];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD1_CFG_OFFSET, ReadReg);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_AVG_LO_OFFSET,
								(u16)Threshold_Settings->ThresholdAvgVal[1]);
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_TRSHD1_AVG_UP_OFFSET,
						(u16)(Threshold_Settings->ThresholdAvgVal[1] >> 16));
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD1_UNDER_OFFSET);
				ReadReg &= ~XRFDC_TRSHD1_UNDER_MASK;
				ReadReg |= Threshold_Settings->ThresholdUnderVal[1];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_UNDER_OFFSET, ReadReg);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_OVER_OFFSET);
				ReadReg &= ~XRFDC_TRSHD1_OVER_MASK;
				ReadReg |= Threshold_Settings->ThresholdOverVal[1];
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_OVER_OFFSET, ReadReg);
				Threshold_Config->ThresholdMode[1] =
										Threshold_Settings->ThresholdMode[1];
				Threshold_Config->ThresholdAvgVal[1] =
										Threshold_Settings->ThresholdAvgVal[1];
				Threshold_Config->ThresholdUnderVal[1] =
								Threshold_Settings->ThresholdUnderVal[1];
				Threshold_Config->ThresholdOverVal[1] =
								Threshold_Settings->ThresholdOverVal[1];
			}
		}
		if ((InstancePtr->ADC_Tile[Tile_Id].
				ADCBlock_Digital_Datapath[Index].DataType ==
				XRFDC_DATA_TYPE_IQ) && (InstancePtr->ADC4GSPS ==
				XRFDC_ADC_4GSPS)) {
			Index += 2U;
		} else {
			Index += 1U;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Threshold settings are read from the corresponding registers and are passed
* back to the caller. There can be two threshold settings:
* threshold0 and threshold1. Both of them are independent of each other.
* The function returns the requested threshold (which can be threshold0,
* threshold1, or both.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Threshold_Settings Pointer through which the register settings
*			for thresholds are passed back..
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for ADC blocks
*
******************************************************************************/
int XRFdc_GetThresholdSettings(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
						XRFdc_Threshold_Settings * Threshold_Settings)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u16 ReadReg;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Threshold_Settings != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(InstancePtr->ADC_Tile[Tile_Id].
					ADCBlock_Digital_Datapath[Block_Id].DataType !=
						XRFDC_DATA_TYPE_IQ)) {
		Block_Id = 2U;
	}

	IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block);
	BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		Threshold_Settings->UpdateThreshold = XRFDC_UPDATE_THRESHOLD_BOTH;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_CFG_OFFSET);
		Threshold_Settings->ThresholdMode[0] = ReadReg &
										XRFDC_TRSHD0_EN_MOD_MASK;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_CFG_OFFSET);
		Threshold_Settings->ThresholdMode[1] =
								ReadReg & XRFDC_TRSHD1_EN_MOD_MASK;
		Threshold_Settings->ThresholdAvgVal[0] =
						XRFdc_ReadReg16(InstancePtr, BaseAddr,
										XRFDC_ADC_TRSHD0_AVG_LO_OFFSET);
		Threshold_Settings->ThresholdAvgVal[0] |=
						(XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_AVG_UP_OFFSET) << 16);
		Threshold_Settings->ThresholdAvgVal[1] =
						XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_AVG_LO_OFFSET);
		Threshold_Settings->ThresholdAvgVal[1] |=
						(XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_AVG_UP_OFFSET) << 16);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_UNDER_OFFSET);
		Threshold_Settings->ThresholdUnderVal[0] = ReadReg &
								XRFDC_TRSHD0_UNDER_MASK;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_UNDER_OFFSET);
		Threshold_Settings->ThresholdUnderVal[1] =
								ReadReg & XRFDC_TRSHD1_UNDER_MASK;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD0_OVER_OFFSET);
		Threshold_Settings->ThresholdOverVal[0] =
								ReadReg & XRFDC_TRSHD0_OVER_MASK;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TRSHD1_OVER_OFFSET);
		Threshold_Settings->ThresholdOverVal[1] =
								ReadReg & XRFDC_TRSHD1_OVER_MASK;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decoder mode is updated into the relevant registers. Driver structure is
* updated with the new values.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is DAC block number inside the tile. Valid values
*			are 0-3.
* @param	DecoderMode Valid values are 1 (Maximum SNR, for non-
*			randomized decoder), 2 (Maximum Linearity, for randomized decoder)
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only DAC blocks
*
******************************************************************************/
int XRFdc_SetDecoderMode(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 DecoderMode)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 *DecoderModeConfig;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	DecoderModeConfig = &InstancePtr->DAC_Tile[Tile_Id].
								DACBlock_Analog_Datapath[Block_Id].DecoderMode;
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if ((DecoderMode != XRFDC_DECODER_MAX_SNR_MODE)
				&& (DecoderMode != XRFDC_DECODER_MAX_LINEARITY_MODE)) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid decoder mode "
											"in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Invalid decoder mode "
											"in %s\r\n", __func__);
#endif
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_DECODER_CTRL_OFFSET);
		ReadReg &= ~XRFDC_DEC_CTRL_MODE_MASK;
		ReadReg |= DecoderMode;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_DECODER_CTRL_OFFSET,
								ReadReg);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_DECODER_CLK_OFFSET);
		ReadReg &= ~XRFDC_DEC_CTRL_MODE_MASK;
		ReadReg |= DecoderMode;
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
				XRFDC_DAC_DECODER_CLK_OFFSET, ReadReg);
		*DecoderModeConfig = DecoderMode;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Decoder mode is read and returned back.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is DAC block number inside the tile. Valid values
*			are 0-3.
* @param	DecoderMode Valid values are 1 (Maximum SNR, for non-randomized
*			decoder), 2 (Maximum Linearity, for randomized decoder)
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only for DAC blocks
*
******************************************************************************/
int XRFdc_GetDecoderMode(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 *DecoderMode)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u16 ReadReg;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DecoderMode != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_DECODER_CTRL_OFFSET);
		*DecoderMode = ReadReg & XRFDC_DEC_CTRL_MODE_MASK;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Resets the NCO phase of the current block phase accumulator.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		None
*
******************************************************************************/
int XRFdc_ResetNCOPhase(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
							Tile_Id, Block_Id);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			/* DAC */
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_NCO_RST_OFFSET);
			ReadReg &= ~XRFDC_NCO_PHASE_RST_MASK;
			ReadReg |= 0x1U;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_NCO_RST_OFFSET,
								ReadReg);
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;

}

/*****************************************************************************/
/**
*
* User-level API to setup multiband configuration.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param    DigitalDataPathMask is the DataPath mask. First 4 bits represent
*           4 data paths, 1 means enabled and 0 means disabled.
* @param    DataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    DataConverterMask is block enabled mask (input/output driving
*           blocks). 1 means enabled and 0 means disabled.
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_MultiBand(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
		u8 DigitalDataPathMask, u32 DataType, u32 DataConverterMask)
{
	u32 Status;
	u32 Block_Id;
	u32 IsBlockAvail;
	u8 NoOfDataPaths = 0U;
	u32 BlockIndex[4];
	u32 DataPathIndex[4];
	u32 NoOfDataConverters = 0U;
	u32 Mode = 0x0;
	u32 NoOfBlocks = 4;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
				(Type == XRFDC_ADC_TILE))
		NoOfBlocks = 2;
	for (Block_Id = 0; Block_Id < NoOfBlocks; Block_Id++) {
		if (DigitalDataPathMask & (1 << Block_Id)) {
			DataPathIndex[NoOfDataPaths++] = Block_Id;
			IsBlockAvail = (Type == XRFDC_ADC_TILE ?
					XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id) :
					XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id));
			if (IsBlockAvail == 0U) {
				Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
									"available in %s\r\n", __func__);
#else
				metal_log(METAL_LOG_ERROR, "\n Requested block not "
								"available in %s\r\n", __func__);
#endif
				goto RETURN_PATH;
			}
		}
		if (DataConverterMask & (1 << Block_Id)) {
			BlockIndex[NoOfDataConverters++] = Block_Id;
		}
	}

	if (NoOfDataPaths == 1U)
		Mode = XRFDC_SINGLEBAND_MODE;
	else if (NoOfDataPaths == 2U)
		Mode = XRFDC_MULTIBAND_MODE_2X;
	else if (NoOfDataPaths == 4U)
		Mode = XRFDC_MULTIBAND_MODE_4X;

	if ((DataType == XRFDC_MB_DATATYPE_C2C) && (Mode == XRFDC_SINGLEBAND_MODE)) {
		if ((Type == XRFDC_ADC_TILE) && (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)) {
			/* Update the structure values */
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedQData = BlockIndex[1U];
			Block_Id = (DataPathIndex[0] == 0U ? 1U : 0U);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].
							ConnectedIData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].
							ConnectedQData = -1;

			if (DataPathIndex[0] == 1U)
				DataPathIndex[0] = 2U;
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0U], BlockIndex[0U]+2);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+1,
					DataType, BlockIndex[1U], BlockIndex[1U]+2);
			Block_Id = (DataPathIndex[0] == 2U ? 0U : 2U);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, Block_Id,
					DataType, -1, -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, Block_Id+1,
					DataType, -1, -1);
		} else {
			DataPathIndex[1] = BlockIndex[0] + BlockIndex[1] - DataPathIndex[0];
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0], BlockIndex[1]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
					DataType, -1, -1);
			if (Type == XRFDC_ADC_TILE) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1];

				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = -1;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
			} else {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1];

				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = -1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
			}

		}
	} else if (((DataType == XRFDC_MB_DATATYPE_R2C) ||
			(DataType == XRFDC_MB_DATATYPE_C2R)) && (Mode == XRFDC_SINGLEBAND_MODE)) {
		if ((Type == XRFDC_ADC_TILE) && (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)) {
			if (DataPathIndex[0] == 1U)
				DataPathIndex[0] = 2U;
			if (BlockIndex[0] == 1U)
				BlockIndex[0] = 2U;
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+1,
					DataType, BlockIndex[0U]+1, -1);
		}
		XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
				DataType, BlockIndex[0U], -1);
		if (Type == XRFDC_ADC_TILE) {
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedQData = -1;
		} else {
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedQData = -1;
		}
	}

	if ((DataType == XRFDC_MB_DATATYPE_C2C) &&
			((Mode == XRFDC_MULTIBAND_MODE_2X) || (Mode == XRFDC_MULTIBAND_MODE_4X))) {
		if ((Type == XRFDC_ADC_TILE) && (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0U], BlockIndex[0U]+2);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+1,
					DataType, BlockIndex[0U]+1, BlockIndex[0U]+3);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+2,
					DataType, BlockIndex[0U], BlockIndex[0U]+2);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+3,
					DataType, BlockIndex[0U]+1, BlockIndex[0U]+3);
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedQData = BlockIndex[1U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
							ConnectedQData = BlockIndex[1U];
		} else if (NoOfDataPaths == 2U) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0U], BlockIndex[1U]);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
					DataType, BlockIndex[0U], BlockIndex[1U]);
			if (Type == XRFDC_ADC_TILE) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = BlockIndex[1U];
			} else {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = BlockIndex[1U];
			}
		}
		if (NoOfDataPaths == 4U) {
			if (Type == XRFDC_ADC_TILE) {
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
						DataType, BlockIndex[0U], BlockIndex[1U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
						DataType,  BlockIndex[0U], BlockIndex[1U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2],
						DataType, BlockIndex[0U], BlockIndex[1U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3],
						DataType, BlockIndex[0U], BlockIndex[1U]);

				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = BlockIndex[1U];

				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedQData = BlockIndex[1U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedQData = BlockIndex[1U];
			} else {
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
						DataType, DataPathIndex[0], DataPathIndex[1U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
						DataType, DataPathIndex[0U], DataPathIndex[1U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2],
						DataType, DataPathIndex[2U], DataPathIndex[3U]);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3],
						DataType, DataPathIndex[2U], DataPathIndex[3U]);

				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = BlockIndex[1U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = BlockIndex[1U];

				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedIData = DataPathIndex[2U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedQData = DataPathIndex[3U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedIData = DataPathIndex[2U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedQData = DataPathIndex[3U];
			}
		}
	}

	if (((DataType == XRFDC_MB_DATATYPE_R2C) || (DataType == XRFDC_MB_DATATYPE_C2R)) &&
				((Mode == XRFDC_MULTIBAND_MODE_2X) || (Mode == XRFDC_MULTIBAND_MODE_4X))) {
		if ((Type == XRFDC_ADC_TILE) && (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)) {
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
							ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
							ConnectedIData = BlockIndex[0U];
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
							ConnectedQData = -1;
			if (BlockIndex[0] == 1U)
				BlockIndex[0] = 2U;
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0U], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
					DataType, BlockIndex[0U]+1, -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0]+2,
					DataType, BlockIndex[0U], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1]+2,
					DataType, BlockIndex[0U]+1, -1);
		} else if (NoOfDataPaths == 2U) {
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
					DataType, BlockIndex[0], -1);
			XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
					DataType, BlockIndex[0], -1);
			if (Type == XRFDC_ADC_TILE) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = -1;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
			} else {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = -1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
			}

		}
		if (NoOfDataPaths == 4U) {
			if (Type == XRFDC_ADC_TILE) {
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
						DataType, BlockIndex[0], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
						DataType, BlockIndex[0], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2],
						DataType, BlockIndex[0], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3],
						DataType, BlockIndex[0], -1);

				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = -1;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedQData = -1;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedIData = BlockIndex[0U];
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedQData = -1;

			} else {
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[0],
						DataType, DataPathIndex[0], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[1],
						DataType, DataPathIndex[0], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[2],
						DataType, DataPathIndex[2], -1);
				XRFdc_SetSignalFlow(InstancePtr, Type, Tile_Id, Mode, DataPathIndex[3],
						DataType, DataPathIndex[2], -1);

				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedIData = DataPathIndex[0];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[0]].
								ConnectedQData = -1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedIData = DataPathIndex[0];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[1]].
								ConnectedQData = -1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedIData = DataPathIndex[2];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[2]].
								ConnectedQData = -1;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedIData = DataPathIndex[2];
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[DataPathIndex[3]].
								ConnectedQData = -1;
			}
		}
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
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param    Mode is connection mode SB/MB_2X/MB_4X.
* @param    DigitalDataPathId for the requested I or Q data.
* @param    DataType is mixer data type, valid values are XRFDC_MB_DATATYPE_*
* @param    ConnectIData is analog blocks that are connected to
*           DigitalDataPath I.
* @param    ConnectQData is analog blocks that are connected to
*           DigitalDataPath Q.
*
* @return   None
*
* @note		static API used internally.
*
******************************************************************************/
static void XRFdc_SetSignalFlow(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
		u32 Mode, u32 DigitalDataPathId, u32 DataType,
		int ConnectIData, int ConnectQData)
{
	u16 ReadReg;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(DigitalDataPathId);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_SWITCH_MATRX_OFFSET);
		ReadReg &= ~XRFDC_SWITCH_MTRX_MASK;
		if (ConnectIData != -1)
			ReadReg |= ConnectIData << 2;
		if (ConnectQData != -1)
			ReadReg |= ConnectQData;
		if ((DataType == XRFDC_MB_DATATYPE_C2C) &&
				(InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS))
			ReadReg |= XRFDC_SEL_CB_TO_QMC_MASK;
		if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS)
			ReadReg |= XRFDC_SEL_CB_TO_DECI_MASK;

		XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_SWITCH_MATRX_OFFSET, ReadReg);
	} else {
		/* DAC */
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
							XRFDC_BLOCK_ADDR_OFFSET(DigitalDataPathId);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
					XRFDC_DAC_MB_CFG_OFFSET);
		ReadReg &= ~XRFDC_MB_CFG_MASK;
		if (Mode == XRFDC_SINGLEBAND_MODE) {
			if ((u32)ConnectIData == DigitalDataPathId) {
				if (ConnectQData != -1)
					ReadReg |= XRFDC_SB_C2C_BLK0;
				else
					ReadReg |= XRFDC_SB_C2R;
			}
			if ((ConnectIData == -1) && (ConnectQData == -1))
				ReadReg |= XRFDC_SB_C2C_BLK1;
		} else {
			if (Mode == XRFDC_MULTIBAND_MODE_4X)
				ReadReg |= XRFDC_MB_EN_4X_MASK;
			if ((u32)ConnectIData == DigitalDataPathId) {
				if (ConnectQData != -1)
					ReadReg |= XRFDC_MB_C2C_BLK0;
				else
					ReadReg |= XRFDC_MB_C2R_BLK0;
			} else {
				if (ConnectQData != -1)
					ReadReg |= XRFDC_MB_C2C_BLK1;
				else
					ReadReg |= XRFDC_MB_C2R_BLK1;
			}
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_DAC_MB_CFG_OFFSET, ReadReg);
	}
}

/*****************************************************************************/
/**
*
* Enable and Disable the ADC/DAC FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
int XRFdc_SetupFIFO(XRFdc* InstancePtr, u32 Type, int Tile_Id, u8 Enable)
{
	s32 Status;
	u32 IsTileEnable;
	u32 BaseAddr;
	u16 NoOfTiles;
	u16 Index;
	u32 ReadReg;
	int Tile;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	/* An input tile if of -1 selects all tiles */
	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = 4;
	} else {
		NoOfTiles = 1;
	}

	Tile = Tile_Id;
	for (Index = 0U; Index < NoOfTiles; Index++) {

		if (Tile == XRFDC_SELECT_ALL_TILES) {
			Tile_Id = Index;
		}

		if (Type == XRFDC_ADC_TILE) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
										Enable;
		}
		else {
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
			IsTileEnable = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
										Enable;
		}

		if ((IsTileEnable == 0U) && (NoOfTiles == 1)) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested tile not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else if (IsTileEnable == 0U) {
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Tile%d not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_DEBUG, "\n Tile%d not "
							"available in %s\r\n", Tile_Id, __func__);
#endif
			continue;
		}

		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_FIFO_ENABLE);
		if (Enable == 1U) {
			ReadReg = ReadReg & (~XRFDC_FIFO_EN_MASK);
		} else {
			ReadReg = ReadReg | XRFDC_FIFO_EN_MASK;
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_FIFO_ENABLE, ReadReg);
	}
	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Current status of ADC/DAC FIFO.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Enable valid values are 1 (FIFO enable) and 0 (FIFO Disable)
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
int XRFdc_GetFIFOStatus(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 *Enable)
{
	s32 Status;
	u32 IsTileEnable;
	u32 BaseAddr;
	u32 ReadReg;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		IsTileEnable = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
									Enable;
	} else {
		BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
		IsTileEnable = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
									Enable;
	}
	if (IsTileEnable == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Tile not available in %s\r\n",
						__func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Tile not available in %s\r\n",
						__func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_FIFO_ENABLE);
		ReadReg = ReadReg & XRFDC_FIFO_EN_MASK;
		if (ReadReg == 1U)
			*Enable = 0;
		else
			*Enable = 1;
	}

	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get Output Current for DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param    OutputCurr pointer to return the output current.
*
* @return
*		- Return Output Current for DAC block
*
******************************************************************************/
int XRFdc_GetOutputCurr(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id, int *OutputCurr)
{
	s32 Status;
	u32 IsBlockAvail;
	u32 BaseAddr;
	u16 ReadReg_Cfg2;
	u16 ReadReg_Cfg3;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(OutputCurr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg_Cfg2 = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_DAC_MC_CFG2_OFFSET);
		ReadReg_Cfg2 &= XRFDC_DAC_MC_CFG2_OPCSCAS_MASK;
		ReadReg_Cfg3 = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_MC_CFG3_OFFSET);
		ReadReg_Cfg3 &= XRFDC_DAC_MC_CFG3_CSGAIN_MASK;
		if ((ReadReg_Cfg2 == XRFDC_DAC_MC_CFG2_OPCSCAS_32MA) &&
				(ReadReg_Cfg3 == XRFDC_DAC_MC_CFG3_CSGAIN_32MA))
			*OutputCurr = XRFDC_OUTPUT_CURRENT_32MA;
		else if ((ReadReg_Cfg2 == XRFDC_DAC_MC_CFG2_OPCSCAS_20MA) &&
				(ReadReg_Cfg3 == XRFDC_DAC_MC_CFG3_CSGAIN_20MA))
			*OutputCurr = XRFDC_OUTPUT_CURRENT_20MA;
		else if ((ReadReg_Cfg2 == 0x0) && (ReadReg_Cfg3 == 0x0))
			*OutputCurr = 0x0;
		else {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid output "
						"current value %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Invalid output "
						"current value %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		}

	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Set the Nyquist zone.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param    NyquistZone valid values are 1 (Odd),2 (Even).
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
int XRFdc_SetNyquistZone(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 NyquistZone)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;
	u8 CalibrationMode;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	Index = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) &&
			(Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	for (; Index < NoOfBlocks; Index++) {
		if (Type == XRFDC_ADC_TILE) {
			/* ADC */
			IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
							Tile_Id, Block_Id);
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		} else {
			/* DAC */
			IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
								Index);
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Index);
		}
		if (IsBlockAvail == 0U) {
			Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#endif
			goto RETURN_PATH;
		} else {
			if (Type == XRFDC_ADC_TILE) {
				Status = XRFdc_GetCalibrationMode(InstancePtr,
					Tile_Id, Block_Id, &CalibrationMode);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (CalibrationMode == XRFDC_CALIB_MODE1) {
					if (NyquistZone ==
						XRFDC_ODD_NYQUIST_ZONE)
						NyquistZone =
							XRFDC_EVEN_NYQUIST_ZONE;
					else
						NyquistZone =
							XRFDC_ODD_NYQUIST_ZONE;
				}
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
									XRFDC_ADC_TI_TISK_CRL0_OFFSET);
				if ((NyquistZone % 2) == 0U) {
					ReadReg |= XRFDC_TI_TISK_ZONE_MASK;
				} else {
					ReadReg &= ~XRFDC_TI_TISK_ZONE_MASK;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_TI_TISK_CRL0_OFFSET, ReadReg);
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].
								NyquistZone = NyquistZone;
			} else {
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_MC_CFG0_OFFSET);
				if ((NyquistZone % 2) == 0U) {
					ReadReg |= XRFDC_MC_CFG0_MIX_MODE_MASK;
				} else {
					ReadReg &= ~XRFDC_MC_CFG0_MIX_MODE_MASK;
				}
				XRFdc_WriteReg16(InstancePtr, BaseAddr,
								XRFDC_DAC_MC_CFG0_OFFSET, ReadReg);
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Index].
								NyquistZone = NyquistZone;
			}
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Get the Nyquist zone.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param    NyquistZone Pointer to return the Nyquist zone.
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Common API for ADC/DAC blocks
*
******************************************************************************/
int XRFdc_GetNyquistZone(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 *NyquistZone)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Block;
	u8 CalibrationMode;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	Block = Block_Id;
	if ((InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) && (Block_Id == 1U) &&
				(Type == XRFDC_ADC_TILE)) {
		Block_Id = 2U;
	}

	if (Type == XRFDC_ADC_TILE) {
		/* ADC */
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
						Tile_Id, Block);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		/* DAC */
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		if (Type == XRFDC_ADC_TILE) {
			Status = XRFdc_GetCalibrationMode(InstancePtr, Tile_Id,
						Block, &CalibrationMode);
			if (Status != XRFDC_SUCCESS)
				return XRFDC_FAILURE;
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
								XRFDC_ADC_TI_TISK_CRL0_OFFSET);
			*NyquistZone = (ReadReg & XRFDC_TI_TISK_ZONE_MASK) >> 2U;
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
							XRFDC_DAC_MC_CFG0_OFFSET);
			*NyquistZone = (ReadReg & XRFDC_MC_CFG0_MIX_MODE_MASK) >> 1U;
		}
		if (*NyquistZone == 0U) {
			*NyquistZone = XRFDC_ODD_NYQUIST_ZONE;
		} else {
			*NyquistZone = XRFDC_EVEN_NYQUIST_ZONE;
		}
		if (CalibrationMode == XRFDC_CALIB_MODE1) {
			if (*NyquistZone == XRFDC_EVEN_NYQUIST_ZONE)
				*NyquistZone = XRFDC_ODD_NYQUIST_ZONE;
			else
				*NyquistZone = XRFDC_EVEN_NYQUIST_ZONE;
		}
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the Calibration mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param    CalibrationMode valid values are 1 and 2.
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_SetCalibrationMode(XRFdc *InstancePtr, int Tile_Id, u32 Block_Id,
						u8 CalibrationMode)
{
	s32 Status;
	u16 ReadReg;
	u32 BaseAddr;
	u16 Index;
	u16 NoOfBlocks;
	XRFdc_Mixer_Settings Mixer_Settings;
	u32 NyquistZone;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	Index = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		NoOfBlocks = 2U;
		if (Block_Id == 1U) {
			Index = 2U;
			NoOfBlocks = 4U;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	Status = XRFdc_GetMixerSettings(InstancePtr, XRFDC_ADC_TILE,
					Tile_Id, Block_Id, &Mixer_Settings);
	if (Status != XRFDC_SUCCESS)
		return XRFDC_FAILURE;

	Status = XRFdc_GetNyquistZone(InstancePtr, XRFDC_ADC_TILE,
					Tile_Id, Block_Id, &NyquistZone);
	if (Status != XRFDC_SUCCESS)
		return XRFDC_FAILURE;

	for (; Index < NoOfBlocks; Index++) {
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
					XRFDC_BLOCK_ADDR_OFFSET(Index);
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_ADC_TI_DCB_CRL0_OFFSET);
		ReadReg &= ~XRFDC_TI_DCB_MODE_MASK;
		if (CalibrationMode == XRFDC_CALIB_MODE1) {
			if (((Index % 2) != 0U) &&
				(InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS))
				ReadReg |= XRFDC_TI_DCB_MODE1_4GSPS;
			else if (InstancePtr->ADC4GSPS != XRFDC_ADC_4GSPS)
				ReadReg |= XRFDC_TI_DCB_MODE1_2GSPS;
		}
		XRFdc_WriteReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_TI_DCB_CRL0_OFFSET, ReadReg);
		InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Index].
					CalibrationMode = CalibrationMode;
	}

	Status = XRFdc_SetNyquistZone(InstancePtr, XRFDC_ADC_TILE,
					Tile_Id, Block_Id, NyquistZone);
	if (Status != XRFDC_SUCCESS)
		return XRFDC_FAILURE;
	Status = XRFdc_SetMixerSettings(InstancePtr, XRFDC_ADC_TILE,
					Tile_Id, Block_Id, &Mixer_Settings);
	if (Status != XRFDC_SUCCESS)
		return XRFDC_FAILURE;

	(void)BaseAddr;

	Status = XRFDC_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
*
* This API is to get the Calibration mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
* @param    CalibrationMode pointer to get the calibration mode.
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		Only for ADC blocks
*
******************************************************************************/
u32 XRFdc_GetCalibrationMode(XRFdc *InstancePtr, int Tile_Id, u32 Block_Id,
						u8 *CalibrationMode)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;
	u32 Block;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif
	Block = Block_Id;
	if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
		if (Block_Id == 1U)
			Block_Id = 3U;
		if (Block_Id == 0U)
			Block_Id = 1U;
	}

	IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr,
					Tile_Id, Block);
	BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
					XRFDC_BLOCK_ADDR_OFFSET(Block_Id);

	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR,
		"\n Requested block not available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR,
		"\n Requested block not available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
					XRFDC_ADC_TI_DCB_CRL0_OFFSET);
		if ((ReadReg & XRFDC_TI_DCB_MODE_MASK) != 0)
			*CalibrationMode = XRFDC_CALIB_MODE1;
		else
			*CalibrationMode = XRFDC_CALIB_MODE2;
	}

	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to set enable/disable the Inverse-Sinc filter.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Enable valid values are 0(disable) and 1(enable).
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only DAC blocks
*
******************************************************************************/
u32 XRFdc_SetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
								u16 Enable)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_INVSINC_OFFSET);
		ReadReg &= ~XRFDC_EN_INVSINC_MASK;
		ReadReg |= Enable;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_DAC_INVSINC_OFFSET,
								ReadReg);
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is used to get the Inverse-Sinc filter status(enable/disable).
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is DAC block number inside the tile. Valid values
*			are 0-3.
* @param	Enable is a pointer to get the inv-sinc status. valid values
*               are 0(disable) and 1(enable).
*
* @return
*		- XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Block not enabled.
*
* @note		Only DAC blocks
*
******************************************************************************/
u32 XRFdc_GetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
								u16 *Enable)
{
	s32 Status;
	u32 IsBlockAvail;
	u16 ReadReg;
	u32 BaseAddr;

#ifdef __BAREMETAL__
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	if (IsBlockAvail == 0U) {
		Status = XRFDC_FAILURE;
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested block not "
							"available in %s\r\n", __func__);
#else
		metal_log(METAL_LOG_ERROR, "\n Requested block not "
						"available in %s\r\n", __func__);
#endif
		goto RETURN_PATH;
	} else {
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
						XRFDC_DAC_INVSINC_OFFSET);
		*Enable = ReadReg & XRFDC_EN_INVSINC_MASK;
	}
	(void)BaseAddr;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This Prints the offset of the register along with the content. This API is
* meant to be used for debug purposes. It prints to the console the contents
* of registers for the passed Tile_Id. If -1 is passed, it prints the contents
* of the registers for all the tiles for the respective ADC or DAC
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3, and -1.
*
* @return
*			None
*
* @note		None
*
******************************************************************************/
void XRFdc_DumpRegs(XRFdc* InstancePtr, u32 Type, int Tile_Id)
{
	u32 BlockId;
	u32 Offset;
	u32 BaseAddr;
	u32 ReadReg;
	u16 NoOfTiles;
	u16 Index;
	u16 IsBlockAvail;
	u32 Block;
	int Tile;

#ifdef __BAREMETAL__
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
#endif

	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = 4;
	} else {
		NoOfTiles = 1;
	}
	Tile = Tile_Id;
	for (Index = 0U; Index < NoOfTiles; Index++) {
		if (Tile == XRFDC_SELECT_ALL_TILES) {
			Tile_Id = Index;
		}
		for (BlockId = 0; BlockId < 4; BlockId++) {
			if (Type == XRFDC_ADC_TILE) {
				Block = BlockId;
				if (InstancePtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
					if (BlockId == 1U)
						Block = 0U;
					if ((BlockId == 3U) || (BlockId == 2U))
						Block = 1U;
				}
				IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id,
								Block);
				if (IsBlockAvail == 0U) {
					continue;
				}
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_GENERAL, "\n ADC%d%d:: \r\n", __func__);
#else
				metal_log(METAL_LOG_DEBUG, "\n ADC%d%d:: \r\n", Tile_Id, BlockId);
#endif
				BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
						XRFDC_BLOCK_ADDR_OFFSET(BlockId);
				for (Offset = 0x0; Offset <= 0x284; Offset += 0x4) {
					if ((Offset >= 0x24 && Offset <= 0x2C) ||
							(Offset >= 0x48 && Offset <= 0x7C) ||
							(Offset >= 0xAC && Offset <= 0xC4) ||
							(Offset >= 0x114 && Offset <= 0x13C) ||
							(Offset >= 0x188 && Offset <= 0x194) ||
							(Offset >= 0x1B8 && Offset <= 0x1BC) ||
							(Offset >= 0x1D8 && Offset <= 0x1FC) ||
							(Offset >= 0x240 && Offset <= 0x27C))
						continue;
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
#ifdef __MICROBLAZE__
					xdbg_printf(XDBG_DEBUG_GENERAL,
					"\n offset = 0x%x and Value = 0x%x \t",
						__func__);
#else
					metal_log(METAL_LOG_DEBUG,
					"\n offset = 0x%x and Value = 0x%x \t",
						Offset, ReadReg);
#endif
				}
			} else {
				IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id,
										BlockId);
				if (IsBlockAvail == 0U) {
					continue;
				}
#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_GENERAL, "\n DAC%d%d:: \r\n", __func__);
#else
				metal_log(METAL_LOG_DEBUG, "\n DAC%d%d:: \r\n", Tile_Id, BlockId);
#endif
				BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) +
						XRFDC_BLOCK_ADDR_OFFSET(BlockId);
				for (Offset = 0x0; Offset <= 0x24C; Offset += 0x4) {
					if ((Offset >= 0x28 && Offset <= 0x34) ||
							(Offset >= 0x48 && Offset <= 0x7C) ||
							(Offset >= 0xA8 && Offset <= 0xBC) ||
							(Offset >= 0xE4 && Offset <= 0xFC) ||
							(Offset >= 0x16C && Offset <= 0x17C) ||
							(Offset >= 0x198 && Offset <= 0x1BC) ||
							(Offset >= 0x1EC && Offset <= 0x1FC) ||
							(Offset >= 0x204 && Offset <= 0x23C))
						continue;
					ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
#ifdef __MICROBLAZE__
					xdbg_printf(XDBG_DEBUG_GENERAL,
					"\n offset = 0x%x and Value = 0x%x \t",
						__func__);
#else
					metal_log(METAL_LOG_DEBUG,
					"\n offset = 0x%x and Value = 0x%x \t",
					Offset, ReadReg);
#endif
				}
			}
		}
	}
	(void)BaseAddr;
	(void)ReadReg;
}

/*****************************************************************************/
/**
*
* This is a stub for the status callback. The stub is here in case the upper
* layers forget to set the handler.
*
* @param	CallBackRef is a pointer to the upper layer callback reference.
* @param	Type indicates ADC/DAC.
* @param	Tile_Id indicates Tile number (0-3).
* @param	Block_Id indicates Block number (0-3).
* @param	StatusEvent indicates one or more interrupt occurred.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubHandler(void *CallBackRef, u32 Type, int Tile_Id,
								u32 Block_Id, u32 StatusEvent)
{
	(void) ((void *)CallBackRef);
	(void) Type;
	(void) Tile_Id;
	(void) Block_Id;
	(void) StatusEvent;
#ifdef __BAREMETAL__
	Xil_AssertVoidAlways();
#endif
}

/*****************************************************************************/
/**
*
* This function gets Clock source
*
* @param	CallBackRef is a pointer to the upper layer callback reference.
* @param	Type indicates ADC/DAC.
* @param	Tile_Id indicates Tile number (0-3).
* @param	ClockSource Pointer to return the clock source
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		None.
*
******************************************************************************/
u32 XRFdc_GetClockSource(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
								u32 *ClockSource)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Status;
	u32 BlocksAvail;

	if(Type == XRFDC_ADC_TILE) {
		/*
		 * Check is ADC Tile enabled
		 */
		BlocksAvail = XRFdc_GetNoOfADCBlocks(InstancePtr, Tile_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
	} else {
		/*
		 * Check is DAC Tile enabled
		 */
		BlocksAvail = XRFdc_GetNoOfDACBlock(InstancePtr, Tile_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);
	}

	if(BlocksAvail == 0) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested Tile %d not "
						"enabled in %s\r\n", Tile_Id, __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Requested Tile %d not "
						"enabled in %s\r\n", Tile_Id, __func__);
#endif
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
	} else {
		BaseAddr += XRFDC_HSCOM_ADDR;
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1);
		if(ReadReg & XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK){
			*ClockSource = XRFDC_INTERNAL_PLL_CLK;
		} else {
			*ClockSource = XRFDC_EXTERNAL_CLK;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function gets PLL lock status
*
* @param	CallBackRef is a pointer to the upper layer callback reference.
* @param	Type indicates ADC/DAC.
* @param	Tile_Id indicates Tile number (0-3).
* @param	LockStatus Pointer to return the PLL lock status
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		None.
*
******************************************************************************/
u32 XRFdc_GetPLLLockStatus(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
							u32 *LockStatus)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 ClkSrc;
	u32 Status;

	/*
	 * Get Tile clock source information
	 */
	if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &ClkSrc)
								!= XRFDC_SUCCESS) {
#ifdef __MICROBLAZE__
			xdbg_printf(XDBG_DEBUG_ERROR, "\n Get clock source request Tile %d "
						"failed in %s\r\n", Tile_Id, __func__);
#else
			metal_log(METAL_LOG_ERROR, "\n Get clock source request Tile %d "
						"failed in %s\r\n", Tile_Id, __func__);
#endif
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if(ClkSrc == XRFDC_EXTERNAL_CLK) {
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested Tile %d "
					"uses external clock source in %s\r\n",Tile_Id, __func__);
#else
		metal_log(METAL_LOG_DEBUG,  "\n Requested Tile %d "
					"uses external clock source in %s\r\n",Tile_Id, __func__);
#endif
			*LockStatus = XRFDC_PLL_LOCKED;
	} else {

		if(Type == XRFDC_ADC_TILE)
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		else
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);

		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET);
		if(ReadReg & XRFDC_PLL_LOCKED_MASK) {
			*LockStatus = XRFDC_PLL_LOCKED;
		} else {
			*LockStatus = XRFDC_PLL_UNLOCKED;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function used for configuring the internal PLL registers
* based on reference clock and sampling rate
*
* @param	CallBackRef is a pointer to the upper layer callback reference.
* @param	Type indicates ADC/DAC.
* @param	Tile_Id indicates Tile number (0-3).
* @param	RefClkFreq Reference Clock Frequency MHz(50MHz - 1.2GHz)
* @param	SamplingRate Sampling Rate in MHz(0.5- 4 GHz)
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		None.
*
******************************************************************************/
static u32 XRFdc_SetPLLConfig(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
		double RefClkFreq, double SamplingRate)
{
	u32 BaseAddr;
	u32 ReadReg;
	u32 Status;
	u32 FeedbackDiv;
	u32 OutputDiv;
	double CalcSamplingRate;
	double PllFreq;
	double SamplingError;
	u32 Best_FeedbackDiv = 0x0U;
	u32 Best_OutputDiv = 0x0U;
	double Best_Error = 0xFFFFFFFFU;
	u32 DivideMode = 0x0U;
	u32 DivideValue = 0x0U;
	u32 PllFreqIndex = 0x0U;
	u32 FbDivIndex = 0x0U;
	u8 VCOBand = XRFDC_VCO_UPPER_BAND;

	/*
	 * Sweep valid integer values of FeedbackDiv(N) and record a list
	 * of values that fall in the valid VCO range 8.5GHz - 12.8GHz
	 */
	for (FeedbackDiv = PLL_FPDIV_MIN; FeedbackDiv <= PLL_FPDIV_MAX;
			FeedbackDiv++) {

		PllFreq = FeedbackDiv * RefClkFreq;

		if ((PllFreq > VCO_RANGE_MIN) && (PllFreq <= VCO_RANGE_MAX)) {
			/*
			 * Sweep values of OutputDiv(M) to find the output frequency
			 * that best matches the user requested value
			 */

			for (OutputDiv = PLL_DIVIDER_MIN; OutputDiv < PLL_DIVIDER_MAX;
					OutputDiv += 2) {

				CalcSamplingRate = (PllFreq / OutputDiv);

				if (SamplingRate > CalcSamplingRate) {
					SamplingError = SamplingRate - CalcSamplingRate;
				} else {
					SamplingError = CalcSamplingRate - SamplingRate;
				}

				if (Best_Error > SamplingError) {
					Best_FeedbackDiv = FeedbackDiv;
					Best_OutputDiv = OutputDiv;
					Best_Error = SamplingError;
				}
			}

			OutputDiv = 3;
			CalcSamplingRate = (PllFreq / OutputDiv);

			if (SamplingRate > CalcSamplingRate) {
				SamplingError = SamplingRate - CalcSamplingRate;
			} else {
				SamplingError = CalcSamplingRate - SamplingRate;
			}

			if (Best_Error > SamplingError) {
				Best_FeedbackDiv = FeedbackDiv;
				Best_OutputDiv = OutputDiv;
				Best_Error = SamplingError;
			}
		}


		if(Type == XRFDC_ADC_TILE)
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
		else
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);

		BaseAddr += XRFDC_HSCOM_ADDR;

		/*
		 * PLL Static configuration
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_CFG0, 0x80U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_SEED0, 0x111U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SDM_SEED1, 0x11U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VREG, 0x45U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VCO0, 0x5800U);
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_VCO1, 0x08U);

		/*
		 * Setting Reference divisor value to one
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_REFDIV, 0x10);

		/*
		 * Set Feedback divisor value
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_FPDIV,
				Best_FeedbackDiv - 2);

		/*
		 * Set Output divisor value
		 */
		if(Best_OutputDiv == 2) {
			DivideMode = 0x1;
		} else if (Best_OutputDiv == 3){
			DivideMode = 0x2;
			DivideValue = 0x1;
		} else if (Best_OutputDiv >= 4){
			DivideMode = 0x3;
			DivideValue = ((Best_OutputDiv - 4)/2);
		}

		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0);
		ReadReg &= ~XRFDC_PLL_DIVIDER0_MASK;
		ReadReg |=	(DivideMode << XRFDC_PLL_DIVIDER0_SHIFT) | DivideValue;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_DIVIDER0, ReadReg);

		/*
		 * Enable fine sweep
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_CRS2, XRFDC_PLL_CRS2_VAL);

		/*
		 * Set default PLL spare inputs LSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x507);

		/*
		 * Set PLL spare inputs MSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE1, 0x0);

		PllFreq = RefClkFreq * Best_FeedbackDiv;

		if(PllFreq < 9400U) {
			PllFreqIndex = 0U;
			FbDivIndex = 2U;
			if(Best_FeedbackDiv < 21U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			}
			VCOBand = XRFDC_VCO_LOWER_BAND;
		} else if(PllFreq < 10070U) {
			PllFreqIndex = 1U;
			FbDivIndex = 2U;
			if(Best_FeedbackDiv < 18U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			}
			VCOBand = XRFDC_VCO_LOWER_BAND;
		} else if(PllFreq < 10690U) {
			PllFreqIndex = 2U;
			FbDivIndex = 3U;
			if(Best_FeedbackDiv < 18U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 25U) {
				FbDivIndex = 1U;
			} else if(Best_FeedbackDiv < 35U) {
				FbDivIndex = 2U;
			}
			VCOBand = XRFDC_VCO_LOWER_BAND;
		} else if(PllFreq < 10990U) {
			PllFreqIndex = 3U;
			FbDivIndex = 3U;
			if(Best_FeedbackDiv < 19U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 27U) {
				FbDivIndex = 1U;
			} else if(Best_FeedbackDiv < 38U) {
				FbDivIndex = 2U;
			}
			VCOBand = XRFDC_VCO_LOWER_BAND;
		} else if(PllFreq < 11430U) {
			PllFreqIndex = 4U;
			FbDivIndex = 3U;
			if(Best_FeedbackDiv < 19U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 27U) {
				FbDivIndex = 1U;
			} else if(Best_FeedbackDiv < 38U) {
				FbDivIndex = 2U;
			}
			VCOBand = XRFDC_VCO_UPPER_BAND;
		} else if(PllFreq < 12040U) {
			PllFreqIndex = 5U;
			FbDivIndex = 3U;
			if(Best_FeedbackDiv < 20U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 28U) {
				FbDivIndex = 1U;
			} else if(Best_FeedbackDiv < 40U) {
				FbDivIndex = 2U;
			}
			VCOBand = XRFDC_VCO_UPPER_BAND;
		} else if(PllFreq < 12530U) {
			PllFreqIndex = 6U;
			FbDivIndex = 3U;
			if(Best_FeedbackDiv < 23U) {
				FbDivIndex = 0U;
			} else if(Best_FeedbackDiv < 30U) {
				FbDivIndex = 1U;
			} else if(Best_FeedbackDiv < 42U) {
				FbDivIndex = 2U;
			}
			VCOBand = XRFDC_VCO_UPPER_BAND;
		} else if(PllFreq < 20000U) {
			PllFreqIndex = 7U;
			FbDivIndex = 2U;
			if(Best_FeedbackDiv < 20U) {
				FbDivIndex = 0U;
				/*
				 * Set PLL spare inputs LSB
				 */
				XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_SPARE0, 0x577);
			} else if(Best_FeedbackDiv < 39U) {
				FbDivIndex = 1U;
			}
			VCOBand = XRFDC_VCO_UPPER_BAND;
		}

		/*
		 * Disable automatic selection of the VCO
		 */
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_PLL_CRS1);
		ReadReg &= ~XRFDC_PLL_CRS1_VCO_SEL_MASK;
		ReadReg |= VCOBand;
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_CRS1, ReadReg);

		/*
		 * PLL bits for loop filters LSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_LPF0,
				PllTuningMatrix[PllFreqIndex][FbDivIndex][0]);

		/*
		 * PLL bits for loop filters MSB
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_LPF1, XRFDC_PLL_LPF1_VAL);

		/*
		 * Set PLL bits for charge pumps
		 */
		XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_PLL_CHARGEPUMP,
				PllTuningMatrix[PllFreqIndex][FbDivIndex][1]);
	}

	if(Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkDivider = 0x1;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = Best_FeedbackDiv;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.OutputDivider = Best_OutputDiv;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkDivider = 0x1;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.FeedbackDivider = Best_FeedbackDiv;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.OutputDivider = Best_OutputDiv;
	}

	Status = XRFDC_SUCCESS;

	return Status;
}


/*****************************************************************************/
/**
*
* This function used for dynamically switch between internal PLL and
* external clcok source and configuring the internal PLL
*
* @param	CallBackRef is a pointer to the upper layer callback reference
* @param	Type indicates ADC/DAC
* @param	Tile_Id indicates Tile number (0-3)
* @param	Source Clock source internal PLL or external clock source
* @param	RefClkFreq Reference Clock Frequency in MHz(50MHz - 1.2GHz)
* @param	SamplingRate Sampling Rate in MHz(0.5- 6.4 GHz)
*
* @return
*       - XRFDC_SUCCESS if successful.
*       - XRFDC_FAILURE if Tile not enabled.
*
* @note		None.
*
******************************************************************************/
u32 XRFdc_DynamicPLLConfig(XRFdc* InstancePtr, u32 Type, u32 Tile_Id,
				u8 Source, double RefClkFreq, double SamplingRate)
{
	u32 ClkSrc;
	u32 LockStatus;
	u32 Status;
	u32 BaseAddr;
	u32 ReadReg;
	u32 PLLEnable = 0x0;

	/*
	 * Get Tile clock source information
	 */
	if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &ClkSrc)
								!= XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if((Source != XRFDC_INTERNAL_PLL_CLK) &&
			(ClkSrc != XRFDC_INTERNAL_PLL_CLK)) {
#ifdef __MICROBLAZE__
		xdbg_printf(XDBG_DEBUG_ERROR, "\n Requested Tile %d "
					"uses external clock source in %s\r\n",Tile_Id, __func__);
#else
		metal_log(METAL_LOG_DEBUG,  "\n Requested Tile %d "
					"uses external clock source in %s\r\n",Tile_Id, __func__);
#endif
		if(Type == XRFDC_ADC_TILE) {
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate =
						(SamplingRate/1000);
			InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		} else {
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate =
						(SamplingRate/1000);
			InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		}
		Status = XRFDC_SUCCESS;
		goto RETURN_PATH;
	} else {
		/*
		 * Stop the ADC or DAC tile by putting tile in reset state
		 */
		if(XRFdc_Shutdown(InstancePtr, Type, Tile_Id) != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}

		/*
		 * Wait till restart bit clear
		 */
		if(Type == XRFDC_ADC_TILE)
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
		else
			BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);

		while(XRFdc_ReadReg16(InstancePtr,BaseAddr, XRFDC_RESTART_OFFSET) != 0U);

		if(Type == XRFDC_ADC_TILE)
			BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
		else
			BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);

		BaseAddr += XRFDC_HSCOM_ADDR;

		if (Source == XRFDC_INTERNAL_PLL_CLK) {

			if((RefClkFreq < 50) ||  (RefClkFreq > 1200)){
		#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid Reference "
							"clock value in %s\r\n", __func__);
		#else
				metal_log(METAL_LOG_ERROR,  "\n Invalid Reference "
						"clock value in %s\r\n", __func__);
		#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			if((SamplingRate < 500) || (SamplingRate > 6400)){
		#ifdef __MICROBLAZE__
				xdbg_printf(XDBG_DEBUG_ERROR, "\n Invalid sampling "
							"rate value in %s\r\n", __func__);
		#else
				metal_log(METAL_LOG_ERROR,  "\n Invalid sampling "
						"rate value in %s\r\n", __func__);
		#endif
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			/*
			 * Configure the PLL
			 */
			if (XRFdc_SetPLLConfig(InstancePtr, Type, Tile_Id, RefClkFreq,
					SamplingRate) != XRFDC_SUCCESS) {
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}

			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
					XRFDC_CLK_NETWORK_CTRL1);
			ReadReg |= XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1,
					ReadReg);
		} else {
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr,
					XRFDC_CLK_NETWORK_CTRL1);
			ReadReg &= ~XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK;
			XRFdc_WriteReg16(InstancePtr, BaseAddr, XRFDC_CLK_NETWORK_CTRL1,
					ReadReg);
		}

	}

	/*
	 * Re-start the ADC or DAC tile
	 */
	if(XRFdc_StartUp(InstancePtr, Type, Tile_Id) != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (Source == XRFDC_INTERNAL_PLL_CLK) {
		PLLEnable = 0x1;
		/*
		 * Wait for internal PLL to lock
		 */
		do {
			if (XRFdc_GetPLLLockStatus(InstancePtr, Type, Tile_Id,
					&LockStatus) != XRFDC_SUCCESS) {
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			}
		} while (LockStatus != XRFDC_PLL_LOCKED);
	}

	/*
	 * Wait till restart bit clear
	 */
	if (Type == XRFDC_ADC_TILE)
		BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
	else
		BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);

	while (XRFdc_ReadReg16(InstancePtr,BaseAddr, XRFDC_RESTART_OFFSET) != 0U);

	if(Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate =
					(SamplingRate/1000);
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.Enabled = PLLEnable;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate =
					(SamplingRate/1000);
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq = RefClkFreq;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.Enabled = PLLEnable;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/** @} */
