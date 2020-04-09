/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file pcam_5C_cfgs.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    19/09/20 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "function_prototype.h"
#include "pcam_5C_cfgs.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/
/* Uncomment or comment the following depending on the board used*/
#define KC705
//#define ZC702
//#define ZCU102

#ifdef KC705
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_0_IIC_1_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x3C
	#define SENSOR_ADDRESS 		0x3C

	#define IIC_MUX_ADDRESS 	0x74
	#define IIC_FMC_CHANNEL		0x07

	#define IIC_MUX_ENABLE		0
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_CSISS_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 1920
	#define HEIGHT 1080
	#define LANES 2
#endif


#ifdef ZC702
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_1_IIC_2_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_1_IIC_2_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_AXI_IIC_0_IIC2INTC_IRPT_INTR
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_AXI_IIC_1_IIC2INTC_IRPT_INTR
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x20
	#define SENSOR_ADDRESS 		0x10

	#define IIC_MUX_ADDRESS 	0x74
	#define IIC_FMC_CHANNEL		0x3F

	#define IIC_MUX_ENABLE
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 640
	#define HEIGHT 480
	#define LANES 4
#endif


#ifdef ZCU102
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_0_IIC_1_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x3C
	#define SENSOR_ADDRESS 		0x3C

	#define IIC_MUX_ADDRESS 	0x75
	#define IIC_FMC_CHANNEL		0x03

	#define IIC_MUX_ENABLE
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_CSISS_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 640
	#define HEIGHT 480
	#define LANES 4

#endif

/**************************** Type Definitions *******************************/

/*
*The AddressType for ML300/ML310/ML410/ML510 boards should be u16 as the address
* pointer in the on board EEPROM is 2 bytes.
* The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605/KC705/ZC702
* /ZC706 boards should be u8 as the address pointer in the on board EEPROM is
* 1 bytes.
 */
/*
*/
/************************** Function Prototypes ******************************/

int pcam5c_mode = 1;

//init_zybo
struct regval_list sensor_pre[] = {
	//[7]=0 Software reset; [6]=1 Software power down; Default=0x02
	{0x3008, 0x42},
	//[1]=1 System input clock from PLL; Default read = 0x11
	{0x3103, 0x03},
	//[3:0]=0000 MD2P,MD2N,MCP,MCN input; Default=0x00
	{0x3017, 0x00},
	//[7:2]=000000 MD1P,MD1N, D3:0 input; Default=0x00
	{0x3018, 0x00},
	//[6:4]=001 PLL charge pump, [3:0]=1000 MIPI 8-bit mode
	{0x3034, 0x18},


	//PLL1 configuration
	{0x3035, 0x11},
	{0x3036, 0x38},
	{0x3037, 0x11},
	{0x3108, 0x01},
	//PLL2 configuration
	{0x303D, 0x10},
	{0x303B, 0x19},

	{0x3630, 0x2e},
	{0x3631, 0x0e},
	{0x3632, 0xe2},
	{0x3633, 0x23},
	{0x3621, 0xe0},
	{0x3704, 0xa0},
	{0x3703, 0x5a},
	{0x3715, 0x78},
	{0x3717, 0x01},
	{0x370b, 0x60},
	{0x3705, 0x1a},
	{0x3905, 0x02},
	{0x3906, 0x10},
	{0x3901, 0x0a},
	{0x3731, 0x02},
	//VCM debug mode
	{0x3600, 0x37},
	{0x3601, 0x33},
	//System control register changing not recommended
	{0x302d, 0x60},
	//??
	{0x3620, 0x52},
	{0x371b, 0x20},
	//?? DVP
	{0x471c, 0x50},

	{0x3a13, 0x43},
	{0x3a18, 0x00},
	{0x3a19, 0xf8},
	{0x3635, 0x13},
	{0x3636, 0x06},
	{0x3634, 0x44},
	{0x3622, 0x01},
	{0x3c01, 0x34},
	{0x3c04, 0x28},
	{0x3c05, 0x98},
	{0x3c06, 0x00},
	{0x3c07, 0x08},
	{0x3c08, 0x00},
	{0x3c09, 0x1c},
	{0x3c0a, 0x9c},
	{0x3c0b, 0x40},

