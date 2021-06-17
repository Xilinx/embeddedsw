/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc.c
* @addtogroup rfdc_v11_0
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
*       sg     13/01/18 Added PLL and external clock switch support.
*                       Added API to get PLL lock status.
*                       Added API to get clock source.
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
* 4.0   sk     04/17/18 Corrected Set/Get MixerSettings API description for
*                       FineMixerScale parameter.
*       sk     04/19/18 Enable VCO Auto selection while configuring the clock.
*       sk     04/24/18 Add API to get PLL Configurations.
*       sk     04/24/18 Add API to get the Link Coupling mode.
*       sk     04/28/18 Implement timeouts for PLL Lock, Startup and shutdown.
*       sk     05/30/18 Removed CalibrationMode check for DAC.
*       sk     06/05/18 Updated minimum Ref clock value to 102.40625MHz.
* 5.0   sk     06/25/18 Update DAC min sampling rate to 500MHz and also update
*                       VCO Range, PLL_DIVIDER and PLL_FPDIV ranges.
*       sk     06/25/18 Add XRFdc_GetFabClkOutDiv() API to read fabric clk div.
*                       Add Inline APIs XRFdc_CheckBlockEnabled(),
*                       XRFdc_CheckTileEnabled().
*       sk     07/06/18 Add support to dump HSCOM regs in XRFdc_DumpRegs() API
*       sk     07/12/18 Fixed Multiband crossbar settings in C2C mode.
*       sk     07/19/18 Add MixerType member to MixerSettings structure and 
*                       Update Mixer Settings APIs to consider the MixerType
*                       variable.
*       sk     07/19/18 Add XRFdc_GetMultibandConfig() API to read Multiband
*                       configuration.
*       sk     07/20/18 Update the APIs to check the corresponding section
*                       (Digital/Analog)enable/disable.
*       sk     07/26/18 Fixed Doxygen, coverity warnings.
*       sk     08/03/18 Fixed MISRAC warnings.
*       sk     08/24/18 Move mixer related APIs to xrfdc_mixer.c file.
*                       Define asserts for Linux, Re-arranged XRFdc_RestartIPSM,
*                       XRFdc_CfgInitialize() and XRFdc_MultiBand()  APIs.
*                       Reorganize the code to improve readability and
*                       optimization.
*       sk     09/24/18 Update powerup-state value based on PLL mode in
*                       XRFdc_DynamicPLLConfig() API.
*       sk     10/10/18 Check for DigitalPath enable in XRFdc_GetNyquistZone()
*                       and XRFdc_GetCalibrationMode() APIs for Multiband.
*       sk     10/13/18 Add support to read the REFCLKDIV param from design.
*                       Update XRFdc_SetPLLConfig() API to support range of
*                       REF_CLK_DIV values(1 to 4).
* 5.1   cog    01/29/19 Replace structure reference ADC checks with
*                       function.
*       cog    01/29/19 Added XRFdc_SetDither() and XRFdc_GetDither() APIs.
*       cog    01/29/19 Rename DataType for mixer input to MixerInputDataType
*                       for readability.
*       cog    01/29/19 Refactoring of interpolation and decimation APIs and
*                       changed fabric rate for decimation X8 for non-high speed ADCs.
*       cog    01/29/19 New inline functions to determine max & min sampling rates
*                       rates in PLL range checking.
* 6.0   cog    02/17/19 Added decimation & interpolation modes
*              02/17/19 Added Inverse-Sinc Second Nyquist Zone Support
*       cog    02/17/19 Added new clock Distribution functionality.
*       cog    02/17/19 Refactored to improve delay balancing in clock
*                       distribution.
*       cog    02/17/19 Added delay calculation & metal log messages.
*       cog    02/17/19 Added intratile clock settings.
*       cog    02/17/19 Moved multiband to a new file xrfdc_mb.c
*       cog    02/17/19 Moved clocking functionality to a new file xrfdc_clock.c
*       cog    02/17/19 Added XRFdc_SetIMRPassMode() and XRFdc_SetIMRPassMode() APIs
*       cog    02/17/19 Added XRFdc_SetDACMode() and XRFdc_GetDACMode() APIs
*       cog    02/17/19	Added XRFdc_SetSignalDetector() and XRFdc_GetSignalDetector() APIs.
*       cog    02/17/19 Added XRFdc_DisableCoefficientsOverride(), XRFdc_SetCalCoefficients
*                       and XRFdc_GetCalCoefficients APIs.
*       cog    02/21/19 Added XRFdc_SetCalFreeze() and XRFdc_GetCalFreeze() APIs.
*       cog    04/09/19 Changed calibration coefficient override control register for OCB1.
*       cog    04/15/19 Rename XRFdc_SetDACMode() and XRFdc_GetDACMode() APIs to
*                       XRFdc_SetDataPathMode() and XRFdc_GetDataPathMode() respectively.
*       cog    04/30/19 Made Changes to the bypass calibration functionality to support Gen2
*                       and below.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    07/16/19 Added XRFdc_SetDACOpCurr() API.
*       cog    07/18/19 Added XRFdc_S/GetDigitalStepAttenuator() APIs.
*       cog    07/25/19 Baremetal Region mapping now taken care of in XRFdc_RegisterMetal().
*       cog    07/25/19 Moved XRFDC_PLL_LOCK_DLY_CNT macro to header file.
*       cog    07/26/19 Added new XRFdc_S/GetLegacyCompatibilityMode() APIs.
*       cog    08/02/19 Formatting changes and added a MACRO for the IP generation.
*       cog    09/01/19 Renamed XRFdc_SetDACOpCurr() to XRFdc_SetDACVOP(). Also explicitly set
*                       API to use register values rather than from fabric.
*       cog    09/01/19 Added support for VOP in XRFdc_GetOutputCurr().
*       cog    09/01/19 Rename new XRFdc_S/GetLegacyCompatibilityMode() APIs to
*                       XRFdc_S/GetDACCompMode().
*       cog    09/01/19 Rename XRFdc_S/GetDigitalStepAttenuator() APIs to XRFdc_S/GetDSA().
*                       Also, refactored DSA to use struct and absolute value for Attenuation.
*       cog    09/18/19 Wider mask now needed for DAC Fabric Rate.
*       cog    09/19/19 Calibration mode 1 does not need the frequency shifting workaround
*                       for Gen 3 devices.
*       cog    10/02/19 Added explicit clock divider for datapath modes.
*       cog    10/02/19 The register value for the link coupling is inverted in Gen 3 Devices.
*       cog    10/18/19 DSA was checking DAC tile rather than ADC.
*       cog    10/18/19 Fix GCB read indexing issue with HSADC devices & TSCB coefficients.
* 7.1   cog    11/14/19 Increased ADC fabric read rate to 12 words per cycle for Gen 3 devices.
*       cog    11/15/19 Added calibration mode support for Gen 3 devices and fixed issue with going
*                       to calibration mode 1 when in real mode.
*       cog    11/28/19 Datapath "Mode 2" is now half bandwith with low pass IMR (previously it was
*                       full bandwidth, no IMR, even Nyquist zone).
*       cog    11/28/19 Set defalult compatibility setting when moving to Bypass Mode (Mode 4).
*       cog    11/28/19 Prevent setting non compliant interpolation rates when in the bypass
*                       datapath mode.
*       cog    12/19/19 Update FIFO widths for higher interpolation & decimation factors.
*       cog    12/20/19 Metal log messages are now more descriptive.
*       cog    01/08/20 Added programmable hysteresis for counters ADC signal detector.
*       cog    01/23/20 Calibration modes for Gen 3 were inverted.
*       cog    01/23/20 Fixed offset and bit for GCB calibration override operations in Gen 3 Devices.
*       cog    01/29/20 Fixed metal log typos.
* 8.0   cog    02/10/20 Updated addtogroup.
*       cog    02/20/20 Adjust FIFO delays for clock gated interpolation/decimation rates.
*       cog    03/13/20 Fixed issue where over threshold flag was asserting as soon as the threshold
*                       settings are applied.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Expand range of DSA for production Si.
*       cog    06/24/20 Expand range of VOP for production Si.
*       cog    06/24/20 Support for Dual Band IQ for new bondout.
*       cog    06/24/20 Added observation FIFO and decimation functionality.
*       cog    06/24/20 Added channel powerdon functionality.
*       cog    06/24/20 Refactor to functionaize the FIFO width setting.
*       cog    07/01/20 Fixed metal log print error in XRFdc_SetQMCSettings.
*       cog    08/04/20 Fixed issues with the connected data initialization for multiband.
*       cog    08/28/20 Allow non bypass datapath modes if digital path is enabled.
*       cog    09/01/20 Added warning message if trying to incorrectly set QMC phase and
*                       QMC phase should only be allowed to be set in IQ pair.
*       cog    09/08/20 The Four LSBs of the BLDR Bias Current should be the same as the
*                       four LSBs of the CS Gain.
*       cog    10/05/20 Change shutdown end state for Gen 3 Quad ADCs to reduce power
*                       consumption.
* 9.0   cog    11/25/20 Upversion.
*       cog    11/25/20 Added autocalibration mode for Gen 3 devices.
* 10.0  cog    11/26/20 Refactor and split files.
*       cog    02/10/21 Added custom startup API.
*       cog    05/05/21 Fixed issue where driver was attempting to start ADC 3
*                       for DFE variants.
*       cog    05/05/21 Some dividers and delays need to be set to run caliration at
*                       high sampling rates.
* 11.0  cog    05/26/21 Fixed issue where any end state could be selected in custom
*                       startup API if a start state of 1 was supplied.
*       cog    05/31/21 Upversion information.
*       cog    06/10/21 When setting the powermode, the IP now takes care of the
*                       configuration registers.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xrfdc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static u32 XRFdc_RestartIPSM(XRFdc *InstancePtr, u32 Type, int Tile_Id, u32 Start, u32 End);
static void StubHandler(void *CallBackRefPtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 StatusEvent);
static void XRFdc_ADCInitialize(XRFdc *InstancePtr);
static void XRFdc_DACInitialize(XRFdc *InstancePtr);
static void XRFdc_DACMBConfigInit(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
static void XRFdc_ADCMBConfigInit(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
static void XRFdc_UpdatePLLStruct(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);
static u32 XRFdc_GetADCBlockStatus(XRFdc *InstancePtr, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				   XRFdc_BlockStatus *BlockStatusPtr);
static u32 XRFdc_GetDACBlockStatus(XRFdc *InstancePtr, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				   XRFdc_BlockStatus *BlockStatusPtr);
static void XRFdc_DumpHSCOMRegs(XRFdc *InstancePtr, u32 Type, int Tile_Id);
static void XRFdc_DumpDACRegs(XRFdc *InstancePtr, int Tile_Id);
static void XRFdc_DumpADCRegs(XRFdc *InstancePtr, int Tile_Id);
static u32 XRFdc_WaitForRestartClr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 BaseAddr, u32 End);

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Initializes a specific XRFdc instance such that the driver is ready to use.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    ConfigPtr is a reference to a structure containing information
*           about xrfdc. This function initializes an InstancePtr object
*           for a specific device specified by the contents of Config.
*
* @return
*           - XRFDC_SUCCESS if successful.
*
* @note     The user needs to first call the XRFdc_LookupConfig() API
*           which returns the Configuration structure pointer which is
*           passed as a parameter to the XRFdc_CfgInitialize() API.
*
******************************************************************************/
u32 XRFdc_CfgInitialize(XRFdc *InstancePtr, XRFdc_Config *ConfigPtr)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

#ifdef __BAREMETAL__
	/*for cases where we haven't registered a custom device*/
	if (InstancePtr->io == NULL) {
		InstancePtr->io = (struct metal_io_region *)metal_allocate_memory(sizeof(struct metal_io_region));
		metal_io_init(InstancePtr->io, (void *)(metal_phys_addr_t)ConfigPtr->BaseAddr, &ConfigPtr->BaseAddr,
			      XRFDC_REGION_SIZE, (unsigned)(-1), 0, NULL);
	}
#endif

	/*
	 * Set the values read from the device config and the base address.
	 */
	InstancePtr->BaseAddr = ConfigPtr->BaseAddr;
	InstancePtr->RFdc_Config = *ConfigPtr;
	InstancePtr->ADC4GSPS = ConfigPtr->ADCType;
	InstancePtr->StatusHandler = StubHandler;

	/* Initialize ADC */
	XRFdc_ADCInitialize(InstancePtr);

	/* Initialize DAC */
	XRFdc_DACInitialize(InstancePtr);

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
* Initialize ADC Tiles.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
*
* @return
*           - None.
*
* @note     Static API used to initialize ADC Tiles
*
******************************************************************************/
static void XRFdc_ADCInitialize(XRFdc *InstancePtr)
{
	u32 Tile_Id;
	u32 Block_Id;
	u8 MixerType;

	for (Tile_Id = XRFDC_TILE_ID0; Tile_Id < XRFDC_TILE_ID4; Tile_Id++) {
		InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks = 0U;
		for (Block_Id = XRFDC_BLK_ID0; Block_Id < XRFDC_BLK_ID4; Block_Id++) {
			if (XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks += 1U;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Analog_Datapath[Block_Id].AnalogPathEnabled =
					XRFDC_ANALOGPATH_ENABLE;
			}
			/* Initialize Data Type */
			if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Analog_Config[Block_Id].MixMode ==
			    XRFDC_MIXER_MODE_BYPASS) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].MixerInputDataType =
					InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id]
						.ADCBlock_Digital_Config[Block_Id]
						.MixerInputDataType;
			} else {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].MixerInputDataType =
					InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id]
						.ADCBlock_Analog_Config[Block_Id]
						.MixMode;
			}
			/* Initialize MixerType */
			MixerType = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id]
					    .ADCBlock_Digital_Config[Block_Id]
					    .MixerType;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].Mixer_Settings.MixerType =
				MixerType;

			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].ConnectedIData = -1;
			InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;
			InstancePtr->ADC_Tile[Tile_Id].MultibandConfig =
				InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].MultibandConfig;
			if (XRFdc_IsADCDigitalPathEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].DigitalPathAvailable =
					XRFDC_DIGITALPATH_ENABLE;
				InstancePtr->ADC_Tile[Tile_Id].ADCBlock_Digital_Datapath[Block_Id].DigitalPathEnabled =
					XRFDC_DIGITALPATH_ENABLE;
				/* Initialize ConnectedI/QData, MB Config */
				XRFdc_ADCMBConfigInit(InstancePtr, Tile_Id, Block_Id);
			}
		}

		/* Initialize PLL Structure */
		XRFdc_UpdatePLLStruct(InstancePtr, XRFDC_ADC_TILE, Tile_Id);
	}
}

