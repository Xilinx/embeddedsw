/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xrfdc.h
* @addtogroup rfdc_v1_0
* @{
* @details
*
* The Xilinx� LogiCORE IP Zynq UltraScale+ RFSoC RF Data Converter IP core
* provides a configurable wrapper to allow the RF DAC and RF ADC blocks to be
* used in IP Integrator designs. Multiple tiles are available on each RFSoC
* and each tile can have a number of data converters (analog-to-digital (ADC)
* and digital-to-analog (DAC)). The RF ADCs can sample input frequencies up
* to 4 GHz at 4 GSPS with excellent noise spectral density. The RF DACs
* generate output carrier frequencies up to 4 GHz using the 2nd Nyquist zone
* with excellent noise spectral density at an update rate of 6.4 GSPS.
* The RF data converters also include power efficient digital down-converters
* (DDCs) and digital up-converters (DUCs) that include programmable interpolation
* and decimation, NCO and complex mixer. The DDCs and DUCs can also support
* dual-band operation.
* A maximum of 4 tiles are available on for DAC and ADC operations each. Each
* tile can have a maximum of 4 blocks/slices.
* This driver provides APIs to configure various functionalities. Similarly
* the driver provides APIs to read back configurations.
* Some of the features that the driver supports are:
* 1) Setting up and reading back fine mixer settings
* 2) Setting up and reading back coarse mixer settings
* 3) Reading back interpolation or decimation factors
* 4) Setting up and reading back QMC settings which include gain, phase etc
* 5) Setting up and reading back decoder mode settings
* 6) Setting up and reading back coarse delay settings
* All the APIs implemented in the driver provide appropriate range checks.
* An API has been provided for debug purpose which will dump all registers
* for a requested tile.
* Inline functions have also been provided to read back the parameters
* initially configured through the GUI.
*
* There are plans to add more features, e.g. Support for multi band, PLL
* configurations etc.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   sk     05/16/17 Initial release
*
* </pre>
*
******************************************************************************/


#ifndef RFDC_H_
#define RFDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <stdint.h>

#define __BAREMETAL__

#ifdef __BAREMETAL__
#include "xil_assert.h"
#include "xdebug.h"
#else
#include <metal/sys.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/atomic.h>
#include <metal/io.h>
#endif
#include "xrfdc_hw.h"

/**************************** Type Definitions *******************************/

#ifndef __BAREMETAL__
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;
typedef __s32 s32;
typedef __u64 u64;
#endif

/**
 * PLL settings.
 */
typedef struct {
	u32 Enabled;	/* PLL Enable */
	float RefClkFreq;
	float SampleRate;
	u32 RefClkDivider;
	u32 FeedbackDivider;
	u32 OutputDivider;
	u32 FractionalMode;
	u64 FractionalData;
	u32 FractWidth;
} XRFdc_PLL_Settings;

/* QMC Settings */
typedef struct {
	u32 EnablePhase;
	u32 EnableGain;
	float GainCorrectionFactor;
	float PhaseCorrectionFactor;
	s32 OffsetCorrectionFactor;
	u32 EventSource;
} XRFdc_QMC_Settings;

/**
 * Coarse delay settings.
 */
typedef struct {
	u32 CoarseDelay;
	u32 EventSource;
} XRFdc_CoarseDelay_Settings;

/**
 * Mixer settings.
 */
typedef struct {
	float Freq;
	float PhaseOffset;
	u32 EventSource;
	u32 FineMixerMode;
	u32 CoarseMixFreq;
} XRFdc_Mixer_Settings;

/**
 * ADC block Threshold settings.
 */
typedef struct {
	u32 UpdateThreshold; /* Selects which threshold to update */
	u32 ThresholdMode[2]; /* Entry 0 for Threshold0 and 1 for Threshold1 */
	u32 ThresholdAvgVal[2]; /* Entry 0 for Threshold0 and 1 for Threshold1 */
	u32 ThresholdUnderVal[2]; /* Entry 0 for Threshold0 and 1 for Threshold1 */
	u32 ThresholdOverVal[2]; /* Entry 0 is for Threshold0 and 1 for Threshold1 */
} XRFdc_Threshold_Settings;

/**
 * RFSoC Tile status.
 */
