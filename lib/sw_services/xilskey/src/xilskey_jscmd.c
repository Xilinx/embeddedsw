/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* </pre>
*
*
******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include <Windows.h>

#if defined(_MSC_VER)
#  pragma warning(disable:4996) /* 'strcpy': This function or variable may be unsafe */
#endif

typedef long ssize_t;
#else
#include <unistd.h>
#endif
#include "xilskey_utils.h"
#include "xilskey_jslib.h"
#include "xilskey_jscmd.h"
#include "xgpiops.h"
#include "xilskey_bbram.h"

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define js_printf		printf
#else
void dummy_printf(const char *ctrl1, ...);
#define js_printf		dummy_printf
#endif

#define DEFAULT_FREQUENCY 10000000
#define MAX_FREQUENCY 30000000
#define ZYNQ_DAP_ID 0x4ba00477

#define set_last_error(JS, ...) js_set_last_error(&(JS)->js.base, __VA_ARGS__)

typedef struct js_port_impl_struct js_port_impl_t;
typedef struct js_command_impl_struct js_command_impl_t;
typedef struct ftd_async_transfer_struct ftd_async_transfer_t;

static js_port_t *g_port = NULL;
static js_server_t *g_js = NULL;
static js_port_descr_t *g_useport = NULL;
extern u32 TimerTicksfor100ns;

void dummy_printf(const char *ctrl1, ...)
{
	return;
}
static int close_port(
    js_lib_port_t *port_arg);


struct js_command_impl_struct {
    /* Command to execute */
    js_lib_command_t lib;

    /* Intermediatre command processing state */
    size_t byte_offset;
    js_state_t start_state;
    js_state_t end_state;
    unsigned int write_bytes;
    unsigned int read_bytes;
    unsigned int partial_bytes;
    int state_bits;
};


struct js_zynq {
    /* Base class - must be first. */
    js_lib_server_t js;

    /* Port list */
    js_port_descr_t *port_list;
    unsigned int port_count;
    unsigned int port_max;
};


struct js_port_impl_struct {
    /* Base class - must be first. */
    js_lib_port_t lib;

    /* Command sequence after normalization */
    js_lib_command_sequence_t *cmdseq;

    /* Current JTAG state */
    js_state_t state;

    int irPrePadBits;
    int irPostPadBits;
    int drPrePadBits;
    int drPostPadBits;

    js_node_t root_node_obj;
};



static XGpioPs structXGpioPs;

int setPin (int pin, int value);
int readPin (int pin);

u32 Bbram_ReadKey[8];

void GpioConfig(unsigned long addr, unsigned long mask, unsigned long val)
{
	unsigned long current_val = *(volatile unsigned long *)addr;
	*(volatile unsigned long *) addr = ( val & mask ) | ( current_val  & ~mask);
}

void JtagInitGpio ()
{
	js_printf("===== Initializing PS GPIO pins...\n\r");
	XGpioPs_Config *ptrConfigPtrPs = XGpioPs_LookupConfig(0);
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
} /* initGpio() */

int getByteCountFromBitCount (int bitCount)
{
   int byteCount = bitCount / 8;
   if (bitCount % 8)
   {
      byteCount++;
   }
   return (byteCount);
}

// File generated by JTAG.EXE. This are the tms values and
// the corresponding bit counts to navigate quickly from any state
// to any state.
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

unsigned int getNavigateTAPValue (unsigned char startState, unsigned char endState)
{
    unsigned char index;
    unsigned int tableValue;

    index = (startState << 4) & 0xF0;
    index |= endState;

    tableValue = JTAGNavigateTable [index];

    return (tableValue);
}

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
	XGpioPs_WritePin(&structXGpioPs, pin, value);
	return (status);
}

int readPin (int pin)
{
	int retVal = XGpioPs_ReadPin(&structXGpioPs, pin);
	return (retVal);
}

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

// This function will be used to shift long bits. Returns a 1 if shift state is to be exited.
// If tdoBuf != NULL, data will be read from device.
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

	if(flags | JS_ONES)
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

int jtag_navigate (js_port_t *port, js_state_t state)
{
	int status = 0;
	js_command_sequence_t *cmds = js_create_command_sequence(port->root_node);
	if (cmds == NULL)
	{
		return -1;
	}
	js_add_state_change(cmds, state, 0);
	status = js_run_command_sequence(cmds);
	js_delete_command_sequence(cmds);
	return (status);
}

// This function takes care of padding
// Must be set up with pre/post ir/dr info

