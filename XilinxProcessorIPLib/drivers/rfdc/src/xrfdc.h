/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef RFDC_H_
#define RFDC_H_

/*****************************************************************************/
/**
*
* @file xrfdc.h
* @addtogroup Overview
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
* (DDCs) and digital up-converters (DUCs) that include programmable
* interpolation and decimation, NCO and complex mixer. The DDCs and DUCs can
* also support dual-band operation.
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
* An API has been provided for debug purpose that dumps all registers
* for a requested tile.
* Inline functions have also been provided to read back the parameters
* initially configured through the GUI.
*
* There are plans to add more features, for example, support for multi band, PLL
* configurations etc.
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
*                       Corrected Interrupt Macro names and values.
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
*              09/18/17 Add API to clear the interrupts.
*       sk     09/21/17 Add __BAREMETAL__ compiler flag option
*                       for Baremetal.
*       sk     09/21/17 Add support for Over voltage and Over
*                       Range interrupts.
*       sk     09/22/17 Add s64 typedef for Linux.
*       sk     09/24/17 Fixed Get_Tile/BlockBaseAddr always
*                       giving ADC related address.
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
*                       In BYPASS mode, input datatype can be Real or IQ
*                       hence checked both while reading the mixer mode.
*              10/17/17 Fixed Set Threshold API Issue.
* 2.2   sk     10/18/17 Add support for FIFO and DATA overflow interrupt
* 2.3   sk     11/06/17 Fixed PhaseOffset truncation issue.
*                       Provide user configurability for FineMixerScale.
*              11/08/17 Return error for DAC R2C mode and ADC C2R mode.
*              11/10/17 Corrected FIFO and DATA Interrupt masks.
*              11/20/17 Fixed StartUp, Shutdown and Reset API for Tile_Id -1.
*              11/20/17 Remove unwanted ADC block checks in 4GSPS mode.
* 3.0   sk     12/11/17 Added DDC and DUC support.
*              12/13/17 Add CoarseMixMode field in Mixer_Settings structure.
*              12/15/17 Add support to switch calibration modes.
*              12/15/17 Add support for mixer frequencies > Fs/2 and < -Fs/2.
*       sg     13/01/18 Added PLL and external clock switch support
*                       Added API to get PLL lock status.
*                       Added API to get clock source.
*       sk     01/18/18 Add API to get driver version.
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
*       jm     03/12/18 Fixed DAC latency calculation in MTS.
*       jm     03/12/18 Added support for reloading DTC scans.
*       jm     03/12/18 Add option to configure sysref capture after MTS.
*       sk     03/22/18 Updated PLL settings based on latest IP values.
* 4.0   sk     04/09/18 Added API to enable/disable the sysref.
*       sk     04/09/18 Updated max VCO to 13108MHz to support max DAC
*                       sample rate of 6.554MHz.
*       rk     04/17/18 Adjust calculated latency by sysref period, where doing
*                       so results in closer alignment to the target latency.
*       sk     04/17/18 Corrected Set/Get MixerSettings API description for
*                       FineMixerScale parameter.
*       sk     04/19/18 Enable VCO Auto selection while configuring the clock.
*       sk     04/24/18 Add API to get PLL Configurations.
*       sk     04/24/18 Add API to get the Link Coupling mode.
*       sk     04/28/18 Implement timeouts for PLL Lock, Startup and shutdown.
*       sk     05/30/18 Removed CalibrationMode check for DAC.
*       sk     06/05/18 Updated minimum Ref clock value to 102.40625MHz.
* 5.0   sk     06/25/18 Update DAC min sampling rate to 500MHz and also update
*                       VCO Range, PLL_DIVIDER and PLL_FPDIV ranges.
*                       Update PLL structure with calculated sampling rate.
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
*       mus    08/17/18 Removed structure paddings from XRFdc_Config structure.
*                       It has been done to have 1:1 mapping between
*                       XRFdc_Config structure and device tree property
*                       "param-list", over linux platform.
*       sk     09/24/18 Update powerup-state value based on PLL mode in
*                       XRFdc_DynamicPLLConfig() API.
*       sk     10/10/18 Check for DigitalPath enable in XRFdc_GetNyquistZone()
*                       and XRFdc_GetCalibrationMode() APIs for Multiband.
*       sk     10/13/18 Add support to read the REFCLKDIV param from design.
*                       Update XRFdc_SetPLLConfig() API to support range of
*                       REF_CLK_DIV values(1 to 4).
*                       Add XRFDC_MIXER_MODE_R2R option to support BYPASS mode
*                       for Real input.
* 5.1   cog    01/29/19 Replace structure reference ADC checks with
*                       function.
*       cog    01/29/19 Added XRFdc_SetDither() and XRFdc_GetDither() APIs.
*       cog    01/29/19 Rename DataType for mixer input to MixerInputDataType
*                       for readability.
*       cog    01/29/19 Refactoring of interpolation and decimation APIs and
*                       changed fabric rate for decimation X8 for non-high speed ADCs.
*       cog    01/29/19 New inline functions to determine max & min sampling rates.
* 6.0   cog    02/17/19 Added Inverse-Sinc Second Nyquist Zone Support
*       cog    02/17/19 Added new clock Distribution functionality.
*       cog    02/17/19 Refactored to improve delay balancing in clock
*                       distribution.
*       cog    02/17/19 Added delay calculation & metal log messages.
*       cog    02/17/19 Added Intratile clock settings.
*       cog    02/17/19 XRFdc_GetPLLConfig() now uses register values to get the
*                       PLL configuration for new IPs and is no longer static.
*       cog    02/17/19 Refactoring of interpolation and decimation APIs and
*                       changed fabric rate for decimation X8 for non-high speed ADCs.
*       cog    02/17/19 Added XRFdc_SetIMRPassMode() and XRFdc_SetIMRPassMode() APIs
*       cog    02/17/19 Added XRFdc_SetDACMode() and XRFdc_GetDACMode() APIs
*       cog    02/17/19	Added XRFdc_SetSignalDetector() and XRFdc_GetSignalDetector() APIs
*       cog    02/17/19 Added XRFdc_DisableCoefficientsOverride(), XRFdc_SetCalCoefficients
*                       and XRFdc_GetCalCoefficients APIs.
*       cog    02/19/19 New definitions for clock detection.
* 6.0   cog    02/20/19	Added handling for new ADC common mode over/under
*                       voltage interrupts.
*       cog    02/20/19	XRFdc_GetIntrStatus now populates a pointer with the
*                       status and returns an error code.
*       cog	   02/20/19	XRFdc_IntrClr, XRFdc_IntrDisable and XRFdc_IntrEnable
*                       now return error codes.
*       cog    02/21/19 Added XRFdc_SetCalFreeze() and XRFdc_GetCalFreeze() APIs
*       cog    04/15/19 Rename XRFdc_SetDACMode() and XRFdc_GetDACMode() APIs to
*                       XRFdc_SetDataPathMode() and XRFdc_GetDataPathMode() respectively.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    05/13/19 Added new bock MACROs.
*       cog    05/13/19 XRFdc_CheckTileEnabled(), XRFdc_IsDACBlockEnabled(),
*                       XRFdc_IsADCBlockEnabled(), XRFdc_IsDACDigitalPathEnabled() &
*                       XRFdc_IsADCDigitalPathEnabled() APIs now derive answer from
*                       DRP rather than context structure.
*       cog    06/12/19 Fixed issue where positive NCO frequencies were not being
*                       set correctly.
*       cog    07/14/19 Added new off mode for mixers (both mixers off).
*       cog    07/16/19 The powerup state is not necessary to be checked for the
*                       tile/block/digital path enabled functions and had potential
*                       to cause lockout.
*       cog    07/16/19 Added XRFdc_SetDACOpCurr() API.
*       cog    07/18/19 Added XRFdc_S/GetDigitalStepAttenuator() APIs.
*       cog    07/25/19 Added new XRFdc_RegisterMetal() API to register RFDC with Libmetal.
*       cog    07/25/19 Moved XRFDC_PLL_LOCK_DLY_CNT macro from source file.
*       cog    07/26/19 Added new XRFdc_S/GetLegacyCompatibilityMode() APIs.
*       cog    07/29/19 Added XRFdc_GetEnabledInterrupts() API.
*       cog    08/02/19 Formatting changes and added a MACRO for the IP generation.
*       cog    09/01/19 Changed the MACRO for turning off the mixer.
*       cog    09/01/19 XRFdc_CheckTileEnabled(), XRFdc_IsDACBlockEnabled(),
*                       XRFdc_IsADCBlockEnabled(), XRFdc_IsDACDigitalPathEnabled() &
*                       XRFdc_IsADCDigitalPathEnabled() APIs now get answer from
*                       context structure.
*       cog    09/01/19 Rename new XRFdc_S/GetLegacyCompatibilityMode() APIs to
*                       XRFdc_S/GetDACCompMode().
*       cog    09/01/19 Rename XRFdc_S/GetDigitalStepAttenuator() APIs to XRFdc_S/GetDSA().
*                       Also, add new XRFdc_DSA_Settings structure.
*       cog    09/12/19 Swapped MIXER_TYPE_OFF and MIXER_TYPE_DISABLED macros.
*       cog    09/18/19 Minimum output divider is now 1 for Gen 3 devices.
*       cog    09/18/19 Changed clock distribution macros, also removed prototype for a function
*                       that is now static.
*       cog    10/02/19 Added macros for the clock divider.
*       cog    10/02/19 Added macro for fabric rate of 16.
*       cog    10/02/19 Added macros for new VCO ranges.
* 7.1   cog    11/14/19 Increased ADC fabric read rate to 12 words per cycle for Gen 3 devices.
*       cog    11/15/19 Added macros for calibration mode support for Gen 3 devices.
*       cog    11/28/19 Datapath mode macros have been changed to reflect the new functionality.
*       cog    01/08/20 Added programmable hysteresis counters for ADC signal detector.
*       cog    01/23/20 Calibration modes for Gen 3 were inverted.
* 8.0   cog    02/10/20 Updated addtogroup and added s16 typedef.
*       cog    02/10/20 Added Silicon revision to driver structures to allow discrimation
*                       between engineering sample & production silicon.
*       cog    02/17/20 Driver now gets tile/path enables from the bitfile.
*       cog    02/20/20 Added macros for Clock Gater handling.
*       cog    03/05/20 IMR datapath modes require the frequency word to be doubled.
*       cog    03/20/20 Updated PowerState masks for Gen 3 Devices.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Expand range of DSA for production Si.
*       cog    06/24/20 Expand range of VOP for production Si.
*       cog    06/24/20 MB config is now read from bitstream.
*       cog    06/24/20 Added observation FIFO and decimation functionality.
*       cog    06/24/20 Added channel powerdon functionality.
*       cog    06/24/20 Refactor to functionaize the FIFO width setting.
*       cog    08/04/20 Added multiband macros.
*       cog    09/08/20 The Four LSBs of the BLDR Bias Current should be the same as the
*                       four LSBs of the CS Gain.
*       cog    09/28/20 Change XRFdc_IsHighSpeedADC to accommodate 43dr parts.
*       cog    10/05/20 Change shutdown end state for Gen 3 Quad ADCs to reduce power
*                       consumption.
*       cog    10/14/20 Get I and Q data now supports warm bitstream swap.
* 9.0   cog    11/25/20 Upversion.
* 10.0  cog    11/26/20 Refactor and split files.
*       cog    11/27/20 Added functionality for 6xdr devices.
*       cog    12/02/20 Supplied FS was being saved rather than actual FS when
*                       setting PLL.
*       cog    12/04/20 Reduce scope of non user interface macros.
*       cog    01/05/21 Signal detector on/off counters needed to be flipped.
*       cog    01/05/21 Second signal detector removed.
*       cog    01/06/21 Added DAC data scaler APIs.
*       cog    01/11/21 Tuning for autocalibration.
*       cog    02/10/21 Added custom startup API.
*       cog    03/08/21 MTS now scans reference tile first. This has required a
*                       change to the prototype of XRFdc_MultiConverter_Init.
*       cog    03/12/21 Tweaks for improved calibration performance.
*       cog    05/05/21 Fixed issue where driver was attempting to start ADC 3
*                       for DFE variants.
*       cog    05/05/21 Rename the MAX/MIN macros to avoid potential conflicts.
* 11.0  cog    05/31/21 Upversion.
*       cog    07/12/21 Simplified clock distribution user interface.
*       cog    08/18/21 Disallow VOP for DC coupled DACs.
* 11.1  cog    11/16/21 Upversion.
*       cog    11/26/21 Header file clean up, this fixes a C++ compilation issue.
*       cog    11/26/21 Pack all structs for RAFT compatibility.
*       cog    12/06/21 Rearrange XRFdc_Distribution_Settings.
*       cog    01/18/22 Added safety checks.
* 12.0  cog    10/26/22 Added API XRFdc_GetCoupling(), this gets the ADC or
*                       DAC coupling.
*       cog    01/07/23 Added VOP support for DC coupled DACs and removed VOP
*                       support for ES1 Parts.
* 12.1  cog    07/04/23 Add support for SDT.
*       cog    07/14/23 Fix issues with SDT flow.
*       cog    07/27/23 Add NCO frequency to config structures.
* 12.3  cog    07/09/24 Corrected GEN3 maximum ADC VCO frequency.
* 13.1  dc     02/18/26 Correct spelling errors
* 13.2  vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

#include <metal/device.h>

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <stdint.h>

#ifdef __BAREMETAL__
#include "xil_assert.h"
#include "xdebug.h"
#include "sleep.h"
#endif
#include <metal/sys.h>
#include <metal/irq.h>
#include <metal/sleep.h>
#include "metal/alloc.h"
#include "xrfdc_hw.h"

/**************************** Type Definitions *******************************/
/**
* @brief Checks whether ADC tile 0 is a high-speed (4 GSPS) ADC.
*
* @param[in]   InstPtr Pointer to the XRfdc instance.
*
* @return      Result of XRFdc_IsHighSpeedADC() for tile 0.
*
* @note        C-style signature:
*              u32 XRFdc_IsADC4GSPS(XRFdc *InstPtr)
*/
#define XRFdc_IsADC4GSPS(InstPtr) XRFdc_IsHighSpeedADC(InstPtr, 0)