/*****************************************************************************/
/**
*
* Initialize DAC Tiles.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
*
* @return
*           - None.
*
* @note     Static API used to initialize DAC Tiles
*
******************************************************************************/
static void XRFdc_DACInitialize(XRFdc *InstancePtr)
{
	u32 Tile_Id;
	u32 Block_Id;
	u8 MixerType;

	for (Tile_Id = XRFDC_TILE_ID0; Tile_Id < XRFDC_TILE_ID4; Tile_Id++) {
		InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks = 0U;
		for (Block_Id = XRFDC_BLK_ID0; Block_Id < XRFDC_BLK_ID4; Block_Id++) {
			if (XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks += 1U;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Block_Id].AnalogPathEnabled =
					XRFDC_ANALOGPATH_ENABLE;
			}
			/* Initialize Data Type */
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Analog_Config[Block_Id].MixMode ==
			    XRFDC_MIXER_MODE_BYPASS) {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].MixerInputDataType =
					InstancePtr->RFdc_Config.DACTile_Config[Tile_Id]
						.DACBlock_Digital_Config[Block_Id]
						.MixerInputDataType;
			} else {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].MixerInputDataType =
					XRFDC_DATA_TYPE_IQ;
			}
			/* Initialize MixerType */
			MixerType = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id]
					    .DACBlock_Digital_Config[Block_Id]
					    .MixerType;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].Mixer_Settings.MixerType =
				MixerType;

			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].ConnectedIData = -1;
			InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].ConnectedQData = -1;
			InstancePtr->DAC_Tile[Tile_Id].MultibandConfig =
				InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].MultibandConfig;
			if (XRFdc_IsDACDigitalPathEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].DigitalPathAvailable =
					XRFDC_DIGITALPATH_ENABLE;
				InstancePtr->DAC_Tile[Tile_Id].DACBlock_Digital_Datapath[Block_Id].DigitalPathEnabled =
					XRFDC_DIGITALPATH_ENABLE;
				/* Initialize ConnectedI/QData, MB Config */
				XRFdc_DACMBConfigInit(InstancePtr, Tile_Id, Block_Id);
			}
		}
		/* Initialize PLL Structure */
		XRFdc_UpdatePLLStruct(InstancePtr, XRFDC_DAC_TILE, Tile_Id);
	}
}

