/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XSYSMONPSU_H_
#define XSYSMONPSU_H_

/*****************************************************************************/
/**
* @file xsysmonpsu.h
* @addtogroup Overview
* @{
*
* @brief Defines the driver API for the Xilinx System Monitor device.
*
* The XSysMon driver supports the Xilinx System Monitor device.
*
* The System Monitor device has the following features:
*	- PL Sysmon instance has 10-bit, 200-KSPS (kilo samples per second)
*		Analog-to-Digital Converter (ADC)
*	- PS Sysmon instance has 10-bit, 1000-KSPS ADC.
*	- Monitoring of on-chip supply voltages and temperature
*	- 1 dedicated differential analog-input pair and
*	  16 auxiliary differential analog-input pairs
*	- Automatic alarms based on user defined limits for the on-chip
*	  supply voltages and temperature
*	- Automatic Channel Sequencer, programmable averaging, programmable
*	  acquisition time for the external inputs, unipolar or differential
*	  input selection for the external inputs
*	- Inbuilt Calibration
*	- Optional interrupt request generation
*	- External Mux
*
* The user should refer to the hardware device specification for detailed
* information about the device.
*
* This header file contains the prototypes of driver functions that can
* be used to access the System Monitor device.
*
* <b> System Monitor Channel Sequencer Modes </b>
*
* The  System Monitor Channel Sequencer supports the following operating modes:
*
*   - <b> Default </b>: This is the default mode after power up.
*		In this mode of operation the System Monitor operates in
*		a sequence mode, monitoring the on chip sensors:
*		Temperature, VCCINT, and VCCAUX.
*   - <b> One pass through sequence </b>: In this mode the System Monitor
*		converts the channels enabled in the Sequencer Channel Enable
*		registers for a single pass and then stops.
*   - <b> Continuous cycling of sequence </b>: In this mode the System Monitor
*		converts the channels enabled in the Sequencer Channel Enable
*		registers continuously.
*   - <b> Single channel mode</b>: In this mode the System Monitor Channel
*		Sequencer is disabled and the System Monitor operates in a
*		Single Channel Mode.
*		The System Monitor can operate either in a Continuous or Event
*		driven sampling mode in the single channel mode.
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (for example, an
* application) to communicate to the System Monitor device.
*
* XSysMonPsu_CfgInitialize() API is used to initialize the System Monitor
* device. The user needs to first call the XSysMonPsu_LookupConfig() API which
* returns the Configuration structure pointer which is passed as a parameter to
* the XSysMonPsu_CfgInitialize() API.
*
* <b>Interrupts</b>
*
* The System Monitor device supports interrupt driven mode and the default
* operation mode is polling mode.
*
* This driver does not provide a Interrupt Service Routine (ISR) for the device.
* It is the responsibility of the application to provide one if needed. Refer to
* the interrupt example provided with this driver for details on using the
* device in interrupt mode.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XSysMonPsu driver is composed of several source files. This allows the
* user to build and link only those parts of the driver that are necessary.
*
* <b> Limitations of the driver </b>
*
* System Monitor device can be accessed through the JTAG port and the AXI
* interface. The driver implementation does not support the simultaneous access
* of the device by both these interfaces. The user has to take care of this
* situation in the user application code.
*
* <br><br>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    12/15/15 First release
*              02/15/16 Corrected Assert function call in
*                       XSysMonPsu_GetMonitorStatus API.
*              03/03/16 Added Temperature remote channel for Setsingle
*                       channel API. Also corrected external mux channel
*                       numbers.
* 1.1   kvn    05/05/16 Modified code for MISRA-C:2012 Compliance.
* 2.0   vns    08/14/16 Fixed CR #956780, added support for enabling/disabling
*                       SEQ_CH2 and SEQ_AVG2 registers, modified function
*                       prototypes of XSysMonPsu_GetSeqAvgEnables,
*                       XSysMonPsu_SetSeqAvgEnables, XSysMonPsu_SetSeqChEnables,
*                       XSysMonPsu_GetSeqChEnables,
*                       XSysMonPsu_SetSeqInputMode, XSysMonPsu_GetSeqInputMode,
*                       XSysMonPsu_SetSeqAcqTime
*                       and XSysMonPsu_GetSeqAcqTime to provide support for
*                       set/get 64 bit value.
*                       Added constants XSM_CFR_ALM_SUPPLY*(8-31)_MASKs to
*                       provide support for enabling extra PS alarams.
* 2.1   sk     03/03/16 Check for PL reset before doing PL Sysmon reset.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
*       ms     04/05/17 Modified Comment lines in functions of sysmonpsu
*                       examples to recognize it as documentation block
*                       for doxygen generation.
* 2.2   sk     04/14/17 Corrected temperature conversion formulas.
* 2.3   mn     12/11/17 Added missing closing bracket error when C++ is used
*       mn     12/12/17 Added Conversion Support for voltages having Range of
*                       1 Volt
*       mn     12/13/17 Correct the AMS block channel numbers
*       ms     12/15/17 Added peripheral test support.
*       ms     01/04/18 Provided conditional checks for interrupt example
*                       in sysmonpsu_header.h
*       mn     03/08/18 Update Clock Divisor to the proper value
* 2.4   mn     04/20/18 Remove looping check for PL accessible bit
* 2.6   aad    11/18/19 Fixed typo in alarm macro comments
* 2.7   aad    10/21/20 Modified code for MISRA-C:2012 Compliance.
*       aad    17/12/20 Removed undefined function.
* 2.9   cog    07/20/23 Added support for SDT flow.
* 3.0   cog    03/20/24 Refactored XSysMonPsu_UpdateAdcClkDivisor for more
*                       robust error handling, this required the API prototype
*                       to be changed.
*       se     07/30/24 OT Upper Alarm threshold requires 12-bit value update
*                       to enable over-temperature default value override.
* 3.3   dc     02/18/26 Correct spelling errors
* 3.4   vn     07/30/26 Fix Doxygen warnings
* 3.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xsysmonpsu_hw.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**
 * @name Indexes for the different channels.
 * @{
 */
