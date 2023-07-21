/******************************************************************************
* Copyright (c) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xilskey_jscmd.c
*
* Contains jtag, efuse and bbram related API's
*
* @note
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  	Date     Changes
* ----- ---- 	-------- --------------------------------------------------------
* 1.00a rpoolla 04/26/13 First release
* 1.01a hk      09/18/13 Added BBRAM functionality. Following API's added:
*                        int JtagServerInitBbram(XilSKey_Bbram *InstancePtr)
*                        int Bbram_Init(XilSKey_Bbram *InstancePtr)
*                        int Bbram_ProgramKey(XilSKey_Bbram *InstancePtr)
*                        int Bbram_VerifyKey(XilSKey_Bbram *InstancePtr)
*                        void Bbram_DeInit(void)
* 2.1   kvn     04/01/15 Fixed warnings. CR#716453.
*
* 3.00  vns     31/07/15 Added efuse functionality for Ultrascale.
* 4.00  vns     09/10/15 Modified JtagWrite API as per IEEE 1149.1 standard
*                        added TCK toggle after RTI state change where programming
*                        will start and toggled TCK again at exit of RTI state to
*                        stop programming. CR#885421.
* 5.0   vns     01/07/16 Modified JtagWrite_Ultrascale API as per IEEE 1149.1
*                        standard added TCK toggle after RTI state change where
*                        programming will start and ends programming at
*                        TCK toggle after DR_SELECT state. CR #924262
*                        Modified JtagServerInitBbram to support Ultrascale
*                        BBRAM programming, added Bbram_Init_Ultra,
*                        Bbram_ProgramKey_Ultra, Bbram_VerifyKey_Ultra
*                        and Bbram_DeInit_Ultra APIs
* 6.0   vns     07/07/16 Initialized hardware module connections
*                        Modified JtagWrite_Ultrascale API, to handover
*                        programming sequence to hardware module to take care
*                        of eFUSE programming.
*                        Once Hardware module is triggered, JTAG state will be
*                        entering to IDLE state and will wait for 5us and
*                        toggles TCK pin at 1Mhz frequency. Finally it exists
*                        when jtag state is navigated to DR SELECT by making
*                        END pin to High state.
*                        Modified return type of JtagWrite_Ultrascale API to int
*                        for returning FAILURE on timeout.
*       vns     07/28/16 Modified Bbram_ProgramKey_Ultra API to program control
*                        word based on user inputs.
* 6.4   vns     02/27/18 Added support for virtex and virtex ultrascale plus
* 6.7   psl     03/18/19 Modified code to mask most significant nibble which
*                        represents production version for ultrascale plus.
*               03/20/19 Added eFuse/BBRAM key write support for SSIT devices.
*       psl     03/29/19 Added Support for user configurable GPIO for jtag
*                        control.
*       arc     04/04/19 Fixed CPP warnings.
*       psl     04/15/19 Corrected zynq Dap ID.
* 6.8   psl     06/26/19 Added support for user to add IDCODE, IR_length, SLR Nos,
*                        device series for different devices.
*       psl     08/23/19 Added Debug define to avoid writing of eFuse.
* 6.9   vns     03/18/20 Fixed Armcc compilation errors
* 7.0	am      10/04/20 Resolved MISRA C violations
* 7.1   am      11/29/20 Resolved MISRA C violations
* 7.1   kpt     04/08/21 Ignored product version of user defined IDCODE when comparing
*                        with the tap code read from Jtag
* 7.2   am      07/13/21 Fixed doxygen warnings
* 7.5   ng      07/13/23 added SDT support
*
* </pre>
*
******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>

#if defined(_MSC_VER)
#  pragma warning(disable:4996) /**< 'strcpy': This function or variable may be unsafe */
#endif

typedef long ssize_t;
#endif
#include "xilskey_utils.h"
#include "xilskey_jslib.h"
#include "xilskey_jscmd.h"
#ifdef XSK_MICROBLAZE_PLATFORM
#include "xgpio.h"
#else
#include "xgpiops.h"
#endif

#include "xilskey_bbram.h"
#ifndef SDT
#include "xilskey_config.h"
#endif

XilSKey_JtagSlr XilSKeyJtag; /**< JTAG Tap Instance */


#ifdef DEBUG_PRINT
#define js_printf		printf	/**< define DEBUG_PRINT */
#else
void dummy_printf(const char *ctrl1, ...); /**< function prototype of dummy_printf */
#define js_printf		dummy_printf	/**< Dummy PRINTF */
#endif

#define DEFAULT_FREQUENCY         10000000 /**< Default frequency */
#define MAX_FREQUENCY             30000000 /**< Maximum frequency */
#define ZYNQ_DAP_ID 0x0ba00477		/**< DAP Id */
#define KINTEX_ULTRA_MB_DAP_ID 0x03822093 /**< Kintex Ultrascale microblaze TAP ID */
#define KINTEX_ULTRAPLUS_DAP_ID 0x04A62093 /**< Kintex Ultrascale plus microblaze TAP ID */

#define VIRTEX110_ULTRA_MB_DAP_ID 0x03931093	/**< VIRTEX 110 Ultrascale microblaze TAP id */
#define VIRTEX108_ULTRA_MB_DAP_ID 0x03842093	/**< VIRTEX 108 Ultrascale microblaze TAP id */
#define VIRTEX_ULTRAPLUS_DAP_ID	0x04b31093		/**< VIRTEX Ultrascale plus microblaze TAP id */
#define VIRTEX_ULTRAPLUS_VC13P_DAP_ID 0x04B51093  /**< XCVU13P VIRTEX Ultrascale Plus microblaze TAP id */
#define ZYNQ_ULTRAPLUS_PL_DAP_ID	0x0484A093	/**< Zynq Ultrascale plus TAP ID */
#define set_last_error(JS, ...) js_set_last_error(&(JS)->js.base, __VA_ARGS__)	/**< Set last error */

typedef struct js_port_impl_struct js_port_impl_t;
typedef struct js_command_impl_struct js_command_impl_t;
typedef struct ftd_async_transfer_struct ftd_async_transfer_t;

static js_port_t *g_port = NULL;
static js_server_t *g_js = NULL;
static js_port_descr_t *g_useport = NULL;
u32 GpoOutValue = 0;
extern XSKEfusePl_Fpga PlFpgaFlag;
#ifndef XSK_ARM_PLATFORM
static INLINE int JtagValidateMioPins_Efuse_Ultra(void);
static INLINE int JtagValidateMioPins_Bbram_Ultra(void);
static INLINE u32 JtagGetZuPlusPlIdcode(void);
#endif
void dummy_printf(const char *ctrl1, ...)
{
	(void) ctrl1;
	return;
}
static int close_port(
    js_lib_port_t *port_arg);


struct js_command_impl_struct {
    js_lib_command_t lib; /**< Command to execute */

    size_t byte_offset; /**< Intermediatre command processing state */
    js_state_t start_state; /**< Start state */
    js_state_t end_state;   /**< End state */
    unsigned int write_bytes; /**< Write bytes */
    unsigned int read_bytes;  /**< Read bytes */
    unsigned int partial_bytes; /**< Partial bytes */
    int state_bits; /**< State bits */
};


struct js_zynq {
    js_lib_server_t js; /**< Base class - must be first. */

    js_port_descr_t *port_list; /**< Port list */
    unsigned int port_count;	/**< Port count */
    unsigned int port_max;	/**< Port max */
};


struct js_port_impl_struct {
    js_lib_port_t lib; /**< Base class - must be first. */

    js_lib_command_sequence_t *cmdseq; /**< Command sequence after
					* normalization */

    js_state_t state; /**< Current JTAG state */

    int irPrePadBits;	/**< IR pre-pad bits */
    int irPostPadBits;	/**< IR post-pad bits */
    int drPrePadBits;	/**< DR pre-pad bits */
    int drPostPadBits;	/**< DR post-pad bits */

    js_node_t root_node_obj; /**< Root node object */
};


#ifdef XSK_ARM_PLATFORM
static XGpioPs structXGpioPs;
#else
static XGpio structXGpio;
#endif

/**
 * @name Xilskey jscmd API declarations
 * @{
 */
/**< Prototype declarations for Xilskey jscmd APIs */
int setPin (int pin, int value);
int readPin (int pin);
/** @} */

u32 Bbram_ReadKey[8];

const id_codes_t IDcodeArray [] = {
  {XSK_FPGA_SERIES_ZYNQ,        ZYNQ_DAP_ID, 					 6,  1,  0},  /**< Zynq TAP ID */
  {XSK_FPGA_SERIES_ULTRA,       KINTEX_ULTRA_MB_DAP_ID, 		 6,  1,  0},  /**< Kintex Ultrascale microblaze TAP ID */
  {XSK_FPGA_SERIES_ULTRA,       VIRTEX110_ULTRA_MB_DAP_ID, 		18,  3,  1},  /**< VCU110 xcvu190 VIRTEX Ultrascale microblaze TAP id */
  {XSK_FPGA_SERIES_ULTRA,       VIRTEX108_ULTRA_MB_DAP_ID, 		 6,  1,  0},  /**< VCU108 VIRTEX Ultrascale microblaze TAP id */
  {XSK_FPGA_SERIES_ULTRA_PLUS,  KINTEX_ULTRAPLUS_DAP_ID, 		 6,  1,  0},  /**< Kintex Ultrascale plus microblaze TAP ID */
  {XSK_FPGA_SERIES_ULTRA_PLUS,  VIRTEX_ULTRAPLUS_DAP_ID, 		18,  3,  1},  /**< VCU140 VIRTEX Ultrascale Plus microblaze TAP id */
  {XSK_FPGA_SERIES_ULTRA_PLUS,  VIRTEX_ULTRAPLUS_VC13P_DAP_ID, 	24,  4,  1},  /**< XCVU13P VIRTEX Ultrascale Plus microblaze TAP id */
  {XSK_FPGA_SERIES_ULTRA_PLUS,  ZYNQ_ULTRAPLUS_PL_DAP_ID,		6,   1,  0},  /**< Zynq Ultrascale plus TAP ID */
  {XSK_USER_DEVICE_SERIES, XSK_USER_DEVICE_ID, XSK_USER_DEVICE_IRLEN, XSK_USER_DEVICE_NUMSLR, XSK_USER_DEVICE_MASTER_SLR}   /**< USER_ENTRY */
};

/****************************************************************************/
/**
*
* This function performs Gpio Configuration
*
*****************************************************************************/
void GpioConfig(unsigned long addr, unsigned long mask, unsigned long val)
{
	unsigned long current_val = *(volatile unsigned long *)addr;
	*(volatile unsigned long *) addr = ( val & mask ) | ( current_val  & ~mask);
}

/****************************************************************************/
/**
*
* This function performs Gpio Jtag initialization
*
*****************************************************************************/
void JtagInitGpio (XilSKey_ModuleSelection Module)
{
#ifdef XSK_ARM_PLATFORM
	XGpioPs_Config *ptrConfigPtrPs;

	(void) Module;
	js_printf("===== Initializing PS GPIO pins...\n\r");
	ptrConfigPtrPs = XGpioPs_LookupConfig(0);
	XGpioPs_CfgInitialize(&structXGpioPs,ptrConfigPtrPs,ptrConfigPtrPs->BaseAddr);

	/* Unlock the SLCR block */
	Xil_Out32 (0xF8000000 + 0x8, 0xDF0D);

	GpioConfig((GPIO_BASE_ADDR+(g_mio_jtag_tdi*4)),GPIO_MASK_VAL, GPIO_TDI_VAL);
    GpioConfig((GPIO_BASE_ADDR+(g_mio_jtag_tdo*4)),GPIO_MASK_VAL, GPIO_TDO_VAL);
    GpioConfig((GPIO_BASE_ADDR+(g_mio_jtag_tck*4)),GPIO_MASK_VAL, GPIO_TCK_VAL);
    GpioConfig((GPIO_BASE_ADDR+(g_mio_jtag_tms*4)),GPIO_MASK_VAL, GPIO_TMS_VAL);
    GpioConfig((GPIO_BASE_ADDR+(g_mio_jtag_mux_sel*4)),GPIO_MASK_VAL, GPIO_MUX_SEL_VAL);

	XGpioPs_SetDirectionPin(&structXGpioPs,MIO_TCK,1);
	XGpioPs_SetOutputEnablePin(&structXGpioPs,MIO_TCK,1);

	XGpioPs_SetDirectionPin(&structXGpioPs,MIO_TMS,1);
	XGpioPs_SetOutputEnablePin(&structXGpioPs,MIO_TMS,1);

	XGpioPs_SetDirectionPin(&structXGpioPs,MIO_TDI,1);
	XGpioPs_SetOutputEnablePin(&structXGpioPs,MIO_TDI,1);

	XGpioPs_SetDirectionPin(&structXGpioPs,MIO_MUX_SELECT,1);
	XGpioPs_SetOutputEnablePin(&structXGpioPs,MIO_MUX_SELECT,1);
	XGpioPs_WritePin(&structXGpioPs, MIO_MUX_SELECT, g_mux_sel_def_val);

	XGpioPs_SetDirectionPin(&structXGpioPs,MIO_TDO,0);
	XGpioPs_SetOutputEnablePin(&structXGpioPs,MIO_TDO,0);
#else
	u32 DataDirection;

	js_printf("===== Initializing PL GPIO pins...\n\r");
	XGpio_Config *ptrConfigPtr = XGpio_LookupConfig(GpioDeviceId);
	XGpio_CfgInitialize(&structXGpio, ptrConfigPtr,
						ptrConfigPtr->BaseAddress);

	/* Setting Data direction for GPIO pins */
	if (GpioInPutCh == GpioOutPutCh) {
		DataDirection = XGpio_GetDataDirection(&structXGpio,
							GpioInPutCh);
		DataDirection = DataDirection & (~((1 << GPIO_TDI) |
					(1 << GPIO_TCK) | (1 << GPIO_TMS)));
		DataDirection = DataDirection | (1 << GPIO_TDO);
		if (Module == XSK_EFUSE) {
			DataDirection = DataDirection | ~(1 << GPIO_HWM_START);
			DataDirection = DataDirection |
				(1 << GPIO_HWM_READY) | (1 << GPIO_HWM_END);
		}
		XGpio_SetDataDirection(&structXGpio, GpioInPutCh,
							DataDirection);
	}
	else {
		DataDirection = XGpio_GetDataDirection(&structXGpio,
							GpioOutPutCh);
		DataDirection = DataDirection & (~((1 << GPIO_TDI) |
					(1 << GPIO_TCK) | (1 << GPIO_TMS)));
		if (Module == XSK_EFUSE) {
			DataDirection = DataDirection |
						~(1 << GPIO_HWM_START);
		}
		XGpio_SetDataDirection(&structXGpio, GpioOutPutCh,
							DataDirection);

		DataDirection = XGpio_GetDataDirection(&structXGpio,
							GpioInPutCh);
		DataDirection = DataDirection | (1 << GPIO_TDO);

		if (Module == XSK_EFUSE) {
			DataDirection = DataDirection |
					(1 << GPIO_HWM_READY) | (1 << GPIO_HWM_END);
		}
		XGpio_SetDataDirection(&structXGpio, GpioInPutCh,
							DataDirection);
	}


#endif
} /* initGpio() */