/*****************************************************************************/
/**
*
* Initialize Multiband Configuration for DAC Tiles.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - None.
*
* @note     Static API used to initialize DAC MB Config
*
******************************************************************************/
static void XRFdc_DACMBConfigInit(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Analog_Config[Block_Id].MixMode ==
	    XRFDC_MIXER_MODE_C2C) {
		/* Mixer Mode is C2C */
		switch (InstancePtr->DAC_Tile[Tile_Id].MultibandConfig) {
		case XRFDC_MB_MODE_4X:
			if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].NumSlices == XRFDC_DUAL_TILE) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 XRFDC_BLK_ID2);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 XRFDC_BLK_ID1);
			}
			break;
		case XRFDC_MB_MODE_2X_BLK01_BLK23_ALT:
			if (Block_Id < XRFDC_BLK_ID2) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 XRFDC_BLK_ID2);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, -1, -1);
			}

			break;
		case XRFDC_MB_MODE_2X_BLK01_BLK23:
		case XRFDC_MB_MODE_2X_BLK01:
		case XRFDC_MB_MODE_2X_BLK23:
			if ((Block_Id == XRFDC_BLK_ID0) || (Block_Id == XRFDC_BLK_ID1)) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 XRFDC_BLK_ID1);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID2,
							 XRFDC_BLK_ID3);
			}
			break;
		default:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, Block_Id,
						 Block_Id + 1U);
			break;
		}
	} else if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Analog_Config[Block_Id].MixMode == 0x0) {
		/* Mixer Mode is C2R */
		switch (InstancePtr->DAC_Tile[Tile_Id].MultibandConfig) {
		case XRFDC_MB_MODE_4X:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0, -1);
			break;
		case XRFDC_MB_MODE_2X_BLK01_BLK23:
		case XRFDC_MB_MODE_2X_BLK01:
		case XRFDC_MB_MODE_2X_BLK23:
			if ((Block_Id == XRFDC_BLK_ID0) || (Block_Id == XRFDC_BLK_ID1)) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 -1);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID2,
							 -1);
			}
			break;
		default:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, Block_Id, -1);
			break;
		}
	} else {
		/* Mixer Mode is BYPASS */
		XRFdc_SetConnectedIQData(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, Block_Id, -1);
	}
}

/*****************************************************************************/
/**
*
* Initialize Multiband Configuration for ADC Tiles.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Block_Id is ADC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - None.
*
* @note     Static API used to initialize ADC MB Config
*
******************************************************************************/
static void XRFdc_ADCMBConfigInit(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id)
{
	if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Analog_Config[Block_Id].MixMode ==
	    XRFDC_MIXER_MODE_C2C) {
		/* Mixer mode is C2C */
		switch (InstancePtr->ADC_Tile[Tile_Id].MultibandConfig) {
		case XRFDC_MB_MODE_4X:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
						 XRFDC_BLK_ID1);
			break;
		case XRFDC_MB_MODE_2X_BLK01_BLK23:
		case XRFDC_MB_MODE_2X_BLK01:
		case XRFDC_MB_MODE_2X_BLK23:
			if ((Block_Id == XRFDC_BLK_ID0) || (Block_Id == XRFDC_BLK_ID1)) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 XRFDC_BLK_ID1);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID2,
							 XRFDC_BLK_ID3);
			}
			break;
		default:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, Block_Id,
						 Block_Id + 1U);
			break;
		}
	} else if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Analog_Config[Block_Id].MixMode == 0x0) {
		/* Mixer mode is R2C */
		switch (InstancePtr->ADC_Tile[Tile_Id].MultibandConfig) {
		case XRFDC_MB_MODE_4X:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0, -1);
			break;
		case XRFDC_MB_MODE_2X_BLK01_BLK23:
		case XRFDC_MB_MODE_2X_BLK01:
		case XRFDC_MB_MODE_2X_BLK23:
			if ((Block_Id == XRFDC_BLK_ID0) || (Block_Id == XRFDC_BLK_ID1)) {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID0,
							 -1);
			} else {
				XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, XRFDC_BLK_ID2,
							 -1);
			}
			break;
		default:
			XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, Block_Id, -1);
			break;
		}
	} else {
		/* Mixer mode is BYPASS */
		XRFdc_SetConnectedIQData(InstancePtr, XRFDC_ADC_TILE, Tile_Id, Block_Id, Block_Id, -1);
	}
}

/*****************************************************************************/
/**
*
* This API updates PLL Structure.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           - None.
*
* @note     Static API used to initialize PLL Settings for ADC and DAC
*
******************************************************************************/
static void XRFdc_UpdatePLLStruct(XRFdc *InstancePtr, u32 Type, u32 Tile_Id)
{
	if (Type == XRFDC_ADC_TILE) {
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].SamplingRate;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkFreq =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].RefClkFreq;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.Enabled =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].PLLEnable;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.FeedbackDivider =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].FeedbackDiv;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.OutputDivider =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].OutputDiv;
		InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.RefClkDivider =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].RefClkDiv;
	} else {
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].SamplingRate;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkFreq =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].RefClkFreq;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.Enabled =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].PLLEnable;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.FeedbackDivider =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].FeedbackDiv;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.OutputDivider =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].OutputDiv;
		InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.RefClkDivider =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].RefClkDiv;
	}
}

/*****************************************************************************/
/**
*
* The API Restarts the requested tile. It can restart a single tile and
* alternatively can restart all the tiles. Existing register settings are not
* lost or altered in the process. It just starts the requested tile(s).
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_StartUp(XRFdc *InstancePtr, u32 Type, int Tile_Id)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_RestartIPSM(InstancePtr, Type, Tile_Id, XRFDC_SM_STATE1, XRFDC_SM_STATE15);
	return Status;
}

/*****************************************************************************/
/**
*
* The API stops the tile as requested. It can also stop all the tiles if
* asked for. It does not clear any of the existing register settings. It just
* stops the requested tile(s).
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_Shutdown(XRFdc *InstancePtr, u32 Type, int Tile_Id)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_RestartIPSM(InstancePtr, Type, Tile_Id, XRFDC_SM_STATE1, XRFDC_SM_STATE1);
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
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
******************************************************************************/
u32 XRFdc_Reset(XRFdc *InstancePtr, u32 Type, int Tile_Id)
{
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_RestartIPSM(InstancePtr, Type, Tile_Id, XRFDC_SM_STATE0, XRFDC_SM_STATE15);
	return Status;
}

