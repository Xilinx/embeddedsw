/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file ddr_init.c
*
* Initialize the DDR controller. When PCW is functioning, this would be gone.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date			 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm	06/19/09 First release
* 2.00a jz	05/11/11 Changed register to #defines, updated to peep11
* 3.00a mb	30/05/12 included fsbl.h
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "fsbl.h"

/************************** Constant Definitions *****************************/
#define DDR_CONFIG_BASE			(XPS_DDR_CTRL_BASEADDR + 0x000)

#define DDR_MSTR_CTRL_REG		(XPS_DDR_CTRL_BASEADDR + 0x000)

#define DDR_TWORANKPHY_REG		(XPS_DDR_CTRL_BASEADDR + 0x004)

#define DDR_HPR_PARAMS_REG		(XPS_DDR_CTRL_BASEADDR + 0x008)
#define DDR_LPR_PARAMS_REG		(XPS_DDR_CTRL_BASEADDR + 0x00C)
#define DDR_W_PARAMS_REG		(XPS_DDR_CTRL_BASEADDR + 0x010)
#define DDR_DRAM_PARAMS_1_REG		(XPS_DDR_CTRL_BASEADDR + 0x014)
#define DDR_DRAM_PARAMS_2_REG		(XPS_DDR_CTRL_BASEADDR + 0x018)
#define DDR_DRAM_PARAMS_3_REG		(XPS_DDR_CTRL_BASEADDR + 0x01C)
#define DDR_DRAM_PARAMS_4_REG		(XPS_DDR_CTRL_BASEADDR + 0x020)
#define DDR_DRAM_PARAMS_5_REG		(XPS_DDR_CTRL_BASEADDR + 0x024)
#define DDR_DRAM_INIT_PARAMS_REG	(XPS_DDR_CTRL_BASEADDR + 0x028)

#define DDR_DRAM_MODE_1_REG		(XPS_DDR_CTRL_BASEADDR + 0x02C)
#define DDR_DRAM_MODE_2_REG		(XPS_DDR_CTRL_BASEADDR + 0x030)
#define DDR_DRAM_BURST8_REG		(XPS_DDR_CTRL_BASEADDR + 0x034)

#define DDR_DEBUG_REG			(XPS_DDR_CTRL_BASEADDR + 0x038)

#define DDR_ADDR_MAP_1_REG		(XPS_DDR_CTRL_BASEADDR + 0x03C)
#define DDR_ADDR_MAP_2_REG		(XPS_DDR_CTRL_BASEADDR + 0x040)
#define DDR_ADDR_MAP_3_REG		(XPS_DDR_CTRL_BASEADDR + 0x044)

#define DDR_ODT_RD_WR_REG		(XPS_DDR_CTRL_BASEADDR + 0x048)
#define DDR_PHY_RDC_FIFO_CTRL_REG	(XPS_DDR_CTRL_BASEADDR + 0x04C)
#define DDR_REG_PHY_RDC_FIFO_CTRL	(XPS_DDR_CTRL_BASEADDR + 0x050)
#define DDR_STATUS_REG			(XPS_DDR_CTRL_BASEADDR + 0x054)

#define DDR_DLL_CALIB_REG		(XPS_DDR_CTRL_BASEADDR + 0x058)
#define DDR_ODT_REG			(XPS_DDR_CTRL_BASEADDR + 0x05C)
#define DDR_MISC_1_REG			(XPS_DDR_CTRL_BASEADDR + 0x060)
#define DDR_MISC_2_REG			(XPS_DDR_CTRL_BASEADDR + 0x064)

#define DDR_WR_DLL_FORCE		(XPS_DDR_CTRL_BASEADDR + 0x068)
#define DDR_RD_DLL_FORCE0_REG		(XPS_DDR_CTRL_BASEADDR + 0x06C)
#define DDR_RD_DLL_FORCE1_REG		(XPS_DDR_CTRL_BASEADDR + 0x070)

#define DDR_WR_RATIO_REG		(XPS_DDR_CTRL_BASEADDR + 0x074)
#define DDR_RD_RATIO_REG		(XPS_DDR_CTRL_BASEADDR + 0x078)

#define DDR_MSTR_DLL_STATUS1_REG	(XPS_DDR_CTRL_BASEADDR + 0x07C)
#define DDR_RD_SLAVE_STATUS0_REG	(XPS_DDR_CTRL_BASEADDR + 0x080)
#define DDR_RD_SLAVE_STATUS1_REG	(XPS_DDR_CTRL_BASEADDR + 0x084)

#define DDR_OF_STATUS0_REG		(XPS_DDR_CTRL_BASEADDR + 0x088)
#define DDR_OF_STATUS1_REG		(XPS_DDR_CTRL_BASEADDR + 0x08C)
#define DDR_OF_STATUS2_REG		(XPS_DDR_CTRL_BASEADDR + 0x090)
#define DDR_OF_STATUS3_REG		(XPS_DDR_CTRL_BASEADDR + 0x094)