/**
* @brief Returns a read-modify-write value.
*
* @param[in]   read Value read from the register.
* @param[in]   mask Mask of the bits to modify.
* @param[in]   data New value of the masked bits.
*
* @return      The read value with the masked bits replaced by data.
*
* @note        C-style signature:
*              u32 XRFDC_MTS_RMW(u32 read, u32 mask, u32 data)
*/
#define XRFDC_MTS_RMW(read, mask, data) (((read) & ~(mask)) | ((data) & (mask)))
/**
* @brief Extracts a field from a register value.
*
* @param[in]   data Register value.
* @param[in]   mask Mask of the field.
* @param[in]   shift Bit shift of the field.
*
* @return      Value of the field.
*
* @note        C-style signature:
*              u32 XRFDC_MTS_FIELD(u32 data, u32 mask, u32 shift)
*/
#define XRFDC_MTS_FIELD(data, mask, shift) (((data) & (mask)) >> (shift))

#ifndef __BAREMETAL__
typedef __u32 u32; /**< 32-bit unsigned integer type. */
typedef __u16 u16; /**< 16-bit unsigned integer type. */
typedef __u8 u8; /**< 8-bit unsigned integer type. */
typedef __s32 s32; /**< 32-bit signed integer type. */
typedef __s16 s16; /**< 16-bit signed integer type. */
typedef __u64 u64; /**< 64-bit unsigned integer type. */
typedef __s64 s64; /**< 64-bit signed integer type. */
typedef __s8 s8; /**< 8-bit signed integer type. */
#endif

/**
* @brief Defines the callback used to respond to interrupt events.
*
* The handler type allows the caller to define a callback function to respond
* to interrupt events in the system. This function runs in interrupt context,
* so the amount of processing must be minimized.
*
* @param[in]   CallBackRef Callback reference passed in by the upper layer when
*              setting the callback functions, and passed back to
*              the upper layer when the callback is invoked. Its
*              type is not important to the driver, so it is a void
*              pointer.
* @param[in]   Type Indicates ADC or DAC.
* @param[in]   Tile_Id Indicates Tile number (0-3).
* @param[in]   Block_Id Indicates Block number (0-3).
* @param[in]   StatusEvent Indicates one or more interrupt occurred.
*/
typedef void (*XRFdc_StatusHandler)(void *CallBackRef, u32 Type, u32 Tile_Id, u32 Block_Id, u32 StatusEvent);
#ifndef __BAREMETAL__
#pragma pack(1)
#endif
/**
 * PLL settings.
 */
typedef struct {
	u32 Enabled; /**< PLL enable status (not a setter). */
	double RefClkFreq; /**< Reference clock frequency. */
	double SampleRate; /**< Sample rate. */
	u32 RefClkDivider; /**< Reference clock divider. */
	u32 FeedbackDivider; /**< Feedback divider. */
	u32 OutputDivider; /**< Output divider. */
	u32 FractionalMode; /**< Fractional mode is currently not supported. */
	u64 FractionalData; /**< Fractional data is currently not supported. */
	u32 FractWidth; /**< Fractional width is currently not supported. */
} XRFdc_PLL_Settings;
/**
* ClkIntraTile Settings.
*/
typedef struct {
	u8 SourceType; /**< Clock source type. */
	u8 SourceTile; /**< Source tile that provides the clock. */
	u32 PLLEnable; /**< PLL enable status. */
	double RefClkFreq; /**< Reference clock frequency. */
	double SampleRate; /**< Sample rate. */
	u8 DivisionFactor; /**< Clock division factor. */
	u8 DistributedClock; /**< Distributed clock configuration. */
	u8 Delay; /**< Clock delay. */
} XRFdc_Tile_Clock_Settings;
/**
* Clk Distribution Settings.
*/
typedef struct {
	u8 MaxDelay; /**< Maximum delay across the distribution. */
	u8 MinDelay; /**< Minimum delay across the distribution. */
	u8 IsDelayBalanced; /**< Indicates whether the delay is balanced. */
	u8 Source; /**< Distribution source tile. */
	u8 UpperBound; /**< Upper bound tile of the distribution. */
	u8 LowerBound; /**< Lower bound tile of the distribution. */
	XRFdc_Tile_Clock_Settings ClkSettings[2][4]; /**< [Type][Tile], for
	 * example ClkSettings[XRFDC_ADC_TILE][1] for ADC1. */
} XRFdc_Distribution_Info;

/**
 * Clock distribution configuration.
 */
typedef struct {
	u32 SourceType; /**< Clock source type. */
	u32 SourceTileId; /**< Source tile that provides the clock. */
	u32 EdgeTileIds[2]; /**< Edge tile identifiers of the distribution. */
	u32 EdgeTypes[2]; /**< Edge types of the distribution. */
	double DistRefClkFreq; /**< Distribution reference clock frequency. */
	u32 DistributedClock; /**< Distributed clock configuration. */
	double SampleRates[2][4]; /**< [Type][Tile], for example
				   * SampleRates[XRFDC_ADC_TILE][1] for ADC1. */
	u32 ShutdownMode; /**< Shutdown mode of the distribution. */
	XRFdc_Distribution_Info Info; /**< Resulting distribution
	 * information. */
} XRFdc_Distribution_Settings;

/**
 * Clock distribution system configuration.
 */
typedef struct {
	XRFdc_Distribution_Settings Distributions[8]; /**< Per-tile
	 * distributions. */
} XRFdc_Distribution_System_Settings;

/**
 * MTS DTC Settings.
 */
typedef struct {
	u32 RefTile; /**< Reference tile for the DTC scan. */
	u32 IsPLL; /**< Indicates whether the scan targets the PLL DTC. */
	int Target[4]; /**< Target DTC code per tile. */
	int Scan_Mode; /**< DTC scan mode. */
	int DTC_Code[4]; /**< Resulting DTC code per tile. */
	int Num_Windows[4]; /**< Number of valid windows found per tile. */
	int Max_Gap[4]; /**< Maximum gap between windows per tile. */
	int Min_Gap[4]; /**< Minimum gap between windows per tile. */
	int Max_Overlap[4]; /**< Maximum window overlap per tile. */
} XRFdc_MTS_DTC_Settings;

/**
 * MTS Sync Settings.
 */
typedef struct {
	u32 RefTile; /**< Reference tile for synchronization. */
	u32 Tiles; /**< Bit mask of tiles to synchronize. */
	int Target_Latency; /**< Target latency for synchronization. */
	int Offset[4]; /**< Measured offset per tile. */
	int Latency[4]; /**< Measured latency per tile. */
	int Marker_Delay; /**< Marker delay applied during synchronization. */
	int SysRef_Enable; /**< Enables SYSREF capture during
	 * synchronization. */
	XRFdc_MTS_DTC_Settings DTC_Set_PLL; /**< PLL DTC scan settings. */
	XRFdc_MTS_DTC_Settings DTC_Set_T1; /**< T1 DTC scan settings. */
} XRFdc_MultiConverter_Sync_Config;

/**
 * MTS Marker Struct.
 */
typedef struct {
	u32 Count[4]; /**< Marker count per tile. */
	u32 Loc[4]; /**< Marker location per tile. */
} XRFdc_MTS_Marker;

/**
 * ADC Signal Detect Settings.
 */
typedef struct {
	u8 Mode; /**< Signal detector mode. */
	u8 TimeConstant; /**< Integration time constant. */
	u8 Flush; /**< Flushes the integrator. */
	u8 EnableIntegrator; /**< Enables the integrator. */
	u16 Threshold; /**< Signal detect threshold. */
	u16 ThreshOnTriggerCnt; /**< Number of times the value must exceed
				 * Threshold before turning on. */
	u16 ThreshOffTriggerCnt; /**< Number of times the value must be less
				  * than Threshold before turning off. */
	u8 HysteresisEnable; /**< Enables hysteresis. */
} XRFdc_Signal_Detector_Settings;
/**
 * QMC settings.
 */
typedef struct {
	u32 EnablePhase; /**< Enables phase correction. */
	u32 EnableGain; /**< Enables gain correction. */
	double GainCorrectionFactor; /**< Gain correction factor. */
	double PhaseCorrectionFactor; /**< Phase correction factor. */
	s32 OffsetCorrectionFactor; /**< Offset correction factor. */
	u32 EventSource; /**< Event source that triggers the update. */
} XRFdc_QMC_Settings;

/**
 * Coarse delay settings.
 */
typedef struct {
	u32 CoarseDelay; /**< Coarse delay value. */
	u32 EventSource; /**< Event source that triggers the update. */
} XRFdc_CoarseDelay_Settings;

/**
 * Mixer settings.
 */
typedef struct {
	double Freq; /**< Fine mixer NCO frequency. */
	double PhaseOffset; /**< NCO phase offset. */
	u32 EventSource; /**< Event source that triggers the update. */
	u32 CoarseMixFreq; /**< Coarse mixer frequency. */
	u32 MixerMode; /**< Mixer mode. */
	u8 FineMixerScale; /**< NCO output scale, valid values 0, 1 and 2. */
	u8 MixerType; /**< Mixer type. */
} XRFdc_Mixer_Settings;

/**
 * ADC block Threshold settings.
 */
typedef struct {
	u32 UpdateThreshold; /**< Selects which threshold to update. */
	u32 ThresholdMode[2]; /**< Entry 0 for Threshold0 and 1 for
	 * Threshold1. */
	u32 ThresholdAvgVal[2]; /**< Entry 0 for Threshold0 and 1 for
	 * Threshold1. */
	u32 ThresholdUnderVal[2]; /**< Entry 0 for Threshold0 and 1 for
	 * Threshold1. */
	u32 ThresholdOverVal[2]; /**< Entry 0 for Threshold0 and 1 for
	 * Threshold1. */
} XRFdc_Threshold_Settings;

/**
 * RFSoC Calibration coefficients generic struct
 */
typedef struct {
	u32 Coeff0; /**< Calibration coefficient 0. */
	u32 Coeff1; /**< Calibration coefficient 1. */
	u32 Coeff2; /**< Calibration coefficient 2. */
	u32 Coeff3; /**< Calibration coefficient 3. */
	u32 Coeff4; /**< Calibration coefficient 4. */
	u32 Coeff5; /**< Calibration coefficient 5. */
	u32 Coeff6; /**< Calibration coefficient 6. */
	u32 Coeff7; /**< Calibration coefficient 7. */
} XRFdc_Calibration_Coefficients;

/**
 * RFSoC Power Mode settings
 */
typedef struct {
	u32 DisableIPControl; /**< Disables IP RTS control of the power mode. */
	u32 PwrMode; /**< The power mode. */
} XRFdc_Pwr_Mode_Settings;

/**
 * RFSoC DSA settings
 */
typedef struct {
	u32 DisableRTS; /**< Disables RTS control of DSA attenuation. */
	float Attenuation; /**< Attenuation. */
} XRFdc_DSA_Settings;

/**
 * RFSoC Calibration freeze settings struct
 */
typedef struct {
	u32 CalFrozen; /**< Status indicates calibration freeze state. */
	u32 DisableFreezePin; /**< Disables the calibration freeze pin. */
	u32 FreezeCalibration; /**< Setter for freezing. */
} XRFdc_Cal_Freeze_Settings;