/*****************************************************************************/
/**
*
* The API starts the requested tile from a provided state and runs to the given
* end state. It can restart a single tile and alternatively can restart all the
* tiles.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    StartState Valid values are XRFDC_START_STATE_*.
* @param    Tile_Id Valid values are XRFDC_END_STATE_*.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     If starting from/ending at XRFDC_START_STATE_OFF/XRFDC_END_STATE_OFF,
*           register settings will be wiped.
*
******************************************************************************/
u32 XRFdc_CustomStartUp(XRFdc *InstancePtr, u32 Type, int Tile_Id, u32 StartState, u32 EndState)
{
	u32 Status;
	u32 BaseAddr;
	u32 FGDelay;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if ((StartState != XRFDC_STATE_OFF) && (StartState != XRFDC_STATE_SHUTDOWN) &&
	    (StartState != XRFDC_STATE_CLK_DET) && (StartState != XRFDC_STATE_CAL)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid start state (%u) in %s\r\n", StartState, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((EndState != XRFDC_STATE_OFF) && (EndState != XRFDC_STATE_SHUTDOWN) && (EndState != XRFDC_STATE_PWRUP) &&
	    (EndState != XRFDC_STATE_CLK_DET) && (EndState != XRFDC_STATE_CAL) && (EndState != XRFDC_STATE_FULL)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid end state (%u) in %s\r\n", EndState, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if (StartState > EndState) {
		metal_log(METAL_LOG_ERROR, "\n Start state (%u) can not be higher than end state (%u) in %s\r\n",
			  StartState, EndState, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	/*This is required if re-running calibration from a state later than shutdown in the IPSM*/
	if (Type == XRFDC_ADC_TILE) {
		if ((InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) && (StartState > XRFDC_STATE_SHUTDOWN)) {
			BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
			if (InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate >
			    XRFDC_CAL_DIV_CUTOFF_FREQ(XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id))) {
				FGDelay = (u32)(XRFdc_ReadReg(InstancePtr, BaseAddr, XRFDC_CAL_TMR_MULT_OFFSET) *
						XRFDC_CAL_AXICLK_MULT);
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_DIV_BYP_OFFSET, XRFDC_CAL_DIV_BYP_MASK,
						XRFDC_DISABLED);
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CAL_DLY_OFFSET, FGDelay);
			} else {
				XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_CAL_DIV_BYP_OFFSET, XRFDC_CAL_DIV_BYP_MASK,
						XRFDC_CAL_DIV_BYP_MASK);
				XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_CAL_DLY_OFFSET, 0U);
			}
		}
	} else if ((StartState == XRFDC_STATE_CAL) || (EndState == XRFDC_STATE_CAL)) {
		metal_log(METAL_LOG_ERROR, "\n Invalid state for DAC tiles in %s\r\n", __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Status = XRFdc_RestartIPSM(InstancePtr, Type, Tile_Id, StartState, EndState);
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This Static API will be used to wait for restart bit clears and also check
* for PLL Lock if clock source is internal PLL.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    End is end state of State Machine.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if timeout occurs.
*
* @note     None.
*
******************************************************************************/
static u32 XRFdc_WaitForRestartClr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 BaseAddr, u32 End)
{
	u32 ClkSrc = 0U;
	u32 DelayCount;
	u32 LockStatus = 0U;
	u32 Status;

	/*
	 * Get Tile clock source information
	 */
	if (XRFdc_GetClockSource(InstancePtr, Type, Tile_Id, &ClkSrc) != XRFDC_SUCCESS) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	if ((ClkSrc == XRFDC_INTERNAL_PLL_CLK) && (End > XRFDC_STATE_CLK_DET)) {
		/*
		 * Wait for internal PLL to lock
		 */
		if (XRFdc_GetPLLLockStatus(InstancePtr, Type, Tile_Id, &LockStatus) != XRFDC_SUCCESS) {
			Status = XRFDC_FAILURE;
			goto RETURN_PATH;
		}
		DelayCount = 0U;
		while (LockStatus != XRFDC_PLL_LOCKED) {
			if (DelayCount == XRFDC_PLL_LOCK_DLY_CNT) {
				metal_log(METAL_LOG_ERROR, "\n %s %u timed out at state %u in %s\r\n",
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
					  XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CURRENT_STATE_OFFSET), __func__);
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			} else {
				/* Wait for 1 msec */
#ifdef __BAREMETAL__
				usleep(XRFDC_PLL_LOCK_WAIT);
#else
				metal_sleep_usec(XRFDC_PLL_LOCK_WAIT);
#endif
				DelayCount++;
				(void)XRFdc_GetPLLLockStatus(InstancePtr, Type, Tile_Id, &LockStatus);
			}
		}
	}

	if (End == XRFDC_STATE_FULL) {
		/* Wait till restart bit clear */
		DelayCount = 0U;
		while (XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_RESTART_OFFSET) != 0U) {
			if (DelayCount == XRFDC_RESTART_CLR_DLY_CNT) {
				metal_log(METAL_LOG_ERROR, "\n %s %u timed out at state %u in %s\r\n",
					  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id,
					  XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CURRENT_STATE_OFFSET), __func__);
				Status = XRFDC_FAILURE;
				goto RETURN_PATH;
			} else {
				/* Wait for 1 msec */
#ifdef __BAREMETAL__
				usleep(XRFDC_RESTART_CLR_WAIT);
#else
				metal_sleep_usec(XRFDC_RESTART_CLR_WAIT);
#endif
				DelayCount++;
			}
		}
	} else {
		Status = XRFdc_WaitForState(InstancePtr, Type, Tile_Id, End);
		if (Status == XRFDC_FAILURE) {
			goto RETURN_PATH;
		}
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This function is used to wait for a tile to reach a given state.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type represents ADC or DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    State represents the state which the tile must reach.
*
* @return
*           - XRFDC_SUCCESS if valid.
*           - XRFDC_FAILURE if not valid.
*
******************************************************************************/
u32 XRFdc_WaitForState(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 State)
{
	u32 Status;
	u32 DelayCount;
	u32 TileState;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Tile_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, __func__);
		goto RETURN_PATH;
	}
	TileState = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id), XRFDC_CURRENT_STATE_OFFSET,
				XRFDC_CURRENT_STATE_MASK);
	DelayCount = 0U;
	while (TileState < State) {
		if (DelayCount == XRFDC_WAIT_ATTEMPTS_CNT) {
			Status = XRFDC_FAILURE;
			metal_log(METAL_LOG_ERROR, "\n timeout error in %s[%u] going to state %u in %s\r\n",
				  (Type ? "DAC" : "ADC"), Tile_Id, State, __func__);
			goto RETURN_PATH;
		} else {
			/* Wait for 0.1 msec */
#ifdef __BAREMETAL__
			usleep(XRFDC_STATE_WAIT);
#else
			metal_sleep_usec(XRFDC_STATE_WAIT);
#endif
			DelayCount++;
			TileState = XRFdc_RDReg(InstancePtr, XRFDC_CTRL_STS_BASE(Type, Tile_Id),
						XRFDC_CURRENT_STATE_OFFSET, XRFDC_CURRENT_STATE_MASK);
		}
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
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
* @param    Start is start state of State Machine
* @param    End is end state of State Machine.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     None.
*
******************************************************************************/
static u32 XRFdc_RestartIPSM(XRFdc *InstancePtr, u32 Type, int Tile_Id, u32 Start, u32 End)
{
	u32 Status;
	u32 BaseAddr;
	u16 NoOfTiles;
	u16 Index;
	u32 TileLayout;

	/* An input tile if of -1 selects all tiles */
	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		TileLayout = XRFdc_GetTileLayout(InstancePtr);
		if (TileLayout == XRFDC_3ADC_2DAC_TILES) {
			NoOfTiles = (Type == XRFDC_ADC_TILE) ? XRFDC_TILE_ID3 : XRFDC_TILE_ID2;
		} else {
			NoOfTiles = XRFDC_NUM_OF_TILES4;
		}
		Index = XRFDC_TILE_ID0;
	} else {
		NoOfTiles = Tile_Id + 1;
		Index = Tile_Id;
	}

	for (; Index < NoOfTiles; Index++) {
		BaseAddr = XRFDC_CTRL_STS_BASE(Type, Index);
		Status = XRFdc_CheckTileEnabled(InstancePtr, Type, Index);

		if ((Status != XRFDC_SUCCESS) && (Tile_Id != XRFDC_SELECT_ALL_TILES)) {
			metal_log(METAL_LOG_ERROR, "\n Requested tile (%s %u) not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Index, __func__);
			goto RETURN_PATH;
		} else if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_DEBUG, "\n %s %u not available in %s\r\n",
				  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Index, __func__);
			continue;
		} else {
			/*power saving for Gen 3 Quad ADCs*/
			if ((InstancePtr->RFdc_Config.IPType >= XRFDC_GEN3) &&
			    (XRFdc_IsHighSpeedADC(InstancePtr, Index) == 0U) && (Type != XRFDC_DAC_TILE) &&
			    (End == XRFDC_SM_STATE1)) {
				End = XRFDC_SM_STATE3;
			}
			/* Write Start and End states */
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_RESTART_STATE_OFFSET, XRFDC_PWR_STATE_MASK,
					(Start << XRFDC_RSR_START_SHIFT) | End);

			/* Trigger restart */
			XRFdc_WriteReg(InstancePtr, BaseAddr, XRFDC_RESTART_OFFSET, XRFDC_RESTART_MASK);

			/* Wait for restart bit clear */
			Status = XRFdc_WaitForRestartClr(InstancePtr, Type, Index, BaseAddr, End);
			if (Status != XRFDC_SUCCESS) {
				goto RETURN_PATH;
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
* The API returns the IP status.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    IPStatusPtr Pointer to the XRFdc_IPStatus structure through
*           which the status is returned.
*
* @return
*           - XRFDC_SUCCESS if successful.
*
* @note     None.
*
******************************************************************************/
u32 XRFdc_GetIPStatus(XRFdc *InstancePtr, XRFdc_IPStatus *IPStatusPtr)
{
	u32 Tile_Id;
	u32 Block_Id;
	u32 BaseAddr;
	u16 ReadReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(IPStatusPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	for (Tile_Id = XRFDC_TILE_ID0; Tile_Id < XRFDC_TILE_ID4; Tile_Id++) {
		IPStatusPtr->ADCTileStatus[Tile_Id].BlockStatusMask = 0x0;
		IPStatusPtr->DACTileStatus[Tile_Id].BlockStatusMask = 0x0;
		for (Block_Id = XRFDC_BLK_ID0; Block_Id < XRFDC_BLK_ID4; Block_Id++) {
			if (XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				IPStatusPtr->ADCTileStatus[Tile_Id].IsEnabled = 1;
				IPStatusPtr->ADCTileStatus[Tile_Id].BlockStatusMask |= (1U << Block_Id);
				BaseAddr = XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile_Id);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET);
				IPStatusPtr->ADCTileStatus[Tile_Id].PowerUpState =
					(ReadReg & XRFDC_PWR_UP_STAT_MASK) >> XRFDC_PWR_UP_STAT_SHIFT;
				IPStatusPtr->ADCTileStatus[Tile_Id].PLLState =
					(ReadReg & XRFDC_PLL_LOCKED_MASK) >> XRFDC_PLL_LOCKED_SHIFT;
				IPStatusPtr->ADCTileStatus[Tile_Id].TileState =
					XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CURRENT_STATE_OFFSET);
			}
			if (XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, Block_Id) != 0U) {
				IPStatusPtr->DACTileStatus[Tile_Id].IsEnabled = 1;
				IPStatusPtr->DACTileStatus[Tile_Id].BlockStatusMask |= (1U << Block_Id);
				BaseAddr = XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile_Id);
				IPStatusPtr->DACTileStatus[Tile_Id].TileState =
					XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_CURRENT_STATE_OFFSET);
				ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, XRFDC_STATUS_OFFSET);
				IPStatusPtr->DACTileStatus[Tile_Id].PowerUpState =
					(ReadReg & XRFDC_PWR_UP_STAT_MASK) >> XRFDC_PWR_UP_STAT_SHIFT;
				IPStatusPtr->DACTileStatus[Tile_Id].PLLState =
					(ReadReg & XRFDC_PLL_LOCKED_MASK) >> XRFDC_PLL_LOCKED_SHIFT;
			}
		}
	}

	/*TODO IP state*/

	return XRFDC_SUCCESS;
}