/****************************************************************************/
/**
*
* This function calculates ByteCount From BitCount
*
* @return	ByteCount
*
*****************************************************************************/
int getByteCountFromBitCount (int bitCount)
{
   int byteCount = bitCount / 8;
   if (bitCount % 8)
   {
      byteCount++;
   }
   return (byteCount);
}

/**
 *  File generated by JTAG.EXE. This are the tms values and
 *  the corresponding bit counts to navigate quickly from any state
 *  to any state.
 */
const unsigned int JTAGNavigateTable [256] =
{
    0x0101, // Start=00, End=00, TMSValue=01, BitCount= 1
    0x0001, // Start=00, End=01, TMSValue=00, BitCount= 1
    0x0202, // Start=00, End=02, TMSValue=02, BitCount= 2
    0x0203, // Start=00, End=03, TMSValue=02, BitCount= 3
    0x0204, // Start=00, End=04, TMSValue=02, BitCount= 4
    0x0A04, // Start=00, End=05, TMSValue=0A, BitCount= 4
    0x0A05, // Start=00, End=06, TMSValue=0A, BitCount= 5
    0x2A06, // Start=00, End=07, TMSValue=2A, BitCount= 6
    0x1A05, // Start=00, End=08, TMSValue=1A, BitCount= 5
    0x0603, // Start=00, End=09, TMSValue=06, BitCount= 3
    0x0604, // Start=00, End=10, TMSValue=06, BitCount= 4
    0x0605, // Start=00, End=11, TMSValue=06, BitCount= 5
    0x1605, // Start=00, End=12, TMSValue=16, BitCount= 5
    0x1606, // Start=00, End=13, TMSValue=16, BitCount= 6
    0x5607, // Start=00, End=14, TMSValue=56, BitCount= 7
    0x3606, // Start=00, End=15, TMSValue=36, BitCount= 6
    0x0703, // Start=01, End=00, TMSValue=07, BitCount= 3
    0x0001, // Start=01, End=01, TMSValue=00, BitCount= 1
    0x0101, // Start=01, End=02, TMSValue=01, BitCount= 1
    0x0102, // Start=01, End=03, TMSValue=01, BitCount= 2
    0x0103, // Start=01, End=04, TMSValue=01, BitCount= 3
    0x0503, // Start=01, End=05, TMSValue=05, BitCount= 3
    0x0504, // Start=01, End=06, TMSValue=05, BitCount= 4
    0x1505, // Start=01, End=07, TMSValue=15, BitCount= 5
    0x0D04, // Start=01, End=08, TMSValue=0D, BitCount= 4
    0x0302, // Start=01, End=09, TMSValue=03, BitCount= 2
    0x0303, // Start=01, End=10, TMSValue=03, BitCount= 3
    0x0304, // Start=01, End=11, TMSValue=03, BitCount= 4
    0x0B04, // Start=01, End=12, TMSValue=0B, BitCount= 4
    0x0B05, // Start=01, End=13, TMSValue=0B, BitCount= 5
    0x2B06, // Start=01, End=14, TMSValue=2B, BitCount= 6
    0x1B05, // Start=01, End=15, TMSValue=1B, BitCount= 5
    0x0302, // Start=02, End=00, TMSValue=03, BitCount= 2
    0x0303, // Start=02, End=01, TMSValue=03, BitCount= 3
    0x0B04, // Start=02, End=02, TMSValue=0B, BitCount= 4
    0x0001, // Start=02, End=03, TMSValue=00, BitCount= 1
    0x0002, // Start=02, End=04, TMSValue=00, BitCount= 2
    0x0202, // Start=02, End=05, TMSValue=02, BitCount= 2
    0x0203, // Start=02, End=06, TMSValue=02, BitCount= 3
    0x0A04, // Start=02, End=07, TMSValue=0A, BitCount= 4
    0x0603, // Start=02, End=08, TMSValue=06, BitCount= 3
    0x0101, // Start=02, End=09, TMSValue=01, BitCount= 1
    0x0102, // Start=02, End=10, TMSValue=01, BitCount= 2
    0x0103, // Start=02, End=11, TMSValue=01, BitCount= 3
    0x0503, // Start=02, End=12, TMSValue=05, BitCount= 3
    0x0504, // Start=02, End=13, TMSValue=05, BitCount= 4
    0x1505, // Start=02, End=14, TMSValue=15, BitCount= 5
    0x0D04, // Start=02, End=15, TMSValue=0D, BitCount= 4
    0x1F05, // Start=03, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=03, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=03, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=03, End=03, TMSValue=07, BitCount= 4
    0x0001, // Start=03, End=04, TMSValue=00, BitCount= 1
    0x0101, // Start=03, End=05, TMSValue=01, BitCount= 1
    0x0102, // Start=03, End=06, TMSValue=01, BitCount= 2
    0x0503, // Start=03, End=07, TMSValue=05, BitCount= 3
    0x0302, // Start=03, End=08, TMSValue=03, BitCount= 2
    0x0F04, // Start=03, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=03, End=10, TMSValue=0F, BitCount= 5
    0x0F06, // Start=03, End=11, TMSValue=0F, BitCount= 6
    0x2F06, // Start=03, End=12, TMSValue=2F, BitCount= 6
    0x2F07, // Start=03, End=13, TMSValue=2F, BitCount= 7
    0xAF08, // Start=03, End=14, TMSValue=AF, BitCount= 8
    0x6F07, // Start=03, End=15, TMSValue=6F, BitCount= 7
    0x1F05, // Start=04, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=04, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=04, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=04, End=03, TMSValue=07, BitCount= 4
    0x0001, // Start=04, End=04, TMSValue=00, BitCount= 1
    0x0101, // Start=04, End=05, TMSValue=01, BitCount= 1
    0x0102, // Start=04, End=06, TMSValue=01, BitCount= 2
    0x0503, // Start=04, End=07, TMSValue=05, BitCount= 3
    0x0302, // Start=04, End=08, TMSValue=03, BitCount= 2
    0x0F04, // Start=04, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=04, End=10, TMSValue=0F, BitCount= 5
    0x0F06, // Start=04, End=11, TMSValue=0F, BitCount= 6
    0x2F06, // Start=04, End=12, TMSValue=2F, BitCount= 6
    0x2F07, // Start=04, End=13, TMSValue=2F, BitCount= 7
    0xAF08, // Start=04, End=14, TMSValue=AF, BitCount= 8
    0x6F07, // Start=04, End=15, TMSValue=6F, BitCount= 7
    0x0F04, // Start=05, End=00, TMSValue=0F, BitCount= 4
    0x0102, // Start=05, End=01, TMSValue=01, BitCount= 2
    0x0302, // Start=05, End=02, TMSValue=03, BitCount= 2
    0x0303, // Start=05, End=03, TMSValue=03, BitCount= 3
    0x0203, // Start=05, End=04, TMSValue=02, BitCount= 3
    0x0B04, // Start=05, End=05, TMSValue=0B, BitCount= 4
    0x0001, // Start=05, End=06, TMSValue=00, BitCount= 1
    0x0202, // Start=05, End=07, TMSValue=02, BitCount= 2
    0x0101, // Start=05, End=08, TMSValue=01, BitCount= 1
    0x0703, // Start=05, End=09, TMSValue=07, BitCount= 3
    0x0704, // Start=05, End=10, TMSValue=07, BitCount= 4
    0x0705, // Start=05, End=11, TMSValue=07, BitCount= 5
    0x1705, // Start=05, End=12, TMSValue=17, BitCount= 5
    0x1706, // Start=05, End=13, TMSValue=17, BitCount= 6
    0x5707, // Start=05, End=14, TMSValue=57, BitCount= 7
    0x3706, // Start=05, End=15, TMSValue=37, BitCount= 6
    0x1F05, // Start=06, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=06, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=06, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=06, End=03, TMSValue=07, BitCount= 4
    0x0102, // Start=06, End=04, TMSValue=01, BitCount= 2
    0x0503, // Start=06, End=05, TMSValue=05, BitCount= 3
    0x0001, // Start=06, End=06, TMSValue=00, BitCount= 1
    0x0101, // Start=06, End=07, TMSValue=01, BitCount= 1
    0x0302, // Start=06, End=08, TMSValue=03, BitCount= 2
    0x0F04, // Start=06, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=06, End=10, TMSValue=0F, BitCount= 5
    0x0F06, // Start=06, End=11, TMSValue=0F, BitCount= 6
    0x2F06, // Start=06, End=12, TMSValue=2F, BitCount= 6
    0x2F07, // Start=06, End=13, TMSValue=2F, BitCount= 7
    0xAF08, // Start=06, End=14, TMSValue=AF, BitCount= 8
    0x6F07, // Start=06, End=15, TMSValue=6F, BitCount= 7
    0x0F04, // Start=07, End=00, TMSValue=0F, BitCount= 4
    0x0102, // Start=07, End=01, TMSValue=01, BitCount= 2
    0x0302, // Start=07, End=02, TMSValue=03, BitCount= 2
    0x0303, // Start=07, End=03, TMSValue=03, BitCount= 3
    0x0001, // Start=07, End=04, TMSValue=00, BitCount= 1
    0x0202, // Start=07, End=05, TMSValue=02, BitCount= 2
    0x0203, // Start=07, End=06, TMSValue=02, BitCount= 3
    0x0A04, // Start=07, End=07, TMSValue=0A, BitCount= 4
    0x0101, // Start=07, End=08, TMSValue=01, BitCount= 1
    0x0703, // Start=07, End=09, TMSValue=07, BitCount= 3
    0x0704, // Start=07, End=10, TMSValue=07, BitCount= 4
    0x0705, // Start=07, End=11, TMSValue=07, BitCount= 5
    0x1705, // Start=07, End=12, TMSValue=17, BitCount= 5
    0x1706, // Start=07, End=13, TMSValue=17, BitCount= 6
    0x5707, // Start=07, End=14, TMSValue=57, BitCount= 7
    0x3706, // Start=07, End=15, TMSValue=37, BitCount= 6
    0x0703, // Start=08, End=00, TMSValue=07, BitCount= 3
    0x0001, // Start=08, End=01, TMSValue=00, BitCount= 1
    0x0101, // Start=08, End=02, TMSValue=01, BitCount= 1
    0x0102, // Start=08, End=03, TMSValue=01, BitCount= 2
    0x0103, // Start=08, End=04, TMSValue=01, BitCount= 3
    0x0503, // Start=08, End=05, TMSValue=05, BitCount= 3
    0x0504, // Start=08, End=06, TMSValue=05, BitCount= 4
    0x1505, // Start=08, End=07, TMSValue=15, BitCount= 5
    0x0D04, // Start=08, End=08, TMSValue=0D, BitCount= 4
    0x0302, // Start=08, End=09, TMSValue=03, BitCount= 2
    0x0303, // Start=08, End=10, TMSValue=03, BitCount= 3
    0x0304, // Start=08, End=11, TMSValue=03, BitCount= 4
    0x0B04, // Start=08, End=12, TMSValue=0B, BitCount= 4
    0x0B05, // Start=08, End=13, TMSValue=0B, BitCount= 5
    0x2B06, // Start=08, End=14, TMSValue=2B, BitCount= 6
    0x1B05, // Start=08, End=15, TMSValue=1B, BitCount= 5
    0x0101, // Start=09, End=00, TMSValue=01, BitCount= 1
    0x0102, // Start=09, End=01, TMSValue=01, BitCount= 2
    0x0503, // Start=09, End=02, TMSValue=05, BitCount= 3
    0x0504, // Start=09, End=03, TMSValue=05, BitCount= 4
    0x0505, // Start=09, End=04, TMSValue=05, BitCount= 5
    0x1505, // Start=09, End=05, TMSValue=15, BitCount= 5
    0x1506, // Start=09, End=06, TMSValue=15, BitCount= 6
    0x5507, // Start=09, End=07, TMSValue=55, BitCount= 7
    0x3506, // Start=09, End=08, TMSValue=35, BitCount= 6
    0x0D04, // Start=09, End=09, TMSValue=0D, BitCount= 4
    0x0001, // Start=09, End=10, TMSValue=00, BitCount= 1
    0x0002, // Start=09, End=11, TMSValue=00, BitCount= 2
    0x0202, // Start=09, End=12, TMSValue=02, BitCount= 2
    0x0203, // Start=09, End=13, TMSValue=02, BitCount= 3
    0x0A04, // Start=09, End=14, TMSValue=0A, BitCount= 4
    0x0603, // Start=09, End=15, TMSValue=06, BitCount= 3
    0x1F05, // Start=10, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=10, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=10, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=10, End=03, TMSValue=07, BitCount= 4
    0x0705, // Start=10, End=04, TMSValue=07, BitCount= 5
    0x1705, // Start=10, End=05, TMSValue=17, BitCount= 5
    0x1706, // Start=10, End=06, TMSValue=17, BitCount= 6
    0x5707, // Start=10, End=07, TMSValue=57, BitCount= 7
    0x3706, // Start=10, End=08, TMSValue=37, BitCount= 6
    0x0F04, // Start=10, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=10, End=10, TMSValue=0F, BitCount= 5
    0x0001, // Start=10, End=11, TMSValue=00, BitCount= 1
    0x0101, // Start=10, End=12, TMSValue=01, BitCount= 1
    0x0102, // Start=10, End=13, TMSValue=01, BitCount= 2
    0x0503, // Start=10, End=14, TMSValue=05, BitCount= 3
    0x0302, // Start=10, End=15, TMSValue=03, BitCount= 2
    0x1F05, // Start=11, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=11, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=11, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=11, End=03, TMSValue=07, BitCount= 4
    0x0705, // Start=11, End=04, TMSValue=07, BitCount= 5
    0x1705, // Start=11, End=05, TMSValue=17, BitCount= 5
    0x1706, // Start=11, End=06, TMSValue=17, BitCount= 6
    0x5707, // Start=11, End=07, TMSValue=57, BitCount= 7
    0x3706, // Start=11, End=08, TMSValue=37, BitCount= 6
    0x0F04, // Start=11, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=11, End=10, TMSValue=0F, BitCount= 5
    0x0001, // Start=11, End=11, TMSValue=00, BitCount= 1
    0x0101, // Start=11, End=12, TMSValue=01, BitCount= 1
    0x0102, // Start=11, End=13, TMSValue=01, BitCount= 2
    0x0503, // Start=11, End=14, TMSValue=05, BitCount= 3
    0x0302, // Start=11, End=15, TMSValue=03, BitCount= 2
    0x0F04, // Start=12, End=00, TMSValue=0F, BitCount= 4
    0x0102, // Start=12, End=01, TMSValue=01, BitCount= 2
    0x0302, // Start=12, End=02, TMSValue=03, BitCount= 2
    0x0303, // Start=12, End=03, TMSValue=03, BitCount= 3
    0x0304, // Start=12, End=04, TMSValue=03, BitCount= 4
    0x0B04, // Start=12, End=05, TMSValue=0B, BitCount= 4
    0x0B05, // Start=12, End=06, TMSValue=0B, BitCount= 5
    0x2B06, // Start=12, End=07, TMSValue=2B, BitCount= 6
    0x1B05, // Start=12, End=08, TMSValue=1B, BitCount= 5
    0x0703, // Start=12, End=09, TMSValue=07, BitCount= 3
    0x0704, // Start=12, End=10, TMSValue=07, BitCount= 4
    0x0203, // Start=12, End=11, TMSValue=02, BitCount= 3
    0x0A04, // Start=12, End=12, TMSValue=0A, BitCount= 4
    0x0001, // Start=12, End=13, TMSValue=00, BitCount= 1
    0x0202, // Start=12, End=14, TMSValue=02, BitCount= 2
    0x0101, // Start=12, End=15, TMSValue=01, BitCount= 1
    0x1F05, // Start=13, End=00, TMSValue=1F, BitCount= 5
    0x0303, // Start=13, End=01, TMSValue=03, BitCount= 3
    0x0703, // Start=13, End=02, TMSValue=07, BitCount= 3
    0x0704, // Start=13, End=03, TMSValue=07, BitCount= 4
    0x0705, // Start=13, End=04, TMSValue=07, BitCount= 5
    0x1705, // Start=13, End=05, TMSValue=17, BitCount= 5
    0x1706, // Start=13, End=06, TMSValue=17, BitCount= 6
    0x5707, // Start=13, End=07, TMSValue=57, BitCount= 7
    0x3706, // Start=13, End=08, TMSValue=37, BitCount= 6
    0x0F04, // Start=13, End=09, TMSValue=0F, BitCount= 4
    0x0F05, // Start=13, End=10, TMSValue=0F, BitCount= 5
    0x0102, // Start=13, End=11, TMSValue=01, BitCount= 2
    0x0503, // Start=13, End=12, TMSValue=05, BitCount= 3
    0x0001, // Start=13, End=13, TMSValue=00, BitCount= 1
    0x0101, // Start=13, End=14, TMSValue=01, BitCount= 1
    0x0302, // Start=13, End=15, TMSValue=03, BitCount= 2
    0x0F04, // Start=14, End=00, TMSValue=0F, BitCount= 4
    0x0102, // Start=14, End=01, TMSValue=01, BitCount= 2
    0x0302, // Start=14, End=02, TMSValue=03, BitCount= 2
    0x0303, // Start=14, End=03, TMSValue=03, BitCount= 3
    0x0304, // Start=14, End=04, TMSValue=03, BitCount= 4
    0x0B04, // Start=14, End=05, TMSValue=0B, BitCount= 4
    0x0B05, // Start=14, End=06, TMSValue=0B, BitCount= 5
    0x2B06, // Start=14, End=07, TMSValue=2B, BitCount= 6
    0x1B05, // Start=14, End=08, TMSValue=1B, BitCount= 5
    0x0703, // Start=14, End=09, TMSValue=07, BitCount= 3
    0x0704, // Start=14, End=10, TMSValue=07, BitCount= 4
    0x0001, // Start=14, End=11, TMSValue=00, BitCount= 1
    0x0202, // Start=14, End=12, TMSValue=02, BitCount= 2
    0x0203, // Start=14, End=13, TMSValue=02, BitCount= 3
    0x0A04, // Start=14, End=14, TMSValue=0A, BitCount= 4
    0x0101, // Start=14, End=15, TMSValue=01, BitCount= 1
    0x0703, // Start=15, End=00, TMSValue=07, BitCount= 3
    0x0001, // Start=15, End=01, TMSValue=00, BitCount= 1
    0x0101, // Start=15, End=02, TMSValue=01, BitCount= 1
    0x0102, // Start=15, End=03, TMSValue=01, BitCount= 2
    0x0103, // Start=15, End=04, TMSValue=01, BitCount= 3
    0x0503, // Start=15, End=05, TMSValue=05, BitCount= 3
    0x0504, // Start=15, End=06, TMSValue=05, BitCount= 4
    0x1505, // Start=15, End=07, TMSValue=15, BitCount= 5
    0x0D04, // Start=15, End=08, TMSValue=0D, BitCount= 4
    0x0302, // Start=15, End=09, TMSValue=03, BitCount= 2
    0x0303, // Start=15, End=10, TMSValue=03, BitCount= 3
    0x0304, // Start=15, End=11, TMSValue=03, BitCount= 4
    0x0B04, // Start=15, End=12, TMSValue=0B, BitCount= 4
    0x0B05, // Start=15, End=13, TMSValue=0B, BitCount= 5
    0x2B06, // Start=15, End=14, TMSValue=2B, BitCount= 6
    0x1B05, // Start=15, End=15, TMSValue=1B, BitCount= 5
};