#define XSM_CH_TEMP 		0x0U  /**< On Chip Temperature */
#define XSM_CH_SUPPLY1		0x1U  /**< SUPPLY1 VCC_PSINTLP */
#define XSM_CH_SUPPLY2		0x2U  /**< SUPPLY2 VCC_PSINTFP */
#define XSM_CH_VPVN 		0x3U  /**< VP/VN Dedicated analog inputs */
#define XSM_CH_VREFP		0x4U  /**< VREFP */
#define XSM_CH_VREFN		0x5U  /**< VREFN */
#define XSM_CH_SUPPLY3		0x6U  /**< SUPPLY3 VCC_PSAUX */
#define XSM_CH_SUPPLY_CALIB	0x08U /**< Supply Calib Data Reg */
#define XSM_CH_ADC_CALIB	0x09U /**< ADC Offset Channel Reg */
#define XSM_CH_GAINERR_CALIB 	0x0AU /**< Gain Error Channel Reg  */
#define XSM_CH_SUPPLY4		0x0DU /**< SUPPLY4 VCC_PSDDR_504 */
#define XSM_CH_SUPPLY5		0x0EU /**< SUPPLY5 VCC_PSIO3_503 */
#define XSM_CH_SUPPLY6		0x0FU /**< SUPPLY6 VCC_PSIO0_500 */
#define XSM_CH_AUX_MIN		16U   /**< Channel number for 1st Aux Channel */
/** Channel number for Last Aux channel */
#define XSM_CH_AUX_MAX		31U
#define XSM_CH_SUPPLY7      32U   /**< SUPPLY7 VCC_PSIO1_501 */
#define XSM_CH_SUPPLY8      33U   /**< SUPPLY8 VCC_PSIO2_502 */
#define XSM_CH_SUPPLY9      34U   /**< SUPPLY9 PS_MGTRAVCC */
#define XSM_CH_SUPPLY10     35U   /**< SUPPLY10 PS_MGTRAVTT */
#define XSM_CH_VCCAMS       36U   /**< VCCAMS */
#define XSM_CH_TEMP_REMTE   37U   /**< Temperature Remote */
#define XSM_CH_VCC_PSLL0    48U   /**< VCC_PSLL0 */
#define XSM_CH_VCC_PSLL3    51U   /**< VCC_PSLL3 */
#define XSM_CH_VCCINT       54U   /**< VCCINT */
#define XSM_CH_VCCBRAM      55U   /**< VCCBRAM */
#define XSM_CH_VCCAUX       56U   /**< VCCAUX */
#define XSM_CH_VCC_PSDDRPLL 57U   /**< VCC_PSDDRPLL */
#define XSM_CH_DDRPHY_VREF  58U   /**< DDRPHY_VREF */
#define XSM_CH_RESERVE1     63U   /**< PSGT_AT0 */