#define DDR_MSTR_DLL_STATUS2_REG	(XPS_DDR_CTRL_BASEADDR + 0x098)

#define DDR_Wr_DLL_FORCE1_REG		(XPS_DDR_CTRL_BASEADDR + 0x09C)
#define DDR_REFRESH_TIMER01_REG		(XPS_DDR_CTRL_BASEADDR + 0x0A0)
#define DDR_T_ZQ_REG			(XPS_DDR_CTRL_BASEADDR + 0x0A4)
#define DDR_T_ZQ_SHORT_INTERVAL_REG	(XPS_DDR_CTRL_BASEADDR + 0x0A8)

#define DDR_STATUS_DATA_SL_DLL_01_REG	(XPS_DDR_CTRL_BASEADDR + 0x0AC)
#define DDR_STATUS_DATA_SL_DLL_23_REG	(XPS_DDR_CTRL_BASEADDR + 0x0B0)
#define DDR_STATUS_DQS_SL_DLL_01_REG	(XPS_DDR_CTRL_BASEADDR + 0x0B4)
#define DDR_STATUS_DQS_SL_DLL_23_REG	(XPS_DDR_CTRL_BASEADDR + 0x0B8)

#define DDR_WR_DATA_SLV0_REG		(XPS_DDR_CTRL_BASEADDR + 0x17c)
#define DDR_WR_DATA_SLV1_REG		(XPS_DDR_CTRL_BASEADDR + 0x180)
#define DDR_WR_DATA_SLV2_REG		(XPS_DDR_CTRL_BASEADDR + 0x184)
#define DDR_WR_DATA_SLV3_REG		(XPS_DDR_CTRL_BASEADDR + 0x188)

#define DDR_ID_REG			(XPS_DDR_CTRL_BASEADDR + 0x200)
#define DDR_DDR_CFG_REG			(XPS_DDR_CTRL_BASEADDR + 0x204)

#define DDR_PRIO_WR_PORT00_REG		(XPS_DDR_CTRL_BASEADDR + 0x208)
#define DDR_PRIO_WR_PORT01_REG		(XPS_DDR_CTRL_BASEADDR + 0x20C)
#define DDR_PRIO_WR_PORT02_REG		(XPS_DDR_CTRL_BASEADDR + 0x210)
#define DDR_PRIO_WR_PORT03_REG		(XPS_DDR_CTRL_BASEADDR + 0x214)
#define DDR_PRIO_RD_PORT00_REG		(XPS_DDR_CTRL_BASEADDR + 0x218)
#define DDR_PRIO_RD_PORT01_REG		(XPS_DDR_CTRL_BASEADDR + 0x21C)
#define DDR_PRIO_RD_PORT02_REG		(XPS_DDR_CTRL_BASEADDR + 0x220)
#define DDR_PRIO_RD_PORT03_REG		(XPS_DDR_CTRL_BASEADDR + 0x224)

#define DDR_PERF_MON_1_PORT0_REG	(XPS_DDR_CTRL_BASEADDR + 0x228)
#define DDR_PERF_MON_1_PORT1_REG	(XPS_DDR_CTRL_BASEADDR + 0x22C)
#define DDR_PERF_MON_1_PORT2_REG	(XPS_DDR_CTRL_BASEADDR + 0x230)
#define DDR_PERF_MON_1_PORT3_REG	(XPS_DDR_CTRL_BASEADDR + 0x234)
#define DDR_PERF_MON_2_PORT0_REG	(XPS_DDR_CTRL_BASEADDR + 0x238)
#define DDR_PERF_MON_2_PORT1_REG	(XPS_DDR_CTRL_BASEADDR + 0x23C)
#define DDR_PERF_MON_2_PORT2_REG	(XPS_DDR_CTRL_BASEADDR + 0x240)
#define DDR_PERF_MON_2_PORT3_REG	(XPS_DDR_CTRL_BASEADDR + 0x244)
#define DDR_PERF_MON_3_PORT0_REG	(XPS_DDR_CTRL_BASEADDR + 0x248)
#define DDR_PERF_MON_3_PORT1_REG	(XPS_DDR_CTRL_BASEADDR + 0x24C)
#define DDR_PERF_MON_3_PORT2_REG	(XPS_DDR_CTRL_BASEADDR + 0x250)
#define DDR_PERF_MON_3_PORT3_REG	(XPS_DDR_CTRL_BASEADDR + 0x254)
#define DDR_TRUSTED_MEM_CFG_REG		(XPS_DDR_CTRL_BASEADDR + 0x258)

#define DDR_EXCLACC_CFG_PORT0_REG	(XPS_DDR_CTRL_BASEADDR + 0x25C)
#define DDR_EXCLACC_CFG_PORT1_REG	(XPS_DDR_CTRL_BASEADDR + 0x260)
#define DDR_EXCLACC_CFG_PORT2_REG	(XPS_DDR_CTRL_BASEADDR + 0x264)
#define DDR_EXCLACC_CFG_PORT3_REG	(XPS_DDR_CTRL_BASEADDR + 0x268)