/**
 * RFSoC Tile status.
 */
typedef struct {
	u32 IsEnabled; /**< 1 if tile is enabled, 0 otherwise. */
	u32 TileState; /**< Indicates tile current state. */
	u8 BlockStatusMask; /**< Bit mask for block status, 1 indicates block
			     * enable. */
	u32 PowerUpState; /**< Power-up state of the tile. */
	u32 PLLState; /**< PLL state of the tile. */
} XRFdc_TileStatus;

/**
 * RFSoC Data converter IP status.
 */
typedef struct {
	XRFdc_TileStatus DACTileStatus[4]; /**< Status of each DAC tile. */
	XRFdc_TileStatus ADCTileStatus[4]; /**< Status of each ADC tile. */
	u32 State; /**< Overall IP state. */
} XRFdc_IPStatus;

/**
 * status of DAC or ADC blocks in the RFSoC Data converter.
 */
typedef struct {
	double SamplingFreq; /**< Sampling frequency. */
	u32 AnalogDataPathStatus; /**< Analog datapath status. */
	u32 DigitalDataPathStatus; /**< Digital datapath status. */
	u8 DataPathClocksStatus; /**< Indicates whether all required datapath
				  * clocks are enabled, 1 if all clocks enabled,
				  * 0 otherwise. */
	u8 IsFIFOFlagsEnabled; /**< Indicates whether FIFO flags are enabled,
				* 1 if all flags enabled, 0 otherwise. */
	u8 IsFIFOFlagsAsserted; /**< Indicates whether FIFO flags are asserted,
				 * 1 if all flags asserted, 0 otherwise. */
} XRFdc_BlockStatus;

/**
 * DAC block Analog DataPath Config settings.
 */
typedef struct {
	u32 BlockAvailable; /**< Indicates whether the block is available. */
	u32 InvSyncEnable; /**< Inverse-sinc filter enable status. */
	u32 MixMode; /**< Mix mode. */
	u32 DecoderMode; /**< Decoder mode. */
} XRFdc_DACBlock_AnalogDataPath_Config;

/**
 * DAC block Digital DataPath Config settings.
 */
typedef struct {
	u32 MixerInputDataType; /**< Mixer input data type. */
	u32 DataWidth; /**< Data width. */
	u32 InterpolationMode; /**< Interpolation mode. */
	u32 FifoEnable; /**< FIFO enable status. */
	u32 AdderEnable; /**< Adder enable status. */
	u32 MixerType; /**< Mixer type. */
	double NCOFreq; /**< NCO frequency. */
} XRFdc_DACBlock_DigitalDataPath_Config;

/**
 * ADC block Analog DataPath Config settings.
 */
typedef struct {
	u32 BlockAvailable; /**< Indicates whether the block is available. */
	u32 MixMode; /**< Mix mode. */
} XRFdc_ADCBlock_AnalogDataPath_Config;

/**
 * ADC block Digital DataPath Config settings.
 */
typedef struct {
	u32 MixerInputDataType; /**< Mixer input data type. */
	u32 DataWidth; /**< Data width. */
	u32 DecimationMode; /**< Decimation mode. */
	u32 FifoEnable; /**< FIFO enable status. */
	u32 MixerType; /**< Mixer type. */
	double NCOFreq; /**< NCO frequency. */
} XRFdc_ADCBlock_DigitalDataPath_Config;

/**
 * DAC Tile Config structure.
 */
typedef struct {
	u32 Enable; /**< Tile enable status. */
	u32 PLLEnable; /**< PLL enable status. */
	double SamplingRate; /**< Sampling rate. */
	double RefClkFreq; /**< Reference clock frequency. */
	double FabClkFreq; /**< Fabric clock frequency. */
	u32 FeedbackDiv; /**< PLL feedback divider. */
	u32 OutputDiv; /**< PLL output divider. */
	u32 RefClkDiv; /**< Reference clock divider. */
	u32 MultibandConfig; /**< Multiband configuration. */
	double MaxSampleRate; /**< Maximum sample rate. */
	u32 NumSlices; /**< Number of slices. */
	u32 LinkCoupling; /**< Link coupling mode. */
	XRFdc_DACBlock_AnalogDataPath_Config DACBlock_Analog_Config[4]; /**<
		Per-block analog datapath configuration. */
	XRFdc_DACBlock_DigitalDataPath_Config DACBlock_Digital_Config[4]; /**<
		Per-block digital datapath configuration. */
} XRFdc_DACTile_Config;

/**
 * ADC Tile Config Structure.
 */
typedef struct {
	u32 Enable; /**< Tile enable status. */
	u32 PLLEnable; /**< PLL enable status. */
	double SamplingRate; /**< Sampling rate. */
	double RefClkFreq; /**< Reference clock frequency. */
	double FabClkFreq; /**< Fabric clock frequency. */
	u32 FeedbackDiv; /**< PLL feedback divider. */
	u32 OutputDiv; /**< PLL output divider. */
	u32 RefClkDiv; /**< Reference clock divider. */
	u32 MultibandConfig; /**< Multiband configuration. */
	double MaxSampleRate; /**< Maximum sample rate. */
	u32 NumSlices; /**< Number of slices. */
	XRFdc_ADCBlock_AnalogDataPath_Config ADCBlock_Analog_Config[4]; /**<
		Per-block analog datapath configuration. */
	XRFdc_ADCBlock_DigitalDataPath_Config ADCBlock_Digital_Config[4]; /**<
		Per-block digital datapath configuration. */
} XRFdc_ADCTile_Config;

/**
 * RFdc Config Structure.
 */
typedef struct {
#ifndef SDT
	u32 DeviceId; /**< Unique device identifier. */
#else
	char *Name; /**< Unique name of the device. */
#endif
	metal_phys_addr_t BaseAddr; /**< Device base address. */
	u32 ADCType; /**< ADC type 4GSPS or 2GSPS. */
	u32 MasterADCTile; /**< ADC master tile. */
	u32 MasterDACTile; /**< DAC master tile. */
	u32 ADCSysRefSource; /**< ADC SYSREF source. */
	u32 DACSysRefSource; /**< DAC SYSREF source. */
	u32 IPType; /**< IP type. */
	u32 SiRevision; /**< Silicon revision. */
	XRFdc_DACTile_Config DACTile_Config[4]; /**< Per-tile DAC
	 * configuration. */
	XRFdc_ADCTile_Config ADCTile_Config[4]; /**< Per-tile ADC
	 * configuration. */
} XRFdc_Config;

/**
 * DAC Block Analog DataPath Structure.
 */
typedef struct {
	u32 Enabled; /**< DAC analog datapath enable. */
	u32 MixedMode; /**< Mixed mode. */
	double TerminationVoltage; /**< Termination voltage. */
	double OutputCurrent; /**< Output current. */
	u32 InverseSincFilterEnable; /**< Inverse-sinc filter enable status. */
	u32 DecoderMode; /**< Decoder mode. */
	void *FuncHandler; /**< Function handler. */
	u32 NyquistZone; /**< Nyquist zone. */
	u8 AnalogPathEnabled; /**< Analog path enabled status. */
	u8 AnalogPathAvailable; /**< Analog path available status. */
	XRFdc_QMC_Settings QMC_Settings; /**< QMC settings. */
	XRFdc_CoarseDelay_Settings CoarseDelay_Settings; /**< Coarse delay
	 * settings. */
} XRFdc_DACBlock_AnalogDataPath;

/**
 * DAC Block Digital DataPath Structure.
 */
typedef struct {
	u32 MixerInputDataType; /**< Mixer input data type. */
	u32 DataWidth; /**< Data width. */
	int ConnectedIData; /**< Connected I data path. */
	int ConnectedQData; /**< Connected Q data path. */
	u32 InterpolationFactor; /**< Interpolation factor. */
	u8 DigitalPathEnabled; /**< Digital path enabled status. */
	u8 DigitalPathAvailable; /**< Digital path available status. */
	XRFdc_Mixer_Settings Mixer_Settings; /**< Mixer settings. */
} XRFdc_DACBlock_DigitalDataPath;

/**
 * ADC Block Analog DataPath Structure.
 */
typedef struct {
	u32 Enabled; /**< ADC analog datapath enable. */
	XRFdc_QMC_Settings QMC_Settings; /**< QMC settings. */
	XRFdc_CoarseDelay_Settings CoarseDelay_Settings; /**< Coarse delay
	 * settings. */
	XRFdc_Threshold_Settings Threshold_Settings; /**< Threshold settings. */
	u32 NyquistZone; /**< Nyquist zone. */
	u8 CalibrationMode; /**< Calibration mode. */
	u8 AnalogPathEnabled; /**< Analog path enabled status. */
	u8 AnalogPathAvailable; /**< Analog path available status. */
} XRFdc_ADCBlock_AnalogDataPath;

/**
 * ADC Block Digital DataPath Structure.
 */
typedef struct {
	u32 MixerInputDataType; /**< Mixer input data type. */
	u32 DataWidth; /**< Data width. */
	u32 DecimationFactor; /**< Decimation factor. */
	int ConnectedIData; /**< Connected I data path. */
	int ConnectedQData; /**< Connected Q data path. */
	u8 DigitalPathEnabled; /**< Digital path enabled status. */
	u8 DigitalPathAvailable; /**< Digital path available status. */
	XRFdc_Mixer_Settings Mixer_Settings; /**< Mixer settings. */
} XRFdc_ADCBlock_DigitalDataPath;

/**
 * DAC Tile Structure.
 */
typedef struct {
	u32 TileBaseAddr; /**< Tile base address. */
	u32 NumOfDACBlocks; /**< Number of DAC blocks enabled. */
	XRFdc_PLL_Settings PLL_Settings; /**< PLL settings. */
	u8 MultibandConfig; /**< Multiband configuration. */
	XRFdc_DACBlock_AnalogDataPath DACBlock_Analog_Datapath[4]; /**<
		Per-block analog datapath. */
	XRFdc_DACBlock_DigitalDataPath DACBlock_Digital_Datapath[4]; /**<
		Per-block digital datapath. */
} XRFdc_DAC_Tile;

/**
 * ADC Tile Structure.
 */
typedef struct {
	u32 TileBaseAddr; /**< Tile base address. */
	u32 NumOfADCBlocks; /**< Number of ADC blocks enabled. */
	XRFdc_PLL_Settings PLL_Settings; /**< PLL settings. */
	u8 MultibandConfig; /**< Multiband configuration. */
	XRFdc_ADCBlock_AnalogDataPath ADCBlock_Analog_Datapath[4]; /**<
		Per-block analog datapath. */
	XRFdc_ADCBlock_DigitalDataPath ADCBlock_Digital_Datapath[4]; /**<
		Per-block digital datapath. */
} XRFdc_ADC_Tile;

/**
 * RFdc Structure.
 */
typedef struct {
	XRFdc_Config RFdc_Config; /**< Config structure. */
	u32 IsReady; /**< Device readiness status. */
	u32 ADC4GSPS; /**< Indicates 4GSPS ADC operation. */
	metal_phys_addr_t BaseAddr; /**< Base address. */
	struct metal_io_region *io; /**< Libmetal IO structure. */
	struct metal_device *device; /**< Libmetal device structure. */
	XRFdc_DAC_Tile DAC_Tile[4]; /**< Per-tile DAC data. */
	XRFdc_ADC_Tile ADC_Tile[4]; /**< Per-tile ADC data. */
	XRFdc_StatusHandler StatusHandler; /**< Event handler function. */
	void *CallBackRef; /**< Callback reference for event handler. */
	u8 UpdateMixerScale; /**< Set to 1 if the caller overwrites mixer
	 * scale. */
} XRFdc;
#ifndef __BAREMETAL__
#pragma pack()
#endif

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef __BAREMETAL__
/**
* @brief Asserts an expression in a function that returns a value.
*
* Loops forever if the expression is false.
*
* @param[in]   Expression Expression to evaluate.
*
* @note        C-style signature:
*              void Xil_AssertNonvoid(int Expression)
*/
#define Xil_AssertNonvoid(Expression)                                                                                  \
	{                                                                                                              \
		if (!(Expression)) {                                                                                   \
			while (1)                                                                                      \
				;                                                                                      \
		}                                                                                                      \
	}
/**
* @brief Asserts an expression in a function that does not return a value.
*
* Loops forever if the expression is false.
*
* @param[in]   Expression Expression to evaluate.
*
* @note        C-style signature:
*              void Xil_AssertVoid(int Expression)
*/
#define Xil_AssertVoid(Expression)                                                                                     \
	{                                                                                                              \
		if (!(Expression)) {                                                                                   \
			while (1)                                                                                      \
				;                                                                                      \
		}                                                                                                      \
	}