/****************************************************************************/
/**
*
* This function calculates instructions
*
* @return	u32-bit val value
*
*****************************************************************************/
u32 calcInstr(u8 Instr, u8 cmd)
{
	int index;
	u32 val=0;
	int SlrNum;
	int SlrMax=XilSKeyJtag.NumSlr;

	if(cmd == CALC_MSTR) {
		SlrNum = 0;
	} else {
		SlrNum=XilSKeyJtag.CurSlr;
	}

	for(index=0; index<SlrMax; index++) {
		val = val << 6;
		if((cmd == CALC_ALL) || (SlrNum == index)) {
			val |= Instr;
		} else {
			val |= FEEDTHROUGH;
		}
	}
	return val;
}

/****************************************************************************/
/**
*
* This function calculates Navigate TAPValue
*
* @return	int tableValue
*
*****************************************************************************/
unsigned int getNavigateTAPValue (unsigned char startState, unsigned char endState)
{
    unsigned char index;
    unsigned int tableValue;

    index = (startState << 4) & 0xF0;
    index |= endState;

    tableValue = JTAGNavigateTable [index];

    return (tableValue);
}

/****************************************************************************/
/**
*
* This function performs NavigateTAP
*
*****************************************************************************/
void navigateTAP (unsigned char startState, unsigned char endState)
{
    unsigned int tableValue = getNavigateTAPValue (startState, endState);
    unsigned char tmsValue = (tableValue >> 8) & 0x00FF;
    unsigned char bitCount = tableValue & 0x00FF;

    unsigned char mask = 0x01;

    js_printf ("navigate - start=%d, end=%d\n", startState, endState);
    js_printf ("tmsvalue = 0x%02X, bitCount = %d\n", tmsValue, bitCount);

    if (endState == JS_RESET)
    {
	// initialize state to JS_RESET
	setPin (MIO_TMS, 1);

	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);

	return;
    }

    if (startState != endState)
    {
	while (bitCount)
	{
		if (tmsValue & mask)
		{
			setPin (MIO_TMS, 1);
		}
		else
		{
			setPin (MIO_TMS, 0);
		}
		// pulse tck pin
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);

		mask <<= 1;
		bitCount--;
	}
    }
    else
    {
	js_printf ("startState == endState; navigateTAP not executed.");
    }
}

int setPin (int pin, int value)
{
	int status = 1;
#ifdef XSK_ARM_PLATFORM
	XGpioPs_WritePin(&structXGpioPs, pin, value);
#else
	u32 Mask = (1 << pin);
	u32 GpoOut = (value) ? (0xFFFFFFFF) : (0x0);

	GpoOutValue = (GpoOutValue & (~Mask)) | (GpoOut & Mask);
	XGpio_DiscreteWrite(&structXGpio, GpioOutPutCh, GpoOutValue);
#endif

	return (status);
}

int readPin (int pin)
{
#ifdef XSK_ARM_PLATFORM
	int retVal = XGpioPs_ReadPin(&structXGpioPs, pin);
#else
	int retVal = XGpio_DiscreteRead(&structXGpio, GpioInPutCh);
	retVal = retVal & (1 << pin);
	retVal = retVal ? 1: 0;
#endif

	return (retVal);
}

/****************************************************************************/
/**
*
* This function performs Jtag shift TDI operation
*
*****************************************************************************/
void jtagShiftTDI (unsigned char tdiData, unsigned char* tdoData, int clkCount, int exitState)
{
   int count = clkCount;
   unsigned char mask = 0x01;

   while (count)
   {
	   if (tdoData) // read tdo bit
	   {
		   if (readPin (MIO_TDO))
		   {
			   *(unsigned char*)tdoData |= mask;
		   }
		   else
		   {
			   *(unsigned char*)tdoData &= ~mask;
		   }
	   }
	   if (tdiData & mask)
	   {
		   setPin (MIO_TDI, 1);
	   }
	   else
       {
		   setPin (MIO_TDI, 0);
       }
       if (exitState == 1)
       {
           if (count == 1)
           {
               setPin (MIO_TMS, 1);
           }
       }

       // pulse tck pin
       setPin (MIO_TCK, 1);
       setPin (MIO_TCK, 0);

       mask <<= 1;
       count--;
   }
}

/****************************************************************************/
/**
*
* This function will be used to shift long bits.
*
* @return	1 if shift state is to be exited.
*
* @note		If tdoBuf != NULL, data will be read from device.
*
*****************************************************************************/
int jtagShiftTDIBits (unsigned char* tdiBuf, unsigned char* tdoBuf, int bitCount, js_state_t endState, unsigned int flags)
{
	unsigned char* tdoByte = NULL;
    int count = 8;
    int exitState = 0;
    int currentByteCount = getByteCountFromBitCount (bitCount);
    int byteCount = currentByteCount;
    int index = 0;
    unsigned char TdiTemp = 0x0;
    int bitsLeft = 8;
    if (bitCount % 8)
    {
	bitsLeft = bitCount % 8;
    }

	TdiTemp = 0xFF;

    while (index < byteCount)
    {
	currentByteCount--;
	if (currentByteCount == 0)  // last byte
	{
		if (bitsLeft != 8)
		{
			count = bitsLeft;
		}
		if (flags & JS_TO_IR)
		{
			if (endState != JS_IRSHIFT)
			{
				exitState = 1;
			}
		}
		else
		{
			if (endState != JS_DRSHIFT)
			{
				exitState = 1;
			}
		}
	}

	if (tdoBuf) // read tdo data
	{
		tdoByte = &tdoBuf [index];
	}

	if(tdiBuf != NULL)
		jtagShiftTDI (tdiBuf [index], tdoByte, count, exitState);
	else
		jtagShiftTDI (TdiTemp, tdoByte, count, exitState);

        if (tdoBuf)
		js_printf ("tdoBuf=0x%02X\n", *tdoByte);

        index++;
    }

    return (exitState);
}