typedef struct {
	u32 IsEnabled;	/* 1, if tile is enabled, 0 otherwise */
	u32 TileState;	/* Indicates Tile Current State */
	u8 BlockStatusMask; /* Bit mask for block status, 1 indicates block enable */
	u32 PowerUpState;
	u32 PLLState;
} XRFdc_TileStatus;

/**
 * RFSoC Data converter IP status.
 */
typedef struct {
	XRFdc_TileStatus DACTileStatus[4];
	XRFdc_TileStatus ADCTileStatus[4];
	u32 State;
} XRFdc_IPStatus;

/**
 * status of DAC or ADC blocks in the RFSoC Data converter..
 */
typedef struct {
	float SamplingFreq;
	u32 AnalogDataPathStatus;
	u32 DigitalDataPathStatus;
} XRFdc_BlockStatus;

typedef struct {
	u32 BlockAvailable;
	u32 InvSyncEnable;
	u32 MixMode;
	u32 DecoderMode;
} XRFdc_DACBlock_AnalogDataPath_Config;

typedef struct {
	u32 DataType;
	u32 DataWidth;
	u32 InterploationMode;
	u32 FifoEnable;
	u32 AdderEnable;
} XRFdc_DACBlock_DigitalDataPath_Config;

typedef struct {
	u32 BlockAvailable;
	u32 MixMode;
} XRFdc_ADCBlock_AnalogDataPath_Config;

typedef struct {
	u32 DataType;
	u32 DataWidth;
	u32 DecimationMode;
	u32 FifoEnable;
} XRFdc_ADCBlock_DigitalDataPath_Config;

typedef struct {
	u32 Enable;
	u32 PLLEnable;
	float SamplingRate;
	float RefClkFreq;
	float FabClkFreq;
	XRFdc_DACBlock_AnalogDataPath_Config DACBlock_Analog_Config[4];
	XRFdc_DACBlock_DigitalDataPath_Config DACBlock_Digital_Config[4];
} XRFdc_DACTile_Config;

typedef struct {
	u32 Enable;	/* Tile Enable status */
	u32 PLLEnable;	/* PLL enable Status */
	float SamplingRate;
	float RefClkFreq;
	float FabClkFreq;
	XRFdc_ADCBlock_AnalogDataPath_Config ADCBlock_Analog_Config[4];
	XRFdc_ADCBlock_DigitalDataPath_Config ADCBlock_Digital_Config[4];
} XRFdc_ADCTile_Config;

typedef struct {
	u32 DeviceId;
	u32 BaseAddr;
	u32 ADCType;	/* ADC Type 4GSPS or 2GSPS*/
	u32 MasterADCTile;	/* ADC master Tile */
	u32 MasterDACTile;	/* DAC Master Tile */
	u32 ADCSysRefSource;
	u32 DACSysRefSource;
	XRFdc_DACTile_Config DACTile_Config[4];
	XRFdc_ADCTile_Config ADCTile_Config[4];
} XRFdc_Config;

typedef struct {
	u32 Enabled;	/* DAC Analog Data Path Enable */
	u32 MixedMode;
	float TerminationVoltage;
	float OutputCurrent;
	u32 InverseSincFilterEnable;
	u32 DecoderMode;
	void * FuncHandler;
	XRFdc_QMC_Settings QMC_Settings;
	XRFdc_CoarseDelay_Settings CoarseDelay_Settings;
} XRFdc_DACBlock_AnalogDataPath;

typedef struct {
	u32 DataType;
	u32 DataWidth;
	int ConnectedIData;
	int ConnectedQData;
	u32 InterpolationFactor;
	XRFdc_Mixer_Settings Mixer_Settings;
} XRFdc_DACBlock_DigitalDataPath;

typedef struct {
	u32 Enabled;	/* ADC Analog Data Path Enable */
	XRFdc_QMC_Settings QMC_Settings;
	XRFdc_CoarseDelay_Settings CoarseDelay_Settings;
	XRFdc_Threshold_Settings Threshold_Settings;
} XRFdc_ADCBlock_AnalogDataPath;

typedef struct {
	u32 DataType;
	u32 DataWidth;
	u32 DecimationFactor;
	int ConnectedIData;
	int ConnectedQData;
	XRFdc_Mixer_Settings Mixer_Settings;
} XRFdc_ADCBlock_DigitalDataPath;