/**
* @brief Asserts unconditionally.
*
* Loops forever.
*
* @note        C-style signature:
*              void Xil_AssertVoidAlways(void)
*/
#define Xil_AssertVoidAlways()                                                                                         \
	{                                                                                                              \
		while (1)                                                                                              \
			;                                                                                              \
	}
#endif

/**
* @brief Returns the larger of two values.
*
* @param[in]   x First value.
* @param[in]   y Second value.
*
* @return      The larger of x and y.
*
* @note        C-style signature:
*              u8 XRFDC_MAX(u8 x, u8 y)
*/
#define XRFDC_MAX(x, y) (x > y) ? x : y
/**
* @brief Returns the smaller of two values.
*
* @param[in]   x First value.
* @param[in]   y Second value.
*
* @return      The smaller of x and y.
*
* @note        C-style signature:
*              u8 XRFDC_MIN(u8 x, u8 y)
*/
#define XRFDC_MIN(x, y) (x < y) ? x : y
#define XRFDC_SUCCESS 0U /**< Status code for success. */
#define XRFDC_FAILURE 1U /**< Status code for failure. */
#define XRFDC_GEN3 2 /**< IP generation value for Gen 3 devices. */
/** Value of IsReady when the instance is initialized. */
#define XRFDC_COMPONENT_IS_READY 0x11111111U
/** Number of slices in a high-speed ADC tile. */
#define XRFDC_NUM_SLICES_HSADC 2
/** Number of slices in a low-speed ADC tile. */
#define XRFDC_NUM_SLICES_LSADC 4
#ifndef __BAREMETAL__
/** Directory of the Linux platform devices. */
#define XRFDC_PLATFORM_DEVICE_DIR "/sys/bus/platform/devices/"
/** Name of the libmetal bus used for the device. */
#define XRFDC_BUS_NAME "platform"
#define XRFDC_SIGNATURE "usp_rf_data_converter" /**< String in RFDC node
						 * name. */
#define XRFDC_SIGNATURE_SDT "usp-rf-data-converter" /**< SDT/Lopper flow. */
#define XRFDC_CONFIG_DATA_PROPERTY "param-list" /**< Device tree property. */
#define XRFDC_COMPATIBLE_PROPERTY "compatible" /**< Device tree property. */
#define XRFDC_NUM_INSTANCES_PROPERTY "num-insts" /**< Device tree property. */
/** Device tree compatible string prefix. */
#define XRFDC_COMPATIBLE_STRING "xlnx,usp-rf-data-converter-"
#define XRFDC_DEVICE_ID_SIZE 4U /**< Size of the device ID property in bytes. */
/** Size of the number of instances property in bytes. */
#define XRFDC_NUM_INST_SIZE 4U
/** Size of the configuration data. */
#define XRFDC_CONFIG_DATA_SIZE sizeof(XRFdc_Config)
#else
/** Name of the libmetal bus used for the device. */
#define XRFDC_BUS_NAME "generic"
#ifndef SDT
#define XRFDC_DEV_NAME XPAR_XRFDC_0_DEV_NAME /**< Name of the RFDC device. */
#endif
#endif
#define XRFDC_REGION_SIZE 0x40000U /**< Size of the RFDC register region. */
#define XRFDC_IP_BASE 0x0U /**< Base offset of the IP registers. */
/**
* @brief Returns the DRP base address of a tile.
*
* @param[in]   type ADC or DAC. 0 for ADC and 1 for DAC.
* @param[in]   tile Tile number.
*
* @return      DRP base address of the tile.
*
* @note        C-style signature:
*              u32 XRFDC_DRP_BASE(u32 type, u32 tile)
*/
#define XRFDC_DRP_BASE(type, tile)                                                                                     \
	((type) == XRFDC_ADC_TILE ? XRFDC_ADC_TILE_DRP_ADDR(tile) : XRFDC_DAC_TILE_DRP_ADDR(tile))

/**
* @brief Returns the control and status base address of a tile.
*
* @param[in]   Type ADC or DAC. 0 for ADC and 1 for DAC.
* @param[in]   Tile Tile number.
*
* @return      Control and status base address of the tile.
*
* @note        C-style signature:
*              u32 XRFDC_CTRL_STS_BASE(u32 Type, u32 Tile)
*/
#define XRFDC_CTRL_STS_BASE(Type, Tile)                                                                                \
	((Type) == XRFDC_ADC_TILE ? XRFDC_ADC_TILE_CTRL_STATS_ADDR(Tile) : XRFDC_DAC_TILE_CTRL_STATS_ADDR(Tile))

/**
* @brief Returns the base address of a block.
*
* @param[in]   Type ADC or DAC. 0 for ADC and 1 for DAC.
* @param[in]   Tile Tile number.
* @param[in]   Block Block number.
*
* @return      Base address of the block.
*
* @note        C-style signature:
*              u32 XRFDC_BLOCK_BASE(u32 Type, u32 Tile, u32 Block)
*/
#define XRFDC_BLOCK_BASE(Type, Tile, Block)                                                                            \
	((Type) == XRFDC_ADC_TILE ? (XRFDC_ADC_TILE_DRP_ADDR(Tile) + XRFDC_BLOCK_ADDR_OFFSET(Block)) :                 \
				    (XRFDC_DAC_TILE_DRP_ADDR(Tile) + XRFDC_BLOCK_ADDR_OFFSET(Block)))

#define XRFDC_ADC_TILE 0U /**< ADC tile type. */
#define XRFDC_DAC_TILE 1U /**< DAC tile type. */
#define XRFDC_TILE_ID_MAX 0x3U /**< Maximum tile ID. */
#define XRFDC_BLOCK_ID_MAX 0x3U /**< Maximum block ID. */
/** Event source: immediate (register write). */
#define XRFDC_EVNT_SRC_IMMEDIATE 0x00000000U
#define XRFDC_EVNT_SRC_SLICE 0x00000001U /**< Event source: slice. */
#define XRFDC_EVNT_SRC_TILE 0x00000002U /**< Event source: tile. */
#define XRFDC_EVNT_SRC_SYSREF 0x00000003U /**< Event source: SYSREF. */
#define XRFDC_EVNT_SRC_MARKER 0x00000004U /**< Event source: marker. */
#define XRFDC_EVNT_SRC_PL 0x00000005U /**< Event source: PL. */
/** Update event for the mixer settings. */
#define XRFDC_EVENT_MIXER 0x00000001U
/** Update event for the coarse delay settings. */
#define XRFDC_EVENT_CRSE_DLY 0x00000002U
#define XRFDC_EVENT_QMC 0x00000004U /**< Update event for the QMC settings. */
#define XRFDC_SELECT_ALL_TILES -1 /**< Selects all the tiles. */
#define XRFDC_ADC_4GSPS 1U /**< ADC type value for a 4 GSPS ADC. */

#define XRFDC_CRSE_DLY_MAX 0x7U /**< Maximum coarse delay. */
#define XRFDC_CRSE_DLY_MAX_EXT 0x28U /**< Maximum extended coarse delay. */
#define XRFDC_NCO_FREQ_MULTIPLIER (0x1LLU << 48U) /**< 2^48. */
#define XRFDC_NCO_PHASE_MULTIPLIER (1U << 17U) /**< 2^17. */
#define XRFDC_QMC_PHASE_MULT (1U << 11U) /**< 2^11. */
#define XRFDC_QMC_GAIN_MULT (1U << 14U) /**< 2^14. */

#define XRFDC_DATA_TYPE_IQ 0x00000001U /**< I/Q data type. */
#define XRFDC_DATA_TYPE_REAL 0x00000000U /**< Real data type. */

#define XRFDC_TRSHD_OFF 0x0U /**< Threshold mode: off. */
#define XRFDC_TRSHD_STICKY_OVER 0x00000001U /**< Threshold mode: sticky over. */
/** Threshold mode: sticky under. */
#define XRFDC_TRSHD_STICKY_UNDER 0x00000002U
#define XRFDC_TRSHD_HYSTERISIS 0x00000003U /**< Threshold mode: hysteresis. */

/* Mixer modes */
#define XRFDC_MIXER_MODE_OFF 0x0U /**< Mixer mode: off. */
#define XRFDC_MIXER_MODE_C2C 0x1U /**< Mixer mode: complex to complex. */
#define XRFDC_MIXER_MODE_C2R 0x2U /**< Mixer mode: complex to real. */
#define XRFDC_MIXER_MODE_R2C 0x3U /**< Mixer mode: real to complex. */
#define XRFDC_MIXER_MODE_R2R 0x4U /**< Mixer mode: real to real. */

/** Coarse mixer I path value for cos and -sin. */
#define XRFDC_I_IQ_COS_MINSIN 0x00000C00U
/** Coarse mixer Q path value for sin and cos. */
#define XRFDC_Q_IQ_SIN_COS 0x00001000U
#define XRFDC_EN_I_IQ 0x00000001U /**< Enable bit for the I path in I/Q mode. */
#define XRFDC_EN_Q_IQ 0x00000004U /**< Enable bit for the Q path in I/Q mode. */

#define XRFDC_MIXER_TYPE_COARSE 0x1U /**< Mixer type: coarse. */
#define XRFDC_MIXER_TYPE_FINE 0x2U /**< Mixer type: fine. */

#define XRFDC_MIXER_TYPE_OFF 0x0U /**< Mixer type: off. */
#define XRFDC_MIXER_TYPE_DISABLED 0x3U /**< Mixer type: disabled. */

#define XRFDC_COARSE_MIX_OFF 0x0U /**< Coarse mixer frequency: off. */
/** Coarse mixer frequency: Fs/2. */
#define XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_TWO 0x2U
/** Coarse mixer frequency: Fs/4. */
#define XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR 0x4U
/** Coarse mixer frequency: -Fs/4. */
#define XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR 0x8U
#define XRFDC_COARSE_MIX_BYPASS 0x10U /**< Coarse mixer frequency: bypass. */

/** Coarse mixer mode: complex to complex or complex to real. */
#define XRFDC_COARSE_MIX_MODE_C2C_C2R 0x1U
/** Coarse mixer mode: real to complex. */
#define XRFDC_COARSE_MIX_MODE_R2C 0x2U

#define XRFDC_CRSE_MIX_OFF 0x924U /**< Coarse mixer register value: off. */
#define XRFDC_CRSE_MIX_BYPASS 0x0U /**< Coarse mixer register value: bypass. */
/** Coarse mixer register value: Fs/2 on odd slices (4 GSPS). */
#define XRFDC_CRSE_4GSPS_ODD_FSBYTWO 0x492U
/** Coarse mixer register value: I path Fs/4 on odd slices. */
#define XRFDC_CRSE_MIX_I_ODD_FSBYFOUR 0x2CBU
/** Coarse mixer register value: Q path Fs/4 on odd slices. */
#define XRFDC_CRSE_MIX_Q_ODD_FSBYFOUR 0x659U
/** Coarse mixer register value: I and Q paths Fs/2. */
#define XRFDC_CRSE_MIX_I_Q_FSBYTWO 0x410U
/** Coarse mixer register value: I path Fs/4. */
#define XRFDC_CRSE_MIX_I_FSBYFOUR 0x298U
/** Coarse mixer register value: Q path Fs/4. */
#define XRFDC_CRSE_MIX_Q_FSBYFOUR 0x688U
/** Coarse mixer register value: I path -Fs/4. */
#define XRFDC_CRSE_MIX_I_MINFSBYFOUR 0x688U
/** Coarse mixer register value: Q path -Fs/4. */
#define XRFDC_CRSE_MIX_Q_MINFSBYFOUR 0x298U
/** Coarse mixer register value: real input, I path Fs/4. */
#define XRFDC_CRSE_MIX_R_I_FSBYFOUR 0x8A0U
/** Coarse mixer register value: real input, Q path Fs/4. */
#define XRFDC_CRSE_MIX_R_Q_FSBYFOUR 0x70CU
/** Coarse mixer register value: real input, I path -Fs/4. */
#define XRFDC_CRSE_MIX_R_I_MINFSBYFOUR 0x8A0U
/** Coarse mixer register value: real input, Q path -Fs/4. */
#define XRFDC_CRSE_MIX_R_Q_MINFSBYFOUR 0x31CU

#define XRFDC_MIXER_SCALE_AUTO 0x0U /**< Mixer scale: automatic. */
#define XRFDC_MIXER_SCALE_1P0 0x1U /**< Mixer scale: 1.0. */
#define XRFDC_MIXER_SCALE_0P7 0x2U /**< Mixer scale: 0.7. */