/****************************************************************************/
/**
*
* This function sets Jtag Pre and Post Pads
*
* @return	status value
*
*****************************************************************************/
int jtag_setPreAndPostPads (js_port_t *port_arg, int irPrePadBits, int irPostPadBits, int drPrePadBits, int drPostPadBits)
{
	int status = 0;
	js_port_impl_t *port = (js_port_impl_t *)port_arg;
	port->irPrePadBits = irPrePadBits;
	port->irPostPadBits = irPostPadBits;
	port->drPrePadBits = drPrePadBits;
	port->drPostPadBits = drPostPadBits;
	return (status);
}

/****************************************************************************/
/**
*
* This function performs Jtag navigate operation
*
* @return	status value
*
*****************************************************************************/
int jtag_navigate (js_port_t *port, js_state_t state)
{
	int status = 0;
	js_command_sequence_t *cmds;

	if (port == NULL) {
		return -1;
	}

	cmds = js_create_command_sequence(port->root_node);
	if (cmds == NULL)
	{
		return -1;
	}
	js_add_state_change(cmds, state, 0);
	status = js_run_command_sequence(cmds);
	js_delete_command_sequence(cmds);
	return (status);
}

/****************************************************************************/
/**
*
* This function takes care of padding Must be set up with pre/post ir/dr info
*
* @return	status value
*
*****************************************************************************/
int jtag_shift (js_port_t *port_arg, unsigned char mode, int bits, unsigned char* wrBuffer, unsigned char* rdBuffer, js_state_t state)
{
	int status = 0;
	unsigned char* rdPtr;
	unsigned char* wrPtr;
	js_command_sequence_t *cmds;
	js_port_impl_t *port;

	cmds = js_create_command_sequence(port_arg->root_node);
	if (cmds == NULL)
	{
		return -1;
	}

	port = (js_port_impl_t *)port_arg;

	if (mode == ATOMIC_IR_SCAN)
	{
		if (port->irPrePadBits > 0)
		{
			js_add_shift (cmds, JS_TO_IR | JS_ONES, port->irPrePadBits, NULL, NULL, JS_IRSHIFT);
		}
		if (port->irPostPadBits == 0)
		{
			js_add_shift (cmds, JS_TO_IR, bits, ((wrBuffer == NULL) ? NULL : &wrPtr), ((rdBuffer == NULL) ? NULL : &rdPtr), state);
		}
		else
		{
			js_add_shift (cmds, JS_TO_IR, bits, ((wrBuffer == NULL) ? NULL : &wrPtr), ((rdBuffer == NULL) ? NULL : &rdPtr), JS_IRSHIFT);
			js_add_shift (cmds, JS_TO_IR | JS_ONES, port->irPostPadBits, NULL, NULL, state);
		}
	}
	else
	{
		if (port->drPrePadBits > 0)
		{
			js_add_shift (cmds, 0, port->drPrePadBits, NULL, NULL, JS_DRSHIFT);
		}
		if (port->drPostPadBits == 0)
		{
			js_add_shift (cmds, 0, bits, ((wrBuffer == NULL) ? NULL : &wrPtr), ((rdBuffer == NULL) ? NULL : &rdPtr), state);
		}
		else
		{
			js_add_shift (cmds, 0, bits, ((wrBuffer == NULL) ? NULL : &wrPtr), ((rdBuffer == NULL) ? NULL : &rdPtr), JS_DRSHIFT);
			js_add_shift (cmds, 0, port->drPostPadBits, NULL, NULL, state);
		}
	}

	if (wrBuffer) // copy input tdi data to cmd tdi buffer.
	{
		memcpy (wrPtr, wrBuffer, getByteCountFromBitCount (bits));
	}

	status = js_run_command_sequence(cmds);

	if (rdBuffer) // copy cmd tdo buffer to tdo buffer.
	{
		memcpy (rdBuffer, rdPtr, getByteCountFromBitCount (bits));
	}

	js_delete_command_sequence(cmds);
	return (status);
}


/*static int update_clock_frequency(
    js_port_impl_t *port)
{
        return 0;
}*/

/****************************************************************************/
/**
*
* This function performs get property operation
*
* @return	-1 value
*
*****************************************************************************/
static int get_property(
    js_lib_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t *valuep)
{
	js_port_impl_t *port;
	(void) kind;
	(void) valuep;

	port = (js_port_impl_t *)port_arg;

    js_set_last_error(port->lib.base.server, "unsupported property");
    return -1;
}

/****************************************************************************/
/**
*
* This function performs set property operation
*
* @return	-1 value
*
*****************************************************************************/
static int set_property(
    js_lib_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t value)
{
	js_port_impl_t *port;

	(void) kind;
	(void) value;
	port = (js_port_impl_t *)port_arg;

    js_set_last_error(port->lib.base.server, "unsupported property");
    return -1;
}

/****************************************************************************/
/**
*
* This function performs run command sequence operation
*
* @return	0 value
*
*****************************************************************************/
static int run_command_sequence(
    js_lib_command_sequence_t *cmds)
{
    js_port_impl_t *port = (js_port_impl_t *)cmds->base.node->port;
    js_command_impl_t *cmd_start;
    js_command_impl_t *cmd_end;
    js_command_impl_t *cmd;
    js_state_t state;

    if (js_clear_command_sequence(&port->cmdseq->base) < 0 ||
        js_lib_normalize_command_sequence(port->cmdseq, cmds) < 0)
        return -1;

    cmd_start = (js_command_impl_t *)port->cmdseq->cmd_list;
    cmd_end = cmd_start + port->cmdseq->cmd_count;

	cmd = cmd_start;
	state = port->state;

    while (cmd < cmd_end)
    {
        js_command_kind_t kind = cmd->lib.kind;

        switch (kind)
        {
		case JS_CMD_SET_STATE:
		{
			navigateTAP (state, cmd->lib.state);
			state = cmd->lib.state; // save current state
			break;
		}
		case JS_CMD_SHIFT:
		{
			if (cmd->lib.flags & JS_TO_IR)
			{
				navigateTAP (state, JS_IRSHIFT);
				state = JS_IRSHIFT;
			}
			else
			{
				navigateTAP (state, JS_DRSHIFT);
				state = JS_DRSHIFT;
			}

			if (jtagShiftTDIBits (cmd->lib.tdi_buf, cmd->lib.tdo_buf, cmd->lib.count, cmd->lib.state, cmd->lib.flags))
			{
				/* Handle state change after scan */
				if (state != cmd->lib.state)
				{
					if (cmd->lib.flags & JS_TO_IR)
					{
						state = JS_IREXIT1;
					}
					else
					{
						state = JS_DREXIT1;
					}
					navigateTAP (state, cmd->lib.state);
					state = cmd->lib.state;
				}
			}
			break;
		}
		default:
			fprintf(stderr, "write_commands: unsupported command %d\n", kind);
			break;
        }
        cmd++;
    }

    port->state = state; // save current state

    return 0;
}

/****************************************************************************/
/**
*
* This function performs get port description list operation
*
* @return	1 value
*
*****************************************************************************/
static int get_port_descr_list(
    js_lib_server_t *server,
    js_port_descr_t **port_listp)
{
	(void) server;
	(void) port_listp;
	return 1;
}

/****************************************************************************/
/**
*
* This function performs open port operation
*
* @return	-1 value
*
*****************************************************************************/
int open_port(
    js_lib_server_t *server,
    js_port_descr_t *port_descr,
    js_lib_port_t **result)
{
	struct js_zynq *js = (struct js_zynq *)server;

	js_port_impl_t *port;
	js_node_t *node;

	(void) port_descr;

	port = (js_port_impl_t *)malloc(sizeof *port);
	if (port == NULL) {
	     js_set_last_error(&server->base, "end of memory");
	     return -1;
	}
	memset(port, 0, sizeof *port);
	port->lib.base.server = &server->base;

	/* Initialize root node */
	port->lib.base.root_node = node = &port->root_node_obj;
	node->port = &port->lib.base;
	node->is_tap = 1;
	node->is_active = 1;
	node->idcode = JS_DUMMY_IDCODE;
	node->name = "whole scan chain";

	port->lib.get_property = get_property;
	port->lib.set_property = set_property;
	port->lib.run_command_sequence = run_command_sequence;
	port->lib.close_port = close_port;

	if ((port->cmdseq = (js_lib_command_sequence_t *)js_create_command_sequence(node)) == NULL) {
	    set_last_error(js, "end of memory");
	    goto error;
	}
	port->cmdseq->cmd_size = sizeof(js_command_impl_t);

	port->irPrePadBits = 0;
	port->irPostPadBits = 0;
	port->drPrePadBits = 0;
	port->drPostPadBits = 0;

	// Initialize JTAG drivers
	setPin (MIO_TCK, 0);
	setPin (MIO_TMS, 0);
	setPin (MIO_TDI, 0);

	// initialize state to JS_RESET
	port->state = JS_RESET;
	jtag_navigate (node->port, JS_RESET);

	*result = &port->lib;
	return 0;

	error:

	return -1;
}

/****************************************************************************/
/**
*
* This function performs close port operation
*
* @return	ret value
*
*****************************************************************************/
static int close_port(
    js_lib_port_t *port_arg)
{
	js_port_impl_t *port = (js_port_impl_t *)port_arg;
	int ret = 0;

	js_printf ("\n\nClosing Port.\n\n");

	setPin (MIO_TDI, 0);
	setPin (MIO_TMS, 0);
	setPin (MIO_TCK, 0);

	js_delete_command_sequence(&port->cmdseq->base);

    free (port);

    return ret;
}

/****************************************************************************/
/**
*
* This function performs deinit server operation
*
*****************************************************************************/
static void deinit_server(
    js_lib_server_t *server)
{
    struct js_zynq *js = (struct js_zynq *)server;

    if (js->port_list)
        free(js->port_list);
    free(js);
}


js_server_t *js_init_zynq()
{
    struct js_zynq *js;

    if ((js = (struct js_zynq *)malloc(sizeof *js)) == NULL) {
        return NULL;
    }
    memset(js, 0, sizeof *js);
    js->js.get_port_descr_list = get_port_descr_list;
    js->js.open_port = open_port;
    js->js.deinit_server = deinit_server;

    return &js->js.base;
}

void JtagWrite(unsigned char row, unsigned char bit)
{

	/* Following is the method to program FUSE register in Direct Macro Access way.
    Go to TLR to clear FUSE_CTS
    Load FUSE_CTS instruction on IR
    Step to CDR/SDR to shift in the command word
    dma=1; pgm=1; a_row<4:0> & a_bit<4:0>
    Continuously shift in MAGIC_CTS_WRITE
    Loop back to E1DR/UDR/SDS/CDR/E1DR/UDR
    Go to RTI and stay in RTI EXACTLY Tpgm = 12 us (tbd) and immediately exit to SDS
    Go to TLR to clear FUSE_CTS
	*/
    unsigned char wrBuffer [8];
    u64 time = 0;
	u32 *WriteBuf32 = (u32 *)wrBuffer;

	// program FUSE_USER bit in row 31 bit 0
	//Go to TLR to clear FUSE_CTS
	jtag_navigate (g_port, JS_RESET);

	//Load FUSE_CTS instruction on IR
	jtag_setPreAndPostPads (g_port, 0, ZYNQ_DAP_IR_LENGTH, 0, 1);
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); // FUSE_CTS instruction
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, wrBuffer, NULL, JS_DRSELECT);

	//prepare FUSE_CTS data.
	wrBuffer [0] = (unsigned char)((row<<3)| 0x3);		//Enable DMA, program and select row.
	wrBuffer [1] = bit;									//bit position
	wrBuffer [2] = 0x00;
	wrBuffer [3] = 0x00;
	//Magic word.
	wrBuffer [4] = 0xAC;
	wrBuffer [5] = 0x28;
	wrBuffer [6] = 0x8A;
	wrBuffer [7] = 0xA0;

	/* Step to CDR/SDR to shift in the command word
     * dma=1; pgm=1; a_row<4:0> & a_bit<4:0>
     * Continuously shift in MAGIC_CTS_WRITE
     */
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, wrBuffer, NULL, JS_DRSELECT);

	jtag_navigate (g_port, JS_DRCAPTURE);
	jtag_navigate (g_port, JS_DREXIT1);
	jtag_navigate (g_port, JS_DRUPDATE);

	//Go to RTI and stay in RTI EXACTLY Tpgm = 12 us (tbd) and immediately exit to SDS

	jtag_navigate (g_port, JS_IDLE);

	/* Toggle Clk after RTI */
	setPin (MIO_TCK, 0);
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);

	//Here we will be providing 12us delay.
		XilSKey_Efuse_SetTimeOut(&time, 110);
		while(1)
		{
			if(XilSKey_Efuse_IsTimerExpired(time) == 1)
				break;
		}

	jtag_navigate (g_port, JS_DRSELECT);
	/*
	 * After exit from RTI toggle Clk after entering DRSELECT state
	 * so programming will be disabled
	 */
		setPin (MIO_TCK, 0);
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);

	jtag_navigate (g_port, JS_IRSELECT);
	jtag_navigate (g_port, JS_RESET);
}