typedef struct {
	u32 TileBaseAddr;	/* Tile  BaseAddress*/
	u32 NumOfDACBlocks;	/* Number of DAC block enabled */
	XRFdc_PLL_Settings PLL_Settings;
	XRFdc_DACBlock_AnalogDataPath DACBlock_Analog_Datapath[4];
	XRFdc_DACBlock_DigitalDataPath DACBlock_Digital_Datapath[4];
} XRFdc_DAC_Tile;

typedef struct {
	u32 TileBaseAddr;
	u32 NumOfADCBlocks;	/* Number of ADC block enabled */
	XRFdc_PLL_Settings PLL_Settings;
	XRFdc_ADCBlock_AnalogDataPath ADCBlock_Analog_Datapath[4];
	XRFdc_ADCBlock_DigitalDataPath ADCBlock_Digital_Datapath[4];
} XRFdc_ADC_Tile;

typedef struct {
	XRFdc_Config RFdc_Config;	/* Config Structure */
	u32 IsReady;
	u32 ADC4GSPS;
#ifdef __BAREMETAL__
	u32 BaseAddr;	/* BaseAddress */
#else
	metal_phys_addr_t BaseAddr;	/* BaseAddress */
	struct metal_io_region *io;	/* Libmetal IO structure */
	struct metal_device *device;	/* Libmetal device structure */
#endif
	XRFdc_DAC_Tile DAC_Tile[4];
	XRFdc_ADC_Tile ADC_Tile[4];
} XRFdc;

/***************** Macros (Inline Functions) Definitions *********************/

#define XRFDC_SUCCESS                     0L
#define XRFDC_FAILURE                     1L
#define XRFDC_COMPONENT_IS_READY     	0x11111111U

#define XRFDC_ADC_TILE				0U
#define XRFDC_DAC_TILE				1U
#define XRFDC_EVNT_SRC_SLICE		0x00000001U
#define XRFDC_EVNT_SRC_TILE			0x00000002U
#define XRFDC_EVNT_SRC_IMMEDIATE	0x00000008U
#define XRFDC_EVENT_MIXER			0x00000001U
#define XRFDC_EVENT_CRSE_DLY		0x00000002U
#define XRFDC_EVENT_QMC				0x00000004U
#define XRFDC_SELECT_ALL_TILES		-1
#define XRFDC_ADC_4GSPS				1U

#define XRFDC_NCO_FREQ_MULTIPLIER		(0x1LL << 48U) /* 2^48 */
#define XRFDC_NCO_PHASE_MULTIPLIER		(0x1U << 17U) /* 2^17 */
#define XRFDC_QMC_PHASE_MULT			(0x1U << 11U) /* 2^11 */
#define XRFDC_QMC_GAIN_MULT				(0x1U << 14U) /* 2^14 */

#define XRFDC_DATA_TYPE_IQ			0x00000001U
#define XRFDC_TRSHD_STICKY_OVER		0x00000001U
#define XRFDC_TRSHD_STICKY_UNDER	0x00000002U
#define XRFDC_TRSHD_HYSTERISIS		0x00000003U

/* Mixer modes */
#define XRFDC_FINE_MIXER_MOD_OFF				0x0U
#define XRFDC_FINE_MIXER_MOD_COMPLX_TO_COMPLX	0x1U
#define XRFDC_FINE_MIXER_MOD_COMPLX_TO_REAL		0x2U
#define XRFDC_FINE_MIXER_MOD_REAL_TO_COMPLX		0x3U
#define XRFDC_MIXER_MAX_SUPP_MIXER_MODE			0x3U

#define XRFDC_I_IQ_COS_MINSIN	0x00000C00U
#define XRFDC_Q_IQ_SIN_COS		0x00001000U
#define XRFDC_EN_I_IQ			0x00000001U
#define XRFDC_EN_Q_IQ			0x00000004U

#define XRFDC_COARSE_MIX_OFF						0x0U
#define XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO			0x2U
#define XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR		0x4U
#define XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR	0x8U

#define XRFDC_CRSE_MIX_OFF					0x924U
#define XRFDC_CRSE_MIX_BYPASS				0x0U
#define XRFDC_CRSE_MIX_I_Q_FSBYTWO			0x410U
#define XRFDC_CRSE_MIX_I_FSBYFOUR			0x298U
#define XRFDC_CRSE_MIX_Q_FSBYFOUR			0x688U
#define XRFDC_CRSE_MIX_I_MINFSBYFOUR		0x688U
#define XRFDC_CRSE_MIX_Q_MINFSBYFOUR		0x298U