/** Upper limit of the mixer phase offset in degrees. */
#define XRFDC_MIXER_PHASE_OFFSET_UP_LIMIT 180.0
/** Lower limit of the mixer phase offset in degrees. */
#define XRFDC_MIXER_PHASE_OFFSET_LOW_LIMIT (-180.0)
#define XRFDC_UPDATE_THRESHOLD_0 0x1U /**< Updates threshold 0. */
#define XRFDC_UPDATE_THRESHOLD_1 0x2U /**< Updates threshold 1. */
#define XRFDC_UPDATE_THRESHOLD_BOTH 0x4U /**< Updates both thresholds. */
/** Threshold clear mode: manual. */
#define XRFDC_THRESHOLD_CLRMD_MANUAL_CLR 0x1U
/** Threshold clear mode: automatic. */
#define XRFDC_THRESHOLD_CLRMD_AUTO_CLR 0x2U
#define XRFDC_DECODER_MAX_SNR_MODE 0x1U /**< Decoder mode: maximum SNR. */
/** Decoder mode: maximum linearity. */
#define XRFDC_DECODER_MAX_LINEARITY_MODE 0x2U
#define XRFDC_OUTPUT_CURRENT_32MA 32U /**< DAC output current: 32 mA. */
#define XRFDC_OUTPUT_CURRENT_20MA 20U /**< DAC output current: 20 mA. */

#define XRFDC_MIXER_MODE_IQ 0x1U /**< Mixer mode value for I/Q. */
#define XRFDC_ADC_MIXER_MODE_IQ 0x1U /**< ADC mixer mode value for I/Q. */
#define XRFDC_DAC_MIXER_MODE_REAL 0x2U /**< DAC mixer mode value for real. */

#define XRFDC_ODD_NYQUIST_ZONE 0x1U /**< Odd Nyquist zone. */
#define XRFDC_EVEN_NYQUIST_ZONE 0x2U /**< Even Nyquist zone. */

#define XRFDC_INTERP_DECIM_OFF 0x0U /**< Interpolation or decimation: off. */
/** Interpolation or decimation factor of 1. */
#define XRFDC_INTERP_DECIM_1X 0x1U
/** Interpolation or decimation factor of 2. */
#define XRFDC_INTERP_DECIM_2X 0x2U
/** Interpolation or decimation factor of 3. */
#define XRFDC_INTERP_DECIM_3X 0x3U
/** Interpolation or decimation factor of 4. */
#define XRFDC_INTERP_DECIM_4X 0x4U
/** Interpolation or decimation factor of 5. */
#define XRFDC_INTERP_DECIM_5X 0x5U
/** Interpolation or decimation factor of 6. */
#define XRFDC_INTERP_DECIM_6X 0x6U
/** Interpolation or decimation factor of 8. */
#define XRFDC_INTERP_DECIM_8X 0x8U
/** Interpolation or decimation factor of 10. */
#define XRFDC_INTERP_DECIM_10X 0xAU
/** Interpolation or decimation factor of 12. */
#define XRFDC_INTERP_DECIM_12X 0xCU
/** Interpolation or decimation factor of 16. */
#define XRFDC_INTERP_DECIM_16X 0x10U
/** Interpolation or decimation factor of 20. */
#define XRFDC_INTERP_DECIM_20X 0x14U
/** Interpolation or decimation factor of 24. */
#define XRFDC_INTERP_DECIM_24X 0x18U
/** Interpolation or decimation factor of 40. */
#define XRFDC_INTERP_DECIM_40X 0x28U

#define XRFDC_FAB_CLK_DIV1 0x1U /**< Fabric clock divider of 1. */
#define XRFDC_FAB_CLK_DIV2 0x2U /**< Fabric clock divider of 2. */
#define XRFDC_FAB_CLK_DIV4 0x3U /**< Fabric clock divider of 4. */
#define XRFDC_FAB_CLK_DIV8 0x4U /**< Fabric clock divider of 8. */
#define XRFDC_FAB_CLK_DIV16 0x5U /**< Fabric clock divider of 16. */

#define XRFDC_CALIB_MODE_AUTO 0x0U /**< Calibration mode: automatic. */
#define XRFDC_CALIB_MODE1 0x1U /**< Calibration mode 1. */
#define XRFDC_CALIB_MODE2 0x2U /**< Calibration mode 2. */
#define XRFDC_CALIB_MODE_MIXED 0x0U /**< Calibration mode: mixed. */
/** Calibration mode: absolute difference. */
#define XRFDC_CALIB_MODE_ABS_DIFF 0x1U
/** Calibration mode: negative absolute sum. */
#define XRFDC_CALIB_MODE_NEG_ABS_SUM 0x2U
/** TI DCB register value for calibration mode 1 (4 GSPS). */
#define XRFDC_TI_DCB_MODE1_4GSPS 0x00007800U
/** TI DCB register value for calibration mode 1 (2 GSPS). */
#define XRFDC_TI_DCB_MODE1_2GSPS 0x00005000U

/* PLL Configuration */
#define XRFDC_PLL_UNLOCKED 0x1U /**< PLL status: unlocked. */
#define XRFDC_PLL_LOCKED 0x2U /**< PLL status: locked. */

#define XRFDC_EXTERNAL_CLK 0x0U /**< Clock source: external clock. */
#define XRFDC_INTERNAL_PLL_CLK 0x1U /**< Clock source: internal PLL. */

#define PLL_FPDIV_MIN 13U /**< Minimum PLL feedback divider. */
#define PLL_FPDIV_MAX 128U /**< Maximum PLL feedback divider. */
#define PLL_DIVIDER_MIN 2U /**< Minimum PLL output divider. */
/** Minimum PLL output divider for Gen 3 devices. */
#define PLL_DIVIDER_MIN_GEN3 1U
#define PLL_DIVIDER_MAX 28U /**< Maximum PLL output divider. */
#define VCO_RANGE_MIN 8500U /**< Minimum VCO frequency in MHz. */
#define VCO_RANGE_MAX 13200U /**< Maximum VCO frequency in MHz. */
#define VCO_RANGE_ADC_MIN 8500U /**< Minimum ADC VCO frequency in MHz. */
#define VCO_RANGE_ADC_MAX 13200U /**< Maximum ADC VCO frequency in MHz. */
#define VCO_RANGE_DAC_MIN 7800U /**< Minimum DAC VCO frequency in MHz. */
#define VCO_RANGE_DAC_MAX 13800U /**< Maximum DAC VCO frequency in MHz. */
#define XRFDC_PLL_LPF1_VAL 0x6U /**< PLL loop filter 1 register value. */
#define XRFDC_PLL_CRS2_VAL 0x7008U /**< PLL CRS2 register value. */
#define XRFDC_VCO_UPPER_BAND 0x0U /**< VCO band: upper. */
#define XRFDC_VCO_LOWER_BAND 0x1U /**< VCO band: lower. */
#define XRFDC_REF_CLK_DIV_1 0x1U /**< Reference clock divider: 1. */
#define XRFDC_REF_CLK_DIV_2 0x2U /**< Reference clock divider: 2. */
#define XRFDC_REF_CLK_DIV_3 0x3U /**< Reference clock divider: 3. */
#define XRFDC_REF_CLK_DIV_4 0x4U /**< Reference clock divider: 4. */

#define XRFDC_SINGLEBAND_MODE 0x1U /**< Single-band mode. */
#define XRFDC_MULTIBAND_MODE_2X 0x2U /**< Dual-band (2X) multiband mode. */
#define XRFDC_MULTIBAND_MODE_4X 0x4U /**< Quad-band (4X) multiband mode. */

/** Multiband data type: complex to complex. */
#define XRFDC_MB_DATATYPE_C2C 0x1U
#define XRFDC_MB_DATATYPE_R2C 0x2U /**< Multiband data type: real to complex. */
#define XRFDC_MB_DATATYPE_C2R 0x4U /**< Multiband data type: complex to real. */

#define XRFDC_MB_DUAL_BAND 2U /**< Number of data paths in dual-band mode. */
#define XRFDC_MB_QUAD_BAND 4U /**< Number of data paths in quad-band mode. */

/** Single-band complex to complex value for block 0. */
#define XRFDC_SB_C2C_BLK0 0x82U
/** Single-band complex to complex value for block 1. */
#define XRFDC_SB_C2C_BLK1 0x64U
#define XRFDC_SB_C2R 0x40U /**< Single-band complex to real value. */
/** Multiband complex to complex value for block 0. */
#define XRFDC_MB_C2C_BLK0 0x5EU
/** Multiband complex to complex value for block 1. */
#define XRFDC_MB_C2C_BLK1 0x5DU
/** Multiband complex to real value for block 0. */
#define XRFDC_MB_C2R_BLK0 0x5CU
/** Multiband complex to real value for block 1. */
#define XRFDC_MB_C2R_BLK1 0x0U

#define XRFDC_MIXER_MODE_BYPASS 0x2U /**< Mixer mode value for bypass. */

#define XRFDC_LINK_COUPLING_DC 0x0U /**< Link coupling: DC. */
#define XRFDC_LINK_COUPLING_AC 0x1U /**< Link coupling: AC. */

#define XRFDC_MB_MODE_SB 0x0U /**< Multiband configuration: single band. */
/** Multiband configuration: 2X on blocks 0 and 1. */
#define XRFDC_MB_MODE_2X_BLK01 0x1U
/** Multiband configuration: 2X on blocks 2 and 3. */
#define XRFDC_MB_MODE_2X_BLK23 0x2U
/** Multiband configuration: 2X on blocks 0-1 and 2-3. */
#define XRFDC_MB_MODE_2X_BLK01_BLK23 0x3U
#define XRFDC_MB_MODE_4X 0x4U /**< Multiband configuration: 4X. */
/** Multiband configuration: alternate 2X on blocks 0-1 and 2-3. */
#define XRFDC_MB_MODE_2X_BLK01_BLK23_ALT 0x5U

#define XRFDC_MILLI 1000U /**< Milli multiplier (1000). */
#define XRFDC_MICRO 1000000U /**< Micro multiplier (1000000). */
#define XRFDC_DAC_SAMPLING_MIN 500 /**< Minimum DAC sampling rate in MHz. */
#define XRFDC_DAC_SAMPLING_MAX 6554 /**< Maximum DAC sampling rate in MHz. */
/** Minimum 4 GSPS ADC sampling rate in MHz. */
#define XRFDC_ADC_4G_SAMPLING_MIN 1000
/** Maximum 4 GSPS ADC sampling rate in MHz. */
#define XRFDC_ADC_4G_SAMPLING_MAX 4116
/** Minimum 2 GSPS ADC sampling rate in MHz. */
#define XRFDC_ADC_2G_SAMPLING_MIN 500
/** Maximum 2 GSPS ADC sampling rate in MHz. */
#define XRFDC_ADC_2G_SAMPLING_MAX 2058
/** Minimum reference clock frequency in MHz. */
#define XRFDC_REFFREQ_MIN 102.40625
/** Maximum reference clock frequency in MHz. */
#define XRFDC_REFFREQ_MAX 614.0

#define XRFDC_DIGITALPATH_ENABLE 0x1U /**< Digital path enabled. */
#define XRFDC_ANALOGPATH_ENABLE 0x1U /**< Analog path enabled. */

#define XRFDC_BLK_ID0 0x0U /**< Block ID 0. */
#define XRFDC_BLK_ID1 0x1U /**< Block ID 1. */
#define XRFDC_BLK_ID2 0x2U /**< Block ID 2. */
#define XRFDC_BLK_ID3 0x3U /**< Block ID 3. */
#define XRFDC_BLK_ID4 0x4U /**< Block ID 4. */

#define XRFDC_BLK_ID_NONE -1 /**< No block ID. */
#define XRFDC_BLK_ID_ALL -1 /**< All block IDs. */
#define XRFDC_BLK_ID_INV 0x4 /**< Invalid block ID. */

#define XRFDC_TILE_ID0 0x0U /**< Tile ID 0. */
#define XRFDC_TILE_ID1 0x1U /**< Tile ID 1. */
#define XRFDC_TILE_ID2 0x2U /**< Tile ID 2. */
#define XRFDC_TILE_ID3 0x3U /**< Tile ID 3. */
#define XRFDC_TILE_ID4 0x4U /**< Tile ID 4. */

#define XRFDC_TILE_ID_INV 0x4U /**< Invalid tile ID. */

#define XRFDC_NUM_OF_BLKS1 0x1U /**< Number of blocks: 1. */
#define XRFDC_NUM_OF_BLKS2 0x2U /**< Number of blocks: 2. */
#define XRFDC_NUM_OF_BLKS3 0x3U /**< Number of blocks: 3. */
#define XRFDC_NUM_OF_BLKS4 0x4U /**< Number of blocks: 4. */