void JtagRead(unsigned char row, unsigned int * row_data, unsigned char marginOption)
{
    /* Following is the method to read FUSE register in Direct Macro Access way.
    Go to TLR to clear FUSE_CTS
    Load FUSE_CTS instruction on IR
    Step to CDR/SDR to shift in 32-bits FUSE_CTS command word
    a_row<4:0>;  dma=1;  pgm=0; tp_sel<1:0>; ecc_dma
    Shift in MAGIC_CTS_WRITE "A08A28AC"
    Step to E1DR/UDR to update FUSE_CTS reg
    Step to SDS/CDR/SDR to shift out captured row, while shifting in a new command with next row address w/ or w/o new tp_sel or ecc_dma setting
	Captured macro word (32 bits) is stored in jtag_dr[63:32]
	If ecc_dma = 1, jtag_dr[61:32] = {DED check-sum, SEC syndrome, decoded payload}
	*/
    unsigned char wrBuffer [8];
    unsigned char rdBuffer [8];
    unsigned char * row_data_ptr = (unsigned char *)row_data;
	u32 *WriteBuf32 = (u32 *)wrBuffer;

		// read 64-bit eFUSE dna
		//Go to TLR to clear FUSE_CTS
		jtag_navigate (g_port, JS_RESET);

	//Load FUSE_CTS instruction on IR
	jtag_setPreAndPostPads (g_port, 0, ZYNQ_DAP_IR_LENGTH, 0, 1);
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); // FUSE_CTS instruction
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, wrBuffer, NULL, JS_DRSELECT);
	//prepare FUSE_CTS data.

		wrBuffer [0] = (unsigned char)((row<<3)| 0x1);		//Enable DMA and select the row number.
		wrBuffer [1] = (unsigned char)(marginOption<<5);
		wrBuffer [2] = 0x00;
		wrBuffer [3] = 0x00;
		//Magic word.
		wrBuffer [4] = 0xAC;
		wrBuffer [5] = 0x28;
		wrBuffer [6] = 0x8A;
		wrBuffer [7] = 0xA0;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, wrBuffer, NULL, JS_DRSELECT);

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, NULL, rdBuffer, JS_DRSELECT);

		row_data_ptr[0] = rdBuffer [4];
		row_data_ptr[1] = rdBuffer [5];
		row_data_ptr[2] = rdBuffer [6];
		row_data_ptr[3] = rdBuffer [7];
}

/****************************************************************************/
/**
*
* This function validates MIO pins
*
* @return	0 value
*
* @note		none
*
*****************************************************************************/
int JtagValidateMioPins(XilSKey_ModuleSelection Module)
{

#ifdef XSK_ARM_PLATFORM
	(void) Module;
	/*
	 * Make sure that each every MIO pin defined is valid
	 */
	if (g_mio_jtag_tdi > 53) {
		return 1;
	}
	if (g_mio_jtag_tdo > 53) {
		return 1;
	}
	if (g_mio_jtag_tck > 53) {
		return 1;
	}
	if (g_mio_jtag_tms > 53) {
		return 1;
	}
	if (g_mio_jtag_mux_sel > 53) {
		return 1;
	}

	/*
	 *	Make sure that MIO pins defined for JTAG operation are unique among themselves
	 */
	if((g_mio_jtag_tdi == g_mio_jtag_tdo)||
	   (g_mio_jtag_tdi == g_mio_jtag_tck)||
	   (g_mio_jtag_tdi == g_mio_jtag_tms)||
	   (g_mio_jtag_tdi == g_mio_jtag_mux_sel))
		return 1;

	if((g_mio_jtag_tdo == g_mio_jtag_tck)||
	   (g_mio_jtag_tdo == g_mio_jtag_tms)||
	   (g_mio_jtag_tdo == g_mio_jtag_mux_sel))
		return 1;

	if((g_mio_jtag_tck == g_mio_jtag_tms)||
	   (g_mio_jtag_tck == g_mio_jtag_mux_sel))
		return 1;

	if(g_mio_jtag_tms == g_mio_jtag_mux_sel)
		return 1;
#else
	if (Module == XSK_EFUSE) {
		if (JtagValidateMioPins_Efuse_Ultra() != 0x00) {
			return 1;
		}
	}

	if (Module == XSK_BBRAM) {
		if (JtagValidateMioPins_Bbram_Ultra() != 0x00) {
			return 1;
		}
	}

#endif

	return 0;
}

int JtagServerInit(XilSKey_EPl *InstancePtr)
{
    int retval=0, i=0, num_taps=0, status=0;
    u32 *tap_codes = NULL;
    u32 index;

#ifdef XSK_ARM_PLATFORM
	g_mio_jtag_tdi		=	InstancePtr->JtagMioTDI;
	g_mio_jtag_tdo		= 	InstancePtr->JtagMioTDO;
	g_mio_jtag_tck		= 	InstancePtr->JtagMioTCK;
	g_mio_jtag_tms		=	InstancePtr->JtagMioTMS;
    g_mio_jtag_mux_sel	=	InstancePtr->JtagMioMuxSel;
    g_mux_sel_def_val   =   InstancePtr->JtagMuxSelLineDefVal;

    status = JtagValidateMioPins(XSK_EFUSE);
    if(status != 0)
    {
	return 1;
    }

    if((g_mux_sel_def_val != 0) && (g_mux_sel_def_val != 1))
    {
	return 1;
    }
#else
	GpioPinMasterJtagTDI = InstancePtr->JtagGpioTDI;
	GpioPinMasterJtagTDO = InstancePtr->JtagGpioTDO;
	GpioPinMasterJtagTMS = InstancePtr->JtagGpioTMS;
	GpioPinMasterJtagTCK = InstancePtr->JtagGpioTCK;

	GpioPinHwmReady = InstancePtr->HwmGpioReady;
	GpioPinHwmStart = InstancePtr->HwmGpioStart;
	GpioPinHwmEnd = InstancePtr->HwmGpioEnd;


	GpioInPutCh = InstancePtr->GpioInputCh;
	GpioOutPutCh = InstancePtr->GpioOutPutCh;
	GpioDeviceId = InstancePtr->JtagGpioID;

	status = JtagValidateMioPins(XSK_EFUSE);
	if(status != 0)
	{
	   return 1;
	}

#endif
    JtagInitGpio(XSK_EFUSE);

    g_js = js_init_zynq();
    if (g_js == NULL) {
		fprintf(stderr, "cannot create JTAG server\n");
		return 1;
    }

    if (js_open_port(g_js, g_useport, &g_port) < 0) {
	js_printf("%s\n", js_get_last_error(g_js));
	retval = 1;
	goto do_deinit;
    }

    if ((num_taps = js_detect_taps(g_port, &tap_codes)) < 0) {
	js_printf("detect_taps error: %s\n", js_get_last_error(g_js));
	retval = 1;
	goto do_deinit;
    }


    js_printf("idcodes: ");
    for (i = num_taps; i > 0; i--) {
	if (i != num_taps){
		js_printf(" ");
	}
	js_printf("%08x", (unsigned int)tap_codes[i - 1]);
    }
    js_printf("\n");

#ifndef XSK_ARM_PLATFORM

	/* This may be a ZU+ device, attempt to get the PL IDCODE */
	if ((num_taps == 1U) && (tap_codes[0] == 0x000000FFU)) {
		js_printf("\r\n*** Searching for ZU+ PL IDCODE ***\r\n");
		tap_codes[0] = JtagGetZuPlusPlIdcode();
	}

#endif

    //Check if one of the tap code is for Zynq DAP ID: 4ba00477
    //and mask out most significant nibble which represents Production Version

  for (i = 0; i < num_taps; i++) {
	for(index = 0; index < sizeof(IDcodeArray)/sizeof(IDcodeArray[0]); index++) {
		if( (tap_codes[i] & 0x0FFFFFFFU) == (IDcodeArray[index].id &
							0x0FFFFFFFU)) {
			InstancePtr->FpgaFlag = IDcodeArray[index].flag;
			InstancePtr->NumSlr    = IDcodeArray[index].numSlr;
			InstancePtr->MasterSlr = IDcodeArray[index].masterSlr;
			XilSKeyJtag.NumSlr    = IDcodeArray[index].numSlr;
			XilSKeyJtag.IrLen     = IDcodeArray[index].irLen;
			js_printf("Match. ID code: %08x\r\n", tap_codes[i]);
			js_printf("Match. numSlr:  %2d\r\n", XilSKeyJtag.NumSlr);
			js_printf("Match. irLen:   %2d\r\n", XilSKeyJtag.IrLen);
			goto IDSCAN_DONE;
			}
		}
		js_printf("DAP ID code: %08x no match for tap: %d\r\n", tap_codes[i], i);
	}

IDSCAN_DONE:

	if(i == num_taps)
	{
		retval = 1;
	}


    if (tap_codes){
		free(tap_codes);
    }
    return retval;

do_deinit:
	if (g_port != NULL)
		js_close_port(g_port);

	js_deinit_server(g_js);

	return retval;
}

/****************************************************************************/
/**
*
* This function initializes JTAG server for use in BBRAM algorithm
*
* @param  InstancePtr - instance pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*
*****************************************************************************/
int JtagServerInitBbram(XilSKey_Bbram *InstancePtr)
{
    int retval=0, i=0, num_taps=0, status=0;
    u32 *tap_codes = NULL;
    u32 index;
#ifdef XSK_ARM_PLATFORM
    g_mio_jtag_tdi		=	InstancePtr->JtagMioTDI;
    g_mio_jtag_tdo		= 	InstancePtr->JtagMioTDO;
    g_mio_jtag_tck		= 	InstancePtr->JtagMioTCK;
    g_mio_jtag_tms		=	InstancePtr->JtagMioTMS;
    g_mio_jtag_mux_sel	=	InstancePtr->JtagMioMuxSel;
    g_mux_sel_def_val   =   InstancePtr->JtagMuxSelLineDefVal;

    status = JtagValidateMioPins(XSK_BBRAM);
    if(status != 0)
    {
	return 1;
    }

    if((g_mux_sel_def_val != 0) && (g_mux_sel_def_val != 1))
    {
	return 1;
    }

#else
	GpioPinMasterJtagTDI = InstancePtr->JtagGpioTDI;
	GpioPinMasterJtagTDO = InstancePtr->JtagGpioTDO;
	GpioPinMasterJtagTMS = InstancePtr->JtagGpioTMS;
	GpioPinMasterJtagTCK = InstancePtr->JtagGpioTCK;

	GpioInPutCh = InstancePtr->GpioInputCh;
	GpioOutPutCh = InstancePtr->GpioOutPutCh;
	GpioDeviceId = InstancePtr->JtagGpioID;

	status = JtagValidateMioPins(XSK_BBRAM);
	if (status != 0) {
		js_printf("JtagValidateMioPins() failed\r\n");
	  return 1;
	}

#endif
    JtagInitGpio(XSK_BBRAM);

    g_js = js_init_zynq();
    if (g_js == NULL) {
		fprintf(stderr, "cannot create JTAG server\n");
		return 1;
    }

    if (js_open_port(g_js, g_useport, &g_port) < 0) {
	js_printf("%s\n", js_get_last_error(g_js));
	retval = 1;
	goto do_deinit;
    }

    if ((num_taps = js_detect_taps(g_port, &tap_codes)) < 0) {
	js_printf("detect_taps error: %s\n", js_get_last_error(g_js));
	retval = 1;
	goto do_deinit;
    }

    js_printf("idcodes: ");
    for (i = num_taps; i > 0; i--) {
	if (i != num_taps){
		js_printf(" ");
	}
	js_printf("%08x", (unsigned int)tap_codes[i - 1]);
    }
    js_printf("\n");

#ifndef XSK_ARM_PLATFORM

	/* This may be a ZU+ device, attempt to get the PL IDCODE */
	if ((num_taps == 1U) && (tap_codes[0] == 0x000000FFU)) {
		js_printf("\r\n*** Searching for ZU+ PL IDCODE ***\r\n");
		tap_codes[0] = JtagGetZuPlusPlIdcode();
	}

#endif

	//Check if one of the tap code is for Zynq DAP ID: 4ba00477
    //and mask out most significant nibble which represents Production version

  for (i = 0; i < num_taps; i++) {
	for(index = 0; index < sizeof(IDcodeArray)/sizeof(IDcodeArray[0]); index++) {
		if( (tap_codes[i] & 0x0FFFFFFFU) == (IDcodeArray[index].id &
						0x0FFFFFFFU)) {
			InstancePtr->FpgaFlag = IDcodeArray[index].flag;
			InstancePtr->NumSlr	  = IDcodeArray[index].numSlr;
			InstancePtr->MasterSlr = IDcodeArray[index].masterSlr;
			XilSKeyJtag.NumSlr    = IDcodeArray[index].numSlr;
			XilSKeyJtag.IrLen     = IDcodeArray[index].irLen;
			js_printf("Match. ID code: %08x\r\n", tap_codes[i]);
			js_printf("Match. numSlr:  %2d\r\n", XilSKeyJtag.NumSlr);
			js_printf("Match. irLen:   %2d\r\n", XilSKeyJtag.IrLen);
			goto IDSCAN_DONE;
			}
		}
		js_printf("DAP ID code: %08x no match for tap: %d\r\n", tap_codes[i], i);
	}

IDSCAN_DONE:

	if(i == num_taps)
	{
		retval = 1;
	}


    if (tap_codes){
		free(tap_codes);
    }
    return retval;

do_deinit:
	if (g_port != NULL)
		js_close_port(g_port);

	js_deinit_server(g_js);

	return retval;
}