#define XRFDC_MIXER_PHASE_OFFSET_UP_LIMIT	180
#define XRFDC_MIXER_PHASE_OFFSET_LOW_LIMIT	(-180)
#define XRFDC_UPDATE_THRESHOLD_0			0x1
#define XRFDC_UPDATE_THRESHOLD_1			0x2
#define XRFDC_UPDATE_THRESHOLD_BOTH			0x4
#define XRFDC_THRESHOLD_CLRMD_MANUAL_CLR	0x1
#define XRFDC_THRESHOLD_CLRMD_AUTO_CLR		0x2
#define XRFDC_DECODER_MAX_SNR_MODE			0x1
#define XRFDC_DECODER_MAX_LINEARITY_MODE	0x2
#define XRFDC_OUTPUT_CURRENT_32MA			32
#define XRFDC_OUTPUT_CURRENT_20MA			20


/*****************************************************************************/
/**
*
* Checks whether DAC block is available or not.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return 1 if DAC block is available, otherwise 0.
*
******************************************************************************/
static inline u32 XRFdc_IsDACBlockEnabled(XRFdc* InstancePtr, int Tile_Id,
												u32 Block_Id)
{
	if (InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
			DACBlock_Analog_Config[Block_Id].BlockAvailable == 0U)
		return 0;
	else
		return 1;
}

/*****************************************************************************/
/**
*
* Checks whether ADC block is available or not.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return 1 if ADC block is available, otherwise 0.
*
******************************************************************************/
static inline u32 XRFdc_IsADCBlockEnabled(XRFdc* InstancePtr, int Tile_Id,
												u32 Block_Id)
{
	if (InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
			ADCBlock_Analog_Config[Block_Id].BlockAvailable == 0U)
		return 0;
	else
		return 1;
}

/*****************************************************************************/
/**
*
* Get IP BaseAddress.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
*
* @return
*		- Return IP BaseAddress.
*
******************************************************************************/
static inline u32 XRFdc_Get_IPBaseAddr(XRFdc* InstancePtr)
{
	return InstancePtr->BaseAddr;
}

/*****************************************************************************/
/**
*
* Get Tile BaseAddress
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
*
* @return
*		- Return Tile BaseAddress.
*
******************************************************************************/
static inline u32 XRFdc_Get_TileBaseAddr(XRFdc* InstancePtr, u32 Type,
								int Tile_Id)
{
	if(Type == XRFDC_ADC_TILE)
		return InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
	else
		return InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id);
}

/*****************************************************************************/
/**
*
* Get Block BaseAddress
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return Block BaseAddress.
*
******************************************************************************/
static inline u32 XRFdc_Get_BlockBaseAddr(XRFdc* InstancePtr, u32 Type,
								int Tile_Id, u32 Block_Id)
{
	if(Type == XRFDC_ADC_TILE) {
		return InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	} else {
		return InstancePtr->BaseAddr + XRFDC_ADC_TILE_DRP_ADDR(Tile_Id) +
								XRFDC_BLOCK_ADDR_OFFSET(Block_Id);
	}
}

/*****************************************************************************/
/**
*
* Get Number of DAC Blocks enabled.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
*
* @return
*		- Return number of DAC blocks enabled.
*
******************************************************************************/
static inline u32 XRFdc_GetNoOfDACBlock(XRFdc* InstancePtr, int Tile_Id)
{
	return InstancePtr->DAC_Tile[Tile_Id].NumOfDACBlocks;
}

/*****************************************************************************/
/**
*
* Get Number of ADC Blocks enabled.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
*
* @return
*		- Return number of ADC blocks enabled.
*
******************************************************************************/
static inline u32 XRFdc_GetNoOfADCBlocks(XRFdc* InstancePtr, int Tile_Id)
{
	return InstancePtr->ADC_Tile[Tile_Id].NumOfADCBlocks;
}

/*****************************************************************************/
/**
*
* Get ADC type is 4GSPS or not.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
*
* @return
*		- Return 1 if ADC type is 4GSPS, otherwise 0.
*
******************************************************************************/
static inline u32 XRFdc_IsADC4GSPS(XRFdc* InstancePtr)
{
	if (InstancePtr->RFdc_Config.ADCType == 4)
		return 1;
	else
		return 0;
}