/** @} */

/**
 * @name Indexes for reading the Calibration Coefficient Data.
 * @{
 */
#define XSM_CALIB_SUPPLY_OFFSET_COEFF 0U /**< Supply Offset Calib Coefficient */
#define XSM_CALIB_ADC_OFFSET_COEFF    1U /**< ADC Offset Calib Coefficient */
#define XSM_CALIB_GAIN_ERROR_COEFF    2U /**< Gain Error Calib Coefficient*/

/** @} */

/**
 * @name Indexes for reading the Minimum/Maximum Measurement Data.
 * @{
 */
#define XSM_MAX_TEMP		0U    /**< Maximum Temperature Data */
#define XSM_MAX_SUPPLY1		1U    /**< Maximum SUPPLY1 Data */
#define XSM_MAX_SUPPLY2		2U    /**< Maximum SUPPLY2 Data */
#define XSM_MAX_SUPPLY3		3U    /**< Maximum SUPPLY3 Data */
#define XSM_MIN_TEMP		4U    /**< Minimum Temperature Data */
#define XSM_MIN_SUPPLY1		5U    /**< Minimum SUPPLY1 Data */
#define XSM_MIN_SUPPLY2     6U    /**< Minimum SUPPLY2 Data */
#define XSM_MIN_SUPPLY3     7U    /**< Minimum SUPPLY3 Data */
#define XSM_MAX_SUPPLY4		8U    /**< Maximum SUPPLY4 Data */
#define XSM_MAX_SUPPLY5		9U    /**< Maximum SUPPLY5 Data */
#define XSM_MAX_SUPPLY6		0xAU  /**< Maximum SUPPLY6 Data */
#define XSM_MIN_SUPPLY4     0xCU  /**< Minimum SUPPLY4 Data */
#define XSM_MIN_SUPPLY5     0xDU  /**< Minimum SUPPLY5 Data */
#define XSM_MIN_SUPPLY6     0xEU  /**< Minimum SUPPLY6 Data */
#define XSM_MAX_SUPPLY7		0x80U  /**< Maximum SUPPLY7 Data */
#define XSM_MAX_SUPPLY8		0x81U  /**< Maximum SUPPLY8 Data */
#define XSM_MAX_SUPPLY9		0x82U  /**< Maximum SUPPLY9 Data */
#define XSM_MAX_SUPPLY10	0x83U  /**< Maximum SUPPLY10 Data */
#define XSM_MAX_VCCAMS  	0x84U  /**< Maximum VCCAMS Data */
#define XSM_MAX_TEMP_REMOTE  	0x85U  /**< Maximum Remote Temperature Data */
#define XSM_MIN_SUPPLY7     0x88U  /**< Minimum SUPPLY7 Data */
#define XSM_MIN_SUPPLY8     0x89U  /**< Minimum SUPPLY8 Data */
#define XSM_MIN_SUPPLY9     0x8AU  /**< Minimum SUPPLY9 Data */
#define XSM_MIN_SUPPLY10    0x8BU  /**< Minimum SUPPLY10 Data */
#define XSM_MIN_VCCAMS      0x8CU  /**< Minimum VCCAMS Data */
#define XSM_MIN_TEMP_REMOTE     0x8DU  /**< Minimum Remote Temperature Data */

/** @} */

/**
 * @name Averaging to be done for the channels.
 * @{
 */
#define XSM_AVG_0_SAMPLES	0U /**< No Averaging */
#define XSM_AVG_16_SAMPLES	1U /**< Average 16 samples */
#define XSM_AVG_64_SAMPLES	2U /**< Average 64 samples */
#define XSM_AVG_256_SAMPLES	3U /**< Average 256 samples */

/** @} */

/**
 * @name Channel Sequencer Modes of operation.
 * @{
 */
#define XSM_SEQ_MODE_SAFE	 0U /**< Default Safe Mode */
#define XSM_SEQ_MODE_ONEPASS	 1U /**< Onepass through Sequencer */
#define XSM_SEQ_MODE_CONTINPASS	 2U /**< Continuous Cycling Seqquencer */
#define XSM_SEQ_MODE_SINGCHAN	 3U /**< Single channel - No Sequencing */
#define XSM_SEQ_MODE_OYLMPUS	 6U /**< Olympus mode */