/****************************************************************************/
/**
*
* This function implements the BBRAM algorithm initialization
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*
*****************************************************************************/
int Bbram_Init(void)
{
	u8 IRCaptureStatus = 0;
	u8 WriteBuffer[4];
	u64 Time = 0;
	u32 *WriteBuf32 = (u32 *)WriteBuffer;

	jtag_navigate (g_port, JS_RESET);
	jtag_navigate (g_port, JS_IDLE);

	/* Load bypass */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	*WriteBuf32 = calcInstr(BYPASS, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Load JPROGRAM
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	*WriteBuf32 = calcInstr(JPROGRAM, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Load ISC_NOOP
	 */
	*WriteBuf32 = calcInstr(ISC_NOOP, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Wait 100 msec
	 */
	Time = 0;

	XilSKey_Efuse_SetTimeOut(&Time, TIMER_100MS);
	while(1){
		if(XilSKey_Efuse_IsTimerExpired(Time) == 1)
		break;
	}

	/*
	 * Load ISC_NOOP
	 */
	*WriteBuf32 = calcInstr(ISC_NOOP, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			&IRCaptureStatus, JS_IDLE);

	/*
	 * Read IRCapture status and check for init_complete bit to be set
	 */
	if((IRCaptureStatus & INITCOMPLETEMASK) != INITCOMPLETEMASK){
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function implements the BBRAM program key
*
* @param  InstancePtr - instance pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*
*****************************************************************************/
int Bbram_ProgramKey(XilSKey_Bbram *InstancePtr)
{
	u32 KeyCnt;
	u8 WriteBuffer[4];
	u8 TckCnt;
	u32 *WriteBuf32 = (u32 *)WriteBuffer;

	/*
	 * Initial state - RTI
	 */
	jtag_navigate (g_port, JS_IDLE);

	/*
	 * Load ISC_ENABLE
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	*WriteBuf32 = calcInstr(ISC_ENABLE, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/*
	 * Shift 5 bits (0x15)
	 */
	WriteBuffer[0] = DR_EN;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_EN, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Wait 12 TCK
	 */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/*
	 * Load ISC_PROGRAM_KEY
	 */
	*WriteBuf32 = calcInstr(ISC_PROGRAM_KEY, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/*
	 * Shift 0xFFFFFFFF
	 */
	WriteBuffer[0] = 0xFF;
	WriteBuffer[1] = 0xFF;
	WriteBuffer[2] = 0xFF;
	WriteBuffer[3] = 0xFF;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
			&WriteBuffer[0], NULL, JS_IDLE);

	/*
	 * Wait 9 TCK
	 */
	for(TckCnt = 0; TckCnt < 9; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/*
	 * Load ISC_PROGRAM
	 */
	*WriteBuf32 = calcInstr(ISC_PROGRAM, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/*
	 * Shift 0xFFFFFFFF
	 */
	WriteBuffer[0] = 0xFF;
	WriteBuffer[1] = 0xFF;
	WriteBuffer[2] = 0xFF;
	WriteBuffer[3] = 0xFF;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
			&WriteBuffer[0], NULL, JS_IDLE);

	/*
	 * Wait 1 TCK
	 */
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);

	/*
	 * Program key - 32 bits at a time
	 */
	KeyCnt = 0;
	while(KeyCnt < NUMCHARINKEY){
		/*
		 * Load ISC_PROGRAM
		 */
		*WriteBuf32 = calcInstr(ISC_PROGRAM, CALC_SINGLE);
		jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
				NULL, JS_IRPAUSE);

		/*
		 * Copy key from Instance structure
		 */
		WriteBuffer[3] = InstancePtr->AESKey[KeyCnt++];
		WriteBuffer[2] = InstancePtr->AESKey[KeyCnt++];
		WriteBuffer[1] = InstancePtr->AESKey[KeyCnt++];
		WriteBuffer[0] = InstancePtr->AESKey[KeyCnt++];

		/*
		 * Shift 32 bit key
		 */
		jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
				&WriteBuffer[0], NULL, JS_IDLE);

		/*
		 * Wait 1 TCK
		 */
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/*
	 * Reset to IDLE
	 */
	jtag_navigate (g_port, JS_IDLE);

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function implements the BBRAM verify key.
* Program and verify key have to be done together;
* These API's cannot be used independently.
*
* @param  InstancePtr - instance pointer
*
* @return
*
*	- XST_FAILURE - In case of failure
*	- XST_SUCCESS - In case of Success
*
*
* @note
*
*****************************************************************************/
int Bbram_VerifyKey(XilSKey_Bbram *InstancePtr)
{
	u32 KeyCnt;
	u32 BufferCnt;
	u32 KeyCnt_Char;
	u32 ReadKey_Char;
	u8 WriteBuffer[5];
	u8 ReadBuffer[5];
	u8 TckCnt;
	unsigned long long DataReg = 0;
	int Status = XST_SUCCESS;
	u32 *WriteBuf32 = (u32 *)WriteBuffer;

	/*
	 * Initial state - RTI
	 */
	jtag_navigate (g_port, JS_IDLE);

	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	/*
	 * Load ISC_ENABLE
	 */
	*WriteBuf32 = calcInstr(ISC_ENABLE, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/*
	 * Shift 5 bits (0x15)
	 */
	WriteBuffer[0] = DR_EN;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_EN, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Wait 12 TCK
	 */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/*
	 * Load ISC_READ
	 */
	*WriteBuf32 = calcInstr(ISC_READ, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/*
	 * Shift 0x1FFFFFFFFF
	 */
	WriteBuffer[0] = 0xFF;
	WriteBuffer[1] = 0xFF;
	WriteBuffer[2] = 0xFF;
	WriteBuffer[3] = 0xFF;
	WriteBuffer[4] = 0x1F;

	/*
	 * Clear Read Buffer
	 */
	for(BufferCnt = 0; BufferCnt < 5; BufferCnt++){
		ReadBuffer[BufferCnt] = 0xFF;
	}

	/*
	 * Shift 37 bits and read 37 bits
	 */

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_VERIFY,
			WriteBuffer, ReadBuffer, JS_IDLE);

	/*
	 * Combine read bits
	 */
	DataReg = ReadBuffer[4];
	DataReg = DataReg << 8;
	DataReg = DataReg | ReadBuffer[3];
	DataReg = DataReg << 8;
	DataReg = DataReg | ReadBuffer[2];
	DataReg = DataReg << 8;
	DataReg = DataReg | ReadBuffer[1];
	DataReg = DataReg << 8;
	DataReg = DataReg | ReadBuffer[0];

	if((DataReg & DATAREGCLEAR) != DATAREGCLEAR){
		return XST_FAILURE;
	}

	/*
	 * Wait 9 TCK
	 */
	for(TckCnt = 0; TckCnt < 9; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/*
	 * Shift 37 bits for each 32 bits of key to be read.
	 * Consider 32 msb bits of the 37 bits read as key.
	 */
	KeyCnt = 0;
	while(KeyCnt < NUMWORDINKEY){
		/*
		 * Load ISC_READ
		 */
	  *WriteBuf32 = calcInstr(ISC_READ, CALC_SINGLE);
		jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
				NULL, JS_IRPAUSE);

		WriteBuffer[0] = 0xFF;
		WriteBuffer[1] = 0xFF;
		WriteBuffer[2] = 0xFF;
		WriteBuffer[3] = 0xFF;
		WriteBuffer[4] = 0x1F;
		/*
		 * Clear Read Buffer
		 */
		for(BufferCnt = 0; BufferCnt < 5; BufferCnt++){
			ReadBuffer[BufferCnt] = 0;
		}

		/*
		 * Shift 37 bits and read 37 bits
		 */
		jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_VERIFY,
				WriteBuffer, ReadBuffer, JS_IDLE);

		/*
		 * Wait 1 TCK
		 */
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);

		/*
		 * Combine the 8 bit array and shift out the 5 LSB bits
		 */
		DataReg = ReadBuffer[4];
		DataReg = DataReg << 8;
		DataReg = DataReg | ReadBuffer[3];
		DataReg = DataReg << 8;
		DataReg = DataReg | ReadBuffer[2];
		DataReg = DataReg << 8;
		DataReg = DataReg | ReadBuffer[1];
		DataReg = DataReg << 8;
		DataReg = DataReg | ReadBuffer[0];

		Bbram_ReadKey[KeyCnt++] = DataReg >> NUMLSBSTATUSBITS;

	}

	/*
	 * Compare read key with programmed key
	 */
	KeyCnt_Char = 0;
	for(KeyCnt = 0; KeyCnt < NUMWORDINKEY; KeyCnt++){

		ReadKey_Char = Bbram_ReadKey[KeyCnt];

		if((ReadKey_Char & 0xFF) !=
			InstancePtr->AESKey[KeyCnt_Char + 3]){
			Status = XST_FAILURE;
			break;
		}

		ReadKey_Char = ReadKey_Char >> 8;
		if((ReadKey_Char & 0xFF) !=
			InstancePtr->AESKey[KeyCnt_Char + 2]){
			Status = XST_FAILURE;
			break;
		}

		ReadKey_Char = ReadKey_Char >> 8;
		if((ReadKey_Char & 0xFF) !=
			InstancePtr->AESKey[KeyCnt_Char + 1]){
			Status = XST_FAILURE;
			break;
		}

		ReadKey_Char = ReadKey_Char >> 8;
		if((ReadKey_Char & 0xFF) !=
			InstancePtr->AESKey[KeyCnt_Char]){
			Status = XST_FAILURE;
			break;
		}

		KeyCnt_Char = KeyCnt_Char + 4;
	}

	/*
	 * Load ISC_DISABLE
	 */
	*WriteBuf32 = calcInstr(ISC_DISABLE, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Wait 12 TCK
	 */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	return Status;

}

/****************************************************************************/
/**
*
* This function does de-initialization
*
* @return none
*
* @note
*
*****************************************************************************/
void Bbram_DeInit(void)
{
	u8 WriteBuffer[5];
	u32 *WriteBuf32 = (u32 *)WriteBuffer;

	/*
	 * Load BYPASS
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER_BYP, IRTRAILER_BYP,
			DRHEADER_BYP, DRTRAILER_BYP);

	*WriteBuf32 = calcInstr(BYPASS, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	WriteBuffer[0] = IRDEINIT_L;
	WriteBuffer[1] = IRDEINIT_H;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRDEINITLEN, WriteBuffer,
			NULL, JS_IDLE);

	WriteBuffer[0] = DRDEINIT;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRDEINITLEN, WriteBuffer,
			NULL, JS_IDLE);

	jtag_navigate (g_port, JS_RESET);
	jtag_navigate (g_port, JS_IDLE);

	/*
	 * De-initialization
	 */
	if (g_port != NULL){
		js_close_port(g_port);
	}

	js_deinit_server(g_js);

}

/****************************************************************************/
/**
*
* This function reads temperature and voltage of Ultrascale
*
* @param	Row specifies the row number to read.
* @param	Row_Data is a pointer to a variable, to store the read value
*		of given row.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void Jtag_Read_Sysmon(u8 Row, u32 *Row_Data)
{

	u8 WriteBuf[4];
	u8 ReadBuf[4];
	u8 *DataPtr = (u8 *)Row_Data;
	u32 TckCnt;
	u32 *WriteBuf32 = (u32 *)WriteBuf;

	jtag_navigate (g_port, JS_RESET);

	*WriteBuf32 = calcInstr(SYSMON_DRP, CALC_MSTR);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuf,
						NULL, JS_IREXIT1);
	WriteBuf[3] = 0xC4;
	WriteBuf[2] = Row;
	WriteBuf[1] = 0x00;
	WriteBuf[0] = 0x00;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM, WriteBuf,
					NULL, JS_IDLE);
	/*
	 * Wait 12 TCK
	 * To ensure arbitration of sysmon DRP is fully resolved
	 */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM, NULL,
						ReadBuf, JS_IDLE);

	DataPtr[0] = ReadBuf[0];
	DataPtr[1] = ReadBuf[1];
}

/****************************************************************************/
/**
*
* This function blows the fuse of Ultrascale with provided parameters.
*
* @param	Row specifies the row number of EFUSE to blow.
* @param	Bit Specifies the bit location in the given row.
* @param	Page tell the page of EFUSE in which the given row is located.
* @param	Redundant is a flag to specify the bit to be programmed is
*		Normal bit or Redundant bit.
*		- Redundant	- XSK_EFUSEPL_REDUNDANT_ULTRA
*		- Normal	- XSK_EFUSEPL_NORMAL_ULTRA
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
int JtagWrite_Ultrascale(u8 Row, u8 Bit, u8 Page, u8 Redundant)
{

	u8 wrBuffer [8];
	u32 *WriteBuf32 = (u32 *)wrBuffer;
	int Status = XST_SUCCESS;

	jtag_navigate (g_port, JS_RESET);

	/* FUSE_CTS instruction */
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE);

	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, wrBuffer, NULL, JS_DRSELECT);

	/* Prepare FUSE_CTS data */
#ifndef DEBUG_FUSE_WRITE_DISABLE
		wrBuffer [0] = (((Bit & 0x1) << 7) | ((Row << 2)| 0x3));
#else
		wrBuffer [0] = (((Bit & 0x1) << 7) | ((Row << 2)| 0x1));
#endif
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		wrBuffer [1] = ((Bit >> 1) & 0xF) | ((0x20 << Redundant) |
				(Page << 4));
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		wrBuffer [1] = ((Bit >> 1) & 0xF) | (Page << 4);
		if (Redundant == 0x1U) {
			wrBuffer [1] = wrBuffer[1] | (0x8);
		}
	}
	wrBuffer [2] = 0x00;
	wrBuffer [3] = 0x00;
	/* Magic word */
	wrBuffer [4] = 0xAC;
	wrBuffer [5] = 0x28;
	wrBuffer [6] = 0xED;
	wrBuffer [7] = 0xFE;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, wrBuffer, NULL, JS_DRSELECT);

	jtag_navigate (g_port, JS_DRCAPTURE);
	jtag_navigate (g_port, JS_DREXIT1);
	jtag_navigate (g_port, JS_DRUPDATE);

	/* Handing over to hardware module */
	XilSKey_Efuse_StartTimer();

	while (!readPin(GPIO_HWM_READY)) {
		/* Wait for hardware module ready signal to go high */
		if (XilSKey_Efuse_IsTimerExpired(TimerTicksfor1000ns) == 1) {
			Status = XST_FAILURE;
			goto SKIP_HARDWARE;
		}
	}
	/* Enable hardware module program start signal (allow hardware to take over from here) */
	setPin(GPIO_HWM_START, 1);

	XilSKey_Efuse_StartTimer();
	while (!readPin(GPIO_HWM_END)) {
	/* Wait for hardware module program end signal to go high */
		if (XilSKey_Efuse_IsTimerExpired(TimerTicksfor1000ns) == 1) {
			Status = XST_FAILURE;
			goto SKIP_HARDWARE;
		}
	}