#define XRFDC_NUM_OF_TILES1 0x1U /**< Number of tiles: 1. */
#define XRFDC_NUM_OF_TILES2 0x2U /**< Number of tiles: 2. */
#define XRFDC_NUM_OF_TILES3 0x3U /**< Number of tiles: 3. */
#define XRFDC_NUM_OF_TILES4 0x4U /**< Number of tiles: 4. */

#define XRFDC_SM_STATE0 0x0U /**< Tile state machine state 0. */
#define XRFDC_SM_STATE1 0x1U /**< Tile state machine state 1. */
#define XRFDC_SM_STATE3 0x3U /**< Tile state machine state 3. */
#define XRFDC_SM_STATE7 0x7U /**< Tile state machine state 7. */
#define XRFDC_SM_STATE15 0xFU /**< Tile state machine state 15. */

#define XRFDC_STATE_OFF 0x0U /**< Tile state: off. */
#define XRFDC_STATE_SHUTDOWN 0x1U /**< Tile state: shutdown. */
#define XRFDC_STATE_PWRUP 0x3U /**< Tile state: power up. */
#define XRFDC_STATE_CLK_DET 0x6U /**< Tile state: clock detect. */
#define XRFDC_STATE_CAL 0xBU /**< Tile state: calibration. */
#define XRFDC_STATE_FULL 0xFU /**< Tile state: fully started. */

/** Decimation data type value for 4 GSPS ADCs. */
#define XRFDC_DECIM_4G_DATA_TYPE 0x3U
/** Decimation data type value for 2 GSPS ADCs in I/Q mode. */
#define XRFDC_DECIM_2G_IQ_DATA_TYPE 0x2U

#define XRFDC_DAC_MAX_WR_FAB_RATE 16U /**< Maximum DAC fabric write rate. */
/**
* @brief Returns the maximum ADC fabric read rate.
*
* @param[in]   X IP type.
*
* @return      8 if the IP type is below XRFDC_GEN3, otherwise 12.
*
* @note        C-style signature:
*              u32 XRFDC_ADC_MAX_RD_FAB_RATE(u32 X)
*/
#define XRFDC_ADC_MAX_RD_FAB_RATE(X) ((X < XRFDC_GEN3) ? 8U : 12U)

/** Minimum QMC phase correction factor. */
#define XRFDC_MIN_PHASE_CORR_FACTOR -26.5
/** Maximum QMC phase correction factor. */
#define XRFDC_MAX_PHASE_CORR_FACTOR 26.5
/** Maximum QMC gain correction factor. */
#define XRFDC_MAX_GAIN_CORR_FACTOR 2.0
/** Minimum QMC gain correction factor. */
#define XRFDC_MIN_GAIN_CORR_FACTOR 0.0

#define XRFDC_FAB_RATE_16 16 /**< Fabric rate of 16 words. */
#define XRFDC_FAB_RATE_8 8 /**< Fabric rate of 8 words. */
#define XRFDC_FAB_RATE_4 4 /**< Fabric rate of 4 words. */
#define XRFDC_FAB_RATE_2 2 /**< Fabric rate of 2 words. */
#define XRFDC_FAB_RATE_1 1 /**< Fabric rate of 1 words. */

/** HSCOM power state value for the PLL clock. */
#define XRFDC_HSCOM_PWR_STATS_PLL 0xFFC0U
/** HSCOM power state value for the external clock. */
#define XRFDC_HSCOM_PWR_STATS_EXTERNAL 0xF240U
/** HSCOM power state value for an external RX clock. */
#define XRFDC_HSCOM_PWR_STATS_RX_EXT 0xF2FCU
/** HSCOM power state value for an external distributed clock. */
#define XRFDC_HSCOM_PWR_STATS_DIST_EXT 0xF0FEU
/** HSCOM power state value for the RX clock with the PLL. */
#define XRFDC_HSCOM_PWR_STATS_RX_PLL 0xFFFCU
/** HSCOM power state value for a distributed clock with the PLL. */
#define XRFDC_HSCOM_PWR_STATS_DIST_PLL 0xFDFEU
/** HSCOM power state value for an external RX clock with the divider. */
#define XRFDC_HSCOM_PWR_STATS_RX_EXT_DIV 0xF2FCU
/**
 * HSCOM power state value for an external distributed clock with the divider.
 */
#define XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV 0xF0FE
/** HSCOM power state value for the external distribution source. */
#define XRFDC_HSCOM_PWR_STATS_DIST_EXT_SRC 0xF2FCU
/**
 * HSCOM power state value for the external distribution source with the
 * divider.
 */
#define XRFDC_HSCOM_PWR_STATS_DIST_EXT_DIV_SRC 0xF2FCU

#define XRFDC_CLK_DST_TILE_231 0 /**< Clock distribution index of tile 231. */
#define XRFDC_CLK_DST_TILE_230 1 /**< Clock distribution index of tile 230. */
#define XRFDC_CLK_DST_TILE_229 2 /**< Clock distribution index of tile 229. */
#define XRFDC_CLK_DST_TILE_228 3 /**< Clock distribution index of tile 228. */
#define XRFDC_CLK_DST_TILE_227 4 /**< Clock distribution index of tile 227. */
#define XRFDC_CLK_DST_TILE_226 5 /**< Clock distribution index of tile 226. */
#define XRFDC_CLK_DST_TILE_225 6 /**< Clock distribution index of tile 225. */
#define XRFDC_CLK_DST_TILE_224 7 /**< Clock distribution index of tile 224. */
#define XRFDC_CLK_DST_INVALID 0xFFU /**< Invalid clock distribution tile. */

#define XRFDC_GLBL_OFST_DAC 0U /**< Global tile index offset for DAC tiles. */
#define XRFDC_GLBL_OFST_ADC 4U /**< Global tile index offset for ADC tiles. */
/**
* @brief Returns the global index of a tile.
*
* @param[in]   X ADC or DAC. 0 for ADC and 1 for DAC.
* @param[in]   Y Tile number.
*
* @return      Global index of the tile.
*
* @note        C-style signature:
*              u32 XRFDC_TILE_GLBL_ADDR(u32 X, u32 Y)
*/
#define XRFDC_TILE_GLBL_ADDR(X, Y) (Y + ((X == XRFDC_ADC_TILE) ? XRFDC_GLBL_OFST_ADC : XRFDC_GLBL_OFST_DAC))

/** Clock distribution MUX4A source: INT. */
#define XRFDC_CLK_DISTR_MUX4A_SRC_INT 0x0008U
/** Clock distribution MUX4A source: STH. */
#define XRFDC_CLK_DISTR_MUX4A_SRC_STH 0x0000U
/** Clock distribution MUX6 source: OFF. */
#define XRFDC_CLK_DISTR_MUX6_SRC_OFF 0x0000U
/** Clock distribution MUX6 source: INT. */
#define XRFDC_CLK_DISTR_MUX6_SRC_INT 0x0100U
/** Clock distribution MUX6 source: NTH. */
#define XRFDC_CLK_DISTR_MUX6_SRC_NTH 0x0080U
/** Clock distribution MUX7 source: OFF. */
#define XRFDC_CLK_DISTR_MUX7_SRC_OFF 0x0000U
/** Clock distribution MUX7 source: STH. */
#define XRFDC_CLK_DISTR_MUX7_SRC_STH 0x0200U
/** Clock distribution MUX7 source: INT. */
#define XRFDC_CLK_DISTR_MUX7_SRC_INT 0x0400U
/** Clock distribution MUX8 source: NTH. */
#define XRFDC_CLK_DISTR_MUX8_SRC_NTH 0x0000U
/** Clock distribution MUX8 source: INT. */
#define XRFDC_CLK_DISTR_MUX8_SRC_INT 0x8000U
/** Clock distribution MUX9 source: NTH. */
#define XRFDC_CLK_DISTR_MUX9_SRC_NTH 0x4000U
/** Clock distribution MUX9 source: INT. */
#define XRFDC_CLK_DISTR_MUX9_SRC_INT 0x0000U
/** Clock distribution MUX5A source: PLL. */
#define XRFDC_CLK_DISTR_MUX5A_SRC_PLL 0x0800U
/** Clock distribution MUX5A source: RX. */
#define XRFDC_CLK_DISTR_MUX5A_SRC_RX 0x0040U
/** Clock distribution: off. */
#define XRFDC_CLK_DISTR_OFF                                                                                            \
	(XRFDC_CLK_DISTR_MUX4A_SRC_INT | XRFDC_CLK_DISTR_MUX6_SRC_OFF | XRFDC_CLK_DISTR_MUX7_SRC_OFF |                 \
	 XRFDC_CLK_DISTR_MUX8_SRC_NTH | XRFDC_CLK_DISTR_MUX9_SRC_INT)
/** Clock distribution value for the leftmost tile. */
#define XRFDC_CLK_DISTR_LEFTMOST_TILE 0x0000U
/** Clock distribution value to continue left (even tile). */
#define XRFDC_CLK_DISTR_CONT_LEFT_EVEN 0x8208U
/** Clock distribution value to continue left (odd tile). */
#define XRFDC_CLK_DISTR_CONT_LEFT_ODD 0x8200U
/** Clock distribution value for the rightmost tile. */
#define XRFDC_CLK_DISTR_RIGHTMOST_TILE 0x4008
/** Clock distribution value to continue right (even tile). */
#define XRFDC_CLK_DISTR_CONT_RIGHT_EVEN 0x4080
/** Clock distribution value to continue right (odd tile). */
#define XRFDC_CLK_DISTR_CONT_RIGHT_HWL_ODD 0x4088

/** Clear mask of the clock distribution MUX4A source. */
#define XRFDC_CLK_DISTR_MUX4A_SRC_CLR 0x0008U
/** Clear mask of the clock distribution MUX6 source. */
#define XRFDC_CLK_DISTR_MUX6_SRC_CLR 0x0180U
/** Clear mask of the clock distribution MUX7 source. */
#define XRFDC_CLK_DISTR_MUX7_SRC_CLR 0x0600U
/** Clear mask of the clock distribution MUX8 source. */
#define XRFDC_CLK_DISTR_MUX8_SRC_CLR 0x8000U
/** Clear mask of the clock distribution MUX9 source. */
#define XRFDC_CLK_DISTR_MUX9_SRC_CLR 0x4000U

#define XRFDC_DIST_MAX 8 /**< Maximum number of clock distributions. */

/** Network control: receive the PLL clock. */
#define XRFDC_NET_CTRL_CLK_REC_PLL 0x1U
/** Network control: receive the distributed T1 clock. */
#define XRFDC_NET_CTRL_CLK_REC_DIST_T1 0x2U
/** Network control: T1 clock from the local source. */
#define XRFDC_NET_CTRL_CLK_T1_SRC_LOCAL 0x4U
/** Network control: T1 clock from the distribution. */
#define XRFDC_NET_CTRL_CLK_T1_SRC_DIST 0x8U
/** Network control: clock input from the distribution. */
#define XRFDC_NET_CTRL_CLK_INPUT_DIST 0x20U
/** Distribution control: send to the PLL divider. */
#define XRFDC_DIST_CTRL_TO_PLL_DIV 0x10U
#define XRFDC_DIST_CTRL_TO_T1 0x20U /**< Distribution control: send to T1. */
/** Distribution control: local distribution source. */
#define XRFDC_DIST_CTRL_DIST_SRC_LOCAL 0x40U
/** Distribution control: PLL distribution source. */
#define XRFDC_DIST_CTRL_DIST_SRC_PLL 0x800U
/** Distribution control: T1 clock from the local source. */
#define XRFDC_DIST_CTRL_CLK_T1_SRC_LOCAL 0x1000U
/** PLL reference divider input: off. */
#define XRFDC_PLLREFDIV_INPUT_OFF 0x20U
/** PLL reference divider input: distribution. */
#define XRFDC_PLLREFDIV_INPUT_DIST 0x40U
/** PLL reference divider input: fabric. */
#define XRFDC_PLLREFDIV_INPUT_FABRIC 0x60U
/** PLL output divider input: local distribution. */
#define XRFDC_PLLOPDIV_INPUT_DIST_LOCAL 0x800U

#define XRFDC_TILE_SOURCE_RX 0U /**< Tile clock source: RX. */
#define XRFDC_TILE_SOURCE_DIST 1U /**< Tile clock source: distribution. */
#define XRFDC_TILE_SOURCE_FABRIC 2U /**< Tile clock source: fabric. */

#define XRFDC_DIST_OUT_NONE 0U /**< Distribution output: none. */
#define XRFDC_DIST_OUT_RX 1U /**< Distribution output: RX clock. */
/** Distribution output: PLL output divider. */
#define XRFDC_DIST_OUT_OUTDIV 2U