/** @} */

/**
 * @name Clock Divisor values range.
 * @{
 */
#define XSM_CLK_DIV_MIN	 0U /**< Minimum Clock Divisor value */
#define XSM_CLK_DIV_MAX	 255U /**< Maximum Clock Divisor value */

/** @} */

/**
 * @name Power down modes.
 * @{
 */
#define XSM_PWR_MODE_NORMAL	 	0U /**< Normal operation */
#define XSM_PWR_MODE_PARTIAL_SLEEP	1U /**< Partial sleep mode */
#define XSM_PWR_MODE_SLEEP		3U /**< Full sleep mode */

/** @} */

/**
 * @name Alarm Threshold(Limit) Register (ATR) indexes.
 * @{
 */
#define XSM_ATR_TEMP_UPPER	 0U   /**< High user Temperature limit */
#define XSM_ATR_SUP1_UPPER	 1U   /**< Supply1 high voltage limit */
#define XSM_ATR_SUP2_UPPER	 2U   /**< Supply2 high voltage limit */
#define XSM_ATR_OT_UPPER	 3U   /**< Upper Over Temperature limit */
#define XSM_ATR_TEMP_LOWER	 4U   /**< Low user Temperature */
#define XSM_ATR_SUP1_LOWER	 5U   /**< Suuply1 low voltage limit */
#define XSM_ATR_SUP2_LOWER	 6U   /**< Supply2 low voltage limit */
#define XSM_ATR_OT_LOWER	 7U   /**< Lower Over Temperature limit */
#define XSM_ATR_SUP3_UPPER	 8U   /**< Supply3 high voltage limit */
#define XSM_ATR_SUP4_UPPER 	 9U   /**< Supply4 high voltage limit */
#define XSM_ATR_SUP5_UPPER 	 0xAU /**< Supply5 high voltage limit */
#define XSM_ATR_SUP6_UPPER 	 0xBU /**< Supply6 high voltage limit */
#define XSM_ATR_SUP3_LOWER	 0xCU /**< Supply3 low voltage limit */
#define XSM_ATR_SUP4_LOWER 	 0xDU /**< Supply4 low voltage limit */
#define XSM_ATR_SUP5_LOWER 	 0xEU /**< Supply5 low voltage limit */
#define XSM_ATR_SUP6_LOWER 	 0xFU /**< Supply6 low voltage limit */
#define XSM_ATR_SUP7_UPPER	 0x10U /**< Supply7 high voltage limit */
#define XSM_ATR_SUP8_UPPER	 0x11U /**< Supply8 high voltage limit */
#define XSM_ATR_SUP9_UPPER	 0x12U /**< Supply9 high voltage limit */
#define XSM_ATR_SUP10_UPPER	 0x13U /**< Supply10 high voltage limit */
#define XSM_ATR_VCCAMS_UPPER	 0x14U /**< VCCAMS high voltage limit */
#define XSM_ATR_TEMP_RMTE_UPPER	 0x15U /**< High remote Temperature limit */
#define XSM_ATR_SUP7_LOWER	 0x18U /**< Supply7 low voltage limit */
#define XSM_ATR_SUP8_LOWER	 0x19U /**< Supply8 low voltage limit */
#define XSM_ATR_SUP9_LOWER	 0x1AU /**< Supply9 low voltage limit */
#define XSM_ATR_SUP10_LOWER	 0x1BU /**< Supply10 low voltage limit */
#define XSM_ATR_VCCAMS_LOWER	 0x1CU /**< VCCAMS low voltage limit */
#define XSM_ATR_TEMP_RMTE_LOWER	 0x1DU /**< Low remote Temperature limit */

/** @} */

/**
 * @name Alarm masks for channels in Configuration registers 1
 * @{
 */