/*****************************************************************************/
/**
*
* The API returns the requested block status.
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3. XRFdc_BlockStatus.
* @param    BlockStatusPtr is Pointer to the XRFdc_BlockStatus structure through
*           which the ADC/DAC block status is returned.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not enabled.
*
* @note     Common API for ADC/DAC blocks.
*
******************************************************************************/
u32 XRFdc_GetBlockStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_BlockStatus *BlockStatusPtr)
{
	u32 Status;
	u32 Block;
	u16 ReadReg;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BlockStatusPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  (Type == XRFDC_ADC_TILE) ? "ADC" : "DAC", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	Block = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) && (Block_Id == XRFDC_BLK_ID1) &&
	    (Type == XRFDC_ADC_TILE)) {
		Block_Id = XRFDC_BLK_ID2;
	}

	BaseAddr = XRFDC_BLOCK_BASE(Type, Tile_Id, Block_Id);
	if (Type == XRFDC_ADC_TILE) {
		Status = XRFdc_GetADCBlockStatus(InstancePtr, BaseAddr, Tile_Id, Block, BlockStatusPtr);
	} else {
		Status = XRFdc_GetDACBlockStatus(InstancePtr, BaseAddr, Tile_Id, Block, BlockStatusPtr);
	}
	if (Status != XRFDC_SUCCESS) {
		goto RETURN_PATH;
	}

	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_CLK_EN_OFFSET, XRFDC_DAT_CLK_EN_MASK);
	if (ReadReg == XRFDC_DAT_CLK_EN_MASK) {
		BlockStatusPtr->DataPathClocksStatus = 0x1U;
	} else {
		BlockStatusPtr->DataPathClocksStatus = 0x0U;
	}

	Status = XRFDC_SUCCESS;

RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* The API returns the requested block status for ADC block
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3. XRFdc_BlockStatus.
* @param    BlockStatus is Pointer to the XRFdc_BlockStatus structure through
*           which the ADC/DAC block status is returned.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not enabled.
*
* @note     Static API for ADC blocks.
*
******************************************************************************/
static u32 XRFdc_GetADCBlockStatus(XRFdc *InstancePtr, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				   XRFdc_BlockStatus *BlockStatusPtr)
{
	u8 FIFOEnable = 0U;
	u32 DecimationFactor = 0U;
	u8 MixerMode;
	u16 ReadReg;
	u32 Status;

	BlockStatusPtr->SamplingFreq = InstancePtr->ADC_Tile[Tile_Id].PLL_Settings.SampleRate;

	/* DigitalDataPathStatus */
	(void)XRFdc_GetFIFOStatus(InstancePtr, XRFDC_ADC_TILE, Tile_Id, &FIFOEnable);
	BlockStatusPtr->DigitalDataPathStatus = FIFOEnable;
	(void)XRFdc_GetDecimationFactor(InstancePtr, Tile_Id, Block_Id, &DecimationFactor);
	BlockStatusPtr->DigitalDataPathStatus |= (DecimationFactor << XRFDC_DIGI_ANALOG_SHIFT4);

	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK));
	switch (ReadReg) {
	case XRFDC_MIXER_MODE_C2C_MASK:
		MixerMode = XRFDC_MIXER_MODE_C2C;
		break;
	case XRFDC_MIXER_MODE_R2C_MASK:
		MixerMode = XRFDC_MIXER_MODE_R2C;
		break;
	case XRFDC_MIXER_MODE_OFF_MASK:
		MixerMode = XRFDC_MIXER_MODE_OFF;
		break;
	default:
		metal_log(METAL_LOG_ERROR, "\n Invalid MixerMode (%u) for ADC %u block %u in %s\r\n", ReadReg, Tile_Id,
			  Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BlockStatusPtr->DigitalDataPathStatus |= (MixerMode << XRFDC_DIGI_ANALOG_SHIFT8);

	/*
	 * Checking ADC block enable for ADC AnalogPath.
	 * This can be changed later,
	 */
	BlockStatusPtr->AnalogDataPathStatus = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block_Id);
	BlockStatusPtr->IsFIFOFlagsEnabled =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_IMR_OFFSET, XRFDC_FAB_IMR_USRDAT_MASK);
	BlockStatusPtr->IsFIFOFlagsAsserted =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_ADC_FABRIC_ISR_OFFSET, XRFDC_FAB_ISR_USRDAT_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* The API returns the requested block status for DAC block