#define XRFDC_PLL_SOURCE_NONE 0U /**< PLL source: none. */
#define XRFDC_PLL_SOURCE_RX 1U /**< PLL source: RX clock. */
#define XRFDC_PLL_SOURCE_OUTDIV 2U /**< PLL source: output divider. */

/** PLL output divider mode for divide by 1. */
#define XRFDC_PLL_OUTDIV_MODE_1 0x0U
/** PLL output divider mode for divide by 2. */
#define XRFDC_PLL_OUTDIV_MODE_2 0x1U
/** PLL output divider mode for divide by 3. */
#define XRFDC_PLL_OUTDIV_MODE_3 0x2U
/** PLL output divider mode for divide by N. */
#define XRFDC_PLL_OUTDIV_MODE_N 0x3U

/** PLL output divider value for divide by 3. */
#define XRFDC_PLL_OUTDIV_MODE_3_VAL 0x1U

#define XRFDC_DIVISION_FACTOR_MIN 1 /**< Minimum division factor. */

#define XRFDC_DITH_ENABLE 1 /**< Dither enabled. */
#define XRFDC_DITH_DISABLE 0 /**< Dither disabled. */

#define XRFDC_SIGDET_MODE_AVG 0 /**< Signal detector mode: average. */
#define XRFDC_SIGDET_MODE_RNDM 1 /**< Signal detector mode: randomized. */
#define XRFDC_SIGDET_TC_2_0 0 /**< Signal detector time constant of 2^0. */
#define XRFDC_SIGDET_TC_2_2 1 /**< Signal detector time constant of 2^2. */
#define XRFDC_SIGDET_TC_2_4 2 /**< Signal detector time constant of 2^4. */
#define XRFDC_SIGDET_TC_2_8 3 /**< Signal detector time constant of 2^8. */
#define XRFDC_SIGDET_TC_2_12 4 /**< Signal detector time constant of 2^12. */
#define XRFDC_SIGDET_TC_2_14 5 /**< Signal detector time constant of 2^14. */
#define XRFDC_SIGDET_TC_2_16 6 /**< Signal detector time constant of 2^16. */
#define XRFDC_SIGDET_TC_2_18 7 /**< Signal detector time constant of 2^18. */

#define XRFDC_DISABLED 0 /**< Disabled. */
#define XRFDC_ENABLED 1 /**< Enabled. */

#define XRFDC_ES1_SI 0U /**< ES1 silicon. */
#define XRFDC_PROD_SI 1U /**< Production silicon. */

#define XRFDC_CAL_BLOCK_OCB1 0 /**< Calibration block: OCB1. */
#define XRFDC_CAL_BLOCK_OCB2 1 /**< Calibration block: OCB2. */
#define XRFDC_CAL_BLOCK_GCB 2 /**< Calibration block: GCB. */
#define XRFDC_CAL_BLOCK_TSCB 3 /**< Calibration block: TSCB. */

/** TSCB tune value with auto-calibration. */
#define XRFDC_TSCB_TUNE_AUTOCAL 0x0550U
/** TSCB tune value without auto-calibration. */
#define XRFDC_TSCB_TUNE_NOT_AUTOCAL 0x0440U

#define XRFDC_INV_SYNC_MODE_MAX 2 /**< Maximum inverse-sinc filter mode. */

/** Maximum inverse-sinc filter enable value. */
#define XRFDC_INV_SYNC_EN_MAX 1

/** Mask of the export and premium control bits. */
#define XRFDC_CTRL_MASK 0x0440
/** Export control bit for clock distribution. */
#define XRFDC_EXPORTCTRL_CLKDIST 0x0400
/** Premium control bit for clock distribution. */
#define XRFDC_PREMIUMCTRL_CLKDIST 0x0040
#define XRFDC_EXPORTCTRL_VOP 0x2000 /**< Export control bit for VOP. */
#define XRFDC_EXPORTCTRL_DSA 0x0400 /**< Export control bit for DSA. */

/** DAC data path mode: DUC, 0 to Fs/2. */
#define XRFDC_DATAPATH_MODE_DUC_0_FSDIVTWO 1U
/** DAC data path mode: DUC, 0 to Fs/4. */
#define XRFDC_DATAPATH_MODE_DUC_0_FSDIVFOUR 2U
/** DAC data path mode: Fs/4 to Fs/2. */
#define XRFDC_DATAPATH_MODE_FSDIVFOUR_FSDIVTWO 3U
/** DAC data path mode: no DUC, 0 to Fs/2. */
#define XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO 4U
/** DAC interpolation mode: full bandwidth. */
#define XRFDC_DAC_INT_MODE_FULL_BW 0U
/** DAC interpolation mode: half bandwidth with IMR. */
#define XRFDC_DAC_INT_MODE_HALF_BW_IMR 2U
/** DAC interpolation mode: full bandwidth bypass. */
#define XRFDC_DAC_INT_MODE_FULL_BW_BYPASS 3U
/** Maximum DAC data path mode. */
#define XRFDC_DAC_MODE_MAX XRFDC_DATAPATH_MODE_NODUC_0_FSDIVTWO

#define XRFDC_FULL_BW_DIVISOR 1U /**< Divisor for full bandwidth. */
#define XRFDC_HALF_BW_DIVISOR 2U /**< Divisor for half bandwidth. */

#define XRFDC_DAC_IMR_MODE_LOWPASS 0U /**< DAC IMR mode: low pass. */
#define XRFDC_DAC_IMR_MODE_HIGHPASS 1U /**< DAC IMR mode: high pass. */
/** Maximum DAC IMR mode. */
#define XRFDC_DAC_IMR_MODE_MAX XRFDC_DAC_IMR_MODE_HIGHPASS

#define XRFDC_CLOCK_DETECT_CLK 0x1U /**< Clock detect: tile clock. */
#define XRFDC_CLOCK_DETECT_DIST 0x2U /**< Clock detect: distributed clock. */
#define XRFDC_CLOCK_DETECT_BOTH 0x3U /**< Clock detect: both clocks. */

#define XRFDC_CAL_UNFREEZE_CALIB 0U /**< Unfreezes the calibration. */
#define XRFDC_CAL_FREEZE_CALIB 1U /**< Freezes the calibration. */
#define XRFDC_CAL_FRZ_PIN_ENABLE 0U /**< Enables the calibration freeze pin. */
/** Disables the calibration freeze pin. */
#define XRFDC_CAL_FRZ_PIN_DISABLE 1U

#define XRFDC_CLK_REG_EN_MASK 0x2000U /**< Clock regulator enable mask. */

#define XRFDC_GEN1_LOW_I 20000U /**< Gen 1 low output current in uA. */
#define XRFDC_GEN1_HIGH_I 32000U /**< Gen 1 high output current in uA. */

/**
* @brief Returns the maximum DSA attenuation in dB.
*
* @param[in]   X Silicon revision.
*
* @return      11.0 if X is 0, otherwise 27.0.
*
* @note        C-style signature:
*              double XRFDC_MAX_ATTEN(u32 X)
*/
#define XRFDC_MAX_ATTEN(X) ((X == 0) ? 11.0 : 27.0)
#define XRFDC_MIN_ATTEN 0 /**< Minimum DSA attenuation in dB. */
/**
* @brief Returns the DSA attenuation step in dB.
*
* @param[in]   X Silicon revision.
*
* @return      0.5 if X is 0, otherwise 1.0.
*
* @note        C-style signature:
*              double XRFDC_STEP_ATTEN(u32 X)
*/
#define XRFDC_STEP_ATTEN(X) ((X == 0) ? 0.5 : 1.0)

/** DAC VOP control register update mask. */
#define XRFDC_DAC_VOP_CTRL_REG_UPDT_MASK 0x2U
/** DAC VOP control test build mask. */
#define XRFDC_DAC_VOP_CTRL_TST_BLD_MASK 0x1U
/** DAC VOP builder low bits mask. */
#define XRFDC_DAC_VOP_BLDR_LOW_BITS_MASK 0xFU

/** Delay count while waiting for the PLL lock. */
#define XRFDC_PLL_LOCK_DLY_CNT 1000U
/** Delay count while waiting for the restart bit to clear. */
#define XRFDC_RESTART_CLR_DLY_CNT 1000U
#define XRFDC_WAIT_ATTEMPTS_CNT 10000U /**< Number of wait attempts. */
#define XRFDC_STATE_WAIT 100U /**< Wait time for a state change. */
/** Wait time for the restart bit to clear. */
#define XRFDC_RESTART_CLR_WAIT 1000U
#define XRFDC_PLL_LOCK_WAIT 1000U /**< Wait time for the PLL lock. */

/** Data path clock divider value for the first mode. */
#define XRFDC_CLK_DIV_DP_FIRST_MODE 0x10U
/** Data path clock divider value for the other modes. */
#define XRFDC_CLK_DIV_DP_OTHER_MODES 0x20U

#define XRFDC_TILE_STARTED XRFDC_SM_STATE15 /**< State of a started tile. */

#define XRFDC_SI_REV_ES 0U /**< Silicon revision: ES. */
#define XRFDC_SI_REV_PROD 1U /**< Silicon revision: production. */

#define XRFDC_CG_WAIT_CYCLES 3U /**< Clock gating wait cycles. */
#define XRFDC_ADC_CG_WAIT_CYCLES 1U /**< ADC clock gating wait cycles. */

/** Total clock gating cycles for the X1, X2, X4, X8 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X1_X2_X4_X8 0U
/** Clock gating cycles kept for the X1, X2, X4, X8 factors. */
#define XRFDC_CG_CYCLES_KEPT_X1_X2_X4_X8 1U
/** Total clock gating cycles for the X3, X6, X12 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X3_X6_X12 3U
/** Clock gating cycles kept for the X3, X6, X12 factors. */
#define XRFDC_CG_CYCLES_KEPT_X3_X6_X12 2U
/** Total clock gating cycles for the X5, X10 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X5_X10 5U
/** Clock gating cycles kept for the X5, X10 factors. */
#define XRFDC_CG_CYCLES_KEPT_X5_X10 4U
/** Total clock gating cycles for the X16 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X16 2U
/** Clock gating cycles kept for the X16 factors. */
#define XRFDC_CG_CYCLES_KEPT_X16 1U
/** Total clock gating cycles for the X20 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X20 5U
/** Clock gating cycles kept for the X20 factors. */
#define XRFDC_CG_CYCLES_KEPT_X20 2U
/** Total clock gating cycles for the X24 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X24 3U
/** Clock gating cycles kept for the X24 factors. */
#define XRFDC_CG_CYCLES_KEPT_X24 1U
/** Total clock gating cycles for the X40 factors. */
#define XRFDC_CG_CYCLES_TOTAL_X40 5U
/** Clock gating cycles kept for the X40 factors. */
#define XRFDC_CG_CYCLES_KEPT_X40 1U

#define XRFDC_CG_FIXED_OFS 2U /**< Clock gating fixed offset. */

#define XRFDC_FIFO_CHANNEL_ACT 0U /**< FIFO channel: actual. */
#define XRFDC_FIFO_CHANNEL_OBS 1U /**< FIFO channel: observation. */
#define XRFDC_FIFO_CHANNEL_BOTH 2U /**< FIFO channel: actual and observation. */

#define XRFDC_PWR_MODE_OFF 0U /**< Power mode: off. */
#define XRFDC_PWR_MODE_ON 1U /**< Power mode: on. */

#define XRFDC_DUAL_TILE 2U /**< Dual tile ADC type. */
#define XRFDC_QUAD_TILE 4U /**< Quad tile ADC type. */

#define XRFDC_4ADC_4DAC_TILES 0U /**< Tile layout: 4 ADC and 4 DAC tiles. */
#define XRFDC_3ADC_2DAC_TILES 1U /**< Tile layout: 3 ADC and 2 DAC tiles. */

#define XRFDC_MTS_SYSREF_DISABLE 0U /**< Disables the MTS SYSREF. */
#define XRFDC_MTS_SYSREF_ENABLE 1U /**< Enables the MTS SYSREF. */

#define XRFDC_MTS_SCAN_INIT 0U /**< MTS scan: initial. */
#define XRFDC_MTS_SCAN_RELOAD 1U /**< MTS scan: reload. */

/* MTS Error Codes */
#define XRFDC_MTS_OK 0U /**< MTS status: success. */
#define XRFDC_MTS_NOT_SUPPORTED 1U /**< MTS error: not supported. */
#define XRFDC_MTS_TIMEOUT 2U /**< MTS error: timeout. */
#define XRFDC_MTS_MARKER_RUN 4U /**< MTS error: marker run. */
#define XRFDC_MTS_MARKER_MISM 8U /**< MTS error: marker mismatch. */
#define XRFDC_MTS_DELAY_OVER 16U /**< MTS error: delay overflow. */
#define XRFDC_MTS_TARGET_LOW 32U /**< MTS error: target too low. */
#define XRFDC_MTS_IP_NOT_READY 64U /**< MTS error: IP not ready. */
#define XRFDC_MTS_DTC_INVALID 128U /**< MTS error: invalid DTC. */
#define XRFDC_MTS_NOT_ENABLED 512U /**< MTS error: not enabled. */
#define XRFDC_MTS_SYSREF_GATE_ERROR 2048U /**< MTS error: SYSREF gate error. */
/** MTS error: SYSREF frequency not done. */
#define XRFDC_MTS_SYSREF_FREQ_NDONE 4096U
#define XRFDC_MTS_BAD_REF_TILE 8192U /**< MTS error: bad reference tile. */