/** Alarm 13 - FPD Temperature */
#define XSM_CFR_ALM_SUPPLY13_MASK	0x200000
/** Alarm 12 - VCCAMS/VCCADC */
#define XSM_CFR_ALM_SUPPLY12_MASK	0x100000
#define XSM_CFR_ALM_SUPPLY11_MASK	0x080000 /**< Alarm 11 - SUPPLY10 */
#define XSM_CFR_ALM_SUPPLY10_MASK	0x040000 /**< Alarm 10 - SUPPLY9 */
#define XSM_CFR_ALM_SUPPLY9_MASK	0x020000 /**< Alarm 9 - SUPPLY8 */
#define XSM_CFR_ALM_SUPPLY8_MASK	0x010000 /**< Alarm 8 - SUPPLY7 */
#define XSM_CFR_ALM_SUPPLY6_MASK	0x0800 /**< Alarm 6 - SUPPLY6 */
#define XSM_CFR_ALM_SUPPLY5_MASK	0x0400 /**< Alarm 5 - SUPPLY5 */
#define XSM_CFR_ALM_SUPPLY4_MASK	0x0200 /**< Alarm 4 - SUPPLY4 */
#define XSM_CFR_ALM_SUPPLY3_MASK	0x0100 /**< Alarm 3 - SUPPLY3 */
#define XSM_CFR_ALM_SUPPLY2_MASK	0x0008 /**< Alarm 2 - SUPPLY2 */
#define XSM_CFR_ALM_SUPPLY1_MASK	0x0004 /**< Alarm 1 - SUPPLY1 */
#define XSM_CFR_ALM_TEMP_MASK		0x0002 /**< Alarm 0 - Temperature */
#define XSM_CFR_ALM_OT_MASK		0x0001 /**< Over Temperature Alarm */

/** @} */

/** @name Miscellaneous constants
 * @{
 */
#define XSM_POLL_TIMEOUT	0XFFFFFFFFU /**< Poll timeout count */

/** @} */

/**************************** Type Definitions *******************************/

/******************************************************************************/
/**
 * @brief Defines the callback handler for System Monitor interrupt events.
 *
 * An application defines this handler to communicate with the interrupt system
 * and retrieve state information about an application.
 *
 * @param[in]   CallBackRef Callback reference passed in by the upper layer
 *              when setting the handler, and passed back to the upper layer
 *              when the handler is called. It is used to find the device driver
 *              instance.
 *
 ******************************************************************************/
typedef void (*XSysMonPsu_Handler) (void *CallBackRef);

/**
 * @brief Holds configuration information for a device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device */
#else
	char *Name;		/**< Compatible device name */
#endif
	UINTPTR BaseAddress;	/**< Register base address */
	u16 InputClockMHz;	/**< Input clock frequency */
} XSysMonPsu_Config;

/**
 * @brief Holds the XSysMonPsu driver instance data.
 *
 * The user must allocate a variable of this type for the SYSMON device in the
 * system. A pointer to a variable of this type is then passed to the driver
 * API functions.
 */
typedef struct {
	XSysMonPsu_Config Config;	/**< Device configuration */
	u32 IsReady;			/**< Device is initialized and ready */
	XSysMonPsu_Handler Handler;	/**< Event handler callback */
	void *CallBackRef;		/**< Callback reference for handler */
	u32 IsPlAccessibleByPs;		/**< PL is accessible by PS */
} XSysMonPsu;

/** @name Base address offsets and Sysmon block selectors
 * @{
 */
#define XSYSMON_PS 1U		/**< PS Sysmon block selector */
#define XSYSMON_PL 2U		/**< PL Sysmon block selector */
#define XSYSMON_AMS 3U		/**< AMS controller register selector */
#define XPS_BA_OFFSET   0x00000800U /**< PS Sysmon base address offset */
#define XPL_BA_OFFSET   0x00000C00U /**< PL Sysmon base address offset */
#define XSM_ADC_CH_OFFSET 0x00000200U /**< ADC channel data offset */
#define XSM_AMS_CH_OFFSET 0x00000060U /**< AMS channel data offset */
#define XSM_MIN_MAX_CH_OFFSET 0x00000080U /**< Min/Max channel data offset */

/** @} */