/*****************************************************************************/
/**
*
* Get Data Type for ADC/DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return DataType of ADC/DAC block.
*
******************************************************************************/
static inline u32 XRFdc_GetDataType(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id)
{
	if (Type == XRFDC_ADC_TILE) {
		return InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
				ADCBlock_Digital_Config[Block_Id].DataType;
	} else {
		return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
				DACBlock_Digital_Config[Block_Id].DataType;
	}
}

/*****************************************************************************/
/**
*
* Get Data Width for ADC/DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return DataWidth of ADC/DAC block.
*
******************************************************************************/
static inline u32 XRFdc_GetDataWidth(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id)
{
	if (Type == XRFDC_ADC_TILE) {
		return InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
				ADCBlock_Digital_Config[Block_Id].DataWidth;
	} else {
		return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
				DACBlock_Digital_Config[Block_Id].DataWidth;
	}
}

/*****************************************************************************/
/**
*
* Get Inversesync filter for DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return Inversesync filter for DAC block
*
******************************************************************************/
static inline u32 XRFdc_GetInverseSincFilter(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id)
{
	return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
				DACBlock_Analog_Config[Block_Id].InvSyncEnable;
}

/*****************************************************************************/
/**
*
* Get Term Voltage for DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return Term Voltage for DAC block
*
******************************************************************************/
static inline float XRFdc_GetTermVoltage(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id)
{
	return InstancePtr->DAC_Tile[Tile_Id].DACBlock_Analog_Datapath[Block_Id].
				TerminationVoltage;
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
*
* @return
*		- Return Output Current for DAC block
*
******************************************************************************/
static inline float XRFdc_GetOutputCurr(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id)
{
	return InstancePtr->DAC_Tile[Tile_Id].
				DACBlock_Analog_Datapath[Block_Id].OutputCurrent;
}

/*****************************************************************************/
/**
*
* Get Mixed mode for DAC block.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return mixed mode for DAC block
*
******************************************************************************/
static inline u32 XRFdc_GetMixedMode(XRFdc* InstancePtr, int Tile_Id,
								u32 Block_Id)
{
	return InstancePtr->DAC_Tile[Tile_Id].
				DACBlock_Analog_Datapath[Block_Id].MixedMode;
}

/*****************************************************************************/
/**
*
* Get Master Tile for ADC/DAC tiles.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
*
* @return
*		- Return Master Tile for ADC/DAC tiles
*
******************************************************************************/
static inline u32 XRFdc_GetMasterTile(XRFdc* InstancePtr, u32 Type)
{
	if (Type == XRFDC_ADC_TILE)
		return InstancePtr->RFdc_Config.MasterADCTile;
	else
		return InstancePtr->RFdc_Config.MasterDACTile;
}

/*****************************************************************************/
/**
*
* Get Sysref source for ADC/DAC tile.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
*
* @return
*		- Return Sysref source for ADC/DAC tile
*
******************************************************************************/
static inline u32 XRFdc_GetSysRefSource(XRFdc* InstancePtr, u32 Type)
{
	if (Type == XRFDC_ADC_TILE)
		return InstancePtr->RFdc_Config.ADCSysRefSource;
	else
		return InstancePtr->RFdc_Config.DACSysRefSource;
}

/*****************************************************************************/
/**
*
* Get Fabric Clock frequency.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
*
* @return
*		- Return Fabric Clock frequency for ADC/DAC tile
*
******************************************************************************/
static inline float XRFdc_GetFabClkFreq(XRFdc* InstancePtr, u32 Type,
								int Tile_Id)
{
	if (Type == XRFDC_ADC_TILE)
		return InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].FabClkFreq;
	else
		return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].FabClkFreq;
}

/*****************************************************************************/
/**
*
* Get whether FIFO is enabled or not.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	Type is ADC or DAC. 0 for ADC and 1 for DAC
* @param	Tile_Id Valid values are 0-3.
* @param	Block_Id is ADC/DAC block number inside the tile. Valid values
*			are 0-3.
*
* @return
*		- Return 1 if FIFO is enabled, otherwise 0.
*
******************************************************************************/
static inline u32 XRFdc_IsFifoEnabled(XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id)
{
	if (Type == XRFDC_ADC_TILE) {
		return InstancePtr->RFdc_Config.ADCTile_Config[Tile_Id].
				ADCBlock_Digital_Config[Block_Id].FifoEnable;
	} else {
		return InstancePtr->RFdc_Config.DACTile_Config[Tile_Id].
				DACBlock_Digital_Config[Block_Id].FifoEnable;
	}
}