/* Trust zone configuration register */
#define SLCR_LOCK_REG			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_REG			(XPS_SYS_CTRL_BASEADDR + 0x8)
#define TZ_DDR_RAM_REG			(XPS_SYS_CTRL_BASEADDR + 0x430)


/* Mask defines */
#define DDR_OUT_RESET_MASK				0x1

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define DDRIn32		Xil_In32
#define DDROut32	Xil_Out32
/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

void init_ddr(void);

#define DDRIn32	Xil_In32
#define DDROut32 Xil_Out32

static int verify = 0;

static void NewDDROut32(u32 Address, u32 Value)
{
	u32 Data;

	if (verify) {
		Data = DDRIn32(Address);
		if (Data != Value)	fsbl_printf(DEBUG_INFO,"Verify failed, Address = %08X, \
				Data = %08X, Expected = %08X\n\r",	Address, Data, Value);
	} else
		DDROut32(Address, Value);
}

#undef DDROut32
#define DDROut32 NewDDROut32

void init_ddr(void)
{
	u32 RegValue;

	RegValue = DDRIn32(DDR_MSTR_CTRL_REG);

	/* If DDR is being taking out of reset, then it has been configured
	 */
	if (RegValue & DDR_OUT_RESET_MASK)
		verify = 1;

	/* Configure DDR */
	DDROut32(DDR_MSTR_CTRL_REG, 0x00000200);

	/* direct rip of the DDR init tcl for the PEEP startup */
	DDROut32(DDR_TWORANKPHY_REG, 0x000C1061); /* # 0 */

	DDROut32(DDR_LPR_PARAMS_REG, 0x03001001); //;#3
	DDROut32(DDR_W_PARAMS_REG, 0x00014001);	//;#4

	DDROut32(DDR_DRAM_PARAMS_1_REG, 0x0004e020); //; #5

#ifdef PEEP_CODE
	DDROut32(DDR_DRAM_PARAMS_2_REG, 0x36264ccf); //; #6
#else
	DDROut32(DDR_DRAM_PARAMS_2_REG, 0x349B48CD); //; #6
#endif
	DDROut32(DDR_DRAM_PARAMS_3_REG, 0x820158a4); //; #7

	DDROut32(DDR_DRAM_PARAMS_4_REG, 0x250882c4); //; #8

	DDROut32(DDR_DRAM_INIT_PARAMS_REG, 0x00809004); //; #10

	DDROut32(DDR_DRAM_MODE_1_REG, 0x0);			//; #11

	DDROut32(DDR_DRAM_MODE_2_REG, 0x00040952);	//; #12

	DDROut32(DDR_DRAM_BURST8_REG, 0x00020022);	//; #13

#ifdef PEEP_CODE
	DDROut32(DDR_ADDR_MAP_1_REG, 0xF88);		 //; #15
#endif

#ifdef PALLADIUM
	DDROut32(DDR_ADDR_MAP_1_REG, 0x777);		 //; #15
#endif

	DDROut32(DDR_ADDR_MAP_2_REG, 0xFF000000);	//; #16

	DDROut32(DDR_ADDR_MAP_3_REG, 0x0FF66666);	//; #17

	DDROut32(DDR_REG_PHY_RDC_FIFO_CTRL, 0x256);	//; #20

	DDROut32(DDR_ODT_REG, 0x2223);				//; #23

	DDROut32(DDR_MISC_2_REG, 0x00020FE0);		//; #25

	DDROut32(DDR_T_ZQ_REG, 0x10200800);			//; #41

	DDROut32(DDR_STATUS_DQS_SL_DLL_23_REG, 0x200065);	 //; #46

	DDROut32(DDR_WR_DATA_SLV0_REG, 0x50);		//; #95

	DDROut32(DDR_WR_DATA_SLV1_REG, 0x50);		//; #96

	DDROut32(DDR_WR_DATA_SLV2_REG, 0x50);		//; #97

	DDROut32(DDR_WR_DATA_SLV3_REG, 0x50);		//; #98

	DDROut32(DDR_ID_REG, 0x0);					//; #128

	/* Enable ddr controller by taking the controller out of reset */
	DDROut32(DDR_MSTR_CTRL_REG,
		DDRIn32(DDR_MSTR_CTRL_REG) | DDR_OUT_RESET_MASK);

#ifdef PALLADIUM

	/* Workaround for early palladium, to be removed for 4.61 */
	DDROut32(SLCR_UNLOCK_REG, 0xDF0D);

	DDROut32(TZ_DDR_RAM_REG, 0xffffffff);

	DDROut32(SLCR_LOCK_REG, 0x767B);

#endif	/* PALLADIUM*/
}