/************************* Variable Definitions ******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* @brief Converts raw data to on-chip sensor temperature in centigrade.
*
* @param[in]   AdcData SysMon Raw ADC Data.
*
* @return      The Temperature in centigrades.
*
* @note        C-Style signature:
*              float XSysMonPsu_RawToTemperature_OnChip(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_RawToTemperature_OnChip(AdcData)				\
	((((float)(AdcData)/65536.0f)/0.00196342531f ) - 280.2309f)

/****************************************************************************/
/**
*
* @brief Converts 12-bit OT raw data to on-chip temperature in centigrade.
*
* @param[in]   AdcData SysMon 12-bit Raw ADC Data.
*
* @return      The Temperature in centigrades.
*
* @note        C-Style signature:
*              float XSysMonPsu_RawToTemperature_OT_OnChip(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_RawToTemperature_OT_OnChip(AdcData)				\
	((((float)(AdcData)/4096.0f)/0.00196342531f ) - 280.2309f)

/****************************************************************************/
/**
*
* @brief Converts raw data to external-reference temperature in centigrade.
*
* @param[in]   AdcData SysMon Raw ADC Data.
*
* @return      The Temperature in centigrades.
*
* @note        C-Style signature:
*              float XSysMonPsu_RawToTemperature_ExternalRef(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_RawToTemperature_ExternalRef(AdcData)			\
	((((float)(AdcData)/65536.0f)/0.00197008621f ) - 279.4266f)

/****************************************************************************/
/**
*
* @brief Converts 12-bit OT raw data to external-reference temperature in
* centigrade.
*
* @param[in]   AdcData SysMon 12-bit Raw ADC Data.
*
* @return      The Temperature in centigrades.
*
* @note        C-Style signature:
*              float XSysMonPsu_RawToTemperature_OT_ExternalRef(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_RawToTemperature_OT_ExternalRef(AdcData)			\
	((((float)(AdcData)/4096.0f)/0.00197008621f ) - 279.4266f)
/****************************************************************************/
/**
*
* @brief Converts System Monitor raw data to VpVn supply voltage in volts.
*
* @param[in]   AdcData System Monitor ADC Raw Data.
*
* @return      The Voltage in volts.
*
* @note        C-Style signature:
*              float XSysMonPsu_VpVnRawToVoltage(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_VpVnRawToVoltage(AdcData) 					\
	((((float)(AdcData))* (1.0f))/65536.0f)

/****************************************************************************/
/**
*
* @brief Converts raw data to voltage in volts for supplies other than
* VCCO_PSIO.
*
* @param[in]   AdcData System Monitor ADC Raw Data.
*
* @return      The Voltage in volts.
*
* @note        C-Style signature:
*              float XSysMonPsu_RawToVoltage(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_RawToVoltage(AdcData) 					\
	((((float)(AdcData))* (3.0f))/65536.0f)

/****************************************************************************/
/**
*
* @brief Converts System Monitor raw data to VCCO_PSIO supply voltage in volts.
*
* @param[in]   AdcData System Monitor ADC Raw Data.
*
* @return      The Voltage in volts.
*
* @note        C-Style signature:
*              float XSysMonPsu_VccopsioRawToVoltage(u32 AdcData)
*
*****************************************************************************/
#define XSysMonPsu_VccopsioRawToVoltage(AdcData) 					\
	((((float)(AdcData))* (6.0f))/65536.0f)

/****************************************************************************/
/**
*
* @brief Converts on-chip temperature in centigrade to System Monitor raw data.
*
* @param[in]   Temperature Temperature in centigrades to be
*              converted to System Monitor ADC Raw Data.
*
* @return      The System Monitor ADC Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_TemperatureToRaw_OnChip(float Temperature)
*
*****************************************************************************/
#define XSysMonPsu_TemperatureToRaw_OnChip(Temperature)				\
	((s32)(((Temperature) + 280.2309f)*65536.0f*0.00196342531f))

/****************************************************************************/
/**
*
* @brief Converts on-chip temperature in centigrade to 12-bit OT System
* Monitor raw data.
*
* @param[in]   Temperature Temperature in centigrades to be
*              converted to System Monitor ADC 12-bit Raw Data.
*
* @return      The System Monitor ADC 12-bit Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_TemperatureToRaw_OT_OnChip(float Temperature)
*
*****************************************************************************/
#define XSysMonPsu_TemperatureToRaw_OT_OnChip(Temperature)			\
	((s32)(((Temperature) + 280.2309f)*4096.0f*0.00196342531f))