/************************** Function Prototypes ******************************/

XRFdc_Config *XRFdc_LookupConfig(u16 DeviceId);
int XRFdc_CfgInitialize(XRFdc* InstancePtr, XRFdc_Config *Config);
int XRFdc_StartUp(XRFdc* InstancePtr, u32 Type, int Tile_Id);
int XRFdc_Shutdown(XRFdc* InstancePtr, u32 Type, int Tile_Id);
int XRFdc_Reset(XRFdc* InstancePtr, u32 Type, int Tile_Id);
int XRFdc_GetIPStatus(XRFdc* InstancePtr, XRFdc_IPStatus* IPStatus);
int XRFdc_GetBlockStatus(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id, XRFdc_BlockStatus* BlockStatus);
int XRFdc_SetMixerSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id, XRFdc_Mixer_Settings * Mixer_Settings);
int XRFdc_GetMixerSettings(XRFdc* InstancePtr, u32 Type,
				int Tile_Id, u32 Block_Id,
				XRFdc_Mixer_Settings * Mixer_Settings);
int XRFdc_SetQMCSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id, XRFdc_QMC_Settings * QMC_Settings);
int XRFdc_GetQMCSettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id, XRFdc_QMC_Settings * QMC_Settings);
int XRFdc_GetCoarseDelaySettings(XRFdc* InstancePtr, u32 Type,
				int Tile_Id, u32 Block_Id,
				XRFdc_CoarseDelay_Settings * CoarseDelay_Settings);
int XRFdc_SetCoarseDelaySettings(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id,
				XRFdc_CoarseDelay_Settings * CoarseDelay_Settings);
int XRFdc_GetInterpolationFactor(XRFdc* InstancePtr, int Tile_Id,
				u32 Block_Id, u32 * InterpolationFactor);
int XRFdc_GetDecimationFactor(XRFdc* InstancePtr, int Tile_Id,
				u32 Block_Id, u32 * DecimationFactor);
int XRFdc_GetFabWrVldWords(XRFdc* InstancePtr, u32 Type,
				int Tile_Id, u32 Block_Id, u32 * FabricDataRate);
int XRFdc_GetFabRdVldWords(XRFdc* InstancePtr, u32 Type,
				int Tile_Id, u32 Block_Id, u32 * FabricDataRate);
int XRFdc_SetFabRdVldWords(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 FabricDataRate);
int XRFdc_SetFabWrVldWords(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 FabricDataRate);
int XRFdc_GetThresholdSettings(XRFdc* InstancePtr, int Tile_Id,
				u32 Block_Id, XRFdc_Threshold_Settings * Threshold_Settings);
int XRFdc_SetThresholdSettings(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
				XRFdc_Threshold_Settings * Threshold_Settings);
int XRFdc_SetDecoderMode(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
				u32 DecoderMode);
int XRFdc_UpdateEvent(XRFdc* InstancePtr, u32 Type, int Tile_Id, u32 Block_Id,
				u32 Event);
int XRFdc_GetDecoderMode(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
				u32 *DecoderMode);
int XRFdc_ResetNCOPhase(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 Block_Id);
void XRFdc_DumpRegs(XRFdc* InstancePtr, u32 Type, int Tile_Id);
void XRFdc_SetSignalFlow(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 AnalogDataPath, u32 ConnectIData, u32 ConnectQData);
void XRFdc_GetSignalFlow(XRFdc* InstancePtr, u32 Type, int Tile_Id,
				u32 AnalogDataPath, u32 * ConnectedIData,
				u32 * ConnectedQData);
void XRFdc_IntrHandler(void * XRFdcPtr);
u32 XRFdc_GetIntrStatus (XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id);
void XRFdc_IntrDisable (XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 IntrMask);
void XRFdc_IntrEnable (XRFdc* InstancePtr, u32 Type, int Tile_Id,
								u32 Block_Id, u32 IntrMask);
int XRFdc_StickyClear(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id);
int XRFdc_SetOutputCurrent(XRFdc* InstancePtr, int Tile_Id, u32 Block_Id,
								u32 OutputCurrent);

#ifdef __cplusplus
}
#endif

#endif /* RFDC_H_ */
/** @} */