SKIP_HARDWARE:
	/* Disable hardware module program start signal */
	setPin(GPIO_HWM_START, 0);

	jtag_navigate (g_port, JS_RESET);

	return Status;

}

/****************************************************************************/
/**
*
* This function reads entire row of Ultrascale's EFUSE.
*
* @param	Row specifies the row number of EFUSE.
* @param	MarginOption is a variable which tells the margin option in
*		which read operation to be performed.
* @param	Page tell the page of EFUSE in which the given row is located.
* @param	Redundant is a flag to specify the bit to be programmed is
*		Normal bit or Redundant bit.
*		- Redundant	- XSK_EFUSEPL_REDUNDANT_ULTRA
*		- Normal	- XSK_EFUSEPL_NORMAL_ULTRA
*		This variable is not valid for Ultrascale plus as each row
*		contains both redundant and normal bits upper 16 are redundant.
*
* @return	None.
*
* @note		Method to read FUSE register in Direct Macro Access way.
*		Go to TLR to clear FUSE_CTS
*		Load FUSE_CTS instruction on IR
*		Step to CDR/SDR to shift in 32-bits FUSE_CTS command word
*		Shift in MAGIC_CTS_WRITE "FEED28AC" for ultrascale
*		Read 64 bit data in IR read state.
*
*****************************************************************************/
void JtagRead_Ultrascale(u8 Row, u32 *RowData, u8 MarginOption,
						u8 Page, u8 Redundant)
{

	u8 WrBuffer [8];
	u8 RdBuffer [8]= {0};
	u8 *RowDataPtr = (u8 *)RowData;
	u32 *WriteBuf32 = (u32 *)WrBuffer;

	jtag_navigate (g_port, JS_RESET);

	/* Load FUSE_CTS instruction on IR */
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuffer, NULL, JS_DRSELECT);

	/*prepare FUSE_CTS data. */
	WrBuffer [0] = ((Row << 2)| 0x1);	/* Select the row number */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		WrBuffer [1] = (0x20 << Redundant) | (Page << 4);
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		WrBuffer [1] = (Page << 4);
	}
				/* Page and Redundant/normal bit selection */
	WrBuffer [2] = MarginOption;
	WrBuffer [3] = 0x00;
	/* Magic word. */
	WrBuffer [4] = 0xAC;
	WrBuffer [5] = 0x28;
	WrBuffer [6] = 0xED;
	WrBuffer [7] = 0xFE;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, WrBuffer, NULL, JS_IDLE);

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, NULL, RdBuffer, JS_IDLE);

	/*
	 * Captured macro word (32 Bits) is stored in [63:32] of
	 * 64 bit buffer
	 */
	RowDataPtr[0] = RdBuffer[4];
	RowDataPtr[1] = RdBuffer[5];
	RowDataPtr[2] = RdBuffer[6];
	RowDataPtr[3] = RdBuffer[7];

}

/****************************************************************************/
/**
*
* This function reads the status row of Ultrascale's EFUSE and updates the
* pointer.
*
* @param	Rowdata is a pointer to a 32 bit variable which stores the
*		status register value read from EFUSE status register.

* @return	None.
*
* @note		Method to read FUSE register in Direct Macro Access way.
*		For reading the status values we need send the bitstream
*		in shift DR state.
*		Shift in MAGIC_CTS_WRITE "FEED28AC" for ultrascale
*		Read 64 bit data in IR read state.
*
*****************************************************************************/
void JtagRead_Status_Ultrascale(u32 *Rowdata)
{
	u8 WrBuffer[32];
	u8 RdBuffer [4];
	u8 *RowDataPtr = (u8 *)Rowdata;
	u32 *WriteBuf32 = (u32 *)WrBuffer;

	/* Go to TLR to clear FUSE_CTS */
	jtag_navigate (g_port, JS_RESET);

	/* CFG_IN instruction */
	*WriteBuf32 = calcInstr(CFG_IN_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuffer, NULL, JS_DRSELECT);

	/* prepare Bit stream data */
	WrBuffer[0] = 0xff;
	WrBuffer[1] = 0xff;
	WrBuffer[2] = 0xff;
	WrBuffer[3] = 0xff;

	WrBuffer[4] = 0x55;
	WrBuffer[5] = 0x99;
	WrBuffer[6] = 0xAA;
	WrBuffer[7] = 0x66;

	WrBuffer[8] = 0x0c;
	WrBuffer[9] = 0x00;
	WrBuffer[10] = 0x01;
	WrBuffer[11] = 0x80;

	WrBuffer[12] = 0x0c;
	WrBuffer[13] = 0x41;
	WrBuffer[14] = 0x04;
	WrBuffer[15] = 0xc9;

	WrBuffer [16] = 0x04;
	WrBuffer [17] = 0x00;
	WrBuffer [18] = 0x00;
	WrBuffer [19] = 0x00;

	WrBuffer [20] = 0x14;
	WrBuffer [21] = 0xc0;
	WrBuffer [22] = 0x05;
	WrBuffer [23] = 0x80;

	WrBuffer [24] = 0x04;
	WrBuffer [25] = 0x00;
	WrBuffer [26] = 0x00;
	WrBuffer [27] = 0x00;

	WrBuffer [28] = 0x04;
	WrBuffer [29] = 0x00;
	WrBuffer [30] = 0x00;
	WrBuffer [31] = 0x00;

	jtag_shift (g_port, ATOMIC_DR_SCAN, 256, WrBuffer, NULL, JS_IDLE);

	*WriteBuf32 = calcInstr(CFG_OUT_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuffer, NULL, JS_DRSELECT);

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM, NULL, RdBuffer, JS_IDLE);

	RowDataPtr[0] = RdBuffer [0];
	RowDataPtr[1] = RdBuffer [1];
	RowDataPtr[2] = RdBuffer [2];
	RowDataPtr[3] = RdBuffer [3];

	*(u32 *)RowDataPtr = XilSKey_Efuse_ReverseHex(*(u32 *)RowDataPtr);

}

/****************************************************************************/
/**
*
* This function verifies the AES key of Ultrascale's EFUSE with provided CRC
* value.
*
* @param	Crc is a pointer to a 32 bit variable which holds the expected
*		AES key's CRC.
* @param	MarginOption is a variable which tells the margin option in
*		which read operation to be performed.
*
* @return	Returns XST_FAILURE/XST_SUCCESS
*		- XST_SUCCESS - If CRC is correct
*		- XST_FAILURE - If CRC is wrong
*
* @note		To verify AES key with provided CRC 256 bits need to be written
*		with all ZEROS and last 32 bits with CRC.
*		And then CTS word should be framed and read 9 rows.
*		The keys are stored in 8 rows (20 to 27) but in UltraScale
*		the device will read all 1s.
*		On the 9th read (row 28), the CRC computation takes
*		place. An extra read after the 9th read is required
*		to read the computed CRC out (can use last row addr
*		of 28 or row 0). The FPGA internally compares the
*		computed CRC with the expected CRC loaded through
*		the FUSE_KEY operation. If they match, then the expected CRC
*		is read. If not, the FPGA will return all 1s.
*
*****************************************************************************/
u32 JtagAES_Check_Ultrascale(u32 *Crc, u8 MarginOption)
{
	u8 WrBuffer [32];
	u8 RdBuffer [8];
	u8 *RowDataPtr = (u8 *)Crc;
	u32 Row;
	u32 Index;
	u8 WrBuf[8];
	u8 AesRowStart;
	u8 AesRowEnd;
	u32 *WriteBuf32 = (u32 *)WrBuf;

	/* Go to TLR to clear FUSE_CTS */
	jtag_navigate (g_port, JS_RESET);

	/* Load FUSE_CTS instruction on IR */
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuf, NULL, JS_DRSELECT);

	/* Prepare FUSE_CTS data. */
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		WrBuffer [0] = 0x00;
		WrBuffer [1] = 0x80;
		WrBuffer [2] = MarginOption;
		WrBuffer [3] = 0x01;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		WrBuffer [0] = 0x00;
		WrBuffer [1] = 0x00;
		WrBuffer [2] = 0x01 | (MarginOption << 1);
		WrBuffer [3] = 0x80;
	}
	/* Magic word. */
	WrBuffer [4] = 0xAC;
	WrBuffer [5] = 0x28;
	WrBuffer [6] = 0xED;
	WrBuffer [7] = 0xFE;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, WrBuffer, NULL, JS_IDLE);

	*WriteBuf32 = calcInstr(FUSE_KEY_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuf, NULL, JS_DRSELECT);


	WrBuffer[0] = RowDataPtr[0];
	WrBuffer[1] = RowDataPtr[1];
	WrBuffer[2] = RowDataPtr[2];
	WrBuffer[3] = RowDataPtr[3];
	for (Index = 4; Index < 32; Index++) {
		WrBuffer[Index] = 0;
	}

	jtag_shift (g_port, ATOMIC_DR_SCAN, 256, WrBuffer, NULL, JS_IDLE);
	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuf, NULL, JS_DRSELECT);


	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
		WrBuffer [0] = 0x01;
		WrBuffer [1] = 0x00;
		WrBuffer [2] = MarginOption;
		WrBuffer [3] = 0x01;
		AesRowStart = 20;
		AesRowEnd = 29;
	}
	if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
		WrBuffer [0] = 0x01;
		WrBuffer [1] = 0x00;
		WrBuffer [2] = 0x01 | (MarginOption << 1);
		WrBuffer [3] = 0x80;
		AesRowStart = 5;
		AesRowEnd = 22;
	}
	/* Magic word. */
	WrBuffer [4] = 0xAC;
	WrBuffer [5] = 0x28;
	WrBuffer [6] = 0xED;
	WrBuffer [7] = 0xFE;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, WrBuffer, NULL, JS_IDLE);
	/*
	 * Repeat for all AES key Rows 20-27 and repeat one extra read
	 * for getting CRC back.
	 */
	for (Row = AesRowStart; Row < AesRowEnd; Row++) {
		*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
		jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuf, NULL, JS_DRSELECT);

		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA) {
			WrBuffer [0] = (Row << 2) | 0x1;/* Select row number */
			WrBuffer [1] = 0x00;
			WrBuffer [2] = MarginOption;
			WrBuffer [3] = 0x01;
		}
		if (PlFpgaFlag == XSK_FPGA_SERIES_ULTRA_PLUS) {
			WrBuffer [0] = (Row << 2) | 0x1;/* Select row number */
			WrBuffer [1] = 0x00;
			WrBuffer [2] = 0x01 | (MarginOption << 1);
			WrBuffer [3] = 0x80;
		}
		/* Magic word */
		WrBuffer [4] = 0xAC;
		WrBuffer [5] = 0x28;
		WrBuffer [6] = 0xED;
		WrBuffer [7] = 0xFE;
		jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, WrBuffer,
						RdBuffer, JS_IDLE);
	}

	*WriteBuf32 = calcInstr(FUSE_CTS_SLRX, CALC_SINGLE); /* FUSE_CTS instruction */
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WrBuf, NULL, JS_DRSELECT);

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_FUSE, WrBuffer, RdBuffer, JS_IDLE);
	if ((RowDataPtr[0] == RdBuffer[4]) &&
		(RowDataPtr[1] == RdBuffer[5]) &&
		(RowDataPtr[2] == RdBuffer[6]) &&
		(RowDataPtr[3] == RdBuffer[7])) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;

}