*
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3. XRFdc_BlockStatus.
* @param    BlockStatus is Pointer to the XRFdc_BlockStatus structure through
*           which the DAC block status is returned.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if block not enabled.
*
* @note     Static API for DAC blocks.
*
******************************************************************************/
static u32 XRFdc_GetDACBlockStatus(XRFdc *InstancePtr, u32 BaseAddr, u32 Tile_Id, u32 Block_Id,
				   XRFdc_BlockStatus *BlockStatusPtr)
{
	u32 InterpolationFactor = 0U;
	u32 DecoderMode = 0U;
	u8 MixerMode;
	u16 ReadReg;
	u32 Status;
	u8 FIFOEnable = 0U;

	BlockStatusPtr->SamplingFreq = InstancePtr->DAC_Tile[Tile_Id].PLL_Settings.SampleRate;

	/* DigitalDataPathStatus */
	(void)XRFdc_GetFIFOStatus(InstancePtr, XRFDC_DAC_TILE, Tile_Id, &FIFOEnable);
	BlockStatusPtr->DigitalDataPathStatus = FIFOEnable;
	(void)XRFdc_GetInterpolationFactor(InstancePtr, Tile_Id, Block_Id, &InterpolationFactor);
	BlockStatusPtr->DigitalDataPathStatus |= (InterpolationFactor << XRFDC_DIGI_ANALOG_SHIFT4);
	/* Adder Enable */
	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_MB_CFG_OFFSET, XRFDC_EN_MB_MASK);
	ReadReg = ReadReg >> XRFDC_EN_MB_SHIFT;
	BlockStatusPtr->DigitalDataPathStatus |= (ReadReg << XRFDC_DIGI_ANALOG_SHIFT8);
	ReadReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_MXR_MODE_OFFSET, (XRFDC_EN_I_IQ_MASK | XRFDC_EN_Q_IQ_MASK));
	switch (ReadReg) {
	case XRFDC_MIXER_MODE_C2C_MASK:
		MixerMode = XRFDC_MIXER_MODE_C2C;
		break;
	case XRFDC_MIXER_MODE_C2R_MASK:
		MixerMode = XRFDC_MIXER_MODE_C2R;
		break;
	case XRFDC_MIXER_MODE_OFF_MASK:
		MixerMode = XRFDC_MIXER_MODE_OFF;
		break;
	default:
		metal_log(METAL_LOG_ERROR, "\n Invalid MixerMode (%u) for DAC %u block %u in %s\r\n", ReadReg, Tile_Id,
			  Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BlockStatusPtr->DigitalDataPathStatus |= (MixerMode << XRFDC_DIGI_ANALOG_SHIFT12);

	/* AnalogDataPathStatus */
	BlockStatusPtr->AnalogDataPathStatus = XRFdc_RDReg(
		InstancePtr, BaseAddr, XRFDC_DAC_INVSINC_OFFSET,
		(InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) ? XRFDC_EN_INVSINC_MASK : XRFDC_MODE_INVSINC_MASK);
	(void)XRFdc_GetDecoderMode(InstancePtr, Tile_Id, Block_Id, &DecoderMode);
	BlockStatusPtr->AnalogDataPathStatus |= (DecoderMode << XRFDC_DIGI_ANALOG_SHIFT4);

	/* FIFO Flags status */
	BlockStatusPtr->IsFIFOFlagsEnabled =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_IMR_OFFSET, XRFDC_FAB_IMR_USRDAT_MASK);
	BlockStatusPtr->IsFIFOFlagsAsserted =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_ISR_OFFSET, XRFDC_FAB_ISR_USRDAT_MASK);

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the DAC Datapath mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Mode valid values are 0-3.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if tile not enabled / out of range.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_SetDataPathMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode)
{
	u32 Status = XRFDC_SUCCESS;
	u32 BaseAddr;
	u32 GetClkDiv;
	u32 SetClkDiv;
	u32 GetInterpolationFactor;
	XRFdc_Mixer_Settings MixerSettings;
	u32 FabricRate;
	u32 DatapathReg;
	u32 CurrentMode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		if (Mode == XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO) {
			metal_log(METAL_LOG_ERROR, "\n Bypass not valid as DAC %u block %u not available in %s\r\n",
				  Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}
		Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id,
				  __func__);
			goto RETURN_PATH;
		}
	}

	SetClkDiv = XRFDC_CLK_DIV_DP_OTHER_MODES;
	FabricRate = XRFDC_FAB_RATE_8;
	switch (Mode) {
	case XRFDC_DATAPATH_MODE_DUC_0_FSDIVTWO:
		DatapathReg = XRFDC_DAC_INT_MODE_FULL_BW;
		SetClkDiv = XRFDC_CLK_DIV_DP_FIRST_MODE;
		break;
	case XRFDC_DATAPATH_MODE_DUC_0_FSDIVFOUR:
		DatapathReg = XRFDC_DAC_INT_MODE_HALF_BW_IMR;
		break;
	case XRFDC_DATAPATH_MODE_FSDIVFOUR_FSDIVTWO:
		DatapathReg = XRFDC_DAC_INT_MODE_HALF_BW_IMR;
		DatapathReg |= (XRFDC_DAC_IMR_MODE_HIGHPASS << XRFDC_DATAPATH_IMR_SHIFT);
		break;
	case XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO:
		DatapathReg = XRFDC_DAC_INT_MODE_FULL_BW_BYPASS;
		FabricRate = XRFDC_FAB_RATE_16;
		break;
	default:
		metal_log(METAL_LOG_ERROR, "\n Invalid Mode value in (%u) for DAC %u block %u in %s\r\n", Mode, Tile_Id,
			  Block_Id, __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);

	/*
	Interpolation factor, mixer settings and fabric rate needs to be set if going to Mode 4.
	Fabric rate needs to be set if going from mode 4 to another Mode.
	*/
	if ((Mode == XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO)) {
		Status = XRFdc_GetMixerSettings(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, &MixerSettings);
		if (Status != XRFDC_SUCCESS) {
			metal_log(METAL_LOG_ERROR, "\n Failed to get mixer settings for DAC %u block %u in %s\r\n",
				  Tile_Id, Block_Id, __func__);
			goto RETURN_PATH;
		}

		if (MixerSettings.MixerMode != XRFDC_MIXER_MODE_R2R) {
			MixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
			MixerSettings.MixerMode = XRFDC_MIXER_MODE_R2R;
			MixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
			metal_log(
				METAL_LOG_WARNING,
				"\n Setting mixer mode to remain compatible with datapath mode for DAC %u block %u (R2R) in %s\r\n",
				Tile_Id, Block_Id, __func__);

			Status = XRFdc_SetMixerSettings(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id, &MixerSettings);
			if (Status != XRFDC_SUCCESS) {
				metal_log(METAL_LOG_ERROR,
					  "\n Failed to set mixer settings for DAC %u block %u in %s\r\n", Tile_Id,
					  Block_Id, __func__);
				goto RETURN_PATH;
			}
		}

		GetInterpolationFactor =
			XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET, XRFDC_INTERP_MODE_I_MASK_EXT);
		if (GetInterpolationFactor != XRFDC_INTERP_DECIM_1X) {
			metal_log(
				METAL_LOG_WARNING,
				"\n Setting Interpolation settings mode to remain compatible with datapath mode for DAC %u block %u (x1 Real) in %s\r\n",
				Tile_Id, Block_Id, __func__);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_INTERP_CTRL_OFFSET, XRFDC_INTERP_MODE_MASK_EXT,
					XRFDC_INTERP_DECIM_1X);
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_ITERP_DATA_OFFSET, XRFDC_DAC_INTERP_DATA_MASK,
					XRFDC_DISABLED);
		}

		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_RD_MASK,
				(FabricRate << XRFDC_FAB_RATE_RD_SHIFT));
	} else { /*Modes 1-3*/
		CurrentMode = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_MODE_MASK);
		if (CurrentMode == XRFDC_DAC_INT_MODE_FULL_BW_BYPASS) {
			XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_FABRIC_RATE_OFFSET, XRFDC_DAC_FAB_RATE_RD_MASK,
					(FabricRate << XRFDC_FAB_RATE_RD_SHIFT));
		}
	}

	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET,
			(XRFDC_DATAPATH_MODE_MASK | XRFDC_DATAPATH_IMR_MASK), DatapathReg);

	BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
	GetClkDiv = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DIV_OFFSET, XRFDC_FAB_CLK_DIV_CAL_MASK);
	if (GetClkDiv != SetClkDiv) {
		metal_log(METAL_LOG_WARNING,
			  "\n Setting mode that may not be compatible with other channels on this tile %s\r\n",
			  __func__);
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_HSCOM_CLK_DIV_OFFSET, XRFDC_FAB_CLK_DIV_CAL_MASK,
				SetClkDiv);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to get the DAC Datapath mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    ModePtr pointer used to return value.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_GetDataPathMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr)
{
	u32 Status = XRFDC_SUCCESS;
	u32 BaseAddr;
	u32 DatapathReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ModePtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckDigitalPathEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	DatapathReg = XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET,
				  (XRFDC_DATAPATH_MODE_MASK | XRFDC_DATAPATH_IMR_MASK));
	switch (DatapathReg & XRFDC_DATAPATH_MODE_MASK) {
	case XRFDC_DAC_INT_MODE_FULL_BW_BYPASS:
		*ModePtr = XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO;
		break;
	case XRFDC_DAC_INT_MODE_HALF_BW_IMR:
		if ((DatapathReg >> XRFDC_DATAPATH_IMR_SHIFT) == XRFDC_DAC_IMR_MODE_HIGHPASS) {
			*ModePtr = XRFDC_DATAPATH_MODE_FSDIVFOUR_FSDIVTWO;
		} else {
			*ModePtr = XRFDC_DATAPATH_MODE_DUC_0_FSDIVFOUR;
		}
		break;
	case XRFDC_DAC_INT_MODE_FULL_BW:
	default:
		*ModePtr = XRFDC_DATAPATH_MODE_DUC_0_FSDIVTWO;
		break;
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* This API is to set the DAC Image Reject Filter Pass mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param    Mode valid values are 0 (for low pass) 1 (for high pass).
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if tile not enabled / bad parameter passed
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_SetIMRPassMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode)
{
	u32 Status = XRFDC_SUCCESS;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		return Status;
	}

	if (Mode > XRFDC_DAC_IMR_MODE_MAX) {
		metal_log(METAL_LOG_ERROR, "\n Invalid Mode value in (%u) for DAC %u block %u in %s\r\n", Mode, Tile_Id,
			  Block_Id, __func__);
		Status = XRFDC_FAILURE;
		return Status;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_IMR_MASK, Mode << 2);

	return Status;
}

/*****************************************************************************/
/**
*
* This API is to get the DAC Image Reject Filter Pass mode.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
* @param     ModePtr pointer used to return value.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Only for DAC blocks
*
******************************************************************************/
u32 XRFdc_GetIMRPassMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr)
{
	u32 Status = XRFDC_SUCCESS;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(ModePtr != NULL);

	Status = XRFdc_CheckBlockEnabled(InstancePtr, XRFDC_DAC_TILE, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n DAC %u block %u not available in %s\r\n", Tile_Id, Block_Id, __func__);
		return Status;
	}

	BaseAddr = XRFDC_BLOCK_BASE(XRFDC_DAC_TILE, Tile_Id, Block_Id);
	*ModePtr = (XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_DAC_DATAPATH_OFFSET, XRFDC_DATAPATH_IMR_MASK)) >> 2;
	return Status;
}

/*****************************************************************************/
/**
*
* Static API to dump ADC registers.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           None
*
* @note     None.
*
******************************************************************************/
static void XRFdc_DumpADCRegs(XRFdc *InstancePtr, int Tile_Id)
{
	u32 BlockId;
	u32 Block;
	u32 IsBlockAvail;
	u32 Offset;
	u32 BaseAddr;
	u32 ReadReg = 0U;

	for (BlockId = XRFDC_BLK_ID0; BlockId < XRFDC_BLK_ID4; BlockId++) {
		Block = BlockId;
		if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == 1) {
			if (BlockId == XRFDC_BLK_ID1) {
				Block = XRFDC_BLK_ID0;
			}
			if ((BlockId == XRFDC_BLK_ID3) || (BlockId == XRFDC_BLK_ID2)) {
				Block = XRFDC_BLK_ID1;
			}
		}
		IsBlockAvail = XRFdc_IsADCBlockEnabled(InstancePtr, Tile_Id, Block);
		if (IsBlockAvail == 0U) {
			IsBlockAvail = XRFdc_IsADCDigitalPathEnabled(InstancePtr, Tile_Id, Block);
			if (IsBlockAvail == 0U) {
				continue;
			}
		}
		metal_log(METAL_LOG_DEBUG, "\n ADC%d%d:: \r\n", Tile_Id, BlockId);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(BlockId);
		for (Offset = 0x0U; Offset <= 0x284U; Offset += 0x4U) {
			if ((Offset >= 0x24U && Offset <= 0x2CU) || (Offset >= 0x48U && Offset <= 0x7CU) ||
			    (Offset >= 0xACU && Offset <= 0xC4U) || (Offset >= 0x114U && Offset <= 0x13CU) ||
			    (Offset >= 0x188U && Offset <= 0x194U) || (Offset >= 0x1B8U && Offset <= 0x1BCU) ||
			    (Offset >= 0x1D8U && Offset <= 0x1FCU) || (Offset >= 0x240U && Offset <= 0x27CU)) {
				continue;
			}
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
			metal_log(METAL_LOG_DEBUG, "\n offset = 0x%x and Value = 0x%x \t", Offset, ReadReg);
		}
	}
	(void)ReadReg;
}