int jtag_shift (js_port_t *port_arg, unsigned char mode, int bits, unsigned char* wrBuffer, unsigned char* rdBuffer, js_state_t state)
{
	int status = 0;
	unsigned char* rdPtr;
	unsigned char* wrPtr;

	js_command_sequence_t *cmds = js_create_command_sequence(port_arg->root_node);
	if (cmds == NULL)
	{
		return -1;
	}

	js_port_impl_t *port = (js_port_impl_t *)port_arg;

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


static int get_property(
    js_lib_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t *valuep)
{
    js_port_impl_t *port = (js_port_impl_t *)port_arg;

    js_set_last_error(port->lib.base.server, "unsupported property");
    return -1;
}


static int set_property(
    js_lib_port_t *port_arg,
    js_property_kind_t kind,
    js_property_value_t value)
{
    js_port_impl_t *port = (js_port_impl_t *)port_arg;

    js_set_last_error(port->lib.base.server, "unsupported property");
    return -1;
}

static int run_command_sequence(
    js_lib_command_sequence_t *cmds)
{
    js_port_impl_t *port = (js_port_impl_t *)cmds->base.node->port;
    js_command_impl_t *cmd_start;
    js_command_impl_t *cmd_end;

    if (js_clear_command_sequence(&port->cmdseq->base) < 0 ||
        js_lib_normalize_command_sequence(port->cmdseq, cmds) < 0)
        return -1;

    cmd_start = (js_command_impl_t *)port->cmdseq->cmd_list;
    cmd_end = cmd_start + port->cmdseq->cmd_count;

    js_command_impl_t *cmd = cmd_start;
    js_state_t state = port->state;

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

static int get_port_descr_list(
    js_lib_server_t *server,
    js_port_descr_t **port_listp)
{
	return 1;
}


int open_port(
    js_lib_server_t *server,
    js_port_descr_t *port_descr,
    js_lib_port_t **result)
{

	struct js_zynq *js = (struct js_zynq *)server;

	js_port_impl_t *port;
	js_node_t *node;

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


static int close_port(
    js_lib_port_t *port_arg)
{

	js_printf ("\n\nClosing Port.\n\n");
	js_port_impl_t *port = (js_port_impl_t *)port_arg;
	int ret = 0;

	setPin (MIO_TDI, 0);
	setPin (MIO_TMS, 0);
	setPin (MIO_TCK, 0);

	js_delete_command_sequence(&port->cmdseq->base);

    free (port);

    return ret;
}


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
    int bits = 0;
    unsigned long long time = 0;
    unsigned long long time_start = 0;
    unsigned long long time_end = 0;
    unsigned int delay = 0;

	// program FUSE_USER bit in row 31 bit 0
	//Go to TLR to clear FUSE_CTS
	jtag_navigate (g_port, JS_RESET);

	//Load FUSE_CTS instruction on IR
	jtag_setPreAndPostPads (g_port, 0, ZYNQ_DAP_IR_LENGTH, 0, 1);
	bits = ZYNQ_TAP_IR_LENGTH; // xc7z020 ir length
	wrBuffer [0] = 0x30; // FUSE_CTS instruction
	jtag_shift (g_port, ATOMIC_IR_SCAN, bits, wrBuffer, NULL, JS_DRSELECT);

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

	bits = 64; // fuse_cts data length

	/* Step to CDR/SDR to shift in the command word
     * dma=1; pgm=1; a_row<4:0> & a_bit<4:0>
     * Continuously shift in MAGIC_CTS_WRITE
     */
	jtag_shift (g_port, ATOMIC_DR_SCAN, bits, wrBuffer, NULL, JS_DRSELECT);

	jtag_navigate (g_port, JS_DRCAPTURE);
	jtag_navigate (g_port, JS_DREXIT1);
	jtag_navigate (g_port, JS_DRUPDATE);

	//Go to RTI and stay in RTI EXACTLY Tpgm = 12 us (tbd) and immediately exit to SDS
	time_start = XilSKey_Efuse_GetTime();
	jtag_navigate (g_port, JS_IDLE);
	time_end = XilSKey_Efuse_GetTime();
	delay = (u32)((time_end - time_start)/(TimerTicksfor100ns));

	//Here we will be providing 12us delay.
	if(delay < 110)
	{

		XilSKey_Efuse_SetTimeOut(&time, 110-delay);
		while(1)
		{
			if(XilSKey_Efuse_IsTimerExpired(time) == 1)
				break;
		}
	}

	jtag_navigate (g_port, JS_DRSELECT);
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
    int bits = 8;
    unsigned char * row_data_ptr = (unsigned char *)row_data;

		// read 64-bit eFUSE dna
		//Go to TLR to clear FUSE_CTS
		jtag_navigate (g_port, JS_RESET);

		//Load FUSE_CTS instruction on IR
		jtag_setPreAndPostPads (g_port, 0, ZYNQ_DAP_IR_LENGTH, 0, 1);
		bits = ZYNQ_TAP_IR_LENGTH; // xc7z020 ir length
		wrBuffer [0] = 0x30; // FUSE_CTS instruction
		jtag_shift (g_port, ATOMIC_IR_SCAN, bits, wrBuffer, NULL, JS_DRSELECT);
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

		bits = 64; // fuse_cts data length
		jtag_shift (g_port, ATOMIC_DR_SCAN, bits, wrBuffer, NULL, JS_DRSELECT);

		jtag_shift (g_port, ATOMIC_DR_SCAN, bits, NULL, rdBuffer, JS_DRSELECT);

		row_data_ptr[0] = rdBuffer [4];
		row_data_ptr[1] = rdBuffer [5];
		row_data_ptr[2] = rdBuffer [6];
		row_data_ptr[3] = rdBuffer [7];
}
int JtagValidateMioPins(void)
{
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

	return 0;
}

int JtagServerInit(XilSKey_EPl *InstancePtr)
{
    int retval=0, i=0, num_taps=0, status=0;
    unsigned long *tap_codes = NULL;

    g_mio_jtag_tdi		=	InstancePtr->JtagMioTDI;
    g_mio_jtag_tdo		= 	InstancePtr->JtagMioTDO;
    g_mio_jtag_tck		= 	InstancePtr->JtagMioTCK;
    g_mio_jtag_tms		=	InstancePtr->JtagMioTMS;
    g_mio_jtag_mux_sel	=	InstancePtr->JtagMioMuxSel;
    g_mux_sel_def_val   =   InstancePtr->JtagMuxSelLineDefVal;

    status = JtagValidateMioPins();
    if(status != 0)
    {
    	return 1;
    }

    if((g_mux_sel_def_val != 0) && (g_mux_sel_def_val != 1))
    {
    	return 1;
    }
    JtagInitGpio();

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


	//Check if one of the tap code is for Zynq DAP ID: 4ba00477

    for (i = 0; i < num_taps; i++) {
		if(tap_codes[i] == ZYNQ_DAP_ID){
			break;
		}
    }

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
* @param  BBRAM instance pointer
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
    unsigned long *tap_codes = NULL;

    g_mio_jtag_tdi		=	InstancePtr->JtagMioTDI;
    g_mio_jtag_tdo		= 	InstancePtr->JtagMioTDO;
    g_mio_jtag_tck		= 	InstancePtr->JtagMioTCK;
    g_mio_jtag_tms		=	InstancePtr->JtagMioTMS;
    g_mio_jtag_mux_sel	=	InstancePtr->JtagMioMuxSel;
    g_mux_sel_def_val   =   InstancePtr->JtagMuxSelLineDefVal;

    status = JtagValidateMioPins();
    if(status != 0)
    {
    	return 1;
    }

    if((g_mux_sel_def_val != 0) && (g_mux_sel_def_val != 1))
    {
    	return 1;
    }

    JtagInitGpio();

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


	//Check if one of the tap code is for Zynq DAP ID: 4ba00477

    for (i = 0; i < num_taps; i++) {
		if(tap_codes[i] == ZYNQ_DAP_ID){
			break;
		}
    }

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
* @param  BBRAM instance pointer
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
int Bbram_Init(XilSKey_Bbram *InstancePtr)
{
	u8 IRCaptureStatus = 0;
	u8 WriteBuffer[4];
	unsigned long long Time = 0;

	jtag_navigate (g_port, JS_RESET);
	jtag_navigate (g_port, JS_IDLE);

	/* Load bypass */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	WriteBuffer[0] = BYPASS;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Load JPROGRAM
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	WriteBuffer[0] = JPROGRAM;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
			NULL, JS_IDLE);

	/*
	 * Load ISC_NOOP
	 */
	WriteBuffer[0] = ISC_NOOP;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
	WriteBuffer[0] = ISC_NOOP;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
* @param  BBRAM instance pointer
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

	/*
	 * Initial state - RTI
	 */
	jtag_navigate (g_port, JS_IDLE);

	/*
	 * Load ISC_ENABLE
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	WriteBuffer[0] = ISC_ENABLE;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
	WriteBuffer[0] = ISC_PROGRAM_KEY;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
	WriteBuffer[0] = ISC_PROGRAM;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
		WriteBuffer[0] = ISC_PROGRAM;
		jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
* @param  BBRAM instance pointer
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

	/*
	 * Initial state - RTI
	 */
	jtag_navigate (g_port, JS_IDLE);

	jtag_setPreAndPostPads (g_port, IRHEADER, IRTRAILER,
			DRHEADER, DRTRAILER);
	/*
	 * Load ISC_ENABLE
	 */
	WriteBuffer[0] = ISC_ENABLE;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
	WriteBuffer[0] = ISC_READ;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
		WriteBuffer[0] = ISC_READ;
		jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
	WriteBuffer[0] = ISC_DISABLE;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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
* @param  none
*
* @return
*
*	none
*
*
* @note
*
*****************************************************************************/
void Bbram_DeInit(void)
{
	u8 WriteBuffer[5];

	/*
	 * Load BYPASS
	 */
	jtag_setPreAndPostPads (g_port, IRHEADER_BYP, IRTRAILER_BYP,
			DRHEADER_BYP, DRTRAILER_BYP);

	WriteBuffer[0] = BYPASS;
	jtag_shift (g_port, ATOMIC_IR_SCAN, IRLENGTH, WriteBuffer,
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