	//[7]=1 color bar enable, [3:2]=00 eight color bar
	{0x503d, 0x00},
	//[2]=1 ISP vflip, [1]=1 sensor vflip
	{0x3820, 0x46},

	{0x300e, 0x45},
	//{0x300e, 0x25},
	{0x4800, 0x14},
	{0x302e, 0x08},
	//[7:4]=0x3 YUV422, [3:0]=0x0 YUYV
	//{0x4300, 0x30},
	//[7:4]=0x6 RGB565, [3:0]=0x0 {b[4:0],g[5:3],g[2:0],r[4:0]}
	{0x4300, 0x6f},
	{0x501f, 0x01},

	{0x4713, 0x03},
	{0x4407, 0x04},
	{0x440e, 0x00},
	{0x460b, 0x35},
	//[1]=0 DVP PCLK divider manual control by 0x3824[4:0]
	{0x460c, 0x20},
	//[4:0]=1 SCALE_DIV=INT(3824[4:0]/2)
	{0x3824, 0x01},

	//MIPI timing
	//		{0x4805, 0x10}, //LPX global timing select=auto
	//		{0x4818, 0x00}, //hs_prepare + hs_zero_min ns
	//		{0x4819, 0x96},
	//		{0x482A, 0x00}, //hs_prepare + hs_zero_min UI
	//
	//		{0x4824, 0x00}, //lpx_p_min ns
	//		{0x4825, 0x32},
	//		{0x4830, 0x00}, //lpx_p_min UI
	//
	//		{0x4826, 0x00}, //hs_prepare_min ns
	//		{0x4827, 0x32},
	//		{0x4831, 0x00}, //hs_prepare_min UI

	{0x5000, 0x07},
	{0x5001, 0x03}
};
//pcam_iic_1080p_
struct regval_list pcam5c_mode1[] =
	{//1920 x 1080 @ 30fps, RAW10, MIPISCLK=420, SCLK=84MHz, PCLK=84M
		//PLL1 configuration
		{0x3035, 0x21}, // 30fps setting
		//[7:0]=105 PLL multiplier
		{0x3036, 0x69},//96,B0,A2,C8=800,E0=900
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x1A},

		//[3:0]=0 X address start high byte
		{0x3800, (336 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801, 336 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (426 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 426 & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (2287 >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 2287 & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (1529 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 1529 & 0xFF},

		//[3:0]=0 timing hoffset high byte
		{0x3810, (16 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 16 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (12 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 12 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1920 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1920 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (1080 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 1080 & 0xFF},

		//HTS line exposure time in # of pixels Tline=HTS/sclk
		{0x380c, (2500 >> 8) & 0x1F},
		{0x380d, 2500 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (1120 >> 8) & 0xFF},
		{0x380f, 1120 & 0xFF},

		{0x3814, 0x11},
		{0x3815, 0x11},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=0 no horizontal binning
		{0x3821, 0x00},

		{0x4837, 24}, // 1/84M*2 17,15,16,13,12

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03}
};



//Default sensor values: Mode1 //advanced_awb
struct regval_list sensor_list[] = {
// Enable Advanced AWB
	{0x3406 ,0x00},
	{0x5192 ,0x04},
	{0x5191 ,0xf8},
	{0x518d ,0x26},
	{0x518f ,0x42},
	{0x518e ,0x2b},
	{0x5190 ,0x42},
	{0x518b ,0xd0},
	{0x518c ,0xbd},
	{0x5187 ,0x18},
	{0x5188 ,0x18},
	{0x5189 ,0x56},
	{0x518a ,0x5c},
	{0x5186 ,0x1c},
	{0x5181 ,0x50},
	{0x5184 ,0x20},
	{0x5182 ,0x11},
	{0x5183 ,0x00},
	{0x5001 ,0x03}
};


/****************************************************************************/

const int length_sensor_pre = sizeof(sensor_pre) / sizeof(sensor_pre[0]);
const int size_sensor_pre = sizeof(sensor_pre);

const int length_pcam5c_mode1 = sizeof(pcam5c_mode1) / sizeof(pcam5c_mode1[0]);
const int size_length_pcam5c_mode1 = sizeof(length_pcam5c_mode1);

const int length_sensor_list= sizeof(sensor_list) / sizeof(sensor_list[0]);
const int size_sensor_list= sizeof(sensor_list);