/****************************************************************************/
/**
*
* @brief Converts external-reference temperature in centigrade to System
* Monitor raw data.
*
* @param[in]   Temperature Temperature in centigrades to be
*              converted to System Monitor ADC Raw Data.
*
* @return      The System Monitor ADC Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_TemperatureToRaw_ExternalRef(float Temperature)
*
*****************************************************************************/
#define XSysMonPsu_TemperatureToRaw_ExternalRef(Temperature)		\
	((s32)(((Temperature) + 279.4266f)*65536.0f*0.00197008621f))

/****************************************************************************/
/**
*
* @brief Converts external-reference temperature in centigrade to 12-bit OT
* raw data.
*
* @param[in]   Temperature Temperature in centigrades to be
*              converted to System Monitor ADC 12-bit Raw Data.
*
* @return      The System Monitor ADC 12-bit Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_TemperatureToRaw_OT_ExternalRef(
*                  float Temperature)
*
*****************************************************************************/
#define XSysMonPsu_TemperatureToRaw_OT_ExternalRef(Temperature)		\
	((s32)(((Temperature) + 279.4266f)*4096.0f*0.00197008621f))
/****************************************************************************/
/**
*
* @brief Converts voltage in volts to raw data for supplies other than
* VCCO_PSIO.
*
* @param[in]   Voltage Voltage in volts to be converted to
*              System Monitor/ADC Raw Data.
*
* @return      The System Monitor ADC Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_VoltageToRaw(float Voltage)
*
*****************************************************************************/
#define XSysMonPsu_VoltageToRaw(Voltage)			 		\
	((s32)((Voltage)*65536.0f/3.0f))

/****************************************************************************/
/**
*
* @brief Converts VCCO_PSIO supply voltage in volts to System Monitor raw data.
*
* @param[in]   Voltage Voltage in volts to be converted to
*              System Monitor/ADC Raw Data.
*
* @return      The System Monitor ADC Raw Data.
*
* @note        C-Style signature:
*              int XSysMonPsu_VccopsioVoltageToRaw(float Voltage)
*
*****************************************************************************/
#define XSysMonPsu_VccopsioVoltageToRaw(Voltage)			 		\
	((s32)((Voltage)*65536.0f/6.0f))

/****************************************************************************/
/**
*
* @brief Calculates the effective base address for the Sysmon instance.
*
* For PS Sysmon, use additional offset XPS_BA_OFFSET and for PL Sysmon, use
* additional offset XPL_BA_OFFSET.
*
* @param[in]   BaseAddress Starting address of the SysMon block in
*              register database.
* @param[in]   SysmonBlk Value that tells whether it is for PS Sysmon
*              block or PL Sysmon block or the AMS controller register region.
*
* @return      Returns the effective baseaddress of the sysmon instance.
*
*****************************************************************************/
static inline UINTPTR XSysMonPsu_GetEffBaseAddress(UINTPTR BaseAddress,
					       u32 SysmonBlk)
{
	UINTPTR EffBaseAddr;

	if (SysmonBlk == XSYSMON_PS) {
		EffBaseAddr = BaseAddress + XPS_BA_OFFSET;
	} else if(SysmonBlk == XSYSMON_PL) {
		EffBaseAddr = BaseAddress + XPL_BA_OFFSET;
	} else {
		EffBaseAddr = BaseAddress;
	}

	return EffBaseAddr;
}

/************************** Function Prototypes ******************************/

/* Functions in xsysmonpsu.c */
s32 XSysMonPsu_CfgInitialize(XSysMonPsu *InstancePtr, XSysMonPsu_Config *ConfigPtr,
			  u32 EffectiveAddr);