/*****************************************************************************/
/**
*
* Static API to dump DAC registers.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           None
*
* @note     None.
*
******************************************************************************/
static void XRFdc_DumpDACRegs(XRFdc *InstancePtr, int Tile_Id)
{
	u32 BlockId;
	u32 IsBlockAvail;
	u32 Offset;
	u32 BaseAddr;
	u32 ReadReg = 0U;

	for (BlockId = XRFDC_BLK_ID0; BlockId < XRFDC_BLK_ID4; BlockId++) {
		IsBlockAvail = XRFdc_IsDACBlockEnabled(InstancePtr, Tile_Id, BlockId);
		if (IsBlockAvail == 0U) {
			IsBlockAvail = XRFdc_IsDACDigitalPathEnabled(InstancePtr, Tile_Id, BlockId);
			if (IsBlockAvail == 0U) {
				continue;
			}
		}
		metal_log(METAL_LOG_DEBUG, "\n DAC%d%d:: \r\n", Tile_Id, BlockId);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(BlockId);
		for (Offset = 0x0U; Offset <= 0x24CU; Offset += 0x4U) {
			if ((Offset >= 0x28U && Offset <= 0x34U) || (Offset >= 0x48U && Offset <= 0x7CU) ||
			    (Offset >= 0xA8U && Offset <= 0xBCU) || (Offset >= 0xE4U && Offset <= 0xFCU) ||
			    (Offset >= 0x16CU && Offset <= 0x17CU) || (Offset >= 0x198U && Offset <= 0x1BCU) ||
			    (Offset >= 0x1ECU && Offset <= 0x1FCU) || (Offset >= 0x204U && Offset <= 0x23CU)) {
				continue;
			}
			ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
			metal_log(METAL_LOG_DEBUG, "\n offset = 0x%x and Value = 0x%x \t", Offset, ReadReg);
		}
	}
	(void)ReadReg;
}

/*****************************************************************************/
/**
*
* Static API to dump HSCOM registers.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           None
*
* @note     None.
*
******************************************************************************/
static void XRFdc_DumpHSCOMRegs(XRFdc *InstancePtr, u32 Type, int Tile_Id)
{
	u32 Offset;
	u32 BaseAddr;
	u32 ReadReg = 0U;

	if (Type == XRFDC_ADC_TILE) {
		metal_log(METAL_LOG_DEBUG, "\n ADC%d HSCOM:: \r\n", Tile_Id);
		BaseAddr = XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;

	} else {
		metal_log(METAL_LOG_DEBUG, "\n DAC%d HSCOM:: \r\n", Tile_Id);
		BaseAddr = XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_HSCOM_ADDR;
	}
	metal_log(METAL_LOG_DEBUG, "\n Offset\tValue \r\n");
	for (Offset = 0x0U; Offset <= 0x148U; Offset += 0x4U) {
		if ((Offset >= 0x60U && Offset <= 0x88U) || (Offset == 0xBCU) || (Offset >= 0xC4U && Offset <= 0xFCU) ||
		    (Offset >= 0x110U && Offset <= 0x11CU) || (Offset >= 0x12CU && Offset <= 0x13CU)) {
			continue;
		}
		ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, Offset);
		metal_log(METAL_LOG_DEBUG, "\n 0x%x \t 0x%x \t", Offset, ReadReg);
	}
	(void)ReadReg;
}

/*****************************************************************************/
/**
*
* This Prints the offset of the register along with the content. This API is
* meant to be used for debug purposes. It prints to the console the contents
* of registers for the passed Tile_Id. If -1 is passed, it prints the contents
* of the registers for all the tiles for the respective ADC or DAC
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3, and -1.
*
* @return
*           None
*
* @note     None.
*
******************************************************************************/
void XRFdc_DumpRegs(XRFdc *InstancePtr, u32 Type, int Tile_Id)
{
	u16 NoOfTiles;
	u16 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (Tile_Id == XRFDC_SELECT_ALL_TILES) {
		NoOfTiles = XRFDC_NUM_OF_TILES4;
	} else {
		NoOfTiles = XRFDC_NUM_OF_TILES1;
	}
	for (Index = XRFDC_TILE_ID0; Index < NoOfTiles; Index++) {
		if (NoOfTiles == XRFDC_NUM_OF_TILES4) {
			Tile_Id = Index;
		}
		if (Type == XRFDC_ADC_TILE) {
			XRFdc_DumpADCRegs(InstancePtr, Tile_Id);
		} else {
			XRFdc_DumpDACRegs(InstancePtr, Tile_Id);
		}
		XRFdc_DumpHSCOMRegs(InstancePtr, Type, Tile_Id);
	}
}

/*****************************************************************************/
/**
*
* This is a stub for the status callback. The stub is here in case the upper
* layers forget to set the handler.
*
* @param    CallBackRefPtr is a pointer to the upper layer callback reference.
* @param    Type indicates ADC/DAC.
* @param    Tile_Id indicates Tile number (0-3).
* @param    Block_Id indicates Block number (0-3).
* @param    StatusEvent indicates one or more interrupt occurred.
*
* @note     None.
*
* @note     None.
*
******************************************************************************/
static void StubHandler(void *CallBackRefPtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 StatusEvent)
{
	(void)((void *)CallBackRefPtr);
	(void)Type;
	(void)Tile_Id;
	(void)Block_Id;
	(void)StatusEvent;

	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* Set The Power up/down mode of a given converter.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id Valid values are 0-3.
* @param    SettingsPtr is a pointer with the power mode settings.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_SetPwrMode(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_Pwr_Mode_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;
	u32 Index;
	u32 NoOfBlocks;
	u32 CtrlSettingsMask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SettingsPtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if (SettingsPtr->PwrMode > XRFDC_PWR_MODE_ON) {
		metal_log(METAL_LOG_ERROR, "\n Invalid power mode selection (%u) in %s %u block %u %s\r\n",
			  SettingsPtr->PwrMode, ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (SettingsPtr->DisableIPControl > XRFDC_ENABLED) {
		metal_log(METAL_LOG_ERROR, "\n Invalid IP control disable selection (%u) in %s %u block %u %s\r\n",
			  SettingsPtr->PwrMode, ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id,
			  __func__);
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	Index = Block_Id;
	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) && (Type == XRFDC_ADC_TILE)) {
		NoOfBlocks = XRFDC_NUM_OF_BLKS2;
		if (Block_Id == XRFDC_BLK_ID1) {
			Index = XRFDC_BLK_ID2;
			NoOfBlocks = XRFDC_NUM_OF_BLKS4;
		}
	} else {
		NoOfBlocks = Block_Id + 1U;
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);
	CtrlSettingsMask = !SettingsPtr->PwrMode;
	CtrlSettingsMask |= (SettingsPtr->DisableIPControl << XRFDC_TDD_CTRL_RTP_SHIFT);

	for (; Index < NoOfBlocks; Index++) {
		XRFdc_ClrSetReg(InstancePtr, BaseAddr, XRFDC_TDD_CTRL_SLICE_OFFSET(Index),
				(XRFDC_TDD_CTRL_MODE0_MASK | XRFDC_TDD_CTRL_RTP_MASK), CtrlSettingsMask);
	}

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}
/*****************************************************************************/
/**
*
* Get The Power up/down mode of a given converter.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id Valid values are 0-3.
* @param    SettingsPtr is a pointer to be filled with the power mode settings.
*
* @return
*           - XRFDC_SUCCESS if successful.
*           - XRFDC_FAILURE if error occurs.
*
* @note     Common API for ADC/DAC blocks
*
******************************************************************************/
u32 XRFdc_GetPwrMode(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_Pwr_Mode_Settings *SettingsPtr)
{
	u32 Status;
	u32 BaseAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);
	Xil_AssertNonvoid(SettingsPtr != NULL);

	if (InstancePtr->RFdc_Config.IPType < XRFDC_GEN3) {
		Status = XRFDC_FAILURE;
		metal_log(METAL_LOG_ERROR, "\n Requested functionality not available for this IP in %s\r\n", __func__);
		goto RETURN_PATH;
	}

	Status = XRFdc_CheckBlockEnabled(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XRFDC_SUCCESS) {
		metal_log(METAL_LOG_ERROR, "\n %s %u block %u not available in %s\r\n",
			  ((Type == XRFDC_ADC_TILE) ? "ADC" : "DAC"), Tile_Id, Block_Id, __func__);
		goto RETURN_PATH;
	}

	if ((XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) && (Type == XRFDC_ADC_TILE)) {
		if (Block_Id == XRFDC_BLK_ID1) {
			Block_Id = XRFDC_BLK_ID2;
		}
	}

	BaseAddr = XRFDC_CTRL_STS_BASE(Type, Tile_Id);
	SettingsPtr->PwrMode =
		!XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_TDD_CTRL_SLICE_OFFSET(Block_Id), XRFDC_TDD_CTRL_MODE0_MASK);
	SettingsPtr->DisableIPControl =
		XRFdc_RDReg(InstancePtr, BaseAddr, XRFDC_TDD_CTRL_SLICE_OFFSET(Block_Id), XRFDC_TDD_CTRL_RTP_MASK) >>
		XRFDC_TDD_CTRL_RTP_SHIFT;

	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Execute Read modify Write
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddr is address of a block.
* @param    RegAddr is register offset value.
* @param    Mask contains bit mask value.
* @param    Data contains value to be written to register.
*
* @return
*           - None
*
******************************************************************************/
void XRFdc_ClrSetReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask, u16 Data)
{
	u16 ReadReg;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, RegAddr);
	ReadReg = (ReadReg & ~Mask) | (Data & Mask);
	XRFdc_WriteReg16(InstancePtr, BaseAddr, RegAddr, ReadReg);
}