/** AXI clock multiplier used in the calibration. */
#define XRFDC_CAL_AXICLK_MULT 12.17085774
/**
* @brief Returns the calibration divider cutoff frequency.
*
* @param[in]   X Nonzero for a high-speed ADC.
*
* @return      5.0 if X is nonzero, otherwise 2.5.
*
* @note        C-style signature:
*              double XRFDC_CAL_DIV_CUTOFF_FREQ(u32 X)
*/
#define XRFDC_CAL_DIV_CUTOFF_FREQ(X) (X ? 5.0 : 2.5)

/*****************************************************************************/
/************************** Function Prototypes ******************************/

#ifndef SDT
XRFdc_Config *XRFdc_LookupConfig(u16 DeviceId);
#else
/**
 * @brief Looks up the device configuration based on the base address.
 *
 * @param[in]   BaseAddr Device base address to look up the configuration for.
 *
 * @return      Pointer to the configuration found, or NULL if not found.
 */
XRFdc_Config *XRFdc_LookupConfig(metal_phys_addr_t BaseAddr);
#endif
u32 XRFdc_RegisterMetal(XRFdc *InstancePtr, u16 DeviceId, struct metal_device **DevicePtr);
u32 XRFdc_CfgInitialize(XRFdc *InstancePtr, XRFdc_Config *ConfigPtr);
u32 XRFdc_StartUp(XRFdc *InstancePtr, u32 Type, int Tile_Id);
u32 XRFdc_Shutdown(XRFdc *InstancePtr, u32 Type, int Tile_Id);
u32 XRFdc_Reset(XRFdc *InstancePtr, u32 Type, int Tile_Id);
u32 XRFdc_CustomStartUp(XRFdc *InstancePtr, u32 Type, int Tile_Id, u32 StartState, u32 EndState);
u32 XRFdc_WaitForState(XRFdc *InstancePtr, u32 Type, u32 Tile, u32 State);
u32 XRFdc_GetIPStatus(XRFdc *InstancePtr, XRFdc_IPStatus *IPStatusPtr);
u32 XRFdc_GetBlockStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_BlockStatus *BlockStatusPtr);
u32 XRFdc_SetMixerSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
			   XRFdc_Mixer_Settings *MixerSettingsPtr);
u32 XRFdc_GetMixerSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
			   XRFdc_Mixer_Settings *MixerSettingsPtr);
u32 XRFdc_SetQMCSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_QMC_Settings *QMCSettingsPtr);
u32 XRFdc_GetQMCSettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_QMC_Settings *QMCSettingsPtr);
u32 XRFdc_GetCoarseDelaySettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_CoarseDelay_Settings *CoarseDelaySettingsPtr);
u32 XRFdc_SetCoarseDelaySettings(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
				 XRFdc_CoarseDelay_Settings *CoarseDelaySettingsPtr);
u32 XRFdc_GetInterpolationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *InterpolationFactorPtr);
u32 XRFdc_GetDecimationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecimationFactorPtr);
u32 XRFdc_GetDecimationFactorObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecimationFactorPtr);
u32 XRFdc_GetFabWrVldWords(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricDataRatePtr);
u32 XRFdc_GetFabWrVldWordsObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricDataRatePtr);
u32 XRFdc_GetFabRdVldWords(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricDataRatePtr);
u32 XRFdc_GetFabRdVldWordsObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *FabricDataRatePtr);
u32 XRFdc_SetFabRdVldWords(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricRdVldWords);
u32 XRFdc_SetFabRdVldWordsObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricRdVldWords);
u32 XRFdc_SetFabWrVldWords(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 FabricWrVldWords);
u32 XRFdc_GetThresholdSettings(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
			       XRFdc_Threshold_Settings *ThresholdSettingsPtr);
u32 XRFdc_SetThresholdSettings(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id,
			       XRFdc_Threshold_Settings *ThresholdSettingsPtr);
u32 XRFdc_SetDecoderMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecoderMode);
u32 XRFdc_UpdateEvent(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 Event);
u32 XRFdc_GetDecoderMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *DecoderModePtr);
u32 XRFdc_ResetNCOPhase(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
void XRFdc_DumpRegs(XRFdc *InstancePtr, u32 Type, int Tile_Id);
u32 XRFdc_MultiBand(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 DigitalDataPathMask, u32 MixerInOutDataType,
		    u32 DataConverterMask);
int XRFdc_GetConnectedIData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
int XRFdc_GetConnectedQData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetConnectedIQData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, int *ConnectedIData,
			     int *ConnectedQData);
u32 XRFdc_IntrHandler(u32 Vector, void *XRFdcPtr);
u32 XRFdc_IntrClr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask);
u32 XRFdc_GetIntrStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *IntrStsPtr);
u32 XRFdc_IntrDisable(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask);
u32 XRFdc_IntrEnable(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 IntrMask);
u32 XRFdc_GetEnabledInterrupts(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *IntrMask);
u32 XRFdc_SetThresholdClrMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 ThresholdToUpdate, u32 ClrMode);
u32 XRFdc_ThresholdStickyClear(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 ThresholdToUpdate);
void XRFdc_SetStatusHandler(XRFdc *InstancePtr, void *CallBackRef, XRFdc_StatusHandler FunctionPtr);
u32 XRFdc_SetupFIFO(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable);
u32 XRFdc_SetupFIFOObs(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable);
u32 XRFdc_SetupFIFOBoth(XRFdc *InstancePtr, u32 Type, int Tile_Id, u8 Enable);
u32 XRFdc_GetFIFOStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 *EnablePtr);
u32 XRFdc_GetFIFOStatusObs(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 *EnablePtr);
u32 XRFdc_SetNyquistZone(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 NyquistZone);
u32 XRFdc_GetNyquistZone(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *NyquistZonePtr);
u32 XRFdc_GetOutputCurr(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *OutputCurrPtr);
u32 XRFdc_SetDecimationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecimationFactor);
u32 XRFdc_SetDecimationFactorObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 DecimationFactor);
u32 XRFdc_SetInterpolationFactor(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 InterpolationFactor);
u32 XRFdc_SetFabClkOutDiv(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u16 FabClkDiv);
u32 XRFdc_SetCalibrationMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u8 CalibrationMode);
u32 XRFdc_GetCalibrationMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u8 *CalibrationModePtr);
u32 XRFdc_GetClockSource(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 *ClockSourcePtr);
u32 XRFdc_GetPLLLockStatus(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 *LockStatusPtr);
u32 XRFdc_GetPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, XRFdc_PLL_Settings *PLLSettings);
u32 XRFdc_DynamicPLLConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u8 Source, double RefClkFreq,
			   double SamplingRate);
u32 XRFdc_SetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u16 Mode);
u32 XRFdc_GetInvSincFIR(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u16 *ModePtr);

u32 XRFdc_GetCoupling(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, u32 *ModePtr);
u32 XRFdc_GetLinkCoupling(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr);
u32 XRFdc_GetFabClkOutDiv(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u16 *FabClkDivPtr);
u32 XRFdc_SetDither(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode);
u32 XRFdc_GetDither(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr);
u32 XRFdc_SetClkDistribution(XRFdc *InstancePtr, XRFdc_Distribution_Settings *DistributionSettingsPtr);
u32 XRFdc_GetClkDistribution(XRFdc *InstancePtr, XRFdc_Distribution_System_Settings *DistributionArrayPtr);
u32 XRFdc_SetDataPathMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode);
u32 XRFdc_GetDataPathMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr);
u32 XRFdc_SetIMRPassMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Mode);
u32 XRFdc_GetIMRPassMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *ModePtr);
u32 XRFdc_SetSignalDetector(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Signal_Detector_Settings *SettingsPtr);
u32 XRFdc_GetSignalDetector(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Signal_Detector_Settings *SettingsPtr);
u32 XRFdc_DisableCoefficientsOverride(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock);
u32 XRFdc_SetCalCoefficients(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock,
			     XRFdc_Calibration_Coefficients *CoeffPtr);
u32 XRFdc_GetCalCoefficients(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 CalibrationBlock,
			     XRFdc_Calibration_Coefficients *CoeffPtr);
u32 XRFdc_SetCalFreeze(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Cal_Freeze_Settings *CalFreezePtr);
u32 XRFdc_GetCalFreeze(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_Cal_Freeze_Settings *CalFreezePtr);
u32 XRFdc_SetDACVOP(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 uACurrent);
u32 XRFdc_SetDACCompMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Enable);
u32 XRFdc_GetDACCompMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *EnabledPtr);
u32 XRFdc_SetDSA(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_DSA_Settings *SettingsPtr);
u32 XRFdc_GetDSA(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, XRFdc_DSA_Settings *SettingsPtr);
u32 XRFdc_SetPwrMode(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_Pwr_Mode_Settings *SettingsPtr);
u32 XRFdc_GetPwrMode(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, XRFdc_Pwr_Mode_Settings *SettingsPtr);
u32 XRFdc_ResetInternalFIFOWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_ResetInternalFIFOWidthObs(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
void XRFdc_ClrSetReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask, u16 Data);
void XRFdc_ClrReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask);
u16 XRFdc_RDReg(XRFdc *InstancePtr, u32 BaseAddr, u32 RegAddr, u16 Mask);
u32 XRFdc_IsHighSpeedADC(XRFdc *InstancePtr, u32 Tile);
u32 XRFdc_IsDACBlockEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_IsADCBlockEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_IsDACDigitalPathEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_IsADCDigitalPathEnabled(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_CheckDigitalPathEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_Get_IPBaseAddr(XRFdc *InstancePtr);
u32 XRFdc_Get_TileBaseAddr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);
u32 XRFdc_Get_BlockBaseAddr(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetNoOfDACBlock(XRFdc *InstancePtr, u32 Tile_Id);
u32 XRFdc_GetNoOfADCBlocks(XRFdc *InstancePtr, u32 Tile_Id);
u32 XRFdc_GetDataType(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetDataWidth(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetInverseSincFilter(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetMixedMode(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_GetMasterTile(XRFdc *InstancePtr, u32 Type);
u32 XRFdc_GetSysRefSource(XRFdc *InstancePtr, u32 Type);
double XRFdc_GetFabClkFreq(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);
u32 XRFdc_IsFifoEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
void XRFdc_SetConnectedIQData(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id, int ConnectedIData,
			      int ConnectedQData);
u32 XRFdc_GetMultibandConfig(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);
u32 XRFdc_CheckBlockEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id);
u32 XRFdc_CheckTileEnabled(XRFdc *InstancePtr, u32 Type, u32 Tile_Id);
u32 XRFdc_GetMaxSampleRate(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double *MaxSampleRatePtr);
u32 XRFdc_GetMinSampleRate(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, double *MinSampleRatePtr);
double XRFdc_GetDriverVersion(void);
u32 XRFdc_MultiConverter_Sync(XRFdc *InstancePtr, u32 Type, XRFdc_MultiConverter_Sync_Config *ConfigPtr);
u32 XRFdc_MultiConverter_Init(XRFdc_MultiConverter_Sync_Config *ConfigPtr, int *PLL_CodesPtr, int *T1_CodesPtr,
			      u32 RefTile);
u32 XRFdc_MTS_Sysref_Config(XRFdc *InstancePtr, XRFdc_MultiConverter_Sync_Config *DACSyncConfigPtr,
			    XRFdc_MultiConverter_Sync_Config *ADCSyncConfigPtr, u32 SysRefEnable);
u32 XRFdc_GetMTSEnable(XRFdc *InstancePtr, u32 Type, u32 Tile, u32 *EnablePtr);
u32 XRFdc_SetDACDataScaler(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 Enable);
u32 XRFdc_GetDACDataScaler(XRFdc *InstancePtr, u32 Tile_Id, u32 Block_Id, u32 *EnablePtr);
u8 XRFdc_GetTileLayout(XRFdc *InstancePtr);
#ifndef __BAREMETAL__
s32 XRFdc_GetDeviceNameByDeviceId(char *DevNamePtr, u16 DevId);
#endif

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* RFDC_H_ */