void XSysMonPsu_Reset(XSysMonPsu *InstancePtr);
u32 XSysMonPsu_GetStatus(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_StartAdcConversion(XSysMonPsu *InstancePtr);
u16 XSysMonPsu_GetAdcData(XSysMonPsu *InstancePtr, u8 Channel, u32 Block);
u16 XSysMonPsu_GetCalibCoefficient(XSysMonPsu *InstancePtr, u8 CoeffType, u32 SysmonBlk);
u16 XSysMonPsu_GetMinMaxMeasurement(XSysMonPsu *InstancePtr, u8 MeasurementType,
		u32 SysmonBlk);
void XSysMonPsu_SetAvg(XSysMonPsu *InstancePtr, u8 Average, u32 SysmonBlk);
u8 XSysMonPsu_GetAvg(XSysMonPsu *InstancePtr, u32 SysmonBlk);
s32 XSysMonPsu_SetSingleChParams(XSysMonPsu *InstancePtr, u8 Channel,
				u32 IncreaseAcqCycles, u32 IsEventMode,
				u32 IsDifferentialMode, u32 SysmonBlk);
void XSysMonPsu_SetAlarmEnables(XSysMonPsu *InstancePtr, u32 AlmEnableMask,
		u32 SysmonBlk);
u32 XSysMonPsu_GetAlarmEnables(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_SetSequencerMode(XSysMonPsu *InstancePtr, u8 SequencerMode,
		u32 SysmonBlk);
u8 XSysMonPsu_GetSequencerMode(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_SetSequencerEvent(XSysMonPsu *InstancePtr, u32 IsEventMode,
		u32 SysmonBlk);
s32 XSysMonPsu_GetSequencerEvent(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_SetExtenalMux(XSysMonPsu *InstancePtr, u8 Channel, u32 SysmonBlk);
u32 XSysMonPsu_GetExtenalMux(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_SetAdcClkDivisor(XSysMonPsu *InstancePtr, u8 Divisor, u32 SysmonBlk);
u8 XSysMonPsu_GetAdcClkDivisor(XSysMonPsu *InstancePtr, u32 SysmonBlk);
u32 XSysMonPsu_UpdateAdcClkDivisor(XSysMonPsu *InstancePtr, u32 SysmonBlk,
				   u8 *DivCode);
s32 XSysMonPsu_SetSeqChEnables(XSysMonPsu *InstancePtr, u64 ChEnableMask,
		u32 SysmonBlk);
u64 XSysMonPsu_GetSeqAvgEnables(XSysMonPsu *InstancePtr, u32 SysmonBlk);
u64 XSysMonPsu_GetSeqChEnables(XSysMonPsu *InstancePtr, u32 SysmonBlk);
s32 XSysMonPsu_SetSeqAvgEnables(XSysMonPsu *InstancePtr, u64 AvgEnableChMask,
		u32 SysmonBlk);
s32 XSysMonPsu_SetSeqInputMode(XSysMonPsu *InstancePtr, u64 InputModeChMask,
		u32 SysmonBlk);
u64 XSysMonPsu_GetSeqInputMode(XSysMonPsu *InstancePtr, u32 SysmonBlk);
s32 XSysMonPsu_SetSeqAcqTime(XSysMonPsu *InstancePtr, u64 AcqCyclesChMask,
		u32 SysmonBlk);
u64 XSysMonPsu_GetSeqAcqTime(XSysMonPsu *InstancePtr, u32 SysmonBlk);
void XSysMonPsu_SetAlarmThreshold(XSysMonPsu *InstancePtr, u8 AlarmThrReg,
		u16 Value, u32 SysmonBlk);
u16 XSysMonPsu_GetAlarmThreshold(XSysMonPsu *InstancePtr, u8 AlarmThrReg,
		u32 SysmonBlk);
void XSysMonPsu_SetPSAutoConversion(XSysMonPsu *InstancePtr);
u32 XSysMonPsu_GetMonitorStatus(XSysMonPsu *InstancePtr);
void XSysMonPsu_InitInstance(XSysMonPsu *InstancePtr,
	XSysMonPsu_Config *ConfigPtr);

/* interrupt functions in xsysmonpsu_intr.c */
void XSysMonPsu_IntrEnable(XSysMonPsu *InstancePtr, u64 Mask);
void XSysMonPsu_IntrDisable(XSysMonPsu *InstancePtr, u64 Mask);
u64 XSysMonPsu_IntrGetEnabled(XSysMonPsu *InstancePtr);
u64 XSysMonPsu_IntrGetStatus(XSysMonPsu *InstancePtr);
void XSysMonPsu_IntrClear(XSysMonPsu *InstancePtr, u64 Mask);

/* Functions in xsysmonpsu_selftest.c */
s32 XSysMonPsu_SelfTest(XSysMonPsu *InstancePtr);

/* Functions in xsysmonpsu_sinit.c */
#ifndef SDT
XSysMonPsu_Config *XSysMonPsu_LookupConfig(u16 DeviceId);
#else
XSysMonPsu_Config *XSysMonPsu_LookupConfig(u32 BaseAddress);
#endif


#ifdef __cplusplus
}
#endif

/** @} */

#endif /* XSYSMONPSU_H_ */