/*****************************************************************************/
/**
*
* Execute Read and clear
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddr is address of a block.
* @param    RegAddr is register offset value.
* @param    Mask contains bit mask value.
*
* @return
*           - None
*
******************************************************************************/
void XRFdc_ClrReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask)
{
	u16 ReadReg;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, RegAddr);
	ReadReg &= ~Mask;
	XRFdc_WriteReg16(InstancePtr, BaseAddr, RegAddr, ReadReg);
}

/*****************************************************************************/
/**
*
* Execute Read and mask with the value
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddr is address of a block.
* @param    RegAddr is register offset value.
* @param    Mask contains bit mask value.
*
* @return
*           - None
*
******************************************************************************/
u16 XRFdc_RDReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask)
{
	u16 ReadReg;

	ReadReg = XRFdc_ReadReg16(InstancePtr, BaseAddr, RegAddr);
	ReadReg &= Mask;

	return ReadReg;
}

/*****************************************************************************/
/**
*
* Get ADC type is High Speed or Medium Speed.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
*
* @return
*           - Return 1 if ADC type is 4GSPS, otherwise 0.
*
******************************************************************************/

u32 XRFdc_IsHighSpeedADC(XRFdc *InstancePtr, int Tile)
{
	if (InstancePtr->RFdc_Config.ADCTile_Config[Tile].NumSlices == 0) {
		return InstancePtr->ADC4GSPS;
	} else {
		return (InstancePtr->RFdc_Config.ADCTile_Config[Tile].NumSlices != XRFDC_NUM_SLICES_LSADC);
	}
}

/*****************************************************************************/
/**
*
* Get IP BaseAddress.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
*
* @return
*           - Return IP BaseAddress.
*
******************************************************************************/
u32 XRFdc_Get_IPBaseAddr(XRFdc *InstancePtr)
{
	return (u32)InstancePtr->BaseAddr;
}

/*****************************************************************************/
/**
*
* Get Tile BaseAddress
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
*
* @return
*           - Return Tile BaseAddress.
*
******************************************************************************/
u32 XRFdc_Get_TileBaseAddr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id)
{
	u32 BaseAddr;

	if (Type == XRFDC_ADC_TILE) {
		BaseAddr = InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
	} else {
		BaseAddr = InstancePtr->BaseAddr + XRFDC_DAC_TILE_DRP_ADDR(Tile_Id);
	}

	return BaseAddr;
}

/*****************************************************************************/
/**
*
* Get Block BaseAddress
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3 in DAC/ADC-2GSPS and 0-1 in ADC-4GSPS.
*
* @return
*           - Return Block BaseAddress.
*
******************************************************************************/
u32 XRFdc_Get_BlockBaseAddr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 BaseAddr;

	if (Type == XRFDC_ADC_TILE) {
		if (XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) == XRFDC_ENABLED) {
			if (Block_Id == 1U) {
				Block_Id = 2U;
			}
		}
		BaseAddr = InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		BaseAddr = InstancePtr->BaseAddr + XRFDC_DAC_TILE_DRP_ADDR(Tile_Id) + XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}

	return BaseAddr;
}

/*****************************************************************************/
/**
*
* Get Number of DAC Blocks enabled.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
*
* @return
*           - Return number of DAC blocks enabled.
*
******************************************************************************/
u32 XRFdc_GetNoOfDACBlock(XRFdc *InstancePtr, u32 Tile_Id)
{
	return InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks;
}

/*****************************************************************************/
/**
*
* Get Number of ADC Blocks enabled.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Tile_Id Valid values are 0-3.
*
* @return
*           - Return number of ADC blocks enabled.
*
******************************************************************************/
u32 XRFdc_GetNoOfADCBlocks(XRFdc *InstancePtr, u32 Tile_Id)
{
	return InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks;
}

/*****************************************************************************/
/**
*
* Get Data Width for ADC/DAC block.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param    Tile_Id Valid values are 0-3.
* @param    Block_Id is ADC/DAC block number inside the tile. Valid values
*           are 0-3.
*
* @return
*           - Return DataWidth of ADC/DAC block.
*
******************************************************************************/
u32 XRFdc_GetDataWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id)
{
	u32 DataWidth;

	if (Type == XRFDC_ADC_TILE) {
		DataWidth =
			InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].ADCBlock_Digital_Config[Block_Id].DataWidth;
	} else {
		DataWidth =
			InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].DACBlock_Digital_Config[Block_Id].DataWidth;
	}

	return DataWidth;
}

/*****************************************************************************/
/**
*
* Checks whether ADC/DAC tile is enabled or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3.
*
* @return
*           - XRFDC_SUCCESS if tile enabled.
*           - XRFDC_FAILURE if tile not enabled.
*
******************************************************************************/
u32 XRFdc_CheckTileEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id)
{
	u32 Status;
	u32 TileMask;
	u32 TileEnableReg;

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}

	TileEnableReg = XRFdc_ReadReg(InstancePtr, XRFDC_IP_BASE, XRFDC_TILES_ENABLED_OFFSET);

	TileMask = XRFDC_ENABLED << Tile_Id;
	if (Type == XRFDC_DAC_TILE) {
		TileMask <<= XRFDC_DAC_TILES_ENABLED_SHIFT;
	}

	if ((TileEnableReg & TileMask) == 0U) {
		Status = XRFDC_FAILURE;
	} else {
		Status = XRFDC_SUCCESS;
	}
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Gets ADC/DAC tile maximum sampling rate.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    MaxSampleRatePtr pointer for maximum sample rate.
*
* @return
*           - XRFDC_SUCCESS if found sampling rate.
*           - XRFDC_FAILURE if could not find sampling rate.
*
******************************************************************************/
u32 XRFdc_GetMaxSampleRate(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double *MaxSampleRatePtr)
{
	u32 Status;

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Type == XRFDC_ADC_TILE) {
		*MaxSampleRatePtr = InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].MaxSampleRate * 1000;
		if (*MaxSampleRatePtr == 0) {
			*MaxSampleRatePtr = XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) ? XRFDC_ADC_4G_SAMPLING_MAX :
											 XRFDC_ADC_2G_SAMPLING_MAX;
		}
	} else {
		*MaxSampleRatePtr = InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].MaxSampleRate * 1000;
		if (*MaxSampleRatePtr == 0) {
			*MaxSampleRatePtr = XRFDC_DAC_SAMPLING_MAX;
		}
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Gets ADC/DAC tile minimum sampling rate.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    Type is ADC or DAC. 0 for ADC and 1 for DAC.
* @param    Tile_Id Valid values are 0-3.
* @param    MinSampleRatePtr pointer for minimum sample rate.
*
* @return
*           - XRFDC_SUCCESS if found sampling rate.
*           - XRFDC_FAILURE if could not find sampling rate.
*
******************************************************************************/
u32 XRFdc_GetMinSampleRate(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double *MinSampleRatePtr)
{
	u32 Status;

	if ((Type != XRFDC_ADC_TILE) && (Type != XRFDC_DAC_TILE)) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Tile_Id > XRFDC_TILE_ID_MAX) {
		Status = XRFDC_FAILURE;
		goto RETURN_PATH;
	}
	if (Type == XRFDC_ADC_TILE) {
		*MinSampleRatePtr = XRFdc_IsHighSpeedADC(InstancePtr, Tile_Id) ? XRFDC_ADC_4G_SAMPLING_MIN :
										 XRFDC_ADC_2G_SAMPLING_MIN;
	} else {
		*MinSampleRatePtr = XRFDC_DAC_SAMPLING_MIN;
	}
	Status = XRFDC_SUCCESS;
RETURN_PATH:
	return Status;
}

/*****************************************************************************/
/**
*
* Gets whether the device is a DFE variant or not.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @return
*           - XRFDC_3ADC_2DAC_TILES if DFE variant.
*           - XRFDC_4ADC_4DAC_TILES if regular Gen 1/2/3.
*
******************************************************************************/
u8 XRFdc_GetTileLayout(XRFdc *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XRFDC_COMPONENT_IS_READY);

	if (InstancePtr->RFdc_Config.ADCTile_Config[XRFDC_TILE_ID3].NumSlices == 0) {
		return XRFDC_3ADC_2DAC_TILES;
	} else {
		return XRFDC_4ADC_4DAC_TILES;
	}
}

/*****************************************************************************/
/**
*
* This API is used to get the driver version.
*
* @param    None
*
* @return
*           Driver version number
*
* @note     None.
*
******************************************************************************/
double XRFdc_GetDriverVersion(void)
{
	return 11.0;
}