#ifdef XSK_MICROBLAZE_PLATFORM
/****************************************************************************/
/**
*
* This function implements the UltraScale's BBRAM algorithm initialization
*
* @return
*		- XST_FAILURE - In case of failure
*		- XST_SUCCESS - In case of Success
*
* @note		None.
*
*****************************************************************************/
int Bbram_Init_Ultra(void)
{
	u8 WriteBuffer[4];
	u32 *WriteBuf32 = (u32 *)WriteBuffer;
	u64 Time = 0;

	jtag_navigate (g_port, JS_RESET);
	jtag_navigate (g_port, JS_IDLE);

	/* Load ISC_NOOP */
		*WriteBuf32 = calcInstr(ISC_NOOP, CALC_ALL);
		jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
				NULL, JS_IDLE);

	/* Wait 100 msec */
	Time = XSK_EFUSEPL_CLCK_FREQ_ULTRA/10000;

	XilSKey_Efuse_StartTimer();

	if (XilSKey_Efuse_GetTime() < Time) {
		while(1) {
			if(XilSKey_Efuse_IsTimerExpired(Time) == 1) {
				break;
			}
		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function implements the Ultrascale's BBRAM program key
*
* @param	BBRAM instance pointer
*
* @return
*		- XST_FAILURE - In case of failure
*		- XST_SUCCESS - In case of Success
*
* @note		None.
*
*****************************************************************************/
int Bbram_ProgramKey_Ultra(XilSKey_Bbram *InstancePtr)
{
	u32 KeyCnt;
	u8 WriteBuffer[4];
	u8 TckCnt;
	u32 *WriteBuf32 = (u32 *)WriteBuffer;

	js_printf("Entered. SLRNUM: %d\r\n", XilSKeyJtag.CurSlr);

	/* Initial state - RTI */
	jtag_navigate (g_port, JS_IDLE);

	/* Load ISC_ENABLE */
	*WriteBuf32 = calcInstr(ISC_ENABLE_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/* Shift 5 bits (0x15) */
	WriteBuffer[0] = DR_EN;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_EN, WriteBuffer,
			NULL, JS_IDLE);

	/* Wait 12 TCK */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/* second ISC enable data shift */
	WriteBuffer[0] = DR_EN;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_EN, WriteBuffer,
				NULL, JS_IDLE);

	/* Wait 12 TCK */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	jtag_navigate (g_port, JS_IDLE);

	/* Load XSC_PROGRAM_SLRx */
	*WriteBuf32 = calcInstr(XSC_PROGRAM_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/* Shift 0xFFFFFFFF */
	*WriteBuf32 = 0xFFFFFFFF;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
			&WriteBuffer[0], NULL, JS_IDLE);

	/* Wait 9 TCK */
	for(TckCnt = 0; TckCnt < 9; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/* Load ISC_PROGRAM_SLRx */
	*WriteBuf32 = calcInstr(ISC_PROGRAM_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);

	/* Shift 0x0000557B Control word */
	*WriteBuf32 = InstancePtr->CtrlWord;
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
			&WriteBuffer[0], NULL, JS_IDLE);

	/* Wait 1 TCK */
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);

	/* Program key - 32 bits at a time */
	KeyCnt = 31;
	while (KeyCnt < NUMCHARINKEY) {
		/* Load ISC_PROGRAM_SLRx */
		*WriteBuf32 = calcInstr(ISC_PROGRAM_SLRX, CALC_SINGLE);
		jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
				NULL, JS_IRPAUSE);

		/* Copy key from Instance structure */
		WriteBuffer[3] = InstancePtr->AESKey[KeyCnt--];
		WriteBuffer[2] = InstancePtr->AESKey[KeyCnt--];
		WriteBuffer[1] = InstancePtr->AESKey[KeyCnt--];
		WriteBuffer[0] = InstancePtr->AESKey[KeyCnt--];

		/* Shift 32 bit key */
		jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
				&WriteBuffer[0], NULL, JS_IDLE);

		/* Wait 1 TCK */
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/* Write CRC of BBRAM key */
	*WriteBuf32 = calcInstr(ISC_PROGRAM_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IRPAUSE);
	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM,
			(u8 *)&(InstancePtr->Crc), NULL, JS_IDLE);

	/* Wait 1 TCK */
	setPin (MIO_TCK, 1);
	setPin (MIO_TCK, 0);

	/* Reset to IDLE */
	jtag_navigate (g_port, JS_IDLE);

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function does de-initialization of UltraScale.
*
* @param 	none
*
* @return	none
*
* @note		none
*
*****************************************************************************/
void Bbram_DeInit_Ultra(void)
{
	u8 WriteBuffer[5];
	u32 *WriteBuf32 = (u32 *)WriteBuffer;
	u32 TckCnt;

	jtag_navigate (g_port, JS_IDLE);

	/* ISC_PROG_SECURITY */
	*WriteBuf32 = calcInstr(XSC_PROGRAM_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);
	WriteBuffer[0] = 0x00;
	WriteBuffer[1] = 0x00;
	WriteBuffer[2] = 0x00;
	WriteBuffer[3] = 0x00;

	jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM, WriteBuffer,
						NULL, JS_IDLE);
	for (TckCnt = 0; TckCnt < 8; TckCnt++) {
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}
}
#endif

/****************************************************************************/
/**
*
* This function close  UltraScale.
*
* @return	none
*
* @note		none
*
*****************************************************************************/
void Bbram_Close_Ultra(void)
{
	u8 WriteBuffer[4];
	u32 *WriteBuf32 = (u32 *)WriteBuffer;
	u32 TckCnt;

	/* Load ISC_DISABLE */
	*WriteBuf32 = calcInstr(ISC_DISABLE, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	/* Wait 12 TCK */
	for(TckCnt = 0; TckCnt < 12; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}
	jtag_navigate (g_port, JS_IDLE);

	for(TckCnt = 0; TckCnt < 10; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	jtag_navigate (g_port, JS_IDLE);

	for(TckCnt = 0; TckCnt < 6; TckCnt++){
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	/* BYPASS */
	*WriteBuf32 = calcInstr(BYPASS, CALC_ALL);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
				NULL, JS_IDLE);

	jtag_navigate (g_port, JS_RESET);
	jtag_navigate (g_port, JS_IDLE);

	/* De-initialization */
	if (g_port != NULL){
		js_close_port(g_port);
	}

	js_deinit_server(g_js);

}

#ifdef XSK_MICROBLAZE_PLATFORM
/****************************************************************************/
/**
*
* This function verifies the programmed BBRAM by reading CRC of programmed
* AES key and control word.
*
* @param 	none
*
* @return	none
*
* @note		BBRAM verification cannot be done separately,
*		The program and verify will only work together in and in
*		that order.
*
*****************************************************************************/
int Bbram_VerifyKey_Ultra(u32 *Crc32)
{
	u8 WriteBuffer[5];
	u32 *WriteBuf32 = (u32 *)WriteBuffer;
	u64 ReadBuffer;
	int Status = XST_SUCCESS;
	u32 Num;

	/* Initial state - RTI */
	jtag_navigate (g_port, JS_IDLE);

	/* Load ISC_READ */
	*WriteBuf32 = calcInstr(ISC_READ_SLRX, CALC_SINGLE);
	jtag_shift (g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuffer,
			NULL, JS_IDLE);

	for (Num = 0; Num < 10; Num++) {
		jtag_shift (g_port, ATOMIC_DR_SCAN, DRLENGTH_VERIFY, NULL,
				(u8 *)&ReadBuffer, JS_IDLE);
		setPin (MIO_TCK, 1);
		setPin (MIO_TCK, 0);
	}

	ReadBuffer = ReadBuffer >> 5;
	if (*Crc32 != (u32)ReadBuffer) {
		return XST_FAILURE;
	}

	return Status;

}
#endif

/****************************************************************************/
/**
*
* This function validates GPIO pin connections of Master JTAG and hardware
* module for eFUSE programming.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
#ifndef XSK_ARM_PLATFORM
static INLINE int JtagValidateMioPins_Efuse_Ultra(void)
{

	/*
	 * Make sure that each every AXI GPIO pin defined is valid
	 */
	if (GpioPinMasterJtagTDI > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTDO > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTMS > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTCK > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinHwmStart > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinHwmEnd > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinHwmReady > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}

	/*
	 * Make sure that provided channel numbers of GPIO is valid
	 */
	if ((GpioInPutCh < XSK_EFUSEPL_GPIO_CH1) ||
			(GpioInPutCh > XSK_EFUSEPL_GPIO_CH2)) {
		return XST_FAILURE;
	}
	if ((GpioOutPutCh < XSK_EFUSEPL_GPIO_CH1) ||
			(GpioOutPutCh > XSK_EFUSEPL_GPIO_CH2)) {
		return XST_FAILURE;
	}
	/*
	 * Make sure that GPIO pins defined for JTAG operation are
	 * unique among themselves
	 */

	/* If both input and output channels is same */
	if (GpioInPutCh == GpioOutPutCh) {
		if((GpioPinMasterJtagTDI == GpioPinMasterJtagTDO)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTCK)||
			(GpioPinMasterJtagTDI == GpioPinHwmStart) ||
			(GpioPinMasterJtagTDI == GpioPinHwmEnd) ||
			(GpioPinMasterJtagTDI == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTDO == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDO == GpioPinMasterJtagTCK) ||
			(GpioPinMasterJtagTDO == GpioPinHwmStart) ||
			(GpioPinMasterJtagTDO == GpioPinHwmEnd) ||
			(GpioPinMasterJtagTDO == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTMS == GpioPinMasterJtagTCK) ||
			(GpioPinMasterJtagTMS == GpioPinHwmStart) ||
			(GpioPinMasterJtagTMS == GpioPinHwmEnd) ||
			(GpioPinMasterJtagTMS == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if ((GpioPinMasterJtagTCK == GpioPinHwmStart) ||
			(GpioPinMasterJtagTCK == GpioPinHwmEnd) ||
			(GpioPinMasterJtagTCK == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if ((GpioPinHwmStart == GpioPinHwmEnd) ||
			(GpioPinHwmStart == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if (GpioPinHwmEnd == GpioPinHwmReady) {
			return XST_FAILURE;
		}
	}
	else {
		/* Signals as output to GPIO */
		if((GpioPinMasterJtagTDI == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTCK) ||
			(GpioPinMasterJtagTDI == GpioPinHwmStart)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTMS == GpioPinMasterJtagTCK) ||
			(GpioPinMasterJtagTMS == GpioPinHwmStart)) {
			return XST_FAILURE;
		}
		if (GpioPinMasterJtagTCK == GpioPinHwmStart) {
			return XST_FAILURE;
		}

		/* Signals as input to GPIO */
		if ((GpioPinMasterJtagTDO == GpioPinHwmEnd) ||
			(GpioPinMasterJtagTDO == GpioPinHwmReady)) {
			return XST_FAILURE;
		}
		if (GpioPinHwmEnd == GpioPinHwmReady) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function validates GPIO pin connections of Master JTAG and hardware
* module for BBRAM programming.
*
* @return	None.
*
* @note		Hardware module is not required over here.
*
*****************************************************************************/
static INLINE int JtagValidateMioPins_Bbram_Ultra(void)
{
	/*
	 * Make sure that each every AXI GPIO pin defined is valid
	 */
	if (GpioPinMasterJtagTDI > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTDO > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTMS > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}
	if (GpioPinMasterJtagTCK > XSK_GPIO_PIN_MAX) {
		return XST_FAILURE;
	}

	/*
	 * Make sure that provided channel numbers of GPIO is valid
	 */
	if ((GpioInPutCh < XSK_EFUSEPL_GPIO_CH1) ||
			(GpioInPutCh > XSK_EFUSEPL_GPIO_CH2)) {
		return XST_FAILURE;
	}
	if ((GpioOutPutCh < XSK_EFUSEPL_GPIO_CH1) ||
			(GpioOutPutCh > XSK_EFUSEPL_GPIO_CH2)) {
		return XST_FAILURE;
	}
	/*
	 * Make sure that GPIO pins defined for JTAG operation are
	 * unique among themselves
	 */
	/* If both input and output channels is same */
	if (GpioInPutCh == GpioOutPutCh) {
		if((GpioPinMasterJtagTDI == GpioPinMasterJtagTDO)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTCK)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTDO == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDO == GpioPinMasterJtagTCK)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTMS == GpioPinMasterJtagTCK)) {
			return XST_FAILURE;
		}
	}
	else {
		if((GpioPinMasterJtagTDI == GpioPinMasterJtagTMS)||
			(GpioPinMasterJtagTDI == GpioPinMasterJtagTCK)) {
			return XST_FAILURE;
		}
		if((GpioPinMasterJtagTMS == GpioPinMasterJtagTCK)) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function attempt to get the ZU+ PL_IDCODE via JTAG.
*
* @return	ZU+ PL IDCODE if it exists
*
* @note		None.
*
*****************************************************************************/
static INLINE u32 JtagGetZuPlusPlIdcode(void)
{
	u8 WriteBuf[4];
	u8 ReadBuf[4];
	u32 *WriteBuf32 = (u32 *)WriteBuf;
	u32 *ReadBuf32 = (u32 *)ReadBuf;
	u32 IdCode;

	/* Set zu+ pl tap parameters */
	XilSKeyJtag.NumSlr = 1U;
	XilSKeyJtag.IrLen = TAP_IR_LENGTH;

	/* Reset the jtag tap */
	jtag_navigate (g_port, JS_RESET);

	/* Write zu+ pl_idcode instruction into the jtag ir */
	*WriteBuf32 = calcInstr(ZUPLUS_PL_IDCODE, CALC_MSTR);
	jtag_shift(g_port, ATOMIC_IR_SCAN, XilSKeyJtag.IrLen, WriteBuf,
						NULL, JS_IREXIT1);

	/* Get the zu+ pl_idcode from the jtag dr */
	jtag_shift(g_port, ATOMIC_DR_SCAN, DRLENGTH_PROGRAM, NULL,
						ReadBuf, JS_IDLE);

	IdCode = *(u32 *)ReadBuf32;

	js_printf("\r\n *** ZU+ PL_IDCODE Read: "
			  "0x%08X ***\r\n", IdCode);

	return IdCode;
}

#endif
