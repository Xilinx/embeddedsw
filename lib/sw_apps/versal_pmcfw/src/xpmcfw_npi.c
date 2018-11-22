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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file xpmcfw_npi.c
*
* This file contains the static NPI configurations taken from verif team
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpmcfw_config.h"
#include "xil_io.h"
#include "xpmcfw_hw.h"
#include "xpmcfw_debug.h"
#include "xpmcfw_main.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define insert_nop(x)		for (int iii=0;iii<x;iii++);
#define  dbg0_pmc(x)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

void prog_reg(int addr,int shift,int mask,int value) {

int rdata =0;

rdata  = Xil_In32(addr);
rdata  = rdata & (~mask);
rdata  = rdata | (value << shift);
Xil_Out32(addr,rdata);

}

void poll_for(int addr,int mask,int shift,int exp_data) {
  int rdata =0;
  int counter = 10000;

  while(counter > 0){
    rdata  = Xil_In32(addr);
    rdata  = rdata & mask;
	rdata  = rdata >> shift;
    if(exp_data == rdata){
      break;
    }
      counter=counter-1;
  }
  if(counter == 0) {

	 // Failure, return error
  }
}

//-------------------------------
void msoc_init_trigger_from_sv() {

	volatile unsigned int rd_val;

	Xil_Out32(PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0, PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0);	// PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0(0xF0500050)

    insert_nop(50);

	rd_val = Xil_In32(PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0);
	while(rd_val == PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0) {
		rd_val = Xil_In32(PMC_GLOBAL_PERS_GLOB_GEN_STORAGE0);
	}

}

void write_to_reg(unsigned int addr, unsigned int data) {
    Xil_Out32(addr, data);
}

//-----------------------------------------

#ifdef XPMCFW_HW50
void msoc_hsr_init()
{

    unsigned int rdata;

    Xil_Out32(0xf615000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0.ddrmc_main_bank.pcsr_lock
    Xil_Out32(0xf611000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0_ub.ddrmc_ub_bank.ddrmc_pcsr_lock
    dbg0_pmc(16398);
    //**************Register programming from noc_cfg start---------------- 
    dbg0_pmc(16384);
    // 103:NOC_CONFIG selected : noc_cfg_ddr_vnoc.c
//    XPmcFw_Printf(DEBUG_GENERAL, " NPI_NIR unlocked \n \r");
    Xil_Out32(0xf600000c,0xf9e8d7c6); 
    Xil_Out32(0xf6000108,0x400003);
//    XPmcFw_Printf(DEBUG_GENERAL, "NIR disabled \n \r");  
    Xil_Out32(0xf6000000, 0x00000002);
    Xil_Out32(0xf6000004, 0x00000000);
    Xil_Out32(0xf6000100, 0x39852310);
    Xil_Out32(0xf600000c, 0x00000000);
    Xil_Out32(0xf60f000c, 0xf9e8d7c6);
    Xil_Out32(0xf60f0000, 0x010382cc);
    Xil_Out32(0xf60f0004, 0x00038200);
    Xil_Out32(0xf60f083c, 0x00000100);
    Xil_Out32(0xf60f0840, 0x00001fff);
    Xil_Out32(0xf60f0844, 0x00000100);
    Xil_Out32(0xf60f0848, 0x00001fff);
    Xil_Out32(0xf60f0858, 0x00000002);
    Xil_Out32(0xf60f0860, 0x00000002);
    Xil_Out32(0xf60f0864, 0x00000001);
    Xil_Out32(0xf60f0868, 0x00000001);
    Xil_Out32(0xf60f086c, 0x00000003);
    Xil_Out32(0xf60f08d4, 0x00000efc);
    Xil_Out32(0xf60f0450, 0x00000781);
    Xil_Out32(0xf60f02c4, 0x00000000);
    Xil_Out32(0xf60f03d0, 0x00000c43);
    Xil_Out32(0xf60f03d4, 0x00000b83);
    Xil_Out32(0xf60f03d8, 0x00000bc3);
    Xil_Out32(0xf60f03dc, 0x00000c03);
    Xil_Out32(0xf60f03e0, 0x00000c43);
    Xil_Out32(0xf60f03e4, 0x00000b83);
    Xil_Out32(0xf60f03e8, 0x00000bc3);
    Xil_Out32(0xf60f03ec, 0x00000c03);
    Xil_Out32(0xf60f0000, 0x00000002);
    Xil_Out32(0xf60f0004, 0x00000000);
    Xil_Out32(0xf60f000c, 0x00000000);
    Xil_Out32(0xf60f200c, 0xf9e8d7c6);
    Xil_Out32(0xf60f2000, 0x010382cc);
    Xil_Out32(0xf60f2004, 0x01038200);
    Xil_Out32(0xf60f283c, 0x00000100);
    Xil_Out32(0xf60f2840, 0x00001fff);
    Xil_Out32(0xf60f2844, 0x00000100);
    Xil_Out32(0xf60f2848, 0x00001fff);
    Xil_Out32(0xf60f2858, 0x00000002);
    Xil_Out32(0xf60f2860, 0x00000002);
    Xil_Out32(0xf60f2864, 0x00000001);
    Xil_Out32(0xf60f2868, 0x00000001);
    Xil_Out32(0xf60f286c, 0x00000003);
    Xil_Out32(0xf60f28d4, 0x00000d77);
    Xil_Out32(0xf60f2450, 0x000007c1);
    Xil_Out32(0xf60f22c4, 0x00000000);
    Xil_Out32(0xf60f23d0, 0x00000c03);
    Xil_Out32(0xf60f23d4, 0x00000c43);
    Xil_Out32(0xf60f23d8, 0x00000b83);
    Xil_Out32(0xf60f23dc, 0x00000bc3);
    Xil_Out32(0xf60f23e0, 0x00000c03);
    Xil_Out32(0xf60f23e4, 0x00000c43);
    Xil_Out32(0xf60f23e8, 0x00000b83);
    Xil_Out32(0xf60f23ec, 0x00000bc3);
    Xil_Out32(0xf60f2000, 0x00000002);
    Xil_Out32(0xf60f2004, 0x00000000);
    Xil_Out32(0xf60f200c, 0x00000000);
    Xil_Out32(0xf60f400c, 0xf9e8d7c6);
    Xil_Out32(0xf60f4000, 0x010382cc);
    Xil_Out32(0xf60f4004, 0x01038200);
    Xil_Out32(0xf60f483c, 0x00000100);
    Xil_Out32(0xf60f4840, 0x00001fff);
    Xil_Out32(0xf60f4844, 0x00000100);
    Xil_Out32(0xf60f4848, 0x00001fff);
    Xil_Out32(0xf60f4858, 0x00000002);
    Xil_Out32(0xf60f4860, 0x00000002);
    Xil_Out32(0xf60f4864, 0x00000001);
    Xil_Out32(0xf60f4868, 0x00000001);
    Xil_Out32(0xf60f486c, 0x00000003);
    Xil_Out32(0xf60f48d4, 0x00000fdd);
    Xil_Out32(0xf60f4450, 0x00000801);
    Xil_Out32(0xf60f42c4, 0x00000000);
    Xil_Out32(0xf60f43d0, 0x00000b83);
    Xil_Out32(0xf60f43d4, 0x00000bc3);
    Xil_Out32(0xf60f43d8, 0x00000c03);
    Xil_Out32(0xf60f43dc, 0x00000c43);
    Xil_Out32(0xf60f43e0, 0x00000b83);
    Xil_Out32(0xf60f43e4, 0x00000bc3);
    Xil_Out32(0xf60f43e8, 0x00000c03);
    Xil_Out32(0xf60f43ec, 0x00000c43);
    Xil_Out32(0xf60f4000, 0x00000002);
    Xil_Out32(0xf60f4004, 0x00000000);
    Xil_Out32(0xf60f400c, 0x00000000);
    Xil_Out32(0xf60f600c, 0xf9e8d7c6);
    Xil_Out32(0xf60f6000, 0x010382cc);
    Xil_Out32(0xf60f6004, 0x00038200);
    Xil_Out32(0xf60f683c, 0x00000100);
    Xil_Out32(0xf60f6840, 0x00001fff);
    Xil_Out32(0xf60f6844, 0x00000100);
    Xil_Out32(0xf60f6848, 0x00001fff);
    Xil_Out32(0xf60f6858, 0x00000002);
    Xil_Out32(0xf60f6860, 0x00000002);
    Xil_Out32(0xf60f6864, 0x00000001);
    Xil_Out32(0xf60f6868, 0x00000001);
    Xil_Out32(0xf60f686c, 0x00000003);
    Xil_Out32(0xf60f68d4, 0x00000fb9);
    Xil_Out32(0xf60f6450, 0x00000841);
    Xil_Out32(0xf60f62c4, 0x00000000);
    Xil_Out32(0xf60f63d0, 0x00000bc3);
    Xil_Out32(0xf60f63d4, 0x00000c03);
    Xil_Out32(0xf60f63d8, 0x00000c43);
    Xil_Out32(0xf60f63dc, 0x00000b83);
    Xil_Out32(0xf60f63e0, 0x00000bc3);
    Xil_Out32(0xf60f63e4, 0x00000c03);
    Xil_Out32(0xf60f63e8, 0x00000c43);
    Xil_Out32(0xf60f63ec, 0x00000b83);
    Xil_Out32(0xf60f6000, 0x00000002);
    Xil_Out32(0xf60f6004, 0x00000000);
    Xil_Out32(0xf60f600c, 0x00000000);
    Xil_Out32(0xf60e000c, 0xf9e8d7c6);
    Xil_Out32(0xf60e0000, 0x010382cc);
    Xil_Out32(0xf60e0004, 0x01038200);
    Xil_Out32(0xf60e083c, 0x00000100);
    Xil_Out32(0xf60e0840, 0x00001fff);
    Xil_Out32(0xf60e0844, 0x00000100);
    Xil_Out32(0xf60e0848, 0x00001fff);
    Xil_Out32(0xf60e0858, 0x00000002);
    Xil_Out32(0xf60e0860, 0x00000002);
    Xil_Out32(0xf60e0864, 0x00000001);
    Xil_Out32(0xf60e0868, 0x00000001);
    Xil_Out32(0xf60e086c, 0x00000003);
    Xil_Out32(0xf60e08d4, 0x00000516);
    Xil_Out32(0xf60e0450, 0x00000881);
    Xil_Out32(0xf60e02c4, 0x00000000);
    Xil_Out32(0xf60e03d0, 0x00000bc3);
    Xil_Out32(0xf60e03d4, 0x00000c03);
    Xil_Out32(0xf60e03d8, 0x00000c43);
    Xil_Out32(0xf60e03dc, 0x00000b83);
    Xil_Out32(0xf60e03e0, 0x00000bc3);
    Xil_Out32(0xf60e03e4, 0x00000c03);
    Xil_Out32(0xf60e03e8, 0x00000c43);
    Xil_Out32(0xf60e03ec, 0x00000b83);
    Xil_Out32(0xf60e0000, 0x00000002);
    Xil_Out32(0xf60e0004, 0x00000000);
    Xil_Out32(0xf60e000c, 0x00000000);
    Xil_Out32(0xf60e200c, 0xf9e8d7c6);
    Xil_Out32(0xf60e2000, 0x010382cc);
    Xil_Out32(0xf60e2004, 0x01038200);
    Xil_Out32(0xf60e283c, 0x00000100);
    Xil_Out32(0xf60e2840, 0x00001fff);
    Xil_Out32(0xf60e2844, 0x00000100);
    Xil_Out32(0xf60e2848, 0x00001fff);
    Xil_Out32(0xf60e2858, 0x00000002);
    Xil_Out32(0xf60e2860, 0x00000002);
    Xil_Out32(0xf60e2864, 0x00000001);
    Xil_Out32(0xf60e2868, 0x00000001);
    Xil_Out32(0xf60e286c, 0x00000003);
    Xil_Out32(0xf60e28d4, 0x00000b4f);
    Xil_Out32(0xf60e2450, 0x000008c1);
    Xil_Out32(0xf60e22c4, 0x00000000);
    Xil_Out32(0xf60e23d0, 0x00000b83);
    Xil_Out32(0xf60e23d4, 0x00000bc3);
    Xil_Out32(0xf60e23d8, 0x00000c03);
    Xil_Out32(0xf60e23dc, 0x00000c43);
    Xil_Out32(0xf60e23e0, 0x00000b83);
    Xil_Out32(0xf60e23e4, 0x00000bc3);
    Xil_Out32(0xf60e23e8, 0x00000c03);
    Xil_Out32(0xf60e23ec, 0x00000c43);
    Xil_Out32(0xf60e2000, 0x00000002);
    Xil_Out32(0xf60e2004, 0x00000000);
    Xil_Out32(0xf60e200c, 0x00000000);
    Xil_Out32(0xf601000c, 0xf9e8d7c6);
    Xil_Out32(0xf6010000, 0x010382cc);
    Xil_Out32(0xf6010004, 0x01020200);
    Xil_Out32(0xf601083c, 0x00000100);
    Xil_Out32(0xf6010840, 0x00001fff);
    Xil_Out32(0xf6010844, 0x00000100);
    Xil_Out32(0xf6010848, 0x00001fff);
    Xil_Out32(0xf6010858, 0x00000002);
    Xil_Out32(0xf6010860, 0x00000002);
    Xil_Out32(0xf6010864, 0x00000001);
    Xil_Out32(0xf6010868, 0x00000001);
    Xil_Out32(0xf601086c, 0x00000003);
    Xil_Out32(0xf60108d4, 0x000002aa);
    Xil_Out32(0xf6010450, 0x00000901);
    Xil_Out32(0xf60102c4, 0x00000000);
    Xil_Out32(0xf60103d0, 0x00000c43);
    Xil_Out32(0xf60103d4, 0x00000b83);
    Xil_Out32(0xf60103d8, 0x00000bc3);
    Xil_Out32(0xf60103dc, 0x00000c03);
    Xil_Out32(0xf60103e0, 0x00000c43);
    Xil_Out32(0xf60103e4, 0x00000b83);
    Xil_Out32(0xf60103e8, 0x00000bc3);
    Xil_Out32(0xf60103ec, 0x00000c03);
    Xil_Out32(0xf6010000, 0x00000002);
    Xil_Out32(0xf6010004, 0x00000000);
    Xil_Out32(0xf601000c, 0x00000000);
    Xil_Out32(0xf601400c, 0xf9e8d7c6);
    Xil_Out32(0xf6014000, 0x010382cc);
    Xil_Out32(0xf6014004, 0x01008200);
    Xil_Out32(0xf601483c, 0x00000100);
    Xil_Out32(0xf6014840, 0x00001fff);
    Xil_Out32(0xf6014844, 0x00000100);
    Xil_Out32(0xf6014848, 0x00001fff);
    Xil_Out32(0xf6014858, 0x00000002);
    Xil_Out32(0xf6014860, 0x00000002);
    Xil_Out32(0xf6014864, 0x00000001);
    Xil_Out32(0xf6014868, 0x00000001);
    Xil_Out32(0xf601486c, 0x00000003);
    Xil_Out32(0xf60148d4, 0x00001237);
    Xil_Out32(0xf6014450, 0x00000941);
    Xil_Out32(0xf60142c4, 0x00000000);
    Xil_Out32(0xf60143d0, 0x00000c03);
    Xil_Out32(0xf60143d4, 0x00000c43);
    Xil_Out32(0xf60143d8, 0x00000b83);
    Xil_Out32(0xf60143dc, 0x00000bc3);
    Xil_Out32(0xf60143e0, 0x00000c03);
    Xil_Out32(0xf60143e4, 0x00000c43);
    Xil_Out32(0xf60143e8, 0x00000b83);
    Xil_Out32(0xf60143ec, 0x00000bc3);
    Xil_Out32(0xf6014000, 0x00000002);
    Xil_Out32(0xf6014004, 0x00000000);
    Xil_Out32(0xf601400c, 0x00000000);
    Xil_Out32(0xf601200c, 0xf9e8d7c6);
    Xil_Out32(0xf6012000, 0x010382cc);
    Xil_Out32(0xf6012004, 0x01018200);
    Xil_Out32(0xf6012838, 0x0000376a);
    Xil_Out32(0xf6012838, 0x000090b9);
    Xil_Out32(0xf601283c, 0x00000100);
    Xil_Out32(0xf6012840, 0x00001fff);
    Xil_Out32(0xf6012844, 0x00000100);
    Xil_Out32(0xf6012848, 0x00001fff);
    Xil_Out32(0xf6012858, 0x00000002);
    Xil_Out32(0xf6012860, 0x00000002);
    Xil_Out32(0xf6012864, 0x00000001);
    Xil_Out32(0xf6012868, 0x00000001);
    Xil_Out32(0xf601286c, 0x00000003);
    Xil_Out32(0xf60128d4, 0x00000bd0);
    Xil_Out32(0xf6012450, 0x00000981);
    Xil_Out32(0xf60121c0, 0x00006f00);
    Xil_Out32(0xf6012200, 0xffffffff);
    Xil_Out32(0xf6012240, 0x02c10000);
    Xil_Out32(0xf60122c0, 0x00000001);
    Xil_Out32(0xf60122c4, 0x00000001);
    Xil_Out32(0xf6012280, 0x0000060c);
    Xil_Out32(0xf60123d0, 0x00000c43);
    Xil_Out32(0xf60123d4, 0x00000b83);
    Xil_Out32(0xf60123d8, 0x00000bc3);
    Xil_Out32(0xf60123dc, 0x00000c03);
    Xil_Out32(0xf60123e0, 0x00000c43);
    Xil_Out32(0xf60123e4, 0x00000b83);
    Xil_Out32(0xf60123e8, 0x00000bc3);
    Xil_Out32(0xf60123ec, 0x00000c03);
    Xil_Out32(0xf6012000, 0x00000002);
    Xil_Out32(0xf6012004, 0x00000000);
    Xil_Out32(0xf601200c, 0x00000000);
    Xil_Out32(0xf601600c, 0xf9e8d7c6);
    Xil_Out32(0xf6016000, 0x010382cc);
    Xil_Out32(0xf6016004, 0x01038200);
    Xil_Out32(0xf601683c, 0x00000100);
    Xil_Out32(0xf6016840, 0x00001fff);
    Xil_Out32(0xf6016844, 0x00000100);
    Xil_Out32(0xf6016848, 0x00001fff);
    Xil_Out32(0xf6016858, 0x00000002);
    Xil_Out32(0xf6016860, 0x00000002);
    Xil_Out32(0xf6016864, 0x00000001);
    Xil_Out32(0xf6016868, 0x00000001);
    Xil_Out32(0xf601686c, 0x00000003);
    Xil_Out32(0xf60168d4, 0x00001428);
    Xil_Out32(0xf6016450, 0x000009c1);
    Xil_Out32(0xf60162c4, 0x00000000);
    Xil_Out32(0xf60163d0, 0x00000c03);
    Xil_Out32(0xf60163d4, 0x00000c43);
    Xil_Out32(0xf60163d8, 0x00000b83);
    Xil_Out32(0xf60163dc, 0x00000bc3);
    Xil_Out32(0xf60163e0, 0x00000c03);
    Xil_Out32(0xf60163e4, 0x00000c43);
    Xil_Out32(0xf60163e8, 0x00000b83);
    Xil_Out32(0xf60163ec, 0x00000bc3);
    Xil_Out32(0xf6016000, 0x00000002);
    Xil_Out32(0xf6016004, 0x00000000);
    Xil_Out32(0xf601600c, 0x00000000);
    Xil_Out32(0xf60d000c, 0xf9e8d7c6);
    Xil_Out32(0xf60d0000, 0x000002ff);
    Xil_Out32(0xf60d0004, 0x00000200);
    Xil_Out32(0xf60d0104, 0x00000004);
    Xil_Out32(0xf60d0108, 0x00000001);
    Xil_Out32(0xf60d0100, 0x00000a02);
    Xil_Out32(0xf60d000c, 0x00000000);
    Xil_Out32(0xf60d200c, 0xf9e8d7c6);
    Xil_Out32(0xf60d2000, 0x000002ff);
    Xil_Out32(0xf60d2004, 0x00000200);
    Xil_Out32(0xf60d2104, 0x00000004);
    Xil_Out32(0xf60d2108, 0x00000000);
    Xil_Out32(0xf60d2100, 0x00000a42);
    Xil_Out32(0xf60d200c, 0x00000000);
    Xil_Out32(0xf60e400c, 0xf9e8d7c6);
    Xil_Out32(0xf60e4000, 0x000002ff);
    Xil_Out32(0xf60e4004, 0x00000200);
    Xil_Out32(0xf60e4104, 0x00000004);
    Xil_Out32(0xf60e4108, 0x00000001);
    Xil_Out32(0xf60e4100, 0x00000a82);
    Xil_Out32(0xf60e400c, 0x00000000);
    Xil_Out32(0xf60e600c, 0xf9e8d7c6);
    Xil_Out32(0xf60e6000, 0x000002ff);
    Xil_Out32(0xf60e6004, 0x00000200);
    Xil_Out32(0xf60e6104, 0x00000004);
    Xil_Out32(0xf60e6108, 0x00000000);
    Xil_Out32(0xf60e6100, 0x00000ac2);
    Xil_Out32(0xf60e600c, 0x00000000);
    Xil_Out32(0xf602000c, 0xf9e8d7c6);
    Xil_Out32(0xf6020000, 0x000002ff);
    Xil_Out32(0xf6020004, 0x00000200);
    Xil_Out32(0xf6020104, 0x00000004);
    Xil_Out32(0xf6020108, 0x00000000);
    Xil_Out32(0xf6020100, 0x00000b02);
    Xil_Out32(0xf602000c, 0x00000000);
    Xil_Out32(0xf602200c, 0xf9e8d7c6);
    Xil_Out32(0xf6022000, 0x000002ff);
    Xil_Out32(0xf6022004, 0x00000200);
    Xil_Out32(0xf6022104, 0x00000004);
    Xil_Out32(0xf6022108, 0x00000000);
    Xil_Out32(0xf6022100, 0x00000b42);
    Xil_Out32(0xf602200c, 0x00000000);
    Xil_Out32(0xf603000c, 0xf9e8d7c6);
    Xil_Out32(0xf6030000, 0x0000023e);
    Xil_Out32(0xf6030004, 0x00000200);
    Xil_Out32(0xf60303a0, 0x00000010);
    Xil_Out32(0xf6030264, 0x55555555);
    Xil_Out32(0xf60301c0, 0xaaaaaaaa);
    Xil_Out32(0xf603026c, 0x55550000);
    Xil_Out32(0xf60301c0, 0xffffaaaa);
    Xil_Out32(0xf603000c, 0x00000000);
    Xil_Out32(0xf606000c, 0xf9e8d7c6);
    Xil_Out32(0xf6060000, 0x0000023e);
    Xil_Out32(0xf6060004, 0x00000200);
    Xil_Out32(0xf60603a0, 0x00000030);
    Xil_Out32(0xf60601c0, 0xaaaaaaaa);
    Xil_Out32(0xf6060234, 0x55555555);
    Xil_Out32(0xf60601c4, 0xaaaa0000);
    Xil_Out32(0xf6060224, 0x5555ffff);
    Xil_Out32(0xf6060288, 0xaaaaaaaa);
    Xil_Out32(0xf6060234, 0x55555555);
    Xil_Out32(0xf606028c, 0xaaaa0000);
    Xil_Out32(0xf6060224, 0x5555ffff);
    Xil_Out32(0xf606000c, 0x00000030);
    Xil_Out32(0xf606200c, 0xf9e8d7c6);
    Xil_Out32(0xf6062000, 0x0000023e);
    Xil_Out32(0xf6062004, 0x00000200);
    Xil_Out32(0xf60623a0, 0x00000040);
    Xil_Out32(0xf606223c, 0x55550000);
    Xil_Out32(0xf60621c0, 0xaaaaaaaa);
    Xil_Out32(0xf606222c, 0x55555555);
    Xil_Out32(0xf606223c, 0x55550000);
    Xil_Out32(0xf6062280, 0xaaaaaaaa);
    Xil_Out32(0xf606222c, 0x55555555);
    Xil_Out32(0xf606200c, 0x00000000);
    Xil_Out32(0xf60a000c, 0xf9e8d7c6);
    Xil_Out32(0xf60a0000, 0x0000023e);
    Xil_Out32(0xf60a0004, 0x00000200);
    Xil_Out32(0xf60a03a0, 0x00000050);
    Xil_Out32(0xf60a01c0, 0xaaaaaaaa);
    Xil_Out32(0xf60a0204, 0x55555555);
    Xil_Out32(0xf60a0264, 0x55550000);
    Xil_Out32(0xf60a026c, 0x55550000);
    Xil_Out32(0xf60a0270, 0xffffaaaa);
    Xil_Out32(0xf60a0274, 0x5555ffff);
    Xil_Out32(0xf60a021c, 0xaaaa0000);
    Xil_Out32(0xf60a000c, 0x00000000);
    Xil_Out32(0xf60a200c, 0xf9e8d7c6);
    Xil_Out32(0xf60a2000, 0x0000023e);
    Xil_Out32(0xf60a2004, 0x00000200);
    Xil_Out32(0xf60a23a0, 0x00000060);
    Xil_Out32(0xf60a21c0, 0xaaaaaaaa);
    Xil_Out32(0xf60a220c, 0x55555555);
    Xil_Out32(0xf60a2278, 0xffffaaaa);
    Xil_Out32(0xf60a227c, 0x5555ffff);
    Xil_Out32(0xf60a2214, 0xaaaa0000);
    Xil_Out32(0xf60a200c, 0x00000000);
    Xil_Out32(0xf610000c, 0xf9e8d7c6);
    Xil_Out32(0xf6100000, 0x0000023e);
    Xil_Out32(0xf6100004, 0x00000200);
    Xil_Out32(0xf61003a0, 0x00000070);
    Xil_Out32(0xf61001c4, 0xaaaa0000);
    Xil_Out32(0xf61001f4, 0x5555ffff);
    Xil_Out32(0xf61001c0, 0xffffaaaa);
    Xil_Out32(0xf6100234, 0x55550000);
    Xil_Out32(0xf61001c0, 0xffffaaaa);
    Xil_Out32(0xf6100224, 0x55550000);
    Xil_Out32(0xf610028c, 0x55550000);
    Xil_Out32(0xf61001f0, 0xffffaaaa);
    Xil_Out32(0xf6100288, 0xffff5555);
    Xil_Out32(0xf6100230, 0x0000aaaa);
    Xil_Out32(0xf6100288, 0xffff5555);
    Xil_Out32(0xf6100220, 0x0000aaaa);
    Xil_Out32(0xf610000c, 0x00000000);
    Xil_Out32(0xf610200c, 0xf9e8d7c6);
    Xil_Out32(0xf6102000, 0x0000023e);
    Xil_Out32(0xf6102004, 0x00000200);
    Xil_Out32(0xf61023a0, 0x00000080);
    Xil_Out32(0xf61021c4, 0xaaaa0000);
    Xil_Out32(0xf61021fc, 0x5555ffff);
    Xil_Out32(0xf61021c0, 0xffffaaaa);
    Xil_Out32(0xf610223c, 0x55550000);
    Xil_Out32(0xf61021c0, 0xffffaaaa);
    Xil_Out32(0xf610222c, 0x55550000);
    Xil_Out32(0xf6102284, 0x55550000);
    Xil_Out32(0xf61021f8, 0xffffaaaa);
    Xil_Out32(0xf6102280, 0xffff5555);
    Xil_Out32(0xf6102238, 0x0000aaaa);
    Xil_Out32(0xf6102280, 0xffff5555);
    Xil_Out32(0xf6102228, 0x0000aaaa);
    Xil_Out32(0xf610200c, 0x00000000);
    Xil_Out32(0xf617000c, 0xf9e8d7c6);
    Xil_Out32(0xf6170000, 0x0000023e);
    Xil_Out32(0xf6170004, 0x00000200);
    Xil_Out32(0xf61703a0, 0x00000090);
    Xil_Out32(0xf6170204, 0x55550000);
    Xil_Out32(0xf61701c4, 0xaaaa0000);
    Xil_Out32(0xf617021c, 0x5555ffff);
    Xil_Out32(0xf6170254, 0x55555555);
    Xil_Out32(0xf61701c0, 0xaaaaaaaa);
    Xil_Out32(0xf6170264, 0x55550000);
    Xil_Out32(0xf617026c, 0x55550000);
    Xil_Out32(0xf6170274, 0x00000000);
    Xil_Out32(0xf6170218, 0xffffffff);
    Xil_Out32(0xf617000c, 0x00000000);
    Xil_Out32(0xf617200c, 0xf9e8d7c6);
    Xil_Out32(0xf6172000, 0x0000023e);
    Xil_Out32(0xf6172004, 0x00000200);
    Xil_Out32(0xf61723a0, 0x000000a0);
    Xil_Out32(0xf617220c, 0x55550000);
    Xil_Out32(0xf61721c0, 0xaaaaaaaa);
    Xil_Out32(0xf6172214, 0x55555555);
    Xil_Out32(0xf617225c, 0x5555ffff);
    Xil_Out32(0xf61721c4, 0xaaaa0000);
    Xil_Out32(0xf6172278, 0x0000aaaa);
    Xil_Out32(0xf6172210, 0xffff5555);
    Xil_Out32(0xf617200c, 0x00000000);
    Xil_Out32(0xf61b000c, 0xf9e8d7c6);
    Xil_Out32(0xf61b0000, 0x0000023e);
    Xil_Out32(0xf61b0004, 0x00000200);
    Xil_Out32(0xf61b03a0, 0x000000b0);
    Xil_Out32(0xf61b01f4, 0x55550000);
    Xil_Out32(0xf61b0234, 0x55550000);
    Xil_Out32(0xf61b0224, 0x55550000);
    Xil_Out32(0xf61b0244, 0x55555555);
    Xil_Out32(0xf61b01c0, 0xaaaaaaaa);
    Xil_Out32(0xf61b000c, 0x00000000);
    Xil_Out32(0xf61b200c, 0xf9e8d7c6);
    Xil_Out32(0xf61b2000, 0x0000023e);
    Xil_Out32(0xf61b2004, 0x00000200);
    Xil_Out32(0xf61b23a0, 0x000000c0);
    Xil_Out32(0xf61b21c0, 0xffffaaaa);
    Xil_Out32(0xf61b21fc, 0x55550000);
    Xil_Out32(0xf61b21c0, 0xffffaaaa);
    Xil_Out32(0xf61b223c, 0x55550000);
    Xil_Out32(0xf61b21c0, 0xffffaaaa);
    Xil_Out32(0xf61b222c, 0x55550000);
    Xil_Out32(0xf61b224c, 0x5555ffff);
    Xil_Out32(0xf61b21c4, 0xaaaa0000);
    Xil_Out32(0xf61b200c, 0x00000000);
    Xil_Out32(0xf61d000c, 0xf9e8d7c6);
    Xil_Out32(0xf61d0000, 0x0000023e);
    Xil_Out32(0xf61d0004, 0x00000200);
    Xil_Out32(0xf61d03a0, 0x00000100);
    Xil_Out32(0xf61d0204, 0x00000000);
    Xil_Out32(0xf61d021c, 0x00000000);
    Xil_Out32(0xf61d0254, 0x00000000);
    Xil_Out32(0xf61d0264, 0x00000000);
    Xil_Out32(0xf61d026c, 0x00000000);
    Xil_Out32(0xf61d01c0, 0xffffffff);
    Xil_Out32(0xf61d000c, 0x00000000);
    Xil_Out32(0xf61d200c, 0xf9e8d7c6);
    Xil_Out32(0xf61d2000, 0x0000023e);
    Xil_Out32(0xf61d2004, 0x00000200);
    Xil_Out32(0xf61d23a0, 0x00000100);
    Xil_Out32(0xf61d2200, 0xffff5555);
    Xil_Out32(0xf61d2218, 0xffff5555);
    Xil_Out32(0xf61d2250, 0xffff5555);
    Xil_Out32(0xf61d2260, 0xffff5555);
    Xil_Out32(0xf61d2268, 0xffff5555);
    Xil_Out32(0xf61d21c0, 0x0000aaaa);
    Xil_Out32(0xf61d2208, 0xffffffff);
    Xil_Out32(0xf61d2210, 0xffffffff);
    Xil_Out32(0xf61d2258, 0xffffffff);
    Xil_Out32(0xf61d21c4, 0x00000000);
    Xil_Out32(0xf61d200c, 0x00000000);
    Xil_Out32(0xf61d400c, 0xf9e8d7c6);
    Xil_Out32(0xf61d4000, 0x0000023e);
    Xil_Out32(0xf61d4004, 0x00000200);
    Xil_Out32(0xf61d43a0, 0x00000100);
    Xil_Out32(0xf61d4208, 0x0000aaaa);
    Xil_Out32(0xf61d4210, 0x0000aaaa);
    Xil_Out32(0xf61d4258, 0x0000aaaa);
    Xil_Out32(0xf61d41c0, 0xffff5555);
    Xil_Out32(0xf61d400c, 0x00000000);
    Xil_Out32(0xf620000c, 0xf9e8d7c6);
    Xil_Out32(0xf6200000, 0x0000023e);
    Xil_Out32(0xf6200004, 0x00000200);
    Xil_Out32(0xf62003a0, 0x00000100);
    Xil_Out32(0xf62001f4, 0x00000000);
    Xil_Out32(0xf6200224, 0x00000000);
    Xil_Out32(0xf6200234, 0x00000000);
    Xil_Out32(0xf6200244, 0x00000000);
    Xil_Out32(0xf62001c0, 0xffffffff);
    Xil_Out32(0xf620000c, 0x00000000);
    Xil_Out32(0xf620200c, 0xf9e8d7c6);
    Xil_Out32(0xf6202000, 0x0000023e);
    Xil_Out32(0xf6202004, 0x00000200);
    Xil_Out32(0xf62023a0, 0x00000100);
    Xil_Out32(0xf62021f4, 0x55555555);
    Xil_Out32(0xf6202224, 0x55555555);
    Xil_Out32(0xf6202234, 0x55555555);
    Xil_Out32(0xf6202244, 0x55555555);
    Xil_Out32(0xf62021c0, 0xaaaaaaaa);
    Xil_Out32(0xf62021fc, 0x5555ffff);
    Xil_Out32(0xf620222c, 0x5555ffff);
    Xil_Out32(0xf620223c, 0x5555ffff);
    Xil_Out32(0xf620224c, 0x5555ffff);
    Xil_Out32(0xf62021c4, 0xaaaa0000);
    Xil_Out32(0xf620200c, 0x00000000);
    Xil_Out32(0xf620400c, 0xf9e8d7c6);
    Xil_Out32(0xf6204000, 0x0000023e);
    Xil_Out32(0xf6204004, 0x00000200);
    Xil_Out32(0xf62043a0, 0x00000100);
    Xil_Out32(0xf62041f8, 0x0000aaaa);
    Xil_Out32(0xf6204228, 0x0000aaaa);
    Xil_Out32(0xf6204238, 0x0000aaaa);
    Xil_Out32(0xf6204248, 0x0000aaaa);
    Xil_Out32(0xf62041c0, 0xffff5555);
    Xil_Out32(0xf620400c, 0x00000000);
    Xil_Out32(0xf6e6000c, 0xf9e8d7c6);
    Xil_Out32(0xf6e60000, 0x010382cc);
    Xil_Out32(0xf6e60004, 0x01030200);
    Xil_Out32(0xf6e6083c, 0x00000100);
    Xil_Out32(0xf6e60840, 0x00001fff);
    Xil_Out32(0xf6e60844, 0x00000100);
    Xil_Out32(0xf6e60848, 0x00001fff);
    Xil_Out32(0xf6e60858, 0x00000002);
    Xil_Out32(0xf6e60860, 0x00000002);
    Xil_Out32(0xf6e60864, 0x00000001);
    Xil_Out32(0xf6e60868, 0x00000001);
    Xil_Out32(0xf6e6086c, 0x00000003);
    Xil_Out32(0xf6e60454, 0x00000006);
    Xil_Out32(0xf6e602c8, 0x00002e70);
    Xil_Out32(0xf6e60450, 0x00000600);
    Xil_Out32(0xf6e602c4, 0x00000000);
    Xil_Out32(0xf6e6040c, 0x00000b42);
    Xil_Out32(0xf6e60410, 0x00000b42);
    Xil_Out32(0xf6e6041c, 0x00000b42);
    Xil_Out32(0xf6e60420, 0x00000b42);
    Xil_Out32(0xf6e60424, 0x00000b42);
    Xil_Out32(0xf6e60428, 0x00000b42);
    Xil_Out32(0xf6e603d0, 0x00000b42);
    Xil_Out32(0xf6e603f0, 0x00000000);
    Xil_Out32(0xf6e60414, 0x00000b02);
    Xil_Out32(0xf6e60418, 0x00000b02);
    Xil_Out32(0xf6e60000, 0x00000002);
    Xil_Out32(0xf6e60004, 0x00000000);
    Xil_Out32(0xf6e6000c, 0x00000000);
    Xil_Out32(0xf6e6200c, 0xf9e8d7c6);
    Xil_Out32(0xf6e62000, 0x000002ff);
    Xil_Out32(0xf6e62004, 0x00000200);
    Xil_Out32(0xf6e62100, 0x00000608);
    Xil_Out32(0xf6e62104, 0x00000006);
    Xil_Out32(0xf6e62108, 0x00000000);
    Xil_Out32(0xf6e6200c, 0x00000000);
    Xil_Out32(0xf6e6400c, 0xf9e8d7c6);
    Xil_Out32(0xf6e64000, 0x0000023e);
    Xil_Out32(0xf6e64004, 0x00000200);
    Xil_Out32(0xf6e643a0, 0x00000180);
    Xil_Out32(0xf6e64200, 0xaaaaaaaa);
    Xil_Out32(0xf6e64364, 0x55555555);
    Xil_Out32(0xf6e64208, 0xaaaaaaaa);
    Xil_Out32(0xf6e6436c, 0x55550000);
    Xil_Out32(0xf6e64210, 0xffffaaaa);
    Xil_Out32(0xf6e64374, 0x55550000);
    Xil_Out32(0xf6e64218, 0xffffaaaa);
    Xil_Out32(0xf6e64250, 0xffffaaaa);
    Xil_Out32(0xf6e64344, 0x55550000);
    Xil_Out32(0xf6e64258, 0xffffaaaa);
    Xil_Out32(0xf6e6434c, 0x55550000);
    Xil_Out32(0xf6e64260, 0xffffaaaa);
    Xil_Out32(0xf6e6432c, 0x55550000);
    Xil_Out32(0xf6e6426c, 0xaaaa0000);
    Xil_Out32(0xf6e64324, 0x5555ffff);
    Xil_Out32(0xf6e6400c, 0x00000000);
    Xil_Out32(0xf6e6600c, 0xf9e8d7c6);
    Xil_Out32(0xf6e66000, 0x0000023e);
    Xil_Out32(0xf6e66004, 0x00000200);
    Xil_Out32(0xf6e663a0, 0x00000180);
    Xil_Out32(0xf6e661f4, 0x00000000);
    Xil_Out32(0xf6e66360, 0xffffffff);
    Xil_Out32(0xf6e661fc, 0x00000000);
    Xil_Out32(0xf6e66204, 0x55550000);
    Xil_Out32(0xf6e6620c, 0x55550000);
    Xil_Out32(0xf6e66224, 0x55550000);
    Xil_Out32(0xf6e66388, 0xffffaaaa);
    Xil_Out32(0xf6e6622c, 0x55550000);
    Xil_Out32(0xf6e66380, 0xffffaaaa);
    Xil_Out32(0xf6e66234, 0x55550000);
    Xil_Out32(0xf6e66378, 0xffffaaaa);
    Xil_Out32(0xf6e6623c, 0x55550000);
    Xil_Out32(0xf6e66244, 0x55550000);
    Xil_Out32(0xf6e66330, 0xffffaaaa);
    Xil_Out32(0xf6e6624c, 0x55550000);
    Xil_Out32(0xf6e66338, 0xffffaaaa);
    Xil_Out32(0xf6e6600c, 0x00000000);
    Xil_Out32(0xf6e9000c, 0xf9e8d7c6);
    Xil_Out32(0xf6e90000, 0x010382cc);
    Xil_Out32(0xf6e90004, 0x00020000);
    Xil_Out32(0xf6e9083c, 0x00000100);
    Xil_Out32(0xf6e90840, 0x00001fff);
    Xil_Out32(0xf6e90844, 0x00000100);
    Xil_Out32(0xf6e90848, 0x00001fff);
    Xil_Out32(0xf6e90858, 0x00000002);
    Xil_Out32(0xf6e90860, 0x00000002);
    Xil_Out32(0xf6e90864, 0x00000001);
    Xil_Out32(0xf6e90868, 0x00000001);
    Xil_Out32(0xf6e9086c, 0x00000003);
    Xil_Out32(0xf6e90454, 0x00000006);
    Xil_Out32(0xf6e902c8, 0x00000330);
    Xil_Out32(0xf6e90450, 0x00000601);
    Xil_Out32(0xf6e902c4, 0x00000000);
    Xil_Out32(0xf6e9040c, 0x00000b02);
    Xil_Out32(0xf6e90410, 0x00000b02);
    Xil_Out32(0xf6e9041c, 0x00000b02);
    Xil_Out32(0xf6e90420, 0x00000b02);
    Xil_Out32(0xf6e90424, 0x00000b02);
    Xil_Out32(0xf6e90428, 0x00000b02);
    Xil_Out32(0xf6e903d0, 0x00000b02);
    Xil_Out32(0xf6e903f0, 0x00000000);
    Xil_Out32(0xf6e90414, 0x00000b02);
    Xil_Out32(0xf6e90418, 0x00000b02);
    Xil_Out32(0xf6e90000, 0x00000002);
    Xil_Out32(0xf6e90004, 0x00000000);
    Xil_Out32(0xf6e9000c, 0x00000000);
    Xil_Out32(0xf6e9200c, 0xf9e8d7c6);
    Xil_Out32(0xf6e92000, 0x000002ff);
    Xil_Out32(0xf6e92004, 0x00000200);
    Xil_Out32(0xf6e92100, 0x00000609);
    Xil_Out32(0xf6e92104, 0x00000006);
    Xil_Out32(0xf6e92108, 0x00000000);
    Xil_Out32(0xf6e9200c, 0x00000000);
    Xil_Out32(0xf6e9400c, 0xf9e8d7c6);
    Xil_Out32(0xf6e94000, 0x0000023e);
    Xil_Out32(0xf6e94004, 0x00000200);
    Xil_Out32(0xf6e943a0, 0x00000180);
    Xil_Out32(0xf6e9436c, 0x55555555);
    Xil_Out32(0xf6e94210, 0xaaaaaaaa);
    Xil_Out32(0xf6e94374, 0x55555555);
    Xil_Out32(0xf6e94218, 0xaaaaaaaa);
    Xil_Out32(0xf6e94250, 0xffffaaaa);
    Xil_Out32(0xf6e94344, 0x55550000);
    Xil_Out32(0xf6e94258, 0xffffaaaa);
    Xil_Out32(0xf6e9434c, 0x55550000);
    Xil_Out32(0xf6e94264, 0xaaaa0000);
    Xil_Out32(0xf6e9432c, 0x5555ffff);
    Xil_Out32(0xf6e9400c, 0x00000000);
    Xil_Out32(0xf6e9600c, 0xf9e8d7c6);
    Xil_Out32(0xf6e96000, 0x0000023e);
    Xil_Out32(0xf6e96004, 0x00000200);
    Xil_Out32(0xf6e963a0, 0x00000180);
    Xil_Out32(0xf6e96368, 0xffffaaaa);
    Xil_Out32(0xf6e96214, 0x55550000);
    Xil_Out32(0xf6e96370, 0xaaaaaaaa);
    Xil_Out32(0xf6e9621c, 0x55555555);
    Xil_Out32(0xf6e96224, 0x55550000);
    Xil_Out32(0xf6e96388, 0xffffaaaa);
    Xil_Out32(0xf6e9622c, 0x55550000);
    Xil_Out32(0xf6e96380, 0xffffaaaa);
    Xil_Out32(0xf6e96234, 0x55550000);
    Xil_Out32(0xf6e96378, 0xffffaaaa);
    Xil_Out32(0xf6e9623c, 0x55550000);
    Xil_Out32(0xf6e96244, 0x55550000);
    Xil_Out32(0xf6e96330, 0xffffaaaa);
    Xil_Out32(0xf6e9624c, 0x55550000);
    Xil_Out32(0xf6e96338, 0xffffaaaa);
    Xil_Out32(0xf6e9600c, 0x00000000);
    Xil_Out32(0xf6ec000c, 0xf9e8d7c6);
    Xil_Out32(0xf6ec0000, 0x010382cc);
    Xil_Out32(0xf6ec0004, 0x01038200);
    Xil_Out32(0xf6ec083c, 0x00000100);
    Xil_Out32(0xf6ec0840, 0x00001fff);
    Xil_Out32(0xf6ec0844, 0x00000100);
    Xil_Out32(0xf6ec0848, 0x00001fff);
    Xil_Out32(0xf6ec0858, 0x00000002);
    Xil_Out32(0xf6ec0860, 0x00000002);
    Xil_Out32(0xf6ec0864, 0x00000001);
    Xil_Out32(0xf6ec0868, 0x00000001);
    Xil_Out32(0xf6ec086c, 0x00000003);
    Xil_Out32(0xf6ec0454, 0x00000006);
    Xil_Out32(0xf6ec02c8, 0x0000ff80);
    Xil_Out32(0xf6ec0450, 0x00000602);
    Xil_Out32(0xf6ec02c4, 0x00000000);
    Xil_Out32(0xf6ec040c, 0x00000a02);
    Xil_Out32(0xf6ec0410, 0x00000a02);
    Xil_Out32(0xf6ec041c, 0x00000a02);
    Xil_Out32(0xf6ec0420, 0x00000a02);
    Xil_Out32(0xf6ec0424, 0x00000a02);
    Xil_Out32(0xf6ec0428, 0x00000a02);
    Xil_Out32(0xf6ec03d0, 0x00000a02);
    Xil_Out32(0xf6ec03f0, 0x00000000);
    Xil_Out32(0xf6ec0414, 0x00000a02);
    Xil_Out32(0xf6ec0418, 0x00000a02);
    Xil_Out32(0xf6ec0000, 0x00000002);
    Xil_Out32(0xf6ec0004, 0x00000000);
    Xil_Out32(0xf6ec000c, 0x00000000);
    Xil_Out32(0xf6ec200c, 0xf9e8d7c6);
    Xil_Out32(0xf6ec2000, 0x000002ff);
    Xil_Out32(0xf6ec2004, 0x00000200);
    Xil_Out32(0xf6ec2100, 0x0000060a);
    Xil_Out32(0xf6ec2104, 0x00000006);
    Xil_Out32(0xf6ec2108, 0x00000000);
    Xil_Out32(0xf6ec200c, 0x00000000);
    Xil_Out32(0xf6ec400c, 0xf9e8d7c6);
    Xil_Out32(0xf6ec4000, 0x0000023e);
    Xil_Out32(0xf6ec4004, 0x00000200);
    Xil_Out32(0xf6ec43a0, 0x00000180);
    Xil_Out32(0xf6ec4244, 0x55550000);
    Xil_Out32(0xf6ec4330, 0xffffaaaa);
    Xil_Out32(0xf6ec4248, 0xffff5555);
    Xil_Out32(0xf6ec4338, 0x0000aaaa);
    Xil_Out32(0xf6ec4250, 0xffffaaaa);
    Xil_Out32(0xf6ec4344, 0x55550000);
    Xil_Out32(0xf6ec4258, 0xffffaaaa);
    Xil_Out32(0xf6ec434c, 0x55550000);
    Xil_Out32(0xf6ec400c, 0x00000000);
    Xil_Out32(0xf6ec600c, 0xf9e8d7c6);
    Xil_Out32(0xf6ec6000, 0x0000023e);
    Xil_Out32(0xf6ec6004, 0x00000200);
    Xil_Out32(0xf6ec63a0, 0x00000180);
    Xil_Out32(0xf6ec6370, 0xffffffff);
    Xil_Out32(0xf6ec621c, 0x00000000);
    Xil_Out32(0xf6ec6224, 0x55550000);
    Xil_Out32(0xf6ec6388, 0xffffaaaa);
    Xil_Out32(0xf6ec622c, 0x55550000);
    Xil_Out32(0xf6ec6380, 0xffffaaaa);
    Xil_Out32(0xf6ec6234, 0x55550000);
    Xil_Out32(0xf6ec6378, 0xffffaaaa);
    Xil_Out32(0xf6ec623c, 0x55550000);
    Xil_Out32(0xf6ec6240, 0x0000aaaa);
    Xil_Out32(0xf6ec6330, 0xffff5555);
    Xil_Out32(0xf6ec6248, 0x0000aaaa);
    Xil_Out32(0xf6ec6338, 0xffff5555);
    Xil_Out32(0xf6ec600c, 0x00000000);
    Xil_Out32(0xf6ee000c, 0xf9e8d7c6);
    Xil_Out32(0xf6ee0000, 0x010382cc);
    Xil_Out32(0xf6ee0004, 0x00030200);
    Xil_Out32(0xf6ee083c, 0x00000100);
    Xil_Out32(0xf6ee0840, 0x00001fff);
    Xil_Out32(0xf6ee0844, 0x00000100);
    Xil_Out32(0xf6ee0848, 0x00001fff);
    Xil_Out32(0xf6ee0858, 0x00000002);
    Xil_Out32(0xf6ee0860, 0x00000002);
    Xil_Out32(0xf6ee0864, 0x00000001);
    Xil_Out32(0xf6ee0868, 0x00000001);
    Xil_Out32(0xf6ee086c, 0x00000003);
    Xil_Out32(0xf6ee0454, 0x00000006);
    Xil_Out32(0xf6ee02c8, 0x0000f030);
    Xil_Out32(0xf6ee0450, 0x00000603);
    Xil_Out32(0xf6ee02c4, 0x00000000);
    Xil_Out32(0xf6ee040c, 0x00000a42);
    Xil_Out32(0xf6ee0410, 0x00000a42);
    Xil_Out32(0xf6ee041c, 0x00000a42);
    Xil_Out32(0xf6ee0420, 0x00000a42);
    Xil_Out32(0xf6ee0424, 0x00000a42);
    Xil_Out32(0xf6ee0428, 0x00000a42);
    Xil_Out32(0xf6ee03d0, 0x00000a42);
    Xil_Out32(0xf6ee03f0, 0x00000000);
    Xil_Out32(0xf6ee0414, 0x00000a42);
    Xil_Out32(0xf6ee0418, 0x00000a42);
    Xil_Out32(0xf6ee0000, 0x00000002);
    Xil_Out32(0xf6ee0004, 0x00000000);
    Xil_Out32(0xf6ee000c, 0x00000000);
    Xil_Out32(0xf6ee200c, 0xf9e8d7c6);
    Xil_Out32(0xf6ee2000, 0x000002ff);
    Xil_Out32(0xf6ee2004, 0x00000200);
    Xil_Out32(0xf6ee2100, 0x0000060b);
    Xil_Out32(0xf6ee2104, 0x00000006);
    Xil_Out32(0xf6ee2108, 0x00000000);
    Xil_Out32(0xf6ee200c, 0x00000000);
    Xil_Out32(0xf6ee400c, 0xf9e8d7c6);
    Xil_Out32(0xf6ee4000, 0x0000023e);
    Xil_Out32(0xf6ee4004, 0x00000200);
    Xil_Out32(0xf6ee43a0, 0x00000180);
    Xil_Out32(0xf6ee424c, 0xaaaa0000);
    Xil_Out32(0xf6ee433c, 0x5555ffff);
    Xil_Out32(0xf6ee4250, 0xffffaaaa);
    Xil_Out32(0xf6ee4344, 0x55550000);
    Xil_Out32(0xf6ee4258, 0xffffaaaa);
    Xil_Out32(0xf6ee434c, 0x55550000);
    Xil_Out32(0xf6ee400c, 0x00000000);
    Xil_Out32(0xf6ee600c, 0xf9e8d7c6);
    Xil_Out32(0xf6ee6000, 0x0000023e);
    Xil_Out32(0xf6ee6004, 0x00000200);
    Xil_Out32(0xf6ee63a0, 0x00000180);
    Xil_Out32(0xf6ee6224, 0x55550000);
    Xil_Out32(0xf6ee6388, 0xffffaaaa);
    Xil_Out32(0xf6ee622c, 0x55550000);
    Xil_Out32(0xf6ee6380, 0xffffaaaa);
    Xil_Out32(0xf6ee6234, 0x55550000);
    Xil_Out32(0xf6ee6378, 0xffffffff);
    Xil_Out32(0xf6ee623c, 0x00000000);
    Xil_Out32(0xf6ee600c, 0x00000000);
    Xil_Out32(0xf6f1000c, 0xf9e8d7c6);
    Xil_Out32(0xf6f10000, 0x010382cc);
    Xil_Out32(0xf6f10004, 0x01018200);
    Xil_Out32(0xf6f1083c, 0x00000100);
    Xil_Out32(0xf6f10840, 0x00001fff);
    Xil_Out32(0xf6f10844, 0x00000100);
    Xil_Out32(0xf6f10848, 0x00001fff);
    Xil_Out32(0xf6f10858, 0x00000002);
    Xil_Out32(0xf6f10860, 0x00000002);
    Xil_Out32(0xf6f10864, 0x00000001);
    Xil_Out32(0xf6f10868, 0x00000001);
    Xil_Out32(0xf6f1086c, 0x00000003);
    Xil_Out32(0xf6f10454, 0x00000006);
    Xil_Out32(0xf6f102c8, 0x00000640);
    Xil_Out32(0xf6f10450, 0x00000604);
    Xil_Out32(0xf6f1040c, 0x00000a82);
    Xil_Out32(0xf6f10410, 0x00000a82);
    Xil_Out32(0xf6f1041c, 0x00000a82);
    Xil_Out32(0xf6f10420, 0x00000a82);
    Xil_Out32(0xf6f10424, 0x00000a82);
    Xil_Out32(0xf6f10428, 0x00000a82);
    Xil_Out32(0xf6f103d0, 0x00000a82);
    Xil_Out32(0xf6f103f0, 0x00000000);
    Xil_Out32(0xf6f10414, 0x00000a82);
    Xil_Out32(0xf6f10418, 0x00000a82);
    Xil_Out32(0xf6f10000, 0x00000002);
    Xil_Out32(0xf6f10004, 0x00000000);
    Xil_Out32(0xf6f1000c, 0x00000000);
    Xil_Out32(0xf6f1200c, 0xf9e8d7c6);
    Xil_Out32(0xf6f12000, 0x000002ff);
    Xil_Out32(0xf6f12004, 0x00000200);
    Xil_Out32(0xf6f12100, 0x0000060c);
    Xil_Out32(0xf6f12104, 0x00000006);
    Xil_Out32(0xf6f12108, 0x00000000);
    Xil_Out32(0xf6f1200c, 0x00000000);
    Xil_Out32(0xf6f1400c, 0xf9e8d7c6);
    Xil_Out32(0xf6f14000, 0x0000023e);
    Xil_Out32(0xf6f14004, 0x00000200);
    Xil_Out32(0xf6f143a0, 0x00000180);
    Xil_Out32(0xf6f14254, 0xaaaa0000);
    Xil_Out32(0xf6f14344, 0x5555ffff);
    Xil_Out32(0xf6f14258, 0xffffaaaa);
    Xil_Out32(0xf6f1434c, 0x55550000);
    Xil_Out32(0xf6f1400c, 0x00000000);
    Xil_Out32(0xf6f1600c, 0xf9e8d7c6);
    Xil_Out32(0xf6f16000, 0x0000023e);
    Xil_Out32(0xf6f16004, 0x00000200);
    Xil_Out32(0xf6f163a0, 0x00000180);
    Xil_Out32(0xf6f16224, 0x55550000);
    Xil_Out32(0xf6f16388, 0xffffaaaa);
    Xil_Out32(0xf6f1622c, 0x55550000);
    Xil_Out32(0xf6f16380, 0xffffffff);
    Xil_Out32(0xf6f16234, 0x00000000);
    Xil_Out32(0xf6f1600c, 0x00000000);
    Xil_Out32(0xf6f3000c, 0xf9e8d7c6);
    Xil_Out32(0xf6f30000, 0x010382cc);
    Xil_Out32(0xf6f30004, 0x01010200);
    Xil_Out32(0xf6f3083c, 0x00000100);
    Xil_Out32(0xf6f30840, 0x00001fff);
    Xil_Out32(0xf6f30844, 0x00000100);
    Xil_Out32(0xf6f30848, 0x00001fff);
    Xil_Out32(0xf6f30858, 0x00000002);
    Xil_Out32(0xf6f30860, 0x00000002);
    Xil_Out32(0xf6f30864, 0x00000001);
    Xil_Out32(0xf6f30868, 0x00000001);
    Xil_Out32(0xf6f3086c, 0x00000003);
    Xil_Out32(0xf6f30454, 0x00000006);
    Xil_Out32(0xf6f302c8, 0x00007bd8);
    Xil_Out32(0xf6f30450, 0x00000605);
    Xil_Out32(0xf6f302c4, 0x00000000);
    Xil_Out32(0xf6f3040c, 0x00000ac2);
    Xil_Out32(0xf6f30410, 0x00000ac2);
    Xil_Out32(0xf6f3041c, 0x00000ac2);
    Xil_Out32(0xf6f30420, 0x00000ac2);
    Xil_Out32(0xf6f30424, 0x00000ac2);
    Xil_Out32(0xf6f30428, 0x00000ac2);
    Xil_Out32(0xf6f303d0, 0x00000ac2);
    Xil_Out32(0xf6f303f0, 0x00000000);
    Xil_Out32(0xf6f30414, 0x00000ac2);
    Xil_Out32(0xf6f30418, 0x00000ac2);
    Xil_Out32(0xf6f30000, 0x00000002);
    Xil_Out32(0xf6f30004, 0x00000000);
    Xil_Out32(0xf6f3000c, 0x00000000);
    Xil_Out32(0xf6f3200c, 0xf9e8d7c6);
    Xil_Out32(0xf6f32000, 0x000002ff);
    Xil_Out32(0xf6f32004, 0x00000200);
    Xil_Out32(0xf6f32100, 0x0000060d);
    Xil_Out32(0xf6f32104, 0x00000006);
    Xil_Out32(0xf6f32108, 0x00000001);
    Xil_Out32(0xf6f3200c, 0x00000000);
    Xil_Out32(0xf6f3400c, 0xf9e8d7c6);
    Xil_Out32(0xf6f34000, 0x0000023e);
    Xil_Out32(0xf6f34004, 0x00000200);
    Xil_Out32(0xf6f343a0, 0x00000180);
    Xil_Out32(0xf6f3425c, 0xaaaa0000);
    Xil_Out32(0xf6f3434c, 0x5555ffff);
    Xil_Out32(0xf6f3400c, 0x00000000);
    Xil_Out32(0xf6f3600c, 0xf9e8d7c6);
    Xil_Out32(0xf6f36000, 0x0000023e);
    Xil_Out32(0xf6f36004, 0x00000200);
    Xil_Out32(0xf6f363a0, 0x00000180);
    Xil_Out32(0xf6f36388, 0xffffffff);
    Xil_Out32(0xf6f36224, 0x00000000);
    Xil_Out32(0xf6f3622c, 0x00000000);
    Xil_Out32(0xf6f36388, 0xffffffff);
    Xil_Out32(0xf6f3600c, 0x00000000);
    Xil_Out32(0xf607000c, 0xf9e8d7c6);
    Xil_Out32(0xf6070010, 0x06170651);
    Xil_Out32(0xf6070014, 0x00637265);
    Xil_Out32(0xf6070018, 0x06170651);
    Xil_Out32(0xf607001c, 0x00637265);
    Xil_Out32(0xf6070020, 0x06170651);
    Xil_Out32(0xf6070024, 0x00637265);
    Xil_Out32(0xf6070028, 0x06170651);
    Xil_Out32(0xf607002c, 0x00637265);
    Xil_Out32(0xf607044c, 0x00421084);
    Xil_Out32(0xf6070448, 0x0003fcff);
    Xil_Out32(0xf6070444, 0x0ff3fcff);
    Xil_Out32(0xf607046c, 0x000fffff);
    Xil_Out32(0xf6070468, 0x000fffff);
    Xil_Out32(0xf6070464, 0x000fffff);
    Xil_Out32(0xf6070460, 0x000fffff);
    Xil_Out32(0xf607045c, 0x000fffff);
    Xil_Out32(0xf6070480, 0x000fffff);
    Xil_Out32(0xf607047c, 0x000fffff);
    Xil_Out32(0xf6070478, 0x000fffff);
    Xil_Out32(0xf6070474, 0x000fffff);
    Xil_Out32(0xf6070470, 0x000fffff);
    Xil_Out32(0xf6070494, 0x000fffff);
    Xil_Out32(0xf6070490, 0x000fffff);
    Xil_Out32(0xf607048c, 0x000fffff);
    Xil_Out32(0xf6070488, 0x000fffff);
    Xil_Out32(0xf6070484, 0x000fffff);
    Xil_Out32(0xf60704a8, 0x000fffff);
    Xil_Out32(0xf60704a4, 0x000fffff);
    Xil_Out32(0xf60704a0, 0x000fffff);
    Xil_Out32(0xf607049c, 0x000fffff);
    Xil_Out32(0xf6070498, 0x000fffff);
    Xil_Out32(0xf6070030, 0x01ffffff);
    Xil_Out32(0xf6070034, 0x009996cd);
    Xil_Out32(0xf60704ac, 0x003fffff);
    Xil_Out32(0xf60704b0, 0x003fffff);
    Xil_Out32(0xf60704b4, 0x003fffff);
    Xil_Out32(0xf60704b8, 0x003fffff);
    Xil_Out32(0xf60704bc, 0x003fffff);
    Xil_Out32(0xf6070418, 0x0ffffff0);
    Xil_Out32(0xf6070410, 0x0ffffff0);
    Xil_Out32(0xf6070408, 0x0ffffff0);
    Xil_Out32(0xf6070400, 0x0ffffff0);
    Xil_Out32(0xf607041c, 0x000ffff0);
    Xil_Out32(0xf6070414, 0x000ffff0);
    Xil_Out32(0xf607040c, 0x000ffff0);
    Xil_Out32(0xf6070404, 0x000ffff0);
    Xil_Out32(0xf6070438, 0x00ffffff);
    Xil_Out32(0xf6070430, 0x00ffffff);
    Xil_Out32(0xf6070428, 0x00ffffff);
    Xil_Out32(0xf6070420, 0x00ffffff);
    Xil_Out32(0xf607043c, 0x0000ffff);
    Xil_Out32(0xf6070434, 0x0000ffff);
    Xil_Out32(0xf607042c, 0x0000ffff);
    Xil_Out32(0xf6070424, 0x0000ffff);
    Xil_Out32(0xf6070068, 0x00000b83);
    Xil_Out32(0xf607006c, 0x00000bc3);
    Xil_Out32(0xf6070070, 0x00000c03);
    Xil_Out32(0xf6070074, 0x00000c43);
    //**************Register programming from noc_cfg end---------------- 

    dbg0_pmc(16385);
     //--------------------------------------
    //**************Register programming from  xphy_pll start---------------- 

    //************************************************************************************************* XPIO_PLL : begin

    Xil_Out32(0xf616900c, 0xf9e8d7c6);
    prog_reg(0xF6169038, 0xC, 0x1000, 0x1);
    prog_reg(0xF616903C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF616903C, 0x0, 0xFF, 0x2);
    prog_reg(0xF6169040, 0xC, 0x1000, 0x1);
    prog_reg(0xF6169040, 0xB, 0x800, 0x0);
    prog_reg(0xF6169038, 0xB, 0x800, 0x0);
    prog_reg(0xF6169048, 0xB, 0x800, 0x0);
    prog_reg(0xF6169050, 0xB, 0x800, 0x0);
    prog_reg(0xF6169044, 0x8, 0xFF00, 0x2);
    prog_reg(0xF6169044, 0x0, 0xFF, 0x2);
    prog_reg(0xF6169048, 0xC, 0x1000, 0x1);
    prog_reg(0xF616904C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF616904C, 0x0, 0xFF, 0x2);
    prog_reg(0xF6169030, 0xD, 0xE000, 0x0);
    prog_reg(0xF6169050, 0xC, 0x1000, 0x1);
    prog_reg(0xF6169054, 0x8, 0xFF00, 0x2);
    prog_reg(0xF6169054, 0x0, 0xFF, 0x2);
    prog_reg(0xF6169034, 0x8, 0xFF00, 0x10);
    prog_reg(0xF6169034, 0x0, 0xFF, 0x10);
    prog_reg(0xF6169030, 0x9, 0x200, 0x1);
    prog_reg(0xF6169030, 0xA, 0xC00, 0x1);
    prog_reg(0xF6169030, 0x0, 0xFF, 0x0);
    prog_reg(0xF6169030, 0x8, 0x100, 0x0);
    prog_reg(0xF6169030, 0xC, 0x1000, 0x0);
    prog_reg(0xF6169088, 0x5, 0x20, 0x0);
    prog_reg(0xF6169094, 0xA, 0x7C00, 0x3);
    prog_reg(0xF6169098, 0x2, 0x7C, 0x3);
    Xil_Out32(0xf616980c, 0xf9e8d7c6);
    prog_reg(0xF6169810, 0xB, 0x1800, 0x3);
    prog_reg(0xF6169810, 0x8, 0x700, 0x3);
    prog_reg(0xF6169810, 0x5, 0xE0, 0x0);
    prog_reg(0xF6169810, 0x2, 0x1C, 0x0);
    prog_reg(0xF6169000, 0x1, 0x2, 0x1);
    Xil_Out32(0xf6169004, 0x00000000);
    prog_reg(0xF6169000, 0x6, 0x40, 0x1);
    Xil_Out32(0xf6169004, 0x00000000);
    prog_reg(0xF6169000, 0x8, 0x100, 0x1);
    Xil_Out32(0xf6169004, 0x00000000);
    prog_reg(0xF6169000, 0x7, 0x80, 0x1);
    Xil_Out32(0xf6169004, 0x00000000);
    prog_reg(0xF6169800, 0x1, 0x2, 0x1);
    Xil_Out32(0xf6169804, 0x00000000);
    prog_reg(0xF6169800, 0x6, 0x40, 0x1);
    Xil_Out32(0xf6169804, 0x00000000);
    prog_reg(0xF6169800, 0x8, 0x100, 0x1);
    Xil_Out32(0xf6169804, 0x00000000);
    prog_reg(0xF6169800, 0x7, 0x80, 0x1);
    Xil_Out32(0xf6169804, 0x00000000);
    dbg0_pmc(16400);
    //polling XPIO_1
    poll_for(0xf6169008, 0x00000010, 0x00000004, 0x00000001);
    Xil_Out32(0xf6169004, 0x00000000);
    dbg0_pmc(16401);
    //************************************************************************************************* XPIO_PLL : end

    Xil_Out32(0xf609900c, 0xf9e8d7c6);
    prog_reg(0xF6099038, 0xC, 0x1000, 0x1);
    prog_reg(0xF609903C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF609903C, 0x0, 0xFF, 0x2);
    prog_reg(0xF6099040, 0xC, 0x1000, 0x1);
    prog_reg(0xF6099040, 0xB, 0x800, 0x0);
    prog_reg(0xF6099038, 0xB, 0x800, 0x0);
    prog_reg(0xF6099048, 0xB, 0x800, 0x0);
    prog_reg(0xF6099050, 0xB, 0x800, 0x0);
    prog_reg(0xF6099044, 0x8, 0xFF00, 0x2);
    prog_reg(0xF6099044, 0x0, 0xFF, 0x2);
    prog_reg(0xF6099048, 0xC, 0x1000, 0x1);
    prog_reg(0xF609904C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF609904C, 0x0, 0xFF, 0x2);
    prog_reg(0xF6099030, 0xD, 0xE000, 0x0);
    prog_reg(0xF6099050, 0xC, 0x1000, 0x1);
    prog_reg(0xF6099054, 0x8, 0xFF00, 0x2);
    prog_reg(0xF6099054, 0x0, 0xFF, 0x2);
    prog_reg(0xF60990A0, 0xA, 0x7C00, 0xA);
    prog_reg(0xF60990A4, 0xA, 0x7C00, 0xA);
    prog_reg(0xF6099034, 0x8, 0xFF00, 0x2);
    prog_reg(0xF6099034, 0x0, 0xFF, 0x2);
    prog_reg(0xF6099030, 0x9, 0x200, 0x1);
    prog_reg(0xF6099030, 0xA, 0xC00, 0x1);
    prog_reg(0xF6099030, 0x0, 0xFF, 0x0);
    prog_reg(0xF6099030, 0x8, 0x100, 0x0);
    prog_reg(0xF6099030, 0xC, 0x1000, 0x0);
    prog_reg(0xF6099088, 0x5, 0x20, 0x0);
    Xil_Out32(0xf609980c, 0xf9e8d7c6);
    prog_reg(0xF6099810, 0xB, 0x1800, 0x3);
    prog_reg(0xF6099810, 0x8, 0x700, 0x2);
    prog_reg(0xF6099810, 0x5, 0xE0, 0x1);
    prog_reg(0xF6099810, 0x2, 0x1C, 0x5);
    prog_reg(0xF6099000, 0x1, 0x2, 0x1);
    Xil_Out32(0xf6099004, 0x00000000);
    prog_reg(0xF6099000, 0x6, 0x40, 0x1);
    Xil_Out32(0xf6099004, 0x00000000);
    prog_reg(0xF6099000, 0x8, 0x100, 0x1);
    Xil_Out32(0xf6099004, 0x00000000);
    prog_reg(0xF6099000, 0x7, 0x80, 0x1);
    Xil_Out32(0xf6099004, 0x00000000);
    prog_reg(0xF6099800, 0x1, 0x2, 0x1);
    Xil_Out32(0xf6099804, 0x00000000);
    prog_reg(0xF6099800, 0x6, 0x40, 0x1);
    Xil_Out32(0xf6099804, 0x00000000);
    prog_reg(0xF6099800, 0x8, 0x100, 0x1);
    Xil_Out32(0xf6099804, 0x00000000);
    prog_reg(0xF6099800, 0x7, 0x80, 0x1);
    Xil_Out32(0xf6099804, 0x00000000);
    dbg0_pmc(16400);
    //polling XPIO_0
    poll_for(0xf6099008, 0x00000010, 0x00000004, 0x00000001);
    Xil_Out32(0xf6099004, 0x00000000);
    dbg0_pmc(16401);
    //************************************************************************************************* XPIO_PLL : end

    Xil_Out32(0xf61c900c, 0xf9e8d7c6);
    prog_reg(0xF61C9038, 0xC, 0x1000, 0x1);
    prog_reg(0xF61C903C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF61C903C, 0x0, 0xFF, 0x2);
    prog_reg(0xF61C9040, 0xC, 0x1000, 0x1);
    prog_reg(0xF61C9040, 0xB, 0x800, 0x0);
    prog_reg(0xF61C9038, 0xB, 0x800, 0x0);
    prog_reg(0xF61C9048, 0xB, 0x800, 0x0);
    prog_reg(0xF61C9050, 0xB, 0x800, 0x0);
    prog_reg(0xF61C9044, 0x8, 0xFF00, 0x2);
    prog_reg(0xF61C9044, 0x0, 0xFF, 0x2);
    prog_reg(0xF61C9048, 0xC, 0x1000, 0x1);
    prog_reg(0xF61C904C, 0x8, 0xFF00, 0x2);
    prog_reg(0xF61C904C, 0x0, 0xFF, 0x2);
    prog_reg(0xF61C9030, 0xD, 0xE000, 0x0);
    prog_reg(0xF61C9050, 0xC, 0x1000, 0x1);
    prog_reg(0xF61C9054, 0x8, 0xFF00, 0x2);
    prog_reg(0xF61C9054, 0x0, 0xFF, 0x2);
    prog_reg(0xF61C90A0, 0xA, 0x7C00, 0xA);
    prog_reg(0xF61C90A4, 0xA, 0x7C00, 0xA);
    prog_reg(0xF61C9034, 0x8, 0xFF00, 0x2);
    prog_reg(0xF61C9034, 0x0, 0xFF, 0x2);
    prog_reg(0xF61C9030, 0x9, 0x200, 0x1);
    prog_reg(0xF61C9030, 0xA, 0xC00, 0x1);
    prog_reg(0xF61C9030, 0x0, 0xFF, 0x0);
    prog_reg(0xF61C9030, 0x8, 0x100, 0x0);
    prog_reg(0xF61C9030, 0xC, 0x1000, 0x0);
    prog_reg(0xF61C9088, 0x5, 0x20, 0x0);
    Xil_Out32(0xf61c980c, 0xf9e8d7c6);
    prog_reg(0xF61C9810, 0xB, 0x1800, 0x3);
    prog_reg(0xF61C9810, 0x8, 0x700, 0x2);
    prog_reg(0xF61C9810, 0x5, 0xE0, 0x1);
    prog_reg(0xF61C9810, 0x2, 0x1C, 0x3);
    prog_reg(0xF61C9000, 0x1, 0x2, 0x1);
    Xil_Out32(0xf61c9004, 0x00000000);
    prog_reg(0xF61C9000, 0x6, 0x40, 0x1);
    Xil_Out32(0xf61c9004, 0x00000000);
    prog_reg(0xF61C9000, 0x8, 0x100, 0x1);
    Xil_Out32(0xf61c9004, 0x00000000);
    prog_reg(0xF61C9000, 0x7, 0x80, 0x1);
    Xil_Out32(0xf61c9004, 0x00000000);
    prog_reg(0xF61C9800, 0x1, 0x2, 0x1);
    Xil_Out32(0xf61c9804, 0x00000000);
    prog_reg(0xF61C9800, 0x6, 0x40, 0x1);
    Xil_Out32(0xf61c9804, 0x00000000);
    prog_reg(0xF61C9800, 0x8, 0x100, 0x1);
    Xil_Out32(0xf61c9804, 0x00000000);
    prog_reg(0xF61C9800, 0x7, 0x80, 0x1);
    Xil_Out32(0xf61c9804, 0x00000000);
    dbg0_pmc(16400);
    //polling XPIO_2
    poll_for(0xf61c9008, 0x00000010, 0x00000004, 0x00000001);
    Xil_Out32(0xf61c9004, 0x00000000);
    dbg0_pmc(16401);
    //************************************************************************************************* XPIO_PLL : end

    //************************************************************************************************* XPIO_PLL : end

    //**************Register programming from  xphy_pll end---------------- 

    //**************Register programming from  xphy_init start---------------- 

    dbg0_pmc(16386);
    //************************************************************************************************* XPHY_CFG : begin

    //****************************************************
    //XPIO_0
    //****************************************************
    Xil_Out32(0xf609900c, 0xf9e8d7c6);
    Xil_Out32(0xf6099000, 0x000011c2);
    Xil_Out32(0xf6099004, 0x00001000);
    //------------------------------nibble_0
    Xil_Out32(0xf609020c, 0xf9e8d7c6);
    Xil_Out32(0xf609040c, 0xf9e8d7c6);
    Xil_Out32(0xf609060c, 0xf9e8d7c6);
    Xil_Out32(0xf6090228, 0x00000006);
    Xil_Out32(0xf6090428, 0x00000006);
    Xil_Out32(0xf6090628, 0x00000006);
    Xil_Out32(0xf6090244, 0x00000006);
    Xil_Out32(0xf6090444, 0x00000006);
    Xil_Out32(0xf6090644, 0x00000006);
    Xil_Out32(0xf609000c, 0xf9e8d7c6);
    Xil_Out32(0xf609001c, 0x000000b3);
    Xil_Out32(0xf6090020, 0x0000020b);
    Xil_Out32(0xf6090034, 0x00001420);
    Xil_Out32(0xf609002c, 0x00000102);
    Xil_Out32(0xf6090058, 0x0000003d);
    Xil_Out32(0xf609004c, 0x00000100);
    Xil_Out32(0xf6090048, 0x00000010);
    Xil_Out32(0xf6090004, 0x000011fe);
    Xil_Out32(0xf6090028, 0x00000000);
    Xil_Out32(0xf60900dc, 0x00000004);
    //------------------------------nibble_1
    Xil_Out32(0xf609120c, 0xf9e8d7c6);
    Xil_Out32(0xf609140c, 0xf9e8d7c6);
    Xil_Out32(0xf609160c, 0xf9e8d7c6);
    Xil_Out32(0xf6091228, 0x00000006);
    Xil_Out32(0xf6091428, 0x00000006);
    Xil_Out32(0xf6091628, 0x00000006);
    Xil_Out32(0xf6091244, 0x00000006);
    Xil_Out32(0xf6091444, 0x00000006);
    Xil_Out32(0xf6091644, 0x00000006);
    Xil_Out32(0xf609100c, 0xf9e8d7c6);
    Xil_Out32(0xf609101c, 0x000000b0);
    Xil_Out32(0xf6091020, 0x0000020f);
    Xil_Out32(0xf6091034, 0x00001420);
    Xil_Out32(0xf6091058, 0x0000003f);
    Xil_Out32(0xf609104c, 0x00000100);
    Xil_Out32(0xf6091048, 0x00000010);
    Xil_Out32(0xf6091004, 0x000011fe);
    Xil_Out32(0xf6091028, 0x00000000);
    Xil_Out32(0xf60910dc, 0x00000004);
    //------------------------------nibble_2
    Xil_Out32(0xf609220c, 0xf9e8d7c6);
    Xil_Out32(0xf609240c, 0xf9e8d7c6);
    Xil_Out32(0xf609260c, 0xf9e8d7c6);
    Xil_Out32(0xf6092228, 0x00000006);
    Xil_Out32(0xf6092428, 0x00000006);
    Xil_Out32(0xf6092628, 0x00000006);
    Xil_Out32(0xf6092244, 0x00000006);
    Xil_Out32(0xf6092444, 0x00000006);
    Xil_Out32(0xf6092644, 0x00000006);
    Xil_Out32(0xf609200c, 0xf9e8d7c6);
    Xil_Out32(0xf6092020, 0x000002fc);
    Xil_Out32(0xf6092034, 0x00001420);
    Xil_Out32(0xf609202c, 0x0000013f);
    Xil_Out32(0xf609204c, 0x00000100);
    Xil_Out32(0xf6092048, 0x00000010);
    Xil_Out32(0xf6092004, 0x000011fe);
    Xil_Out32(0xf6092028, 0x00000000);
    Xil_Out32(0xf60920dc, 0x00000004);
    //------------------------------nibble_3
    Xil_Out32(0xf609320c, 0xf9e8d7c6);
    Xil_Out32(0xf609340c, 0xf9e8d7c6);
    Xil_Out32(0xf609360c, 0xf9e8d7c6);
    Xil_Out32(0xf6093228, 0x00000006);
    Xil_Out32(0xf6093428, 0x00000006);
    Xil_Out32(0xf6093628, 0x00000006);
    Xil_Out32(0xf6093244, 0x00000006);
    Xil_Out32(0xf6093444, 0x00000006);
    Xil_Out32(0xf6093644, 0x00000006);
    Xil_Out32(0xf609300c, 0xf9e8d7c6);
    Xil_Out32(0xf6093020, 0x000002fc);
    Xil_Out32(0xf6093034, 0x00001420);
    Xil_Out32(0xf609302c, 0x0000013c);
    Xil_Out32(0xf609304c, 0x00000100);
    Xil_Out32(0xf6093048, 0x00000010);
    Xil_Out32(0xf6093004, 0x000011fe);
    Xil_Out32(0xf6093028, 0x00000000);
    Xil_Out32(0xf60930dc, 0x00000004);
    //------------------------------nibble_4
    Xil_Out32(0xf609420c, 0xf9e8d7c6);
    Xil_Out32(0xf609440c, 0xf9e8d7c6);
    Xil_Out32(0xf609460c, 0xf9e8d7c6);
    Xil_Out32(0xf6094228, 0x00000006);
    Xil_Out32(0xf6094428, 0x00000006);
    Xil_Out32(0xf6094628, 0x00000006);
    Xil_Out32(0xf6094244, 0x00000006);
    Xil_Out32(0xf6094444, 0x00000006);
    Xil_Out32(0xf6094644, 0x00000006);
    Xil_Out32(0xf609400c, 0xf9e8d7c6);
    Xil_Out32(0xf6094020, 0x000002fc);
    Xil_Out32(0xf6094034, 0x00001420);
    Xil_Out32(0xf609402c, 0x0000013f);
    Xil_Out32(0xf609404c, 0x00000100);
    Xil_Out32(0xf6094048, 0x00000010);
    Xil_Out32(0xf6094004, 0x000011fe);
    Xil_Out32(0xf6094028, 0x00000000);
    Xil_Out32(0xf60940dc, 0x00000004);
    //------------------------------nibble_5
    Xil_Out32(0xf609520c, 0xf9e8d7c6);
    Xil_Out32(0xf609540c, 0xf9e8d7c6);
    Xil_Out32(0xf609560c, 0xf9e8d7c6);
    Xil_Out32(0xf6095228, 0x00000006);
    Xil_Out32(0xf6095428, 0x00000006);
    Xil_Out32(0xf6095628, 0x00000006);
    Xil_Out32(0xf6095244, 0x00000006);
    Xil_Out32(0xf6095444, 0x00000006);
    Xil_Out32(0xf6095644, 0x00000006);
    Xil_Out32(0xf609500c, 0xf9e8d7c6);
    Xil_Out32(0xf6095020, 0x000002fc);
    Xil_Out32(0xf6095034, 0x00001420);
    Xil_Out32(0xf609502c, 0x0000013f);
    Xil_Out32(0xf609504c, 0x00000100);
    Xil_Out32(0xf6095048, 0x00000010);
    Xil_Out32(0xf6095004, 0x000011fe);
    Xil_Out32(0xf6095028, 0x00000000);
    Xil_Out32(0xf60950dc, 0x00000004);
    //------------------------------nibble_6
    Xil_Out32(0xf609620c, 0xf9e8d7c6);
    Xil_Out32(0xf609640c, 0xf9e8d7c6);
    Xil_Out32(0xf609660c, 0xf9e8d7c6);
    Xil_Out32(0xf6096228, 0x00000006);
    Xil_Out32(0xf6096428, 0x00000006);
    Xil_Out32(0xf6096628, 0x00000006);
    Xil_Out32(0xf6096244, 0x00000006);
    Xil_Out32(0xf6096444, 0x00000006);
    Xil_Out32(0xf6096644, 0x00000006);
    Xil_Out32(0xf609600c, 0xf9e8d7c6);
    Xil_Out32(0xf6096020, 0x000002fc);
    Xil_Out32(0xf6096034, 0x00001420);
    Xil_Out32(0xf609602c, 0x0000013f);
    Xil_Out32(0xf609604c, 0x00000100);
    Xil_Out32(0xf6096048, 0x00000010);
    Xil_Out32(0xf6096004, 0x000011fe);
    Xil_Out32(0xf6096028, 0x00000000);
    Xil_Out32(0xf60960dc, 0x00000004);
    //------------------------------nibble_7
    Xil_Out32(0xf609720c, 0xf9e8d7c6);
    Xil_Out32(0xf609740c, 0xf9e8d7c6);
    Xil_Out32(0xf609760c, 0xf9e8d7c6);
    Xil_Out32(0xf6097228, 0x00000006);
    Xil_Out32(0xf6097428, 0x00000006);
    Xil_Out32(0xf6097628, 0x00000006);
    Xil_Out32(0xf6097244, 0x00000006);
    Xil_Out32(0xf6097444, 0x00000006);
    Xil_Out32(0xf6097644, 0x00000006);
    Xil_Out32(0xf609700c, 0xf9e8d7c6);
    Xil_Out32(0xf6097020, 0x000002fc);
    Xil_Out32(0xf6097034, 0x00001420);
    Xil_Out32(0xf609702c, 0x0000012f);
    Xil_Out32(0xf609704c, 0x00000100);
    Xil_Out32(0xf6097048, 0x00000010);
    Xil_Out32(0xf6097004, 0x000011fe);
    Xil_Out32(0xf6097028, 0x00000000);
    Xil_Out32(0xf60970dc, 0x00000004);
    //------------------------------nibble_8
    Xil_Out32(0xf609820c, 0xf9e8d7c6);
    Xil_Out32(0xf609840c, 0xf9e8d7c6);
    Xil_Out32(0xf609860c, 0xf9e8d7c6);
    Xil_Out32(0xf6098228, 0x00000006);
    Xil_Out32(0xf6098428, 0x00000006);
    Xil_Out32(0xf6098628, 0x00000006);
    Xil_Out32(0xf6098244, 0x00000006);
    Xil_Out32(0xf6098444, 0x00000006);
    Xil_Out32(0xf6098644, 0x00000006);
    Xil_Out32(0xf609800c, 0xf9e8d7c6);
    Xil_Out32(0xf6098020, 0x000002fc);
    Xil_Out32(0xf6098034, 0x00001420);
    Xil_Out32(0xf609802c, 0x0000013f);
    Xil_Out32(0xf609804c, 0x00000100);
    Xil_Out32(0xf6098048, 0x00000010);
    Xil_Out32(0xf6098004, 0x000011fe);
    Xil_Out32(0xf6098028, 0x00000000);
    Xil_Out32(0xf60980dc, 0x00000004);
    //****************************************************
    //XPIO_1
    //****************************************************
    Xil_Out32(0xf616900c, 0xf9e8d7c6);
    Xil_Out32(0xf6169000, 0x000011c2);
    Xil_Out32(0xf6169004, 0x00001000);
    //------------------------------nibble_0
    Xil_Out32(0xf616020c, 0xf9e8d7c6);
    Xil_Out32(0xf616040c, 0xf9e8d7c6);
    Xil_Out32(0xf616060c, 0xf9e8d7c6);
    Xil_Out32(0xf6160228, 0x00000006);
    Xil_Out32(0xf6160428, 0x00000006);
    Xil_Out32(0xf6160628, 0x00000006);
    Xil_Out32(0xf6160244, 0x00000006);
    Xil_Out32(0xf6160444, 0x00000006);
    Xil_Out32(0xf6160644, 0x00000006);
    Xil_Out32(0xf616000c, 0xf9e8d7c6);
    Xil_Out32(0xf616001c, 0x000000b3);
    Xil_Out32(0xf6160020, 0x0000020b);
    Xil_Out32(0xf6160034, 0x00001420);
    Xil_Out32(0xf616002c, 0x00000102);
    Xil_Out32(0xf6160058, 0x0000003d);
    Xil_Out32(0xf616004c, 0x00000100);
    Xil_Out32(0xf6160048, 0x00000010);
    Xil_Out32(0xf6160004, 0x000011fe);
    Xil_Out32(0xf6160028, 0x00000000);
    Xil_Out32(0xf61600dc, 0x00000004);
    //------------------------------nibble_1
    Xil_Out32(0xf616120c, 0xf9e8d7c6);
    Xil_Out32(0xf616140c, 0xf9e8d7c6);
    Xil_Out32(0xf616160c, 0xf9e8d7c6);
    Xil_Out32(0xf6161228, 0x00000006);
    Xil_Out32(0xf6161428, 0x00000006);
    Xil_Out32(0xf6161628, 0x00000006);
    Xil_Out32(0xf6161244, 0x00000006);
    Xil_Out32(0xf6161444, 0x00000006);
    Xil_Out32(0xf6161644, 0x00000006);
    Xil_Out32(0xf616100c, 0xf9e8d7c6);
    Xil_Out32(0xf616101c, 0x000000b0);
    Xil_Out32(0xf6161020, 0x0000020f);
    Xil_Out32(0xf6161034, 0x00001420);
    Xil_Out32(0xf6161058, 0x0000003f);
    Xil_Out32(0xf616104c, 0x00000100);
    Xil_Out32(0xf6161048, 0x00000010);
    Xil_Out32(0xf6161004, 0x000011fe);
    Xil_Out32(0xf6161028, 0x00000000);
    Xil_Out32(0xf61610dc, 0x00000004);
    //------------------------------nibble_2
    Xil_Out32(0xf616220c, 0xf9e8d7c6);
    Xil_Out32(0xf616240c, 0xf9e8d7c6);
    Xil_Out32(0xf616260c, 0xf9e8d7c6);
    Xil_Out32(0xf6162228, 0x00000006);
    Xil_Out32(0xf6162428, 0x00000006);
    Xil_Out32(0xf6162628, 0x00000006);
    Xil_Out32(0xf6162244, 0x00000006);
    Xil_Out32(0xf6162444, 0x00000006);
    Xil_Out32(0xf6162644, 0x00000006);
    Xil_Out32(0xf616200c, 0xf9e8d7c6);
    Xil_Out32(0xf616201c, 0x000000b0);
    Xil_Out32(0xf6162020, 0x0000020f);
    Xil_Out32(0xf6162034, 0x00001420);
    Xil_Out32(0xf6162058, 0x0000003f);
    Xil_Out32(0xf616204c, 0x00000100);
    Xil_Out32(0xf6162048, 0x00000010);
    Xil_Out32(0xf6162004, 0x000011fe);
    Xil_Out32(0xf6162028, 0x00000000);
    Xil_Out32(0xf61620dc, 0x00000004);
    //------------------------------nibble_3
    Xil_Out32(0xf616320c, 0xf9e8d7c6);
    Xil_Out32(0xf616340c, 0xf9e8d7c6);
    Xil_Out32(0xf616360c, 0xf9e8d7c6);
    Xil_Out32(0xf6163228, 0x00000006);
    Xil_Out32(0xf6163428, 0x00000006);
    Xil_Out32(0xf6163628, 0x00000006);
    Xil_Out32(0xf6163244, 0x00000006);
    Xil_Out32(0xf6163444, 0x00000006);
    Xil_Out32(0xf6163644, 0x00000006);
    Xil_Out32(0xf616300c, 0xf9e8d7c6);
    Xil_Out32(0xf616301c, 0x000000b3);
    Xil_Out32(0xf6163020, 0x0000020b);
    Xil_Out32(0xf6163034, 0x00001420);
    Xil_Out32(0xf616302c, 0x00000102);
    Xil_Out32(0xf6163058, 0x0000003d);
    Xil_Out32(0xf616304c, 0x00000100);
    Xil_Out32(0xf6163048, 0x00000010);
    Xil_Out32(0xf6163004, 0x000011fe);
    Xil_Out32(0xf6163028, 0x00000000);
    Xil_Out32(0xf61630dc, 0x00000004);
    //------------------------------nibble_4
    Xil_Out32(0xf616420c, 0xf9e8d7c6);
    Xil_Out32(0xf616440c, 0xf9e8d7c6);
    Xil_Out32(0xf616460c, 0xf9e8d7c6);
    Xil_Out32(0xf6164228, 0x00000006);
    Xil_Out32(0xf6164428, 0x00000006);
    Xil_Out32(0xf6164628, 0x00000006);
    Xil_Out32(0xf6164244, 0x00000006);
    Xil_Out32(0xf6164444, 0x00000006);
    Xil_Out32(0xf6164644, 0x00000006);
    Xil_Out32(0xf616400c, 0xf9e8d7c6);
    Xil_Out32(0xf616401c, 0x000000b3);
    Xil_Out32(0xf6164020, 0x0000020b);
    Xil_Out32(0xf6164034, 0x00001420);
    Xil_Out32(0xf616402c, 0x00000102);
    Xil_Out32(0xf6164058, 0x0000003d);
    Xil_Out32(0xf616404c, 0x00000100);
    Xil_Out32(0xf6164048, 0x00000010);
    Xil_Out32(0xf6164004, 0x000011fe);
    Xil_Out32(0xf6164028, 0x00000000);
    Xil_Out32(0xf61640dc, 0x00000004);
    //------------------------------nibble_5
    Xil_Out32(0xf616520c, 0xf9e8d7c6);
    Xil_Out32(0xf616540c, 0xf9e8d7c6);
    Xil_Out32(0xf616560c, 0xf9e8d7c6);
    Xil_Out32(0xf6165228, 0x00000006);
    Xil_Out32(0xf6165428, 0x00000006);
    Xil_Out32(0xf6165628, 0x00000006);
    Xil_Out32(0xf6165244, 0x00000006);
    Xil_Out32(0xf6165444, 0x00000006);
    Xil_Out32(0xf6165644, 0x00000006);
    Xil_Out32(0xf616500c, 0xf9e8d7c6);
    Xil_Out32(0xf616501c, 0x000000b0);
    Xil_Out32(0xf6165020, 0x0000020f);
    Xil_Out32(0xf6165034, 0x00001420);
    Xil_Out32(0xf6165058, 0x0000003f);
    Xil_Out32(0xf616504c, 0x00000100);
    Xil_Out32(0xf6165048, 0x00000010);
    Xil_Out32(0xf6165004, 0x000011fe);
    Xil_Out32(0xf6165028, 0x00000000);
    Xil_Out32(0xf61650dc, 0x00000004);
    //------------------------------nibble_6
    Xil_Out32(0xf616620c, 0xf9e8d7c6);
    Xil_Out32(0xf616640c, 0xf9e8d7c6);
    Xil_Out32(0xf616660c, 0xf9e8d7c6);
    Xil_Out32(0xf6166228, 0x00000006);
    Xil_Out32(0xf6166428, 0x00000006);
    Xil_Out32(0xf6166628, 0x00000006);
    Xil_Out32(0xf6166244, 0x00000006);
    Xil_Out32(0xf6166444, 0x00000006);
    Xil_Out32(0xf6166644, 0x00000006);
    Xil_Out32(0xf616600c, 0xf9e8d7c6);
    Xil_Out32(0xf616601c, 0x000000b3);
    Xil_Out32(0xf6166020, 0x0000020b);
    Xil_Out32(0xf6166034, 0x00001420);
    Xil_Out32(0xf616602c, 0x00000102);
    Xil_Out32(0xf6166058, 0x0000003d);
    Xil_Out32(0xf616604c, 0x00000100);
    Xil_Out32(0xf6166048, 0x00000010);
    Xil_Out32(0xf6166004, 0x000011fe);
    Xil_Out32(0xf6166028, 0x00000000);
    Xil_Out32(0xf61660dc, 0x00000004);
    //------------------------------nibble_7
    Xil_Out32(0xf616720c, 0xf9e8d7c6);
    Xil_Out32(0xf616740c, 0xf9e8d7c6);
    Xil_Out32(0xf616760c, 0xf9e8d7c6);
    Xil_Out32(0xf6167228, 0x00000006);
    Xil_Out32(0xf6167428, 0x00000006);
    Xil_Out32(0xf6167628, 0x00000006);
    Xil_Out32(0xf6167244, 0x00000006);
    Xil_Out32(0xf6167444, 0x00000006);
    Xil_Out32(0xf6167644, 0x00000006);
    Xil_Out32(0xf616700c, 0xf9e8d7c6);
    Xil_Out32(0xf616701c, 0x000000b0);
    Xil_Out32(0xf6167020, 0x0000020f);
    Xil_Out32(0xf6167034, 0x00001420);
    Xil_Out32(0xf6167058, 0x0000003f);
    Xil_Out32(0xf616704c, 0x00000100);
    Xil_Out32(0xf6167048, 0x00000010);
    Xil_Out32(0xf6167004, 0x000011fe);
    Xil_Out32(0xf6167028, 0x00000000);
    Xil_Out32(0xf61670dc, 0x00000004);
    //------------------------------nibble_8
    Xil_Out32(0xf616820c, 0xf9e8d7c6);
    Xil_Out32(0xf616840c, 0xf9e8d7c6);
    Xil_Out32(0xf616860c, 0xf9e8d7c6);
    Xil_Out32(0xf6168228, 0x00000006);
    Xil_Out32(0xf6168428, 0x00000006);
    Xil_Out32(0xf6168628, 0x00000006);
    Xil_Out32(0xf6168244, 0x00000006);
    Xil_Out32(0xf6168444, 0x00000006);
    Xil_Out32(0xf6168644, 0x00000006);
    Xil_Out32(0xf616800c, 0xf9e8d7c6);
    Xil_Out32(0xf6168020, 0x000002fc);
    Xil_Out32(0xf6168034, 0x00001420);
    Xil_Out32(0xf616802c, 0x0000013b);
    Xil_Out32(0xf6168058, 0x00000008);
    Xil_Out32(0xf616804c, 0x00000100);
    Xil_Out32(0xf6168040, 0x0000cf3f);
    Xil_Out32(0xf6168048, 0x00000010);
    Xil_Out32(0xf6168004, 0x000011fe);
    Xil_Out32(0xf6168028, 0x00000000);
    Xil_Out32(0xf61680dc, 0x00000004);
    //****************************************************
    //XPIO_2
    //****************************************************
    Xil_Out32(0xf61c900c, 0xf9e8d7c6);
    Xil_Out32(0xf61c9000, 0x000011c2);
    Xil_Out32(0xf61c9004, 0x00001000);
    //------------------------------nibble_0
    Xil_Out32(0xf61c020c, 0xf9e8d7c6);
    Xil_Out32(0xf61c040c, 0xf9e8d7c6);
    Xil_Out32(0xf61c060c, 0xf9e8d7c6);
    Xil_Out32(0xf61c0228, 0x00000006);
    Xil_Out32(0xf61c0428, 0x00000006);
    Xil_Out32(0xf61c0628, 0x00000006);
    Xil_Out32(0xf61c0244, 0x00000006);
    Xil_Out32(0xf61c0444, 0x00000006);
    Xil_Out32(0xf61c0644, 0x00000006);
    Xil_Out32(0xf61c000c, 0xf9e8d7c6);
    Xil_Out32(0xf61c001c, 0x000000b0);
    Xil_Out32(0xf61c0020, 0x0000020f);
    Xil_Out32(0xf61c0034, 0x00001420);
    Xil_Out32(0xf61c0058, 0x0000003f);
    Xil_Out32(0xf61c004c, 0x00000100);
    Xil_Out32(0xf61c0048, 0x00000010);
    Xil_Out32(0xf61c0004, 0x000011fe);
    Xil_Out32(0xf61c0028, 0x00000000);
    Xil_Out32(0xf61c00dc, 0x00000004);
    //------------------------------nibble_1
    Xil_Out32(0xf61c120c, 0xf9e8d7c6);
    Xil_Out32(0xf61c140c, 0xf9e8d7c6);
    Xil_Out32(0xf61c160c, 0xf9e8d7c6);
    Xil_Out32(0xf61c1228, 0x00000006);
    Xil_Out32(0xf61c1428, 0x00000006);
    Xil_Out32(0xf61c1628, 0x00000006);
    Xil_Out32(0xf61c1244, 0x00000006);
    Xil_Out32(0xf61c1444, 0x00000006);
    Xil_Out32(0xf61c1644, 0x00000006);
    Xil_Out32(0xf61c100c, 0xf9e8d7c6);
    Xil_Out32(0xf61c101c, 0x000000b3);
    Xil_Out32(0xf61c1020, 0x0000020b);
    Xil_Out32(0xf61c1034, 0x00001420);
    Xil_Out32(0xf61c102c, 0x00000102);
    Xil_Out32(0xf61c1058, 0x0000003d);
    Xil_Out32(0xf61c104c, 0x00000100);
    Xil_Out32(0xf61c1048, 0x00000010);
    Xil_Out32(0xf61c1004, 0x000011fe);
    Xil_Out32(0xf61c1028, 0x00000000);
    Xil_Out32(0xf61c10dc, 0x00000004);
    //------------------------------nibble_2
    Xil_Out32(0xf61c220c, 0xf9e8d7c6);
    Xil_Out32(0xf61c240c, 0xf9e8d7c6);
    Xil_Out32(0xf61c260c, 0xf9e8d7c6);
    Xil_Out32(0xf61c2228, 0x00000006);
    Xil_Out32(0xf61c2428, 0x00000006);
    Xil_Out32(0xf61c2628, 0x00000006);
    Xil_Out32(0xf61c2244, 0x00000006);
    Xil_Out32(0xf61c2444, 0x00000006);
    Xil_Out32(0xf61c2644, 0x00000006);
    Xil_Out32(0xf61c200c, 0xf9e8d7c6);
    Xil_Out32(0xf61c201c, 0x000000b3);
    Xil_Out32(0xf61c2020, 0x0000020b);
    Xil_Out32(0xf61c2034, 0x00001420);
    Xil_Out32(0xf61c202c, 0x00000102);
    Xil_Out32(0xf61c2058, 0x0000003d);
    Xil_Out32(0xf61c204c, 0x00000100);
    Xil_Out32(0xf61c2048, 0x00000010);
    Xil_Out32(0xf61c2004, 0x000011fe);
    Xil_Out32(0xf61c2028, 0x00000000);
    Xil_Out32(0xf61c20dc, 0x00000004);
    //------------------------------nibble_3
    Xil_Out32(0xf61c320c, 0xf9e8d7c6);
    Xil_Out32(0xf61c340c, 0xf9e8d7c6);
    Xil_Out32(0xf61c360c, 0xf9e8d7c6);
    Xil_Out32(0xf61c3228, 0x00000006);
    Xil_Out32(0xf61c3428, 0x00000006);
    Xil_Out32(0xf61c3628, 0x00000006);
    Xil_Out32(0xf61c3244, 0x00000006);
    Xil_Out32(0xf61c3444, 0x00000006);
    Xil_Out32(0xf61c3644, 0x00000006);
    Xil_Out32(0xf61c300c, 0xf9e8d7c6);
    Xil_Out32(0xf61c301c, 0x000000b0);
    Xil_Out32(0xf61c3020, 0x0000020f);
    Xil_Out32(0xf61c3034, 0x00001420);
    Xil_Out32(0xf61c3058, 0x0000003f);
    Xil_Out32(0xf61c304c, 0x00000100);
    Xil_Out32(0xf61c3048, 0x00000010);
    Xil_Out32(0xf61c3004, 0x000011fe);
    Xil_Out32(0xf61c3028, 0x00000000);
    Xil_Out32(0xf61c30dc, 0x00000004);
    //------------------------------nibble_4
    Xil_Out32(0xf61c420c, 0xf9e8d7c6);
    Xil_Out32(0xf61c440c, 0xf9e8d7c6);
    Xil_Out32(0xf61c460c, 0xf9e8d7c6);
    Xil_Out32(0xf61c4228, 0x00000006);
    Xil_Out32(0xf61c4428, 0x00000006);
    Xil_Out32(0xf61c4628, 0x00000006);
    Xil_Out32(0xf61c4244, 0x00000006);
    Xil_Out32(0xf61c4444, 0x00000006);
    Xil_Out32(0xf61c4644, 0x00000006);
    Xil_Out32(0xf61c400c, 0xf9e8d7c6);
    Xil_Out32(0xf61c401c, 0x000000b3);
    Xil_Out32(0xf61c4020, 0x0000020b);
    Xil_Out32(0xf61c4034, 0x00001420);
    Xil_Out32(0xf61c402c, 0x00000102);
    Xil_Out32(0xf61c4058, 0x0000003d);
    Xil_Out32(0xf61c404c, 0x00000100);
    Xil_Out32(0xf61c4048, 0x00000010);
    Xil_Out32(0xf61c4004, 0x000011fe);
    Xil_Out32(0xf61c4028, 0x00000000);
    Xil_Out32(0xf61c40dc, 0x00000004);
    //------------------------------nibble_5
    Xil_Out32(0xf61c520c, 0xf9e8d7c6);
    Xil_Out32(0xf61c540c, 0xf9e8d7c6);
    Xil_Out32(0xf61c560c, 0xf9e8d7c6);
    Xil_Out32(0xf61c5228, 0x00000006);
    Xil_Out32(0xf61c5428, 0x00000006);
    Xil_Out32(0xf61c5628, 0x00000006);
    Xil_Out32(0xf61c5244, 0x00000006);
    Xil_Out32(0xf61c5444, 0x00000006);
    Xil_Out32(0xf61c5644, 0x00000006);
    Xil_Out32(0xf61c500c, 0xf9e8d7c6);
    Xil_Out32(0xf61c501c, 0x000000b0);
    Xil_Out32(0xf61c5020, 0x0000020f);
    Xil_Out32(0xf61c5034, 0x00001420);
    Xil_Out32(0xf61c5058, 0x0000003f);
    Xil_Out32(0xf61c504c, 0x00000100);
    Xil_Out32(0xf61c5048, 0x00000010);
    Xil_Out32(0xf61c5004, 0x000011fe);
    Xil_Out32(0xf61c5028, 0x00000000);
    Xil_Out32(0xf61c50dc, 0x00000004);
    //------------------------------nibble_6
    Xil_Out32(0xf61c620c, 0xf9e8d7c6);
    Xil_Out32(0xf61c640c, 0xf9e8d7c6);
    Xil_Out32(0xf61c660c, 0xf9e8d7c6);
    Xil_Out32(0xf61c6228, 0x00000006);
    Xil_Out32(0xf61c6428, 0x00000006);
    Xil_Out32(0xf61c6628, 0x00000006);
    Xil_Out32(0xf61c6244, 0x00000006);
    Xil_Out32(0xf61c6444, 0x00000006);
    Xil_Out32(0xf61c6644, 0x00000006);
    Xil_Out32(0xf61c600c, 0xf9e8d7c6);
    Xil_Out32(0xf61c601c, 0x000000b3);
    Xil_Out32(0xf61c6020, 0x0000020b);
    Xil_Out32(0xf61c6034, 0x00001420);
    Xil_Out32(0xf61c602c, 0x0000013f);
    Xil_Out32(0xf61c6058, 0x0000003d);
    Xil_Out32(0xf61c604c, 0x00000100);
    Xil_Out32(0xf61c6048, 0x00000010);
    Xil_Out32(0xf61c6004, 0x000011fe);
    Xil_Out32(0xf61c6028, 0x00000000);
    Xil_Out32(0xf61c60dc, 0x00000004);
    //------------------------------nibble_7
    Xil_Out32(0xf61c720c, 0xf9e8d7c6);
    Xil_Out32(0xf61c740c, 0xf9e8d7c6);
    Xil_Out32(0xf61c760c, 0xf9e8d7c6);
    Xil_Out32(0xf61c7228, 0x00000006);
    Xil_Out32(0xf61c7428, 0x00000006);
    Xil_Out32(0xf61c7628, 0x00000006);
    Xil_Out32(0xf61c7244, 0x00000006);
    Xil_Out32(0xf61c7444, 0x00000006);
    Xil_Out32(0xf61c7644, 0x00000006);
    Xil_Out32(0xf61c700c, 0xf9e8d7c6);
    Xil_Out32(0xf61c701c, 0x000000b0);
    Xil_Out32(0xf61c7020, 0x0000020f);
    Xil_Out32(0xf61c7034, 0x00001420);
    Xil_Out32(0xf61c702c, 0x0000013f);
    Xil_Out32(0xf61c7058, 0x0000003f);
    Xil_Out32(0xf61c704c, 0x00000100);
    Xil_Out32(0xf61c7048, 0x00000010);
    Xil_Out32(0xf61c7004, 0x000011fe);
    Xil_Out32(0xf61c7028, 0x00000000);
    Xil_Out32(0xf61c70dc, 0x00000004);
    //------------------------------nibble_8
    Xil_Out32(0xf61c820c, 0xf9e8d7c6);
    Xil_Out32(0xf61c840c, 0xf9e8d7c6);
    Xil_Out32(0xf61c860c, 0xf9e8d7c6);
    Xil_Out32(0xf61c8228, 0x00000006);
    Xil_Out32(0xf61c8428, 0x00000006);
    Xil_Out32(0xf61c8628, 0x00000006);
    Xil_Out32(0xf61c8244, 0x00000006);
    Xil_Out32(0xf61c8444, 0x00000006);
    Xil_Out32(0xf61c8644, 0x00000006);
    Xil_Out32(0xf61c800c, 0xf9e8d7c6);
    Xil_Out32(0xf61c8020, 0x000002fc);
    Xil_Out32(0xf61c8034, 0x00001420);
    Xil_Out32(0xf61c802c, 0x0000013f);
    Xil_Out32(0xf61c804c, 0x00000100);
    Xil_Out32(0xf61c8048, 0x00000010);
    Xil_Out32(0xf61c8004, 0x000011fe);
    Xil_Out32(0xf61c8028, 0x00000000);
    Xil_Out32(0xf61c80dc, 0x00000004);
    //****************************************************
    //XPIO_0
    //****************************************************
    //------------------------------nibble_0
    Xil_Out32(0xf6090000, 0x00000040);
    Xil_Out32(0xf6090004, 0x000011be);
    Xil_Out32(0xf6090000, 0x00000042);
    Xil_Out32(0xf6090004, 0x000011bc);
    Xil_Out32(0xf6090000, 0x00000142);
    Xil_Out32(0xf6090004, 0x000010bc);
    Xil_Out32(0xf6090000, 0x000001c2);
    Xil_Out32(0xf6090004, 0x0000103c);
    Xil_Out32(0xf6090200, 0x00000106);
    Xil_Out32(0xf6090400, 0x00000106);
    Xil_Out32(0xf6090600, 0x00000106);
    Xil_Out32(0xf6090204, 0x000000f8);
    Xil_Out32(0xf6090404, 0x000000f8);
    Xil_Out32(0xf6090604, 0x000000f8);
    //------------------------------nibble_1
    Xil_Out32(0xf6091000, 0x00000040);
    Xil_Out32(0xf6091004, 0x000011be);
    Xil_Out32(0xf6091000, 0x00000042);
    Xil_Out32(0xf6091004, 0x000011bc);
    Xil_Out32(0xf6091000, 0x00000142);
    Xil_Out32(0xf6091004, 0x000010bc);
    Xil_Out32(0xf6091000, 0x000001c2);
    Xil_Out32(0xf6091004, 0x0000103c);
    Xil_Out32(0xf6091200, 0x00000106);
    Xil_Out32(0xf6091400, 0x00000106);
    Xil_Out32(0xf6091600, 0x00000106);
    Xil_Out32(0xf6091204, 0x000000f8);
    Xil_Out32(0xf6091404, 0x000000f8);
    Xil_Out32(0xf6091604, 0x000000f8);
    //------------------------------nibble_2
    Xil_Out32(0xf6092000, 0x00000040);
    Xil_Out32(0xf6092004, 0x000011be);
    Xil_Out32(0xf6092000, 0x00000042);
    Xil_Out32(0xf6092004, 0x000011bc);
    Xil_Out32(0xf6092000, 0x00000142);
    Xil_Out32(0xf6092004, 0x000010bc);
    Xil_Out32(0xf6092000, 0x000001c2);
    Xil_Out32(0xf6092004, 0x0000103c);
    Xil_Out32(0xf6092200, 0x00000106);
    Xil_Out32(0xf6092400, 0x00000106);
    Xil_Out32(0xf6092600, 0x00000106);
    Xil_Out32(0xf6092204, 0x000000f8);
    Xil_Out32(0xf6092404, 0x000000f8);
    Xil_Out32(0xf6092604, 0x000000f8);
    //------------------------------nibble_3
    Xil_Out32(0xf6093000, 0x00000040);
    Xil_Out32(0xf6093004, 0x000011be);
    Xil_Out32(0xf6093000, 0x00000042);
    Xil_Out32(0xf6093004, 0x000011bc);
    Xil_Out32(0xf6093000, 0x00000142);
    Xil_Out32(0xf6093004, 0x000010bc);
    Xil_Out32(0xf6093000, 0x000001c2);
    Xil_Out32(0xf6093004, 0x0000103c);
    Xil_Out32(0xf6093200, 0x00000106);
    Xil_Out32(0xf6093400, 0x00000106);
    Xil_Out32(0xf6093600, 0x00000106);
    Xil_Out32(0xf6093204, 0x000000f8);
    Xil_Out32(0xf6093404, 0x000000f8);
    Xil_Out32(0xf6093604, 0x000000f8);
    //------------------------------nibble_4
    Xil_Out32(0xf6094000, 0x00000040);
    Xil_Out32(0xf6094004, 0x000011be);
    Xil_Out32(0xf6094000, 0x00000042);
    Xil_Out32(0xf6094004, 0x000011bc);
    Xil_Out32(0xf6094000, 0x00000142);
    Xil_Out32(0xf6094004, 0x000010bc);
    Xil_Out32(0xf6094000, 0x000001c2);
    Xil_Out32(0xf6094004, 0x0000103c);
    Xil_Out32(0xf6094200, 0x00000106);
    Xil_Out32(0xf6094400, 0x00000106);
    Xil_Out32(0xf6094600, 0x00000106);
    Xil_Out32(0xf6094204, 0x000000f8);
    Xil_Out32(0xf6094404, 0x000000f8);
    Xil_Out32(0xf6094604, 0x000000f8);
    //------------------------------nibble_5
    Xil_Out32(0xf6095000, 0x00000040);
    Xil_Out32(0xf6095004, 0x000011be);
    Xil_Out32(0xf6095000, 0x00000042);
    Xil_Out32(0xf6095004, 0x000011bc);
    Xil_Out32(0xf6095000, 0x00000142);
    Xil_Out32(0xf6095004, 0x000010bc);
    Xil_Out32(0xf6095000, 0x000001c2);
    Xil_Out32(0xf6095004, 0x0000103c);
    Xil_Out32(0xf6095200, 0x00000106);
    Xil_Out32(0xf6095400, 0x00000106);
    Xil_Out32(0xf6095600, 0x00000106);
    Xil_Out32(0xf6095204, 0x000000f8);
    Xil_Out32(0xf6095404, 0x000000f8);
    Xil_Out32(0xf6095604, 0x000000f8);
    //------------------------------nibble_6
    Xil_Out32(0xf6096000, 0x00000040);
    Xil_Out32(0xf6096004, 0x000011be);
    Xil_Out32(0xf6096000, 0x00000042);
    Xil_Out32(0xf6096004, 0x000011bc);
    Xil_Out32(0xf6096000, 0x00000142);
    Xil_Out32(0xf6096004, 0x000010bc);
    Xil_Out32(0xf6096000, 0x000001c2);
    Xil_Out32(0xf6096004, 0x0000103c);
    Xil_Out32(0xf6096200, 0x00000106);
    Xil_Out32(0xf6096400, 0x00000106);
    Xil_Out32(0xf6096600, 0x00000106);
    Xil_Out32(0xf6096204, 0x000000f8);
    Xil_Out32(0xf6096404, 0x000000f8);
    Xil_Out32(0xf6096604, 0x000000f8);
    //------------------------------nibble_7
    Xil_Out32(0xf6097000, 0x00000040);
    Xil_Out32(0xf6097004, 0x000011be);
    Xil_Out32(0xf6097000, 0x00000042);
    Xil_Out32(0xf6097004, 0x000011bc);
    Xil_Out32(0xf6097000, 0x00000142);
    Xil_Out32(0xf6097004, 0x000010bc);
    Xil_Out32(0xf6097000, 0x000001c2);
    Xil_Out32(0xf6097004, 0x0000103c);
    Xil_Out32(0xf6097200, 0x00000106);
    Xil_Out32(0xf6097400, 0x00000106);
    Xil_Out32(0xf6097600, 0x00000106);
    Xil_Out32(0xf6097204, 0x000000f8);
    Xil_Out32(0xf6097404, 0x000000f8);
    Xil_Out32(0xf6097604, 0x000000f8);
    //------------------------------nibble_8
    Xil_Out32(0xf6098000, 0x00000040);
    Xil_Out32(0xf6098004, 0x000011be);
    Xil_Out32(0xf6098000, 0x00000042);
    Xil_Out32(0xf6098004, 0x000011bc);
    Xil_Out32(0xf6098000, 0x00000142);
    Xil_Out32(0xf6098004, 0x000010bc);
    Xil_Out32(0xf6098000, 0x000001c2);
    Xil_Out32(0xf6098004, 0x0000103c);
    Xil_Out32(0xf6098200, 0x00000106);
    Xil_Out32(0xf6098400, 0x00000106);
    Xil_Out32(0xf6098600, 0x00000106);
    Xil_Out32(0xf6098204, 0x000000f8);
    Xil_Out32(0xf6098404, 0x000000f8);
    Xil_Out32(0xf6098604, 0x000000f8);
    //****************************************************
    //XPIO_1
    //****************************************************
    //------------------------------nibble_0
    Xil_Out32(0xf6160000, 0x00000040);
    Xil_Out32(0xf6160004, 0x000011be);
    Xil_Out32(0xf6160000, 0x00000042);
    Xil_Out32(0xf6160004, 0x000011bc);
    Xil_Out32(0xf6160000, 0x00000142);
    Xil_Out32(0xf6160004, 0x000010bc);
    Xil_Out32(0xf6160000, 0x000001c2);
    Xil_Out32(0xf6160004, 0x0000103c);
    Xil_Out32(0xf6160200, 0x00000106);
    Xil_Out32(0xf6160400, 0x00000106);
    Xil_Out32(0xf6160600, 0x00000106);
    Xil_Out32(0xf6160204, 0x000000f8);
    Xil_Out32(0xf6160404, 0x000000f8);
    Xil_Out32(0xf6160604, 0x000000f8);
    //------------------------------nibble_1
    Xil_Out32(0xf6161000, 0x00000040);
    Xil_Out32(0xf6161004, 0x000011be);
    Xil_Out32(0xf6161000, 0x00000042);
    Xil_Out32(0xf6161004, 0x000011bc);
    Xil_Out32(0xf6161000, 0x00000142);
    Xil_Out32(0xf6161004, 0x000010bc);
    Xil_Out32(0xf6161000, 0x000001c2);
    Xil_Out32(0xf6161004, 0x0000103c);
    Xil_Out32(0xf6161200, 0x00000106);
    Xil_Out32(0xf6161400, 0x00000106);
    Xil_Out32(0xf6161600, 0x00000106);
    Xil_Out32(0xf6161204, 0x000000f8);
    Xil_Out32(0xf6161404, 0x000000f8);
    Xil_Out32(0xf6161604, 0x000000f8);
    //------------------------------nibble_2
    Xil_Out32(0xf6162000, 0x00000040);
    Xil_Out32(0xf6162004, 0x000011be);
    Xil_Out32(0xf6162000, 0x00000042);
    Xil_Out32(0xf6162004, 0x000011bc);
    Xil_Out32(0xf6162000, 0x00000142);
    Xil_Out32(0xf6162004, 0x000010bc);
    Xil_Out32(0xf6162000, 0x000001c2);
    Xil_Out32(0xf6162004, 0x0000103c);
    Xil_Out32(0xf6162200, 0x00000106);
    Xil_Out32(0xf6162400, 0x00000106);
    Xil_Out32(0xf6162600, 0x00000106);
    Xil_Out32(0xf6162204, 0x000000f8);
    Xil_Out32(0xf6162404, 0x000000f8);
    Xil_Out32(0xf6162604, 0x000000f8);
    //------------------------------nibble_3
    Xil_Out32(0xf6163000, 0x00000040);
    Xil_Out32(0xf6163004, 0x000011be);
    Xil_Out32(0xf6163000, 0x00000042);
    Xil_Out32(0xf6163004, 0x000011bc);
    Xil_Out32(0xf6163000, 0x00000142);
    Xil_Out32(0xf6163004, 0x000010bc);
    Xil_Out32(0xf6163000, 0x000001c2);
    Xil_Out32(0xf6163004, 0x0000103c);
    Xil_Out32(0xf6163200, 0x00000106);
    Xil_Out32(0xf6163400, 0x00000106);
    Xil_Out32(0xf6163600, 0x00000106);
    Xil_Out32(0xf6163204, 0x000000f8);
    Xil_Out32(0xf6163404, 0x000000f8);
    Xil_Out32(0xf6163604, 0x000000f8);
    //------------------------------nibble_4
    Xil_Out32(0xf6164000, 0x00000040);
    Xil_Out32(0xf6164004, 0x000011be);
    Xil_Out32(0xf6164000, 0x00000042);
    Xil_Out32(0xf6164004, 0x000011bc);
    Xil_Out32(0xf6164000, 0x00000142);
    Xil_Out32(0xf6164004, 0x000010bc);
    Xil_Out32(0xf6164000, 0x000001c2);
    Xil_Out32(0xf6164004, 0x0000103c);
    Xil_Out32(0xf6164200, 0x00000106);
    Xil_Out32(0xf6164400, 0x00000106);
    Xil_Out32(0xf6164600, 0x00000106);
    Xil_Out32(0xf6164204, 0x000000f8);
    Xil_Out32(0xf6164404, 0x000000f8);
    Xil_Out32(0xf6164604, 0x000000f8);
    //------------------------------nibble_5
    Xil_Out32(0xf6165000, 0x00000040);
    Xil_Out32(0xf6165004, 0x000011be);
    Xil_Out32(0xf6165000, 0x00000042);
    Xil_Out32(0xf6165004, 0x000011bc);
    Xil_Out32(0xf6165000, 0x00000142);
    Xil_Out32(0xf6165004, 0x000010bc);
    Xil_Out32(0xf6165000, 0x000001c2);
    Xil_Out32(0xf6165004, 0x0000103c);
    Xil_Out32(0xf6165200, 0x00000106);
    Xil_Out32(0xf6165400, 0x00000106);
    Xil_Out32(0xf6165600, 0x00000106);
    Xil_Out32(0xf6165204, 0x000000f8);
    Xil_Out32(0xf6165404, 0x000000f8);
    Xil_Out32(0xf6165604, 0x000000f8);
    //------------------------------nibble_6
    Xil_Out32(0xf6166000, 0x00000040);
    Xil_Out32(0xf6166004, 0x000011be);
    Xil_Out32(0xf6166000, 0x00000042);
    Xil_Out32(0xf6166004, 0x000011bc);
    Xil_Out32(0xf6166000, 0x00000142);
    Xil_Out32(0xf6166004, 0x000010bc);
    Xil_Out32(0xf6166000, 0x000001c2);
    Xil_Out32(0xf6166004, 0x0000103c);
    Xil_Out32(0xf6166200, 0x00000106);
    Xil_Out32(0xf6166400, 0x00000106);
    Xil_Out32(0xf6166600, 0x00000106);
    Xil_Out32(0xf6166204, 0x000000f8);
    Xil_Out32(0xf6166404, 0x000000f8);
    Xil_Out32(0xf6166604, 0x000000f8);
    //------------------------------nibble_7
    Xil_Out32(0xf6167000, 0x00000040);
    Xil_Out32(0xf6167004, 0x000011be);
    Xil_Out32(0xf6167000, 0x00000042);
    Xil_Out32(0xf6167004, 0x000011bc);
    Xil_Out32(0xf6167000, 0x00000142);
    Xil_Out32(0xf6167004, 0x000010bc);
    Xil_Out32(0xf6167000, 0x000001c2);
    Xil_Out32(0xf6167004, 0x0000103c);
    Xil_Out32(0xf6167200, 0x00000106);
    Xil_Out32(0xf6167400, 0x00000106);
    Xil_Out32(0xf6167600, 0x00000106);
    Xil_Out32(0xf6167204, 0x000000f8);
    Xil_Out32(0xf6167404, 0x000000f8);
    Xil_Out32(0xf6167604, 0x000000f8);
    //------------------------------nibble_8
    Xil_Out32(0xf6168000, 0x00000040);
    Xil_Out32(0xf6168004, 0x000011be);
    Xil_Out32(0xf6168000, 0x00000042);
    Xil_Out32(0xf6168004, 0x000011bc);
    Xil_Out32(0xf6168000, 0x00000142);
    Xil_Out32(0xf6168004, 0x000010bc);
    Xil_Out32(0xf6168000, 0x000001c2);
    Xil_Out32(0xf6168004, 0x0000103c);
    Xil_Out32(0xf6168200, 0x00000106);
    Xil_Out32(0xf6168400, 0x00000106);
    Xil_Out32(0xf6168600, 0x00000106);
    Xil_Out32(0xf6168204, 0x000000f8);
    Xil_Out32(0xf6168404, 0x000000f8);
    Xil_Out32(0xf6168604, 0x000000f8);
    //****************************************************
    //XPIO_2
    //****************************************************
    //------------------------------nibble_0
    Xil_Out32(0xf61c0000, 0x00000040);
    Xil_Out32(0xf61c0004, 0x000011be);
    Xil_Out32(0xf61c0000, 0x00000042);
    Xil_Out32(0xf61c0004, 0x000011bc);
    Xil_Out32(0xf61c0000, 0x00000142);
    Xil_Out32(0xf61c0004, 0x000010bc);
    Xil_Out32(0xf61c0000, 0x000001c2);
    Xil_Out32(0xf61c0004, 0x0000103c);
    Xil_Out32(0xf61c0200, 0x00000106);
    Xil_Out32(0xf61c0400, 0x00000106);
    Xil_Out32(0xf61c0600, 0x00000106);
    Xil_Out32(0xf61c0204, 0x000000f8);
    Xil_Out32(0xf61c0404, 0x000000f8);
    Xil_Out32(0xf61c0604, 0x000000f8);
    //------------------------------nibble_1
    Xil_Out32(0xf61c1000, 0x00000040);
    Xil_Out32(0xf61c1004, 0x000011be);
    Xil_Out32(0xf61c1000, 0x00000042);
    Xil_Out32(0xf61c1004, 0x000011bc);
    Xil_Out32(0xf61c1000, 0x00000142);
    Xil_Out32(0xf61c1004, 0x000010bc);
    Xil_Out32(0xf61c1000, 0x000001c2);
    Xil_Out32(0xf61c1004, 0x0000103c);
    Xil_Out32(0xf61c1200, 0x00000106);
    Xil_Out32(0xf61c1400, 0x00000106);
    Xil_Out32(0xf61c1600, 0x00000106);
    Xil_Out32(0xf61c1204, 0x000000f8);
    Xil_Out32(0xf61c1404, 0x000000f8);
    Xil_Out32(0xf61c1604, 0x000000f8);
    //------------------------------nibble_2
    Xil_Out32(0xf61c2000, 0x00000040);
    Xil_Out32(0xf61c2004, 0x000011be);
    Xil_Out32(0xf61c2000, 0x00000042);
    Xil_Out32(0xf61c2004, 0x000011bc);
    Xil_Out32(0xf61c2000, 0x00000142);
    Xil_Out32(0xf61c2004, 0x000010bc);
    Xil_Out32(0xf61c2000, 0x000001c2);
    Xil_Out32(0xf61c2004, 0x0000103c);
    Xil_Out32(0xf61c2200, 0x00000106);
    Xil_Out32(0xf61c2400, 0x00000106);
    Xil_Out32(0xf61c2600, 0x00000106);
    Xil_Out32(0xf61c2204, 0x000000f8);
    Xil_Out32(0xf61c2404, 0x000000f8);
    Xil_Out32(0xf61c2604, 0x000000f8);
    //------------------------------nibble_3
    Xil_Out32(0xf61c3000, 0x00000040);
    Xil_Out32(0xf61c3004, 0x000011be);
    Xil_Out32(0xf61c3000, 0x00000042);
    Xil_Out32(0xf61c3004, 0x000011bc);
    Xil_Out32(0xf61c3000, 0x00000142);
    Xil_Out32(0xf61c3004, 0x000010bc);
    Xil_Out32(0xf61c3000, 0x000001c2);
    Xil_Out32(0xf61c3004, 0x0000103c);
    Xil_Out32(0xf61c3200, 0x00000106);
    Xil_Out32(0xf61c3400, 0x00000106);
    Xil_Out32(0xf61c3600, 0x00000106);
    Xil_Out32(0xf61c3204, 0x000000f8);
    Xil_Out32(0xf61c3404, 0x000000f8);
    Xil_Out32(0xf61c3604, 0x000000f8);
    //------------------------------nibble_4
    Xil_Out32(0xf61c4000, 0x00000040);
    Xil_Out32(0xf61c4004, 0x000011be);
    Xil_Out32(0xf61c4000, 0x00000042);
    Xil_Out32(0xf61c4004, 0x000011bc);
    Xil_Out32(0xf61c4000, 0x00000142);
    Xil_Out32(0xf61c4004, 0x000010bc);
    Xil_Out32(0xf61c4000, 0x000001c2);
    Xil_Out32(0xf61c4004, 0x0000103c);
    Xil_Out32(0xf61c4200, 0x00000106);
    Xil_Out32(0xf61c4400, 0x00000106);
    Xil_Out32(0xf61c4600, 0x00000106);
    Xil_Out32(0xf61c4204, 0x000000f8);
    Xil_Out32(0xf61c4404, 0x000000f8);
    Xil_Out32(0xf61c4604, 0x000000f8);
    //------------------------------nibble_5
    Xil_Out32(0xf61c5000, 0x00000040);
    Xil_Out32(0xf61c5004, 0x000011be);
    Xil_Out32(0xf61c5000, 0x00000042);
    Xil_Out32(0xf61c5004, 0x000011bc);
    Xil_Out32(0xf61c5000, 0x00000142);
    Xil_Out32(0xf61c5004, 0x000010bc);
    Xil_Out32(0xf61c5000, 0x000001c2);
    Xil_Out32(0xf61c5004, 0x0000103c);
    Xil_Out32(0xf61c5200, 0x00000106);
    Xil_Out32(0xf61c5400, 0x00000106);
    Xil_Out32(0xf61c5600, 0x00000106);
    Xil_Out32(0xf61c5204, 0x000000f8);
    Xil_Out32(0xf61c5404, 0x000000f8);
    Xil_Out32(0xf61c5604, 0x000000f8);
    //------------------------------nibble_6
    Xil_Out32(0xf61c6000, 0x00000040);
    Xil_Out32(0xf61c6004, 0x000011be);
    Xil_Out32(0xf61c6000, 0x00000042);
    Xil_Out32(0xf61c6004, 0x000011bc);
    Xil_Out32(0xf61c6000, 0x00000142);
    Xil_Out32(0xf61c6004, 0x000010bc);
    Xil_Out32(0xf61c6000, 0x000001c2);
    Xil_Out32(0xf61c6004, 0x0000103c);
    Xil_Out32(0xf61c6200, 0x00000106);
    Xil_Out32(0xf61c6400, 0x00000106);
    Xil_Out32(0xf61c6600, 0x00000106);
    Xil_Out32(0xf61c6204, 0x000000f8);
    Xil_Out32(0xf61c6404, 0x000000f8);
    Xil_Out32(0xf61c6604, 0x000000f8);
    //------------------------------nibble_7
    Xil_Out32(0xf61c7000, 0x00000040);
    Xil_Out32(0xf61c7004, 0x000011be);
    Xil_Out32(0xf61c7000, 0x00000042);
    Xil_Out32(0xf61c7004, 0x000011bc);
    Xil_Out32(0xf61c7000, 0x00000142);
    Xil_Out32(0xf61c7004, 0x000010bc);
    Xil_Out32(0xf61c7000, 0x000001c2);
    Xil_Out32(0xf61c7004, 0x0000103c);
    Xil_Out32(0xf61c7200, 0x00000106);
    Xil_Out32(0xf61c7400, 0x00000106);
    Xil_Out32(0xf61c7600, 0x00000106);
    Xil_Out32(0xf61c7204, 0x000000f8);
    Xil_Out32(0xf61c7404, 0x000000f8);
    Xil_Out32(0xf61c7604, 0x000000f8);
    //------------------------------nibble_8
    Xil_Out32(0xf61c8000, 0x00000040);
    Xil_Out32(0xf61c8004, 0x000011be);
    Xil_Out32(0xf61c8000, 0x00000042);
    Xil_Out32(0xf61c8004, 0x000011bc);
    Xil_Out32(0xf61c8000, 0x00000142);
    Xil_Out32(0xf61c8004, 0x000010bc);
    Xil_Out32(0xf61c8000, 0x000001c2);
    Xil_Out32(0xf61c8004, 0x0000103c);
    Xil_Out32(0xf61c8200, 0x00000106);
    Xil_Out32(0xf61c8400, 0x00000106);
    Xil_Out32(0xf61c8600, 0x00000106);
    Xil_Out32(0xf61c8204, 0x000000f8);
    Xil_Out32(0xf61c8404, 0x000000f8);
    Xil_Out32(0xf61c8604, 0x000000f8);
    //*************************************************************************************************XPHY_CFG : end

    //**************Register programming from  xphy_init end---------------- 

    dbg0_pmc(16387);
    //**************Register programming from  ddr_ub start---------------- 

    dbg0_pmc(16388);
    Xil_Out32(0xf6110000, 0x000000c2);
    Xil_Out32(0xf6110004, 0x00000002);
    Xil_Out32(0xf6110000, 0x01000000);
    Xil_Out32(0xf6110004, 0x00000000);
    prog_reg(0xF6110200, 0x2, 0x4, 0x1);
    prog_reg(0xF6110200, 0x0, 0x3, 0x1);
    prog_reg(0xF6110000, 0x1, 0x2, 0x1);
    Xil_Out32(0xf6110004, 0x00000000);
    Xil_Out32(0xf6130000, 0xb0000002);
    Xil_Out32(0xf6130004, 0xb8080050);
    Xil_Out32(0xf6130008, 0xb0000002);
    Xil_Out32(0xf613000c, 0xb80804ac);
    Xil_Out32(0xf6130010, 0xb0000002);
    Xil_Out32(0xf6130014, 0xb8080930);
    Xil_Out32(0xf6130020, 0xb0000002);
    Xil_Out32(0xf6130024, 0xb80804b0);
    Xil_Out32(0xf6130050, 0xb0000002);
    Xil_Out32(0xf6130054, 0x30201288);
    Xil_Out32(0xf6130058, 0xb0000002);
    Xil_Out32(0xf613005c, 0x31601298);
    Xil_Out32(0xf6130060, 0x940bc802);
    Xil_Out32(0xf6130064, 0xb0000002);
    Xil_Out32(0xf6130068, 0x31600e98);
    Xil_Out32(0xf613006c, 0x940bc800);
    Xil_Out32(0xf6130070, 0x20c00000);
    Xil_Out32(0xf6130074, 0x20e00000);
    Xil_Out32(0xf6130078, 0xb0000000);
    Xil_Out32(0xf613007c, 0xb9f403f8);
    Xil_Out32(0xf6130080, 0x20a00000);
    Xil_Out32(0xf6130084, 0xb9f40008);
    Xil_Out32(0xf6130088, 0x30a30000);
    Xil_Out32(0xf613008c, 0xb8000000);
    Xil_Out32(0xf6130090, 0x3021fff4);
    Xil_Out32(0xf6130094, 0xfa610008);
    Xil_Out32(0xf6130098, 0x12610000);
    Xil_Out32(0xf613009c, 0xf8b30004);
    Xil_Out32(0xf61300a0, 0xe8730004);
    Xil_Out32(0xf61300a4, 0xe8630000);
    Xil_Out32(0xf61300a8, 0x10330000);
    Xil_Out32(0xf61300ac, 0xea610008);
    Xil_Out32(0xf61300b0, 0x3021000c);
    Xil_Out32(0xf61300b4, 0xb60f0008);
    Xil_Out32(0xf61300b8, 0x80000000);
    Xil_Out32(0xf61300bc, 0x3021ffec);
    Xil_Out32(0xf61300c0, 0xfa610010);
    Xil_Out32(0xf61300c4, 0x12610000);
    Xil_Out32(0xf61300c8, 0xf8b30008);
    Xil_Out32(0xf61300cc, 0xf8d3000c);
    Xil_Out32(0xf61300d0, 0xe8730008);
    Xil_Out32(0xf61300d4, 0xf8730004);
    Xil_Out32(0xf61300d8, 0xe8730004);
    Xil_Out32(0xf61300dc, 0xe893000c);
    Xil_Out32(0xf61300e0, 0xf8830000);
    Xil_Out32(0xf61300e4, 0x80000000);
    Xil_Out32(0xf61300e8, 0x10330000);
    Xil_Out32(0xf61300ec, 0xea610010);
    Xil_Out32(0xf61300f0, 0x30210014);
    Xil_Out32(0xf61300f4, 0xb60f0008);
    Xil_Out32(0xf61300f8, 0x80000000);
    Xil_Out32(0xf61300fc, 0x3021ffec);
    Xil_Out32(0xf6130100, 0xf9e10000);
    Xil_Out32(0xf6130104, 0xfa610010);
    Xil_Out32(0xf6130108, 0x12610000);
    Xil_Out32(0xf613010c, 0x30c0003c);
    Xil_Out32(0xf6130110, 0xb0000148);
    Xil_Out32(0xf6130114, 0x30a01b00);
    Xil_Out32(0xf6130118, 0xb9f4ffa4);
    Xil_Out32(0xf613011c, 0x80000000);
    Xil_Out32(0xf6130120, 0x30c0003c);
    Xil_Out32(0xf6130124, 0xb0000148);
    Xil_Out32(0xf6130128, 0x30a01b01);
    Xil_Out32(0xf613012c, 0xb9f4ff90);
    Xil_Out32(0xf6130130, 0x80000000);
    Xil_Out32(0xf6130134, 0x30c0003c);
    Xil_Out32(0xf6130138, 0xb0000148);
    Xil_Out32(0xf613013c, 0x30a01c00);
    Xil_Out32(0xf6130140, 0xb9f4ff7c);
    Xil_Out32(0xf6130144, 0x80000000);
    Xil_Out32(0xf6130148, 0x30c0003c);
    Xil_Out32(0xf613014c, 0xb0000148);
    Xil_Out32(0xf6130150, 0x30a01c01);
    Xil_Out32(0xf6130154, 0xb9f4ff68);
    Xil_Out32(0xf6130158, 0x80000000);
    Xil_Out32(0xf613015c, 0x30c0003c);
    Xil_Out32(0xf6130160, 0xb0000148);
    Xil_Out32(0xf6130164, 0x30a01d00);
    Xil_Out32(0xf6130168, 0xb9f4ff54);
    Xil_Out32(0xf613016c, 0x80000000);
    Xil_Out32(0xf6130170, 0x30c0003c);
    Xil_Out32(0xf6130174, 0xb0000148);
    Xil_Out32(0xf6130178, 0x30a01d01);
    Xil_Out32(0xf613017c, 0xb9f4ff40);
    Xil_Out32(0xf6130180, 0x80000000);
    Xil_Out32(0xf6130184, 0x30c0003c);
    Xil_Out32(0xf6130188, 0xb0000148);
    Xil_Out32(0xf613018c, 0x30a01e00);
    Xil_Out32(0xf6130190, 0xb9f4ff2c);
    Xil_Out32(0xf6130194, 0x80000000);
    Xil_Out32(0xf6130198, 0x30c0003c);
    Xil_Out32(0xf613019c, 0xb0000148);
    Xil_Out32(0xf61301a0, 0x30a01e01);
    Xil_Out32(0xf61301a4, 0xb9f4ff18);
    Xil_Out32(0xf61301a8, 0x80000000);
    Xil_Out32(0xf61301ac, 0x30c0003c);
    Xil_Out32(0xf61301b0, 0xb0000148);
    Xil_Out32(0xf61301b4, 0x30a01f00);
    Xil_Out32(0xf61301b8, 0xb9f4ff04);
    Xil_Out32(0xf61301bc, 0x80000000);
    Xil_Out32(0xf61301c0, 0x30c0003c);
    Xil_Out32(0xf61301c4, 0xb0000148);
    Xil_Out32(0xf61301c8, 0x30a01f01);
    Xil_Out32(0xf61301cc, 0xb9f4fef0);
    Xil_Out32(0xf61301d0, 0x80000000);
    Xil_Out32(0xf61301d4, 0x30c0003c);
    Xil_Out32(0xf61301d8, 0xb0000148);
    Xil_Out32(0xf61301dc, 0x30a02000);
    Xil_Out32(0xf61301e0, 0xb9f4fedc);
    Xil_Out32(0xf61301e4, 0x80000000);
    Xil_Out32(0xf61301e8, 0x30c0003c);
    Xil_Out32(0xf61301ec, 0xb0000148);
    Xil_Out32(0xf61301f0, 0x30a02001);
    Xil_Out32(0xf61301f4, 0xb9f4fec8);
    Xil_Out32(0xf61301f8, 0x80000000);
    Xil_Out32(0xf61301fc, 0x30c0ffff);
    Xil_Out32(0xf6130200, 0xb0000148);
    Xil_Out32(0xf6130204, 0x30a08000);
    Xil_Out32(0xf6130208, 0xb9f4feb4);
    Xil_Out32(0xf613020c, 0x80000000);
    Xil_Out32(0xf6130210, 0x30c0ffff);
    Xil_Out32(0xf6130214, 0xb0000148);
    Xil_Out32(0xf6130218, 0x30a08001);
    Xil_Out32(0xf613021c, 0xb9f4fea0);
    Xil_Out32(0xf6130220, 0x80000000);
    Xil_Out32(0xf6130224, 0x30c0ffff);
    Xil_Out32(0xf6130228, 0xb0000148);
    Xil_Out32(0xf613022c, 0x30a08002);
    Xil_Out32(0xf6130230, 0xb9f4fe8c);
    Xil_Out32(0xf6130234, 0x80000000);
    Xil_Out32(0xf6130238, 0x30c0ffff);
    Xil_Out32(0xf613023c, 0xb0000148);
    Xil_Out32(0xf6130240, 0x30a08003);
    Xil_Out32(0xf6130244, 0xb9f4fe78);
    Xil_Out32(0xf6130248, 0x80000000);
    Xil_Out32(0xf613024c, 0x30c0ffff);
    Xil_Out32(0xf6130250, 0xb0000148);
    Xil_Out32(0xf6130254, 0x30a08004);
    Xil_Out32(0xf6130258, 0xb9f4fe64);
    Xil_Out32(0xf613025c, 0x80000000);
    Xil_Out32(0xf6130260, 0x30c0ffff);
    Xil_Out32(0xf6130264, 0xb0000148);
    Xil_Out32(0xf6130268, 0x30a08005);
    Xil_Out32(0xf613026c, 0xb9f4fe50);
    Xil_Out32(0xf6130270, 0x80000000);
    Xil_Out32(0xf6130274, 0x30c0ffff);
    Xil_Out32(0xf6130278, 0xb0000148);
    Xil_Out32(0xf613027c, 0x30a08006);
    Xil_Out32(0xf6130280, 0xb9f4fe3c);
    Xil_Out32(0xf6130284, 0x80000000);
    Xil_Out32(0xf6130288, 0x30c0ffff);
    Xil_Out32(0xf613028c, 0xb0000148);
    Xil_Out32(0xf6130290, 0x30a08007);
    Xil_Out32(0xf6130294, 0xb9f4fe28);
    Xil_Out32(0xf6130298, 0x80000000);
    Xil_Out32(0xf613029c, 0xb0000148);
    Xil_Out32(0xf61302a0, 0x30a04003);
    Xil_Out32(0xf61302a4, 0xb9f4fdec);
    Xil_Out32(0xf61302a8, 0x80000000);
    Xil_Out32(0xf61302ac, 0xf8730004);
    Xil_Out32(0xf61302b0, 0xe8730004);
    Xil_Out32(0xf61302b4, 0xa0630020);
    Xil_Out32(0xf61302b8, 0xf8730008);
    Xil_Out32(0xf61302bc, 0xe8d30008);
    Xil_Out32(0xf61302c0, 0xb0000148);
    Xil_Out32(0xf61302c4, 0x30a04003);
    Xil_Out32(0xf61302c8, 0xb9f4fdf4);
    Xil_Out32(0xf61302cc, 0x80000000);
    Xil_Out32(0xf61302d0, 0xe8d30004);
    Xil_Out32(0xf61302d4, 0xb0000148);
    Xil_Out32(0xf61302d8, 0x30a04003);
    Xil_Out32(0xf61302dc, 0xb9f4fde0);
    Xil_Out32(0xf61302e0, 0x80000000);
    Xil_Out32(0xf61302e4, 0xb0000148);
    Xil_Out32(0xf61302e8, 0x30a0402b);
    Xil_Out32(0xf61302ec, 0xb9f4fda4);
    Xil_Out32(0xf61302f0, 0x80000000);
    Xil_Out32(0xf61302f4, 0xf873000c);
    Xil_Out32(0xf61302f8, 0xe873000c);
    Xil_Out32(0xf61302fc, 0xa463fff7);
    Xil_Out32(0xf6130300, 0xf873000c);
    Xil_Out32(0xf6130304, 0xe8d3000c);
    Xil_Out32(0xf6130308, 0xb0000148);
    Xil_Out32(0xf613030c, 0x30a0402b);
    Xil_Out32(0xf6130310, 0xb9f4fdac);
    Xil_Out32(0xf6130314, 0x80000000);
    Xil_Out32(0xf6130318, 0x30c0003c);
    Xil_Out32(0xf613031c, 0xb0000148);
    Xil_Out32(0xf6130320, 0x30a01b00);
    Xil_Out32(0xf6130324, 0xb9f4fd98);
    Xil_Out32(0xf6130328, 0x80000000);
    Xil_Out32(0xf613032c, 0x10c00000);
    Xil_Out32(0xf6130330, 0xb0000148);
    Xil_Out32(0xf6130334, 0x30a01b01);
    Xil_Out32(0xf6130338, 0xb9f4fd84);
    Xil_Out32(0xf613033c, 0x80000000);
    Xil_Out32(0xf6130340, 0x30c0003c);
    Xil_Out32(0xf6130344, 0xb0000148);
    Xil_Out32(0xf6130348, 0x30a01c00);
    Xil_Out32(0xf613034c, 0xb9f4fd70);
    Xil_Out32(0xf6130350, 0x80000000);
    Xil_Out32(0xf6130354, 0x10c00000);
    Xil_Out32(0xf6130358, 0xb0000148);
    Xil_Out32(0xf613035c, 0x30a01c01);
    Xil_Out32(0xf6130360, 0xb9f4fd5c);
    Xil_Out32(0xf6130364, 0x80000000);
    Xil_Out32(0xf6130368, 0x30c0003c);
    Xil_Out32(0xf613036c, 0xb0000148);
    Xil_Out32(0xf6130370, 0x30a01f00);
    Xil_Out32(0xf6130374, 0xb9f4fd48);
    Xil_Out32(0xf6130378, 0x80000000);
    Xil_Out32(0xf613037c, 0x10c00000);
    Xil_Out32(0xf6130380, 0xb0000148);
    Xil_Out32(0xf6130384, 0x30a01f01);
    Xil_Out32(0xf6130388, 0xb9f4fd34);
    Xil_Out32(0xf613038c, 0x80000000);
    Xil_Out32(0xf6130390, 0x30c0003c);
    Xil_Out32(0xf6130394, 0xb0000148);
    Xil_Out32(0xf6130398, 0x30a02000);
    Xil_Out32(0xf613039c, 0xb9f4fd20);
    Xil_Out32(0xf61303a0, 0x80000000);
    Xil_Out32(0xf61303a4, 0x10c00000);
    Xil_Out32(0xf61303a8, 0xb0000148);
    Xil_Out32(0xf61303ac, 0x30a02001);
    Xil_Out32(0xf61303b0, 0xb9f4fd0c);
    Xil_Out32(0xf61303b4, 0x80000000);
    Xil_Out32(0xf61303b8, 0x30c0003c);
    Xil_Out32(0xf61303bc, 0xb0000148);
    Xil_Out32(0xf61303c0, 0x30a01d00);
    Xil_Out32(0xf61303c4, 0xb9f4fcf8);
    Xil_Out32(0xf61303c8, 0x80000000);
    Xil_Out32(0xf61303cc, 0x10c00000);
    Xil_Out32(0xf61303d0, 0xb0000148);
    Xil_Out32(0xf61303d4, 0x30a01d01);
    Xil_Out32(0xf61303d8, 0xb9f4fce4);
    Xil_Out32(0xf61303dc, 0x80000000);
    Xil_Out32(0xf61303e0, 0x30c0003c);
    Xil_Out32(0xf61303e4, 0xb0000148);
    Xil_Out32(0xf61303e8, 0x30a01e00);
    Xil_Out32(0xf61303ec, 0xb9f4fcd0);
    Xil_Out32(0xf61303f0, 0x80000000);
    Xil_Out32(0xf61303f4, 0x10c00000);
    Xil_Out32(0xf61303f8, 0xb0000148);
    Xil_Out32(0xf61303fc, 0x30a01e01);
    Xil_Out32(0xf6130400, 0xb9f4fcbc);
    Xil_Out32(0xf6130404, 0x80000000);
    Xil_Out32(0xf6130408, 0xb000feed);
    Xil_Out32(0xf613040c, 0x30c00001);
    Xil_Out32(0xf6130410, 0xb0000001);
    Xil_Out32(0xf6130414, 0x30a0fffc);
    Xil_Out32(0xf6130418, 0xb9f4fca4);
    Xil_Out32(0xf613041c, 0x80000000);
    Xil_Out32(0xf6130420, 0x80000000);
    Xil_Out32(0xf6130424, 0xe9e10000);
    Xil_Out32(0xf6130428, 0x10330000);
    Xil_Out32(0xf613042c, 0xea610010);
    Xil_Out32(0xf6130430, 0x30210014);
    Xil_Out32(0xf6130434, 0xb60f0008);
    Xil_Out32(0xf6130438, 0x80000000);
    Xil_Out32(0xf613043c, 0x3021ffe0);
    Xil_Out32(0xf6130440, 0xf9e10000);
    Xil_Out32(0xf6130444, 0xfa61001c);
    Xil_Out32(0xf6130448, 0x12610000);
    Xil_Out32(0xf613044c, 0x30a00009);
    Xil_Out32(0xf6130450, 0xb9f4fcac);
    Xil_Out32(0xf6130454, 0x80000000);
    Xil_Out32(0xf6130458, 0x80000000);
    Xil_Out32(0xf613045c, 0xe9e10000);
    Xil_Out32(0xf6130460, 0x10330000);
    Xil_Out32(0xf6130464, 0xea61001c);
    Xil_Out32(0xf6130468, 0x30210020);
    Xil_Out32(0xf613046c, 0xb60f0008);
    Xil_Out32(0xf6130470, 0x80000000);
    Xil_Out32(0xf6130474, 0x3021ffe0);
    Xil_Out32(0xf6130478, 0xf9e10000);
    Xil_Out32(0xf613047c, 0xfa61001c);
    Xil_Out32(0xf6130480, 0x12610000);
    Xil_Out32(0xf6130484, 0xb000ffff);
    Xil_Out32(0xf6130488, 0xb9f4ffb4);
    Xil_Out32(0xf613048c, 0x80000000);
    Xil_Out32(0xf6130490, 0x10600000);
    Xil_Out32(0xf6130494, 0xe9e10000);
    Xil_Out32(0xf6130498, 0x10330000);
    Xil_Out32(0xf613049c, 0xea61001c);
    Xil_Out32(0xf61304a0, 0x30210020);
    Xil_Out32(0xf61304a4, 0xb60f0008);
    Xil_Out32(0xf61304a8, 0x80000000);
    prog_reg(0xF6110200, 0x2, 0x4, 0x0);
    prog_reg(0xF6110200, 0x0, 0x3, 0x0);
    prog_reg(0xF6070750, 0x0, 0x1, 0x1);
    dbg0_pmc(16392);
    poll_for(0xf612fffc, 0xffffffff, 0x00000000, 0xfeed0001);
    dbg0_pmc(16393);
    //**************Register programming from  ddr_ub end---------------- 

    dbg0_pmc(16389);
    //**************Register programming from ddrmc_init start---------------- 

    dbg0_pmc(16390);
    Xil_Out32(0xf6070038, 0x00000000); //reg_adec0
    Xil_Out32(0xf607003c, 0x00000000); //reg_adec1
    Xil_Out32(0xf6070040, 0x00100080); //reg_adec2
    Xil_Out32(0xf6070044, 0x00000078); //reg_adec3
    Xil_Out32(0xf6070048, 0x00000021); //reg_adec4
    Xil_Out32(0xf607004c, 0x15513491); //reg_adec5
    Xil_Out32(0xf6070050, 0x1a6585d6); //reg_adec6
    Xil_Out32(0xf6070054, 0x1f79d71b); //reg_adec7
    Xil_Out32(0xf6070058, 0x03000020); //reg_adec8
    Xil_Out32(0xf607005c, 0x09207144); //reg_adec9
    Xil_Out32(0xf6070060, 0x0f34c2ca); //reg_adec10
    Xil_Out32(0xf6070064, 0x0000e190); //reg_adec11
    Xil_Out32(0xf6070444, 0x00100401); //reg_qos1
    Xil_Out32(0xf6070448, 0x00000401); //reg_qos2
    Xil_Out32(0xf6150200, 0x1a410404); //reg_safe_config0
    Xil_Out32(0xf6150204, 0x0081c207); //reg_safe_config1
    Xil_Out32(0xf6150208, 0x4252c3cf); //reg_safe_config2
    Xil_Out32(0xf615020c, 0x02044899); //reg_safe_config3
    Xil_Out32(0xf6150210, 0x250050e6); //reg_safe_config4
    Xil_Out32(0xf6150214, 0x00001016); //reg_safe_config5
    Xil_Out32(0xf6150218, 0x00c830c0); //reg_safe_config6
    Xil_Out32(0xf615021c, 0x00040230); //reg_safe_config7
    Xil_Out32(0xf6150220, 0x00000200); //reg_safe_config8
    Xil_Out32(0xf6150224, 0x08ca4855); //reg_retry_0
    Xil_Out32(0xf6150228, 0x145f6ac0); //reg_retry_1
    Xil_Out32(0xf6150230, 0x00001002); //reg_ref_3
    Xil_Out32(0xf6150234, 0x00014248); //reg_com_3
    Xil_Out32(0xf6150238, 0xf820079e); //reg_mrs_0
    Xil_Out32(0xf615024c, 0x1001001d); //reg_rd_config
    Xil_Out32(0xf6150250, 0x00006071); //reg_pt_config
    Xil_Out32(0xf6150258, 0x00c04210); //reg_config0
    Xil_Out32(0xf615025c, 0x00000009); //reg_pinout
    Xil_Out32(0xf6150268, 0x00000013); //seq_init_cmd_valid
    Xil_Out32(0xf615026c, 0x0001901f); //seq_init_cmd0
    Xil_Out32(0xf6150270, 0x0002585f); //seq_init_cmd1
    Xil_Out32(0xf6150274, 0x0004807f); //seq_init_cmd2
    Xil_Out32(0xf6150278, 0x00000268); //seq_init_cmd3
    Xil_Out32(0xf615027c, 0x00001468); //seq_init_cmd4
    Xil_Out32(0xf6150280, 0x00000268); //seq_init_cmd5
    Xil_Out32(0xf6150284, 0x00001468); //seq_init_cmd6
    Xil_Out32(0xf6150288, 0x00000268); //seq_init_cmd7
    Xil_Out32(0xf615028c, 0x00001468); //seq_init_cmd8
    Xil_Out32(0xf6150290, 0x00000268); //seq_init_cmd9
    Xil_Out32(0xf6150294, 0x00001468); //seq_init_cmd10
    Xil_Out32(0xf6150298, 0x00000268); //seq_init_cmd11
    Xil_Out32(0xf615029c, 0x00001468); //seq_init_cmd12
    Xil_Out32(0xf61502a0, 0x00000268); //seq_init_cmd13
    Xil_Out32(0xf61502a4, 0x00001468); //seq_init_cmd14
    Xil_Out32(0xf61502a8, 0x00000268); //seq_init_cmd15
    Xil_Out32(0xf61502ac, 0x00003068); //seq_init_cmd16
    Xil_Out32(0xf61502b0, 0x0000026e); //seq_init_cmd17
    Xil_Out32(0xf61502b4, 0x0008006e); //seq_init_cmd18
    Xil_Out32(0xf61502b8, 0x00000000); //seq_init_cmd19
    Xil_Out32(0xf61502bc, 0x00000000); //seq_init_cmd20
    Xil_Out32(0xf61502c0, 0x00000000); //seq_init_cmd21
    Xil_Out32(0xf61502c4, 0x00000000); //seq_init_cmd22
    Xil_Out32(0xf61502c8, 0x00000000); //seq_init_cmd23
    Xil_Out32(0xf61502cc, 0x00000000); //seq_init_cmd24
    Xil_Out32(0xf61502d0, 0x00000000); //seq_init_cmd25
    Xil_Out32(0xf61502d4, 0x00000000); //seq_init_cmd26
    Xil_Out32(0xf61502d8, 0x00000000); //seq_init_cmd27
    Xil_Out32(0xf61502dc, 0x00000000); //seq_init_cmd28
    Xil_Out32(0xf61502e0, 0x00000000); //seq_init_cmd29
    Xil_Out32(0xf61502e4, 0x00000000); //seq_init_cmd30
    Xil_Out32(0xf61502ec, 0x0000007f); //seq_init_cmd_set
    Xil_Out32(0xf61502f0, 0x00000000); //seq_init_addr0
    Xil_Out32(0xf61502f4, 0x00000000); //seq_init_addr1
    Xil_Out32(0xf61502f8, 0x00000000); //seq_init_addr2
    Xil_Out32(0xf61502fc, 0x03000400); //seq_init_addr3
    Xil_Out32(0xf6150300, 0x03000400); //seq_init_addr4
    Xil_Out32(0xf6150304, 0x06001000); //seq_init_addr5
    Xil_Out32(0xf6150308, 0x06001000); //seq_init_addr6
    Xil_Out32(0xf615030c, 0x05000500); //seq_init_addr7
    Xil_Out32(0xf6150310, 0x05000500); //seq_init_addr8
    Xil_Out32(0xf6150314, 0x04000000); //seq_init_addr9
    Xil_Out32(0xf6150318, 0x04000000); //seq_init_addr10
    Xil_Out32(0xf615031c, 0x02000028); //seq_init_addr11
    Xil_Out32(0xf6150320, 0x02000028); //seq_init_addr12
    Xil_Out32(0xf6150324, 0x01000001); //seq_init_addr13
    Xil_Out32(0xf6150328, 0x01000001); //seq_init_addr14
    Xil_Out32(0xf615032c, 0x00000d50); //seq_init_addr15
    Xil_Out32(0xf6150330, 0x00000d50); //seq_init_addr16
    Xil_Out32(0xf6150334, 0x00000524); //seq_init_addr17
    Xil_Out32(0xf6150338, 0x00000524); //seq_init_addr18
    Xil_Out32(0xf615033c, 0x00000000); //seq_init_addr19
    Xil_Out32(0xf6150340, 0x00000000); //seq_init_addr20
    Xil_Out32(0xf6150344, 0x00000000); //seq_init_addr21
    Xil_Out32(0xf6150348, 0x00000000); //seq_init_addr22
    Xil_Out32(0xf615034c, 0x00000000); //seq_init_addr23
    Xil_Out32(0xf6150350, 0x00000000); //seq_init_addr24
    Xil_Out32(0xf6150354, 0x00000000); //seq_init_addr25
    Xil_Out32(0xf6150358, 0x00000000); //seq_init_addr26
    Xil_Out32(0xf615035c, 0x00000000); //seq_init_addr27
    Xil_Out32(0xf6150360, 0x00000000); //seq_init_addr28
    Xil_Out32(0xf6150364, 0x00000000); //seq_init_addr29
    Xil_Out32(0xf6150368, 0x00000000); //seq_init_addr30
    Xil_Out32(0xf6150378, 0x00000001); //seq_init_cntrl0
    Xil_Out32(0xf615037c, 0x00000001); //seq_init_cntrl1
    Xil_Out32(0xf6150380, 0x00000000); //seq_init_cntrl2
    Xil_Out32(0xf6150384, 0x00000010); //seq_init_cntrl3
    Xil_Out32(0xf6150388, 0x00000022); //seq_init_cntrl4
    Xil_Out32(0xf615038c, 0x00000010); //seq_init_cntrl5
    Xil_Out32(0xf6150390, 0x00000022); //seq_init_cntrl6
    Xil_Out32(0xf6150394, 0x00000010); //seq_init_cntrl7
    Xil_Out32(0xf6150398, 0x00000022); //seq_init_cntrl8
    Xil_Out32(0xf615039c, 0x00000010); //seq_init_cntrl9
    Xil_Out32(0xf61503a0, 0x00000022); //seq_init_cntrl10
    Xil_Out32(0xf61503a4, 0x00000010); //seq_init_cntrl11
    Xil_Out32(0xf61503a8, 0x00000022); //seq_init_cntrl12
    Xil_Out32(0xf61503ac, 0x00000010); //seq_init_cntrl13
    Xil_Out32(0xf61503b0, 0x00000022); //seq_init_cntrl14
    Xil_Out32(0xf61503b4, 0x00000010); //seq_init_cntrl15
    Xil_Out32(0xf61503b8, 0x00000022); //seq_init_cntrl16
    Xil_Out32(0xf61503bc, 0x00000010); //seq_init_cntrl17
    Xil_Out32(0xf61503c0, 0x00000022); //seq_init_cntrl18
    Xil_Out32(0xf61503c4, 0x00000010); //seq_init_cntrl19
    Xil_Out32(0xf61503c8, 0x00000022); //seq_init_cntrl20
    Xil_Out32(0xf61503cc, 0x00000010); //seq_init_cntrl21
    Xil_Out32(0xf61503d0, 0x00000022); //seq_init_cntrl22
    Xil_Out32(0xf61503d4, 0x00000010); //seq_init_cntrl23
    Xil_Out32(0xf61503d8, 0x00000022); //seq_init_cntrl24
    Xil_Out32(0xf61503dc, 0x00000010); //seq_init_cntrl25
    Xil_Out32(0xf61503e0, 0x00000022); //seq_init_cntrl26
    Xil_Out32(0xf61503e4, 0x00000010); //seq_init_cntrl27
    Xil_Out32(0xf61503e8, 0x00000022); //seq_init_cntrl28
    Xil_Out32(0xf61503ec, 0x00000010); //seq_init_cntrl29
    Xil_Out32(0xf61503f0, 0x00000022); //seq_init_cntrl30
    Xil_Out32(0xf615081c, 0x00000001); //reg_scrub9
    Xil_Out32(0xf6150820, 0x00000006); //reg_config1
    Xil_Out32(0xf6150828, 0x00000001); //cal_mode
    Xil_Out32(0xf615082c, 0x00000016); //phy_rden0
    Xil_Out32(0xf6150830, 0x00000016); //phy_rden1
    Xil_Out32(0xf6150834, 0x00000016); //phy_rden2
    Xil_Out32(0xf6150838, 0x00000016); //phy_rden3
    Xil_Out32(0xf615083c, 0x00000016); //phy_rden4
    Xil_Out32(0xf6150840, 0x00000016); //phy_rden5
    Xil_Out32(0xf6150844, 0x00000016); //phy_rden6
    Xil_Out32(0xf6150848, 0x00000016); //phy_rden7
    Xil_Out32(0xf615084c, 0x00000016); //phy_rden8
    Xil_Out32(0xf6150850, 0x00000016); //phy_rden9
    Xil_Out32(0xf6150854, 0x00000016); //phy_rden10
    Xil_Out32(0xf6150858, 0x00000016); //phy_rden11
    Xil_Out32(0xf615085c, 0x00000016); //phy_rden12
    Xil_Out32(0xf6150860, 0x00000016); //phy_rden13
    Xil_Out32(0xf6150864, 0x00000016); //phy_rden14
    Xil_Out32(0xf6150868, 0x00000016); //phy_rden15
    Xil_Out32(0xf615086c, 0x00000016); //phy_rden16
    Xil_Out32(0xf6150870, 0x00000016); //phy_rden17
    Xil_Out32(0xf6150874, 0x00000016); //phy_rden18
    Xil_Out32(0xf6150878, 0x00000016); //phy_rden19
    Xil_Out32(0xf615087c, 0x00000016); //phy_rden20
    Xil_Out32(0xf6150880, 0x00000016); //phy_rden21
    Xil_Out32(0xf6150884, 0x00000016); //phy_rden22
    Xil_Out32(0xf6150888, 0x00000016); //phy_rden23
    Xil_Out32(0xf615088c, 0x00000016); //phy_rden24
    Xil_Out32(0xf6150890, 0x00000016); //phy_rden25
    Xil_Out32(0xf6150894, 0x00000016); //phy_rden26
    Xil_Out32(0xf6150898, 0x0000000f); //fifo_rden
    Xil_Out32(0xf615089c, 0x07ffffff); //dlyctl_en
    Xil_Out32(0xf61508a8, 0x00000000); //seq_mode
    Xil_Out32(0xf61508ac, 0x0000007f); //seq_cmd_default
    Xil_Out32(0xf61508cc, 0x00000000); //seq_ck_cal
    Xil_Out32(0xf615093c, 0x00000410); //xpi_oe_all_nib
    Xil_Out32(0xf6150940, 0x00000410); //xpi_wrdata_all_nib
    Xil_Out32(0xf6150944, 0x00000101); //xpi_oe
    Xil_Out32(0xf6150948, 0x000000c0); //xpi_pmi_config
    Xil_Out32(0xf6150950, 0x00000001); //xpi_write_dm_dbi
    Xil_Out32(0xf6150958, 0x000000a5); //default_pattern
    Xil_Out32(0xf615095c, 0x00000000); //t_txbit
    Xil_Out32(0xf6150c04, 0x0000000f); //xpi_dqs
    //**************Register programming from ddrmc_init end---------------- 

    dbg0_pmc(16391);
    Xil_Out32(0xf6150960, 0x00000640);

    dbg0_pmc(16402);
    prog_reg(0xF6150E34, 0x0, 0x1, 0x1);
    dbg0_pmc(16396);
    poll_for(0xf6150e38, 0x00000001, 0x00000000, 0x00000001);
    dbg0_pmc(16397);
    dbg0_pmc(16399);
    //  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

    // msoc_driver.c ver. 1.23, May  9 2018 , 06:25:40 ********

    //  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // ---- INPUT PARAMETERS ----
    // sim mode    = 1
    // ecc         = 0
    // bus width   = 64
    // dram width  = 8
    // memory type = DDR4
    // speed bin   = 3200
    // frequency   = 1600.000 MHz
    // device capacity  = 8192 Mbit
    // rank   addr cnt  = 1
    // bank group  addr cnt  = 0
    // bank   addr cnt  = 0
    // row    addr cnt  = 0
    // column addr cnt  = 10
    // temp ctrl ref mode = 0
    // temp ctrl ref range = 0
    // CL   = 22 cycles
    // CWL  = 16 cycles
    // tRCD = 22 cycles
    // tRP  = 22 cycles
    // AL   = 0 cycles
    // BL   = 8 (burst length)
    // tRC  = 45.750 nsec
    // tRAS = 32.000 nsec
    // tFAW = 21.000 nsec
    // clock_stop_en = 0
    // rd_dbi = 0
    // wr_dbi = 0
    // ecc_scrub = 0
    // ecc_poison = 0
    // parity = 0
    // ca_parity_latency = 0
    // geardown = 0
    // cal_mode_en = 0
    // udimm = 0
    // rdimm = 0
    // addr_mirror = 0
    // dimm_addr_mirror = 0
    // crc = 0
    // brc_mapping = 0
    // wr_preamble = 0
    // rd_preamble = 0
    // wr_postamble = 0
    // rd_postamble = 0

    Xil_Out32(0xf6150cb0, 0x00000002);
    Xil_Out32(0xf6110000, 0x000000fe);
    Xil_Out32(0xf6110004, 0x00000d00);


}
#endif

#ifdef XPMCFW_HW60
void msoc_hsr_init()
{

unsigned int rdata;

Xil_Out32(0xf615000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0.ddrmc_main_bank.pcsr_lock
Xil_Out32(0xf611000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0_ub.ddrmc_ub_bank.ddrmc_pcsr_lock
dbg0_pmc(16398);
//**************Register programming from noc_cfg start----------------
dbg0_pmc(16384);
// 105:NOC_CONFIG selected : noc_cfg_ddr_vnoc_noitlv.c
Xil_Out32(0xf600000c, 0xf9e8d7c6);
Xil_Out32(0xf6000000, 0x00000002);
Xil_Out32(0xf6000004, 0x00000000);
Xil_Out32(0xf6000100, 0x39852310);
Xil_Out32(0xf600000c, 0x00000000);
Xil_Out32(0xf606000c, 0xf9e8d7c6);
Xil_Out32(0xf6060000, 0x0000023e);
Xil_Out32(0xf6060004, 0x00000200);
Xil_Out32(0xf60603a0, 0x00000030);
Xil_Out32(0xf6060110, 0xfff0aaaa);
Xil_Out32(0xf6060120, 0xfff0aaaa);
Xil_Out32(0xf6060124, 0x55500000);
Xil_Out32(0xf6060154, 0x55500000);
Xil_Out32(0xf6060168, 0xfff0aaaa);
Xil_Out32(0xf606016c, 0x55500000);
Xil_Out32(0xf6060180, 0xfffaaaaa);
Xil_Out32(0xf6060184, 0x555a0000);
Xil_Out32(0xf60601c0, 0xfffaaaaa);
Xil_Out32(0xf60601c4, 0x555a0000);
Xil_Out32(0xf6060240, 0xfff0aaaa);
Xil_Out32(0xf6060244, 0x55500000);
Xil_Out32(0xf6060290, 0xfff0aaaa);
Xil_Out32(0xf6060294, 0x55500000);
Xil_Out32(0xf6060320, 0xffffaafa);
Xil_Out32(0xf6060324, 0x555500f0);
Xil_Out32(0xf6060328, 0xffffaa5a);
Xil_Out32(0xf606032c, 0x55550050);
Xil_Out32(0xf606000c, 0x00000000);
Xil_Out32(0xf603200c, 0xf9e8d7c6);
Xil_Out32(0xf6032000, 0x0000023e);
Xil_Out32(0xf6032004, 0x00000200);
Xil_Out32(0xf60323a0, 0x00000040);
Xil_Out32(0xf6032100, 0xffafaaaa);
Xil_Out32(0xf6032110, 0xffffaaa5);
Xil_Out32(0xf6032118, 0xff0faaaa);
Xil_Out32(0xf6032128, 0xff0faaaa);
Xil_Out32(0xf6032150, 0xffffaaa5);
Xil_Out32(0xf6032168, 0xffffaaa5);
Xil_Out32(0xf6032240, 0xffffaaa5);
Xil_Out32(0xf6032290, 0xffffaaa5);
Xil_Out32(0xf6032320, 0xffafaaaa);
Xil_Out32(0xf6032328, 0xffffaaa5);
Xil_Out32(0xf603232c, 0x55550005);
Xil_Out32(0xf603200c, 0x00000000);
Xil_Out32(0xf603000c, 0xf9e8d7c6);
Xil_Out32(0xf6030000, 0x0000023e);
Xil_Out32(0xf6030004, 0x00000200);
Xil_Out32(0xf60303a0, 0x00000020);
Xil_Out32(0xf6030104, 0x555500f0);
Xil_Out32(0xf6030108, 0xffffaafa);
Xil_Out32(0xf603010c, 0x555500f0);
Xil_Out32(0xf603011c, 0x555500f0);
Xil_Out32(0xf6030124, 0x555a00f0);
Xil_Out32(0xf603012c, 0x555500f0);
Xil_Out32(0xf603013c, 0x555500f0);
Xil_Out32(0xf6030154, 0x555a0000);
Xil_Out32(0xf603016c, 0x555a0000);
Xil_Out32(0xf60301c0, 0xffafaaaa);
Xil_Out32(0xf6030244, 0x555a0000);
Xil_Out32(0xf6030294, 0x555a0000);
Xil_Out32(0xf6030324, 0x55500000);
Xil_Out32(0xf603032c, 0x55550005);
Xil_Out32(0xf603000c, 0x00000000);
Xil_Out32(0xf60a000c, 0xf9e8d7c6);
Xil_Out32(0xf60a0000, 0x0000023e);
Xil_Out32(0xf60a0004, 0x00000200);
Xil_Out32(0xf60a03a0, 0x00000060);
Xil_Out32(0xf60a0104, 0x55050000);
Xil_Out32(0xf60a010c, 0x55050000);
Xil_Out32(0xf60a011c, 0x55050000);
Xil_Out32(0xf60a0120, 0xffffaaaf);
Xil_Out32(0xf60a0124, 0x5505000f);
Xil_Out32(0xf60a012c, 0x55050000);
Xil_Out32(0xf60a013c, 0x55050000);
Xil_Out32(0xf60a0154, 0x55a50000);
Xil_Out32(0xf60a0168, 0xfffaaaaa);
Xil_Out32(0xf60a0180, 0xfffaaaaa);
Xil_Out32(0xf60a01c0, 0xfffaaaaa);
Xil_Out32(0xf60a01c4, 0x55a50000);
Xil_Out32(0xf60a01cc, 0x55a50000);
Xil_Out32(0xf60a0240, 0xfffaaaaa);
Xil_Out32(0xf60a0290, 0xfffaaaaa);
Xil_Out32(0xf60a0324, 0x55550050);
Xil_Out32(0xf60a000c, 0x00000000);
Xil_Out32(0xf606200c, 0xf9e8d7c6);
Xil_Out32(0xf6062000, 0x0000023e);
Xil_Out32(0xf6062004, 0x00000200);
Xil_Out32(0xf60623a0, 0x00000010);
Xil_Out32(0xf6062110, 0xffffaaaf);
Xil_Out32(0xf6062120, 0xffffaaaf);
Xil_Out32(0xf6062124, 0x5555000f);
Xil_Out32(0xf606213c, 0x55a50000);
Xil_Out32(0xf6062168, 0xffffaaaf);
Xil_Out32(0xf6062180, 0xfffaaaaa);
Xil_Out32(0xf60621c0, 0xfffaaaaa);
Xil_Out32(0xf6062240, 0xffffaaaf);
Xil_Out32(0xf6062290, 0xffffaaaf);
Xil_Out32(0xf6062324, 0x55550050);
Xil_Out32(0xf606232c, 0x55050000);
Xil_Out32(0xf606200c, 0x00000000);
Xil_Out32(0xf617000c, 0xf9e8d7c6);
Xil_Out32(0xf6170000, 0x0000023e);
Xil_Out32(0xf6170004, 0x00000200);
Xil_Out32(0xf61703a0, 0x000000a0);
Xil_Out32(0xf6170118, 0xff0faaaa);
Xil_Out32(0xf6170124, 0x55500000);
Xil_Out32(0xf617016c, 0x555a0000);
Xil_Out32(0xf6170184, 0x555a0000);
Xil_Out32(0xf61701c0, 0xffafaaaa);
Xil_Out32(0xf61701c4, 0x555a0000);
Xil_Out32(0xf6170244, 0x555a0000);
Xil_Out32(0xf6170294, 0x555a0000);
Xil_Out32(0xf6170320, 0xffffaafa);
Xil_Out32(0xf6170324, 0x555500f0);
Xil_Out32(0xf6170328, 0xffffaaa5);
Xil_Out32(0xf617032c, 0x55550005);
Xil_Out32(0xf617000c, 0x00000000);
Xil_Out32(0xf6e6400c, 0xf9e8d7c6);
Xil_Out32(0xf6e64000, 0x0000023e);
Xil_Out32(0xf6e64004, 0x00000200);
Xil_Out32(0xf6e643a0, 0x00000180);
Xil_Out32(0xf6e64114, 0x555a0000);
Xil_Out32(0xf6e64124, 0x555a0000);
Xil_Out32(0xf6e6416c, 0x555a0000);
Xil_Out32(0xf6e64244, 0x555a0000);
Xil_Out32(0xf6e64294, 0x555a0000);
Xil_Out32(0xf6e64320, 0xffffaafa);
Xil_Out32(0xf6e64324, 0x555500f0);
Xil_Out32(0xf6e64328, 0xff0faaaa);
Xil_Out32(0xf6e64338, 0xff0faaaa);
Xil_Out32(0xf6e64348, 0xff0faaaa);
Xil_Out32(0xf6e64360, 0xffffaaa5);
Xil_Out32(0xf6e6436c, 0x55500000);
Xil_Out32(0xf6e6400c, 0x00000000);
Xil_Out32(0xf61d200c, 0xf9e8d7c6);
Xil_Out32(0xf61d2000, 0x0000023e);
Xil_Out32(0xf61d2004, 0x00000200);
Xil_Out32(0xf61d23a0, 0x000000c0);
Xil_Out32(0xf61d2100, 0xffffaaff);
Xil_Out32(0xf61d2104, 0x555500f0);
Xil_Out32(0xf61d2110, 0xffffaaa5);
Xil_Out32(0xf61d2120, 0xffffaaf5);
Xil_Out32(0xf61d2124, 0x555500f0);
Xil_Out32(0xf61d2128, 0xffafaaaa);
Xil_Out32(0xf61d2130, 0xffffaa5a);
Xil_Out32(0xf61d2134, 0x55550050);
Xil_Out32(0xf61d2150, 0xffffaa55);
Xil_Out32(0xf61d2154, 0x55550050);
Xil_Out32(0xf61d2168, 0xffffaaa5);
Xil_Out32(0xf61d216c, 0x55550005);
Xil_Out32(0xf61d2180, 0xfff0aaaa);
Xil_Out32(0xf61d2184, 0x555a0000);
Xil_Out32(0xf61d21c0, 0xff0aaaaa);
Xil_Out32(0xf61d21c4, 0x550a0000);
Xil_Out32(0xf61d2240, 0xffffaaa5);
Xil_Out32(0xf61d2244, 0x55550005);
Xil_Out32(0xf61d2290, 0xffffaaa5);
Xil_Out32(0xf61d200c, 0x00000000);
Xil_Out32(0xf61d000c, 0xf9e8d7c6);
Xil_Out32(0xf61d0000, 0x0000023e);
Xil_Out32(0xf61d0004, 0x00000200);
Xil_Out32(0xf61d03a0, 0x000000c0);
Xil_Out32(0xf61d0104, 0x555500f0);
Xil_Out32(0xf61d0114, 0x55500000);
Xil_Out32(0xf61d0124, 0x555000f0);
Xil_Out32(0xf61d012c, 0x555500f0);
Xil_Out32(0xf61d0134, 0x55050000);
Xil_Out32(0xf61d0154, 0x55000000);
Xil_Out32(0xf61d016c, 0x555a0000);
Xil_Out32(0xf61d0180, 0xffffaaaf);
Xil_Out32(0xf61d01c0, 0xffffaaff);
Xil_Out32(0xf61d01c4, 0x555500f0);
Xil_Out32(0xf61d0244, 0x555a0000);
Xil_Out32(0xf61d0294, 0x555a0000);
Xil_Out32(0xf61d000c, 0x00000000);
Xil_Out32(0xf6e9400c, 0xf9e8d7c6);
Xil_Out32(0xf6e94000, 0x0000023e);
Xil_Out32(0xf6e94004, 0x00000200);
Xil_Out32(0xf6e943a0, 0x00000180);
Xil_Out32(0xf6e94100, 0xffafaaaa);
Xil_Out32(0xf6e94104, 0x555a0000);
Xil_Out32(0xf6e94120, 0xffafaaaa);
Xil_Out32(0xf6e94124, 0x555a0000);
Xil_Out32(0xf6e94130, 0xffafaaaa);
Xil_Out32(0xf6e94150, 0xffafaaaa);
Xil_Out32(0xf6e9416c, 0x555a0000);
Xil_Out32(0xf6e94244, 0x555a0000);
Xil_Out32(0xf6e94294, 0x555a0000);
Xil_Out32(0xf6e94320, 0xffafaaaa);
Xil_Out32(0xf6e9432c, 0x555500f0);
Xil_Out32(0xf6e94330, 0xff0faaaa);
Xil_Out32(0xf6e94364, 0x555a0000);
Xil_Out32(0xf6e94368, 0xffffaaa5);
Xil_Out32(0xf6e9436c, 0x55550005);
Xil_Out32(0xf6e94374, 0x55550005);
Xil_Out32(0xf6e9437c, 0x55550005);
Xil_Out32(0xf6e9400c, 0x00000000);
Xil_Out32(0xf61d400c, 0xf9e8d7c6);
Xil_Out32(0xf61d4000, 0x0000023e);
Xil_Out32(0xf61d4004, 0x00000200);
Xil_Out32(0xf61d43a0, 0x000000c0);
Xil_Out32(0xf61d4100, 0xff00aaaa);
Xil_Out32(0xf61d4120, 0xff0faaaa);
Xil_Out32(0xf61d4168, 0xffffaaa5);
Xil_Out32(0xf61d4180, 0xffffaaa5);
Xil_Out32(0xf61d41c0, 0xffffaa55);
Xil_Out32(0xf61d4240, 0xffffaaa5);
Xil_Out32(0xf61d4290, 0xffffaaa5);
Xil_Out32(0xf61d400c, 0x00000000);
Xil_Out32(0xf617200c, 0xf9e8d7c6);
Xil_Out32(0xf6172000, 0x0000023e);
Xil_Out32(0xf6172004, 0x00000200);
Xil_Out32(0xf6172120, 0xfff0aaaa);
Xil_Out32(0xf6172168, 0xfffaaaaa);
Xil_Out32(0xf6172180, 0xfffaaaaa);
Xil_Out32(0xf61721c0, 0xfffaaaaa);
Xil_Out32(0xf61721c4, 0x55a50000);
Xil_Out32(0xf6172240, 0xfffaaaaa);
Xil_Out32(0xf6172290, 0xfffaaaaa);
Xil_Out32(0xf6172320, 0xffffaa5a);
Xil_Out32(0xf6172324, 0x55550050);
Xil_Out32(0xf617232c, 0x5555000f);
Xil_Out32(0xf617200c, 0x00000000);
Xil_Out32(0xf610000c, 0xf9e8d7c6);
Xil_Out32(0xf6100000, 0x0000023e);
Xil_Out32(0xf6100004, 0x00000200);
Xil_Out32(0xf61003a0, 0x00000050);
Xil_Out32(0xf6100124, 0x55500000);
Xil_Out32(0xf610016c, 0x555a0000);
Xil_Out32(0xf6100184, 0x555a0000);
Xil_Out32(0xf61001c4, 0x555a0000);
Xil_Out32(0xf6100244, 0x555a0000);
Xil_Out32(0xf6100294, 0x555a0000);
Xil_Out32(0xf6100320, 0xffffaafa);
Xil_Out32(0xf6100324, 0x555500f0);
Xil_Out32(0xf610000c, 0x00000000);
Xil_Out32(0xf610200c, 0xf9e8d7c6);
Xil_Out32(0xf6102000, 0x0000023e);
Xil_Out32(0xf6102004, 0x00000200);
Xil_Out32(0xf61023a0, 0x00000070);
Xil_Out32(0xf6102124, 0x55500000);
Xil_Out32(0xf610216c, 0x555a0000);
Xil_Out32(0xf6102184, 0x555a0000);
Xil_Out32(0xf61021c4, 0x555a0000);
Xil_Out32(0xf6102244, 0x555a0000);
Xil_Out32(0xf6102294, 0x555a0000);
Xil_Out32(0xf6102320, 0xffffaafa);
Xil_Out32(0xf6102324, 0x555500f0);
Xil_Out32(0xf610200c, 0x00000000);
Xil_Out32(0xf60a200c, 0xf9e8d7c6);
Xil_Out32(0xf60a2000, 0x0000023e);
Xil_Out32(0xf60a2004, 0x00000200);
Xil_Out32(0xf60a23a0, 0x00000040);
Xil_Out32(0xf60a2168, 0xfffaaaaa);
Xil_Out32(0xf60a2180, 0xfffaaaaa);
Xil_Out32(0xf60a21c0, 0xfffaaaaa);
Xil_Out32(0xf60a2240, 0xfffaaaaa);
Xil_Out32(0xf60a2290, 0xfffaaaaa);
Xil_Out32(0xf60a2320, 0xffffaa5a);
Xil_Out32(0xf60a2324, 0x55550050);
Xil_Out32(0xf60a2328, 0xfff0aaaa);
Xil_Out32(0xf60a200c, 0x00000000);
Xil_Out32(0xf624000c, 0xf9e8d7c6);
Xil_Out32(0xf6240000, 0x0000023e);
Xil_Out32(0xf6240004, 0x00000200);
Xil_Out32(0xf62403a0, 0x000000c0);
Xil_Out32(0xf6240100, 0xff0faaaa);
Xil_Out32(0xf6240108, 0xff0faaaa);
Xil_Out32(0xf6240118, 0xff0faaaa);
Xil_Out32(0xf6240120, 0xff0faaaa);
Xil_Out32(0xf6240128, 0xff0faaaa);
Xil_Out32(0xf6240130, 0xff0faaaa);
Xil_Out32(0xf6240150, 0xff0faaaa);
Xil_Out32(0xf6240168, 0xffffaaa5);
Xil_Out32(0xf624016c, 0x55550005);
Xil_Out32(0xf62401c0, 0xff0faaaa);
Xil_Out32(0xf62401c8, 0xffafaaaa);
Xil_Out32(0xf624000c, 0x00000000);
Xil_Out32(0xf63b000c, 0xf9e8d7c6);
Xil_Out32(0xf63b0000, 0x0000023e);
Xil_Out32(0xf63b0004, 0x00000200);
Xil_Out32(0xf63b03a0, 0x000000c0);
Xil_Out32(0xf63b0100, 0xff0faaaa);
Xil_Out32(0xf63b0108, 0xff0faaaa);
Xil_Out32(0xf63b0118, 0xff0faaaa);
Xil_Out32(0xf63b0120, 0xff0faaaa);
Xil_Out32(0xf63b0128, 0xff0faaaa);
Xil_Out32(0xf63b0130, 0xff0faaaa);
Xil_Out32(0xf63b0138, 0xff0faaaa);
Xil_Out32(0xf63b0150, 0xff0faaaa);
Xil_Out32(0xf63b01c0, 0xff0faaaa);
Xil_Out32(0xf63b01c8, 0xff0faaaa);
Xil_Out32(0xf63b0240, 0xffffaaa5);
Xil_Out32(0xf63b000c, 0x00000000);
Xil_Out32(0xf652400c, 0xf9e8d7c6);
Xil_Out32(0xf6524000, 0x0000023e);
Xil_Out32(0xf6524004, 0x00000200);
Xil_Out32(0xf65243a0, 0x000000c0);
Xil_Out32(0xf6524120, 0xff0faaaa);
Xil_Out32(0xf6524290, 0xffffaaa5);
Xil_Out32(0xf652400c, 0x00000000);
Xil_Out32(0xf652000c, 0xf9e8d7c6);
Xil_Out32(0xf6520000, 0x0000023e);
Xil_Out32(0xf6520004, 0x00000200);
Xil_Out32(0xf65203a0, 0x000000c0);
Xil_Out32(0xf6520100, 0xff0faaaa);
Xil_Out32(0xf6520108, 0xff0faaaa);
Xil_Out32(0xf6520118, 0xff0faaaa);
Xil_Out32(0xf6520130, 0xff0faaaa);
Xil_Out32(0xf6520150, 0xff0faaaa);
Xil_Out32(0xf65201c0, 0xff0faaaa);
Xil_Out32(0xf65201c8, 0xff0faaaa);
Xil_Out32(0xf6520290, 0xffffaaa5);
Xil_Out32(0xf652000c, 0x00000000);
Xil_Out32(0xf620000c, 0xf9e8d7c6);
Xil_Out32(0xf6200000, 0x0000023e);
Xil_Out32(0xf6200004, 0x00000200);
Xil_Out32(0xf62003a0, 0x000000c0);
Xil_Out32(0xf620011c, 0x55050000);
Xil_Out32(0xf620012c, 0x55050000);
Xil_Out32(0xf620015c, 0x55500000);
Xil_Out32(0xf6200168, 0xffffaaaf);
Xil_Out32(0xf6200180, 0xffffaaaf);
Xil_Out32(0xf62001c0, 0xffffaaff);
Xil_Out32(0xf6200240, 0xffffaaaf);
Xil_Out32(0xf620000c, 0x00000000);
Xil_Out32(0xf620200c, 0xf9e8d7c6);
Xil_Out32(0xf6202000, 0x0000023e);
Xil_Out32(0xf6202004, 0x00000200);
Xil_Out32(0xf62023a0, 0x000000c0);
Xil_Out32(0xf620210c, 0x555500f0);
Xil_Out32(0xf620211c, 0x55550050);
Xil_Out32(0xf6202128, 0xffffaa5a);
Xil_Out32(0xf620212c, 0x55550050);
Xil_Out32(0xf620213c, 0x555500f0);
Xil_Out32(0xf620214c, 0x5555000f);
Xil_Out32(0xf620215c, 0x55550005);
Xil_Out32(0xf6202168, 0xfff0aaaa);
Xil_Out32(0xf6202180, 0xfffaaaaa);
Xil_Out32(0xf6202184, 0x555a0000);
Xil_Out32(0xf62021c0, 0xffaaaaaa);
Xil_Out32(0xf62021c4, 0x55aa0000);
Xil_Out32(0xf6202240, 0xfff0aaaa);
Xil_Out32(0xf620200c, 0x00000000);
Xil_Out32(0xf655000c, 0xf9e8d7c6);
Xil_Out32(0xf6550000, 0x0000023e);
Xil_Out32(0xf6550004, 0x00000200);
Xil_Out32(0xf65503a0, 0x000000c0);
Xil_Out32(0xf655012c, 0x55050000);
Xil_Out32(0xf6550290, 0xffffaaaf);
Xil_Out32(0xf655000c, 0x00000000);
Xil_Out32(0xf655200c, 0xf9e8d7c6);
Xil_Out32(0xf6552000, 0x0000023e);
Xil_Out32(0xf6552004, 0x00000200);
Xil_Out32(0xf65523a0, 0x000000c0);
Xil_Out32(0xf6552128, 0xffffaa5a);
Xil_Out32(0xf6552138, 0xffffaafa);
Xil_Out32(0xf6552290, 0xfff0aaaa);
Xil_Out32(0xf6552294, 0x55500000);
Xil_Out32(0xf655200c, 0x00000000);
Xil_Out32(0xf652200c, 0xf9e8d7c6);
Xil_Out32(0xf6522000, 0x0000023e);
Xil_Out32(0xf6522004, 0x00000200);
Xil_Out32(0xf65223a0, 0x000000c0);
Xil_Out32(0xf6522128, 0xffafaaaa);
Xil_Out32(0xf6522138, 0xffafaaaa);
Xil_Out32(0xf6522294, 0x55550005);
Xil_Out32(0xf652200c, 0x00000000);
Xil_Out32(0xf627400c, 0xf9e8d7c6);
Xil_Out32(0xf6274000, 0x0000023e);
Xil_Out32(0xf6274004, 0x00000200);
Xil_Out32(0xf62743a0, 0x000000c0);
Xil_Out32(0xf6274138, 0xff0faaaa);
Xil_Out32(0xf6274168, 0xffffaaa5);
Xil_Out32(0xf627400c, 0x00000000);
Xil_Out32(0xf627200c, 0xf9e8d7c6);
Xil_Out32(0xf6272000, 0x0000023e);
Xil_Out32(0xf6272004, 0x00000200);
Xil_Out32(0xf62723a0, 0x000000c0);
Xil_Out32(0xf6272138, 0xffffaafa);
Xil_Out32(0xf627216c, 0x55500000);
Xil_Out32(0xf627200c, 0x00000000);
Xil_Out32(0xf624200c, 0xf9e8d7c6);
Xil_Out32(0xf6242000, 0x0000023e);
Xil_Out32(0xf6242004, 0x00000200);
Xil_Out32(0xf62423a0, 0x000000c0);
Xil_Out32(0xf6242138, 0xffafaaaa);
Xil_Out32(0xf624216c, 0x55550005);
Xil_Out32(0xf624200c, 0x00000000);
Xil_Out32(0xf632400c, 0xf9e8d7c6);
Xil_Out32(0xf6324000, 0x0000023e);
Xil_Out32(0xf6324004, 0x00000200);
Xil_Out32(0xf63243a0, 0x000000c0);
Xil_Out32(0xf6324138, 0xff0faaaa);
Xil_Out32(0xf6324240, 0xffffaaa5);
Xil_Out32(0xf632400c, 0x00000000);
Xil_Out32(0xf632200c, 0xf9e8d7c6);
Xil_Out32(0xf6322000, 0x0000023e);
Xil_Out32(0xf6322004, 0x00000200);
Xil_Out32(0xf63223a0, 0x000000c0);
Xil_Out32(0xf6322138, 0xffffaafa);
Xil_Out32(0xf6322244, 0x55500000);
Xil_Out32(0xf632200c, 0x00000000);
Xil_Out32(0xf62e200c, 0xf9e8d7c6);
Xil_Out32(0xf62e2000, 0x0000023e);
Xil_Out32(0xf62e2004, 0x00000200);
Xil_Out32(0xf62e23a0, 0x000000c0);
Xil_Out32(0xf62e2120, 0xffffaaa5);
Xil_Out32(0xf62e2138, 0xffafaaaa);
Xil_Out32(0xf62e2168, 0xffffaaa5);
Xil_Out32(0xf62e21c8, 0xff0faaaa);
Xil_Out32(0xf62e2240, 0xffffaaa5);
Xil_Out32(0xf62e2244, 0x55550005);
Xil_Out32(0xf62e2290, 0xffffaaa5);
Xil_Out32(0xf62e200c, 0x00000000);
Xil_Out32(0xf62e000c, 0xf9e8d7c6);
Xil_Out32(0xf62e0000, 0x0000023e);
Xil_Out32(0xf62e0004, 0x00000200);
Xil_Out32(0xf62e03a0, 0x000000c0);
Xil_Out32(0xf62e0124, 0x55500000);
Xil_Out32(0xf62e013c, 0x555500f0);
Xil_Out32(0xf62e016c, 0x55500000);
Xil_Out32(0xf62e01c8, 0xffffaafa);
Xil_Out32(0xf62e01cc, 0x555500f0);
Xil_Out32(0xf62e0244, 0x555a0000);
Xil_Out32(0xf62e0294, 0x555a0000);
Xil_Out32(0xf62e000c, 0x00000000);
Xil_Out32(0xf655400c, 0xf9e8d7c6);
Xil_Out32(0xf6554000, 0x0000023e);
Xil_Out32(0xf6554004, 0x00000200);
Xil_Out32(0xf65543a0, 0x000000c0);
Xil_Out32(0xf6554138, 0xff0faaaa);
Xil_Out32(0xf6554290, 0xffffaaa5);
Xil_Out32(0xf655400c, 0x00000000);
Xil_Out32(0xf6ec600c, 0xf9e8d7c6);
Xil_Out32(0xf6ec6000, 0x0000023e);
Xil_Out32(0xf6ec6004, 0x00000200);
Xil_Out32(0xf6ec63a0, 0x00000180);
Xil_Out32(0xf6ec6104, 0x55050000);
Xil_Out32(0xf6ec610c, 0x55050000);
Xil_Out32(0xf6ec611c, 0x55050000);
Xil_Out32(0xf6ec6124, 0x55050000);
Xil_Out32(0xf6ec612c, 0x55050000);
Xil_Out32(0xf6ec6134, 0x55050000);
Xil_Out32(0xf6ec613c, 0x55050000);
Xil_Out32(0xf6ec6148, 0xfff0aaaa);
Xil_Out32(0xf6ec6154, 0x55050000);
Xil_Out32(0xf6ec6158, 0xfff0aaaa);
Xil_Out32(0xf6ec6180, 0xfffaaaaa);
Xil_Out32(0xf6ec6324, 0x55050000);
Xil_Out32(0xf6ec632c, 0x55050000);
Xil_Out32(0xf6ec6330, 0xffffaa5a);
Xil_Out32(0xf6ec6334, 0x55550050);
Xil_Out32(0xf6ec6338, 0xffffaa5a);
Xil_Out32(0xf6ec633c, 0x55a50000);
Xil_Out32(0xf6ec634c, 0x55a50000);
Xil_Out32(0xf6ec6360, 0xfff0aaaa);
Xil_Out32(0xf6ec6368, 0xfff0aaaa);
Xil_Out32(0xf6ec6370, 0xffffaaaf);
Xil_Out32(0xf6ec6374, 0x5555000f);
Xil_Out32(0xf6ec6378, 0xfffaaaaa);
Xil_Out32(0xf6ec600c, 0x00000000);
Xil_Out32(0xf61b000c, 0xf9e8d7c6);
Xil_Out32(0xf61b0000, 0x0000023e);
Xil_Out32(0xf61b0004, 0x00000200);
Xil_Out32(0xf61b03a0, 0x000000b0);
Xil_Out32(0xf61b01c0, 0xffafaaaa);
Xil_Out32(0xf61b0324, 0x55550005);
Xil_Out32(0xf61b000c, 0x00000000);
Xil_Out32(0xf6ee400c, 0xf9e8d7c6);
Xil_Out32(0xf6ee4000, 0x0000023e);
Xil_Out32(0xf6ee4004, 0x00000200);
Xil_Out32(0xf6ee43a0, 0x00000180);
Xil_Out32(0xf6ee414c, 0x555a0000);
Xil_Out32(0xf6ee4338, 0xffffaafa);
Xil_Out32(0xf6ee433c, 0x555500f0);
Xil_Out32(0xf6ee4364, 0x555a0000);
Xil_Out32(0xf6ee436c, 0x555a0000);
Xil_Out32(0xf6ee438c, 0x55500000);
Xil_Out32(0xf6ee400c, 0x00000000);
Xil_Out32(0xf6ec400c, 0xf9e8d7c6);
Xil_Out32(0xf6ec4000, 0x0000023e);
Xil_Out32(0xf6ec4004, 0x00000200);
Xil_Out32(0xf6ec43a0, 0x00000180);
Xil_Out32(0xf6ec4148, 0xffffaaa5);
Xil_Out32(0xf6ec4330, 0xffffaafa);
Xil_Out32(0xf6ec4334, 0x555500f0);
Xil_Out32(0xf6ec4338, 0xff0faaaa);
Xil_Out32(0xf6ec4368, 0xffffaaa5);
Xil_Out32(0xf6ec436c, 0x555a0000);
Xil_Out32(0xf6ec438c, 0x55500000);
Xil_Out32(0xf6ec400c, 0x00000000);
Xil_Out32(0xf620400c, 0xf9e8d7c6);
Xil_Out32(0xf6204000, 0x0000023e);
Xil_Out32(0xf6204004, 0x00000200);
Xil_Out32(0xf62043a0, 0x000000c0);
Xil_Out32(0xf6204108, 0xff0faaaa);
Xil_Out32(0xf6204138, 0xff0faaaa);
Xil_Out32(0xf6204148, 0xfff0aaaa);
Xil_Out32(0xf6204180, 0xffffaaa5);
Xil_Out32(0xf62041c0, 0xffffaa55);
Xil_Out32(0xf620400c, 0x00000000);
Xil_Out32(0xf61b200c, 0xf9e8d7c6);
Xil_Out32(0xf61b2000, 0x0000023e);
Xil_Out32(0xf61b2004, 0x00000200);
Xil_Out32(0xf61b23a0, 0x00000090);
Xil_Out32(0xf61b21c4, 0x55a50000);
Xil_Out32(0xf61b2324, 0x5555000f);
Xil_Out32(0xf61b200c, 0x00000000);
Xil_Out32(0xf6f3400c, 0xf9e8d7c6);
Xil_Out32(0xf6f34000, 0x0000023e);
Xil_Out32(0xf6f34004, 0x00000200);
Xil_Out32(0xf6f343a0, 0x00000180);
Xil_Out32(0xf6f34150, 0xffafaaaa);
Xil_Out32(0xf6f34154, 0x555a0000);
Xil_Out32(0xf6f34330, 0xffafaaaa);
Xil_Out32(0xf6f3434c, 0x555500f0);
Xil_Out32(0xf6f34364, 0x555a0000);
Xil_Out32(0xf6f3438c, 0x55550005);
Xil_Out32(0xf6f3400c, 0x00000000);
Xil_Out32(0xf6e9600c, 0xf9e8d7c6);
Xil_Out32(0xf6e96000, 0x0000023e);
Xil_Out32(0xf6e96004, 0x00000200);
Xil_Out32(0xf6e963a0, 0x00000180);
Xil_Out32(0xf6e96104, 0x55550050);
Xil_Out32(0xf6e9610c, 0x55050000);
Xil_Out32(0xf6e9611c, 0x55050000);
Xil_Out32(0xf6e96124, 0x55550050);
Xil_Out32(0xf6e9612c, 0x55050000);
Xil_Out32(0xf6e96134, 0x55550050);
Xil_Out32(0xf6e9613c, 0x55050000);
Xil_Out32(0xf6e96154, 0x55550050);
Xil_Out32(0xf6e96180, 0xfffaaaaa);
Xil_Out32(0xf6e9632c, 0x55550050);
Xil_Out32(0xf6e9633c, 0x55a50000);
Xil_Out32(0xf6e9634c, 0x55a50000);
Xil_Out32(0xf6e96368, 0xffffaaaf);
Xil_Out32(0xf6e9636c, 0x5555000f);
Xil_Out32(0xf6e96370, 0xfffaaaaa);
Xil_Out32(0xf6e96378, 0xfffaaaaa);
Xil_Out32(0xf6e96388, 0xfffaaaaa);
Xil_Out32(0xf6e9600c, 0x00000000);
Xil_Out32(0xf6e6600c, 0xf9e8d7c6);
Xil_Out32(0xf6e66000, 0x0000023e);
Xil_Out32(0xf6e66004, 0x00000200);
Xil_Out32(0xf6e663a0, 0x00000180);
Xil_Out32(0xf6e66104, 0x55050000);
Xil_Out32(0xf6e6610c, 0x55050000);
Xil_Out32(0xf6e6611c, 0x55050000);
Xil_Out32(0xf6e66124, 0x55050000);
Xil_Out32(0xf6e6612c, 0x55050000);
Xil_Out32(0xf6e66134, 0x55050000);
Xil_Out32(0xf6e6613c, 0x55050000);
Xil_Out32(0xf6e66154, 0x55050000);
Xil_Out32(0xf6e66324, 0x55550050);
Xil_Out32(0xf6e66334, 0x55a50000);
Xil_Out32(0xf6e66360, 0xffffaaaf);
Xil_Out32(0xf6e66364, 0x5555000f);
Xil_Out32(0xf6e66370, 0xfffaaaaa);
Xil_Out32(0xf6e66378, 0xfffaaaaa);
Xil_Out32(0xf6e66388, 0xfffaaaaa);
Xil_Out32(0xf6e6600c, 0x00000000);
Xil_Out32(0xf6ee600c, 0xf9e8d7c6);
Xil_Out32(0xf6ee6000, 0x0000023e);
Xil_Out32(0xf6ee6004, 0x00000200);
Xil_Out32(0xf6ee63a0, 0x00000180);
Xil_Out32(0xf6ee6104, 0x55050000);
Xil_Out32(0xf6ee610c, 0x55050000);
Xil_Out32(0xf6ee611c, 0x55050000);
Xil_Out32(0xf6ee6124, 0x55050000);
Xil_Out32(0xf6ee612c, 0x55050000);
Xil_Out32(0xf6ee6134, 0x55050000);
Xil_Out32(0xf6ee613c, 0x55050000);
Xil_Out32(0xf6ee6154, 0x55050000);
Xil_Out32(0xf6ee6324, 0x55050000);
Xil_Out32(0xf6ee632c, 0x55050000);
Xil_Out32(0xf6ee6334, 0x55050000);
Xil_Out32(0xf6ee6338, 0xffffaa5a);
Xil_Out32(0xf6ee634c, 0x55a50000);
Xil_Out32(0xf6ee6370, 0xfff0aaaa);
Xil_Out32(0xf6ee6378, 0xffffaaaf);
Xil_Out32(0xf6ee637c, 0x5555000f);
Xil_Out32(0xf6ee600c, 0x00000000);
Xil_Out32(0xf6f3600c, 0xf9e8d7c6);
Xil_Out32(0xf6f36000, 0x0000023e);
Xil_Out32(0xf6f36004, 0x00000200);
Xil_Out32(0xf6f363a0, 0x00000180);
Xil_Out32(0xf6f36104, 0x55050000);
Xil_Out32(0xf6f3610c, 0x55050000);
Xil_Out32(0xf6f3611c, 0x55050000);
Xil_Out32(0xf6f36124, 0x55050000);
Xil_Out32(0xf6f3612c, 0x55050000);
Xil_Out32(0xf6f36134, 0x55050000);
Xil_Out32(0xf6f3613c, 0x55050000);
Xil_Out32(0xf6f36324, 0x55050000);
Xil_Out32(0xf6f3632c, 0x55050000);
Xil_Out32(0xf6f3633c, 0x55050000);
Xil_Out32(0xf6f36348, 0xffffaa5a);
Xil_Out32(0xf6f36368, 0xfff0aaaa);
Xil_Out32(0xf6f36370, 0xfff0aaaa);
Xil_Out32(0xf6f36378, 0xfff0aaaa);
Xil_Out32(0xf6f36388, 0xffffaaaf);
Xil_Out32(0xf6f3600c, 0x00000000);
Xil_Out32(0xf6f1400c, 0xf9e8d7c6);
Xil_Out32(0xf6f14000, 0x0000023e);
Xil_Out32(0xf6f14004, 0x00000200);
Xil_Out32(0xf6f143a0, 0x00000180);
Xil_Out32(0xf6f14338, 0xffafaaaa);
Xil_Out32(0xf6f1438c, 0x55550005);
Xil_Out32(0xf6f1400c, 0x00000000);
Xil_Out32(0xf6f1600c, 0xf9e8d7c6);
Xil_Out32(0xf6f16000, 0x0000023e);
Xil_Out32(0xf6f16004, 0x00000200);
Xil_Out32(0xf6f163a0, 0x00000180);
Xil_Out32(0xf6f1633c, 0x55550050);
Xil_Out32(0xf6f16388, 0xfffaaaaa);
Xil_Out32(0xf6f1600c, 0x00000000);
Xil_Out32(0xf6c9400c, 0xf9e8d7c6);
Xil_Out32(0xf6c94000, 0x0000023e);
Xil_Out32(0xf6c94004, 0x00000200);
Xil_Out32(0xf6c943a0, 0x00000190);
Xil_Out32(0xf6c94124, 0x555a0000);
Xil_Out32(0xf6c9416c, 0x555a0000);
Xil_Out32(0xf6c94194, 0x55500000);
Xil_Out32(0xf6c9419c, 0x55500000);
Xil_Out32(0xf6c94244, 0x555a0000);
Xil_Out32(0xf6c94294, 0x555a0000);
Xil_Out32(0xf6c94320, 0xffffaafa);
Xil_Out32(0xf6c94324, 0x555500f0);
Xil_Out32(0xf6c9400c, 0x00000000);
Xil_Out32(0xf6d4400c, 0xf9e8d7c6);
Xil_Out32(0xf6d44000, 0x0000023e);
Xil_Out32(0xf6d44004, 0x00000200);
Xil_Out32(0xf6d443a0, 0x00000190);
Xil_Out32(0xf6d44194, 0x55550005);
Xil_Out32(0xf6d44320, 0xffafaaaa);
Xil_Out32(0xf6d4400c, 0x00000000);
Xil_Out32(0xf6d4600c, 0xf9e8d7c6);
Xil_Out32(0xf6d46000, 0x0000023e);
Xil_Out32(0xf6d46004, 0x00000200);
Xil_Out32(0xf6d463a0, 0x00000190);
Xil_Out32(0xf6d46190, 0xfffaaaaa);
Xil_Out32(0xf6d46324, 0x55550050);
Xil_Out32(0xf6d4600c, 0x00000000);
Xil_Out32(0xf6cc400c, 0xf9e8d7c6);
Xil_Out32(0xf6cc4000, 0x0000023e);
Xil_Out32(0xf6cc4004, 0x00000200);
Xil_Out32(0xf6cc43a0, 0x00000190);
Xil_Out32(0xf6cc4124, 0x555a0000);
Xil_Out32(0xf6cc416c, 0x555a0000);
Xil_Out32(0xf6cc41a4, 0x55500000);
Xil_Out32(0xf6cc4244, 0x555a0000);
Xil_Out32(0xf6cc4294, 0x555a0000);
Xil_Out32(0xf6cc4328, 0xffffaafa);
Xil_Out32(0xf6cc432c, 0x555500f0);
Xil_Out32(0xf6cc400c, 0x00000000);
Xil_Out32(0xf6cc600c, 0xf9e8d7c6);
Xil_Out32(0xf6cc6000, 0x0000023e);
Xil_Out32(0xf6cc6004, 0x00000200);
Xil_Out32(0xf6cc63a0, 0x00000190);
Xil_Out32(0xf6cc61a8, 0xfffaaaaa);
Xil_Out32(0xf6cc632c, 0x55550050);
Xil_Out32(0xf6cc600c, 0x00000000);
Xil_Out32(0xf60e000c, 0xf9e8d7c6);
Xil_Out32(0xf60e0000, 0x01038000);
Xil_Out32(0xf60e0004, 0x01038000);
Xil_Out32(0xf60e02c8, 0x000095e0);
Xil_Out32(0xf60e086c, 0x00000003);
Xil_Out32(0xf60e0864, 0x00000001);
Xil_Out32(0xf60e0868, 0x00000001);
Xil_Out32(0xf60e0858, 0x00000002);
Xil_Out32(0xf60e0860, 0x00000002);
Xil_Out32(0xf60e083c, 0x00000100);
Xil_Out32(0xf60e0844, 0x00000100);
Xil_Out32(0xf60e0840, 0x00001fff);
Xil_Out32(0xf60e0848, 0x00001fff);
Xil_Out32(0xf60e03f0, 0x00004000);
Xil_Out32(0xf60e03f4, 0x00001000);
Xil_Out32(0xf60e03f8, 0x00005000);
Xil_Out32(0xf60e03fc, 0x00003000);
Xil_Out32(0xf60e0400, 0x00003000);
Xil_Out32(0xf60e0404, 0x00004000);
Xil_Out32(0xf60e0408, 0x00002000);
Xil_Out32(0xf60e03d0, 0x00000101);
Xil_Out32(0xf60e03d4, 0x00000181);
Xil_Out32(0xf60e03d8, 0x000001c1);
Xil_Out32(0xf60e03dc, 0x00000141);
Xil_Out32(0xf60e03e0, 0x00000341);
Xil_Out32(0xf60e03e4, 0x00000181);
Xil_Out32(0xf60e03e8, 0x00000a01);
Xil_Out32(0xf60e03ec, 0x00000c81);
Xil_Out32(0xf60e0440, 0x00000409);
Xil_Out32(0xf60e0444, 0x00000f54);
Xil_Out32(0xf60e0448, 0x00000da8);
Xil_Out32(0xf60e044c, 0x0000088f);
Xil_Out32(0xf60e0450, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000080);
Xil_Out32(0xf60e02c4, 0x00000000);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000002);
Xil_Out32(0xf60e02c4, 0x00000000);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000040);
Xil_Out32(0xf60e02c4, 0x00000000);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x0000020c);
Xil_Out32(0xf60e0004, 0x00000200);
Xil_Out32(0xf60e000c, 0x00000000);
Xil_Out32(0xf60e200c, 0xf9e8d7c6);
Xil_Out32(0xf60e2000, 0x01038000);
Xil_Out32(0xf60e2004, 0x00030000);
Xil_Out32(0xf60e22c8, 0x0000aeb8);
Xil_Out32(0xf60e286c, 0x00000003);
Xil_Out32(0xf60e2864, 0x00000001);
Xil_Out32(0xf60e2868, 0x00000001);
Xil_Out32(0xf60e2858, 0x00000002);
Xil_Out32(0xf60e2860, 0x00000002);
Xil_Out32(0xf60e283c, 0x00000100);
Xil_Out32(0xf60e2844, 0x00000100);
Xil_Out32(0xf60e2840, 0x00001fff);
Xil_Out32(0xf60e2848, 0x00001fff);
Xil_Out32(0xf60e23f0, 0x00004000);
Xil_Out32(0xf60e23f4, 0x00004000);
Xil_Out32(0xf60e23f8, 0x00005000);
Xil_Out32(0xf60e23fc, 0x00001000);
Xil_Out32(0xf60e2400, 0x00001000);
Xil_Out32(0xf60e2404, 0x00004000);
Xil_Out32(0xf60e2408, 0x00001000);
Xil_Out32(0xf60e23d0, 0x00000101);
Xil_Out32(0xf60e23d4, 0x00000181);
Xil_Out32(0xf60e23d8, 0x000001c1);
Xil_Out32(0xf60e23dc, 0x00000141);
Xil_Out32(0xf60e23e0, 0x00000341);
Xil_Out32(0xf60e23e4, 0x00000181);
Xil_Out32(0xf60e23e8, 0x00000a01);
Xil_Out32(0xf60e23ec, 0x00000c81);
Xil_Out32(0xf60e2440, 0x00000409);
Xil_Out32(0xf60e2444, 0x0000025d);
Xil_Out32(0xf60e2448, 0x00000c62);
Xil_Out32(0xf60e244c, 0x000006ff);
Xil_Out32(0xf60e2450, 0x00000280);
Xil_Out32(0xf60e2000, 0x00000080);
Xil_Out32(0xf60e22c4, 0x00000000);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x00000002);
Xil_Out32(0xf60e22c4, 0x00000000);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x00000040);
Xil_Out32(0xf60e22c4, 0x00000000);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x0000020c);
Xil_Out32(0xf60e2004, 0x00000200);
Xil_Out32(0xf60e200c, 0x00000000);
Xil_Out32(0xf601200c, 0xf9e8d7c6);
Xil_Out32(0xf6012000, 0x01038000);
Xil_Out32(0xf6012004, 0x01038000);
Xil_Out32(0xf60122c8, 0x0000e2f0);
Xil_Out32(0xf601286c, 0x00000003);
Xil_Out32(0xf6012864, 0x00000001);
Xil_Out32(0xf6012868, 0x00000001);
Xil_Out32(0xf6012858, 0x00000002);
Xil_Out32(0xf6012860, 0x00000002);
Xil_Out32(0xf601283c, 0x00000100);
Xil_Out32(0xf6012844, 0x00000100);
Xil_Out32(0xf6012840, 0x00001fff);
Xil_Out32(0xf6012848, 0x00001fff);
Xil_Out32(0xf60123f0, 0x00004000);
Xil_Out32(0xf60123f4, 0x00006000);
Xil_Out32(0xf60123f8, 0x00004000);
Xil_Out32(0xf60123fc, 0x00003000);
Xil_Out32(0xf6012400, 0x00003000);
Xil_Out32(0xf6012404, 0x00001000);
Xil_Out32(0xf6012408, 0x00003000);
Xil_Out32(0xf60123d0, 0x00000101);
Xil_Out32(0xf60123d4, 0x00000181);
Xil_Out32(0xf60123d8, 0x000001c1);
Xil_Out32(0xf60123dc, 0x00000141);
Xil_Out32(0xf60123e0, 0x00000341);
Xil_Out32(0xf60123e4, 0x00000181);
Xil_Out32(0xf60123e8, 0x00000a01);
Xil_Out32(0xf60123ec, 0x00000c81);
Xil_Out32(0xf6012414, 0x00000081);
Xil_Out32(0xf6012418, 0x00000081);
Xil_Out32(0xf6012428, 0x00000f59);
Xil_Out32(0xf601242c, 0x00000947);
Xil_Out32(0xf6012440, 0x00000409);
Xil_Out32(0xf6012444, 0x00000ba0);
Xil_Out32(0xf6012448, 0x00000c59);
Xil_Out32(0xf601244c, 0x00000fbe);
Xil_Out32(0xf6012450, 0x000000c1);
Xil_Out32(0xf6012000, 0x00000080);
Xil_Out32(0xf60122c4, 0x00000000);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x00000002);
Xil_Out32(0xf60122c4, 0x00000000);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x00000040);
Xil_Out32(0xf60122c4, 0x00000000);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x0000020c);
Xil_Out32(0xf6012004, 0x00000200);
Xil_Out32(0xf601200c, 0x00000000);
Xil_Out32(0xf601600c, 0xf9e8d7c6);
Xil_Out32(0xf6016000, 0x01038000);
Xil_Out32(0xf6016004, 0x01010000);
Xil_Out32(0xf60162c8, 0x00004ef8);
Xil_Out32(0xf601686c, 0x00000003);
Xil_Out32(0xf6016864, 0x00000001);
Xil_Out32(0xf6016868, 0x00000001);
Xil_Out32(0xf6016858, 0x00000002);
Xil_Out32(0xf6016860, 0x00000002);
Xil_Out32(0xf601683c, 0x00000100);
Xil_Out32(0xf6016844, 0x00000100);
Xil_Out32(0xf6016840, 0x00001fff);
Xil_Out32(0xf6016848, 0x00001fff);
Xil_Out32(0xf60163f0, 0x00001000);
Xil_Out32(0xf60163f4, 0x00006000);
Xil_Out32(0xf60163f8, 0x00003000);
Xil_Out32(0xf60163fc, 0x00006000);
Xil_Out32(0xf6016400, 0x00006000);
Xil_Out32(0xf6016404, 0x00004000);
Xil_Out32(0xf6016408, 0x00002000);
Xil_Out32(0xf60163d0, 0x00000101);
Xil_Out32(0xf60163d4, 0x00000181);
Xil_Out32(0xf60163d8, 0x000001c1);
Xil_Out32(0xf60163dc, 0x00000141);
Xil_Out32(0xf60163e0, 0x00000341);
Xil_Out32(0xf60163e4, 0x00000181);
Xil_Out32(0xf60163e8, 0x00000a01);
Xil_Out32(0xf60163ec, 0x00000c81);
Xil_Out32(0xf6016414, 0x00000081);
Xil_Out32(0xf6016418, 0x00000081);
Xil_Out32(0xf6016428, 0x0000053e);
Xil_Out32(0xf601642c, 0x00000722);
Xil_Out32(0xf6016440, 0x00000409);
Xil_Out32(0xf6016444, 0x00000de3);
Xil_Out32(0xf6016448, 0x000009a8);
Xil_Out32(0xf601644c, 0x0000038f);
Xil_Out32(0xf6016450, 0x00000041);
Xil_Out32(0xf6016000, 0x00000080);
Xil_Out32(0xf60162c4, 0x00000000);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x00000002);
Xil_Out32(0xf60162c4, 0x00000000);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x00000040);
Xil_Out32(0xf60162c4, 0x00000000);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x0000020c);
Xil_Out32(0xf6016004, 0x00000200);
Xil_Out32(0xf601600c, 0x00000000);
Xil_Out32(0xf601000c, 0xf9e8d7c6);
Xil_Out32(0xf6010000, 0x01038000);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf60102c8, 0x0000f380);
Xil_Out32(0xf601086c, 0x00000003);
Xil_Out32(0xf6010864, 0x00000001);
Xil_Out32(0xf6010868, 0x00000001);
Xil_Out32(0xf6010858, 0x00000002);
Xil_Out32(0xf6010860, 0x00000002);
Xil_Out32(0xf601083c, 0x00000100);
Xil_Out32(0xf6010844, 0x00000100);
Xil_Out32(0xf6010840, 0x00001fff);
Xil_Out32(0xf6010848, 0x00001fff);
Xil_Out32(0xf60103f0, 0x00005000);
Xil_Out32(0xf60103f4, 0x00003000);
Xil_Out32(0xf60103f8, 0x00001000);
Xil_Out32(0xf60103fc, 0x00006000);
Xil_Out32(0xf6010400, 0x00004000);
Xil_Out32(0xf6010404, 0x00002000);
Xil_Out32(0xf6010408, 0x00004000);
Xil_Out32(0xf60103d0, 0x00000101);
Xil_Out32(0xf60103d4, 0x00000181);
Xil_Out32(0xf60103d8, 0x000001c1);
Xil_Out32(0xf60103dc, 0x00000141);
Xil_Out32(0xf60103e0, 0x00000341);
Xil_Out32(0xf60103e4, 0x00000181);
Xil_Out32(0xf60103e8, 0x00000a01);
Xil_Out32(0xf60103ec, 0x00000c81);
Xil_Out32(0xf601040c, 0x00000281);
Xil_Out32(0xf6010418, 0x000001c6);
Xil_Out32(0xf6010428, 0x00000281);
Xil_Out32(0xf601042c, 0x00000999);
Xil_Out32(0xf601041c, 0x00000281);
Xil_Out32(0xf6010420, 0x00000514);
Xil_Out32(0xf6010424, 0x000006dd);
Xil_Out32(0xf6010440, 0x00000408);
Xil_Out32(0xf6010444, 0x0000059d);
Xil_Out32(0xf6010448, 0x00000341);
Xil_Out32(0xf601044c, 0x0000048e);
Xil_Out32(0xf6010450, 0x000000c0);
Xil_Out32(0xf6010000, 0x00000080);
Xil_Out32(0xf60102c4, 0x00000000);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x00000002);
Xil_Out32(0xf60102c4, 0x00000000);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x00000040);
Xil_Out32(0xf60102c4, 0x00000000);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x0000020c);
Xil_Out32(0xf6010004, 0x00000200);
Xil_Out32(0xf601000c, 0x00000000);
Xil_Out32(0xf601400c, 0xf9e8d7c6);
Xil_Out32(0xf6014000, 0x01038000);
Xil_Out32(0xf6014004, 0x00010000);
Xil_Out32(0xf60142c8, 0x00006b08);
Xil_Out32(0xf601486c, 0x00000003);
Xil_Out32(0xf6014864, 0x00000001);
Xil_Out32(0xf6014868, 0x00000001);
Xil_Out32(0xf6014858, 0x00000002);
Xil_Out32(0xf6014860, 0x00000002);
Xil_Out32(0xf601483c, 0x00000100);
Xil_Out32(0xf6014844, 0x00000100);
Xil_Out32(0xf6014840, 0x00001fff);
Xil_Out32(0xf6014848, 0x00001fff);
Xil_Out32(0xf60143f0, 0x00002000);
Xil_Out32(0xf60143f4, 0x00006000);
Xil_Out32(0xf60143f8, 0x00006000);
Xil_Out32(0xf60143fc, 0x00004000);
Xil_Out32(0xf6014400, 0x00006000);
Xil_Out32(0xf6014404, 0x00003000);
Xil_Out32(0xf6014408, 0x00005000);
Xil_Out32(0xf60143d0, 0x00000101);
Xil_Out32(0xf60143d4, 0x00000181);
Xil_Out32(0xf60143d8, 0x000001c1);
Xil_Out32(0xf60143dc, 0x00000141);
Xil_Out32(0xf60143e0, 0x00000341);
Xil_Out32(0xf60143e4, 0x00000181);
Xil_Out32(0xf60143e8, 0x00000a01);
Xil_Out32(0xf60143ec, 0x00000c81);
Xil_Out32(0xf601440c, 0x00000080);
Xil_Out32(0xf6014418, 0x00000fe7);
Xil_Out32(0xf6014428, 0x00000080);
Xil_Out32(0xf601442c, 0x000003ed);
Xil_Out32(0xf601441c, 0x00000080);
Xil_Out32(0xf6014420, 0x000009d6);
Xil_Out32(0xf6014424, 0x00000e23);
Xil_Out32(0xf6014440, 0x00000409);
Xil_Out32(0xf6014444, 0x000006e0);
Xil_Out32(0xf6014448, 0x00000978);
Xil_Out32(0xf601444c, 0x0000043d);
Xil_Out32(0xf6014450, 0x00000040);
Xil_Out32(0xf6014000, 0x00000080);
Xil_Out32(0xf60142c4, 0x00000000);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x00000002);
Xil_Out32(0xf60142c4, 0x00000000);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x00000040);
Xil_Out32(0xf60142c4, 0x00000000);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x0000020c);
Xil_Out32(0xf6014004, 0x00000200);
Xil_Out32(0xf601400c, 0x00000000);
Xil_Out32(0xf60f000c, 0xf9e8d7c6);
Xil_Out32(0xf60f0000, 0x01038000);
Xil_Out32(0xf60f0004, 0x01020000);
Xil_Out32(0xf60f02c8, 0x00000a28);
Xil_Out32(0xf60f086c, 0x00000003);
Xil_Out32(0xf60f0864, 0x00000001);
Xil_Out32(0xf60f0868, 0x00000001);
Xil_Out32(0xf60f0858, 0x00000002);
Xil_Out32(0xf60f0860, 0x00000002);
Xil_Out32(0xf60f083c, 0x00000100);
Xil_Out32(0xf60f0844, 0x00000100);
Xil_Out32(0xf60f0840, 0x00001fff);
Xil_Out32(0xf60f0848, 0x00001fff);
Xil_Out32(0xf60f03f0, 0x00004000);
Xil_Out32(0xf60f03f4, 0x00005000);
Xil_Out32(0xf60f03f8, 0x00001000);
Xil_Out32(0xf60f03fc, 0x00003000);
Xil_Out32(0xf60f0400, 0x00001000);
Xil_Out32(0xf60f0404, 0x00002000);
Xil_Out32(0xf60f0408, 0x00002000);
Xil_Out32(0xf60f03d0, 0x00000101);
Xil_Out32(0xf60f03d4, 0x00000181);
Xil_Out32(0xf60f03d8, 0x000001c1);
Xil_Out32(0xf60f03dc, 0x00000141);
Xil_Out32(0xf60f03e0, 0x00000341);
Xil_Out32(0xf60f03e4, 0x00000181);
Xil_Out32(0xf60f03e8, 0x00000a01);
Xil_Out32(0xf60f03ec, 0x00000c81);
Xil_Out32(0xf60f0440, 0x00000408);
Xil_Out32(0xf60f0444, 0x000008c1);
Xil_Out32(0xf60f0448, 0x000005c5);
Xil_Out32(0xf60f044c, 0x0000019d);
Xil_Out32(0xf60f0450, 0x00000140);
Xil_Out32(0xf60f0000, 0x00000080);
Xil_Out32(0xf60f02c4, 0x00000000);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x00000002);
Xil_Out32(0xf60f02c4, 0x00000000);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x00000040);
Xil_Out32(0xf60f02c4, 0x00000000);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x0000020c);
Xil_Out32(0xf60f0004, 0x00000200);
Xil_Out32(0xf60f000c, 0x00000000);
Xil_Out32(0xf60f200c, 0xf9e8d7c6);
Xil_Out32(0xf60f2000, 0x01038000);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f22c8, 0x00007460);
Xil_Out32(0xf60f286c, 0x00000003);
Xil_Out32(0xf60f2864, 0x00000001);
Xil_Out32(0xf60f2868, 0x00000001);
Xil_Out32(0xf60f2858, 0x00000002);
Xil_Out32(0xf60f2860, 0x00000002);
Xil_Out32(0xf60f283c, 0x00000100);
Xil_Out32(0xf60f2844, 0x00000100);
Xil_Out32(0xf60f2840, 0x00001fff);
Xil_Out32(0xf60f2848, 0x00001fff);
Xil_Out32(0xf60f23f0, 0x00003000);
Xil_Out32(0xf60f23f4, 0x00002000);
Xil_Out32(0xf60f23f8, 0x00006000);
Xil_Out32(0xf60f23fc, 0x00003000);
Xil_Out32(0xf60f2400, 0x00002000);
Xil_Out32(0xf60f2404, 0x00001000);
Xil_Out32(0xf60f2408, 0x00002000);
Xil_Out32(0xf60f23d0, 0x00000101);
Xil_Out32(0xf60f23d4, 0x00000181);
Xil_Out32(0xf60f23d8, 0x000001c1);
Xil_Out32(0xf60f23dc, 0x00000141);
Xil_Out32(0xf60f23e0, 0x00000341);
Xil_Out32(0xf60f23e4, 0x00000181);
Xil_Out32(0xf60f23e8, 0x00000a01);
Xil_Out32(0xf60f23ec, 0x00000c81);
Xil_Out32(0xf60f2440, 0x00000408);
Xil_Out32(0xf60f2444, 0x00000ba2);
Xil_Out32(0xf60f2448, 0x00000b2c);
Xil_Out32(0xf60f244c, 0x00000e69);
Xil_Out32(0xf60f2450, 0x000001c0);
Xil_Out32(0xf60f2000, 0x00000080);
Xil_Out32(0xf60f22c4, 0x00000000);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x00000002);
Xil_Out32(0xf60f22c4, 0x00000000);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x00000040);
Xil_Out32(0xf60f22c4, 0x00000000);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x0000020c);
Xil_Out32(0xf60f2004, 0x00000200);
Xil_Out32(0xf60f200c, 0x00000000);
Xil_Out32(0xf60f400c, 0xf9e8d7c6);
Xil_Out32(0xf60f4000, 0x01038000);
Xil_Out32(0xf60f4004, 0x00030000);
Xil_Out32(0xf60f42c8, 0x00002498);
Xil_Out32(0xf60f486c, 0x00000003);
Xil_Out32(0xf60f4864, 0x00000001);
Xil_Out32(0xf60f4868, 0x00000001);
Xil_Out32(0xf60f4858, 0x00000002);
Xil_Out32(0xf60f4860, 0x00000002);
Xil_Out32(0xf60f483c, 0x00000100);
Xil_Out32(0xf60f4844, 0x00000100);
Xil_Out32(0xf60f4840, 0x00001fff);
Xil_Out32(0xf60f4848, 0x00001fff);
Xil_Out32(0xf60f43f0, 0x00005000);
Xil_Out32(0xf60f43f4, 0x00001000);
Xil_Out32(0xf60f43f8, 0x00002000);
Xil_Out32(0xf60f43fc, 0x00003000);
Xil_Out32(0xf60f4400, 0x00001000);
Xil_Out32(0xf60f4404, 0x00004000);
Xil_Out32(0xf60f4408, 0x00002000);
Xil_Out32(0xf60f43d0, 0x00000101);
Xil_Out32(0xf60f43d4, 0x00000181);
Xil_Out32(0xf60f43d8, 0x000001c1);
Xil_Out32(0xf60f43dc, 0x00000141);
Xil_Out32(0xf60f43e0, 0x00000341);
Xil_Out32(0xf60f43e4, 0x00000181);
Xil_Out32(0xf60f43e8, 0x00000a01);
Xil_Out32(0xf60f43ec, 0x00000c81);
Xil_Out32(0xf60f4440, 0x00000408);
Xil_Out32(0xf60f4444, 0x0000077b);
Xil_Out32(0xf60f4448, 0x00000f93);
Xil_Out32(0xf60f444c, 0x000001dd);
Xil_Out32(0xf60f4450, 0x00000180);
Xil_Out32(0xf60f4000, 0x00000080);
Xil_Out32(0xf60f42c4, 0x00000000);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x00000002);
Xil_Out32(0xf60f42c4, 0x00000000);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x00000040);
Xil_Out32(0xf60f42c4, 0x00000000);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x0000020c);
Xil_Out32(0xf60f4004, 0x00000200);
Xil_Out32(0xf60f400c, 0x00000000);
Xil_Out32(0xf60f600c, 0xf9e8d7c6);
Xil_Out32(0xf60f6000, 0x01038000);
Xil_Out32(0xf60f6004, 0x00030000);
Xil_Out32(0xf60f62c8, 0x00009690);
Xil_Out32(0xf60f686c, 0x00000003);
Xil_Out32(0xf60f6864, 0x00000001);
Xil_Out32(0xf60f6868, 0x00000001);
Xil_Out32(0xf60f6858, 0x00000002);
Xil_Out32(0xf60f6860, 0x00000002);
Xil_Out32(0xf60f683c, 0x00000100);
Xil_Out32(0xf60f6844, 0x00000100);
Xil_Out32(0xf60f6840, 0x00001fff);
Xil_Out32(0xf60f6848, 0x00001fff);
Xil_Out32(0xf60f63f0, 0x00001000);
Xil_Out32(0xf60f63f4, 0x00004000);
Xil_Out32(0xf60f63f8, 0x00001000);
Xil_Out32(0xf60f63fc, 0x00002000);
Xil_Out32(0xf60f6400, 0x00003000);
Xil_Out32(0xf60f6404, 0x00006000);
Xil_Out32(0xf60f6408, 0x00004000);
Xil_Out32(0xf60f63d0, 0x00000101);
Xil_Out32(0xf60f63d4, 0x00000181);
Xil_Out32(0xf60f63d8, 0x000001c1);
Xil_Out32(0xf60f63dc, 0x00000141);
Xil_Out32(0xf60f63e0, 0x00000341);
Xil_Out32(0xf60f63e4, 0x00000181);
Xil_Out32(0xf60f63e8, 0x00000a01);
Xil_Out32(0xf60f63ec, 0x00000c81);
Xil_Out32(0xf60f6440, 0x00000408);
Xil_Out32(0xf60f6444, 0x000000d3);
Xil_Out32(0xf60f6448, 0x00000676);
Xil_Out32(0xf60f644c, 0x0000010a);
Xil_Out32(0xf60f6450, 0x00000100);
Xil_Out32(0xf60f6000, 0x00000080);
Xil_Out32(0xf60f62c4, 0x00000000);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x00000002);
Xil_Out32(0xf60f62c4, 0x00000000);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x00000040);
Xil_Out32(0xf60f62c4, 0x00000000);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x0000020c);
Xil_Out32(0xf60f6004, 0x00000200);
Xil_Out32(0xf60f600c, 0x00000000);
Xil_Out32(0xf6e6000c, 0xf9e8d7c6);
Xil_Out32(0xf6e60000, 0x01038000);
Xil_Out32(0xf6e60004, 0x01038000);
Xil_Out32(0xf6e60454, 0x00000006);
Xil_Out32(0xf6e602c8, 0x000097e8);
Xil_Out32(0xf6e6086c, 0x00000003);
Xil_Out32(0xf6e60864, 0x00000001);
Xil_Out32(0xf6e60868, 0x00000001);
Xil_Out32(0xf6e60858, 0x00000002);
Xil_Out32(0xf6e60860, 0x00000002);
Xil_Out32(0xf6e6083c, 0x00000100);
Xil_Out32(0xf6e60844, 0x00000100);
Xil_Out32(0xf6e60840, 0x00001fff);
Xil_Out32(0xf6e60848, 0x00001fff);
Xil_Out32(0xf6e603f0, 0x00005000);
Xil_Out32(0xf6e603f4, 0x00002000);
Xil_Out32(0xf6e603f8, 0x00003000);
Xil_Out32(0xf6e603fc, 0x00002000);
Xil_Out32(0xf6e60400, 0x00001000);
Xil_Out32(0xf6e60404, 0x00006000);
Xil_Out32(0xf6e60408, 0x00001000);
Xil_Out32(0xf6e603d0, 0x00000101);
Xil_Out32(0xf6e603d4, 0x00000181);
Xil_Out32(0xf6e603d8, 0x000001c1);
Xil_Out32(0xf6e603dc, 0x00000141);
Xil_Out32(0xf6e603e0, 0x00000341);
Xil_Out32(0xf6e603e4, 0x00000181);
Xil_Out32(0xf6e603e8, 0x00000a01);
Xil_Out32(0xf6e603ec, 0x00000c81);
Xil_Out32(0xf6e6040c, 0x00000080);
Xil_Out32(0xf6e60410, 0x00000080);
Xil_Out32(0xf6e60414, 0x00000081);
Xil_Out32(0xf6e60418, 0x00000081);
Xil_Out32(0xf6e60428, 0x00000080);
Xil_Out32(0xf6e6042c, 0x00000ffb);
Xil_Out32(0xf6e6041c, 0x00000080);
Xil_Out32(0xf6e60420, 0x00000080);
Xil_Out32(0xf6e60424, 0x00000080);
Xil_Out32(0xf6e60450, 0x00000600);
Xil_Out32(0xf6e60000, 0x00000080);
Xil_Out32(0xf6e602c4, 0x00000000);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x00000002);
Xil_Out32(0xf6e602c4, 0x00000000);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x00000040);
Xil_Out32(0xf6e602c4, 0x00000000);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x0000020c);
Xil_Out32(0xf6e60004, 0x00000200);
Xil_Out32(0xf6e6000c, 0x00000000);
Xil_Out32(0xf6e9000c, 0xf9e8d7c6);
Xil_Out32(0xf6e90000, 0x01038000);
Xil_Out32(0xf6e90004, 0x01028000);
Xil_Out32(0xf6e90454, 0x00000006);
Xil_Out32(0xf6e902c8, 0x00005798);
Xil_Out32(0xf6e9086c, 0x00000003);
Xil_Out32(0xf6e90864, 0x00000001);
Xil_Out32(0xf6e90868, 0x00000001);
Xil_Out32(0xf6e90858, 0x00000002);
Xil_Out32(0xf6e90860, 0x00000002);
Xil_Out32(0xf6e9083c, 0x00000100);
Xil_Out32(0xf6e90844, 0x00000100);
Xil_Out32(0xf6e90840, 0x00001fff);
Xil_Out32(0xf6e90848, 0x00001fff);
Xil_Out32(0xf6e903f0, 0x00006000);
Xil_Out32(0xf6e903f4, 0x00004000);
Xil_Out32(0xf6e903f8, 0x00005000);
Xil_Out32(0xf6e903fc, 0x00005000);
Xil_Out32(0xf6e90400, 0x00005000);
Xil_Out32(0xf6e90404, 0x00005000);
Xil_Out32(0xf6e90408, 0x00003000);
Xil_Out32(0xf6e903d0, 0x00000101);
Xil_Out32(0xf6e903d4, 0x00000181);
Xil_Out32(0xf6e903d8, 0x000001c1);
Xil_Out32(0xf6e903dc, 0x00000141);
Xil_Out32(0xf6e903e0, 0x00000341);
Xil_Out32(0xf6e903e4, 0x00000181);
Xil_Out32(0xf6e903e8, 0x00000a01);
Xil_Out32(0xf6e903ec, 0x00000c81);
Xil_Out32(0xf6e9040c, 0x00000001);
Xil_Out32(0xf6e90410, 0x00000001);
Xil_Out32(0xf6e90414, 0x00000001);
Xil_Out32(0xf6e90418, 0x00000001);
Xil_Out32(0xf6e90428, 0x00000001);
Xil_Out32(0xf6e9042c, 0x000001ac);
Xil_Out32(0xf6e9041c, 0x00000001);
Xil_Out32(0xf6e90420, 0x00000001);
Xil_Out32(0xf6e90424, 0x00000001);
Xil_Out32(0xf6e90440, 0x00000418);
Xil_Out32(0xf6e90444, 0x00000315);
Xil_Out32(0xf6e90448, 0x000003a7);
Xil_Out32(0xf6e9044c, 0x00000efa);
Xil_Out32(0xf6e90450, 0x00000601);
Xil_Out32(0xf6e90000, 0x00000080);
Xil_Out32(0xf6e902c4, 0x00000000);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x00000002);
Xil_Out32(0xf6e902c4, 0x00000000);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x00000040);
Xil_Out32(0xf6e902c4, 0x00000000);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x0000020c);
Xil_Out32(0xf6e90004, 0x00000200);
Xil_Out32(0xf6e9000c, 0x00000000);
Xil_Out32(0xf6ec000c, 0xf9e8d7c6);
Xil_Out32(0xf6ec0000, 0x01038000);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0454, 0x00000006);
Xil_Out32(0xf6ec02c8, 0x0000ffc0);
Xil_Out32(0xf6ec086c, 0x00000003);
Xil_Out32(0xf6ec0864, 0x00000001);
Xil_Out32(0xf6ec0868, 0x00000001);
Xil_Out32(0xf6ec0858, 0x00000002);
Xil_Out32(0xf6ec0860, 0x00000002);
Xil_Out32(0xf6ec083c, 0x00000100);
Xil_Out32(0xf6ec0844, 0x00000100);
Xil_Out32(0xf6ec0840, 0x00001fff);
Xil_Out32(0xf6ec0848, 0x00001fff);
Xil_Out32(0xf6ec03f0, 0x00003000);
Xil_Out32(0xf6ec03f4, 0x00003000);
Xil_Out32(0xf6ec03f8, 0x00001000);
Xil_Out32(0xf6ec03fc, 0x00002000);
Xil_Out32(0xf6ec0400, 0x00005000);
Xil_Out32(0xf6ec0404, 0x00005000);
Xil_Out32(0xf6ec0408, 0x00001000);
Xil_Out32(0xf6ec03d0, 0x000002c0);
Xil_Out32(0xf6ec03d4, 0x000002c0);
Xil_Out32(0xf6ec03d8, 0x000002c0);
Xil_Out32(0xf6ec03dc, 0x000002c0);
Xil_Out32(0xf6ec03e0, 0x000002c0);
Xil_Out32(0xf6ec03e4, 0x000002c0);
Xil_Out32(0xf6ec03e8, 0x000002c0);
Xil_Out32(0xf6ec03ec, 0x000002c0);
Xil_Out32(0xf6ec040c, 0x000002c0);
Xil_Out32(0xf6ec0410, 0x000002c0);
Xil_Out32(0xf6ec0414, 0x000002c0);
Xil_Out32(0xf6ec0418, 0x000002c0);
Xil_Out32(0xf6ec0428, 0x000002c0);
Xil_Out32(0xf6ec042c, 0x00000660);
Xil_Out32(0xf6ec041c, 0x000002c0);
Xil_Out32(0xf6ec0420, 0x000002c0);
Xil_Out32(0xf6ec0424, 0x000002c0);
Xil_Out32(0xf6ec0440, 0x00000419);
Xil_Out32(0xf6ec0444, 0x00000d87);
Xil_Out32(0xf6ec0448, 0x00000ef5);
Xil_Out32(0xf6ec044c, 0x000008e7);
Xil_Out32(0xf6ec0450, 0x00000602);
Xil_Out32(0xf6ec0000, 0x00000080);
Xil_Out32(0xf6ec02c4, 0x00000000);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x00000002);
Xil_Out32(0xf6ec02c4, 0x00000000);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x00000040);
Xil_Out32(0xf6ec02c4, 0x00000000);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x0000020c);
Xil_Out32(0xf6ec0004, 0x00000200);
Xil_Out32(0xf6ec000c, 0x00000000);
Xil_Out32(0xf6ee000c, 0xf9e8d7c6);
Xil_Out32(0xf6ee0000, 0x01038000);
Xil_Out32(0xf6ee0004, 0x01030000);
Xil_Out32(0xf6ee0454, 0x00000006);
Xil_Out32(0xf6ee02c8, 0x0000d198);
Xil_Out32(0xf6ee086c, 0x00000003);
Xil_Out32(0xf6ee0864, 0x00000001);
Xil_Out32(0xf6ee0868, 0x00000001);
Xil_Out32(0xf6ee0858, 0x00000002);
Xil_Out32(0xf6ee0860, 0x00000002);
Xil_Out32(0xf6ee083c, 0x00000100);
Xil_Out32(0xf6ee0844, 0x00000100);
Xil_Out32(0xf6ee0840, 0x00001fff);
Xil_Out32(0xf6ee0848, 0x00001fff);
Xil_Out32(0xf6ee03f0, 0x00004000);
Xil_Out32(0xf6ee03f4, 0x00003000);
Xil_Out32(0xf6ee03f8, 0x00002000);
Xil_Out32(0xf6ee03fc, 0x00005000);
Xil_Out32(0xf6ee0400, 0x00003000);
Xil_Out32(0xf6ee0404, 0x00003000);
Xil_Out32(0xf6ee0408, 0x00003000);
Xil_Out32(0xf6ee03d0, 0x00000240);
Xil_Out32(0xf6ee03d4, 0x00000240);
Xil_Out32(0xf6ee03d8, 0x00000240);
Xil_Out32(0xf6ee03dc, 0x00000240);
Xil_Out32(0xf6ee03e0, 0x00000240);
Xil_Out32(0xf6ee03e4, 0x00000240);
Xil_Out32(0xf6ee03e8, 0x00000240);
Xil_Out32(0xf6ee03ec, 0x00000240);
Xil_Out32(0xf6ee040c, 0x00000240);
Xil_Out32(0xf6ee0410, 0x00000240);
Xil_Out32(0xf6ee0414, 0x00000240);
Xil_Out32(0xf6ee0418, 0x00000240);
Xil_Out32(0xf6ee0428, 0x00000240);
Xil_Out32(0xf6ee042c, 0x00000095);
Xil_Out32(0xf6ee041c, 0x00000240);
Xil_Out32(0xf6ee0420, 0x00000240);
Xil_Out32(0xf6ee0424, 0x00000240);
Xil_Out32(0xf6ee0450, 0x00000603);
Xil_Out32(0xf6ee0000, 0x00000080);
Xil_Out32(0xf6ee02c4, 0x00000000);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x00000002);
Xil_Out32(0xf6ee02c4, 0x00000000);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x00000040);
Xil_Out32(0xf6ee02c4, 0x00000000);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x0000020c);
Xil_Out32(0xf6ee0004, 0x00000200);
Xil_Out32(0xf6ee000c, 0x00000000);
Xil_Out32(0xf6f3000c, 0xf9e8d7c6);
Xil_Out32(0xf6f30000, 0x01038000);
Xil_Out32(0xf6f30004, 0x01030000);
Xil_Out32(0xf6f30454, 0x00000006);
Xil_Out32(0xf6f302c8, 0x00005938);
Xil_Out32(0xf6f3086c, 0x00000003);
Xil_Out32(0xf6f30864, 0x00000001);
Xil_Out32(0xf6f30868, 0x00000001);
Xil_Out32(0xf6f30858, 0x00000002);
Xil_Out32(0xf6f30860, 0x00000002);
Xil_Out32(0xf6f3083c, 0x00000100);
Xil_Out32(0xf6f30844, 0x00000100);
Xil_Out32(0xf6f30840, 0x00001fff);
Xil_Out32(0xf6f30848, 0x00001fff);
Xil_Out32(0xf6f303f0, 0x00004000);
Xil_Out32(0xf6f303f4, 0x00003000);
Xil_Out32(0xf6f303f8, 0x00003000);
Xil_Out32(0xf6f303fc, 0x00004000);
Xil_Out32(0xf6f30400, 0x00004000);
Xil_Out32(0xf6f30404, 0x00001000);
Xil_Out32(0xf6f30408, 0x00005000);
Xil_Out32(0xf6f303d0, 0x00000281);
Xil_Out32(0xf6f303d4, 0x00000281);
Xil_Out32(0xf6f303d8, 0x00000281);
Xil_Out32(0xf6f303dc, 0x00000281);
Xil_Out32(0xf6f303e0, 0x00000281);
Xil_Out32(0xf6f303e4, 0x00000281);
Xil_Out32(0xf6f303e8, 0x00000281);
Xil_Out32(0xf6f303ec, 0x00000281);
Xil_Out32(0xf6f3040c, 0x00000281);
Xil_Out32(0xf6f30410, 0x00000281);
Xil_Out32(0xf6f30414, 0x00000281);
Xil_Out32(0xf6f30418, 0x00000281);
Xil_Out32(0xf6f30428, 0x00000281);
Xil_Out32(0xf6f3042c, 0x00000227);
Xil_Out32(0xf6f3041c, 0x00000281);
Xil_Out32(0xf6f30420, 0x00000281);
Xil_Out32(0xf6f30424, 0x00000281);
Xil_Out32(0xf6f30450, 0x00000605);
Xil_Out32(0xf6f30000, 0x00000080);
Xil_Out32(0xf6f302c4, 0x00000000);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x00000002);
Xil_Out32(0xf6f302c4, 0x00000000);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x00000040);
Xil_Out32(0xf6f302c4, 0x00000000);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x0000020c);
Xil_Out32(0xf6f30004, 0x00000200);
Xil_Out32(0xf6f3000c, 0x00000000);
Xil_Out32(0xf6c9000c, 0xf9e8d7c6);
Xil_Out32(0xf6c90000, 0x01038000);
Xil_Out32(0xf6c90004, 0x00010000);
Xil_Out32(0xf6c90454, 0x00000006);
Xil_Out32(0xf6c902c8, 0x000015b8);
Xil_Out32(0xf6c9086c, 0x00000003);
Xil_Out32(0xf6c90864, 0x00000001);
Xil_Out32(0xf6c90868, 0x00000001);
Xil_Out32(0xf6c90858, 0x00000002);
Xil_Out32(0xf6c90860, 0x00000002);
Xil_Out32(0xf6c9083c, 0x00000100);
Xil_Out32(0xf6c90844, 0x00000100);
Xil_Out32(0xf6c90840, 0x00001fff);
Xil_Out32(0xf6c90848, 0x00001fff);
Xil_Out32(0xf6c903f0, 0x00003000);
Xil_Out32(0xf6c903f4, 0x00003000);
Xil_Out32(0xf6c903f8, 0x00006000);
Xil_Out32(0xf6c903fc, 0x00003000);
Xil_Out32(0xf6c90400, 0x00006000);
Xil_Out32(0xf6c90404, 0x00005000);
Xil_Out32(0xf6c90408, 0x00003000);
Xil_Out32(0xf6c903d0, 0x00000101);
Xil_Out32(0xf6c903d4, 0x00000181);
Xil_Out32(0xf6c903d8, 0x000001c1);
Xil_Out32(0xf6c903dc, 0x00000141);
Xil_Out32(0xf6c903e0, 0x00000341);
Xil_Out32(0xf6c903e4, 0x00000181);
Xil_Out32(0xf6c903e8, 0x00000a01);
Xil_Out32(0xf6c903ec, 0x00000c81);
Xil_Out32(0xf6c90440, 0x00000488);
Xil_Out32(0xf6c90444, 0x00000489);
Xil_Out32(0xf6c90448, 0x000004c8);
Xil_Out32(0xf6c9044c, 0x000004c9);
Xil_Out32(0xf6c90450, 0x00000640);
Xil_Out32(0xf6c90000, 0x00000080);
Xil_Out32(0xf6c902c4, 0x00000000);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x00000002);
Xil_Out32(0xf6c902c4, 0x00000000);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x00000040);
Xil_Out32(0xf6c902c4, 0x00000000);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x0000020c);
Xil_Out32(0xf6c90004, 0x00000200);
Xil_Out32(0xf6c9000c, 0x00000000);
Xil_Out32(0xf6cc000c, 0xf9e8d7c6);
Xil_Out32(0xf6cc0000, 0x01038000);
Xil_Out32(0xf6cc0004, 0x01020000);
Xil_Out32(0xf6cc0454, 0x00000006);
Xil_Out32(0xf6cc02c8, 0x0000f2b8);
Xil_Out32(0xf6cc086c, 0x00000003);
Xil_Out32(0xf6cc0864, 0x00000001);
Xil_Out32(0xf6cc0868, 0x00000001);
Xil_Out32(0xf6cc0858, 0x00000002);
Xil_Out32(0xf6cc0860, 0x00000002);
Xil_Out32(0xf6cc083c, 0x00000100);
Xil_Out32(0xf6cc0844, 0x00000100);
Xil_Out32(0xf6cc0840, 0x00001fff);
Xil_Out32(0xf6cc0848, 0x00001fff);
Xil_Out32(0xf6cc03f0, 0x00006000);
Xil_Out32(0xf6cc03f4, 0x00003000);
Xil_Out32(0xf6cc03f8, 0x00001000);
Xil_Out32(0xf6cc03fc, 0x00006000);
Xil_Out32(0xf6cc0400, 0x00006000);
Xil_Out32(0xf6cc0404, 0x00005000);
Xil_Out32(0xf6cc0408, 0x00006000);
Xil_Out32(0xf6cc03d0, 0x00000101);
Xil_Out32(0xf6cc03d4, 0x00000181);
Xil_Out32(0xf6cc03d8, 0x000001c1);
Xil_Out32(0xf6cc03dc, 0x00000141);
Xil_Out32(0xf6cc03e0, 0x00000341);
Xil_Out32(0xf6cc03e4, 0x00000181);
Xil_Out32(0xf6cc03e8, 0x00000a01);
Xil_Out32(0xf6cc03ec, 0x00000c81);
Xil_Out32(0xf6cc0440, 0x00000508);
Xil_Out32(0xf6cc0444, 0x00000509);
Xil_Out32(0xf6cc0448, 0x00000548);
Xil_Out32(0xf6cc044c, 0x00000549);
Xil_Out32(0xf6cc0450, 0x00000641);
Xil_Out32(0xf6cc0000, 0x00000080);
Xil_Out32(0xf6cc02c4, 0x00000000);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x00000002);
Xil_Out32(0xf6cc02c4, 0x00000000);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x00000040);
Xil_Out32(0xf6cc02c4, 0x00000000);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x0000020c);
Xil_Out32(0xf6cc0004, 0x00000200);
Xil_Out32(0xf6cc000c, 0x00000000);
Xil_Out32(0xf602000c, 0xf9e8d7c6);
Xil_Out32(0xf6020000, 0x000002ff);
Xil_Out32(0xf6020004, 0x00000200);
Xil_Out32(0xf6020100, 0x00000081);
Xil_Out32(0xf6020104, 0x00000004);
Xil_Out32(0xf6020108, 0x00000001);
Xil_Out32(0xf602000c, 0x00000000);
Xil_Out32(0xf60e400c, 0xf9e8d7c6);
Xil_Out32(0xf60e4000, 0x000002ff);
Xil_Out32(0xf60e4004, 0x00000200);
Xil_Out32(0xf60e4100, 0x00000281);
Xil_Out32(0xf60e4104, 0x00000004);
Xil_Out32(0xf60e4108, 0x00000000);
Xil_Out32(0xf60e400c, 0x00000000);
Xil_Out32(0xf60e600c, 0xf9e8d7c6);
Xil_Out32(0xf60e6000, 0x000002ff);
Xil_Out32(0xf60e6004, 0x00000200);
Xil_Out32(0xf60e6100, 0x00000001);
Xil_Out32(0xf60e6104, 0x00000004);
Xil_Out32(0xf60e6108, 0x00000000);
Xil_Out32(0xf60e600c, 0x00000000);
Xil_Out32(0xf602200c, 0xf9e8d7c6);
Xil_Out32(0xf6022000, 0x000002ff);
Xil_Out32(0xf6022004, 0x00000200);
Xil_Out32(0xf6022100, 0x00000080);
Xil_Out32(0xf6022104, 0x00000004);
Xil_Out32(0xf6022108, 0x00000001);
Xil_Out32(0xf602200c, 0x00000000);
Xil_Out32(0xf60d000c, 0xf9e8d7c6);
Xil_Out32(0xf60d0000, 0x000002ff);
Xil_Out32(0xf60d0004, 0x00000200);
Xil_Out32(0xf60d0100, 0x000002c0);
Xil_Out32(0xf60d0104, 0x00000004);
Xil_Out32(0xf60d0108, 0x00000001);
Xil_Out32(0xf60d000c, 0x00000000);
Xil_Out32(0xf60d200c, 0xf9e8d7c6);
Xil_Out32(0xf60d2000, 0x000002ff);
Xil_Out32(0xf60d2004, 0x00000200);
Xil_Out32(0xf60d2100, 0x00000240);
Xil_Out32(0xf60d2104, 0x00000004);
Xil_Out32(0xf60d2108, 0x00000000);
Xil_Out32(0xf60d200c, 0x00000000);
Xil_Out32(0xf6e6200c, 0xf9e8d7c6);
Xil_Out32(0xf6e62000, 0x000002ff);
Xil_Out32(0xf6e62004, 0x00000200);
Xil_Out32(0xf6e62100, 0x00000608);
Xil_Out32(0xf6e62104, 0x00000006);
Xil_Out32(0xf6e62108, 0x00000001);
Xil_Out32(0xf6e6200c, 0x00000000);
Xil_Out32(0xf6e9200c, 0xf9e8d7c6);
Xil_Out32(0xf6e92000, 0x000002ff);
Xil_Out32(0xf6e92004, 0x00000200);
Xil_Out32(0xf6e92100, 0x00000609);
Xil_Out32(0xf6e92104, 0x00000006);
Xil_Out32(0xf6e92108, 0x00000000);
Xil_Out32(0xf6e9200c, 0x00000000);
Xil_Out32(0xf6ec200c, 0xf9e8d7c6);
Xil_Out32(0xf6ec2000, 0x000002ff);
Xil_Out32(0xf6ec2004, 0x00000200);
Xil_Out32(0xf6ec2100, 0x0000060a);
Xil_Out32(0xf6ec2104, 0x00000006);
Xil_Out32(0xf6ec2108, 0x00000000);
Xil_Out32(0xf6ec200c, 0x00000000);
Xil_Out32(0xf6ee200c, 0xf9e8d7c6);
Xil_Out32(0xf6ee2000, 0x000002ff);
Xil_Out32(0xf6ee2004, 0x00000200);
Xil_Out32(0xf6ee2100, 0x0000060b);
Xil_Out32(0xf6ee2104, 0x00000006);
Xil_Out32(0xf6ee2108, 0x00000001);
Xil_Out32(0xf6ee200c, 0x00000000);
Xil_Out32(0xf6f1200c, 0xf9e8d7c6);
Xil_Out32(0xf6f12000, 0x000002ff);
Xil_Out32(0xf6f12004, 0x00000200);
Xil_Out32(0xf6f12100, 0x0000060c);
Xil_Out32(0xf6f12104, 0x00000006);
Xil_Out32(0xf6f12108, 0x00000001);
Xil_Out32(0xf6f1200c, 0x00000000);
Xil_Out32(0xf6f3200c, 0xf9e8d7c6);
Xil_Out32(0xf6f32000, 0x000002ff);
Xil_Out32(0xf6f32004, 0x00000200);
Xil_Out32(0xf6f32100, 0x0000060d);
Xil_Out32(0xf6f32104, 0x00000006);
Xil_Out32(0xf6f32108, 0x00000001);
Xil_Out32(0xf6f3200c, 0x00000000);
Xil_Out32(0xf6f6200c, 0xf9e8d7c6);
Xil_Out32(0xf6f62000, 0x000002ff);
Xil_Out32(0xf6f62004, 0x00000200);
Xil_Out32(0xf6f62100, 0x0000060e);
Xil_Out32(0xf6f62104, 0x00000006);
Xil_Out32(0xf6f62108, 0x00000001);
Xil_Out32(0xf6f6200c, 0x00000000);
Xil_Out32(0xf607000c, 0xf9e8d7c6);
Xil_Out32(0xf607000c, 0xf9e8d7c6);
Xil_Out32(0xf6070010, 0x06170651);
Xil_Out32(0xf6070014, 0x00637265);
Xil_Out32(0xf6070018, 0x06170651);
Xil_Out32(0xf607001c, 0x00637265);
Xil_Out32(0xf6070020, 0x06170651);
Xil_Out32(0xf6070024, 0x00637265);
Xil_Out32(0xf6070028, 0x06170651);
Xil_Out32(0xf607002c, 0x00637265);
Xil_Out32(0xf6070048, 0x15513491);
Xil_Out32(0xf607004c, 0x1a6585d6);
Xil_Out32(0xf6070050, 0x1f79d71b);
Xil_Out32(0xf6070054, 0x03030c20);
Xil_Out32(0xf6070058, 0x081c6144);
Xil_Out32(0xf607005c, 0x0f30b289);
Xil_Out32(0xf6070060, 0x00c0e350);
Xil_Out32(0xf6070044, 0x30c30c30);
Xil_Out32(0xf607044c, 0x00884210);
Xil_Out32(0xf607044c, 0x00844210);
Xil_Out32(0xf607044c, 0x00842210);
Xil_Out32(0xf607044c, 0x00842110);
Xil_Out32(0xf607044c, 0x00842108);
Xil_Out32(0xf6070448, 0x0003fc02);
Xil_Out32(0xf6070448, 0x0003fcff);
Xil_Out32(0xf6070444, 0x0ff00804);
Xil_Out32(0xf6070444, 0x0ff3fc04);
Xil_Out32(0xf6070444, 0x0ff3fcff);
Xil_Out32(0xf607046c, 0x003fffff);
Xil_Out32(0xf6070468, 0x003fffff);
Xil_Out32(0xf6070464, 0x003fffff);
Xil_Out32(0xf6070460, 0x003fffff);
Xil_Out32(0xf607045c, 0x003fffff);
Xil_Out32(0xf607046c, 0x000fffff);
Xil_Out32(0xf6070468, 0x000fffff);
Xil_Out32(0xf6070464, 0x000fffff);
Xil_Out32(0xf6070460, 0x000fffff);
Xil_Out32(0xf607045c, 0x000fffff);
Xil_Out32(0xf6070480, 0x003fffff);
Xil_Out32(0xf607047c, 0x003fffff);
Xil_Out32(0xf6070478, 0x003fffff);
Xil_Out32(0xf6070474, 0x003fffff);
Xil_Out32(0xf6070470, 0x003fffff);
Xil_Out32(0xf6070480, 0x000fffff);
Xil_Out32(0xf607047c, 0x000fffff);
Xil_Out32(0xf6070478, 0x000fffff);
Xil_Out32(0xf6070474, 0x000fffff);
Xil_Out32(0xf6070470, 0x000fffff);
Xil_Out32(0xf6070494, 0x003fffff);
Xil_Out32(0xf6070490, 0x003fffff);
Xil_Out32(0xf607048c, 0x003fffff);
Xil_Out32(0xf6070488, 0x003fffff);
Xil_Out32(0xf6070484, 0x003fffff);
Xil_Out32(0xf6070494, 0x000fffff);
Xil_Out32(0xf6070490, 0x000fffff);
Xil_Out32(0xf607048c, 0x000fffff);
Xil_Out32(0xf6070488, 0x000fffff);
Xil_Out32(0xf6070484, 0x000fffff);
Xil_Out32(0xf60704a8, 0x003fffff);
Xil_Out32(0xf60704a4, 0x003fffff);
Xil_Out32(0xf60704a0, 0x003fffff);
Xil_Out32(0xf607049c, 0x003fffff);
Xil_Out32(0xf6070498, 0x003fffff);
Xil_Out32(0xf60704a8, 0x000fffff);
Xil_Out32(0xf60704a4, 0x000fffff);
Xil_Out32(0xf60704a0, 0x000fffff);
Xil_Out32(0xf607049c, 0x000fffff);
Xil_Out32(0xf6070498, 0x000fffff);
Xil_Out32(0xf60704ac, 0x01f84210);
Xil_Out32(0xf60704ac, 0x01ffc210);
Xil_Out32(0xf60704ac, 0x01fffe10);
Xil_Out32(0xf60704ac, 0x01fffff0);
Xil_Out32(0xf60704ac, 0x01ffffff);
Xil_Out32(0xf60704b0, 0x00084210);
Xil_Out32(0xf60704b0, 0x00004210);
Xil_Out32(0xf60704b0, 0x00000210);
Xil_Out32(0xf60704b0, 0x00000010);
Xil_Out32(0xf60704b0, 0x00000000);
Xil_Out32(0xf60704b4, 0x003fffff);
Xil_Out32(0xf60704b8, 0x003fffff);
Xil_Out32(0xf60704bc, 0x003fffff);
Xil_Out32(0xf60704c0, 0x003fffff);
Xil_Out32(0xf60704c4, 0x003fffff);
Xil_Out32(0xf6070418, 0x01010ff0);
Xil_Out32(0xf6070410, 0x01010ff0);
Xil_Out32(0xf6070408, 0x01010ff0);
Xil_Out32(0xf6070400, 0x01010ff0);
Xil_Out32(0xf6070418, 0x010ffff0);
Xil_Out32(0xf6070410, 0x010ffff0);
Xil_Out32(0xf6070408, 0x010ffff0);
Xil_Out32(0xf6070400, 0x010ffff0);
Xil_Out32(0xf6070418, 0x0ffffff0);
Xil_Out32(0xf6070410, 0x0ffffff0);
Xil_Out32(0xf6070408, 0x0ffffff0);
Xil_Out32(0xf6070400, 0x0ffffff0);
Xil_Out32(0xf607041c, 0x00010ff0);
Xil_Out32(0xf6070414, 0x00010ff0);
Xil_Out32(0xf607040c, 0x00010ff0);
Xil_Out32(0xf6070404, 0x00010ff0);
Xil_Out32(0xf607041c, 0x000ffff0);
Xil_Out32(0xf6070414, 0x000ffff0);
Xil_Out32(0xf607040c, 0x000ffff0);
Xil_Out32(0xf6070404, 0x000ffff0);
Xil_Out32(0xf6070438, 0x000404ff);
Xil_Out32(0xf6070430, 0x000404ff);
Xil_Out32(0xf6070428, 0x000404ff);
Xil_Out32(0xf6070420, 0x000404ff);
Xil_Out32(0xf6070438, 0x0004ffff);
Xil_Out32(0xf6070430, 0x0004ffff);
Xil_Out32(0xf6070428, 0x0004ffff);
Xil_Out32(0xf6070420, 0x0004ffff);
Xil_Out32(0xf6070438, 0x00ffffff);
Xil_Out32(0xf6070430, 0x00ffffff);
Xil_Out32(0xf6070428, 0x00ffffff);
Xil_Out32(0xf6070420, 0x00ffffff);
Xil_Out32(0xf607043c, 0x000004ff);
Xil_Out32(0xf6070434, 0x000004ff);
Xil_Out32(0xf607042c, 0x000004ff);
Xil_Out32(0xf6070424, 0x000004ff);
Xil_Out32(0xf607043c, 0x0000ffff);
Xil_Out32(0xf6070434, 0x0000ffff);
Xil_Out32(0xf607042c, 0x0000ffff);
Xil_Out32(0xf6070424, 0x0000ffff);
Xil_Out32(0xf6070064, 0x00000101);
Xil_Out32(0xf6070068, 0x00000181);
Xil_Out32(0xf607006c, 0x000001c1);
Xil_Out32(0xf6070070, 0x00000141);
Xil_Out32(0xf6070758, 0x00000001);
//**************Register programming from noc_cfg end----------------

dbg0_pmc(16385); // ***** rank_addr_count is overwritten to 0 *****

 //--------------------------------------
//**************Register programming from  xphy_pll start----------------

//************************************************************************************************* XPIO_PLL : begin

Xil_Out32(0xf616900c, 0xf9e8d7c6);
prog_reg(0xF6169038, 0xC, 0x1000, 0x1);
prog_reg(0xF616903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF616903C, 0x0, 0xFF, 0x2);
prog_reg(0xF6169040, 0xC, 0x1000, 0x1);
prog_reg(0xF6169040, 0xB, 0x800, 0x0);
prog_reg(0xF6169038, 0xB, 0x800, 0x0);
prog_reg(0xF6169048, 0xB, 0x800, 0x0);
prog_reg(0xF6169050, 0xB, 0x800, 0x0);
prog_reg(0xF6169044, 0x8, 0xFF00, 0x2);
prog_reg(0xF6169044, 0x0, 0xFF, 0x2);
prog_reg(0xF6169048, 0xC, 0x1000, 0x1);
prog_reg(0xF616904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF616904C, 0x0, 0xFF, 0x2);
prog_reg(0xF6169030, 0xD, 0xE000, 0x0);
prog_reg(0xF6169050, 0xC, 0x1000, 0x1);
prog_reg(0xF6169054, 0x8, 0xFF00, 0x2);
prog_reg(0xF6169054, 0x0, 0xFF, 0x2);
prog_reg(0xF6169034, 0x8, 0xFF00, 0x10);
prog_reg(0xF6169034, 0x0, 0xFF, 0x10);
prog_reg(0xF6169030, 0x9, 0x200, 0x1);
prog_reg(0xF6169030, 0xA, 0xC00, 0x1);
prog_reg(0xF6169030, 0x0, 0xFF, 0x0);
prog_reg(0xF6169030, 0x8, 0x100, 0x0);
prog_reg(0xF6169030, 0xC, 0x1000, 0x0);
prog_reg(0xF6169088, 0x5, 0x20, 0x0);
prog_reg(0xF6169094, 0xA, 0x7C00, 0x3);
prog_reg(0xF6169098, 0x2, 0x7C, 0x3);
Xil_Out32(0xf616980c, 0xf9e8d7c6);
prog_reg(0xF6169810, 0xB, 0x1800, 0x3);
prog_reg(0xF6169810, 0x8, 0x700, 0x3);
prog_reg(0xF6169810, 0x5, 0xE0, 0x0);
prog_reg(0xF6169810, 0x2, 0x1C, 0x0);
prog_reg(0xF6169000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x6, 0x40, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x8, 0x100, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x7, 0x80, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169800, 0x1, 0x2, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x6, 0x40, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x8, 0x100, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x7, 0x80, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
dbg0_pmc(16400);
//polling XPIO_1
poll_for(0xf6169008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf6169004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL : end

Xil_Out32(0xf609900c, 0xf9e8d7c6);
prog_reg(0xF6099038, 0xC, 0x1000, 0x1);
prog_reg(0xF609903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF609903C, 0x0, 0xFF, 0x2);
prog_reg(0xF6099040, 0xC, 0x1000, 0x1);
prog_reg(0xF6099040, 0xB, 0x800, 0x0);
prog_reg(0xF6099038, 0xB, 0x800, 0x0);
prog_reg(0xF6099048, 0xB, 0x800, 0x0);
prog_reg(0xF6099050, 0xB, 0x800, 0x0);
prog_reg(0xF6099044, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099044, 0x0, 0xFF, 0x2);
prog_reg(0xF6099048, 0xC, 0x1000, 0x1);
prog_reg(0xF609904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF609904C, 0x0, 0xFF, 0x2);
prog_reg(0xF6099030, 0xD, 0xE000, 0x0);
prog_reg(0xF6099050, 0xC, 0x1000, 0x1);
prog_reg(0xF6099054, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099054, 0x0, 0xFF, 0x2);
prog_reg(0xF60990A0, 0xA, 0x7C00, 0xA);
prog_reg(0xF60990A4, 0xA, 0x7C00, 0xA);
prog_reg(0xF6099034, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099034, 0x0, 0xFF, 0x2);
prog_reg(0xF6099030, 0x9, 0x200, 0x1);
prog_reg(0xF6099030, 0xA, 0xC00, 0x1);
prog_reg(0xF6099030, 0x0, 0xFF, 0x0);
prog_reg(0xF6099030, 0x8, 0x100, 0x0);
prog_reg(0xF6099030, 0xC, 0x1000, 0x0);
prog_reg(0xF6099088, 0x5, 0x20, 0x0);
Xil_Out32(0xf609980c, 0xf9e8d7c6);
prog_reg(0xF6099810, 0xB, 0x1800, 0x3);
prog_reg(0xF6099810, 0x8, 0x700, 0x2);
prog_reg(0xF6099810, 0x5, 0xE0, 0x1);
prog_reg(0xF6099810, 0x2, 0x1C, 0x5);
prog_reg(0xF6099000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x6, 0x40, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x8, 0x100, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x7, 0x80, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099800, 0x1, 0x2, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x6, 0x40, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x8, 0x100, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x7, 0x80, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
dbg0_pmc(16400);
//polling XPIO_0
poll_for(0xf6099008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf6099004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL : end

Xil_Out32(0xf61c900c, 0xf9e8d7c6);
prog_reg(0xF61C9038, 0xC, 0x1000, 0x1);
prog_reg(0xF61C903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C903C, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9040, 0xC, 0x1000, 0x1);
prog_reg(0xF61C9040, 0xB, 0x800, 0x0);
prog_reg(0xF61C9038, 0xB, 0x800, 0x0);
prog_reg(0xF61C9048, 0xB, 0x800, 0x0);
prog_reg(0xF61C9050, 0xB, 0x800, 0x0);
prog_reg(0xF61C9044, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9044, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9048, 0xC, 0x1000, 0x1);
prog_reg(0xF61C904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C904C, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9030, 0xD, 0xE000, 0x0);
prog_reg(0xF61C9050, 0xC, 0x1000, 0x1);
prog_reg(0xF61C9054, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9054, 0x0, 0xFF, 0x2);
prog_reg(0xF61C90A0, 0xA, 0x7C00, 0xA);
prog_reg(0xF61C90A4, 0xA, 0x7C00, 0xA);
prog_reg(0xF61C9034, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9034, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9030, 0x9, 0x200, 0x1);
prog_reg(0xF61C9030, 0xA, 0xC00, 0x1);
prog_reg(0xF61C9030, 0x0, 0xFF, 0x0);
prog_reg(0xF61C9030, 0x8, 0x100, 0x0);
prog_reg(0xF61C9030, 0xC, 0x1000, 0x0);
prog_reg(0xF61C9088, 0x5, 0x20, 0x0);
Xil_Out32(0xf61c980c, 0xf9e8d7c6);
prog_reg(0xF61C9810, 0xB, 0x1800, 0x3);
prog_reg(0xF61C9810, 0x8, 0x700, 0x2);
prog_reg(0xF61C9810, 0x5, 0xE0, 0x1);
prog_reg(0xF61C9810, 0x2, 0x1C, 0x3);
prog_reg(0xF61C9000, 0x1, 0x2, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x6, 0x40, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x8, 0x100, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x7, 0x80, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9800, 0x1, 0x2, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x6, 0x40, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x8, 0x100, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x7, 0x80, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
dbg0_pmc(16400);
//polling XPIO_2
poll_for(0xf61c9008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf61c9004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL : end

//************************************************************************************************* XPIO_PLL : end

//**************Register programming from  xphy_pll end----------------

//**************Register programming from  xphy_init start----------------

dbg0_pmc(16386);
//************************************************************************************************* XPHY_CFG : begin

//****************************************************
//XPIO_0
//****************************************************
Xil_Out32(0xf609900c, 0xf9e8d7c6);
Xil_Out32(0xf6099000, 0x000011c2);
Xil_Out32(0xf6099004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf609020c, 0xf9e8d7c6);
Xil_Out32(0xf609040c, 0xf9e8d7c6);
Xil_Out32(0xf609060c, 0xf9e8d7c6);
Xil_Out32(0xf6090228, 0x00000006);
Xil_Out32(0xf6090428, 0x00000006);
Xil_Out32(0xf6090628, 0x00000006);
Xil_Out32(0xf6090244, 0x00000006);
Xil_Out32(0xf6090444, 0x00000006);
Xil_Out32(0xf6090644, 0x00000006);
Xil_Out32(0xf609000c, 0xf9e8d7c6);
Xil_Out32(0xf609001c, 0x000000b3);
Xil_Out32(0xf6090020, 0x0000020b);
Xil_Out32(0xf6090034, 0x00001420);
Xil_Out32(0xf609002c, 0x00000102);
Xil_Out32(0xf6090058, 0x0000003d);
Xil_Out32(0xf609004c, 0x00000100);
Xil_Out32(0xf6090048, 0x00000010);
Xil_Out32(0xf6090004, 0x000011fe);
Xil_Out32(0xf6090028, 0x00000000);
Xil_Out32(0xf60900dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf609120c, 0xf9e8d7c6);
Xil_Out32(0xf609140c, 0xf9e8d7c6);
Xil_Out32(0xf609160c, 0xf9e8d7c6);
Xil_Out32(0xf6091228, 0x00000006);
Xil_Out32(0xf6091428, 0x00000006);
Xil_Out32(0xf6091628, 0x00000006);
Xil_Out32(0xf6091244, 0x00000006);
Xil_Out32(0xf6091444, 0x00000006);
Xil_Out32(0xf6091644, 0x00000006);
Xil_Out32(0xf609100c, 0xf9e8d7c6);
Xil_Out32(0xf609101c, 0x000000b0);
Xil_Out32(0xf6091020, 0x0000020f);
Xil_Out32(0xf6091034, 0x00001420);
Xil_Out32(0xf6091058, 0x0000003f);
Xil_Out32(0xf609104c, 0x00000100);
Xil_Out32(0xf6091048, 0x00000010);
Xil_Out32(0xf6091004, 0x000011fe);
Xil_Out32(0xf6091028, 0x00000000);
Xil_Out32(0xf60910dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf609220c, 0xf9e8d7c6);
Xil_Out32(0xf609240c, 0xf9e8d7c6);
Xil_Out32(0xf609260c, 0xf9e8d7c6);
Xil_Out32(0xf6092228, 0x00000006);
Xil_Out32(0xf6092428, 0x00000006);
Xil_Out32(0xf6092628, 0x00000006);
Xil_Out32(0xf6092244, 0x00000006);
Xil_Out32(0xf6092444, 0x00000006);
Xil_Out32(0xf6092644, 0x00000006);
Xil_Out32(0xf609200c, 0xf9e8d7c6);
Xil_Out32(0xf6092020, 0x000002fc);
Xil_Out32(0xf6092034, 0x00001420);
Xil_Out32(0xf609202c, 0x0000013f);
Xil_Out32(0xf609204c, 0x00000100);
Xil_Out32(0xf6092048, 0x00000010);
Xil_Out32(0xf6092004, 0x000011fe);
Xil_Out32(0xf6092028, 0x00000000);
Xil_Out32(0xf60920dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf609320c, 0xf9e8d7c6);
Xil_Out32(0xf609340c, 0xf9e8d7c6);
Xil_Out32(0xf609360c, 0xf9e8d7c6);
Xil_Out32(0xf6093228, 0x00000006);
Xil_Out32(0xf6093428, 0x00000006);
Xil_Out32(0xf6093628, 0x00000006);
Xil_Out32(0xf6093244, 0x00000006);
Xil_Out32(0xf6093444, 0x00000006);
Xil_Out32(0xf6093644, 0x00000006);
Xil_Out32(0xf609300c, 0xf9e8d7c6);
Xil_Out32(0xf6093020, 0x000002fc);
Xil_Out32(0xf6093034, 0x00001420);
Xil_Out32(0xf609302c, 0x0000013c);
Xil_Out32(0xf609304c, 0x00000100);
Xil_Out32(0xf6093048, 0x00000010);
Xil_Out32(0xf6093004, 0x000011fe);
Xil_Out32(0xf6093028, 0x00000000);
Xil_Out32(0xf60930dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf609420c, 0xf9e8d7c6);
Xil_Out32(0xf609440c, 0xf9e8d7c6);
Xil_Out32(0xf609460c, 0xf9e8d7c6);
Xil_Out32(0xf6094228, 0x00000006);
Xil_Out32(0xf6094428, 0x00000006);
Xil_Out32(0xf6094628, 0x00000006);
Xil_Out32(0xf6094244, 0x00000006);
Xil_Out32(0xf6094444, 0x00000006);
Xil_Out32(0xf6094644, 0x00000006);
Xil_Out32(0xf609400c, 0xf9e8d7c6);
Xil_Out32(0xf6094020, 0x000002fc);
Xil_Out32(0xf6094034, 0x00001420);
Xil_Out32(0xf609402c, 0x0000013f);
Xil_Out32(0xf609404c, 0x00000100);
Xil_Out32(0xf6094048, 0x00000010);
Xil_Out32(0xf6094004, 0x000011fe);
Xil_Out32(0xf6094028, 0x00000000);
Xil_Out32(0xf60940dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf609520c, 0xf9e8d7c6);
Xil_Out32(0xf609540c, 0xf9e8d7c6);
Xil_Out32(0xf609560c, 0xf9e8d7c6);
Xil_Out32(0xf6095228, 0x00000006);
Xil_Out32(0xf6095428, 0x00000006);
Xil_Out32(0xf6095628, 0x00000006);
Xil_Out32(0xf6095244, 0x00000006);
Xil_Out32(0xf6095444, 0x00000006);
Xil_Out32(0xf6095644, 0x00000006);
Xil_Out32(0xf609500c, 0xf9e8d7c6);
Xil_Out32(0xf6095020, 0x000002fc);
Xil_Out32(0xf6095034, 0x00001420);
Xil_Out32(0xf609502c, 0x0000013f);
Xil_Out32(0xf609504c, 0x00000100);
Xil_Out32(0xf6095048, 0x00000010);
Xil_Out32(0xf6095004, 0x000011fe);
Xil_Out32(0xf6095028, 0x00000000);
Xil_Out32(0xf60950dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf609620c, 0xf9e8d7c6);
Xil_Out32(0xf609640c, 0xf9e8d7c6);
Xil_Out32(0xf609660c, 0xf9e8d7c6);
Xil_Out32(0xf6096228, 0x00000006);
Xil_Out32(0xf6096428, 0x00000006);
Xil_Out32(0xf6096628, 0x00000006);
Xil_Out32(0xf6096244, 0x00000006);
Xil_Out32(0xf6096444, 0x00000006);
Xil_Out32(0xf6096644, 0x00000006);
Xil_Out32(0xf609600c, 0xf9e8d7c6);
Xil_Out32(0xf6096020, 0x000002fc);
Xil_Out32(0xf6096034, 0x00001420);
Xil_Out32(0xf609602c, 0x0000013f);
Xil_Out32(0xf609604c, 0x00000100);
Xil_Out32(0xf6096048, 0x00000010);
Xil_Out32(0xf6096004, 0x000011fe);
Xil_Out32(0xf6096028, 0x00000000);
Xil_Out32(0xf60960dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf609720c, 0xf9e8d7c6);
Xil_Out32(0xf609740c, 0xf9e8d7c6);
Xil_Out32(0xf609760c, 0xf9e8d7c6);
Xil_Out32(0xf6097228, 0x00000006);
Xil_Out32(0xf6097428, 0x00000006);
Xil_Out32(0xf6097628, 0x00000006);
Xil_Out32(0xf6097244, 0x00000006);
Xil_Out32(0xf6097444, 0x00000006);
Xil_Out32(0xf6097644, 0x00000006);
Xil_Out32(0xf609700c, 0xf9e8d7c6);
Xil_Out32(0xf6097020, 0x000002fc);
Xil_Out32(0xf6097034, 0x00001420);
Xil_Out32(0xf609702c, 0x0000012f);
Xil_Out32(0xf609704c, 0x00000100);
Xil_Out32(0xf6097048, 0x00000010);
Xil_Out32(0xf6097004, 0x000011fe);
Xil_Out32(0xf6097028, 0x00000000);
Xil_Out32(0xf60970dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf609820c, 0xf9e8d7c6);
Xil_Out32(0xf609840c, 0xf9e8d7c6);
Xil_Out32(0xf609860c, 0xf9e8d7c6);
Xil_Out32(0xf6098228, 0x00000006);
Xil_Out32(0xf6098428, 0x00000006);
Xil_Out32(0xf6098628, 0x00000006);
Xil_Out32(0xf6098244, 0x00000006);
Xil_Out32(0xf6098444, 0x00000006);
Xil_Out32(0xf6098644, 0x00000006);
Xil_Out32(0xf609800c, 0xf9e8d7c6);
Xil_Out32(0xf6098020, 0x000002fc);
Xil_Out32(0xf6098034, 0x00001420);
Xil_Out32(0xf609802c, 0x0000013f);
Xil_Out32(0xf609804c, 0x00000100);
Xil_Out32(0xf6098048, 0x00000010);
Xil_Out32(0xf6098004, 0x000011fe);
Xil_Out32(0xf6098028, 0x00000000);
Xil_Out32(0xf60980dc, 0x00000004);
//****************************************************
//XPIO_1
//****************************************************
Xil_Out32(0xf616900c, 0xf9e8d7c6);
Xil_Out32(0xf6169000, 0x000011c2);
Xil_Out32(0xf6169004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf616020c, 0xf9e8d7c6);
Xil_Out32(0xf616040c, 0xf9e8d7c6);
Xil_Out32(0xf616060c, 0xf9e8d7c6);
Xil_Out32(0xf6160228, 0x00000006);
Xil_Out32(0xf6160428, 0x00000006);
Xil_Out32(0xf6160628, 0x00000006);
Xil_Out32(0xf6160244, 0x00000006);
Xil_Out32(0xf6160444, 0x00000006);
Xil_Out32(0xf6160644, 0x00000006);
Xil_Out32(0xf616000c, 0xf9e8d7c6);
Xil_Out32(0xf616001c, 0x000000b3);
Xil_Out32(0xf6160020, 0x0000020b);
Xil_Out32(0xf6160034, 0x00001420);
Xil_Out32(0xf616002c, 0x00000102);
Xil_Out32(0xf6160058, 0x0000003d);
Xil_Out32(0xf616004c, 0x00000100);
Xil_Out32(0xf6160048, 0x00000010);
Xil_Out32(0xf6160004, 0x000011fe);
Xil_Out32(0xf6160028, 0x00000000);
Xil_Out32(0xf61600dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf616120c, 0xf9e8d7c6);
Xil_Out32(0xf616140c, 0xf9e8d7c6);
Xil_Out32(0xf616160c, 0xf9e8d7c6);
Xil_Out32(0xf6161228, 0x00000006);
Xil_Out32(0xf6161428, 0x00000006);
Xil_Out32(0xf6161628, 0x00000006);
Xil_Out32(0xf6161244, 0x00000006);
Xil_Out32(0xf6161444, 0x00000006);
Xil_Out32(0xf6161644, 0x00000006);
Xil_Out32(0xf616100c, 0xf9e8d7c6);
Xil_Out32(0xf616101c, 0x000000b0);
Xil_Out32(0xf6161020, 0x0000020f);
Xil_Out32(0xf6161034, 0x00001420);
Xil_Out32(0xf6161058, 0x0000003f);
Xil_Out32(0xf616104c, 0x00000100);
Xil_Out32(0xf6161048, 0x00000010);
Xil_Out32(0xf6161004, 0x000011fe);
Xil_Out32(0xf6161028, 0x00000000);
Xil_Out32(0xf61610dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf616220c, 0xf9e8d7c6);
Xil_Out32(0xf616240c, 0xf9e8d7c6);
Xil_Out32(0xf616260c, 0xf9e8d7c6);
Xil_Out32(0xf6162228, 0x00000006);
Xil_Out32(0xf6162428, 0x00000006);
Xil_Out32(0xf6162628, 0x00000006);
Xil_Out32(0xf6162244, 0x00000006);
Xil_Out32(0xf6162444, 0x00000006);
Xil_Out32(0xf6162644, 0x00000006);
Xil_Out32(0xf616200c, 0xf9e8d7c6);
Xil_Out32(0xf616201c, 0x000000b0);
Xil_Out32(0xf6162020, 0x0000020f);
Xil_Out32(0xf6162034, 0x00001420);
Xil_Out32(0xf6162058, 0x0000003f);
Xil_Out32(0xf616204c, 0x00000100);
Xil_Out32(0xf6162048, 0x00000010);
Xil_Out32(0xf6162004, 0x000011fe);
Xil_Out32(0xf6162028, 0x00000000);
Xil_Out32(0xf61620dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf616320c, 0xf9e8d7c6);
Xil_Out32(0xf616340c, 0xf9e8d7c6);
Xil_Out32(0xf616360c, 0xf9e8d7c6);
Xil_Out32(0xf6163228, 0x00000006);
Xil_Out32(0xf6163428, 0x00000006);
Xil_Out32(0xf6163628, 0x00000006);
Xil_Out32(0xf6163244, 0x00000006);
Xil_Out32(0xf6163444, 0x00000006);
Xil_Out32(0xf6163644, 0x00000006);
Xil_Out32(0xf616300c, 0xf9e8d7c6);
Xil_Out32(0xf616301c, 0x000000b3);
Xil_Out32(0xf6163020, 0x0000020b);
Xil_Out32(0xf6163034, 0x00001420);
Xil_Out32(0xf616302c, 0x00000102);
Xil_Out32(0xf6163058, 0x0000003d);
Xil_Out32(0xf616304c, 0x00000100);
Xil_Out32(0xf6163048, 0x00000010);
Xil_Out32(0xf6163004, 0x000011fe);
Xil_Out32(0xf6163028, 0x00000000);
Xil_Out32(0xf61630dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf616420c, 0xf9e8d7c6);
Xil_Out32(0xf616440c, 0xf9e8d7c6);
Xil_Out32(0xf616460c, 0xf9e8d7c6);
Xil_Out32(0xf6164228, 0x00000006);
Xil_Out32(0xf6164428, 0x00000006);
Xil_Out32(0xf6164628, 0x00000006);
Xil_Out32(0xf6164244, 0x00000006);
Xil_Out32(0xf6164444, 0x00000006);
Xil_Out32(0xf6164644, 0x00000006);
Xil_Out32(0xf616400c, 0xf9e8d7c6);
Xil_Out32(0xf616401c, 0x000000b3);
Xil_Out32(0xf6164020, 0x0000020b);
Xil_Out32(0xf6164034, 0x00001420);
Xil_Out32(0xf616402c, 0x00000102);
Xil_Out32(0xf6164058, 0x0000003d);
Xil_Out32(0xf616404c, 0x00000100);
Xil_Out32(0xf6164048, 0x00000010);
Xil_Out32(0xf6164004, 0x000011fe);
Xil_Out32(0xf6164028, 0x00000000);
Xil_Out32(0xf61640dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf616520c, 0xf9e8d7c6);
Xil_Out32(0xf616540c, 0xf9e8d7c6);
Xil_Out32(0xf616560c, 0xf9e8d7c6);
Xil_Out32(0xf6165228, 0x00000006);
Xil_Out32(0xf6165428, 0x00000006);
Xil_Out32(0xf6165628, 0x00000006);
Xil_Out32(0xf6165244, 0x00000006);
Xil_Out32(0xf6165444, 0x00000006);
Xil_Out32(0xf6165644, 0x00000006);
Xil_Out32(0xf616500c, 0xf9e8d7c6);
Xil_Out32(0xf616501c, 0x000000b0);
Xil_Out32(0xf6165020, 0x0000020f);
Xil_Out32(0xf6165034, 0x00001420);
Xil_Out32(0xf6165058, 0x0000003f);
Xil_Out32(0xf616504c, 0x00000100);
Xil_Out32(0xf6165048, 0x00000010);
Xil_Out32(0xf6165004, 0x000011fe);
Xil_Out32(0xf6165028, 0x00000000);
Xil_Out32(0xf61650dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf616620c, 0xf9e8d7c6);
Xil_Out32(0xf616640c, 0xf9e8d7c6);
Xil_Out32(0xf616660c, 0xf9e8d7c6);
Xil_Out32(0xf6166228, 0x00000006);
Xil_Out32(0xf6166428, 0x00000006);
Xil_Out32(0xf6166628, 0x00000006);
Xil_Out32(0xf6166244, 0x00000006);
Xil_Out32(0xf6166444, 0x00000006);
Xil_Out32(0xf6166644, 0x00000006);
Xil_Out32(0xf616600c, 0xf9e8d7c6);
Xil_Out32(0xf616601c, 0x000000b3);
Xil_Out32(0xf6166020, 0x0000020b);
Xil_Out32(0xf6166034, 0x00001420);
Xil_Out32(0xf616602c, 0x00000102);
Xil_Out32(0xf6166058, 0x0000003d);
Xil_Out32(0xf616604c, 0x00000100);
Xil_Out32(0xf6166048, 0x00000010);
Xil_Out32(0xf6166004, 0x000011fe);
Xil_Out32(0xf6166028, 0x00000000);
Xil_Out32(0xf61660dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf616720c, 0xf9e8d7c6);
Xil_Out32(0xf616740c, 0xf9e8d7c6);
Xil_Out32(0xf616760c, 0xf9e8d7c6);
Xil_Out32(0xf6167228, 0x00000006);
Xil_Out32(0xf6167428, 0x00000006);
Xil_Out32(0xf6167628, 0x00000006);
Xil_Out32(0xf6167244, 0x00000006);
Xil_Out32(0xf6167444, 0x00000006);
Xil_Out32(0xf6167644, 0x00000006);
Xil_Out32(0xf616700c, 0xf9e8d7c6);
Xil_Out32(0xf616701c, 0x000000b0);
Xil_Out32(0xf6167020, 0x0000020f);
Xil_Out32(0xf6167034, 0x00001420);
Xil_Out32(0xf6167058, 0x0000003f);
Xil_Out32(0xf616704c, 0x00000100);
Xil_Out32(0xf6167048, 0x00000010);
Xil_Out32(0xf6167004, 0x000011fe);
Xil_Out32(0xf6167028, 0x00000000);
Xil_Out32(0xf61670dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf616820c, 0xf9e8d7c6);
Xil_Out32(0xf616840c, 0xf9e8d7c6);
Xil_Out32(0xf616860c, 0xf9e8d7c6);
Xil_Out32(0xf6168228, 0x00000006);
Xil_Out32(0xf6168428, 0x00000006);
Xil_Out32(0xf6168628, 0x00000006);
Xil_Out32(0xf6168244, 0x00000006);
Xil_Out32(0xf6168444, 0x00000006);
Xil_Out32(0xf6168644, 0x00000006);
Xil_Out32(0xf616800c, 0xf9e8d7c6);
Xil_Out32(0xf6168020, 0x000002fc);
Xil_Out32(0xf6168034, 0x00001420);
Xil_Out32(0xf616802c, 0x0000013b);
Xil_Out32(0xf6168058, 0x00000008);
Xil_Out32(0xf616804c, 0x00000100);
Xil_Out32(0xf6168040, 0x0000cf3f);
Xil_Out32(0xf6168048, 0x00000010);
Xil_Out32(0xf6168004, 0x000011fe);
Xil_Out32(0xf6168028, 0x00000000);
Xil_Out32(0xf61680dc, 0x00000004);
//****************************************************
//XPIO_2
//****************************************************
Xil_Out32(0xf61c900c, 0xf9e8d7c6);
Xil_Out32(0xf61c9000, 0x000011c2);
Xil_Out32(0xf61c9004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf61c020c, 0xf9e8d7c6);
Xil_Out32(0xf61c040c, 0xf9e8d7c6);
Xil_Out32(0xf61c060c, 0xf9e8d7c6);
Xil_Out32(0xf61c0228, 0x00000006);
Xil_Out32(0xf61c0428, 0x00000006);
Xil_Out32(0xf61c0628, 0x00000006);
Xil_Out32(0xf61c0244, 0x00000006);
Xil_Out32(0xf61c0444, 0x00000006);
Xil_Out32(0xf61c0644, 0x00000006);
Xil_Out32(0xf61c000c, 0xf9e8d7c6);
Xil_Out32(0xf61c001c, 0x000000b0);
Xil_Out32(0xf61c0020, 0x0000020f);
Xil_Out32(0xf61c0034, 0x00001420);
Xil_Out32(0xf61c0058, 0x0000003f);
Xil_Out32(0xf61c004c, 0x00000100);
Xil_Out32(0xf61c0048, 0x00000010);
Xil_Out32(0xf61c0004, 0x000011fe);
Xil_Out32(0xf61c0028, 0x00000000);
Xil_Out32(0xf61c00dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf61c120c, 0xf9e8d7c6);
Xil_Out32(0xf61c140c, 0xf9e8d7c6);
Xil_Out32(0xf61c160c, 0xf9e8d7c6);
Xil_Out32(0xf61c1228, 0x00000006);
Xil_Out32(0xf61c1428, 0x00000006);
Xil_Out32(0xf61c1628, 0x00000006);
Xil_Out32(0xf61c1244, 0x00000006);
Xil_Out32(0xf61c1444, 0x00000006);
Xil_Out32(0xf61c1644, 0x00000006);
Xil_Out32(0xf61c100c, 0xf9e8d7c6);
Xil_Out32(0xf61c101c, 0x000000b3);
Xil_Out32(0xf61c1020, 0x0000020b);
Xil_Out32(0xf61c1034, 0x00001420);
Xil_Out32(0xf61c102c, 0x00000102);
Xil_Out32(0xf61c1058, 0x0000003d);
Xil_Out32(0xf61c104c, 0x00000100);
Xil_Out32(0xf61c1048, 0x00000010);
Xil_Out32(0xf61c1004, 0x000011fe);
Xil_Out32(0xf61c1028, 0x00000000);
Xil_Out32(0xf61c10dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf61c220c, 0xf9e8d7c6);
Xil_Out32(0xf61c240c, 0xf9e8d7c6);
Xil_Out32(0xf61c260c, 0xf9e8d7c6);
Xil_Out32(0xf61c2228, 0x00000006);
Xil_Out32(0xf61c2428, 0x00000006);
Xil_Out32(0xf61c2628, 0x00000006);
Xil_Out32(0xf61c2244, 0x00000006);
Xil_Out32(0xf61c2444, 0x00000006);
Xil_Out32(0xf61c2644, 0x00000006);
Xil_Out32(0xf61c200c, 0xf9e8d7c6);
Xil_Out32(0xf61c201c, 0x000000b3);
Xil_Out32(0xf61c2020, 0x0000020b);
Xil_Out32(0xf61c2034, 0x00001420);
Xil_Out32(0xf61c202c, 0x00000102);
Xil_Out32(0xf61c2058, 0x0000003d);
Xil_Out32(0xf61c204c, 0x00000100);
Xil_Out32(0xf61c2048, 0x00000010);
Xil_Out32(0xf61c2004, 0x000011fe);
Xil_Out32(0xf61c2028, 0x00000000);
Xil_Out32(0xf61c20dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf61c320c, 0xf9e8d7c6);
Xil_Out32(0xf61c340c, 0xf9e8d7c6);
Xil_Out32(0xf61c360c, 0xf9e8d7c6);
Xil_Out32(0xf61c3228, 0x00000006);
Xil_Out32(0xf61c3428, 0x00000006);
Xil_Out32(0xf61c3628, 0x00000006);
Xil_Out32(0xf61c3244, 0x00000006);
Xil_Out32(0xf61c3444, 0x00000006);
Xil_Out32(0xf61c3644, 0x00000006);
Xil_Out32(0xf61c300c, 0xf9e8d7c6);
Xil_Out32(0xf61c301c, 0x000000b0);
Xil_Out32(0xf61c3020, 0x0000020f);
Xil_Out32(0xf61c3034, 0x00001420);
Xil_Out32(0xf61c3058, 0x0000003f);
Xil_Out32(0xf61c304c, 0x00000100);
Xil_Out32(0xf61c3048, 0x00000010);
Xil_Out32(0xf61c3004, 0x000011fe);
Xil_Out32(0xf61c3028, 0x00000000);
Xil_Out32(0xf61c30dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf61c420c, 0xf9e8d7c6);
Xil_Out32(0xf61c440c, 0xf9e8d7c6);
Xil_Out32(0xf61c460c, 0xf9e8d7c6);
Xil_Out32(0xf61c4228, 0x00000006);
Xil_Out32(0xf61c4428, 0x00000006);
Xil_Out32(0xf61c4628, 0x00000006);
Xil_Out32(0xf61c4244, 0x00000006);
Xil_Out32(0xf61c4444, 0x00000006);
Xil_Out32(0xf61c4644, 0x00000006);
Xil_Out32(0xf61c400c, 0xf9e8d7c6);
Xil_Out32(0xf61c401c, 0x000000b3);
Xil_Out32(0xf61c4020, 0x0000020b);
Xil_Out32(0xf61c4034, 0x00001420);
Xil_Out32(0xf61c402c, 0x00000102);
Xil_Out32(0xf61c4058, 0x0000003d);
Xil_Out32(0xf61c404c, 0x00000100);
Xil_Out32(0xf61c4048, 0x00000010);
Xil_Out32(0xf61c4004, 0x000011fe);
Xil_Out32(0xf61c4028, 0x00000000);
Xil_Out32(0xf61c40dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf61c520c, 0xf9e8d7c6);
Xil_Out32(0xf61c540c, 0xf9e8d7c6);
Xil_Out32(0xf61c560c, 0xf9e8d7c6);
Xil_Out32(0xf61c5228, 0x00000006);
Xil_Out32(0xf61c5428, 0x00000006);
Xil_Out32(0xf61c5628, 0x00000006);
Xil_Out32(0xf61c5244, 0x00000006);
Xil_Out32(0xf61c5444, 0x00000006);
Xil_Out32(0xf61c5644, 0x00000006);
Xil_Out32(0xf61c500c, 0xf9e8d7c6);
Xil_Out32(0xf61c501c, 0x000000b0);
Xil_Out32(0xf61c5020, 0x0000020f);
Xil_Out32(0xf61c5034, 0x00001420);
Xil_Out32(0xf61c5058, 0x0000003f);
Xil_Out32(0xf61c504c, 0x00000100);
Xil_Out32(0xf61c5048, 0x00000010);
Xil_Out32(0xf61c5004, 0x000011fe);
Xil_Out32(0xf61c5028, 0x00000000);
Xil_Out32(0xf61c50dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf61c620c, 0xf9e8d7c6);
Xil_Out32(0xf61c640c, 0xf9e8d7c6);
Xil_Out32(0xf61c660c, 0xf9e8d7c6);
Xil_Out32(0xf61c6228, 0x00000006);
Xil_Out32(0xf61c6428, 0x00000006);
Xil_Out32(0xf61c6628, 0x00000006);
Xil_Out32(0xf61c6244, 0x00000006);
Xil_Out32(0xf61c6444, 0x00000006);
Xil_Out32(0xf61c6644, 0x00000006);
Xil_Out32(0xf61c600c, 0xf9e8d7c6);
Xil_Out32(0xf61c601c, 0x000000b3);
Xil_Out32(0xf61c6020, 0x0000020b);
Xil_Out32(0xf61c6034, 0x00001420);
Xil_Out32(0xf61c602c, 0x0000013f);
Xil_Out32(0xf61c6058, 0x0000003d);
Xil_Out32(0xf61c604c, 0x00000100);
Xil_Out32(0xf61c6048, 0x00000010);
Xil_Out32(0xf61c6004, 0x000011fe);
Xil_Out32(0xf61c6028, 0x00000000);
Xil_Out32(0xf61c60dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf61c720c, 0xf9e8d7c6);
Xil_Out32(0xf61c740c, 0xf9e8d7c6);
Xil_Out32(0xf61c760c, 0xf9e8d7c6);
Xil_Out32(0xf61c7228, 0x00000006);
Xil_Out32(0xf61c7428, 0x00000006);
Xil_Out32(0xf61c7628, 0x00000006);
Xil_Out32(0xf61c7244, 0x00000006);
Xil_Out32(0xf61c7444, 0x00000006);
Xil_Out32(0xf61c7644, 0x00000006);
Xil_Out32(0xf61c700c, 0xf9e8d7c6);
Xil_Out32(0xf61c701c, 0x000000b0);
Xil_Out32(0xf61c7020, 0x0000020f);
Xil_Out32(0xf61c7034, 0x00001420);
Xil_Out32(0xf61c702c, 0x0000013f);
Xil_Out32(0xf61c7058, 0x0000003f);
Xil_Out32(0xf61c704c, 0x00000100);
Xil_Out32(0xf61c7048, 0x00000010);
Xil_Out32(0xf61c7004, 0x000011fe);
Xil_Out32(0xf61c7028, 0x00000000);
Xil_Out32(0xf61c70dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf61c820c, 0xf9e8d7c6);
Xil_Out32(0xf61c840c, 0xf9e8d7c6);
Xil_Out32(0xf61c860c, 0xf9e8d7c6);
Xil_Out32(0xf61c8228, 0x00000006);
Xil_Out32(0xf61c8428, 0x00000006);
Xil_Out32(0xf61c8628, 0x00000006);
Xil_Out32(0xf61c8244, 0x00000006);
Xil_Out32(0xf61c8444, 0x00000006);
Xil_Out32(0xf61c8644, 0x00000006);
Xil_Out32(0xf61c800c, 0xf9e8d7c6);
Xil_Out32(0xf61c8020, 0x000002fc);
Xil_Out32(0xf61c8034, 0x00001420);
Xil_Out32(0xf61c802c, 0x0000013f);
Xil_Out32(0xf61c804c, 0x00000100);
Xil_Out32(0xf61c8048, 0x00000010);
Xil_Out32(0xf61c8004, 0x000011fe);
Xil_Out32(0xf61c8028, 0x00000000);
Xil_Out32(0xf61c80dc, 0x00000004);
//****************************************************
//XPIO_0
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf6090000, 0x00000040);
Xil_Out32(0xf6090004, 0x000011be);
Xil_Out32(0xf6090000, 0x00000042);
Xil_Out32(0xf6090004, 0x000011bc);
Xil_Out32(0xf6090000, 0x00000142);
Xil_Out32(0xf6090004, 0x000010bc);
Xil_Out32(0xf6090000, 0x000001c2);
Xil_Out32(0xf6090004, 0x0000103c);
Xil_Out32(0xf6090200, 0x00000106);
Xil_Out32(0xf6090400, 0x00000106);
Xil_Out32(0xf6090600, 0x00000106);
Xil_Out32(0xf6090204, 0x000000f8);
Xil_Out32(0xf6090404, 0x000000f8);
Xil_Out32(0xf6090604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf6091000, 0x00000040);
Xil_Out32(0xf6091004, 0x000011be);
Xil_Out32(0xf6091000, 0x00000042);
Xil_Out32(0xf6091004, 0x000011bc);
Xil_Out32(0xf6091000, 0x00000142);
Xil_Out32(0xf6091004, 0x000010bc);
Xil_Out32(0xf6091000, 0x000001c2);
Xil_Out32(0xf6091004, 0x0000103c);
Xil_Out32(0xf6091200, 0x00000106);
Xil_Out32(0xf6091400, 0x00000106);
Xil_Out32(0xf6091600, 0x00000106);
Xil_Out32(0xf6091204, 0x000000f8);
Xil_Out32(0xf6091404, 0x000000f8);
Xil_Out32(0xf6091604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf6092000, 0x00000040);
Xil_Out32(0xf6092004, 0x000011be);
Xil_Out32(0xf6092000, 0x00000042);
Xil_Out32(0xf6092004, 0x000011bc);
Xil_Out32(0xf6092000, 0x00000142);
Xil_Out32(0xf6092004, 0x000010bc);
Xil_Out32(0xf6092000, 0x000001c2);
Xil_Out32(0xf6092004, 0x0000103c);
Xil_Out32(0xf6092200, 0x00000106);
Xil_Out32(0xf6092400, 0x00000106);
Xil_Out32(0xf6092600, 0x00000106);
Xil_Out32(0xf6092204, 0x000000f8);
Xil_Out32(0xf6092404, 0x000000f8);
Xil_Out32(0xf6092604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf6093000, 0x00000040);
Xil_Out32(0xf6093004, 0x000011be);
Xil_Out32(0xf6093000, 0x00000042);
Xil_Out32(0xf6093004, 0x000011bc);
Xil_Out32(0xf6093000, 0x00000142);
Xil_Out32(0xf6093004, 0x000010bc);
Xil_Out32(0xf6093000, 0x000001c2);
Xil_Out32(0xf6093004, 0x0000103c);
Xil_Out32(0xf6093200, 0x00000106);
Xil_Out32(0xf6093400, 0x00000106);
Xil_Out32(0xf6093600, 0x00000106);
Xil_Out32(0xf6093204, 0x000000f8);
Xil_Out32(0xf6093404, 0x000000f8);
Xil_Out32(0xf6093604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf6094000, 0x00000040);
Xil_Out32(0xf6094004, 0x000011be);
Xil_Out32(0xf6094000, 0x00000042);
Xil_Out32(0xf6094004, 0x000011bc);
Xil_Out32(0xf6094000, 0x00000142);
Xil_Out32(0xf6094004, 0x000010bc);
Xil_Out32(0xf6094000, 0x000001c2);
Xil_Out32(0xf6094004, 0x0000103c);
Xil_Out32(0xf6094200, 0x00000106);
Xil_Out32(0xf6094400, 0x00000106);
Xil_Out32(0xf6094600, 0x00000106);
Xil_Out32(0xf6094204, 0x000000f8);
Xil_Out32(0xf6094404, 0x000000f8);
Xil_Out32(0xf6094604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf6095000, 0x00000040);
Xil_Out32(0xf6095004, 0x000011be);
Xil_Out32(0xf6095000, 0x00000042);
Xil_Out32(0xf6095004, 0x000011bc);
Xil_Out32(0xf6095000, 0x00000142);
Xil_Out32(0xf6095004, 0x000010bc);
Xil_Out32(0xf6095000, 0x000001c2);
Xil_Out32(0xf6095004, 0x0000103c);
Xil_Out32(0xf6095200, 0x00000106);
Xil_Out32(0xf6095400, 0x00000106);
Xil_Out32(0xf6095600, 0x00000106);
Xil_Out32(0xf6095204, 0x000000f8);
Xil_Out32(0xf6095404, 0x000000f8);
Xil_Out32(0xf6095604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf6096000, 0x00000040);
Xil_Out32(0xf6096004, 0x000011be);
Xil_Out32(0xf6096000, 0x00000042);
Xil_Out32(0xf6096004, 0x000011bc);
Xil_Out32(0xf6096000, 0x00000142);
Xil_Out32(0xf6096004, 0x000010bc);
Xil_Out32(0xf6096000, 0x000001c2);
Xil_Out32(0xf6096004, 0x0000103c);
Xil_Out32(0xf6096200, 0x00000106);
Xil_Out32(0xf6096400, 0x00000106);
Xil_Out32(0xf6096600, 0x00000106);
Xil_Out32(0xf6096204, 0x000000f8);
Xil_Out32(0xf6096404, 0x000000f8);
Xil_Out32(0xf6096604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf6097000, 0x00000040);
Xil_Out32(0xf6097004, 0x000011be);
Xil_Out32(0xf6097000, 0x00000042);
Xil_Out32(0xf6097004, 0x000011bc);
Xil_Out32(0xf6097000, 0x00000142);
Xil_Out32(0xf6097004, 0x000010bc);
Xil_Out32(0xf6097000, 0x000001c2);
Xil_Out32(0xf6097004, 0x0000103c);
Xil_Out32(0xf6097200, 0x00000106);
Xil_Out32(0xf6097400, 0x00000106);
Xil_Out32(0xf6097600, 0x00000106);
Xil_Out32(0xf6097204, 0x000000f8);
Xil_Out32(0xf6097404, 0x000000f8);
Xil_Out32(0xf6097604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf6098000, 0x00000040);
Xil_Out32(0xf6098004, 0x000011be);
Xil_Out32(0xf6098000, 0x00000042);
Xil_Out32(0xf6098004, 0x000011bc);
Xil_Out32(0xf6098000, 0x00000142);
Xil_Out32(0xf6098004, 0x000010bc);
Xil_Out32(0xf6098000, 0x000001c2);
Xil_Out32(0xf6098004, 0x0000103c);
Xil_Out32(0xf6098200, 0x00000106);
Xil_Out32(0xf6098400, 0x00000106);
Xil_Out32(0xf6098600, 0x00000106);
Xil_Out32(0xf6098204, 0x000000f8);
Xil_Out32(0xf6098404, 0x000000f8);
Xil_Out32(0xf6098604, 0x000000f8);
//****************************************************
//XPIO_1
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf6160000, 0x00000040);
Xil_Out32(0xf6160004, 0x000011be);
Xil_Out32(0xf6160000, 0x00000042);
Xil_Out32(0xf6160004, 0x000011bc);
Xil_Out32(0xf6160000, 0x00000142);
Xil_Out32(0xf6160004, 0x000010bc);
Xil_Out32(0xf6160000, 0x000001c2);
Xil_Out32(0xf6160004, 0x0000103c);
Xil_Out32(0xf6160200, 0x00000106);
Xil_Out32(0xf6160400, 0x00000106);
Xil_Out32(0xf6160600, 0x00000106);
Xil_Out32(0xf6160204, 0x000000f8);
Xil_Out32(0xf6160404, 0x000000f8);
Xil_Out32(0xf6160604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf6161000, 0x00000040);
Xil_Out32(0xf6161004, 0x000011be);
Xil_Out32(0xf6161000, 0x00000042);
Xil_Out32(0xf6161004, 0x000011bc);
Xil_Out32(0xf6161000, 0x00000142);
Xil_Out32(0xf6161004, 0x000010bc);
Xil_Out32(0xf6161000, 0x000001c2);
Xil_Out32(0xf6161004, 0x0000103c);
Xil_Out32(0xf6161200, 0x00000106);
Xil_Out32(0xf6161400, 0x00000106);
Xil_Out32(0xf6161600, 0x00000106);
Xil_Out32(0xf6161204, 0x000000f8);
Xil_Out32(0xf6161404, 0x000000f8);
Xil_Out32(0xf6161604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf6162000, 0x00000040);
Xil_Out32(0xf6162004, 0x000011be);
Xil_Out32(0xf6162000, 0x00000042);
Xil_Out32(0xf6162004, 0x000011bc);
Xil_Out32(0xf6162000, 0x00000142);
Xil_Out32(0xf6162004, 0x000010bc);
Xil_Out32(0xf6162000, 0x000001c2);
Xil_Out32(0xf6162004, 0x0000103c);
Xil_Out32(0xf6162200, 0x00000106);
Xil_Out32(0xf6162400, 0x00000106);
Xil_Out32(0xf6162600, 0x00000106);
Xil_Out32(0xf6162204, 0x000000f8);
Xil_Out32(0xf6162404, 0x000000f8);
Xil_Out32(0xf6162604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf6163000, 0x00000040);
Xil_Out32(0xf6163004, 0x000011be);
Xil_Out32(0xf6163000, 0x00000042);
Xil_Out32(0xf6163004, 0x000011bc);
Xil_Out32(0xf6163000, 0x00000142);
Xil_Out32(0xf6163004, 0x000010bc);
Xil_Out32(0xf6163000, 0x000001c2);
Xil_Out32(0xf6163004, 0x0000103c);
Xil_Out32(0xf6163200, 0x00000106);
Xil_Out32(0xf6163400, 0x00000106);
Xil_Out32(0xf6163600, 0x00000106);
Xil_Out32(0xf6163204, 0x000000f8);
Xil_Out32(0xf6163404, 0x000000f8);
Xil_Out32(0xf6163604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf6164000, 0x00000040);
Xil_Out32(0xf6164004, 0x000011be);
Xil_Out32(0xf6164000, 0x00000042);
Xil_Out32(0xf6164004, 0x000011bc);
Xil_Out32(0xf6164000, 0x00000142);
Xil_Out32(0xf6164004, 0x000010bc);
Xil_Out32(0xf6164000, 0x000001c2);
Xil_Out32(0xf6164004, 0x0000103c);
Xil_Out32(0xf6164200, 0x00000106);
Xil_Out32(0xf6164400, 0x00000106);
Xil_Out32(0xf6164600, 0x00000106);
Xil_Out32(0xf6164204, 0x000000f8);
Xil_Out32(0xf6164404, 0x000000f8);
Xil_Out32(0xf6164604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf6165000, 0x00000040);
Xil_Out32(0xf6165004, 0x000011be);
Xil_Out32(0xf6165000, 0x00000042);
Xil_Out32(0xf6165004, 0x000011bc);
Xil_Out32(0xf6165000, 0x00000142);
Xil_Out32(0xf6165004, 0x000010bc);
Xil_Out32(0xf6165000, 0x000001c2);
Xil_Out32(0xf6165004, 0x0000103c);
Xil_Out32(0xf6165200, 0x00000106);
Xil_Out32(0xf6165400, 0x00000106);
Xil_Out32(0xf6165600, 0x00000106);
Xil_Out32(0xf6165204, 0x000000f8);
Xil_Out32(0xf6165404, 0x000000f8);
Xil_Out32(0xf6165604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf6166000, 0x00000040);
Xil_Out32(0xf6166004, 0x000011be);
Xil_Out32(0xf6166000, 0x00000042);
Xil_Out32(0xf6166004, 0x000011bc);
Xil_Out32(0xf6166000, 0x00000142);
Xil_Out32(0xf6166004, 0x000010bc);
Xil_Out32(0xf6166000, 0x000001c2);
Xil_Out32(0xf6166004, 0x0000103c);
Xil_Out32(0xf6166200, 0x00000106);
Xil_Out32(0xf6166400, 0x00000106);
Xil_Out32(0xf6166600, 0x00000106);
Xil_Out32(0xf6166204, 0x000000f8);
Xil_Out32(0xf6166404, 0x000000f8);
Xil_Out32(0xf6166604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf6167000, 0x00000040);
Xil_Out32(0xf6167004, 0x000011be);
Xil_Out32(0xf6167000, 0x00000042);
Xil_Out32(0xf6167004, 0x000011bc);
Xil_Out32(0xf6167000, 0x00000142);
Xil_Out32(0xf6167004, 0x000010bc);
Xil_Out32(0xf6167000, 0x000001c2);
Xil_Out32(0xf6167004, 0x0000103c);
Xil_Out32(0xf6167200, 0x00000106);
Xil_Out32(0xf6167400, 0x00000106);
Xil_Out32(0xf6167600, 0x00000106);
Xil_Out32(0xf6167204, 0x000000f8);
Xil_Out32(0xf6167404, 0x000000f8);
Xil_Out32(0xf6167604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf6168000, 0x00000040);
Xil_Out32(0xf6168004, 0x000011be);
Xil_Out32(0xf6168000, 0x00000042);
Xil_Out32(0xf6168004, 0x000011bc);
Xil_Out32(0xf6168000, 0x00000142);
Xil_Out32(0xf6168004, 0x000010bc);
Xil_Out32(0xf6168000, 0x000001c2);
Xil_Out32(0xf6168004, 0x0000103c);
Xil_Out32(0xf6168200, 0x00000106);
Xil_Out32(0xf6168400, 0x00000106);
Xil_Out32(0xf6168600, 0x00000106);
Xil_Out32(0xf6168204, 0x000000f8);
Xil_Out32(0xf6168404, 0x000000f8);
Xil_Out32(0xf6168604, 0x000000f8);
//****************************************************
//XPIO_2
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf61c0000, 0x00000040);
Xil_Out32(0xf61c0004, 0x000011be);
Xil_Out32(0xf61c0000, 0x00000042);
Xil_Out32(0xf61c0004, 0x000011bc);
Xil_Out32(0xf61c0000, 0x00000142);
Xil_Out32(0xf61c0004, 0x000010bc);
Xil_Out32(0xf61c0000, 0x000001c2);
Xil_Out32(0xf61c0004, 0x0000103c);
Xil_Out32(0xf61c0200, 0x00000106);
Xil_Out32(0xf61c0400, 0x00000106);
Xil_Out32(0xf61c0600, 0x00000106);
Xil_Out32(0xf61c0204, 0x000000f8);
Xil_Out32(0xf61c0404, 0x000000f8);
Xil_Out32(0xf61c0604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf61c1000, 0x00000040);
Xil_Out32(0xf61c1004, 0x000011be);
Xil_Out32(0xf61c1000, 0x00000042);
Xil_Out32(0xf61c1004, 0x000011bc);
Xil_Out32(0xf61c1000, 0x00000142);
Xil_Out32(0xf61c1004, 0x000010bc);
Xil_Out32(0xf61c1000, 0x000001c2);
Xil_Out32(0xf61c1004, 0x0000103c);
Xil_Out32(0xf61c1200, 0x00000106);
Xil_Out32(0xf61c1400, 0x00000106);
Xil_Out32(0xf61c1600, 0x00000106);
Xil_Out32(0xf61c1204, 0x000000f8);
Xil_Out32(0xf61c1404, 0x000000f8);
Xil_Out32(0xf61c1604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf61c2000, 0x00000040);
Xil_Out32(0xf61c2004, 0x000011be);
Xil_Out32(0xf61c2000, 0x00000042);
Xil_Out32(0xf61c2004, 0x000011bc);
Xil_Out32(0xf61c2000, 0x00000142);
Xil_Out32(0xf61c2004, 0x000010bc);
Xil_Out32(0xf61c2000, 0x000001c2);
Xil_Out32(0xf61c2004, 0x0000103c);
Xil_Out32(0xf61c2200, 0x00000106);
Xil_Out32(0xf61c2400, 0x00000106);
Xil_Out32(0xf61c2600, 0x00000106);
Xil_Out32(0xf61c2204, 0x000000f8);
Xil_Out32(0xf61c2404, 0x000000f8);
Xil_Out32(0xf61c2604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf61c3000, 0x00000040);
Xil_Out32(0xf61c3004, 0x000011be);
Xil_Out32(0xf61c3000, 0x00000042);
Xil_Out32(0xf61c3004, 0x000011bc);
Xil_Out32(0xf61c3000, 0x00000142);
Xil_Out32(0xf61c3004, 0x000010bc);
Xil_Out32(0xf61c3000, 0x000001c2);
Xil_Out32(0xf61c3004, 0x0000103c);
Xil_Out32(0xf61c3200, 0x00000106);
Xil_Out32(0xf61c3400, 0x00000106);
Xil_Out32(0xf61c3600, 0x00000106);
Xil_Out32(0xf61c3204, 0x000000f8);
Xil_Out32(0xf61c3404, 0x000000f8);
Xil_Out32(0xf61c3604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf61c4000, 0x00000040);
Xil_Out32(0xf61c4004, 0x000011be);
Xil_Out32(0xf61c4000, 0x00000042);
Xil_Out32(0xf61c4004, 0x000011bc);
Xil_Out32(0xf61c4000, 0x00000142);
Xil_Out32(0xf61c4004, 0x000010bc);
Xil_Out32(0xf61c4000, 0x000001c2);
Xil_Out32(0xf61c4004, 0x0000103c);
Xil_Out32(0xf61c4200, 0x00000106);
Xil_Out32(0xf61c4400, 0x00000106);
Xil_Out32(0xf61c4600, 0x00000106);
Xil_Out32(0xf61c4204, 0x000000f8);
Xil_Out32(0xf61c4404, 0x000000f8);
Xil_Out32(0xf61c4604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf61c5000, 0x00000040);
Xil_Out32(0xf61c5004, 0x000011be);
Xil_Out32(0xf61c5000, 0x00000042);
Xil_Out32(0xf61c5004, 0x000011bc);
Xil_Out32(0xf61c5000, 0x00000142);
Xil_Out32(0xf61c5004, 0x000010bc);
Xil_Out32(0xf61c5000, 0x000001c2);
Xil_Out32(0xf61c5004, 0x0000103c);
Xil_Out32(0xf61c5200, 0x00000106);
Xil_Out32(0xf61c5400, 0x00000106);
Xil_Out32(0xf61c5600, 0x00000106);
Xil_Out32(0xf61c5204, 0x000000f8);
Xil_Out32(0xf61c5404, 0x000000f8);
Xil_Out32(0xf61c5604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf61c6000, 0x00000040);
Xil_Out32(0xf61c6004, 0x000011be);
Xil_Out32(0xf61c6000, 0x00000042);
Xil_Out32(0xf61c6004, 0x000011bc);
Xil_Out32(0xf61c6000, 0x00000142);
Xil_Out32(0xf61c6004, 0x000010bc);
Xil_Out32(0xf61c6000, 0x000001c2);
Xil_Out32(0xf61c6004, 0x0000103c);
Xil_Out32(0xf61c6200, 0x00000106);
Xil_Out32(0xf61c6400, 0x00000106);
Xil_Out32(0xf61c6600, 0x00000106);
Xil_Out32(0xf61c6204, 0x000000f8);
Xil_Out32(0xf61c6404, 0x000000f8);
Xil_Out32(0xf61c6604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf61c7000, 0x00000040);
Xil_Out32(0xf61c7004, 0x000011be);
Xil_Out32(0xf61c7000, 0x00000042);
Xil_Out32(0xf61c7004, 0x000011bc);
Xil_Out32(0xf61c7000, 0x00000142);
Xil_Out32(0xf61c7004, 0x000010bc);
Xil_Out32(0xf61c7000, 0x000001c2);
Xil_Out32(0xf61c7004, 0x0000103c);
Xil_Out32(0xf61c7200, 0x00000106);
Xil_Out32(0xf61c7400, 0x00000106);
Xil_Out32(0xf61c7600, 0x00000106);
Xil_Out32(0xf61c7204, 0x000000f8);
Xil_Out32(0xf61c7404, 0x000000f8);
Xil_Out32(0xf61c7604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf61c8000, 0x00000040);
Xil_Out32(0xf61c8004, 0x000011be);
Xil_Out32(0xf61c8000, 0x00000042);
Xil_Out32(0xf61c8004, 0x000011bc);
Xil_Out32(0xf61c8000, 0x00000142);
Xil_Out32(0xf61c8004, 0x000010bc);
Xil_Out32(0xf61c8000, 0x000001c2);
Xil_Out32(0xf61c8004, 0x0000103c);
Xil_Out32(0xf61c8200, 0x00000106);
Xil_Out32(0xf61c8400, 0x00000106);
Xil_Out32(0xf61c8600, 0x00000106);
Xil_Out32(0xf61c8204, 0x000000f8);
Xil_Out32(0xf61c8404, 0x000000f8);
Xil_Out32(0xf61c8604, 0x000000f8);
//*************************************************************************************************XPHY_CFG : end

//**************Register programming from  xphy_init end----------------

dbg0_pmc(16387);
//**************Register programming from  ddr_ub start----------------

dbg0_pmc(16388);
Xil_Out32(0xf6110000, 0x000000c2);
Xil_Out32(0xf6110004, 0x00000002);
Xil_Out32(0xf6110000, 0x01000000);
Xil_Out32(0xf6110004, 0x00000000);
prog_reg(0xF6110200, 0x2, 0x4, 0x1);
prog_reg(0xF6110200, 0x0, 0x3, 0x1);
prog_reg(0xF6110000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6110004, 0x00000000);
Xil_Out32(0xf6130000, 0xb0000002);
Xil_Out32(0xf6130004, 0xb8080050);
Xil_Out32(0xf6130008, 0xb0000002);
Xil_Out32(0xf613000c, 0xb80804ac);
Xil_Out32(0xf6130010, 0xb0000002);
Xil_Out32(0xf6130014, 0xb8080930);
Xil_Out32(0xf6130020, 0xb0000002);
Xil_Out32(0xf6130024, 0xb80804b0);
Xil_Out32(0xf6130050, 0xb0000002);
Xil_Out32(0xf6130054, 0x30201288);
Xil_Out32(0xf6130058, 0xb0000002);
Xil_Out32(0xf613005c, 0x31601298);
Xil_Out32(0xf6130060, 0x940bc802);
Xil_Out32(0xf6130064, 0xb0000002);
Xil_Out32(0xf6130068, 0x31600e98);
Xil_Out32(0xf613006c, 0x940bc800);
Xil_Out32(0xf6130070, 0x20c00000);
Xil_Out32(0xf6130074, 0x20e00000);
Xil_Out32(0xf6130078, 0xb0000000);
Xil_Out32(0xf613007c, 0xb9f403f8);
Xil_Out32(0xf6130080, 0x20a00000);
Xil_Out32(0xf6130084, 0xb9f40008);
Xil_Out32(0xf6130088, 0x30a30000);
Xil_Out32(0xf613008c, 0xb8000000);
Xil_Out32(0xf6130090, 0x3021fff4);
Xil_Out32(0xf6130094, 0xfa610008);
Xil_Out32(0xf6130098, 0x12610000);
Xil_Out32(0xf613009c, 0xf8b30004);
Xil_Out32(0xf61300a0, 0xe8730004);
Xil_Out32(0xf61300a4, 0xe8630000);
Xil_Out32(0xf61300a8, 0x10330000);
Xil_Out32(0xf61300ac, 0xea610008);
Xil_Out32(0xf61300b0, 0x3021000c);
Xil_Out32(0xf61300b4, 0xb60f0008);
Xil_Out32(0xf61300b8, 0x80000000);
Xil_Out32(0xf61300bc, 0x3021ffec);
Xil_Out32(0xf61300c0, 0xfa610010);
Xil_Out32(0xf61300c4, 0x12610000);
Xil_Out32(0xf61300c8, 0xf8b30008);
Xil_Out32(0xf61300cc, 0xf8d3000c);
Xil_Out32(0xf61300d0, 0xe8730008);
Xil_Out32(0xf61300d4, 0xf8730004);
Xil_Out32(0xf61300d8, 0xe8730004);
Xil_Out32(0xf61300dc, 0xe893000c);
Xil_Out32(0xf61300e0, 0xf8830000);
Xil_Out32(0xf61300e4, 0x80000000);
Xil_Out32(0xf61300e8, 0x10330000);
Xil_Out32(0xf61300ec, 0xea610010);
Xil_Out32(0xf61300f0, 0x30210014);
Xil_Out32(0xf61300f4, 0xb60f0008);
Xil_Out32(0xf61300f8, 0x80000000);
Xil_Out32(0xf61300fc, 0x3021ffec);
Xil_Out32(0xf6130100, 0xf9e10000);
Xil_Out32(0xf6130104, 0xfa610010);
Xil_Out32(0xf6130108, 0x12610000);
Xil_Out32(0xf613010c, 0x30c0003c);
Xil_Out32(0xf6130110, 0xb0000008);
Xil_Out32(0xf6130114, 0x30a06c00);
Xil_Out32(0xf6130118, 0xb9f4ffa4);
Xil_Out32(0xf613011c, 0x80000000);
Xil_Out32(0xf6130120, 0x30c0003c);
Xil_Out32(0xf6130124, 0xb0000008);
Xil_Out32(0xf6130128, 0x30a06c04);
Xil_Out32(0xf613012c, 0xb9f4ff90);
Xil_Out32(0xf6130130, 0x80000000);
Xil_Out32(0xf6130134, 0x30c0003c);
Xil_Out32(0xf6130138, 0xb0000008);
Xil_Out32(0xf613013c, 0x30a07000);
Xil_Out32(0xf6130140, 0xb9f4ff7c);
Xil_Out32(0xf6130144, 0x80000000);
Xil_Out32(0xf6130148, 0x30c0003c);
Xil_Out32(0xf613014c, 0xb0000008);
Xil_Out32(0xf6130150, 0x30a07004);
Xil_Out32(0xf6130154, 0xb9f4ff68);
Xil_Out32(0xf6130158, 0x80000000);
Xil_Out32(0xf613015c, 0x30c0003c);
Xil_Out32(0xf6130160, 0xb0000008);
Xil_Out32(0xf6130164, 0x30a07400);
Xil_Out32(0xf6130168, 0xb9f4ff54);
Xil_Out32(0xf613016c, 0x80000000);
Xil_Out32(0xf6130170, 0x30c0003c);
Xil_Out32(0xf6130174, 0xb0000008);
Xil_Out32(0xf6130178, 0x30a07404);
Xil_Out32(0xf613017c, 0xb9f4ff40);
Xil_Out32(0xf6130180, 0x80000000);
Xil_Out32(0xf6130184, 0x30c0003c);
Xil_Out32(0xf6130188, 0xb0000008);
Xil_Out32(0xf613018c, 0x30a07800);
Xil_Out32(0xf6130190, 0xb9f4ff2c);
Xil_Out32(0xf6130194, 0x80000000);
Xil_Out32(0xf6130198, 0x30c0003c);
Xil_Out32(0xf613019c, 0xb0000008);
Xil_Out32(0xf61301a0, 0x30a07804);
Xil_Out32(0xf61301a4, 0xb9f4ff18);
Xil_Out32(0xf61301a8, 0x80000000);
Xil_Out32(0xf61301ac, 0x30c0003c);
Xil_Out32(0xf61301b0, 0xb0000008);
Xil_Out32(0xf61301b4, 0x30a07c00);
Xil_Out32(0xf61301b8, 0xb9f4ff04);
Xil_Out32(0xf61301bc, 0x80000000);
Xil_Out32(0xf61301c0, 0x30c0003c);
Xil_Out32(0xf61301c4, 0xb0000008);
Xil_Out32(0xf61301c8, 0x30a07c04);
Xil_Out32(0xf61301cc, 0xb9f4fef0);
Xil_Out32(0xf61301d0, 0x80000000);
Xil_Out32(0xf61301d4, 0x30c0003c);
Xil_Out32(0xf61301d8, 0xb0000008);
Xil_Out32(0xf61301dc, 0x30a08000);
Xil_Out32(0xf61301e0, 0xb9f4fedc);
Xil_Out32(0xf61301e4, 0x80000000);
Xil_Out32(0xf61301e8, 0x30c0003c);
Xil_Out32(0xf61301ec, 0xb0000008);
Xil_Out32(0xf61301f0, 0x30a08004);
Xil_Out32(0xf61301f4, 0xb9f4fec8);
Xil_Out32(0xf61301f8, 0x80000000);
Xil_Out32(0xf61301fc, 0x30c0ffff);
Xil_Out32(0xf6130200, 0xb000000a);
Xil_Out32(0xf6130204, 0x30a00000);
Xil_Out32(0xf6130208, 0xb9f4feb4);
Xil_Out32(0xf613020c, 0x80000000);
Xil_Out32(0xf6130210, 0x30c0ffff);
Xil_Out32(0xf6130214, 0xb000000a);
Xil_Out32(0xf6130218, 0x30a00004);
Xil_Out32(0xf613021c, 0xb9f4fea0);
Xil_Out32(0xf6130220, 0x80000000);
Xil_Out32(0xf6130224, 0x30c0ffff);
Xil_Out32(0xf6130228, 0xb000000a);
Xil_Out32(0xf613022c, 0x30a00008);
Xil_Out32(0xf6130230, 0xb9f4fe8c);
Xil_Out32(0xf6130234, 0x80000000);
Xil_Out32(0xf6130238, 0x30c0ffff);
Xil_Out32(0xf613023c, 0xb000000a);
Xil_Out32(0xf6130240, 0x30a0000c);
Xil_Out32(0xf6130244, 0xb9f4fe78);
Xil_Out32(0xf6130248, 0x80000000);
Xil_Out32(0xf613024c, 0x30c0ffff);
Xil_Out32(0xf6130250, 0xb000000a);
Xil_Out32(0xf6130254, 0x30a00010);
Xil_Out32(0xf6130258, 0xb9f4fe64);
Xil_Out32(0xf613025c, 0x80000000);
Xil_Out32(0xf6130260, 0x30c0ffff);
Xil_Out32(0xf6130264, 0xb000000a);
Xil_Out32(0xf6130268, 0x30a00014);
Xil_Out32(0xf613026c, 0xb9f4fe50);
Xil_Out32(0xf6130270, 0x80000000);
Xil_Out32(0xf6130274, 0x30c0ffff);
Xil_Out32(0xf6130278, 0xb000000a);
Xil_Out32(0xf613027c, 0x30a00018);
Xil_Out32(0xf6130280, 0xb9f4fe3c);
Xil_Out32(0xf6130284, 0x80000000);
Xil_Out32(0xf6130288, 0x30c0ffff);
Xil_Out32(0xf613028c, 0xb000000a);
Xil_Out32(0xf6130290, 0x30a0001c);
Xil_Out32(0xf6130294, 0xb9f4fe28);
Xil_Out32(0xf6130298, 0x80000000);
Xil_Out32(0xf613029c, 0xb0000009);
Xil_Out32(0xf61302a0, 0x30a0000c);
Xil_Out32(0xf61302a4, 0xb9f4fdec);
Xil_Out32(0xf61302a8, 0x80000000);
Xil_Out32(0xf61302ac, 0xf8730004);
Xil_Out32(0xf61302b0, 0xe8730004);
Xil_Out32(0xf61302b4, 0xa0630020);
Xil_Out32(0xf61302b8, 0xf8730008);
Xil_Out32(0xf61302bc, 0xe8d30008);
Xil_Out32(0xf61302c0, 0xb0000009);
Xil_Out32(0xf61302c4, 0x30a0000c);
Xil_Out32(0xf61302c8, 0xb9f4fdf4);
Xil_Out32(0xf61302cc, 0x80000000);
Xil_Out32(0xf61302d0, 0xe8d30004);
Xil_Out32(0xf61302d4, 0xb0000009);
Xil_Out32(0xf61302d8, 0x30a0000c);
Xil_Out32(0xf61302dc, 0xb9f4fde0);
Xil_Out32(0xf61302e0, 0x80000000);
Xil_Out32(0xf61302e4, 0xb0000009);
Xil_Out32(0xf61302e8, 0x30a000ac);
Xil_Out32(0xf61302ec, 0xb9f4fda4);
Xil_Out32(0xf61302f0, 0x80000000);
Xil_Out32(0xf61302f4, 0xf873000c);
Xil_Out32(0xf61302f8, 0xe873000c);
Xil_Out32(0xf61302fc, 0xa463fff7);
Xil_Out32(0xf6130300, 0xf873000c);
Xil_Out32(0xf6130304, 0xe8d3000c);
Xil_Out32(0xf6130308, 0xb0000009);
Xil_Out32(0xf613030c, 0x30a000ac);
Xil_Out32(0xf6130310, 0xb9f4fdac);
Xil_Out32(0xf6130314, 0x80000000);
Xil_Out32(0xf6130318, 0x30c0003c);
Xil_Out32(0xf613031c, 0xb0000008);
Xil_Out32(0xf6130320, 0x30a06c00);
Xil_Out32(0xf6130324, 0xb9f4fd98);
Xil_Out32(0xf6130328, 0x80000000);
Xil_Out32(0xf613032c, 0x10c00000);
Xil_Out32(0xf6130330, 0xb0000008);
Xil_Out32(0xf6130334, 0x30a06c04);
Xil_Out32(0xf6130338, 0xb9f4fd84);
Xil_Out32(0xf613033c, 0x80000000);
Xil_Out32(0xf6130340, 0x30c0003c);
Xil_Out32(0xf6130344, 0xb0000008);
Xil_Out32(0xf6130348, 0x30a07000);
Xil_Out32(0xf613034c, 0xb9f4fd70);
Xil_Out32(0xf6130350, 0x80000000);
Xil_Out32(0xf6130354, 0x10c00000);
Xil_Out32(0xf6130358, 0xb0000008);
Xil_Out32(0xf613035c, 0x30a07004);
Xil_Out32(0xf6130360, 0xb9f4fd5c);
Xil_Out32(0xf6130364, 0x80000000);
Xil_Out32(0xf6130368, 0x30c0003c);
Xil_Out32(0xf613036c, 0xb0000008);
Xil_Out32(0xf6130370, 0x30a07c00);
Xil_Out32(0xf6130374, 0xb9f4fd48);
Xil_Out32(0xf6130378, 0x80000000);
Xil_Out32(0xf613037c, 0x10c00000);
Xil_Out32(0xf6130380, 0xb0000008);
Xil_Out32(0xf6130384, 0x30a07c04);
Xil_Out32(0xf6130388, 0xb9f4fd34);
Xil_Out32(0xf613038c, 0x80000000);
Xil_Out32(0xf6130390, 0x30c0003c);
Xil_Out32(0xf6130394, 0xb0000008);
Xil_Out32(0xf6130398, 0x30a08000);
Xil_Out32(0xf613039c, 0xb9f4fd20);
Xil_Out32(0xf61303a0, 0x80000000);
Xil_Out32(0xf61303a4, 0x10c00000);
Xil_Out32(0xf61303a8, 0xb0000008);
Xil_Out32(0xf61303ac, 0x30a08004);
Xil_Out32(0xf61303b0, 0xb9f4fd0c);
Xil_Out32(0xf61303b4, 0x80000000);
Xil_Out32(0xf61303b8, 0x30c0003c);
Xil_Out32(0xf61303bc, 0xb0000008);
Xil_Out32(0xf61303c0, 0x30a07400);
Xil_Out32(0xf61303c4, 0xb9f4fcf8);
Xil_Out32(0xf61303c8, 0x80000000);
Xil_Out32(0xf61303cc, 0x10c00000);
Xil_Out32(0xf61303d0, 0xb0000008);
Xil_Out32(0xf61303d4, 0x30a07404);
Xil_Out32(0xf61303d8, 0xb9f4fce4);
Xil_Out32(0xf61303dc, 0x80000000);
Xil_Out32(0xf61303e0, 0x30c0003c);
Xil_Out32(0xf61303e4, 0xb0000008);
Xil_Out32(0xf61303e8, 0x30a07800);
Xil_Out32(0xf61303ec, 0xb9f4fcd0);
Xil_Out32(0xf61303f0, 0x80000000);
Xil_Out32(0xf61303f4, 0x10c00000);
Xil_Out32(0xf61303f8, 0xb0000008);
Xil_Out32(0xf61303fc, 0x30a07804);
Xil_Out32(0xf6130400, 0xb9f4fcbc);
Xil_Out32(0xf6130404, 0x80000000);
Xil_Out32(0xf6130408, 0xb000feed);
Xil_Out32(0xf613040c, 0x30c00001);
Xil_Out32(0xf6130410, 0xb0000001);
Xil_Out32(0xf6130414, 0x30a0fffc);
Xil_Out32(0xf6130418, 0xb9f4fca4);
Xil_Out32(0xf613041c, 0x80000000);
Xil_Out32(0xf6130420, 0x80000000);
Xil_Out32(0xf6130424, 0xe9e10000);
Xil_Out32(0xf6130428, 0x10330000);
Xil_Out32(0xf613042c, 0xea610010);
Xil_Out32(0xf6130430, 0x30210014);
Xil_Out32(0xf6130434, 0xb60f0008);
Xil_Out32(0xf6130438, 0x80000000);
Xil_Out32(0xf613043c, 0x3021ffe0);
Xil_Out32(0xf6130440, 0xf9e10000);
Xil_Out32(0xf6130444, 0xfa61001c);
Xil_Out32(0xf6130448, 0x12610000);
Xil_Out32(0xf613044c, 0x30a00009);
Xil_Out32(0xf6130450, 0xb9f4fcac);
Xil_Out32(0xf6130454, 0x80000000);
Xil_Out32(0xf6130458, 0x80000000);
Xil_Out32(0xf613045c, 0xe9e10000);
Xil_Out32(0xf6130460, 0x10330000);
Xil_Out32(0xf6130464, 0xea61001c);
Xil_Out32(0xf6130468, 0x30210020);
Xil_Out32(0xf613046c, 0xb60f0008);
Xil_Out32(0xf6130470, 0x80000000);
Xil_Out32(0xf6130474, 0x3021ffe0);
Xil_Out32(0xf6130478, 0xf9e10000);
Xil_Out32(0xf613047c, 0xfa61001c);
Xil_Out32(0xf6130480, 0x12610000);
Xil_Out32(0xf6130484, 0xb000ffff);
Xil_Out32(0xf6130488, 0xb9f4ffb4);
Xil_Out32(0xf613048c, 0x80000000);
Xil_Out32(0xf6130490, 0x10600000);
Xil_Out32(0xf6130494, 0xe9e10000);
Xil_Out32(0xf6130498, 0x10330000);
Xil_Out32(0xf613049c, 0xea61001c);
Xil_Out32(0xf61304a0, 0x30210020);
Xil_Out32(0xf61304a4, 0xb60f0008);
Xil_Out32(0xf61304a8, 0x80000000);
prog_reg(0xF6110200, 0x2, 0x4, 0x0);
prog_reg(0xF6110200, 0x0, 0x3, 0x0);
prog_reg(0xF6070758, 0x0, 0x1, 0x1);
dbg0_pmc(16392);
poll_for(0xf612fffc, 0xffffffff, 0x00000000, 0xfeed0001);
dbg0_pmc(16393);
//**************Register programming from  ddr_ub end----------------

dbg0_pmc(16389);
//**************Register programming from ddrmc_init start----------------

dbg0_pmc(16390);
Xil_Out32(0xf6070034, 0x00000000); //reg_adec0
Xil_Out32(0xf6070038, 0x00000000); //reg_adec1
Xil_Out32(0xf607003c, 0x00100080); //reg_adec2
Xil_Out32(0xf6070040, 0x00000078); //reg_adec3
Xil_Out32(0xf6070044, 0x00000000); //reg_adec4
Xil_Out32(0xf6070048, 0x15513491); //reg_adec5
Xil_Out32(0xf607004c, 0x1a6585d6); //reg_adec6
Xil_Out32(0xf6070050, 0x1f79d71b); //reg_adec7
Xil_Out32(0xf6070054, 0x03000020); //reg_adec8
Xil_Out32(0xf6070058, 0x09207144); //reg_adec9
Xil_Out32(0xf607005c, 0x0f34c2ca); //reg_adec10
Xil_Out32(0xf6070060, 0x0000e190); //reg_adec11
Xil_Out32(0xf6070444, 0x00100401); //reg_qos1
Xil_Out32(0xf6070448, 0x00000401); //reg_qos2
Xil_Out32(0xf60704b0, 0x01084210); //reg_cmdq_ctrl1
Xil_Out32(0xf6150200, 0x1a410404); //reg_safe_config0
Xil_Out32(0xf6150204, 0x0081c207); //reg_safe_config1
Xil_Out32(0xf6150208, 0x4252c3cf); //reg_safe_config2
Xil_Out32(0xf615020c, 0x02044899); //reg_safe_config3
Xil_Out32(0xf6150210, 0x250050e6); //reg_safe_config4
Xil_Out32(0xf6150214, 0x00c830c0); //reg_safe_config6
Xil_Out32(0xf6150218, 0x00040230); //reg_safe_config7
Xil_Out32(0xf615021c, 0x00000200); //reg_safe_config8
Xil_Out32(0xf6150220, 0x08ca4855); //reg_retry_0
Xil_Out32(0xf6150224, 0x145f6ac0); //reg_retry_1
Xil_Out32(0xf615022c, 0x00009002); //reg_ref_3
Xil_Out32(0xf6150230, 0x00014248); //reg_com_3
Xil_Out32(0xf6150234, 0x0020079e); //reg_mrs_0
Xil_Out32(0xf615023c, 0x00000000); //reg_mrs_7
Xil_Out32(0xf615024c, 0x1001001d); //reg_rd_config
Xil_Out32(0xf6150250, 0x00006071); //reg_pt_config
Xil_Out32(0xf6150258, 0x00400210); //reg_config0
Xil_Out32(0xf615025c, 0x00000009); //reg_pinout
Xil_Out32(0xf6150268, 0x0000000b); //seq_init_cmd_valid
Xil_Out32(0xf615026c, 0x0001901f); //seq_init_cmd0
Xil_Out32(0xf6150270, 0x0002585f); //seq_init_cmd1
Xil_Out32(0xf6150274, 0x0004807f); //seq_init_cmd2
Xil_Out32(0xf6150278, 0x00001468); //seq_init_cmd3
Xil_Out32(0xf615027c, 0x00001468); //seq_init_cmd4
Xil_Out32(0xf6150280, 0x00001468); //seq_init_cmd5
Xil_Out32(0xf6150284, 0x00001468); //seq_init_cmd6
Xil_Out32(0xf6150288, 0x00001468); //seq_init_cmd7
Xil_Out32(0xf615028c, 0x00001468); //seq_init_cmd8
Xil_Out32(0xf6150290, 0x00003068); //seq_init_cmd9
Xil_Out32(0xf6150294, 0x0008006e); //seq_init_cmd10
Xil_Out32(0xf6150298, 0x00000268); //seq_init_cmd11
Xil_Out32(0xf615029c, 0x00001468); //seq_init_cmd12
Xil_Out32(0xf61502a0, 0x00000268); //seq_init_cmd13
Xil_Out32(0xf61502a4, 0x00001468); //seq_init_cmd14
Xil_Out32(0xf61502a8, 0x00000268); //seq_init_cmd15
Xil_Out32(0xf61502ac, 0x00003068); //seq_init_cmd16
Xil_Out32(0xf61502b0, 0x0000026e); //seq_init_cmd17
Xil_Out32(0xf61502b4, 0x0008006e); //seq_init_cmd18
Xil_Out32(0xf61502b8, 0x00000000); //seq_init_cmd19
Xil_Out32(0xf61502bc, 0x00000000); //seq_init_cmd20
Xil_Out32(0xf61502c0, 0x00000000); //seq_init_cmd21
Xil_Out32(0xf61502c4, 0x00000000); //seq_init_cmd22
Xil_Out32(0xf61502c8, 0x00000000); //seq_init_cmd23
Xil_Out32(0xf61502cc, 0x00000000); //seq_init_cmd24
Xil_Out32(0xf61502d0, 0x00000000); //seq_init_cmd25
Xil_Out32(0xf61502d4, 0x00000000); //seq_init_cmd26
Xil_Out32(0xf61502d8, 0x00000000); //seq_init_cmd27
Xil_Out32(0xf61502dc, 0x00000000); //seq_init_cmd28
Xil_Out32(0xf61502e0, 0x00000000); //seq_init_cmd29
Xil_Out32(0xf61502e4, 0x00000000); //seq_init_cmd30
Xil_Out32(0xf61502ec, 0x0000007f); //seq_init_cmd_set
Xil_Out32(0xf61502f0, 0x00000000); //seq_init_addr0
Xil_Out32(0xf61502f4, 0x00000000); //seq_init_addr1
Xil_Out32(0xf61502f8, 0x00000000); //seq_init_addr2
Xil_Out32(0xf61502fc, 0x03000400); //seq_init_addr3
Xil_Out32(0xf6150300, 0x06001000); //seq_init_addr4
Xil_Out32(0xf6150304, 0x050004c0); //seq_init_addr5
Xil_Out32(0xf6150308, 0x04000000); //seq_init_addr6
Xil_Out32(0xf615030c, 0x02000028); //seq_init_addr7
Xil_Out32(0xf6150310, 0x01000001); //seq_init_addr8
Xil_Out32(0xf6150314, 0x00000d50); //seq_init_addr9
Xil_Out32(0xf6150318, 0x00000524); //seq_init_addr10
Xil_Out32(0xf615031c, 0x02000028); //seq_init_addr11
Xil_Out32(0xf6150320, 0x02000028); //seq_init_addr12
Xil_Out32(0xf6150324, 0x01000001); //seq_init_addr13
Xil_Out32(0xf6150328, 0x01000001); //seq_init_addr14
Xil_Out32(0xf615032c, 0x00000d50); //seq_init_addr15
Xil_Out32(0xf6150330, 0x00000d50); //seq_init_addr16
Xil_Out32(0xf6150334, 0x00000524); //seq_init_addr17
Xil_Out32(0xf6150338, 0x00000524); //seq_init_addr18
Xil_Out32(0xf615033c, 0x00000000); //seq_init_addr19
Xil_Out32(0xf6150340, 0x00000000); //seq_init_addr20
Xil_Out32(0xf6150344, 0x00000000); //seq_init_addr21
Xil_Out32(0xf6150348, 0x00000000); //seq_init_addr22
Xil_Out32(0xf615034c, 0x00000000); //seq_init_addr23
Xil_Out32(0xf6150350, 0x00000000); //seq_init_addr24
Xil_Out32(0xf6150354, 0x00000000); //seq_init_addr25
Xil_Out32(0xf6150358, 0x00000000); //seq_init_addr26
Xil_Out32(0xf615035c, 0x00000000); //seq_init_addr27
Xil_Out32(0xf6150360, 0x00000000); //seq_init_addr28
Xil_Out32(0xf6150364, 0x00000000); //seq_init_addr29
Xil_Out32(0xf6150368, 0x00000000); //seq_init_addr30
Xil_Out32(0xf6150378, 0x00000001); //seq_init_cntrl0
Xil_Out32(0xf615037c, 0x00000001); //seq_init_cntrl1
Xil_Out32(0xf6150380, 0x00000000); //seq_init_cntrl2
Xil_Out32(0xf6150384, 0x00000010); //seq_init_cntrl3
Xil_Out32(0xf6150388, 0x00000010); //seq_init_cntrl4
Xil_Out32(0xf615038c, 0x00000010); //seq_init_cntrl5
Xil_Out32(0xf6150390, 0x00000010); //seq_init_cntrl6
Xil_Out32(0xf6150394, 0x00000010); //seq_init_cntrl7
Xil_Out32(0xf6150398, 0x00000010); //seq_init_cntrl8
Xil_Out32(0xf615039c, 0x00000010); //seq_init_cntrl9
Xil_Out32(0xf61503a0, 0x00000010); //seq_init_cntrl10
Xil_Out32(0xf61503a4, 0x00000010); //seq_init_cntrl11
Xil_Out32(0xf61503a8, 0x00000010); //seq_init_cntrl12
Xil_Out32(0xf61503ac, 0x00000010); //seq_init_cntrl13
Xil_Out32(0xf61503b0, 0x00000010); //seq_init_cntrl14
Xil_Out32(0xf61503b4, 0x00000010); //seq_init_cntrl15
Xil_Out32(0xf61503b8, 0x00000010); //seq_init_cntrl16
Xil_Out32(0xf61503bc, 0x00000010); //seq_init_cntrl17
Xil_Out32(0xf61503c0, 0x00000010); //seq_init_cntrl18
Xil_Out32(0xf61503c4, 0x00000010); //seq_init_cntrl19
Xil_Out32(0xf61503c8, 0x00000010); //seq_init_cntrl20
Xil_Out32(0xf61503cc, 0x00000010); //seq_init_cntrl21
Xil_Out32(0xf61503d0, 0x00000010); //seq_init_cntrl22
Xil_Out32(0xf61503d4, 0x00000010); //seq_init_cntrl23
Xil_Out32(0xf61503d8, 0x00000010); //seq_init_cntrl24
Xil_Out32(0xf61503dc, 0x00000010); //seq_init_cntrl25
Xil_Out32(0xf61503e0, 0x00000010); //seq_init_cntrl26
Xil_Out32(0xf61503e4, 0x00000010); //seq_init_cntrl27
Xil_Out32(0xf61503e8, 0x00000010); //seq_init_cntrl28
Xil_Out32(0xf61503ec, 0x00000010); //seq_init_cntrl29
Xil_Out32(0xf61503f0, 0x00000010); //seq_init_cntrl30
Xil_Out32(0xf6150800, 0x00001016); //reg_safe_config5
Xil_Out32(0xf6150820, 0x00000001); //reg_scrub9
Xil_Out32(0xf6150824, 0x00000006); //reg_config1
Xil_Out32(0xf615082c, 0x00000001); //cal_mode
Xil_Out32(0xf6150830, 0x00000016); //phy_rden0
Xil_Out32(0xf6150834, 0x00000016); //phy_rden1
Xil_Out32(0xf6150838, 0x00000016); //phy_rden2
Xil_Out32(0xf615083c, 0x00000016); //phy_rden3
Xil_Out32(0xf6150840, 0x00000016); //phy_rden4
Xil_Out32(0xf6150844, 0x00000016); //phy_rden5
Xil_Out32(0xf6150848, 0x00000016); //phy_rden6
Xil_Out32(0xf615084c, 0x00000016); //phy_rden7
Xil_Out32(0xf6150850, 0x00000016); //phy_rden8
Xil_Out32(0xf6150854, 0x00000016); //phy_rden9
Xil_Out32(0xf6150858, 0x00000016); //phy_rden10
Xil_Out32(0xf615085c, 0x00000016); //phy_rden11
Xil_Out32(0xf6150860, 0x00000016); //phy_rden12
Xil_Out32(0xf6150864, 0x00000016); //phy_rden13
Xil_Out32(0xf6150868, 0x00000016); //phy_rden14
Xil_Out32(0xf615086c, 0x00000016); //phy_rden15
Xil_Out32(0xf6150870, 0x00000016); //phy_rden16
Xil_Out32(0xf6150874, 0x00000016); //phy_rden17
Xil_Out32(0xf6150878, 0x00000016); //phy_rden18
Xil_Out32(0xf615087c, 0x00000016); //phy_rden19
Xil_Out32(0xf6150880, 0x00000016); //phy_rden20
Xil_Out32(0xf6150884, 0x00000016); //phy_rden21
Xil_Out32(0xf6150888, 0x00000016); //phy_rden22
Xil_Out32(0xf615088c, 0x00000016); //phy_rden23
Xil_Out32(0xf6150890, 0x00000016); //phy_rden24
Xil_Out32(0xf6150894, 0x00000016); //phy_rden25
Xil_Out32(0xf6150898, 0x00000016); //phy_rden26
Xil_Out32(0xf615089c, 0x0000000f); //fifo_rden
Xil_Out32(0xf61508ac, 0x00000000); //seq_mode
Xil_Out32(0xf61508b0, 0x0000007f); //seq_cmd_default
Xil_Out32(0xf61508d0, 0x00000000); //seq_ck_cal
Xil_Out32(0xf6150944, 0x00000410); //xpi_oe_all_nib
Xil_Out32(0xf6150948, 0x00000410); //xpi_wrdata_all_nib
Xil_Out32(0xf615094c, 0x00000101); //xpi_oe
Xil_Out32(0xf6150950, 0x000000c0); //xpi_pmi_config
Xil_Out32(0xf6150958, 0x00000001); //xpi_write_dm_dbi
Xil_Out32(0xf6150960, 0x000000a5); //default_pattern
Xil_Out32(0xf6150964, 0x00000000); //t_txbit
Xil_Out32(0xf6150c0c, 0x0000005f); //xpi_dqs
//**************Register programming from ddrmc_init end----------------

dbg0_pmc(16391);
Xil_Out32(0xf6150968, 0x00000640);

dbg0_pmc(16402);
prog_reg(0xF6150E3C, 0x0, 0x1, 0x1);
dbg0_pmc(16396);
poll_for(0xf6150e40, 0x00000001, 0x00000000, 0x00000001);
dbg0_pmc(16397);
dbg0_pmc(16399);
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// msoc_driver.c ver. 1.24, Jun 17 2018 , 18:47:42 ********

//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ---- INPUT PARAMETERS ----
// sim mode    = 1
// ecc         = 0
// bus width   = 64
// dram width  = 8
// memory type = DDR4
// speed bin   = 3200
// frequency   = 1600.000 MHz
// device capacity  = 8192 Mbit
// rank   addr cnt  = 0
// bank group  addr cnt  = 0
// bank   addr cnt  = 0
// row    addr cnt  = 0
// column addr cnt  = 10
// temp ctrl ref mode = 0
// temp ctrl ref range = 0
// CL   = 22 cycles
// CWL  = 16 cycles
// tRCD = 22 cycles
// tRP  = 22 cycles
// AL   = 0 cycles
// BL   = 8 (burst length)
// tRC  = 45.750 nsec
// tRAS = 32.000 nsec
// tFAW = 21.000 nsec
// clock_stop_en = 0
// rd_dbi = 0
// wr_dbi = 0
// ecc_scrub = 0
// ecc_poison = 0
// parity = 0
// ca_parity_latency = 0
// geardown = 0
// cal_mode_en = 0
// udimm = 0
// rdimm = 0
// addr_mirror = 0
// dimm_addr_mirror = 0
// crc = 0
// brc_mapping = 0
// wr_preamble = 0
// rd_preamble = 0
// wr_postamble = 0
// rd_postamble = 0

Xil_Out32(0xf6150cbc, 0x00000002);
Xil_Out32(0xf6110000, 0x000000fe);
Xil_Out32(0xf6110004, 0x00000d00);


}
#endif


#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80) || defined(XPMCFW_HW90)
void msoc_hsr_init()
{

unsigned int rdata;

Xil_Out32(0xf615000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0.ddrmc_main_bank.pcsr_lock
Xil_Out32(0xf611000c, 0xf9e8d7c6); //reg_model_global.u_ddrmc_0_ub.ddrmc_ub_bank.ddrmc_pcsr_lock
dbg0_pmc(16398);
//**************Register programming from noc_cfg start----------------
dbg0_pmc(16384);
// 105:NOC_CONFIG selected : noc_cfg_ddr_vnoc_noitlv.c
Xil_Out32(0xf600000c, 0xf9e8d7c6);
Xil_Out32(0xf6000000, 0x00000002);
Xil_Out32(0xf6000004, 0x00000000);
Xil_Out32(0xf6000100, 0x39852310);
Xil_Out32(0xf600000c, 0x00000000);
Xil_Out32(0xf606000c, 0xf9e8d7c6);
Xil_Out32(0xf6060000, 0x0000023e);
Xil_Out32(0xf6060004, 0x00000200);
Xil_Out32(0xf60603a0, 0x00000030);
Xil_Out32(0xf6060110, 0xfff0aaaa);
Xil_Out32(0xf6060114, 0x55500000);
Xil_Out32(0xf6060128, 0xfffaaaaa);
Xil_Out32(0xf606012c, 0x555a0000);
Xil_Out32(0xf6060154, 0x55500000);
Xil_Out32(0xf6060180, 0xfffaaaaa);
Xil_Out32(0xf6060184, 0x555a0000);
Xil_Out32(0xf60601c0, 0xfffaaaaa);
Xil_Out32(0xf60601c4, 0x555a0000);
Xil_Out32(0xf6060320, 0xffffaafa);
Xil_Out32(0xf6060324, 0x555500f0);
Xil_Out32(0xf6060328, 0xffffaa5a);
Xil_Out32(0xf606032c, 0x55550050);
Xil_Out32(0xf606000c, 0x00000000);
Xil_Out32(0xf610000c, 0xf9e8d7c6);
Xil_Out32(0xf6100000, 0x0000023e);
Xil_Out32(0xf6100004, 0x00000200);
Xil_Out32(0xf61003a0, 0x00000050);
Xil_Out32(0xf6100118, 0xff0faaaa);
Xil_Out32(0xf6100184, 0x555a0000);
Xil_Out32(0xf61001c4, 0x555a0000);
Xil_Out32(0xf6100324, 0x555500f0);
Xil_Out32(0xf6100328, 0xffffaaa5);
Xil_Out32(0xf610000c, 0x00000000);
Xil_Out32(0xf603200c, 0xf9e8d7c6);
Xil_Out32(0xf6032000, 0x0000023e);
Xil_Out32(0xf6032004, 0x00000200);
Xil_Out32(0xf60323a0, 0x00000040);
Xil_Out32(0xf6032110, 0xffffaaa5);
Xil_Out32(0xf6032118, 0xff0faaaa);
Xil_Out32(0xf6032150, 0xffffaaa5);
Xil_Out32(0xf603200c, 0x00000000);
Xil_Out32(0xf610200c, 0xf9e8d7c6);
Xil_Out32(0xf6102000, 0x0000023e);
Xil_Out32(0xf6102004, 0x00000200);
Xil_Out32(0xf61023a0, 0x00000070);
Xil_Out32(0xf6102108, 0xff0faaaa);
Xil_Out32(0xf6102184, 0x555a0000);
Xil_Out32(0xf61021c4, 0x555a0000);
Xil_Out32(0xf6102324, 0x555500f0);
Xil_Out32(0xf6102328, 0xffffaaa5);
Xil_Out32(0xf610200c, 0x00000000);
Xil_Out32(0xf606200c, 0xf9e8d7c6);
Xil_Out32(0xf6062000, 0x0000023e);
Xil_Out32(0xf6062004, 0x00000200);
Xil_Out32(0xf60623a0, 0x00000010);
Xil_Out32(0xf6062110, 0xffffaaaf);
Xil_Out32(0xf6062138, 0xfffaaaaa);
Xil_Out32(0xf6062180, 0xfffaaaaa);
Xil_Out32(0xf60621c0, 0xfffaaaaa);
Xil_Out32(0xf6062324, 0x55550050);
Xil_Out32(0xf606232c, 0x55050000);
Xil_Out32(0xf606200c, 0x00000000);
Xil_Out32(0xf603000c, 0xf9e8d7c6);
Xil_Out32(0xf6030000, 0x0000023e);
Xil_Out32(0xf6030004, 0x00000200);
Xil_Out32(0xf60303a0, 0x00000020);
Xil_Out32(0xf6030108, 0xffffaafa);
Xil_Out32(0xf6030118, 0xffffaafa);
Xil_Out32(0xf603011c, 0x555500f0);
Xil_Out32(0xf6030154, 0x555a0000);
Xil_Out32(0xf6030180, 0xffafaaaa);
Xil_Out32(0xf60301c0, 0xffafaaaa);
Xil_Out32(0xf6030324, 0x55500000);
Xil_Out32(0xf603032c, 0x55550005);
Xil_Out32(0xf603000c, 0x00000000);
Xil_Out32(0xf617000c, 0xf9e8d7c6);
Xil_Out32(0xf6170000, 0x0000023e);
Xil_Out32(0xf6170004, 0x00000200);
Xil_Out32(0xf61703a0, 0x000000a0);
Xil_Out32(0xf6170118, 0xff0faaaa);
Xil_Out32(0xf6170124, 0x55500000);
Xil_Out32(0xf6170180, 0xffafaaaa);
Xil_Out32(0xf6170184, 0x555a0000);
Xil_Out32(0xf61701c0, 0xffafaaaa);
Xil_Out32(0xf61701c4, 0x555a0000);
Xil_Out32(0xf6170320, 0xffffaafa);
Xil_Out32(0xf6170324, 0x555500f0);
Xil_Out32(0xf6170328, 0xffffaaa5);
Xil_Out32(0xf617032c, 0x55550005);
Xil_Out32(0xf617000c, 0x00000000);
Xil_Out32(0xf6e6400c, 0xf9e8d7c6);
Xil_Out32(0xf6e64000, 0x0000023e);
Xil_Out32(0xf6e64004, 0x00000200);
Xil_Out32(0xf6e643a0, 0x00000180);
Xil_Out32(0xf6e64114, 0x555a0000);
Xil_Out32(0xf6e64124, 0x555a0000);
Xil_Out32(0xf6e64320, 0xffffaafa);
Xil_Out32(0xf6e64324, 0x555500f0);
Xil_Out32(0xf6e64328, 0xff0faaaa);
Xil_Out32(0xf6e64338, 0xff0faaaa);
Xil_Out32(0xf6e64348, 0xff0faaaa);
Xil_Out32(0xf6e64360, 0xffffaaa5);
Xil_Out32(0xf6e6436c, 0x55500000);
Xil_Out32(0xf6e6400c, 0x00000000);
Xil_Out32(0xf61d200c, 0xf9e8d7c6);
Xil_Out32(0xf61d2000, 0x0000023e);
Xil_Out32(0xf61d2004, 0x00000200);
Xil_Out32(0xf61d23a0, 0x000000c0);
Xil_Out32(0xf61d2100, 0xffffaaff);
Xil_Out32(0xf61d2104, 0x555500f0);
Xil_Out32(0xf61d2110, 0xffffaaa5);
Xil_Out32(0xf61d2120, 0xffffaaf5);
Xil_Out32(0xf61d2124, 0x555500f0);
Xil_Out32(0xf61d2130, 0xffffaa5a);
Xil_Out32(0xf61d2134, 0x55550050);
Xil_Out32(0xf61d2150, 0xffffaa55);
Xil_Out32(0xf61d2154, 0x55550050);
Xil_Out32(0xf61d2180, 0xff00aaaa);
Xil_Out32(0xf61d2184, 0x555a0000);
Xil_Out32(0xf61d2188, 0xff0faaaa);
Xil_Out32(0xf61d2190, 0xff0faaaa);
Xil_Out32(0xf61d21c0, 0xff0aaaaa);
Xil_Out32(0xf61d21c4, 0x550a0000);
Xil_Out32(0xf61d200c, 0x00000000);
Xil_Out32(0xf61d000c, 0xf9e8d7c6);
Xil_Out32(0xf61d0000, 0x0000023e);
Xil_Out32(0xf61d0004, 0x00000200);
Xil_Out32(0xf61d03a0, 0x000000c0);
Xil_Out32(0xf61d0114, 0x55500000);
Xil_Out32(0xf61d0124, 0x55500000);
Xil_Out32(0xf61d0134, 0x55050000);
Xil_Out32(0xf61d0154, 0x55000000);
Xil_Out32(0xf61d0180, 0xffffaaff);
Xil_Out32(0xf61d0188, 0xffffaafa);
Xil_Out32(0xf61d0190, 0xffffaafa);
Xil_Out32(0xf61d01c0, 0xffffaaff);
Xil_Out32(0xf61d000c, 0x00000000);
Xil_Out32(0xf60a000c, 0xf9e8d7c6);
Xil_Out32(0xf60a0000, 0x0000023e);
Xil_Out32(0xf60a0004, 0x00000200);
Xil_Out32(0xf60a03a0, 0x00000060);
Xil_Out32(0xf60a0124, 0x5555000f);
Xil_Out32(0xf60a0154, 0x55a50000);
Xil_Out32(0xf60a0180, 0xfffaaaaa);
Xil_Out32(0xf60a0184, 0x55a50000);
Xil_Out32(0xf60a018c, 0x55a50000);
Xil_Out32(0xf60a0194, 0x55a50000);
Xil_Out32(0xf60a019c, 0x55a50000);
Xil_Out32(0xf60a01a4, 0x55a50000);
Xil_Out32(0xf60a01ac, 0x55a50000);
Xil_Out32(0xf60a01c0, 0xfffaaaaa);
Xil_Out32(0xf60a01c4, 0x55a50000);
Xil_Out32(0xf60a01cc, 0x55a50000);
Xil_Out32(0xf60a0324, 0x55550050);
Xil_Out32(0xf60a000c, 0x00000000);
Xil_Out32(0xf6e9400c, 0xf9e8d7c6);
Xil_Out32(0xf6e94000, 0x0000023e);
Xil_Out32(0xf6e94004, 0x00000200);
Xil_Out32(0xf6e943a0, 0x00000180);
Xil_Out32(0xf6e94100, 0xffafaaaa);
Xil_Out32(0xf6e94104, 0x555a0000);
Xil_Out32(0xf6e94120, 0xffafaaaa);
Xil_Out32(0xf6e94124, 0x555a0000);
Xil_Out32(0xf6e94130, 0xffafaaaa);
Xil_Out32(0xf6e94150, 0xffafaaaa);
Xil_Out32(0xf6e94320, 0xffafaaaa);
Xil_Out32(0xf6e9432c, 0x555500f0);
Xil_Out32(0xf6e94330, 0xff0faaaa);
Xil_Out32(0xf6e94364, 0x555a0000);
Xil_Out32(0xf6e94368, 0xffffaaa5);
Xil_Out32(0xf6e9436c, 0x55550005);
Xil_Out32(0xf6e94374, 0x55550005);
Xil_Out32(0xf6e9437c, 0x55550005);
Xil_Out32(0xf6e9400c, 0x00000000);
Xil_Out32(0xf61d400c, 0xf9e8d7c6);
Xil_Out32(0xf61d4000, 0x0000023e);
Xil_Out32(0xf61d4004, 0x00000200);
Xil_Out32(0xf61d43a0, 0x000000c0);
Xil_Out32(0xf61d4100, 0xff00aaaa);
Xil_Out32(0xf61d4120, 0xff0faaaa);
Xil_Out32(0xf61d4180, 0xffffaaa5);
Xil_Out32(0xf61d41c0, 0xffffaa55);
Xil_Out32(0xf61d400c, 0x00000000);
Xil_Out32(0xf617200c, 0xf9e8d7c6);
Xil_Out32(0xf6172000, 0x0000023e);
Xil_Out32(0xf6172004, 0x00000200);
Xil_Out32(0xf6172130, 0xfff0aaaa);
Xil_Out32(0xf6172180, 0xfffaaaaa);
Xil_Out32(0xf61721c0, 0xfffaaaaa);
Xil_Out32(0xf61721c4, 0x55a50000);
Xil_Out32(0xf6172320, 0xffffaa5a);
Xil_Out32(0xf6172324, 0x55550050);
Xil_Out32(0xf617232c, 0x5555000f);
Xil_Out32(0xf617200c, 0x00000000);
Xil_Out32(0xf60a200c, 0xf9e8d7c6);
Xil_Out32(0xf60a2000, 0x0000023e);
Xil_Out32(0xf60a2004, 0x00000200);
Xil_Out32(0xf60a23a0, 0x00000040);
Xil_Out32(0xf60a2104, 0x55a50000);
Xil_Out32(0xf60a2134, 0x5555000f);
Xil_Out32(0xf60a2180, 0xfffaaaaa);
Xil_Out32(0xf60a21c0, 0xfffaaaaa);
Xil_Out32(0xf60a2324, 0x55550050);
Xil_Out32(0xf60a200c, 0x00000000);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf63b000c, 0xf9e8d7c6);
Xil_Out32(0xf63b0000, 0x0000023e);
Xil_Out32(0xf63b0004, 0x00000200);
Xil_Out32(0xf63b03a0, 0x000000c0);
Xil_Out32(0xf63b0124, 0x55500000);
Xil_Out32(0xf63b0198, 0xffffaafa);
Xil_Out32(0xf63b000c, 0x00000000);
#endif
Xil_Out32(0xf6ec600c, 0xf9e8d7c6);
Xil_Out32(0xf6ec6000, 0x0000023e);
Xil_Out32(0xf6ec6004, 0x00000200);
Xil_Out32(0xf6ec63a0, 0x00000180);
Xil_Out32(0xf6ec6104, 0x55050000);
Xil_Out32(0xf6ec610c, 0x55050000);
Xil_Out32(0xf6ec611c, 0x55050000);
Xil_Out32(0xf6ec6124, 0x55050000);
Xil_Out32(0xf6ec612c, 0x55050000);
Xil_Out32(0xf6ec6134, 0x55050000);
Xil_Out32(0xf6ec613c, 0x55050000);
Xil_Out32(0xf6ec6148, 0xfff0aaaa);
Xil_Out32(0xf6ec6154, 0x55050000);
Xil_Out32(0xf6ec6158, 0xfff0aaaa);
Xil_Out32(0xf6ec6180, 0xfffaaaaa);
Xil_Out32(0xf6ec6324, 0x55050000);
Xil_Out32(0xf6ec632c, 0x55050000);
Xil_Out32(0xf6ec6330, 0xffffaa5a);
Xil_Out32(0xf6ec6334, 0x55550050);
Xil_Out32(0xf6ec6338, 0xffffaa5a);
Xil_Out32(0xf6ec633c, 0x55a50000);
Xil_Out32(0xf6ec634c, 0x55a50000);
Xil_Out32(0xf6ec6360, 0xfff0aaaa);
Xil_Out32(0xf6ec6368, 0xfff0aaaa);
Xil_Out32(0xf6ec6370, 0xffffaaaf);
Xil_Out32(0xf6ec6374, 0x5555000f);
Xil_Out32(0xf6ec6378, 0xfffaaaaa);
Xil_Out32(0xf6ec600c, 0x00000000);
Xil_Out32(0xf620200c, 0xf9e8d7c6);
Xil_Out32(0xf6202000, 0x0000023e);
Xil_Out32(0xf6202004, 0x00000200);
Xil_Out32(0xf62023a0, 0x000000c0);
Xil_Out32(0xf620210c, 0x555500f0);
Xil_Out32(0xf620211c, 0x55550050);
Xil_Out32(0xf620212c, 0x55550050);
Xil_Out32(0xf620213c, 0x555500f0);
Xil_Out32(0xf620214c, 0x5555000f);
Xil_Out32(0xf620215c, 0x55550005);
Xil_Out32(0xf6202180, 0xffaaaaaa);
Xil_Out32(0xf6202184, 0x555a0000);
Xil_Out32(0xf62021c0, 0xffaaaaaa);
Xil_Out32(0xf62021c4, 0x55aa0000);
Xil_Out32(0xf620200c, 0x00000000);
Xil_Out32(0xf620000c, 0xf9e8d7c6);
Xil_Out32(0xf6200000, 0x0000023e);
Xil_Out32(0xf6200004, 0x00000200);
Xil_Out32(0xf62003a0, 0x000000c0);
Xil_Out32(0xf620011c, 0x55050000);
Xil_Out32(0xf620012c, 0x55050000);
Xil_Out32(0xf620015c, 0x55500000);
Xil_Out32(0xf6200180, 0xffffaaff);
Xil_Out32(0xf62001c0, 0xffffaaff);
Xil_Out32(0xf620000c, 0x00000000);
Xil_Out32(0xf61b000c, 0xf9e8d7c6);
Xil_Out32(0xf61b0000, 0x0000023e);
Xil_Out32(0xf61b0004, 0x00000200);
Xil_Out32(0xf61b03a0, 0x000000b0);
Xil_Out32(0xf61b0180, 0xffafaaaa);
Xil_Out32(0xf61b01c0, 0xffafaaaa);
Xil_Out32(0xf61b0324, 0x55550005);
Xil_Out32(0xf61b000c, 0x00000000);
Xil_Out32(0xf6ee400c, 0xf9e8d7c6);
Xil_Out32(0xf6ee4000, 0x0000023e);
Xil_Out32(0xf6ee4004, 0x00000200);
Xil_Out32(0xf6ee43a0, 0x00000180);
Xil_Out32(0xf6ee414c, 0x555a0000);
Xil_Out32(0xf6ee4338, 0xffffaafa);
Xil_Out32(0xf6ee433c, 0x555500f0);
Xil_Out32(0xf6ee4364, 0x555a0000);
Xil_Out32(0xf6ee436c, 0x555a0000);
Xil_Out32(0xf6ee438c, 0x55500000);
Xil_Out32(0xf6ee400c, 0x00000000);
Xil_Out32(0xf6ec400c, 0xf9e8d7c6);
Xil_Out32(0xf6ec4000, 0x0000023e);
Xil_Out32(0xf6ec4004, 0x00000200);
Xil_Out32(0xf6ec43a0, 0x00000180);
Xil_Out32(0xf6ec4148, 0xffffaaa5);
Xil_Out32(0xf6ec4330, 0xffffaafa);
Xil_Out32(0xf6ec4334, 0x555500f0);
Xil_Out32(0xf6ec4338, 0xff0faaaa);
Xil_Out32(0xf6ec4368, 0xffffaaa5);
Xil_Out32(0xf6ec436c, 0x555a0000);
Xil_Out32(0xf6ec438c, 0x55500000);
Xil_Out32(0xf6ec400c, 0x00000000);
Xil_Out32(0xf620400c, 0xf9e8d7c6);
Xil_Out32(0xf6204000, 0x0000023e);
Xil_Out32(0xf6204004, 0x00000200);
Xil_Out32(0xf62043a0, 0x000000c0);
Xil_Out32(0xf6204108, 0xff0faaaa);
Xil_Out32(0xf6204138, 0xff0faaaa);
Xil_Out32(0xf6204148, 0xfff0aaaa);
Xil_Out32(0xf6204180, 0xffffaaa5);
Xil_Out32(0xf62041c0, 0xffffaa55);
Xil_Out32(0xf620400c, 0x00000000);
Xil_Out32(0xf61b200c, 0xf9e8d7c6);
Xil_Out32(0xf61b2000, 0x0000023e);
Xil_Out32(0xf61b2004, 0x00000200);
Xil_Out32(0xf61b23a0, 0x00000090);
Xil_Out32(0xf61b21c4, 0x55a50000);
Xil_Out32(0xf61b2324, 0x5555000f);
Xil_Out32(0xf61b200c, 0x00000000);
Xil_Out32(0xf6f3400c, 0xf9e8d7c6);
Xil_Out32(0xf6f34000, 0x0000023e);
Xil_Out32(0xf6f34004, 0x00000200);
Xil_Out32(0xf6f343a0, 0x00000180);
Xil_Out32(0xf6f34150, 0xffafaaaa);
Xil_Out32(0xf6f34154, 0x555a0000);
Xil_Out32(0xf6f34158, 0xffffaaa5);
Xil_Out32(0xf6f34180, 0xff0faaaa);
Xil_Out32(0xf6f34330, 0xffafaaaa);
Xil_Out32(0xf6f3434c, 0x555500f0);
Xil_Out32(0xf6f34350, 0xff0faaaa);
Xil_Out32(0xf6f34360, 0xffffaaa5);
Xil_Out32(0xf6f34364, 0x555a0000);
Xil_Out32(0xf6f34368, 0xffffaaa5);
Xil_Out32(0xf6f34380, 0xffffaaa5);
Xil_Out32(0xf6f3438c, 0x55550005);
Xil_Out32(0xf6f3400c, 0x00000000);
Xil_Out32(0xf6e9600c, 0xf9e8d7c6);
Xil_Out32(0xf6e96000, 0x0000023e);
Xil_Out32(0xf6e96004, 0x00000200);
Xil_Out32(0xf6e963a0, 0x00000180);
Xil_Out32(0xf6e96104, 0x55550050);
Xil_Out32(0xf6e9610c, 0x55050000);
Xil_Out32(0xf6e9611c, 0x55050000);
Xil_Out32(0xf6e96124, 0x55550050);
Xil_Out32(0xf6e9612c, 0x55050000);
Xil_Out32(0xf6e96134, 0x55550050);
Xil_Out32(0xf6e9613c, 0x55050000);
Xil_Out32(0xf6e96154, 0x55550050);
Xil_Out32(0xf6e96180, 0xfffaaaaa);
Xil_Out32(0xf6e96184, 0x55a50000);
Xil_Out32(0xf6e9632c, 0x55550050);
Xil_Out32(0xf6e9633c, 0x55a50000);
Xil_Out32(0xf6e9634c, 0x55a50000);
Xil_Out32(0xf6e96368, 0xffffaaaf);
Xil_Out32(0xf6e9636c, 0x5555000f);
Xil_Out32(0xf6e96370, 0xfffaaaaa);
Xil_Out32(0xf6e96378, 0xfffaaaaa);
Xil_Out32(0xf6e96388, 0xfffaaaaa);
Xil_Out32(0xf6e9600c, 0x00000000);
Xil_Out32(0xf6e6600c, 0xf9e8d7c6);
Xil_Out32(0xf6e66000, 0x0000023e);
Xil_Out32(0xf6e66004, 0x00000200);
Xil_Out32(0xf6e663a0, 0x00000180);
Xil_Out32(0xf6e66104, 0x55050000);
Xil_Out32(0xf6e6610c, 0x55050000);
Xil_Out32(0xf6e6611c, 0x55050000);
Xil_Out32(0xf6e66124, 0x55050000);
Xil_Out32(0xf6e6612c, 0x55050000);
Xil_Out32(0xf6e66134, 0x55050000);
Xil_Out32(0xf6e6613c, 0x55050000);
Xil_Out32(0xf6e66154, 0x55050000);
Xil_Out32(0xf6e66184, 0x55a50000);
Xil_Out32(0xf6e66324, 0x55550050);
Xil_Out32(0xf6e66334, 0x55a50000);
Xil_Out32(0xf6e66360, 0xffffaaaf);
Xil_Out32(0xf6e66364, 0x5555000f);
Xil_Out32(0xf6e66370, 0xfffaaaaa);
Xil_Out32(0xf6e66378, 0xfffaaaaa);
Xil_Out32(0xf6e66388, 0xfffaaaaa);
Xil_Out32(0xf6e6600c, 0x00000000);
Xil_Out32(0xf6ee600c, 0xf9e8d7c6);
Xil_Out32(0xf6ee6000, 0x0000023e);
Xil_Out32(0xf6ee6004, 0x00000200);
Xil_Out32(0xf6ee63a0, 0x00000180);
Xil_Out32(0xf6ee6104, 0x55050000);
Xil_Out32(0xf6ee610c, 0x55050000);
Xil_Out32(0xf6ee611c, 0x55050000);
Xil_Out32(0xf6ee6124, 0x55050000);
Xil_Out32(0xf6ee612c, 0x55050000);
Xil_Out32(0xf6ee6134, 0x55050000);
Xil_Out32(0xf6ee613c, 0x55050000);
Xil_Out32(0xf6ee6154, 0x55050000);
Xil_Out32(0xf6ee6324, 0x55050000);
Xil_Out32(0xf6ee632c, 0x55050000);
Xil_Out32(0xf6ee6334, 0x55050000);
Xil_Out32(0xf6ee6338, 0xffffaa5a);
Xil_Out32(0xf6ee634c, 0x55a50000);
Xil_Out32(0xf6ee6370, 0xfff0aaaa);
Xil_Out32(0xf6ee6378, 0xffffaaaf);
Xil_Out32(0xf6ee637c, 0x5555000f);
Xil_Out32(0xf6ee600c, 0x00000000);
Xil_Out32(0xf6f3600c, 0xf9e8d7c6);
Xil_Out32(0xf6f36000, 0x0000023e);
Xil_Out32(0xf6f36004, 0x00000200);
Xil_Out32(0xf6f363a0, 0x00000180);
Xil_Out32(0xf6f36104, 0x55050000);
Xil_Out32(0xf6f3610c, 0x55050000);
Xil_Out32(0xf6f3611c, 0x55050000);
Xil_Out32(0xf6f36124, 0x55050000);
Xil_Out32(0xf6f3612c, 0x55050000);
Xil_Out32(0xf6f36134, 0x55050000);
Xil_Out32(0xf6f3613c, 0x55050000);
Xil_Out32(0xf6f36158, 0xfff0aaaa);
Xil_Out32(0xf6f36180, 0xffffaa5a);
Xil_Out32(0xf6f36324, 0x55050000);
Xil_Out32(0xf6f3632c, 0x55050000);
Xil_Out32(0xf6f3633c, 0x55050000);
Xil_Out32(0xf6f36348, 0xffffaa5a);
Xil_Out32(0xf6f36350, 0xffffaa5a);
Xil_Out32(0xf6f36360, 0xfff0aaaa);
Xil_Out32(0xf6f36368, 0xfff0aaaa);
Xil_Out32(0xf6f36370, 0xfff0aaaa);
Xil_Out32(0xf6f36378, 0xfff0aaaa);
Xil_Out32(0xf6f36380, 0xfff0aaaa);
Xil_Out32(0xf6f36388, 0xffffaaaf);
Xil_Out32(0xf6f3600c, 0x00000000);
Xil_Out32(0xf6f1400c, 0xf9e8d7c6);
Xil_Out32(0xf6f14000, 0x0000023e);
Xil_Out32(0xf6f14004, 0x00000200);
Xil_Out32(0xf6f143a0, 0x00000180);
Xil_Out32(0xf6f14338, 0xffafaaaa);
Xil_Out32(0xf6f1438c, 0x55550005);
Xil_Out32(0xf6f1400c, 0x00000000);
Xil_Out32(0xf6f1600c, 0xf9e8d7c6);
Xil_Out32(0xf6f16000, 0x0000023e);
Xil_Out32(0xf6f16004, 0x00000200);
Xil_Out32(0xf6f163a0, 0x00000180);
Xil_Out32(0xf6f16180, 0xfffaaaaa);
Xil_Out32(0xf6f16184, 0x55a50000);
Xil_Out32(0xf6f1633c, 0x55550050);
Xil_Out32(0xf6f16344, 0x55550050);
Xil_Out32(0xf6f16354, 0x55a50000);
Xil_Out32(0xf6f16384, 0x5555000f);
Xil_Out32(0xf6f16388, 0xfffaaaaa);
Xil_Out32(0xf6f16390, 0xfffaaaaa);
Xil_Out32(0xf6f1600c, 0x00000000);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6c9400c, 0xf9e8d7c6);
Xil_Out32(0xf6c94000, 0x0000023e);
Xil_Out32(0xf6c94004, 0x00000200);
Xil_Out32(0xf6c943a0, 0x00000190);
Xil_Out32(0xf6c94124, 0x555a0000);
Xil_Out32(0xf6c94194, 0x55500000);
Xil_Out32(0xf6c9419c, 0x55500000);
Xil_Out32(0xf6c94320, 0xffffaafa);
Xil_Out32(0xf6c94324, 0x555500f0);
Xil_Out32(0xf6c9400c, 0x00000000);
Xil_Out32(0xf62e200c, 0xf9e8d7c6);
Xil_Out32(0xf62e2000, 0x0000023e);
Xil_Out32(0xf62e2004, 0x00000200);
Xil_Out32(0xf62e23a0, 0x000000c0);
Xil_Out32(0xf62e2120, 0xffffaaa5);
Xil_Out32(0xf62e21c8, 0xff0faaaa);
Xil_Out32(0xf62e200c, 0x00000000);
Xil_Out32(0xf62e000c, 0xf9e8d7c6);
Xil_Out32(0xf62e0000, 0x0000023e);
Xil_Out32(0xf62e0004, 0x00000200);
Xil_Out32(0xf62e03a0, 0x000000c0);
Xil_Out32(0xf62e0124, 0x55500000);
Xil_Out32(0xf62e01c8, 0xffffaafa);
Xil_Out32(0xf62e000c, 0x00000000);
Xil_Out32(0xf6d4400c, 0xf9e8d7c6);
Xil_Out32(0xf6d44000, 0x0000023e);
Xil_Out32(0xf6d44004, 0x00000200);
Xil_Out32(0xf6d443a0, 0x00000190);
Xil_Out32(0xf6d44194, 0x55550005);
Xil_Out32(0xf6d44320, 0xffafaaaa);
Xil_Out32(0xf6d4400c, 0x00000000);
Xil_Out32(0xf6d4600c, 0xf9e8d7c6);
Xil_Out32(0xf6d46000, 0x0000023e);
Xil_Out32(0xf6d46004, 0x00000200);
Xil_Out32(0xf6d463a0, 0x00000190);
Xil_Out32(0xf6d46190, 0xfffaaaaa);
Xil_Out32(0xf6d46324, 0x55550050);
Xil_Out32(0xf6d4600c, 0x00000000);
Xil_Out32(0xf6cc400c, 0xf9e8d7c6);
Xil_Out32(0xf6cc4000, 0x0000023e);
Xil_Out32(0xf6cc4004, 0x00000200);
Xil_Out32(0xf6cc43a0, 0x00000190);
Xil_Out32(0xf6cc4124, 0x555a0000);
Xil_Out32(0xf6cc41a4, 0x55500000);
Xil_Out32(0xf6cc4328, 0xffffaafa);
Xil_Out32(0xf6cc432c, 0x555500f0);
Xil_Out32(0xf6cc400c, 0x00000000);
Xil_Out32(0xf6cc600c, 0xf9e8d7c6);
Xil_Out32(0xf6cc6000, 0x0000023e);
Xil_Out32(0xf6cc6004, 0x00000200);
Xil_Out32(0xf6cc63a0, 0x00000190);
Xil_Out32(0xf6cc61a8, 0xfffaaaaa);
Xil_Out32(0xf6cc632c, 0x55550050);
Xil_Out32(0xf6cc600c, 0x00000000);
Xil_Out32(0xf6f6600c, 0xf9e8d7c6);
Xil_Out32(0xf6f66000, 0x0000023e);
Xil_Out32(0xf6f66004, 0x00000200);
Xil_Out32(0xf6f663a0, 0x00000180);
Xil_Out32(0xf6f66180, 0xfffaaaaa);
Xil_Out32(0xf6f66344, 0x55050000);
Xil_Out32(0xf6f66354, 0x55550050);
Xil_Out32(0xf6f66390, 0xffffaaaf);
Xil_Out32(0xf6f6600c, 0x00000000);
Xil_Out32(0xf6f6400c, 0xf9e8d7c6);
Xil_Out32(0xf6f64000, 0x0000023e);
Xil_Out32(0xf6f64004, 0x00000200);
Xil_Out32(0xf6f643a0, 0x00000180);
Xil_Out32(0xf6f64180, 0xff0faaaa);
Xil_Out32(0xf6f64354, 0x555500f0);
Xil_Out32(0xf6f64384, 0x555a0000);
Xil_Out32(0xf6f64390, 0xffffaaa5);
Xil_Out32(0xf6f6400c, 0x00000000);
Xil_Out32(0xf63b200c, 0xf9e8d7c6);
Xil_Out32(0xf63b2000, 0x0000023e);
Xil_Out32(0xf63b2004, 0x00000200);
Xil_Out32(0xf63b23a0, 0x000000c0);
Xil_Out32(0xf63b2120, 0xffffaaa5);
Xil_Out32(0xf63b2198, 0xff0faaaa);
Xil_Out32(0xf63b200c, 0x00000000);
Xil_Out32(0xf64b200c, 0xf9e8d7c6);
Xil_Out32(0xf64b2000, 0x0000023e);
Xil_Out32(0xf64b2004, 0x00000200);
Xil_Out32(0xf64b23a0, 0x000000c0);
Xil_Out32(0xf64b2120, 0xffffaaa5);
Xil_Out32(0xf64b21a0, 0xff0faaaa);
Xil_Out32(0xf64b21a8, 0xff0faaaa);
Xil_Out32(0xf64b200c, 0x00000000);
Xil_Out32(0xf64b000c, 0xf9e8d7c6);
Xil_Out32(0xf64b0000, 0x0000023e);
Xil_Out32(0xf64b0004, 0x00000200);
Xil_Out32(0xf64b03a0, 0x000000c0);
Xil_Out32(0xf64b0124, 0x55500000);
Xil_Out32(0xf64b01a0, 0xffffaafa);
Xil_Out32(0xf64b01a8, 0xffffaafa);
Xil_Out32(0xf64b000c, 0x00000000);
#endif
Xil_Out32(0xf60e000c, 0xf9e8d7c6);
Xil_Out32(0xf60e0000, 0x01038000);
Xil_Out32(0xf60e0004, 0x01018000);
Xil_Out32(0xf60e02c8, 0x00003e40);
Xil_Out32(0xf60e086c, 0x00000003);
Xil_Out32(0xf60e0864, 0x00000001);
Xil_Out32(0xf60e0868, 0x00000001);
Xil_Out32(0xf60e0858, 0x00000002);
Xil_Out32(0xf60e0860, 0x00000002);
Xil_Out32(0xf60e083c, 0x00000100);
Xil_Out32(0xf60e0844, 0x00000100);
Xil_Out32(0xf60e0840, 0x00001fff);
Xil_Out32(0xf60e0848, 0x00001fff);
Xil_Out32(0xf60e03f0, 0x00001000);
Xil_Out32(0xf60e03f4, 0x00003000);
Xil_Out32(0xf60e03f8, 0x00003000);
Xil_Out32(0xf60e03fc, 0x00006000);
Xil_Out32(0xf60e0400, 0x00002000);
Xil_Out32(0xf60e0404, 0x00005000);
Xil_Out32(0xf60e0408, 0x00006000);
Xil_Out32(0xf60e03d0, 0x00000181);
Xil_Out32(0xf60e03d4, 0x00000181);
Xil_Out32(0xf60e03d8, 0x00000181);
Xil_Out32(0xf60e03dc, 0x00000181);
Xil_Out32(0xf60e03e0, 0x00000181);
Xil_Out32(0xf60e03e4, 0x00000181);
Xil_Out32(0xf60e03e8, 0x00000181);
Xil_Out32(0xf60e03ec, 0x00000181);
Xil_Out32(0xf60e0440, 0x00000409);
Xil_Out32(0xf60e0444, 0x00000b40);
Xil_Out32(0xf60e0448, 0x000004eb);
Xil_Out32(0xf60e044c, 0x00000511);
Xil_Out32(0xf60e0450, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000080);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000002);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x00000040);
Xil_Out32(0xf60e0004, 0x00000000);
Xil_Out32(0xf60e0000, 0x0000020c);
Xil_Out32(0xf60e0004, 0x00000200);
Xil_Out32(0xf60e000c, 0x00000000);
Xil_Out32(0xf60e200c, 0xf9e8d7c6);
Xil_Out32(0xf60e2000, 0x01038000);
Xil_Out32(0xf60e2004, 0x00028000);
Xil_Out32(0xf60e22c8, 0x00009818);
Xil_Out32(0xf60e286c, 0x00000003);
Xil_Out32(0xf60e2864, 0x00000001);
Xil_Out32(0xf60e2868, 0x00000001);
Xil_Out32(0xf60e2858, 0x00000002);
Xil_Out32(0xf60e2860, 0x00000002);
Xil_Out32(0xf60e283c, 0x00000100);
Xil_Out32(0xf60e2844, 0x00000100);
Xil_Out32(0xf60e2840, 0x00001fff);
Xil_Out32(0xf60e2848, 0x00001fff);
Xil_Out32(0xf60e23f0, 0x00005000);
Xil_Out32(0xf60e23f4, 0x00006000);
Xil_Out32(0xf60e23f8, 0x00003000);
Xil_Out32(0xf60e23fc, 0x00005000);
Xil_Out32(0xf60e2400, 0x00003000);
Xil_Out32(0xf60e2404, 0x00005000);
Xil_Out32(0xf60e2408, 0x00003000);
Xil_Out32(0xf60e23d0, 0x00000101);
Xil_Out32(0xf60e23d4, 0x00000101);
Xil_Out32(0xf60e23d8, 0x00000101);
Xil_Out32(0xf60e23dc, 0x00000101);
Xil_Out32(0xf60e23e0, 0x00000101);
Xil_Out32(0xf60e23e4, 0x00000101);
Xil_Out32(0xf60e23e8, 0x00000101);
Xil_Out32(0xf60e23ec, 0x00000101);
Xil_Out32(0xf60e2440, 0x00000409);
Xil_Out32(0xf60e2444, 0x000003ba);
Xil_Out32(0xf60e2448, 0x0000038c);
Xil_Out32(0xf60e244c, 0x000006cd);
Xil_Out32(0xf60e2450, 0x00000280);
Xil_Out32(0xf60e2000, 0x00000080);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x00000002);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x00000040);
Xil_Out32(0xf60e2004, 0x00000000);
Xil_Out32(0xf60e2000, 0x0000020c);
Xil_Out32(0xf60e2004, 0x00000200);
Xil_Out32(0xf60e200c, 0x00000000);
Xil_Out32(0xf601200c, 0xf9e8d7c6);
Xil_Out32(0xf6012000, 0x01038000);
Xil_Out32(0xf6012004, 0x01038000);
Xil_Out32(0xf60122c8, 0x00004f78);
Xil_Out32(0xf601286c, 0x00000003);
Xil_Out32(0xf6012864, 0x00000001);
Xil_Out32(0xf6012868, 0x00000001);
Xil_Out32(0xf6012858, 0x00000002);
Xil_Out32(0xf6012860, 0x00000002);
Xil_Out32(0xf601283c, 0x00000100);
Xil_Out32(0xf6012844, 0x00000100);
Xil_Out32(0xf6012840, 0x00001fff);
Xil_Out32(0xf6012848, 0x00001fff);
Xil_Out32(0xf60123f0, 0x00001000);
Xil_Out32(0xf60123f4, 0x00002000);
Xil_Out32(0xf60123f8, 0x00004000);
Xil_Out32(0xf60123fc, 0x00004000);
Xil_Out32(0xf6012400, 0x00002000);
Xil_Out32(0xf6012404, 0x00002000);
Xil_Out32(0xf6012408, 0x00002000);
Xil_Out32(0xf60123d0, 0x00000141);
Xil_Out32(0xf60123d4, 0x00000141);
Xil_Out32(0xf60123d8, 0x00000141);
Xil_Out32(0xf60123dc, 0x00000141);
Xil_Out32(0xf60123e0, 0x00000141);
Xil_Out32(0xf60123e4, 0x00000141);
Xil_Out32(0xf60123e8, 0x00000141);
Xil_Out32(0xf60123ec, 0x00000141);
Xil_Out32(0xf6012414, 0x00000081);
Xil_Out32(0xf6012418, 0x00000081);
Xil_Out32(0xf6012428, 0x0000089d);
Xil_Out32(0xf601242c, 0x000002d0);
Xil_Out32(0xf6012440, 0x00000409);
Xil_Out32(0xf6012444, 0x000002e9);
Xil_Out32(0xf6012448, 0x00000f6b);
Xil_Out32(0xf601244c, 0x0000024a);
Xil_Out32(0xf6012450, 0x000000c1);
Xil_Out32(0xf6012000, 0x00000080);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x00000002);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x00000040);
Xil_Out32(0xf6012004, 0x00000000);
Xil_Out32(0xf6012000, 0x0000020c);
Xil_Out32(0xf6012004, 0x00000200);
Xil_Out32(0xf601200c, 0x00000000);
Xil_Out32(0xf601600c, 0xf9e8d7c6);
Xil_Out32(0xf6016000, 0x01038000);
Xil_Out32(0xf6016004, 0x00028000);
Xil_Out32(0xf60162c8, 0x00001018);
Xil_Out32(0xf601686c, 0x00000003);
Xil_Out32(0xf6016864, 0x00000001);
Xil_Out32(0xf6016868, 0x00000001);
Xil_Out32(0xf6016858, 0x00000002);
Xil_Out32(0xf6016860, 0x00000002);
Xil_Out32(0xf601683c, 0x00000100);
Xil_Out32(0xf6016844, 0x00000100);
Xil_Out32(0xf6016840, 0x00001fff);
Xil_Out32(0xf6016848, 0x00001fff);
Xil_Out32(0xf60163f0, 0x00004000);
Xil_Out32(0xf60163f4, 0x00006000);
Xil_Out32(0xf60163f8, 0x00006000);
Xil_Out32(0xf60163fc, 0x00002000);
Xil_Out32(0xf6016400, 0x00004000);
Xil_Out32(0xf6016404, 0x00003000);
Xil_Out32(0xf6016408, 0x00003000);
Xil_Out32(0xf60163d0, 0x000001c1);
Xil_Out32(0xf60163d4, 0x000001c1);
Xil_Out32(0xf60163d8, 0x000001c1);
Xil_Out32(0xf60163dc, 0x000001c1);
Xil_Out32(0xf60163e0, 0x000001c1);
Xil_Out32(0xf60163e4, 0x000001c1);
Xil_Out32(0xf60163e8, 0x000001c1);
Xil_Out32(0xf60163ec, 0x000001c1);
Xil_Out32(0xf6016414, 0x00000081);
Xil_Out32(0xf6016418, 0x00000081);
Xil_Out32(0xf6016428, 0x00000041);
Xil_Out32(0xf601642c, 0x00000bf3);
Xil_Out32(0xf6016440, 0x00000409);
Xil_Out32(0xf6016444, 0x0000011d);
Xil_Out32(0xf6016448, 0x00000cb0);
Xil_Out32(0xf601644c, 0x00000caa);
Xil_Out32(0xf6016450, 0x00000041);
Xil_Out32(0xf6016000, 0x00000080);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x00000002);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x00000040);
Xil_Out32(0xf6016004, 0x00000000);
Xil_Out32(0xf6016000, 0x0000020c);
Xil_Out32(0xf6016004, 0x00000200);
Xil_Out32(0xf601600c, 0x00000000);
Xil_Out32(0xf601000c, 0xf9e8d7c6);
Xil_Out32(0xf6010000, 0x01038000);
Xil_Out32(0xf6010004, 0x00010000);
Xil_Out32(0xf60102c8, 0x00005780);
Xil_Out32(0xf601086c, 0x00000003);
Xil_Out32(0xf6010864, 0x00000001);
Xil_Out32(0xf6010868, 0x00000001);
Xil_Out32(0xf6010858, 0x00000002);
Xil_Out32(0xf6010860, 0x00000002);
Xil_Out32(0xf601083c, 0x00000100);
Xil_Out32(0xf6010844, 0x00000100);
Xil_Out32(0xf6010840, 0x00001fff);
Xil_Out32(0xf6010848, 0x00001fff);
Xil_Out32(0xf60103f0, 0x00003000);
Xil_Out32(0xf60103f4, 0x00001000);
Xil_Out32(0xf60103f8, 0x00006000);
Xil_Out32(0xf60103fc, 0x00001000);
Xil_Out32(0xf6010400, 0x00003000);
Xil_Out32(0xf6010404, 0x00006000);
Xil_Out32(0xf6010408, 0x00002000);
Xil_Out32(0xf60103d0, 0x00000141);
Xil_Out32(0xf60103d4, 0x00000141);
Xil_Out32(0xf60103d8, 0x00000141);
Xil_Out32(0xf60103dc, 0x00000141);
Xil_Out32(0xf60103e0, 0x00000141);
Xil_Out32(0xf60103e4, 0x00000141);
Xil_Out32(0xf60103e8, 0x00000141);
Xil_Out32(0xf60103ec, 0x00000141);
Xil_Out32(0xf601040c, 0x00000281);
Xil_Out32(0xf6010410, 0x00000080);
Xil_Out32(0xf6010418, 0x00000534);
Xil_Out32(0xf6010428, 0x00000281);
Xil_Out32(0xf601042c, 0x00000b7c);
Xil_Out32(0xf601041c, 0x00000281);
Xil_Out32(0xf6010420, 0x00000a02);
Xil_Out32(0xf6010424, 0x00000c88);
Xil_Out32(0xf6010440, 0x00000408);
Xil_Out32(0xf6010444, 0x00000162);
Xil_Out32(0xf6010448, 0x000005f5);
Xil_Out32(0xf601044c, 0x00000154);
Xil_Out32(0xf6010450, 0x000000c0);
Xil_Out32(0xf6010000, 0x00000080);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x00000002);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x00000040);
Xil_Out32(0xf6010004, 0x00000000);
Xil_Out32(0xf6010000, 0x0000020c);
Xil_Out32(0xf6010004, 0x00000200);
Xil_Out32(0xf601000c, 0x00000000);
Xil_Out32(0xf601400c, 0xf9e8d7c6);
Xil_Out32(0xf6014000, 0x01038000);
Xil_Out32(0xf6014004, 0x01020000);
Xil_Out32(0xf60142c8, 0x0000f4d8);
Xil_Out32(0xf601486c, 0x00000003);
Xil_Out32(0xf6014864, 0x00000001);
Xil_Out32(0xf6014868, 0x00000001);
Xil_Out32(0xf6014858, 0x00000002);
Xil_Out32(0xf6014860, 0x00000002);
Xil_Out32(0xf601483c, 0x00000100);
Xil_Out32(0xf6014844, 0x00000100);
Xil_Out32(0xf6014840, 0x00001fff);
Xil_Out32(0xf6014848, 0x00001fff);
Xil_Out32(0xf60143f0, 0x00006000);
Xil_Out32(0xf60143f4, 0x00002000);
Xil_Out32(0xf60143f8, 0x00005000);
Xil_Out32(0xf60143fc, 0x00003000);
Xil_Out32(0xf6014400, 0x00005000);
Xil_Out32(0xf6014404, 0x00006000);
Xil_Out32(0xf6014408, 0x00001000);
Xil_Out32(0xf60143d0, 0x000001c1);
Xil_Out32(0xf60143d4, 0x000001c1);
Xil_Out32(0xf60143d8, 0x000001c1);
Xil_Out32(0xf60143dc, 0x000001c1);
Xil_Out32(0xf60143e0, 0x000001c1);
Xil_Out32(0xf60143e4, 0x000001c1);
Xil_Out32(0xf60143e8, 0x000001c1);
Xil_Out32(0xf60143ec, 0x000001c1);
Xil_Out32(0xf601440c, 0x00000080);
Xil_Out32(0xf6014410, 0x00000080);
Xil_Out32(0xf6014418, 0x00000eea);
Xil_Out32(0xf6014428, 0x00000080);
Xil_Out32(0xf601442c, 0x00000bf3);
Xil_Out32(0xf601441c, 0x00000080);
Xil_Out32(0xf6014420, 0x00000368);
Xil_Out32(0xf6014424, 0x00000338);
Xil_Out32(0xf6014440, 0x00000409);
Xil_Out32(0xf6014444, 0x00000fe8);
Xil_Out32(0xf6014448, 0x00000088);
Xil_Out32(0xf601444c, 0x00000059);
Xil_Out32(0xf6014450, 0x00000040);
Xil_Out32(0xf6014000, 0x00000080);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x00000002);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x00000040);
Xil_Out32(0xf6014004, 0x00000000);
Xil_Out32(0xf6014000, 0x0000020c);
Xil_Out32(0xf6014004, 0x00000200);
Xil_Out32(0xf601400c, 0x00000000);
Xil_Out32(0xf60f000c, 0xf9e8d7c6);
Xil_Out32(0xf60f0000, 0x01038000);
Xil_Out32(0xf60f0004, 0x01028000);
Xil_Out32(0xf60f02c8, 0x0000ed90);
Xil_Out32(0xf60f086c, 0x00000003);
Xil_Out32(0xf60f0864, 0x00000001);
Xil_Out32(0xf60f0868, 0x00000001);
Xil_Out32(0xf60f0858, 0x00000002);
Xil_Out32(0xf60f0860, 0x00000002);
Xil_Out32(0xf60f083c, 0x00000100);
Xil_Out32(0xf60f0844, 0x00000100);
Xil_Out32(0xf60f0840, 0x00001fff);
Xil_Out32(0xf60f0848, 0x00001fff);
Xil_Out32(0xf60f03f0, 0x00003000);
Xil_Out32(0xf60f03f4, 0x00002000);
Xil_Out32(0xf60f03f8, 0x00001000);
Xil_Out32(0xf60f03fc, 0x00002000);
Xil_Out32(0xf60f0400, 0x00006000);
Xil_Out32(0xf60f0404, 0x00006000);
Xil_Out32(0xf60f0408, 0x00001000);
Xil_Out32(0xf60f03d0, 0x00000141);
Xil_Out32(0xf60f03d4, 0x00000141);
Xil_Out32(0xf60f03d8, 0x00000141);
Xil_Out32(0xf60f03dc, 0x00000141);
Xil_Out32(0xf60f03e0, 0x00000141);
Xil_Out32(0xf60f03e4, 0x00000141);
Xil_Out32(0xf60f03e8, 0x00000141);
Xil_Out32(0xf60f03ec, 0x00000141);
Xil_Out32(0xf60f0440, 0x00000408);
Xil_Out32(0xf60f0444, 0x00000082);
Xil_Out32(0xf60f0448, 0x00000e91);
Xil_Out32(0xf60f044c, 0x00000643);
Xil_Out32(0xf60f0450, 0x00000140);
Xil_Out32(0xf60f0000, 0x00000080);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x00000002);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x00000040);
Xil_Out32(0xf60f0004, 0x00000000);
Xil_Out32(0xf60f0000, 0x0000020c);
Xil_Out32(0xf60f0004, 0x00000200);
Xil_Out32(0xf60f000c, 0x00000000);
Xil_Out32(0xf60f200c, 0xf9e8d7c6);
Xil_Out32(0xf60f2000, 0x01038000);
Xil_Out32(0xf60f2004, 0x00038000);
Xil_Out32(0xf60f22c8, 0x00009b10);
Xil_Out32(0xf60f286c, 0x00000003);
Xil_Out32(0xf60f2864, 0x00000001);
Xil_Out32(0xf60f2868, 0x00000001);
Xil_Out32(0xf60f2858, 0x00000002);
Xil_Out32(0xf60f2860, 0x00000002);
Xil_Out32(0xf60f283c, 0x00000100);
Xil_Out32(0xf60f2844, 0x00000100);
Xil_Out32(0xf60f2840, 0x00001fff);
Xil_Out32(0xf60f2848, 0x00001fff);
Xil_Out32(0xf60f23f0, 0x00005000);
Xil_Out32(0xf60f23f4, 0x00004000);
Xil_Out32(0xf60f23f8, 0x00003000);
Xil_Out32(0xf60f23fc, 0x00002000);
Xil_Out32(0xf60f2400, 0x00005000);
Xil_Out32(0xf60f2404, 0x00006000);
Xil_Out32(0xf60f2408, 0x00006000);
Xil_Out32(0xf60f23d0, 0x000001c1);
Xil_Out32(0xf60f23d4, 0x000001c1);
Xil_Out32(0xf60f23d8, 0x000001c1);
Xil_Out32(0xf60f23dc, 0x000001c1);
Xil_Out32(0xf60f23e0, 0x000001c1);
Xil_Out32(0xf60f23e4, 0x000001c1);
Xil_Out32(0xf60f23e8, 0x000001c1);
Xil_Out32(0xf60f23ec, 0x000001c1);
Xil_Out32(0xf60f2440, 0x00000408);
Xil_Out32(0xf60f2444, 0x00000b4a);
Xil_Out32(0xf60f2448, 0x00000f95);
Xil_Out32(0xf60f244c, 0x000004a2);
Xil_Out32(0xf60f2450, 0x000001c0);
Xil_Out32(0xf60f2000, 0x00000080);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x00000002);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x00000040);
Xil_Out32(0xf60f2004, 0x00000000);
Xil_Out32(0xf60f2000, 0x0000020c);
Xil_Out32(0xf60f2004, 0x00000200);
Xil_Out32(0xf60f200c, 0x00000000);
Xil_Out32(0xf60f400c, 0xf9e8d7c6);
Xil_Out32(0xf60f4000, 0x01038000);
Xil_Out32(0xf60f4004, 0x01000000);
Xil_Out32(0xf60f42c8, 0x00001d40);
Xil_Out32(0xf60f486c, 0x00000003);
Xil_Out32(0xf60f4864, 0x00000001);
Xil_Out32(0xf60f4868, 0x00000001);
Xil_Out32(0xf60f4858, 0x00000002);
Xil_Out32(0xf60f4860, 0x00000002);
Xil_Out32(0xf60f483c, 0x00000100);
Xil_Out32(0xf60f4844, 0x00000100);
Xil_Out32(0xf60f4840, 0x00001fff);
Xil_Out32(0xf60f4848, 0x00001fff);
Xil_Out32(0xf60f43f0, 0x00003000);
Xil_Out32(0xf60f43f4, 0x00002000);
Xil_Out32(0xf60f43f8, 0x00003000);
Xil_Out32(0xf60f43fc, 0x00003000);
Xil_Out32(0xf60f4400, 0x00006000);
Xil_Out32(0xf60f4404, 0x00002000);
Xil_Out32(0xf60f4408, 0x00004000);
Xil_Out32(0xf60f43d0, 0x00000101);
Xil_Out32(0xf60f43d4, 0x00000101);
Xil_Out32(0xf60f43d8, 0x00000101);
Xil_Out32(0xf60f43dc, 0x00000101);
Xil_Out32(0xf60f43e0, 0x00000101);
Xil_Out32(0xf60f43e4, 0x00000101);
Xil_Out32(0xf60f43e8, 0x00000101);
Xil_Out32(0xf60f43ec, 0x00000101);
Xil_Out32(0xf60f4440, 0x00000408);
Xil_Out32(0xf60f4444, 0x00000a2e);
Xil_Out32(0xf60f4448, 0x00000ef6);
Xil_Out32(0xf60f444c, 0x00000990);
Xil_Out32(0xf60f4450, 0x00000180);
Xil_Out32(0xf60f4000, 0x00000080);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x00000002);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x00000040);
Xil_Out32(0xf60f4004, 0x00000000);
Xil_Out32(0xf60f4000, 0x0000020c);
Xil_Out32(0xf60f4004, 0x00000200);
Xil_Out32(0xf60f400c, 0x00000000);
Xil_Out32(0xf60f600c, 0xf9e8d7c6);
Xil_Out32(0xf60f6000, 0x01038000);
Xil_Out32(0xf60f6004, 0x01000000);
Xil_Out32(0xf60f62c8, 0x000098a8);
Xil_Out32(0xf60f686c, 0x00000003);
Xil_Out32(0xf60f6864, 0x00000001);
Xil_Out32(0xf60f6868, 0x00000001);
Xil_Out32(0xf60f6858, 0x00000002);
Xil_Out32(0xf60f6860, 0x00000002);
Xil_Out32(0xf60f683c, 0x00000100);
Xil_Out32(0xf60f6844, 0x00000100);
Xil_Out32(0xf60f6840, 0x00001fff);
Xil_Out32(0xf60f6848, 0x00001fff);
Xil_Out32(0xf60f63f0, 0x00006000);
Xil_Out32(0xf60f63f4, 0x00001000);
Xil_Out32(0xf60f63f8, 0x00002000);
Xil_Out32(0xf60f63fc, 0x00002000);
Xil_Out32(0xf60f6400, 0x00003000);
Xil_Out32(0xf60f6404, 0x00001000);
Xil_Out32(0xf60f6408, 0x00001000);
Xil_Out32(0xf60f63d0, 0x00000181);
Xil_Out32(0xf60f63d4, 0x00000181);
Xil_Out32(0xf60f63d8, 0x00000181);
Xil_Out32(0xf60f63dc, 0x00000181);
Xil_Out32(0xf60f63e0, 0x00000181);
Xil_Out32(0xf60f63e4, 0x00000181);
Xil_Out32(0xf60f63e8, 0x00000181);
Xil_Out32(0xf60f63ec, 0x00000181);
Xil_Out32(0xf60f6440, 0x00000408);
Xil_Out32(0xf60f6444, 0x000004fd);
Xil_Out32(0xf60f6448, 0x0000083c);
Xil_Out32(0xf60f644c, 0x00000fbd);
Xil_Out32(0xf60f6450, 0x00000100);
Xil_Out32(0xf60f6000, 0x00000080);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x00000002);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x00000040);
Xil_Out32(0xf60f6004, 0x00000000);
Xil_Out32(0xf60f6000, 0x0000020c);
Xil_Out32(0xf60f6004, 0x00000200);
Xil_Out32(0xf60f600c, 0x00000000);
Xil_Out32(0xf6e6000c, 0xf9e8d7c6);
Xil_Out32(0xf6e60000, 0x01038000);
Xil_Out32(0xf6e60004, 0x00018000);
Xil_Out32(0xf6e60454, 0x00000006);
Xil_Out32(0xf6e602c8, 0x00007360);
Xil_Out32(0xf6e6086c, 0x00000003);
Xil_Out32(0xf6e60864, 0x00000001);
Xil_Out32(0xf6e60868, 0x00000001);
Xil_Out32(0xf6e60858, 0x00000002);
Xil_Out32(0xf6e60860, 0x00000002);
Xil_Out32(0xf6e6083c, 0x00000100);
Xil_Out32(0xf6e60844, 0x00000100);
Xil_Out32(0xf6e60840, 0x00001fff);
Xil_Out32(0xf6e60848, 0x00001fff);
Xil_Out32(0xf6e603f0, 0x00005000);
Xil_Out32(0xf6e603f4, 0x00002000);
Xil_Out32(0xf6e603f8, 0x00006000);
Xil_Out32(0xf6e603fc, 0x00002000);
Xil_Out32(0xf6e60400, 0x00004000);
Xil_Out32(0xf6e60404, 0x00005000);
Xil_Out32(0xf6e60408, 0x00001000);
Xil_Out32(0xf6e603d0, 0x00000101);
Xil_Out32(0xf6e603d4, 0x00000101);
Xil_Out32(0xf6e603d8, 0x00000101);
Xil_Out32(0xf6e603dc, 0x00000101);
Xil_Out32(0xf6e603e0, 0x00000101);
Xil_Out32(0xf6e603e4, 0x00000101);
Xil_Out32(0xf6e603e8, 0x00000101);
Xil_Out32(0xf6e603ec, 0x00000101);
Xil_Out32(0xf6e6040c, 0x00000080);
Xil_Out32(0xf6e60410, 0x00000080);
Xil_Out32(0xf6e60414, 0x00000081);
Xil_Out32(0xf6e60418, 0x00000081);
Xil_Out32(0xf6e60428, 0x00000080);
Xil_Out32(0xf6e6042c, 0x0000045b);
Xil_Out32(0xf6e6041c, 0x00000080);
Xil_Out32(0xf6e60420, 0x00000080);
Xil_Out32(0xf6e60424, 0x00000080);
Xil_Out32(0xf6e60450, 0x00000600);
Xil_Out32(0xf6e60000, 0x00000080);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x00000002);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x00000040);
Xil_Out32(0xf6e60004, 0x00000000);
Xil_Out32(0xf6e60000, 0x0000020c);
Xil_Out32(0xf6e60004, 0x00000200);
Xil_Out32(0xf6e6000c, 0x00000000);
Xil_Out32(0xf6e9000c, 0xf9e8d7c6);
Xil_Out32(0xf6e90000, 0x01038000);
Xil_Out32(0xf6e90004, 0x00018000);
Xil_Out32(0xf6e90454, 0x00000006);
Xil_Out32(0xf6e902c8, 0x00008518);
Xil_Out32(0xf6e9086c, 0x00000003);
Xil_Out32(0xf6e90864, 0x00000001);
Xil_Out32(0xf6e90868, 0x00000001);
Xil_Out32(0xf6e90858, 0x00000002);
Xil_Out32(0xf6e90860, 0x00000002);
Xil_Out32(0xf6e9083c, 0x00000100);
Xil_Out32(0xf6e90844, 0x00000100);
Xil_Out32(0xf6e90840, 0x00001fff);
Xil_Out32(0xf6e90848, 0x00001fff);
Xil_Out32(0xf6e903f0, 0x00001000);
Xil_Out32(0xf6e903f4, 0x00005000);
Xil_Out32(0xf6e903f8, 0x00003000);
Xil_Out32(0xf6e903fc, 0x00003000);
Xil_Out32(0xf6e90400, 0x00006000);
Xil_Out32(0xf6e90404, 0x00002000);
Xil_Out32(0xf6e90408, 0x00005000);
Xil_Out32(0xf6e903d0, 0x00000101);
Xil_Out32(0xf6e903d4, 0x00000101);
Xil_Out32(0xf6e903d8, 0x00000101);
Xil_Out32(0xf6e903dc, 0x00000101);
Xil_Out32(0xf6e903e0, 0x00000101);
Xil_Out32(0xf6e903e4, 0x00000101);
Xil_Out32(0xf6e903e8, 0x00000101);
Xil_Out32(0xf6e903ec, 0x00000101);
Xil_Out32(0xf6e9040c, 0x00000001);
Xil_Out32(0xf6e90410, 0x00000001);
Xil_Out32(0xf6e90414, 0x00000001);
Xil_Out32(0xf6e90418, 0x00000001);
Xil_Out32(0xf6e90428, 0x00000001);
Xil_Out32(0xf6e9042c, 0x000006a8);
Xil_Out32(0xf6e9041c, 0x00000001);
Xil_Out32(0xf6e90420, 0x00000001);
Xil_Out32(0xf6e90424, 0x00000001);
Xil_Out32(0xf6e90440, 0x00000418);
Xil_Out32(0xf6e90444, 0x0000000f);
Xil_Out32(0xf6e90448, 0x000007df);
Xil_Out32(0xf6e9044c, 0x00000d63);
Xil_Out32(0xf6e90450, 0x00000601);
Xil_Out32(0xf6e90000, 0x00000080);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x00000002);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x00000040);
Xil_Out32(0xf6e90004, 0x00000000);
Xil_Out32(0xf6e90000, 0x0000020c);
Xil_Out32(0xf6e90004, 0x00000200);
Xil_Out32(0xf6e9000c, 0x00000000);
Xil_Out32(0xf6ec000c, 0xf9e8d7c6);
Xil_Out32(0xf6ec0000, 0x01038000);
Xil_Out32(0xf6ec0004, 0x01000000);
Xil_Out32(0xf6ec0454, 0x00000006);
Xil_Out32(0xf6ec02c8, 0x000017b8);
Xil_Out32(0xf6ec086c, 0x00000003);
Xil_Out32(0xf6ec0864, 0x00000001);
Xil_Out32(0xf6ec0868, 0x00000001);
Xil_Out32(0xf6ec0858, 0x00000002);
Xil_Out32(0xf6ec0860, 0x00000002);
Xil_Out32(0xf6ec083c, 0x00000100);
Xil_Out32(0xf6ec0844, 0x00000100);
Xil_Out32(0xf6ec0840, 0x00001fff);
Xil_Out32(0xf6ec0848, 0x00001fff);
Xil_Out32(0xf6ec03f0, 0x00006000);
Xil_Out32(0xf6ec03f4, 0x00006000);
Xil_Out32(0xf6ec03f8, 0x00001000);
Xil_Out32(0xf6ec03fc, 0x00004000);
Xil_Out32(0xf6ec0400, 0x00002000);
Xil_Out32(0xf6ec0404, 0x00006000);
Xil_Out32(0xf6ec0408, 0x00004000);
Xil_Out32(0xf6ec03d0, 0x000002c0);
Xil_Out32(0xf6ec03d4, 0x000002c0);
Xil_Out32(0xf6ec03d8, 0x000002c0);
Xil_Out32(0xf6ec03dc, 0x000002c0);
Xil_Out32(0xf6ec03e0, 0x000002c0);
Xil_Out32(0xf6ec03e4, 0x000002c0);
Xil_Out32(0xf6ec03e8, 0x000002c0);
Xil_Out32(0xf6ec03ec, 0x000002c0);
Xil_Out32(0xf6ec040c, 0x000002c0);
Xil_Out32(0xf6ec0410, 0x000002c0);
Xil_Out32(0xf6ec0414, 0x000002c0);
Xil_Out32(0xf6ec0418, 0x000002c0);
Xil_Out32(0xf6ec0428, 0x000002c0);
Xil_Out32(0xf6ec042c, 0x000004f8);
Xil_Out32(0xf6ec041c, 0x000002c0);
Xil_Out32(0xf6ec0420, 0x000002c0);
Xil_Out32(0xf6ec0424, 0x000002c0);
Xil_Out32(0xf6ec0440, 0x00000419);
Xil_Out32(0xf6ec0444, 0x00000464);
Xil_Out32(0xf6ec0448, 0x00000076);
Xil_Out32(0xf6ec044c, 0x00000d6a);
Xil_Out32(0xf6ec0450, 0x00000602);
Xil_Out32(0xf6ec0000, 0x00000080);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x00000002);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x00000040);
Xil_Out32(0xf6ec0004, 0x00000000);
Xil_Out32(0xf6ec0000, 0x0000020c);
Xil_Out32(0xf6ec0004, 0x00000200);
Xil_Out32(0xf6ec000c, 0x00000000);
Xil_Out32(0xf6ee000c, 0xf9e8d7c6);
Xil_Out32(0xf6ee0000, 0x01038000);
Xil_Out32(0xf6ee0004, 0x01008000);
Xil_Out32(0xf6ee0454, 0x00000006);
Xil_Out32(0xf6ee02c8, 0x00002ae0);
Xil_Out32(0xf6ee086c, 0x00000003);
Xil_Out32(0xf6ee0864, 0x00000001);
Xil_Out32(0xf6ee0868, 0x00000001);
Xil_Out32(0xf6ee0858, 0x00000002);
Xil_Out32(0xf6ee0860, 0x00000002);
Xil_Out32(0xf6ee083c, 0x00000100);
Xil_Out32(0xf6ee0844, 0x00000100);
Xil_Out32(0xf6ee0840, 0x00001fff);
Xil_Out32(0xf6ee0848, 0x00001fff);
Xil_Out32(0xf6ee03f0, 0x00006000);
Xil_Out32(0xf6ee03f4, 0x00004000);
Xil_Out32(0xf6ee03f8, 0x00005000);
Xil_Out32(0xf6ee03fc, 0x00003000);
Xil_Out32(0xf6ee0400, 0x00003000);
Xil_Out32(0xf6ee0404, 0x00001000);
Xil_Out32(0xf6ee0408, 0x00003000);
Xil_Out32(0xf6ee03d0, 0x00000240);
Xil_Out32(0xf6ee03d4, 0x00000240);
Xil_Out32(0xf6ee03d8, 0x00000240);
Xil_Out32(0xf6ee03dc, 0x00000240);
Xil_Out32(0xf6ee03e0, 0x00000240);
Xil_Out32(0xf6ee03e4, 0x00000240);
Xil_Out32(0xf6ee03e8, 0x00000240);
Xil_Out32(0xf6ee03ec, 0x00000240);
Xil_Out32(0xf6ee040c, 0x00000240);
Xil_Out32(0xf6ee0410, 0x00000240);
Xil_Out32(0xf6ee0414, 0x00000240);
Xil_Out32(0xf6ee0418, 0x00000240);
Xil_Out32(0xf6ee0428, 0x00000240);
Xil_Out32(0xf6ee042c, 0x000008b6);
Xil_Out32(0xf6ee041c, 0x00000240);
Xil_Out32(0xf6ee0420, 0x00000240);
Xil_Out32(0xf6ee0424, 0x00000240);
Xil_Out32(0xf6ee0450, 0x00000603);
Xil_Out32(0xf6ee0000, 0x00000080);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x00000002);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x00000040);
Xil_Out32(0xf6ee0004, 0x00000000);
Xil_Out32(0xf6ee0000, 0x0000020c);
Xil_Out32(0xf6ee0004, 0x00000200);
Xil_Out32(0xf6ee000c, 0x00000000);
Xil_Out32(0xf6f1000c, 0xf9e8d7c6);
Xil_Out32(0xf6f10000, 0x01038000);
Xil_Out32(0xf6f10004, 0x00018000);
Xil_Out32(0xf6f10454, 0x00000004);
Xil_Out32(0xf6f102c8, 0x00004286);
Xil_Out32(0xf6f1086c, 0x00000003);
Xil_Out32(0xf6f10864, 0x00000001);
Xil_Out32(0xf6f10868, 0x00000001);
Xil_Out32(0xf6f10858, 0x00000002);
Xil_Out32(0xf6f10860, 0x00000002);
Xil_Out32(0xf6f1083c, 0x00000100);
Xil_Out32(0xf6f10844, 0x00000100);
Xil_Out32(0xf6f10840, 0x00001fff);
Xil_Out32(0xf6f10848, 0x00001fff);
Xil_Out32(0xf6f10440, 0x00000428);
Xil_Out32(0xf6f10444, 0x00000f02);
Xil_Out32(0xf6f10448, 0x00000df2);
Xil_Out32(0xf6f1044c, 0x00000f2f);
Xil_Out32(0xf6f10450, 0x00000604);
Xil_Out32(0xf6f10000, 0x00000080);
Xil_Out32(0xf6f10004, 0x00000000);
Xil_Out32(0xf6f10000, 0x00000002);
Xil_Out32(0xf6f10004, 0x00000000);
Xil_Out32(0xf6f10000, 0x00000040);
Xil_Out32(0xf6f10004, 0x00000000);
Xil_Out32(0xf6f10000, 0x0000020c);
Xil_Out32(0xf6f10004, 0x00000200);
Xil_Out32(0xf6f1000c, 0x00000000);
Xil_Out32(0xf6f3000c, 0xf9e8d7c6);
Xil_Out32(0xf6f30000, 0x01038000);
Xil_Out32(0xf6f30004, 0x01008000);
Xil_Out32(0xf6f30454, 0x00000006);
Xil_Out32(0xf6f302c8, 0x00005408);
Xil_Out32(0xf6f3086c, 0x00000003);
Xil_Out32(0xf6f30864, 0x00000001);
Xil_Out32(0xf6f30868, 0x00000001);
Xil_Out32(0xf6f30858, 0x00000002);
Xil_Out32(0xf6f30860, 0x00000002);
Xil_Out32(0xf6f3083c, 0x00000100);
Xil_Out32(0xf6f30844, 0x00000100);
Xil_Out32(0xf6f30840, 0x00001fff);
Xil_Out32(0xf6f30848, 0x00001fff);
Xil_Out32(0xf6f303f0, 0x00005000);
Xil_Out32(0xf6f303f4, 0x00006000);
Xil_Out32(0xf6f303f8, 0x00004000);
Xil_Out32(0xf6f303fc, 0x00003000);
Xil_Out32(0xf6f30400, 0x00001000);
Xil_Out32(0xf6f30404, 0x00005000);
Xil_Out32(0xf6f30408, 0x00002000);
Xil_Out32(0xf6f303d0, 0x00000281);
Xil_Out32(0xf6f303d4, 0x00000281);
Xil_Out32(0xf6f303d8, 0x00000281);
Xil_Out32(0xf6f303dc, 0x00000281);
Xil_Out32(0xf6f303e0, 0x00000281);
Xil_Out32(0xf6f303e4, 0x00000281);
Xil_Out32(0xf6f303e8, 0x00000281);
Xil_Out32(0xf6f303ec, 0x00000281);
Xil_Out32(0xf6f3040c, 0x00000281);
Xil_Out32(0xf6f30410, 0x00000281);
Xil_Out32(0xf6f30414, 0x00000281);
Xil_Out32(0xf6f30418, 0x00000281);
Xil_Out32(0xf6f30428, 0x00000281);
Xil_Out32(0xf6f3042c, 0x00000195);
Xil_Out32(0xf6f3041c, 0x00000281);
Xil_Out32(0xf6f30420, 0x00000281);
Xil_Out32(0xf6f30424, 0x00000281);
Xil_Out32(0xf6f30450, 0x00000605);
Xil_Out32(0xf6f30000, 0x00000080);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x00000002);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x00000040);
Xil_Out32(0xf6f30004, 0x00000000);
Xil_Out32(0xf6f30000, 0x0000020c);
Xil_Out32(0xf6f30004, 0x00000200);
Xil_Out32(0xf6f3000c, 0x00000000);
Xil_Out32(0xf6f6000c, 0xf9e8d7c6);
Xil_Out32(0xf6f60000, 0x01038000);
Xil_Out32(0xf6f60004, 0x01008000);
Xil_Out32(0xf6f60454, 0x00000004);
Xil_Out32(0xf6f602c8, 0x00004296);
Xil_Out32(0xf6f6086c, 0x00000003);
Xil_Out32(0xf6f60864, 0x00000001);
Xil_Out32(0xf6f60868, 0x00000001);
Xil_Out32(0xf6f60858, 0x00000002);
Xil_Out32(0xf6f60860, 0x00000002);
Xil_Out32(0xf6f6083c, 0x00000100);
Xil_Out32(0xf6f60844, 0x00000100);
Xil_Out32(0xf6f60840, 0x00001fff);
Xil_Out32(0xf6f60848, 0x00001fff);
Xil_Out32(0xf6f60440, 0x00000429);
Xil_Out32(0xf6f60444, 0x00000f99);
Xil_Out32(0xf6f60448, 0x000003cc);
Xil_Out32(0xf6f6044c, 0x00000624);
Xil_Out32(0xf6f60450, 0x00000606);
Xil_Out32(0xf6f60000, 0x00000080);
Xil_Out32(0xf6f60004, 0x00000000);
Xil_Out32(0xf6f60000, 0x00000002);
Xil_Out32(0xf6f60004, 0x00000000);
Xil_Out32(0xf6f60000, 0x00000040);
Xil_Out32(0xf6f60004, 0x00000000);
Xil_Out32(0xf6f60000, 0x0000020c);
Xil_Out32(0xf6f60004, 0x00000200);
Xil_Out32(0xf6f6000c, 0x00000000);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6c9000c, 0xf9e8d7c6);
Xil_Out32(0xf6c90000, 0x01038000);
Xil_Out32(0xf6c90004, 0x01020000);
Xil_Out32(0xf6c90454, 0x00000006);
Xil_Out32(0xf6c902c8, 0x0000d770);
Xil_Out32(0xf6c9086c, 0x00000003);
Xil_Out32(0xf6c90864, 0x00000001);
Xil_Out32(0xf6c90868, 0x00000001);
Xil_Out32(0xf6c90858, 0x00000002);
Xil_Out32(0xf6c90860, 0x00000002);
Xil_Out32(0xf6c9083c, 0x00000100);
Xil_Out32(0xf6c90844, 0x00000100);
Xil_Out32(0xf6c90840, 0x00001fff);
Xil_Out32(0xf6c90848, 0x00001fff);
Xil_Out32(0xf6c903f0, 0x00003000);
Xil_Out32(0xf6c903f4, 0x00005000);
Xil_Out32(0xf6c903f8, 0x00002000);
Xil_Out32(0xf6c903fc, 0x00004000);
Xil_Out32(0xf6c90400, 0x00001000);
Xil_Out32(0xf6c90404, 0x00006000);
Xil_Out32(0xf6c90408, 0x00006000);
Xil_Out32(0xf6c903d0, 0x00000101);
Xil_Out32(0xf6c903d4, 0x00000101);
Xil_Out32(0xf6c903d8, 0x00000101);
Xil_Out32(0xf6c903dc, 0x00000101);
Xil_Out32(0xf6c903e0, 0x00000101);
Xil_Out32(0xf6c903e4, 0x00000101);
Xil_Out32(0xf6c903e8, 0x00000101);
Xil_Out32(0xf6c903ec, 0x00000101);
Xil_Out32(0xf6c90440, 0x00000488);
Xil_Out32(0xf6c90444, 0x00000489);
Xil_Out32(0xf6c90448, 0x000004c8);
Xil_Out32(0xf6c9044c, 0x000004c9);
Xil_Out32(0xf6c90450, 0x00000640);
Xil_Out32(0xf6c90000, 0x00000080);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x00000002);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x00000040);
Xil_Out32(0xf6c90004, 0x00000000);
Xil_Out32(0xf6c90000, 0x0000020c);
Xil_Out32(0xf6c90004, 0x00000200);
Xil_Out32(0xf6c9000c, 0x00000000);
Xil_Out32(0xf6cc000c, 0xf9e8d7c6);
Xil_Out32(0xf6cc0000, 0x01038000);
Xil_Out32(0xf6cc0004, 0x00020000);
Xil_Out32(0xf6cc0454, 0x00000006);
Xil_Out32(0xf6cc02c8, 0x000013c8);
Xil_Out32(0xf6cc086c, 0x00000003);
Xil_Out32(0xf6cc0864, 0x00000001);
Xil_Out32(0xf6cc0868, 0x00000001);
Xil_Out32(0xf6cc0858, 0x00000002);
Xil_Out32(0xf6cc0860, 0x00000002);
Xil_Out32(0xf6cc083c, 0x00000100);
Xil_Out32(0xf6cc0844, 0x00000100);
Xil_Out32(0xf6cc0840, 0x00001fff);
Xil_Out32(0xf6cc0848, 0x00001fff);
Xil_Out32(0xf6cc03f0, 0x00002000);
Xil_Out32(0xf6cc03f4, 0x00006000);
Xil_Out32(0xf6cc03f8, 0x00006000);
Xil_Out32(0xf6cc03fc, 0x00003000);
Xil_Out32(0xf6cc0400, 0x00006000);
Xil_Out32(0xf6cc0404, 0x00002000);
Xil_Out32(0xf6cc0408, 0x00003000);
Xil_Out32(0xf6cc03d0, 0x00000101);
Xil_Out32(0xf6cc03d4, 0x00000101);
Xil_Out32(0xf6cc03d8, 0x00000101);
Xil_Out32(0xf6cc03dc, 0x00000101);
Xil_Out32(0xf6cc03e0, 0x00000101);
Xil_Out32(0xf6cc03e4, 0x00000101);
Xil_Out32(0xf6cc03e8, 0x00000101);
Xil_Out32(0xf6cc03ec, 0x00000101);
Xil_Out32(0xf6cc0440, 0x00000508);
Xil_Out32(0xf6cc0444, 0x00000509);
Xil_Out32(0xf6cc0448, 0x00000548);
Xil_Out32(0xf6cc044c, 0x00000549);
Xil_Out32(0xf6cc0450, 0x00000641);
Xil_Out32(0xf6cc0000, 0x00000080);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x00000002);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x00000040);
Xil_Out32(0xf6cc0004, 0x00000000);
Xil_Out32(0xf6cc0000, 0x0000020c);
Xil_Out32(0xf6cc0004, 0x00000200);
Xil_Out32(0xf6cc000c, 0x00000000);
#endif
Xil_Out32(0xf602000c, 0xf9e8d7c6);
Xil_Out32(0xf6020000, 0x000002ff);
Xil_Out32(0xf6020004, 0x00000200);
Xil_Out32(0xf6020100, 0x00000081);
Xil_Out32(0xf6020104, 0x00000004);
Xil_Out32(0xf6020108, 0x00000000);
Xil_Out32(0xf602000c, 0x00000000);
Xil_Out32(0xf60e400c, 0xf9e8d7c6);
Xil_Out32(0xf60e4000, 0x000002ff);
Xil_Out32(0xf60e4004, 0x00000200);
Xil_Out32(0xf60e4100, 0x00000281);
Xil_Out32(0xf60e4104, 0x00000004);
Xil_Out32(0xf60e4108, 0x00000000);
Xil_Out32(0xf60e400c, 0x00000000);
Xil_Out32(0xf60e600c, 0xf9e8d7c6);
Xil_Out32(0xf60e6000, 0x000002ff);
Xil_Out32(0xf60e6004, 0x00000200);
Xil_Out32(0xf60e6100, 0x00000001);
Xil_Out32(0xf60e6104, 0x00000004);
Xil_Out32(0xf60e6108, 0x00000000);
Xil_Out32(0xf60e600c, 0x00000000);
Xil_Out32(0xf602200c, 0xf9e8d7c6);
Xil_Out32(0xf6022000, 0x000002ff);
Xil_Out32(0xf6022004, 0x00000200);
Xil_Out32(0xf6022100, 0x00000080);
Xil_Out32(0xf6022104, 0x00000004);
Xil_Out32(0xf6022108, 0x00000000);
Xil_Out32(0xf602200c, 0x00000000);
Xil_Out32(0xf60d000c, 0xf9e8d7c6);
Xil_Out32(0xf60d0000, 0x000002ff);
Xil_Out32(0xf60d0004, 0x00000200);
Xil_Out32(0xf60d0100, 0x000002c0);
Xil_Out32(0xf60d0104, 0x00000004);
Xil_Out32(0xf60d0108, 0x00000000);
Xil_Out32(0xf60d000c, 0x00000000);
Xil_Out32(0xf60d200c, 0xf9e8d7c6);
Xil_Out32(0xf60d2000, 0x000002ff);
Xil_Out32(0xf60d2004, 0x00000200);
Xil_Out32(0xf60d2100, 0x00000240);
Xil_Out32(0xf60d2104, 0x00000004);
Xil_Out32(0xf60d2108, 0x00000000);
Xil_Out32(0xf60d200c, 0x00000000);
Xil_Out32(0xf6e6200c, 0xf9e8d7c6);
Xil_Out32(0xf6e62000, 0x000002ff);
Xil_Out32(0xf6e62004, 0x00000200);
Xil_Out32(0xf6e62100, 0x00000608);
Xil_Out32(0xf6e62104, 0x00000006);
Xil_Out32(0xf6e62108, 0x00000000);
Xil_Out32(0xf6e6200c, 0x00000000);
Xil_Out32(0xf6e9200c, 0xf9e8d7c6);
Xil_Out32(0xf6e92000, 0x000002ff);
Xil_Out32(0xf6e92004, 0x00000200);
Xil_Out32(0xf6e92100, 0x00000609);
Xil_Out32(0xf6e92104, 0x00000006);
Xil_Out32(0xf6e92108, 0x00000000);
Xil_Out32(0xf6e9200c, 0x00000000);
Xil_Out32(0xf6ec200c, 0xf9e8d7c6);
Xil_Out32(0xf6ec2000, 0x000002ff);
Xil_Out32(0xf6ec2004, 0x00000200);
Xil_Out32(0xf6ec2100, 0x0000060a);
Xil_Out32(0xf6ec2104, 0x00000006);
Xil_Out32(0xf6ec2108, 0x00000000);
Xil_Out32(0xf6ec200c, 0x00000000);
Xil_Out32(0xf6ee200c, 0xf9e8d7c6);
Xil_Out32(0xf6ee2000, 0x000002ff);
Xil_Out32(0xf6ee2004, 0x00000200);
Xil_Out32(0xf6ee2100, 0x0000060b);
Xil_Out32(0xf6ee2104, 0x00000006);
Xil_Out32(0xf6ee2108, 0x00000000);
Xil_Out32(0xf6ee200c, 0x00000000);
Xil_Out32(0xf6f1200c, 0xf9e8d7c6);
Xil_Out32(0xf6f12000, 0x000002ff);
Xil_Out32(0xf6f12004, 0x00000200);
Xil_Out32(0xf6f12100, 0x0000060c);
Xil_Out32(0xf6f12104, 0x00000004);
Xil_Out32(0xf6f12108, 0x00000002);
Xil_Out32(0xf6f1200c, 0x00000000);
Xil_Out32(0xf6f3200c, 0xf9e8d7c6);
Xil_Out32(0xf6f32000, 0x000002ff);
Xil_Out32(0xf6f32004, 0x00000200);
Xil_Out32(0xf6f32100, 0x0000060d);
Xil_Out32(0xf6f32104, 0x00000006);
Xil_Out32(0xf6f32108, 0x00000000);
Xil_Out32(0xf6f3200c, 0x00000000);
Xil_Out32(0xf6f6200c, 0xf9e8d7c6);
Xil_Out32(0xf6f62000, 0x000002ff);
Xil_Out32(0xf6f62004, 0x00000200);
Xil_Out32(0xf6f62100, 0x0000060e);
Xil_Out32(0xf6f62104, 0x00000004);
Xil_Out32(0xf6f62108, 0x00000002);
Xil_Out32(0xf6f6200c, 0x00000000);
Xil_Out32(0xf607000c, 0xf9e8d7c6);
Xil_Out32(0xf607000c, 0xf9e8d7c6);
Xil_Out32(0xf6070010, 0x06170651);
Xil_Out32(0xf6070014, 0x00637265);
Xil_Out32(0xf6070018, 0x06170651);
Xil_Out32(0xf607001c, 0x00637265);
Xil_Out32(0xf6070020, 0x06170651);
Xil_Out32(0xf6070024, 0x00637265);
Xil_Out32(0xf6070028, 0x06170651);
Xil_Out32(0xf607002c, 0x00637265);
Xil_Out32(0xf6070048, 0x1349140f);
Xil_Out32(0xf607004c, 0x185d6554);
Xil_Out32(0xf6070050, 0x1d71b699);
Xil_Out32(0xf6070054, 0x03030c1e);
Xil_Out32(0xf6070058, 0x0a248144);
Xil_Out32(0xf607005c, 0x1f38d30b);
Xil_Out32(0xf6070060, 0x00c22860);
Xil_Out32(0xf6070044, 0x301c6c30);
Xil_Out32(0xf607003c, 0x00000000);
Xil_Out32(0xf607044c, 0x00884210);
Xil_Out32(0xf607044c, 0x00844210);
Xil_Out32(0xf607044c, 0x00842210);
Xil_Out32(0xf607044c, 0x00842110);
Xil_Out32(0xf607044c, 0x00842108);
Xil_Out32(0xf6070448, 0x0003fc02);
Xil_Out32(0xf6070448, 0x0003fcff);
Xil_Out32(0xf6070444, 0x0ff00804);
Xil_Out32(0xf6070444, 0x0ff3fc04);
Xil_Out32(0xf6070444, 0x0ff3fcff);
Xil_Out32(0xf607046c, 0x003fffff);
Xil_Out32(0xf6070468, 0x003fffff);
Xil_Out32(0xf6070464, 0x003fffff);
Xil_Out32(0xf6070460, 0x003fffff);
Xil_Out32(0xf607045c, 0x003fffff);
Xil_Out32(0xf607046c, 0x000fffff);
Xil_Out32(0xf6070468, 0x000fffff);
Xil_Out32(0xf6070464, 0x000fffff);
Xil_Out32(0xf6070460, 0x000fffff);
Xil_Out32(0xf607045c, 0x000fffff);
Xil_Out32(0xf6070480, 0x003fffff);
Xil_Out32(0xf607047c, 0x003fffff);
Xil_Out32(0xf6070478, 0x003fffff);
Xil_Out32(0xf6070474, 0x003fffff);
Xil_Out32(0xf6070470, 0x003fffff);
Xil_Out32(0xf6070480, 0x000fffff);
Xil_Out32(0xf607047c, 0x000fffff);
Xil_Out32(0xf6070478, 0x000fffff);
Xil_Out32(0xf6070474, 0x000fffff);
Xil_Out32(0xf6070470, 0x000fffff);
Xil_Out32(0xf6070494, 0x003fffff);
Xil_Out32(0xf6070490, 0x003fffff);
Xil_Out32(0xf607048c, 0x003fffff);
Xil_Out32(0xf6070488, 0x003fffff);
Xil_Out32(0xf6070484, 0x003fffff);
Xil_Out32(0xf6070494, 0x000fffff);
Xil_Out32(0xf6070490, 0x000fffff);
Xil_Out32(0xf607048c, 0x000fffff);
Xil_Out32(0xf6070488, 0x000fffff);
Xil_Out32(0xf6070484, 0x000fffff);
Xil_Out32(0xf60704a8, 0x003fffff);
Xil_Out32(0xf60704a4, 0x003fffff);
Xil_Out32(0xf60704a0, 0x003fffff);
Xil_Out32(0xf607049c, 0x003fffff);
Xil_Out32(0xf6070498, 0x003fffff);
Xil_Out32(0xf60704a8, 0x000fffff);
Xil_Out32(0xf60704a4, 0x000fffff);
Xil_Out32(0xf60704a0, 0x000fffff);
Xil_Out32(0xf607049c, 0x000fffff);
Xil_Out32(0xf6070498, 0x000fffff);
Xil_Out32(0xf60704ac, 0x01f84210);
Xil_Out32(0xf60704ac, 0x01ffc210);
Xil_Out32(0xf60704ac, 0x01fffe10);
Xil_Out32(0xf60704ac, 0x01fffff0);
Xil_Out32(0xf60704ac, 0x01ffffff);
Xil_Out32(0xf60704b0, 0x00084210);
Xil_Out32(0xf60704b0, 0x00004210);
Xil_Out32(0xf60704b0, 0x00000210);
Xil_Out32(0xf60704b0, 0x00000010);
Xil_Out32(0xf60704b0, 0x00000000);
Xil_Out32(0xf60704b4, 0x003fffff);
Xil_Out32(0xf60704b8, 0x003fffff);
Xil_Out32(0xf60704bc, 0x003fffff);
Xil_Out32(0xf60704c0, 0x003fffff);
Xil_Out32(0xf60704c4, 0x003fffff);
Xil_Out32(0xf6070418, 0x01010ff0);
Xil_Out32(0xf6070410, 0x01010ff0);
Xil_Out32(0xf6070408, 0x01010ff0);
Xil_Out32(0xf6070400, 0x01010ff0);
Xil_Out32(0xf6070418, 0x010ffff0);
Xil_Out32(0xf6070410, 0x010ffff0);
Xil_Out32(0xf6070408, 0x010ffff0);
Xil_Out32(0xf6070400, 0x010ffff0);
Xil_Out32(0xf6070418, 0x0ffffff0);
Xil_Out32(0xf6070410, 0x0ffffff0);
Xil_Out32(0xf6070408, 0x0ffffff0);
Xil_Out32(0xf6070400, 0x0ffffff0);
Xil_Out32(0xf607041c, 0x00010ff0);
Xil_Out32(0xf6070414, 0x00010ff0);
Xil_Out32(0xf607040c, 0x00010ff0);
Xil_Out32(0xf6070404, 0x00010ff0);
Xil_Out32(0xf607041c, 0x000ffff0);
Xil_Out32(0xf6070414, 0x000ffff0);
Xil_Out32(0xf607040c, 0x000ffff0);
Xil_Out32(0xf6070404, 0x000ffff0);
Xil_Out32(0xf6070438, 0x000404ff);
Xil_Out32(0xf6070430, 0x000404ff);
Xil_Out32(0xf6070428, 0x000404ff);
Xil_Out32(0xf6070420, 0x000404ff);
Xil_Out32(0xf6070438, 0x0004ffff);
Xil_Out32(0xf6070430, 0x0004ffff);
Xil_Out32(0xf6070428, 0x0004ffff);
Xil_Out32(0xf6070420, 0x0004ffff);
Xil_Out32(0xf6070438, 0x00ffffff);
Xil_Out32(0xf6070430, 0x00ffffff);
Xil_Out32(0xf6070428, 0x00ffffff);
Xil_Out32(0xf6070420, 0x00ffffff);
Xil_Out32(0xf607043c, 0x000004ff);
Xil_Out32(0xf6070434, 0x000004ff);
Xil_Out32(0xf607042c, 0x000004ff);
Xil_Out32(0xf6070424, 0x000004ff);
Xil_Out32(0xf607043c, 0x0000ffff);
Xil_Out32(0xf6070434, 0x0000ffff);
Xil_Out32(0xf607042c, 0x0000ffff);
Xil_Out32(0xf6070424, 0x0000ffff);
Xil_Out32(0xf6070064, 0x00000101);
Xil_Out32(0xf6070068, 0x00000181);
Xil_Out32(0xf607006c, 0x000001c1);
Xil_Out32(0xf6070070, 0x00000141);
Xil_Out32(0xf6070758, 0x00000001);
//**************Register programming from noc_cfg end----------------

dbg0_pmc(16385);
 //--------------------------------------
#ifdef XPMCFW_HW90
//**************Register programming from xpio_iobpair start----------------
dbg0_pmc(16386);
//*************************************************************************************************XPIO_IOBPAIR : start

//****************************************************
//XPIO_0
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf609020c, 0xf9e8d7c6);
Xil_Out32(0xf609040c, 0xf9e8d7c6);
Xil_Out32(0xf609060c, 0xf9e8d7c6);
Xil_Out32(0xf6090228, 0x00000006);
Xil_Out32(0xf6090428, 0x00000006);
Xil_Out32(0xf6090628, 0x00000006);
Xil_Out32(0xf6090244, 0x00000006);
Xil_Out32(0xf6090444, 0x00000006);
Xil_Out32(0xf6090644, 0x00000006);
Xil_Out32(0xf6090200, 0x00000002);
Xil_Out32(0xf6090204, 0x00000000);
Xil_Out32(0xf6090400, 0x00000002);
Xil_Out32(0xf6090404, 0x00000000);
Xil_Out32(0xf6090600, 0x00000002);
Xil_Out32(0xf6090604, 0x00000000);
Xil_Out32(0xf6090200, 0x00000004);
Xil_Out32(0xf6090204, 0x00000000);
Xil_Out32(0xf6090400, 0x00000004);
Xil_Out32(0xf6090404, 0x00000000);
Xil_Out32(0xf6090600, 0x00000004);
Xil_Out32(0xf6090604, 0x00000000);
Xil_Out32(0xf6090200, 0x00000101);
Xil_Out32(0xf6090204, 0x00000001);
Xil_Out32(0xf6090400, 0x00000101);
Xil_Out32(0xf6090404, 0x00000001);
Xil_Out32(0xf6090600, 0x00000101);
Xil_Out32(0xf6090604, 0x00000001);
//------------------------------nibble_1
Xil_Out32(0xf609120c, 0xf9e8d7c6);
Xil_Out32(0xf609140c, 0xf9e8d7c6);
Xil_Out32(0xf609160c, 0xf9e8d7c6);
Xil_Out32(0xf6091228, 0x00000006);
Xil_Out32(0xf6091428, 0x00000006);
Xil_Out32(0xf6091628, 0x00000006);
Xil_Out32(0xf6091244, 0x00000006);
Xil_Out32(0xf6091444, 0x00000006);
Xil_Out32(0xf6091644, 0x00000006);
Xil_Out32(0xf6091200, 0x00000002);
Xil_Out32(0xf6091204, 0x00000000);
Xil_Out32(0xf6091400, 0x00000002);
Xil_Out32(0xf6091404, 0x00000000);
Xil_Out32(0xf6091600, 0x00000002);
Xil_Out32(0xf6091604, 0x00000000);
Xil_Out32(0xf6091200, 0x00000004);
Xil_Out32(0xf6091204, 0x00000000);
Xil_Out32(0xf6091400, 0x00000004);
Xil_Out32(0xf6091404, 0x00000000);
Xil_Out32(0xf6091600, 0x00000004);
Xil_Out32(0xf6091604, 0x00000000);
Xil_Out32(0xf6091200, 0x00000101);
Xil_Out32(0xf6091204, 0x00000001);
Xil_Out32(0xf6091400, 0x00000101);
Xil_Out32(0xf6091404, 0x00000001);
Xil_Out32(0xf6091600, 0x00000101);
Xil_Out32(0xf6091604, 0x00000001);
//------------------------------nibble_2
Xil_Out32(0xf609220c, 0xf9e8d7c6);
Xil_Out32(0xf609240c, 0xf9e8d7c6);
Xil_Out32(0xf609260c, 0xf9e8d7c6);
Xil_Out32(0xf6092228, 0x00000006);
Xil_Out32(0xf6092428, 0x00000006);
Xil_Out32(0xf6092628, 0x00000006);
Xil_Out32(0xf6092244, 0x00000006);
Xil_Out32(0xf6092444, 0x00000006);
Xil_Out32(0xf6092644, 0x00000006);
Xil_Out32(0xf6092200, 0x00000002);
Xil_Out32(0xf6092204, 0x00000000);
Xil_Out32(0xf6092400, 0x00000002);
Xil_Out32(0xf6092404, 0x00000000);
Xil_Out32(0xf6092600, 0x00000002);
Xil_Out32(0xf6092604, 0x00000000);
Xil_Out32(0xf6092200, 0x00000004);
Xil_Out32(0xf6092204, 0x00000000);
Xil_Out32(0xf6092400, 0x00000004);
Xil_Out32(0xf6092404, 0x00000000);
Xil_Out32(0xf6092600, 0x00000004);
Xil_Out32(0xf6092604, 0x00000000);
Xil_Out32(0xf6092200, 0x00000101);
Xil_Out32(0xf6092204, 0x00000001);
Xil_Out32(0xf6092400, 0x00000101);
Xil_Out32(0xf6092404, 0x00000001);
Xil_Out32(0xf6092600, 0x00000101);
Xil_Out32(0xf6092604, 0x00000001);
//------------------------------nibble_3
Xil_Out32(0xf609320c, 0xf9e8d7c6);
Xil_Out32(0xf609340c, 0xf9e8d7c6);
Xil_Out32(0xf609360c, 0xf9e8d7c6);
Xil_Out32(0xf6093228, 0x00000006);
Xil_Out32(0xf6093428, 0x00000006);
Xil_Out32(0xf6093628, 0x00000006);
Xil_Out32(0xf6093244, 0x00000006);
Xil_Out32(0xf6093444, 0x00000006);
Xil_Out32(0xf6093644, 0x00000006);
Xil_Out32(0xf6093200, 0x00000002);
Xil_Out32(0xf6093204, 0x00000000);
Xil_Out32(0xf6093400, 0x00000002);
Xil_Out32(0xf6093404, 0x00000000);
Xil_Out32(0xf6093600, 0x00000002);
Xil_Out32(0xf6093604, 0x00000000);
Xil_Out32(0xf6093200, 0x00000004);
Xil_Out32(0xf6093204, 0x00000000);
Xil_Out32(0xf6093400, 0x00000004);
Xil_Out32(0xf6093404, 0x00000000);
Xil_Out32(0xf6093600, 0x00000004);
Xil_Out32(0xf6093604, 0x00000000);
Xil_Out32(0xf6093200, 0x00000101);
Xil_Out32(0xf6093204, 0x00000001);
Xil_Out32(0xf6093400, 0x00000101);
Xil_Out32(0xf6093404, 0x00000001);
Xil_Out32(0xf6093600, 0x00000101);
Xil_Out32(0xf6093604, 0x00000001);
//------------------------------nibble_4
Xil_Out32(0xf609420c, 0xf9e8d7c6);
Xil_Out32(0xf609440c, 0xf9e8d7c6);
Xil_Out32(0xf609460c, 0xf9e8d7c6);
Xil_Out32(0xf6094228, 0x00000006);
Xil_Out32(0xf6094428, 0x00000006);
Xil_Out32(0xf6094628, 0x00000006);
Xil_Out32(0xf6094244, 0x00000006);
Xil_Out32(0xf6094444, 0x00000006);
Xil_Out32(0xf6094644, 0x00000006);
Xil_Out32(0xf6094200, 0x00000002);
Xil_Out32(0xf6094204, 0x00000000);
Xil_Out32(0xf6094400, 0x00000002);
Xil_Out32(0xf6094404, 0x00000000);
Xil_Out32(0xf6094600, 0x00000002);
Xil_Out32(0xf6094604, 0x00000000);
Xil_Out32(0xf6094200, 0x00000004);
Xil_Out32(0xf6094204, 0x00000000);
Xil_Out32(0xf6094400, 0x00000004);
Xil_Out32(0xf6094404, 0x00000000);
Xil_Out32(0xf6094600, 0x00000004);
Xil_Out32(0xf6094604, 0x00000000);
Xil_Out32(0xf6094200, 0x00000101);
Xil_Out32(0xf6094204, 0x00000001);
Xil_Out32(0xf6094400, 0x00000101);
Xil_Out32(0xf6094404, 0x00000001);
Xil_Out32(0xf6094600, 0x00000101);
Xil_Out32(0xf6094604, 0x00000001);
//------------------------------nibble_5
Xil_Out32(0xf609520c, 0xf9e8d7c6);
Xil_Out32(0xf609540c, 0xf9e8d7c6);
Xil_Out32(0xf609560c, 0xf9e8d7c6);
Xil_Out32(0xf6095228, 0x00000006);
Xil_Out32(0xf6095428, 0x00000006);
Xil_Out32(0xf6095628, 0x00000006);
Xil_Out32(0xf6095244, 0x00000006);
Xil_Out32(0xf6095444, 0x00000006);
Xil_Out32(0xf6095644, 0x00000006);
Xil_Out32(0xf6095200, 0x00000002);
Xil_Out32(0xf6095204, 0x00000000);
Xil_Out32(0xf6095400, 0x00000002);
Xil_Out32(0xf6095404, 0x00000000);
Xil_Out32(0xf6095600, 0x00000002);
Xil_Out32(0xf6095604, 0x00000000);
Xil_Out32(0xf6095200, 0x00000004);
Xil_Out32(0xf6095204, 0x00000000);
Xil_Out32(0xf6095400, 0x00000004);
Xil_Out32(0xf6095404, 0x00000000);
Xil_Out32(0xf6095600, 0x00000004);
Xil_Out32(0xf6095604, 0x00000000);
Xil_Out32(0xf6095200, 0x00000101);
Xil_Out32(0xf6095204, 0x00000001);
Xil_Out32(0xf6095400, 0x00000101);
Xil_Out32(0xf6095404, 0x00000001);
Xil_Out32(0xf6095600, 0x00000101);
Xil_Out32(0xf6095604, 0x00000001);
//------------------------------nibble_6
Xil_Out32(0xf609620c, 0xf9e8d7c6);
Xil_Out32(0xf609640c, 0xf9e8d7c6);
Xil_Out32(0xf609660c, 0xf9e8d7c6);
Xil_Out32(0xf6096228, 0x00000006);
Xil_Out32(0xf6096428, 0x00000006);
Xil_Out32(0xf6096628, 0x00000006);
Xil_Out32(0xf6096244, 0x00000006);
Xil_Out32(0xf6096444, 0x00000006);
Xil_Out32(0xf6096644, 0x00000006);
Xil_Out32(0xf6096200, 0x00000002);
Xil_Out32(0xf6096204, 0x00000000);
Xil_Out32(0xf6096400, 0x00000002);
Xil_Out32(0xf6096404, 0x00000000);
Xil_Out32(0xf6096600, 0x00000002);
Xil_Out32(0xf6096604, 0x00000000);
Xil_Out32(0xf6096200, 0x00000004);
Xil_Out32(0xf6096204, 0x00000000);
Xil_Out32(0xf6096400, 0x00000004);
Xil_Out32(0xf6096404, 0x00000000);
Xil_Out32(0xf6096600, 0x00000004);
Xil_Out32(0xf6096604, 0x00000000);
Xil_Out32(0xf6096200, 0x00000101);
Xil_Out32(0xf6096204, 0x00000001);
Xil_Out32(0xf6096400, 0x00000101);
Xil_Out32(0xf6096404, 0x00000001);
Xil_Out32(0xf6096600, 0x00000101);
Xil_Out32(0xf6096604, 0x00000001);
//------------------------------nibble_7
Xil_Out32(0xf609720c, 0xf9e8d7c6);
Xil_Out32(0xf609740c, 0xf9e8d7c6);
Xil_Out32(0xf609760c, 0xf9e8d7c6);
Xil_Out32(0xf6097228, 0x00000006);
Xil_Out32(0xf6097428, 0x00000006);
Xil_Out32(0xf6097628, 0x00000006);
Xil_Out32(0xf6097244, 0x00000006);
Xil_Out32(0xf6097444, 0x00000006);
Xil_Out32(0xf6097644, 0x00000006);
Xil_Out32(0xf6097200, 0x00000002);
Xil_Out32(0xf6097204, 0x00000000);
Xil_Out32(0xf6097400, 0x00000002);
Xil_Out32(0xf6097404, 0x00000000);
Xil_Out32(0xf6097600, 0x00000002);
Xil_Out32(0xf6097604, 0x00000000);
Xil_Out32(0xf6097200, 0x00000004);
Xil_Out32(0xf6097204, 0x00000000);
Xil_Out32(0xf6097400, 0x00000004);
Xil_Out32(0xf6097404, 0x00000000);
Xil_Out32(0xf6097600, 0x00000004);
Xil_Out32(0xf6097604, 0x00000000);
Xil_Out32(0xf6097200, 0x00000101);
Xil_Out32(0xf6097204, 0x00000001);
Xil_Out32(0xf6097400, 0x00000101);
Xil_Out32(0xf6097404, 0x00000001);
Xil_Out32(0xf6097600, 0x00000101);
Xil_Out32(0xf6097604, 0x00000001);
//------------------------------nibble_8
Xil_Out32(0xf609820c, 0xf9e8d7c6);
Xil_Out32(0xf609840c, 0xf9e8d7c6);
Xil_Out32(0xf609860c, 0xf9e8d7c6);
Xil_Out32(0xf6098228, 0x00000006);
Xil_Out32(0xf6098428, 0x00000006);
Xil_Out32(0xf6098628, 0x00000006);
Xil_Out32(0xf6098244, 0x00000006);
Xil_Out32(0xf6098444, 0x00000006);
Xil_Out32(0xf6098644, 0x00000006);
Xil_Out32(0xf6098200, 0x00000002);
Xil_Out32(0xf6098204, 0x00000000);
Xil_Out32(0xf6098400, 0x00000002);
Xil_Out32(0xf6098404, 0x00000000);
Xil_Out32(0xf6098600, 0x00000002);
Xil_Out32(0xf6098604, 0x00000000);
Xil_Out32(0xf6098200, 0x00000004);
Xil_Out32(0xf6098204, 0x00000000);
Xil_Out32(0xf6098400, 0x00000004);
Xil_Out32(0xf6098404, 0x00000000);
Xil_Out32(0xf6098600, 0x00000004);
Xil_Out32(0xf6098604, 0x00000000);
Xil_Out32(0xf6098200, 0x00000101);
Xil_Out32(0xf6098204, 0x00000001);
Xil_Out32(0xf6098400, 0x00000101);
Xil_Out32(0xf6098404, 0x00000001);
Xil_Out32(0xf6098600, 0x00000101);
Xil_Out32(0xf6098604, 0x00000001);
//****************************************************
//XPIO_1
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf616020c, 0xf9e8d7c6);
Xil_Out32(0xf616040c, 0xf9e8d7c6);
Xil_Out32(0xf616060c, 0xf9e8d7c6);
Xil_Out32(0xf6160228, 0x00000006);
Xil_Out32(0xf6160428, 0x00000006);
Xil_Out32(0xf6160628, 0x00000006);
Xil_Out32(0xf6160244, 0x00000006);
Xil_Out32(0xf6160444, 0x00000006);
Xil_Out32(0xf6160644, 0x00000006);
Xil_Out32(0xf6160200, 0x00000002);
Xil_Out32(0xf6160204, 0x00000000);
Xil_Out32(0xf6160400, 0x00000002);
Xil_Out32(0xf6160404, 0x00000000);
Xil_Out32(0xf6160600, 0x00000002);
Xil_Out32(0xf6160604, 0x00000000);
Xil_Out32(0xf6160200, 0x00000004);
Xil_Out32(0xf6160204, 0x00000000);
Xil_Out32(0xf6160400, 0x00000004);
Xil_Out32(0xf6160404, 0x00000000);
Xil_Out32(0xf6160600, 0x00000004);
Xil_Out32(0xf6160604, 0x00000000);
Xil_Out32(0xf6160200, 0x00000101);
Xil_Out32(0xf6160204, 0x00000001);
Xil_Out32(0xf6160400, 0x00000101);
Xil_Out32(0xf6160404, 0x00000001);
Xil_Out32(0xf6160600, 0x00000101);
Xil_Out32(0xf6160604, 0x00000001);
//------------------------------nibble_1
Xil_Out32(0xf616120c, 0xf9e8d7c6);
Xil_Out32(0xf616140c, 0xf9e8d7c6);
Xil_Out32(0xf616160c, 0xf9e8d7c6);
Xil_Out32(0xf6161228, 0x00000006);
Xil_Out32(0xf6161428, 0x00000006);
Xil_Out32(0xf6161628, 0x00000006);
Xil_Out32(0xf6161244, 0x00000006);
Xil_Out32(0xf6161444, 0x00000006);
Xil_Out32(0xf6161644, 0x00000006);
Xil_Out32(0xf6161200, 0x00000002);
Xil_Out32(0xf6161204, 0x00000000);
Xil_Out32(0xf6161400, 0x00000002);
Xil_Out32(0xf6161404, 0x00000000);
Xil_Out32(0xf6161600, 0x00000002);
Xil_Out32(0xf6161604, 0x00000000);
Xil_Out32(0xf6161200, 0x00000004);
Xil_Out32(0xf6161204, 0x00000000);
Xil_Out32(0xf6161400, 0x00000004);
Xil_Out32(0xf6161404, 0x00000000);
Xil_Out32(0xf6161600, 0x00000004);
Xil_Out32(0xf6161604, 0x00000000);
Xil_Out32(0xf6161200, 0x00000101);
Xil_Out32(0xf6161204, 0x00000001);
Xil_Out32(0xf6161400, 0x00000101);
Xil_Out32(0xf6161404, 0x00000001);
Xil_Out32(0xf6161600, 0x00000101);
Xil_Out32(0xf6161604, 0x00000001);
//------------------------------nibble_2
Xil_Out32(0xf616220c, 0xf9e8d7c6);
Xil_Out32(0xf616240c, 0xf9e8d7c6);
Xil_Out32(0xf616260c, 0xf9e8d7c6);
Xil_Out32(0xf6162228, 0x00000006);
Xil_Out32(0xf6162428, 0x00000006);
Xil_Out32(0xf6162628, 0x00000006);
Xil_Out32(0xf6162244, 0x00000006);
Xil_Out32(0xf6162444, 0x00000006);
Xil_Out32(0xf6162644, 0x00000006);
Xil_Out32(0xf6162200, 0x00000002);
Xil_Out32(0xf6162204, 0x00000000);
Xil_Out32(0xf6162400, 0x00000002);
Xil_Out32(0xf6162404, 0x00000000);
Xil_Out32(0xf6162600, 0x00000002);
Xil_Out32(0xf6162604, 0x00000000);
Xil_Out32(0xf6162200, 0x00000004);
Xil_Out32(0xf6162204, 0x00000000);
Xil_Out32(0xf6162400, 0x00000004);
Xil_Out32(0xf6162404, 0x00000000);
Xil_Out32(0xf6162600, 0x00000004);
Xil_Out32(0xf6162604, 0x00000000);
Xil_Out32(0xf6162200, 0x00000101);
Xil_Out32(0xf6162204, 0x00000001);
Xil_Out32(0xf6162400, 0x00000101);
Xil_Out32(0xf6162404, 0x00000001);
Xil_Out32(0xf6162600, 0x00000101);
Xil_Out32(0xf6162604, 0x00000001);
//------------------------------nibble_3
Xil_Out32(0xf616320c, 0xf9e8d7c6);
Xil_Out32(0xf616340c, 0xf9e8d7c6);
Xil_Out32(0xf616360c, 0xf9e8d7c6);
Xil_Out32(0xf6163228, 0x00000006);
Xil_Out32(0xf6163428, 0x00000006);
Xil_Out32(0xf6163628, 0x00000006);
Xil_Out32(0xf6163244, 0x00000006);
Xil_Out32(0xf6163444, 0x00000006);
Xil_Out32(0xf6163644, 0x00000006);
Xil_Out32(0xf6163200, 0x00000002);
Xil_Out32(0xf6163204, 0x00000000);
Xil_Out32(0xf6163400, 0x00000002);
Xil_Out32(0xf6163404, 0x00000000);
Xil_Out32(0xf6163600, 0x00000002);
Xil_Out32(0xf6163604, 0x00000000);
Xil_Out32(0xf6163200, 0x00000004);
Xil_Out32(0xf6163204, 0x00000000);
Xil_Out32(0xf6163400, 0x00000004);
Xil_Out32(0xf6163404, 0x00000000);
Xil_Out32(0xf6163600, 0x00000004);
Xil_Out32(0xf6163604, 0x00000000);
Xil_Out32(0xf6163200, 0x00000101);
Xil_Out32(0xf6163204, 0x00000001);
Xil_Out32(0xf6163400, 0x00000101);
Xil_Out32(0xf6163404, 0x00000001);
Xil_Out32(0xf6163600, 0x00000101);
Xil_Out32(0xf6163604, 0x00000001);
//------------------------------nibble_4
Xil_Out32(0xf616420c, 0xf9e8d7c6);
Xil_Out32(0xf616440c, 0xf9e8d7c6);
Xil_Out32(0xf616460c, 0xf9e8d7c6);
Xil_Out32(0xf6164228, 0x00000006);
Xil_Out32(0xf6164428, 0x00000006);
Xil_Out32(0xf6164628, 0x00000006);
Xil_Out32(0xf6164244, 0x00000006);
Xil_Out32(0xf6164444, 0x00000006);
Xil_Out32(0xf6164644, 0x00000006);
Xil_Out32(0xf6164200, 0x00000002);
Xil_Out32(0xf6164204, 0x00000000);
Xil_Out32(0xf6164400, 0x00000002);
Xil_Out32(0xf6164404, 0x00000000);
Xil_Out32(0xf6164600, 0x00000002);
Xil_Out32(0xf6164604, 0x00000000);
Xil_Out32(0xf6164200, 0x00000004);
Xil_Out32(0xf6164204, 0x00000000);
Xil_Out32(0xf6164400, 0x00000004);
Xil_Out32(0xf6164404, 0x00000000);
Xil_Out32(0xf6164600, 0x00000004);
Xil_Out32(0xf6164604, 0x00000000);
Xil_Out32(0xf6164200, 0x00000101);
Xil_Out32(0xf6164204, 0x00000001);
Xil_Out32(0xf6164400, 0x00000101);
Xil_Out32(0xf6164404, 0x00000001);
Xil_Out32(0xf6164600, 0x00000101);
Xil_Out32(0xf6164604, 0x00000001);
//------------------------------nibble_5
Xil_Out32(0xf616520c, 0xf9e8d7c6);
Xil_Out32(0xf616540c, 0xf9e8d7c6);
Xil_Out32(0xf616560c, 0xf9e8d7c6);
Xil_Out32(0xf6165228, 0x00000006);
Xil_Out32(0xf6165428, 0x00000006);
Xil_Out32(0xf6165628, 0x00000006);
Xil_Out32(0xf6165244, 0x00000006);
Xil_Out32(0xf6165444, 0x00000006);
Xil_Out32(0xf6165644, 0x00000006);
Xil_Out32(0xf6165200, 0x00000002);
Xil_Out32(0xf6165204, 0x00000000);
Xil_Out32(0xf6165400, 0x00000002);
Xil_Out32(0xf6165404, 0x00000000);
Xil_Out32(0xf6165600, 0x00000002);
Xil_Out32(0xf6165604, 0x00000000);
Xil_Out32(0xf6165200, 0x00000004);
Xil_Out32(0xf6165204, 0x00000000);
Xil_Out32(0xf6165400, 0x00000004);
Xil_Out32(0xf6165404, 0x00000000);
Xil_Out32(0xf6165600, 0x00000004);
Xil_Out32(0xf6165604, 0x00000000);
Xil_Out32(0xf6165200, 0x00000101);
Xil_Out32(0xf6165204, 0x00000001);
Xil_Out32(0xf6165400, 0x00000101);
Xil_Out32(0xf6165404, 0x00000001);
Xil_Out32(0xf6165600, 0x00000101);
Xil_Out32(0xf6165604, 0x00000001);
//------------------------------nibble_6
Xil_Out32(0xf616620c, 0xf9e8d7c6);
Xil_Out32(0xf616640c, 0xf9e8d7c6);
Xil_Out32(0xf616660c, 0xf9e8d7c6);
Xil_Out32(0xf6166228, 0x00000006);
Xil_Out32(0xf6166428, 0x00000006);
Xil_Out32(0xf6166628, 0x00000006);
Xil_Out32(0xf6166244, 0x00000006);
Xil_Out32(0xf6166444, 0x00000006);
Xil_Out32(0xf6166644, 0x00000006);
Xil_Out32(0xf6166200, 0x00000002);
Xil_Out32(0xf6166204, 0x00000000);
Xil_Out32(0xf6166400, 0x00000002);
Xil_Out32(0xf6166404, 0x00000000);
Xil_Out32(0xf6166600, 0x00000002);
Xil_Out32(0xf6166604, 0x00000000);
Xil_Out32(0xf6166200, 0x00000004);
Xil_Out32(0xf6166204, 0x00000000);
Xil_Out32(0xf6166400, 0x00000004);
Xil_Out32(0xf6166404, 0x00000000);
Xil_Out32(0xf6166600, 0x00000004);
Xil_Out32(0xf6166604, 0x00000000);
Xil_Out32(0xf6166200, 0x00000101);
Xil_Out32(0xf6166204, 0x00000001);
Xil_Out32(0xf6166400, 0x00000101);
Xil_Out32(0xf6166404, 0x00000001);
Xil_Out32(0xf6166600, 0x00000101);
Xil_Out32(0xf6166604, 0x00000001);
//------------------------------nibble_7
Xil_Out32(0xf616720c, 0xf9e8d7c6);
Xil_Out32(0xf616740c, 0xf9e8d7c6);
Xil_Out32(0xf616760c, 0xf9e8d7c6);
Xil_Out32(0xf6167228, 0x00000006);
Xil_Out32(0xf6167428, 0x00000006);
Xil_Out32(0xf6167628, 0x00000006);
Xil_Out32(0xf6167244, 0x00000006);
Xil_Out32(0xf6167444, 0x00000006);
Xil_Out32(0xf6167644, 0x00000006);
Xil_Out32(0xf6167200, 0x00000002);
Xil_Out32(0xf6167204, 0x00000000);
Xil_Out32(0xf6167400, 0x00000002);
Xil_Out32(0xf6167404, 0x00000000);
Xil_Out32(0xf6167600, 0x00000002);
Xil_Out32(0xf6167604, 0x00000000);
Xil_Out32(0xf6167200, 0x00000004);
Xil_Out32(0xf6167204, 0x00000000);
Xil_Out32(0xf6167400, 0x00000004);
Xil_Out32(0xf6167404, 0x00000000);
Xil_Out32(0xf6167600, 0x00000004);
Xil_Out32(0xf6167604, 0x00000000);
Xil_Out32(0xf6167200, 0x00000101);
Xil_Out32(0xf6167204, 0x00000001);
Xil_Out32(0xf6167400, 0x00000101);
Xil_Out32(0xf6167404, 0x00000001);
Xil_Out32(0xf6167600, 0x00000101);
Xil_Out32(0xf6167604, 0x00000001);
//------------------------------nibble_8
Xil_Out32(0xf616820c, 0xf9e8d7c6);
Xil_Out32(0xf616840c, 0xf9e8d7c6);
Xil_Out32(0xf616860c, 0xf9e8d7c6);
Xil_Out32(0xf6168228, 0x00000006);
Xil_Out32(0xf6168428, 0x00000006);
Xil_Out32(0xf6168628, 0x00000006);
Xil_Out32(0xf6168244, 0x00000006);
Xil_Out32(0xf6168444, 0x00000006);
Xil_Out32(0xf6168644, 0x00000006);
Xil_Out32(0xf6168200, 0x00000002);
Xil_Out32(0xf6168204, 0x00000000);
Xil_Out32(0xf6168400, 0x00000002);
Xil_Out32(0xf6168404, 0x00000000);
Xil_Out32(0xf6168600, 0x00000002);
Xil_Out32(0xf6168604, 0x00000000);
Xil_Out32(0xf6168200, 0x00000004);
Xil_Out32(0xf6168204, 0x00000000);
Xil_Out32(0xf6168400, 0x00000004);
Xil_Out32(0xf6168404, 0x00000000);
Xil_Out32(0xf6168600, 0x00000004);
Xil_Out32(0xf6168604, 0x00000000);
Xil_Out32(0xf6168200, 0x00000101);
Xil_Out32(0xf6168204, 0x00000001);
Xil_Out32(0xf6168400, 0x00000101);
Xil_Out32(0xf6168404, 0x00000001);
Xil_Out32(0xf6168600, 0x00000101);
Xil_Out32(0xf6168604, 0x00000001);
//****************************************************
//XPIO_2
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf61c020c, 0xf9e8d7c6);
Xil_Out32(0xf61c040c, 0xf9e8d7c6);
Xil_Out32(0xf61c060c, 0xf9e8d7c6);
Xil_Out32(0xf61c0228, 0x00000006);
Xil_Out32(0xf61c0428, 0x00000006);
Xil_Out32(0xf61c0628, 0x00000006);
Xil_Out32(0xf61c0244, 0x00000006);
Xil_Out32(0xf61c0444, 0x00000006);
Xil_Out32(0xf61c0644, 0x00000006);
Xil_Out32(0xf61c0200, 0x00000002);
Xil_Out32(0xf61c0204, 0x00000000);
Xil_Out32(0xf61c0400, 0x00000002);
Xil_Out32(0xf61c0404, 0x00000000);
Xil_Out32(0xf61c0600, 0x00000002);
Xil_Out32(0xf61c0604, 0x00000000);
Xil_Out32(0xf61c0200, 0x00000004);
Xil_Out32(0xf61c0204, 0x00000000);
Xil_Out32(0xf61c0400, 0x00000004);
Xil_Out32(0xf61c0404, 0x00000000);
Xil_Out32(0xf61c0600, 0x00000004);
Xil_Out32(0xf61c0604, 0x00000000);
Xil_Out32(0xf61c0200, 0x00000101);
Xil_Out32(0xf61c0204, 0x00000001);
Xil_Out32(0xf61c0400, 0x00000101);
Xil_Out32(0xf61c0404, 0x00000001);
Xil_Out32(0xf61c0600, 0x00000101);
Xil_Out32(0xf61c0604, 0x00000001);
//------------------------------nibble_1
Xil_Out32(0xf61c120c, 0xf9e8d7c6);
Xil_Out32(0xf61c140c, 0xf9e8d7c6);
Xil_Out32(0xf61c160c, 0xf9e8d7c6);
Xil_Out32(0xf61c1228, 0x00000006);
Xil_Out32(0xf61c1428, 0x00000006);
Xil_Out32(0xf61c1628, 0x00000006);
Xil_Out32(0xf61c1244, 0x00000006);
Xil_Out32(0xf61c1444, 0x00000006);
Xil_Out32(0xf61c1644, 0x00000006);
Xil_Out32(0xf61c1200, 0x00000002);
Xil_Out32(0xf61c1204, 0x00000000);
Xil_Out32(0xf61c1400, 0x00000002);
Xil_Out32(0xf61c1404, 0x00000000);
Xil_Out32(0xf61c1600, 0x00000002);
Xil_Out32(0xf61c1604, 0x00000000);
Xil_Out32(0xf61c1200, 0x00000004);
Xil_Out32(0xf61c1204, 0x00000000);
Xil_Out32(0xf61c1400, 0x00000004);
Xil_Out32(0xf61c1404, 0x00000000);
Xil_Out32(0xf61c1600, 0x00000004);
Xil_Out32(0xf61c1604, 0x00000000);
Xil_Out32(0xf61c1200, 0x00000101);
Xil_Out32(0xf61c1204, 0x00000001);
Xil_Out32(0xf61c1400, 0x00000101);
Xil_Out32(0xf61c1404, 0x00000001);
Xil_Out32(0xf61c1600, 0x00000101);
Xil_Out32(0xf61c1604, 0x00000001);
//------------------------------nibble_2
Xil_Out32(0xf61c220c, 0xf9e8d7c6);
Xil_Out32(0xf61c240c, 0xf9e8d7c6);
Xil_Out32(0xf61c260c, 0xf9e8d7c6);
Xil_Out32(0xf61c2228, 0x00000006);
Xil_Out32(0xf61c2428, 0x00000006);
Xil_Out32(0xf61c2628, 0x00000006);
Xil_Out32(0xf61c2244, 0x00000006);
Xil_Out32(0xf61c2444, 0x00000006);
Xil_Out32(0xf61c2644, 0x00000006);
Xil_Out32(0xf61c2200, 0x00000002);
Xil_Out32(0xf61c2204, 0x00000000);
Xil_Out32(0xf61c2400, 0x00000002);
Xil_Out32(0xf61c2404, 0x00000000);
Xil_Out32(0xf61c2600, 0x00000002);
Xil_Out32(0xf61c2604, 0x00000000);
Xil_Out32(0xf61c2200, 0x00000004);
Xil_Out32(0xf61c2204, 0x00000000);
Xil_Out32(0xf61c2400, 0x00000004);
Xil_Out32(0xf61c2404, 0x00000000);
Xil_Out32(0xf61c2600, 0x00000004);
Xil_Out32(0xf61c2604, 0x00000000);
Xil_Out32(0xf61c2200, 0x00000101);
Xil_Out32(0xf61c2204, 0x00000001);
Xil_Out32(0xf61c2400, 0x00000101);
Xil_Out32(0xf61c2404, 0x00000001);
Xil_Out32(0xf61c2600, 0x00000101);
Xil_Out32(0xf61c2604, 0x00000001);
//------------------------------nibble_3
Xil_Out32(0xf61c320c, 0xf9e8d7c6);
Xil_Out32(0xf61c340c, 0xf9e8d7c6);
Xil_Out32(0xf61c360c, 0xf9e8d7c6);
Xil_Out32(0xf61c3228, 0x00000006);
Xil_Out32(0xf61c3428, 0x00000006);
Xil_Out32(0xf61c3628, 0x00000006);
Xil_Out32(0xf61c3244, 0x00000006);
Xil_Out32(0xf61c3444, 0x00000006);
Xil_Out32(0xf61c3644, 0x00000006);
Xil_Out32(0xf61c3200, 0x00000002);
Xil_Out32(0xf61c3204, 0x00000000);
Xil_Out32(0xf61c3400, 0x00000002);
Xil_Out32(0xf61c3404, 0x00000000);
Xil_Out32(0xf61c3600, 0x00000002);
Xil_Out32(0xf61c3604, 0x00000000);
Xil_Out32(0xf61c3200, 0x00000004);
Xil_Out32(0xf61c3204, 0x00000000);
Xil_Out32(0xf61c3400, 0x00000004);
Xil_Out32(0xf61c3404, 0x00000000);
Xil_Out32(0xf61c3600, 0x00000004);
Xil_Out32(0xf61c3604, 0x00000000);
Xil_Out32(0xf61c3200, 0x00000101);
Xil_Out32(0xf61c3204, 0x00000001);
Xil_Out32(0xf61c3400, 0x00000101);
Xil_Out32(0xf61c3404, 0x00000001);
Xil_Out32(0xf61c3600, 0x00000101);
Xil_Out32(0xf61c3604, 0x00000001);
//------------------------------nibble_4
Xil_Out32(0xf61c420c, 0xf9e8d7c6);
Xil_Out32(0xf61c440c, 0xf9e8d7c6);
Xil_Out32(0xf61c460c, 0xf9e8d7c6);
Xil_Out32(0xf61c4228, 0x00000006);
Xil_Out32(0xf61c4428, 0x00000006);
Xil_Out32(0xf61c4628, 0x00000006);
Xil_Out32(0xf61c4244, 0x00000006);
Xil_Out32(0xf61c4444, 0x00000006);
Xil_Out32(0xf61c4644, 0x00000006);
Xil_Out32(0xf61c4200, 0x00000002);
Xil_Out32(0xf61c4204, 0x00000000);
Xil_Out32(0xf61c4400, 0x00000002);
Xil_Out32(0xf61c4404, 0x00000000);
Xil_Out32(0xf61c4600, 0x00000002);
Xil_Out32(0xf61c4604, 0x00000000);
Xil_Out32(0xf61c4200, 0x00000004);
Xil_Out32(0xf61c4204, 0x00000000);
Xil_Out32(0xf61c4400, 0x00000004);
Xil_Out32(0xf61c4404, 0x00000000);
Xil_Out32(0xf61c4600, 0x00000004);
Xil_Out32(0xf61c4604, 0x00000000);
Xil_Out32(0xf61c4200, 0x00000101);
Xil_Out32(0xf61c4204, 0x00000001);
Xil_Out32(0xf61c4400, 0x00000101);
Xil_Out32(0xf61c4404, 0x00000001);
Xil_Out32(0xf61c4600, 0x00000101);
Xil_Out32(0xf61c4604, 0x00000001);
//------------------------------nibble_5
Xil_Out32(0xf61c520c, 0xf9e8d7c6);
Xil_Out32(0xf61c540c, 0xf9e8d7c6);
Xil_Out32(0xf61c560c, 0xf9e8d7c6);
Xil_Out32(0xf61c5228, 0x00000006);
Xil_Out32(0xf61c5428, 0x00000006);
Xil_Out32(0xf61c5628, 0x00000006);
Xil_Out32(0xf61c5244, 0x00000006);
Xil_Out32(0xf61c5444, 0x00000006);
Xil_Out32(0xf61c5644, 0x00000006);
Xil_Out32(0xf61c5200, 0x00000002);
Xil_Out32(0xf61c5204, 0x00000000);
Xil_Out32(0xf61c5400, 0x00000002);
Xil_Out32(0xf61c5404, 0x00000000);
Xil_Out32(0xf61c5600, 0x00000002);
Xil_Out32(0xf61c5604, 0x00000000);
Xil_Out32(0xf61c5200, 0x00000004);
Xil_Out32(0xf61c5204, 0x00000000);
Xil_Out32(0xf61c5400, 0x00000004);
Xil_Out32(0xf61c5404, 0x00000000);
Xil_Out32(0xf61c5600, 0x00000004);
Xil_Out32(0xf61c5604, 0x00000000);
Xil_Out32(0xf61c5200, 0x00000101);
Xil_Out32(0xf61c5204, 0x00000001);
Xil_Out32(0xf61c5400, 0x00000101);
Xil_Out32(0xf61c5404, 0x00000001);
Xil_Out32(0xf61c5600, 0x00000101);
Xil_Out32(0xf61c5604, 0x00000001);
//------------------------------nibble_6
Xil_Out32(0xf61c620c, 0xf9e8d7c6);
Xil_Out32(0xf61c640c, 0xf9e8d7c6);
Xil_Out32(0xf61c660c, 0xf9e8d7c6);
Xil_Out32(0xf61c6228, 0x00000006);
Xil_Out32(0xf61c6428, 0x00000006);
Xil_Out32(0xf61c6628, 0x00000006);
Xil_Out32(0xf61c6244, 0x00000006);
Xil_Out32(0xf61c6444, 0x00000006);
Xil_Out32(0xf61c6644, 0x00000006);
Xil_Out32(0xf61c6200, 0x00000002);
Xil_Out32(0xf61c6204, 0x00000000);
Xil_Out32(0xf61c6400, 0x00000002);
Xil_Out32(0xf61c6404, 0x00000000);
Xil_Out32(0xf61c6600, 0x00000002);
Xil_Out32(0xf61c6604, 0x00000000);
Xil_Out32(0xf61c6200, 0x00000004);
Xil_Out32(0xf61c6204, 0x00000000);
Xil_Out32(0xf61c6400, 0x00000004);
Xil_Out32(0xf61c6404, 0x00000000);
Xil_Out32(0xf61c6600, 0x00000004);
Xil_Out32(0xf61c6604, 0x00000000);
Xil_Out32(0xf61c6200, 0x00000101);
Xil_Out32(0xf61c6204, 0x00000001);
Xil_Out32(0xf61c6400, 0x00000101);
Xil_Out32(0xf61c6404, 0x00000001);
Xil_Out32(0xf61c6600, 0x00000101);
Xil_Out32(0xf61c6604, 0x00000001);
//------------------------------nibble_7
Xil_Out32(0xf61c720c, 0xf9e8d7c6);
Xil_Out32(0xf61c740c, 0xf9e8d7c6);
Xil_Out32(0xf61c760c, 0xf9e8d7c6);
Xil_Out32(0xf61c7228, 0x00000006);
Xil_Out32(0xf61c7428, 0x00000006);
Xil_Out32(0xf61c7628, 0x00000006);
Xil_Out32(0xf61c7244, 0x00000006);
Xil_Out32(0xf61c7444, 0x00000006);
Xil_Out32(0xf61c7644, 0x00000006);
Xil_Out32(0xf61c7200, 0x00000002);
Xil_Out32(0xf61c7204, 0x00000000);
Xil_Out32(0xf61c7400, 0x00000002);
Xil_Out32(0xf61c7404, 0x00000000);
Xil_Out32(0xf61c7600, 0x00000002);
Xil_Out32(0xf61c7604, 0x00000000);
Xil_Out32(0xf61c7200, 0x00000004);
Xil_Out32(0xf61c7204, 0x00000000);
Xil_Out32(0xf61c7400, 0x00000004);
Xil_Out32(0xf61c7404, 0x00000000);
Xil_Out32(0xf61c7600, 0x00000004);
Xil_Out32(0xf61c7604, 0x00000000);
Xil_Out32(0xf61c7200, 0x00000101);
Xil_Out32(0xf61c7204, 0x00000001);
Xil_Out32(0xf61c7400, 0x00000101);
Xil_Out32(0xf61c7404, 0x00000001);
Xil_Out32(0xf61c7600, 0x00000101);
Xil_Out32(0xf61c7604, 0x00000001);
//------------------------------nibble_8
Xil_Out32(0xf61c820c, 0xf9e8d7c6);
Xil_Out32(0xf61c840c, 0xf9e8d7c6);
Xil_Out32(0xf61c860c, 0xf9e8d7c6);
Xil_Out32(0xf61c8228, 0x00000006);
Xil_Out32(0xf61c8428, 0x00000006);
Xil_Out32(0xf61c8628, 0x00000006);
Xil_Out32(0xf61c8244, 0x00000006);
Xil_Out32(0xf61c8444, 0x00000006);
Xil_Out32(0xf61c8644, 0x00000006);
Xil_Out32(0xf61c8200, 0x00000002);
Xil_Out32(0xf61c8204, 0x00000000);
Xil_Out32(0xf61c8400, 0x00000002);
Xil_Out32(0xf61c8404, 0x00000000);
Xil_Out32(0xf61c8600, 0x00000002);
Xil_Out32(0xf61c8604, 0x00000000);
Xil_Out32(0xf61c8200, 0x00000004);
Xil_Out32(0xf61c8204, 0x00000000);
Xil_Out32(0xf61c8400, 0x00000004);
Xil_Out32(0xf61c8404, 0x00000000);
Xil_Out32(0xf61c8600, 0x00000004);
Xil_Out32(0xf61c8604, 0x00000000);
Xil_Out32(0xf61c8200, 0x00000101);
Xil_Out32(0xf61c8204, 0x00000001);
Xil_Out32(0xf61c8400, 0x00000101);
Xil_Out32(0xf61c8404, 0x00000001);
Xil_Out32(0xf61c8600, 0x00000101);
Xil_Out32(0xf61c8604, 0x00000001);
//*************************************************************************************************XPIO_IOBPAIR : end

//**************Register programming from  xpio_iobpair end----------------

dbg0_pmc(16387);
//**************Register programming from  xphy_pll start----------------

//************************************************************************************************* PLL_AND_PHY REGS 1: begin

Xil_Out32(0xf616980c, 0xf9e8d7c6);
prog_reg(0xF6169810, 0xB, 0x1800, 0x3);
prog_reg(0xF6169810, 0x8, 0x700, 0x3);
prog_reg(0xF6169810, 0x5, 0xE0, 0x0);
prog_reg(0xF6169810, 0x2, 0x1C, 0x0);
Xil_Out32(0xf6169800, 0x00001003);
Xil_Out32(0xf6169804, 0x00001001);
//************************************************************************************************* PLL_AND_PHY REGS 1: end

//************************************************************************************************* PLL_AND_PHY REGS 0: begin

Xil_Out32(0xf609980c, 0xf9e8d7c6);
prog_reg(0xF6099810, 0xB, 0x1800, 0x3);
prog_reg(0xF6099810, 0x8, 0x700, 0x2);
prog_reg(0xF6099810, 0x5, 0xE0, 0x1);
prog_reg(0xF6099810, 0x2, 0x1C, 0x5);
Xil_Out32(0xf6099800, 0x00001003);
Xil_Out32(0xf6099804, 0x00001001);
//************************************************************************************************* PLL_AND_PHY REGS 0: end

//************************************************************************************************* PLL_AND_PHY REGS 2: begin

Xil_Out32(0xf61c980c, 0xf9e8d7c6);
prog_reg(0xF61C9810, 0xB, 0x1800, 0x3);
prog_reg(0xF61C9810, 0x8, 0x700, 0x2);
prog_reg(0xF61C9810, 0x5, 0xE0, 0x1);
prog_reg(0xF61C9810, 0x2, 0x1C, 0x3);
Xil_Out32(0xf61c9800, 0x00001003);
Xil_Out32(0xf61c9804, 0x00001001);
//************************************************************************************************* PLL_AND_PHY REGS 2: end
#endif
//************************************************************************************************* XPIO_PLL : begin

Xil_Out32(0xf616900c, 0xf9e8d7c6);
prog_reg(0xF6169038, 0xC, 0x1000, 0x1);
prog_reg(0xF616903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF616903C, 0x0, 0xFF, 0x2);
prog_reg(0xF6169040, 0xC, 0x1000, 0x1);
prog_reg(0xF6169040, 0xB, 0x800, 0x0);
prog_reg(0xF6169038, 0xB, 0x800, 0x0);
prog_reg(0xF6169048, 0xB, 0x800, 0x0);
prog_reg(0xF6169050, 0xB, 0x800, 0x0);
prog_reg(0xF6169044, 0x8, 0xFF00, 0x2);
prog_reg(0xF6169044, 0x0, 0xFF, 0x2);
prog_reg(0xF6169048, 0xC, 0x1000, 0x1);
prog_reg(0xF616904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF616904C, 0x0, 0xFF, 0x2);
prog_reg(0xF6169030, 0xD, 0xE000, 0x0);
prog_reg(0xF6169050, 0xC, 0x1000, 0x1);
prog_reg(0xF6169054, 0x8, 0xFF00, 0x2);
prog_reg(0xF6169054, 0x0, 0xFF, 0x2);
prog_reg(0xF6169034, 0x8, 0xFF00, 0x10);
prog_reg(0xF6169034, 0x0, 0xFF, 0x10);
prog_reg(0xF6169030, 0x9, 0x200, 0x1);
prog_reg(0xF6169030, 0xA, 0xC00, 0x1);
prog_reg(0xF6169030, 0x0, 0xFF, 0x0);
prog_reg(0xF6169030, 0x8, 0x100, 0x0);
prog_reg(0xF6169030, 0xC, 0x1000, 0x0);
prog_reg(0xF6169088, 0x5, 0x20, 0x0);
prog_reg(0xF6169094, 0xA, 0x7C00, 0x3);
prog_reg(0xF6169098, 0x2, 0x7C, 0x3);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf616980c, 0xf9e8d7c6);
prog_reg(0xF6169810, 0xB, 0x1800, 0x3);
prog_reg(0xF6169810, 0x8, 0x700, 0x3);
prog_reg(0xF6169810, 0x5, 0xE0, 0x0);
prog_reg(0xF6169810, 0x2, 0x1C, 0x0);
prog_reg(0xF6169000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x6, 0x40, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x8, 0x100, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169000, 0x7, 0x80, 0x1);
Xil_Out32(0xf6169004, 0x00000000);
prog_reg(0xF6169800, 0x1, 0x2, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x6, 0x40, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x8, 0x100, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
prog_reg(0xF6169800, 0x7, 0x80, 0x1);
Xil_Out32(0xf6169804, 0x00000000);
#else //XPMCFW_HW90
Xil_Out32(0xf6169000, 0x00001043);
Xil_Out32(0xf6169004, 0x00001001);
#endif
dbg0_pmc(16400);
//polling XPIO_1
poll_for(0xf6169008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf6169004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL 1: end

//************************************************************************************************* XPIO_PLL 0: begin

Xil_Out32(0xf609900c, 0xf9e8d7c6);
prog_reg(0xF6099038, 0xC, 0x1000, 0x1);
prog_reg(0xF609903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF609903C, 0x0, 0xFF, 0x2);
prog_reg(0xF6099040, 0xC, 0x1000, 0x1);
prog_reg(0xF6099040, 0xB, 0x800, 0x0);
prog_reg(0xF6099038, 0xB, 0x800, 0x0);
prog_reg(0xF6099048, 0xB, 0x800, 0x0);
prog_reg(0xF6099050, 0xB, 0x800, 0x0);
prog_reg(0xF6099044, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099044, 0x0, 0xFF, 0x2);
prog_reg(0xF6099048, 0xC, 0x1000, 0x1);
prog_reg(0xF609904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF609904C, 0x0, 0xFF, 0x2);
prog_reg(0xF6099030, 0xD, 0xE000, 0x0);
prog_reg(0xF6099050, 0xC, 0x1000, 0x1);
prog_reg(0xF6099054, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099054, 0x0, 0xFF, 0x2);
prog_reg(0xF60990A0, 0xA, 0x7C00, 0xA);
prog_reg(0xF60990A4, 0xA, 0x7C00, 0xA);
prog_reg(0xF6099034, 0x8, 0xFF00, 0x2);
prog_reg(0xF6099034, 0x0, 0xFF, 0x2);
prog_reg(0xF6099030, 0x9, 0x200, 0x1);
prog_reg(0xF6099030, 0xA, 0xC00, 0x1);
prog_reg(0xF6099030, 0x0, 0xFF, 0x0);
prog_reg(0xF6099030, 0x8, 0x100, 0x0);
prog_reg(0xF6099030, 0xC, 0x1000, 0x0);
prog_reg(0xF6099088, 0x5, 0x20, 0x0);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf609980c, 0xf9e8d7c6);
prog_reg(0xF6099810, 0xB, 0x1800, 0x3);
prog_reg(0xF6099810, 0x8, 0x700, 0x2);
prog_reg(0xF6099810, 0x5, 0xE0, 0x1);
prog_reg(0xF6099810, 0x2, 0x1C, 0x5);
prog_reg(0xF6099000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x6, 0x40, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x8, 0x100, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099000, 0x7, 0x80, 0x1);
Xil_Out32(0xf6099004, 0x00000000);
prog_reg(0xF6099800, 0x1, 0x2, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x6, 0x40, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x8, 0x100, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
prog_reg(0xF6099800, 0x7, 0x80, 0x1);
Xil_Out32(0xf6099804, 0x00000000);
#else //XPMCFW_HW90
Xil_Out32(0xf6099000, 0x00001043);
Xil_Out32(0xf6099004, 0x00001001);
#endif
dbg0_pmc(16400);
//polling XPIO_0
poll_for(0xf6099008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf6099004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL 0: end

//************************************************************************************************* XPIO_PLL 2: begin

Xil_Out32(0xf61c900c, 0xf9e8d7c6);
prog_reg(0xF61C9038, 0xC, 0x1000, 0x1);
prog_reg(0xF61C903C, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C903C, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9040, 0xC, 0x1000, 0x1);
prog_reg(0xF61C9040, 0xB, 0x800, 0x0);
prog_reg(0xF61C9038, 0xB, 0x800, 0x0);
prog_reg(0xF61C9048, 0xB, 0x800, 0x0);
prog_reg(0xF61C9050, 0xB, 0x800, 0x0);
prog_reg(0xF61C9044, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9044, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9048, 0xC, 0x1000, 0x1);
prog_reg(0xF61C904C, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C904C, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9030, 0xD, 0xE000, 0x0);
prog_reg(0xF61C9050, 0xC, 0x1000, 0x1);
prog_reg(0xF61C9054, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9054, 0x0, 0xFF, 0x2);
prog_reg(0xF61C90A0, 0xA, 0x7C00, 0xA);
prog_reg(0xF61C90A4, 0xA, 0x7C00, 0xA);
prog_reg(0xF61C9034, 0x8, 0xFF00, 0x2);
prog_reg(0xF61C9034, 0x0, 0xFF, 0x2);
prog_reg(0xF61C9030, 0x9, 0x200, 0x1);
prog_reg(0xF61C9030, 0xA, 0xC00, 0x1);
prog_reg(0xF61C9030, 0x0, 0xFF, 0x0);
prog_reg(0xF61C9030, 0x8, 0x100, 0x0);
prog_reg(0xF61C9030, 0xC, 0x1000, 0x0);
prog_reg(0xF61C9088, 0x5, 0x20, 0x0);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c980c, 0xf9e8d7c6);
prog_reg(0xF61C9810, 0xB, 0x1800, 0x3);
prog_reg(0xF61C9810, 0x8, 0x700, 0x2);
prog_reg(0xF61C9810, 0x5, 0xE0, 0x1);
prog_reg(0xF61C9810, 0x2, 0x1C, 0x3);
prog_reg(0xF61C9000, 0x1, 0x2, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x6, 0x40, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x8, 0x100, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9000, 0x7, 0x80, 0x1);
Xil_Out32(0xf61c9004, 0x00000000);
prog_reg(0xF61C9800, 0x1, 0x2, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x6, 0x40, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x8, 0x100, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
prog_reg(0xF61C9800, 0x7, 0x80, 0x1);
Xil_Out32(0xf61c9804, 0x00000000);
#else //XPMCFW_HW90
Xil_Out32(0xf61c9000, 0x00001043);
Xil_Out32(0xf61c9004, 0x00001001);
#endif
dbg0_pmc(16400);
//polling XPIO_2
poll_for(0xf61c9008, 0x00000010, 0x00000004, 0x00000001);
Xil_Out32(0xf61c9004, 0x00000000);
dbg0_pmc(16401);
//************************************************************************************************* XPIO_PLL 2: end

//**************Register programming from  xphy_pll end----------------

//**************Register programming from  xphy_init start----------------

dbg0_pmc(16386);
//************************************************************************************************* XPHY_CFG : begin

//****************************************************
//XPIO_0
//****************************************************
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf609900c, 0xf9e8d7c6);
Xil_Out32(0xf6099000, 0x000011c2);
Xil_Out32(0xf6099004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf609020c, 0xf9e8d7c6);
Xil_Out32(0xf609040c, 0xf9e8d7c6);
Xil_Out32(0xf609060c, 0xf9e8d7c6);
Xil_Out32(0xf6090228, 0x00000006);
Xil_Out32(0xf6090428, 0x00000006);
Xil_Out32(0xf6090628, 0x00000006);
Xil_Out32(0xf6090244, 0x00000006);
Xil_Out32(0xf6090444, 0x00000006);
Xil_Out32(0xf6090644, 0x00000006);
#endif
Xil_Out32(0xf609000c, 0xf9e8d7c6);
Xil_Out32(0xf609001c, 0x000000b3);
Xil_Out32(0xf6090020, 0x0000020b);
Xil_Out32(0xf6090034, 0x00001420);
Xil_Out32(0xf609002c, 0x00000102);
Xil_Out32(0xf6090058, 0x0000003d);
Xil_Out32(0xf609004c, 0x00000100);
Xil_Out32(0xf6090040, 0x0000c03f);
Xil_Out32(0xf6090048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6090004, 0x000011fe);
Xil_Out32(0xf6090028, 0x00000000);
Xil_Out32(0xf60900dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf609120c, 0xf9e8d7c6);
Xil_Out32(0xf609140c, 0xf9e8d7c6);
Xil_Out32(0xf609160c, 0xf9e8d7c6);
Xil_Out32(0xf6091228, 0x00000006);
Xil_Out32(0xf6091428, 0x00000006);
Xil_Out32(0xf6091628, 0x00000006);
Xil_Out32(0xf6091244, 0x00000006);
Xil_Out32(0xf6091444, 0x00000006);
Xil_Out32(0xf6091644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6090028, 0x00000000);
Xil_Out32(0xf60900dc, 0x00000004);
Xil_Out32(0xf6090000, 0x00000002);
Xil_Out32(0xf6090004, 0x00000000);
Xil_Out32(0xf6090000, 0x00000040);
Xil_Out32(0xf6090004, 0x00000000);
Xil_Out32(0xf6090000, 0x00000004);
Xil_Out32(0xf6090004, 0x00000000);
Xil_Out32(0xf6090000, 0x00000080);
Xil_Out32(0xf6090004, 0x00000000);
Xil_Out32(0xf6090000, 0x00000101);
Xil_Out32(0xf6090004, 0x00000001);
//------------------------------nibble_1
#endif
Xil_Out32(0xf609100c, 0xf9e8d7c6);
Xil_Out32(0xf609101c, 0x000000b0);
Xil_Out32(0xf6091020, 0x0000020f);
Xil_Out32(0xf6091034, 0x00001420);
Xil_Out32(0xf6091058, 0x0000003f);
Xil_Out32(0xf609104c, 0x00000100);
Xil_Out32(0xf6091040, 0x0000c03f);
Xil_Out32(0xf6091048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6091004, 0x000011fe);
Xil_Out32(0xf6091028, 0x00000000);
Xil_Out32(0xf60910dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf609220c, 0xf9e8d7c6);
Xil_Out32(0xf609240c, 0xf9e8d7c6);
Xil_Out32(0xf609260c, 0xf9e8d7c6);
Xil_Out32(0xf6092228, 0x00000006);
Xil_Out32(0xf6092428, 0x00000006);
Xil_Out32(0xf6092628, 0x00000006);
Xil_Out32(0xf6092244, 0x00000006);
Xil_Out32(0xf6092444, 0x00000006);
Xil_Out32(0xf6092644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6091028, 0x00000000);
Xil_Out32(0xf60910dc, 0x00000004);
Xil_Out32(0xf6091000, 0x00000002);
Xil_Out32(0xf6091004, 0x00000000);
Xil_Out32(0xf6091000, 0x00000040);
Xil_Out32(0xf6091004, 0x00000000);
Xil_Out32(0xf6091000, 0x00000004);
Xil_Out32(0xf6091004, 0x00000000);
Xil_Out32(0xf6091000, 0x00000080);
Xil_Out32(0xf6091004, 0x00000000);
Xil_Out32(0xf6091000, 0x00000101);
Xil_Out32(0xf6091004, 0x00000001);
//------------------------------nibble_2
#endif
Xil_Out32(0xf609200c, 0xf9e8d7c6);
Xil_Out32(0xf6092020, 0x000002fc);
Xil_Out32(0xf6092034, 0x00001420);
Xil_Out32(0xf609202c, 0x0000013f);
Xil_Out32(0xf609204c, 0x00000100);
Xil_Out32(0xf6092040, 0x0000c03f);
Xil_Out32(0xf6092048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6092004, 0x000011fe);
Xil_Out32(0xf6092028, 0x00000000);
Xil_Out32(0xf60920dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf609320c, 0xf9e8d7c6);
Xil_Out32(0xf609340c, 0xf9e8d7c6);
Xil_Out32(0xf609360c, 0xf9e8d7c6);
Xil_Out32(0xf6093228, 0x00000006);
Xil_Out32(0xf6093428, 0x00000006);
Xil_Out32(0xf6093628, 0x00000006);
Xil_Out32(0xf6093244, 0x00000006);
Xil_Out32(0xf6093444, 0x00000006);
Xil_Out32(0xf6093644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6092028, 0x00000000);
Xil_Out32(0xf60920dc, 0x00000004);
Xil_Out32(0xf6092000, 0x00000002);
Xil_Out32(0xf6092004, 0x00000000);
Xil_Out32(0xf6092000, 0x00000040);
Xil_Out32(0xf6092004, 0x00000000);
Xil_Out32(0xf6092000, 0x00000004);
Xil_Out32(0xf6092004, 0x00000000);
Xil_Out32(0xf6092000, 0x00000080);
Xil_Out32(0xf6092004, 0x00000000);
Xil_Out32(0xf6092000, 0x00000101);
Xil_Out32(0xf6092004, 0x00000001);
//------------------------------nibble_3
#endif
Xil_Out32(0xf609300c, 0xf9e8d7c6);
Xil_Out32(0xf6093020, 0x000002fc);
Xil_Out32(0xf6093034, 0x00001420);
Xil_Out32(0xf609302c, 0x0000013c);
Xil_Out32(0xf609304c, 0x00000100);
Xil_Out32(0xf6093040, 0x0000c03f);
Xil_Out32(0xf6093048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6093004, 0x000011fe);
Xil_Out32(0xf6093028, 0x00000000);
Xil_Out32(0xf60930dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf609420c, 0xf9e8d7c6);
Xil_Out32(0xf609440c, 0xf9e8d7c6);
Xil_Out32(0xf609460c, 0xf9e8d7c6);
Xil_Out32(0xf6094228, 0x00000006);
Xil_Out32(0xf6094428, 0x00000006);
Xil_Out32(0xf6094628, 0x00000006);
Xil_Out32(0xf6094244, 0x00000006);
Xil_Out32(0xf6094444, 0x00000006);
Xil_Out32(0xf6094644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6093028, 0x00000000);
Xil_Out32(0xf60930dc, 0x00000004);
Xil_Out32(0xf6093000, 0x00000002);
Xil_Out32(0xf6093004, 0x00000000);
Xil_Out32(0xf6093000, 0x00000040);
Xil_Out32(0xf6093004, 0x00000000);
Xil_Out32(0xf6093000, 0x00000004);
Xil_Out32(0xf6093004, 0x00000000);
Xil_Out32(0xf6093000, 0x00000080);
Xil_Out32(0xf6093004, 0x00000000);
Xil_Out32(0xf6093000, 0x00000101);
Xil_Out32(0xf6093004, 0x00000001);
//------------------------------nibble_4
#endif
Xil_Out32(0xf609400c, 0xf9e8d7c6);
Xil_Out32(0xf6094020, 0x000002fc);
Xil_Out32(0xf6094034, 0x00001420);
Xil_Out32(0xf609402c, 0x0000013f);
Xil_Out32(0xf609404c, 0x00000100);
Xil_Out32(0xf6094040, 0x0000c03f);
Xil_Out32(0xf6094048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6094004, 0x000011fe);
Xil_Out32(0xf6094028, 0x00000000);
Xil_Out32(0xf60940dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf609520c, 0xf9e8d7c6);
Xil_Out32(0xf609540c, 0xf9e8d7c6);
Xil_Out32(0xf609560c, 0xf9e8d7c6);
Xil_Out32(0xf6095228, 0x00000006);
Xil_Out32(0xf6095428, 0x00000006);
Xil_Out32(0xf6095628, 0x00000006);
Xil_Out32(0xf6095244, 0x00000006);
Xil_Out32(0xf6095444, 0x00000006);
Xil_Out32(0xf6095644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6094028, 0x00000000);
Xil_Out32(0xf60940dc, 0x00000004);
Xil_Out32(0xf6094000, 0x00000002);
Xil_Out32(0xf6094004, 0x00000000);
Xil_Out32(0xf6094000, 0x00000040);
Xil_Out32(0xf6094004, 0x00000000);
Xil_Out32(0xf6094000, 0x00000004);
Xil_Out32(0xf6094004, 0x00000000);
Xil_Out32(0xf6094000, 0x00000080);
Xil_Out32(0xf6094004, 0x00000000);
Xil_Out32(0xf6094000, 0x00000101);
Xil_Out32(0xf6094004, 0x00000001);
//------------------------------nibble_5
#endif
Xil_Out32(0xf609500c, 0xf9e8d7c6);
Xil_Out32(0xf6095020, 0x000002fc);
Xil_Out32(0xf6095034, 0x00001420);
Xil_Out32(0xf609502c, 0x0000013f);
Xil_Out32(0xf609504c, 0x00000100);
Xil_Out32(0xf6095040, 0x0000c03f);
Xil_Out32(0xf6095048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6095004, 0x000011fe);
Xil_Out32(0xf6095028, 0x00000000);
Xil_Out32(0xf60950dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf609620c, 0xf9e8d7c6);
Xil_Out32(0xf609640c, 0xf9e8d7c6);
Xil_Out32(0xf609660c, 0xf9e8d7c6);
Xil_Out32(0xf6096228, 0x00000006);
Xil_Out32(0xf6096428, 0x00000006);
Xil_Out32(0xf6096628, 0x00000006);
Xil_Out32(0xf6096244, 0x00000006);
Xil_Out32(0xf6096444, 0x00000006);
Xil_Out32(0xf6096644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6095028, 0x00000000);
Xil_Out32(0xf60950dc, 0x00000004);
Xil_Out32(0xf6095000, 0x00000002);
Xil_Out32(0xf6095004, 0x00000000);
Xil_Out32(0xf6095000, 0x00000040);
Xil_Out32(0xf6095004, 0x00000000);
Xil_Out32(0xf6095000, 0x00000004);
Xil_Out32(0xf6095004, 0x00000000);
Xil_Out32(0xf6095000, 0x00000080);
Xil_Out32(0xf6095004, 0x00000000);
Xil_Out32(0xf6095000, 0x00000101);
Xil_Out32(0xf6095004, 0x00000001);
//------------------------------nibble_6
#endif
Xil_Out32(0xf609600c, 0xf9e8d7c6);
Xil_Out32(0xf6096020, 0x000002fc);
Xil_Out32(0xf6096034, 0x00001420);
Xil_Out32(0xf609602c, 0x0000013f);
Xil_Out32(0xf609604c, 0x00000100);
Xil_Out32(0xf6096040, 0x0000c03f);
Xil_Out32(0xf6096048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6096004, 0x000011fe);
Xil_Out32(0xf6096028, 0x00000000);
Xil_Out32(0xf60960dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf609720c, 0xf9e8d7c6);
Xil_Out32(0xf609740c, 0xf9e8d7c6);
Xil_Out32(0xf609760c, 0xf9e8d7c6);
Xil_Out32(0xf6097228, 0x00000006);
Xil_Out32(0xf6097428, 0x00000006);
Xil_Out32(0xf6097628, 0x00000006);
Xil_Out32(0xf6097244, 0x00000006);
Xil_Out32(0xf6097444, 0x00000006);
Xil_Out32(0xf6097644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6096028, 0x00000000);
Xil_Out32(0xf60960dc, 0x00000004);
Xil_Out32(0xf6096000, 0x00000002);
Xil_Out32(0xf6096004, 0x00000000);
Xil_Out32(0xf6096000, 0x00000040);
Xil_Out32(0xf6096004, 0x00000000);
Xil_Out32(0xf6096000, 0x00000004);
Xil_Out32(0xf6096004, 0x00000000);
Xil_Out32(0xf6096000, 0x00000080);
Xil_Out32(0xf6096004, 0x00000000);
Xil_Out32(0xf6096000, 0x00000101);
Xil_Out32(0xf6096004, 0x00000001);
//------------------------------nibble_7
#endif
Xil_Out32(0xf609700c, 0xf9e8d7c6);
Xil_Out32(0xf6097020, 0x000002fc);
Xil_Out32(0xf6097034, 0x00001420);
Xil_Out32(0xf609702c, 0x0000012f);
Xil_Out32(0xf609704c, 0x00000100);
Xil_Out32(0xf6097040, 0x0000c03f);
Xil_Out32(0xf6097048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6097004, 0x000011fe);
Xil_Out32(0xf6097028, 0x00000000);
Xil_Out32(0xf60970dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf609820c, 0xf9e8d7c6);
Xil_Out32(0xf609840c, 0xf9e8d7c6);
Xil_Out32(0xf609860c, 0xf9e8d7c6);
Xil_Out32(0xf6098228, 0x00000006);
Xil_Out32(0xf6098428, 0x00000006);
Xil_Out32(0xf6098628, 0x00000006);
Xil_Out32(0xf6098244, 0x00000006);
Xil_Out32(0xf6098444, 0x00000006);
Xil_Out32(0xf6098644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6097028, 0x00000000);
Xil_Out32(0xf60970dc, 0x00000004);
Xil_Out32(0xf6097000, 0x00000002);
Xil_Out32(0xf6097004, 0x00000000);
Xil_Out32(0xf6097000, 0x00000040);
Xil_Out32(0xf6097004, 0x00000000);
Xil_Out32(0xf6097000, 0x00000004);
Xil_Out32(0xf6097004, 0x00000000);
Xil_Out32(0xf6097000, 0x00000080);
Xil_Out32(0xf6097004, 0x00000000);
Xil_Out32(0xf6097000, 0x00000101);
Xil_Out32(0xf6097004, 0x00000001);
//------------------------------nibble_8
#endif
Xil_Out32(0xf609800c, 0xf9e8d7c6);
Xil_Out32(0xf6098020, 0x000002fc);
Xil_Out32(0xf6098034, 0x00001420);
Xil_Out32(0xf609802c, 0x0000013f);
Xil_Out32(0xf609804c, 0x00000100);
Xil_Out32(0xf6098040, 0x0000c03f);
Xil_Out32(0xf6098048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6098004, 0x000011fe);
#endif
Xil_Out32(0xf6098028, 0x00000000);
Xil_Out32(0xf60980dc, 0x00000004);
#ifdef XPMCFW_HW90
Xil_Out32(0xf6098000, 0x00000002);
#endif
Xil_Out32(0xf6098004, 0x00000000);
Xil_Out32(0xf6098000, 0x00000040);
Xil_Out32(0xf6098004, 0x00000000);
Xil_Out32(0xf6098000, 0x00000004);
Xil_Out32(0xf6098004, 0x00000000);
Xil_Out32(0xf6098000, 0x00000080);
Xil_Out32(0xf6098004, 0x00000000);
Xil_Out32(0xf6098000, 0x00000101);
Xil_Out32(0xf6098004, 0x00000001);
//****************************************************
//XPIO_1
//****************************************************
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf616900c, 0xf9e8d7c6);
Xil_Out32(0xf6169000, 0x000011c2);
Xil_Out32(0xf6169004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf616020c, 0xf9e8d7c6);
Xil_Out32(0xf616040c, 0xf9e8d7c6);
Xil_Out32(0xf616060c, 0xf9e8d7c6);
Xil_Out32(0xf6160228, 0x00000006);
Xil_Out32(0xf6160428, 0x00000006);
Xil_Out32(0xf6160628, 0x00000006);
Xil_Out32(0xf6160244, 0x00000006);
Xil_Out32(0xf6160444, 0x00000006);
Xil_Out32(0xf6160644, 0x00000006);
#endif
Xil_Out32(0xf616000c, 0xf9e8d7c6);
Xil_Out32(0xf616001c, 0x000000b3);
Xil_Out32(0xf6160020, 0x0000020b);
Xil_Out32(0xf6160034, 0x00001420);
Xil_Out32(0xf616002c, 0x00000102);
Xil_Out32(0xf6160058, 0x0000003d);
Xil_Out32(0xf616004c, 0x00000100);
Xil_Out32(0xf6160040, 0x0000c03f);
Xil_Out32(0xf6160048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6160004, 0x000011fe);
Xil_Out32(0xf6160028, 0x00000000);
Xil_Out32(0xf61600dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf616120c, 0xf9e8d7c6);
Xil_Out32(0xf616140c, 0xf9e8d7c6);
Xil_Out32(0xf616160c, 0xf9e8d7c6);
Xil_Out32(0xf6161228, 0x00000006);
Xil_Out32(0xf6161428, 0x00000006);
Xil_Out32(0xf6161628, 0x00000006);
Xil_Out32(0xf6161244, 0x00000006);
Xil_Out32(0xf6161444, 0x00000006);
Xil_Out32(0xf6161644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6160028, 0x00000000);
Xil_Out32(0xf61600dc, 0x00000004);
Xil_Out32(0xf6160000, 0x00000002);
Xil_Out32(0xf6160004, 0x00000000);
Xil_Out32(0xf6160000, 0x00000040);
Xil_Out32(0xf6160004, 0x00000000);
Xil_Out32(0xf6160000, 0x00000004);
Xil_Out32(0xf6160004, 0x00000000);
Xil_Out32(0xf6160000, 0x00000080);
Xil_Out32(0xf6160004, 0x00000000);
Xil_Out32(0xf6160000, 0x00000101);
Xil_Out32(0xf6160004, 0x00000001);
//------------------------------nibble_1
#endif
Xil_Out32(0xf616100c, 0xf9e8d7c6);
Xil_Out32(0xf616101c, 0x000000b0);
Xil_Out32(0xf6161020, 0x0000020f);
Xil_Out32(0xf6161034, 0x00001420);
Xil_Out32(0xf6161058, 0x0000003f);
Xil_Out32(0xf616104c, 0x00000100);
Xil_Out32(0xf6161040, 0x0000c03f);
Xil_Out32(0xf6161048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6161004, 0x000011fe);
Xil_Out32(0xf6161028, 0x00000000);
Xil_Out32(0xf61610dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf616220c, 0xf9e8d7c6);
Xil_Out32(0xf616240c, 0xf9e8d7c6);
Xil_Out32(0xf616260c, 0xf9e8d7c6);
Xil_Out32(0xf6162228, 0x00000006);
Xil_Out32(0xf6162428, 0x00000006);
Xil_Out32(0xf6162628, 0x00000006);
Xil_Out32(0xf6162244, 0x00000006);
Xil_Out32(0xf6162444, 0x00000006);
Xil_Out32(0xf6162644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6161028, 0x00000000);
Xil_Out32(0xf61610dc, 0x00000004);
Xil_Out32(0xf6161000, 0x00000002);
Xil_Out32(0xf6161004, 0x00000000);
Xil_Out32(0xf6161000, 0x00000040);
Xil_Out32(0xf6161004, 0x00000000);
Xil_Out32(0xf6161000, 0x00000004);
Xil_Out32(0xf6161004, 0x00000000);
Xil_Out32(0xf6161000, 0x00000080);
Xil_Out32(0xf6161004, 0x00000000);
Xil_Out32(0xf6161000, 0x00000101);
Xil_Out32(0xf6161004, 0x00000001);
//------------------------------nibble_2
#endif
Xil_Out32(0xf616200c, 0xf9e8d7c6);
Xil_Out32(0xf616201c, 0x000000b0);
Xil_Out32(0xf6162020, 0x0000020f);
Xil_Out32(0xf6162034, 0x00001420);
Xil_Out32(0xf6162058, 0x0000003f);
Xil_Out32(0xf616204c, 0x00000100);
Xil_Out32(0xf6162040, 0x0000c03f);
Xil_Out32(0xf6162048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6162004, 0x000011fe);
Xil_Out32(0xf6162028, 0x00000000);
Xil_Out32(0xf61620dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf616320c, 0xf9e8d7c6);
Xil_Out32(0xf616340c, 0xf9e8d7c6);
Xil_Out32(0xf616360c, 0xf9e8d7c6);
Xil_Out32(0xf6163228, 0x00000006);
Xil_Out32(0xf6163428, 0x00000006);
Xil_Out32(0xf6163628, 0x00000006);
Xil_Out32(0xf6163244, 0x00000006);
Xil_Out32(0xf6163444, 0x00000006);
Xil_Out32(0xf6163644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6162028, 0x00000000);
Xil_Out32(0xf61620dc, 0x00000004);
Xil_Out32(0xf6162000, 0x00000002);
Xil_Out32(0xf6162004, 0x00000000);
Xil_Out32(0xf6162000, 0x00000040);
Xil_Out32(0xf6162004, 0x00000000);
Xil_Out32(0xf6162000, 0x00000004);
Xil_Out32(0xf6162004, 0x00000000);
Xil_Out32(0xf6162000, 0x00000080);
Xil_Out32(0xf6162004, 0x00000000);
Xil_Out32(0xf6162000, 0x00000101);
Xil_Out32(0xf6162004, 0x00000001);
//------------------------------nibble_3
#endif
Xil_Out32(0xf616300c, 0xf9e8d7c6);
Xil_Out32(0xf616301c, 0x000000b3);
Xil_Out32(0xf6163020, 0x0000020b);
Xil_Out32(0xf6163034, 0x00001420);
Xil_Out32(0xf616302c, 0x00000102);
Xil_Out32(0xf6163058, 0x0000003d);
Xil_Out32(0xf616304c, 0x00000100);
Xil_Out32(0xf6163040, 0x0000c03f);
Xil_Out32(0xf6163048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6163004, 0x000011fe);
Xil_Out32(0xf6163028, 0x00000000);
Xil_Out32(0xf61630dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf616420c, 0xf9e8d7c6);
Xil_Out32(0xf616440c, 0xf9e8d7c6);
Xil_Out32(0xf616460c, 0xf9e8d7c6);
Xil_Out32(0xf6164228, 0x00000006);
Xil_Out32(0xf6164428, 0x00000006);
Xil_Out32(0xf6164628, 0x00000006);
Xil_Out32(0xf6164244, 0x00000006);
Xil_Out32(0xf6164444, 0x00000006);
Xil_Out32(0xf6164644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6163028, 0x00000000);
Xil_Out32(0xf61630dc, 0x00000004);
Xil_Out32(0xf6163000, 0x00000002);
Xil_Out32(0xf6163004, 0x00000000);
Xil_Out32(0xf6163000, 0x00000040);
Xil_Out32(0xf6163004, 0x00000000);
Xil_Out32(0xf6163000, 0x00000004);
Xil_Out32(0xf6163004, 0x00000000);
Xil_Out32(0xf6163000, 0x00000080);
Xil_Out32(0xf6163004, 0x00000000);
Xil_Out32(0xf6163000, 0x00000101);
Xil_Out32(0xf6163004, 0x00000001);
//------------------------------nibble_4
#endif
Xil_Out32(0xf616400c, 0xf9e8d7c6);
Xil_Out32(0xf616401c, 0x000000b3);
Xil_Out32(0xf6164020, 0x0000020b);
Xil_Out32(0xf6164034, 0x00001420);
Xil_Out32(0xf616402c, 0x00000102);
Xil_Out32(0xf6164058, 0x0000003d);
Xil_Out32(0xf616404c, 0x00000100);
Xil_Out32(0xf6164040, 0x0000c03f);
Xil_Out32(0xf6164048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6164004, 0x000011fe);
Xil_Out32(0xf6164028, 0x00000000);
Xil_Out32(0xf61640dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf616520c, 0xf9e8d7c6);
Xil_Out32(0xf616540c, 0xf9e8d7c6);
Xil_Out32(0xf616560c, 0xf9e8d7c6);
Xil_Out32(0xf6165228, 0x00000006);
Xil_Out32(0xf6165428, 0x00000006);
Xil_Out32(0xf6165628, 0x00000006);
Xil_Out32(0xf6165244, 0x00000006);
Xil_Out32(0xf6165444, 0x00000006);
Xil_Out32(0xf6165644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6164028, 0x00000000);
Xil_Out32(0xf61640dc, 0x00000004);
Xil_Out32(0xf6164000, 0x00000002);
Xil_Out32(0xf6164004, 0x00000000);
Xil_Out32(0xf6164000, 0x00000040);
Xil_Out32(0xf6164004, 0x00000000);
Xil_Out32(0xf6164000, 0x00000004);
Xil_Out32(0xf6164004, 0x00000000);
Xil_Out32(0xf6164000, 0x00000080);
Xil_Out32(0xf6164004, 0x00000000);
Xil_Out32(0xf6164000, 0x00000101);
Xil_Out32(0xf6164004, 0x00000001);
//------------------------------nibble_5
#endif
Xil_Out32(0xf616500c, 0xf9e8d7c6);
Xil_Out32(0xf616501c, 0x000000b0);
Xil_Out32(0xf6165020, 0x0000020f);
Xil_Out32(0xf6165034, 0x00001420);
Xil_Out32(0xf6165058, 0x0000003f);
Xil_Out32(0xf616504c, 0x00000100);
Xil_Out32(0xf6165040, 0x0000c03f);
Xil_Out32(0xf6165048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6165004, 0x000011fe);
Xil_Out32(0xf6165028, 0x00000000);
Xil_Out32(0xf61650dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf616620c, 0xf9e8d7c6);
Xil_Out32(0xf616640c, 0xf9e8d7c6);
Xil_Out32(0xf616660c, 0xf9e8d7c6);
Xil_Out32(0xf6166228, 0x00000006);
Xil_Out32(0xf6166428, 0x00000006);
Xil_Out32(0xf6166628, 0x00000006);
Xil_Out32(0xf6166244, 0x00000006);
Xil_Out32(0xf6166444, 0x00000006);
Xil_Out32(0xf6166644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6165028, 0x00000000);
Xil_Out32(0xf61650dc, 0x00000004);
Xil_Out32(0xf6165000, 0x00000002);
Xil_Out32(0xf6165004, 0x00000000);
Xil_Out32(0xf6165000, 0x00000040);
Xil_Out32(0xf6165004, 0x00000000);
Xil_Out32(0xf6165000, 0x00000004);
Xil_Out32(0xf6165004, 0x00000000);
Xil_Out32(0xf6165000, 0x00000080);
Xil_Out32(0xf6165004, 0x00000000);
Xil_Out32(0xf6165000, 0x00000101);
Xil_Out32(0xf6165004, 0x00000001);
//------------------------------nibble_6
#endif
Xil_Out32(0xf616600c, 0xf9e8d7c6);
Xil_Out32(0xf616601c, 0x000000b3);
Xil_Out32(0xf6166020, 0x0000020b);
Xil_Out32(0xf6166034, 0x00001420);
Xil_Out32(0xf616602c, 0x00000102);
Xil_Out32(0xf6166058, 0x0000003d);
Xil_Out32(0xf616604c, 0x00000100);
Xil_Out32(0xf6166040, 0x0000c03f);
Xil_Out32(0xf6166048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6166004, 0x000011fe);
Xil_Out32(0xf6166028, 0x00000000);
Xil_Out32(0xf61660dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf616720c, 0xf9e8d7c6);
Xil_Out32(0xf616740c, 0xf9e8d7c6);
Xil_Out32(0xf616760c, 0xf9e8d7c6);
Xil_Out32(0xf6167228, 0x00000006);
Xil_Out32(0xf6167428, 0x00000006);
Xil_Out32(0xf6167628, 0x00000006);
Xil_Out32(0xf6167244, 0x00000006);
Xil_Out32(0xf6167444, 0x00000006);
Xil_Out32(0xf6167644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6166028, 0x00000000);
Xil_Out32(0xf61660dc, 0x00000004);
Xil_Out32(0xf6166000, 0x00000002);
Xil_Out32(0xf6166004, 0x00000000);
Xil_Out32(0xf6166000, 0x00000040);
Xil_Out32(0xf6166004, 0x00000000);
Xil_Out32(0xf6166000, 0x00000004);
Xil_Out32(0xf6166004, 0x00000000);
Xil_Out32(0xf6166000, 0x00000080);
Xil_Out32(0xf6166004, 0x00000000);
Xil_Out32(0xf6166000, 0x00000101);
Xil_Out32(0xf6166004, 0x00000001);
//------------------------------nibble_7
#endif
Xil_Out32(0xf616700c, 0xf9e8d7c6);
Xil_Out32(0xf616701c, 0x000000b0);
Xil_Out32(0xf6167020, 0x0000020f);
Xil_Out32(0xf6167034, 0x00001420);
Xil_Out32(0xf6167058, 0x0000003f);
Xil_Out32(0xf616704c, 0x00000100);
Xil_Out32(0xf6167040, 0x0000c03f);
Xil_Out32(0xf6167048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6167004, 0x000011fe);
Xil_Out32(0xf6167028, 0x00000000);
Xil_Out32(0xf61670dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf616820c, 0xf9e8d7c6);
Xil_Out32(0xf616840c, 0xf9e8d7c6);
Xil_Out32(0xf616860c, 0xf9e8d7c6);
Xil_Out32(0xf6168228, 0x00000006);
Xil_Out32(0xf6168428, 0x00000006);
Xil_Out32(0xf6168628, 0x00000006);
Xil_Out32(0xf6168244, 0x00000006);
Xil_Out32(0xf6168444, 0x00000006);
Xil_Out32(0xf6168644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf6167028, 0x00000000);
Xil_Out32(0xf61670dc, 0x00000004);
Xil_Out32(0xf6167000, 0x00000002);
Xil_Out32(0xf6167004, 0x00000000);
Xil_Out32(0xf6167000, 0x00000040);
Xil_Out32(0xf6167004, 0x00000000);
Xil_Out32(0xf6167000, 0x00000004);
Xil_Out32(0xf6167004, 0x00000000);
Xil_Out32(0xf6167000, 0x00000080);
Xil_Out32(0xf6167004, 0x00000000);
Xil_Out32(0xf6167000, 0x00000101);
Xil_Out32(0xf6167004, 0x00000001);
//------------------------------nibble_8
#endif
Xil_Out32(0xf616800c, 0xf9e8d7c6);
Xil_Out32(0xf6168020, 0x000002fc);
Xil_Out32(0xf6168034, 0x00001420);
Xil_Out32(0xf616802c, 0x0000013b);
Xil_Out32(0xf6168058, 0x00000008);
Xil_Out32(0xf616804c, 0x00000100);
Xil_Out32(0xf6168040, 0x0000c03f);
Xil_Out32(0xf6168048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6168004, 0x000011fe);
#endif
Xil_Out32(0xf6168028, 0x00000000);
Xil_Out32(0xf61680dc, 0x00000004);
#ifdef XPMCFW_HW90
Xil_Out32(0xf6168000, 0x00000002);
Xil_Out32(0xf6168004, 0x00000000);
Xil_Out32(0xf6168000, 0x00000040);
Xil_Out32(0xf6168004, 0x00000000);
Xil_Out32(0xf6168000, 0x00000004);
Xil_Out32(0xf6168004, 0x00000000);
Xil_Out32(0xf6168000, 0x00000080);
Xil_Out32(0xf6168004, 0x00000000);
Xil_Out32(0xf6168000, 0x00000101);
Xil_Out32(0xf6168004, 0x00000001);
#endif

//****************************************************
//XPIO_2
//****************************************************
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c900c, 0xf9e8d7c6);
Xil_Out32(0xf61c9000, 0x000011c2);
Xil_Out32(0xf61c9004, 0x00001000);
//------------------------------nibble_0
Xil_Out32(0xf61c020c, 0xf9e8d7c6);
Xil_Out32(0xf61c040c, 0xf9e8d7c6);
Xil_Out32(0xf61c060c, 0xf9e8d7c6);
Xil_Out32(0xf61c0228, 0x00000006);
Xil_Out32(0xf61c0428, 0x00000006);
Xil_Out32(0xf61c0628, 0x00000006);
Xil_Out32(0xf61c0244, 0x00000006);
Xil_Out32(0xf61c0444, 0x00000006);
Xil_Out32(0xf61c0644, 0x00000006);
#endif
Xil_Out32(0xf61c000c, 0xf9e8d7c6);
Xil_Out32(0xf61c001c, 0x000000b0);
Xil_Out32(0xf61c0020, 0x0000020f);
Xil_Out32(0xf61c0034, 0x00001420);
Xil_Out32(0xf61c0058, 0x0000003f);
Xil_Out32(0xf61c004c, 0x00000100);
Xil_Out32(0xf61c0040, 0x0000c03f);
Xil_Out32(0xf61c0048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c0004, 0x000011fe);
Xil_Out32(0xf61c0028, 0x00000000);
Xil_Out32(0xf61c00dc, 0x00000004);
//------------------------------nibble_1
Xil_Out32(0xf61c120c, 0xf9e8d7c6);
Xil_Out32(0xf61c140c, 0xf9e8d7c6);
Xil_Out32(0xf61c160c, 0xf9e8d7c6);
Xil_Out32(0xf61c1228, 0x00000006);
Xil_Out32(0xf61c1428, 0x00000006);
Xil_Out32(0xf61c1628, 0x00000006);
Xil_Out32(0xf61c1244, 0x00000006);
Xil_Out32(0xf61c1444, 0x00000006);
Xil_Out32(0xf61c1644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c0028, 0x00000000);
Xil_Out32(0xf61c00dc, 0x00000004);
Xil_Out32(0xf61c0000, 0x00000002);
Xil_Out32(0xf61c0004, 0x00000000);
Xil_Out32(0xf61c0000, 0x00000040);
Xil_Out32(0xf61c0004, 0x00000000);
Xil_Out32(0xf61c0000, 0x00000004);
Xil_Out32(0xf61c0004, 0x00000000);
Xil_Out32(0xf61c0000, 0x00000080);
Xil_Out32(0xf61c0004, 0x00000000);
Xil_Out32(0xf61c0000, 0x00000101);
Xil_Out32(0xf61c0004, 0x00000001);
//------------------------------nibble_1
#endif
Xil_Out32(0xf61c100c, 0xf9e8d7c6);
Xil_Out32(0xf61c101c, 0x000000b3);
Xil_Out32(0xf61c1020, 0x0000020b);
Xil_Out32(0xf61c1034, 0x00001420);
Xil_Out32(0xf61c102c, 0x00000102);
Xil_Out32(0xf61c1058, 0x0000003d);
Xil_Out32(0xf61c104c, 0x00000100);
Xil_Out32(0xf61c1040, 0x0000c03f);
Xil_Out32(0xf61c1048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c1004, 0x000011fe);
Xil_Out32(0xf61c1028, 0x00000000);
Xil_Out32(0xf61c10dc, 0x00000004);
//------------------------------nibble_2
Xil_Out32(0xf61c220c, 0xf9e8d7c6);
Xil_Out32(0xf61c240c, 0xf9e8d7c6);
Xil_Out32(0xf61c260c, 0xf9e8d7c6);
Xil_Out32(0xf61c2228, 0x00000006);
Xil_Out32(0xf61c2428, 0x00000006);
Xil_Out32(0xf61c2628, 0x00000006);
Xil_Out32(0xf61c2244, 0x00000006);
Xil_Out32(0xf61c2444, 0x00000006);
Xil_Out32(0xf61c2644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c1028, 0x00000000);
Xil_Out32(0xf61c10dc, 0x00000004);
Xil_Out32(0xf61c1000, 0x00000002);
Xil_Out32(0xf61c1004, 0x00000000);
Xil_Out32(0xf61c1000, 0x00000040);
Xil_Out32(0xf61c1004, 0x00000000);
Xil_Out32(0xf61c1000, 0x00000004);
Xil_Out32(0xf61c1004, 0x00000000);
Xil_Out32(0xf61c1000, 0x00000080);
Xil_Out32(0xf61c1004, 0x00000000);
Xil_Out32(0xf61c1000, 0x00000101);
Xil_Out32(0xf61c1004, 0x00000001);
//------------------------------nibble_2
#endif
Xil_Out32(0xf61c200c, 0xf9e8d7c6);
Xil_Out32(0xf61c201c, 0x000000b3);
Xil_Out32(0xf61c2020, 0x0000020b);
Xil_Out32(0xf61c2034, 0x00001420);
Xil_Out32(0xf61c202c, 0x00000102);
Xil_Out32(0xf61c2058, 0x0000003d);
Xil_Out32(0xf61c204c, 0x00000100);
Xil_Out32(0xf61c2040, 0x0000c03f);
Xil_Out32(0xf61c2048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c2004, 0x000011fe);
Xil_Out32(0xf61c2028, 0x00000000);
Xil_Out32(0xf61c20dc, 0x00000004);
//------------------------------nibble_3
Xil_Out32(0xf61c320c, 0xf9e8d7c6);
Xil_Out32(0xf61c340c, 0xf9e8d7c6);
Xil_Out32(0xf61c360c, 0xf9e8d7c6);
Xil_Out32(0xf61c3228, 0x00000006);
Xil_Out32(0xf61c3428, 0x00000006);
Xil_Out32(0xf61c3628, 0x00000006);
Xil_Out32(0xf61c3244, 0x00000006);
Xil_Out32(0xf61c3444, 0x00000006);
Xil_Out32(0xf61c3644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c2028, 0x00000000);
Xil_Out32(0xf61c20dc, 0x00000004);
Xil_Out32(0xf61c2000, 0x00000002);
Xil_Out32(0xf61c2004, 0x00000000);
Xil_Out32(0xf61c2000, 0x00000040);
Xil_Out32(0xf61c2004, 0x00000000);
Xil_Out32(0xf61c2000, 0x00000004);
Xil_Out32(0xf61c2004, 0x00000000);
Xil_Out32(0xf61c2000, 0x00000080);
Xil_Out32(0xf61c2004, 0x00000000);
Xil_Out32(0xf61c2000, 0x00000101);
Xil_Out32(0xf61c2004, 0x00000001);
//------------------------------nibble_3
#endif
Xil_Out32(0xf61c300c, 0xf9e8d7c6);
Xil_Out32(0xf61c301c, 0x000000b0);
Xil_Out32(0xf61c3020, 0x0000020f);
Xil_Out32(0xf61c3034, 0x00001420);
Xil_Out32(0xf61c3058, 0x0000003f);
Xil_Out32(0xf61c304c, 0x00000100);
Xil_Out32(0xf61c3040, 0x0000c03f);
Xil_Out32(0xf61c3048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c3004, 0x000011fe);
Xil_Out32(0xf61c3028, 0x00000000);
Xil_Out32(0xf61c30dc, 0x00000004);
//------------------------------nibble_4
Xil_Out32(0xf61c420c, 0xf9e8d7c6);
Xil_Out32(0xf61c440c, 0xf9e8d7c6);
Xil_Out32(0xf61c460c, 0xf9e8d7c6);
Xil_Out32(0xf61c4228, 0x00000006);
Xil_Out32(0xf61c4428, 0x00000006);
Xil_Out32(0xf61c4628, 0x00000006);
Xil_Out32(0xf61c4244, 0x00000006);
Xil_Out32(0xf61c4444, 0x00000006);
Xil_Out32(0xf61c4644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c3028, 0x00000000);
Xil_Out32(0xf61c30dc, 0x00000004);
Xil_Out32(0xf61c3000, 0x00000002);
Xil_Out32(0xf61c3004, 0x00000000);
Xil_Out32(0xf61c3000, 0x00000040);
Xil_Out32(0xf61c3004, 0x00000000);
Xil_Out32(0xf61c3000, 0x00000004);
Xil_Out32(0xf61c3004, 0x00000000);
Xil_Out32(0xf61c3000, 0x00000080);
Xil_Out32(0xf61c3004, 0x00000000);
Xil_Out32(0xf61c3000, 0x00000101);
Xil_Out32(0xf61c3004, 0x00000001);
//------------------------------nibble_4
#endif
Xil_Out32(0xf61c400c, 0xf9e8d7c6);
Xil_Out32(0xf61c401c, 0x000000b3);
Xil_Out32(0xf61c4020, 0x0000020b);
Xil_Out32(0xf61c4034, 0x00001420);
Xil_Out32(0xf61c402c, 0x00000102);
Xil_Out32(0xf61c4058, 0x0000003d);
Xil_Out32(0xf61c404c, 0x00000100);
Xil_Out32(0xf61c4040, 0x0000c03f);
Xil_Out32(0xf61c4048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c4004, 0x000011fe);
Xil_Out32(0xf61c4028, 0x00000000);
Xil_Out32(0xf61c40dc, 0x00000004);
//------------------------------nibble_5
Xil_Out32(0xf61c520c, 0xf9e8d7c6);
Xil_Out32(0xf61c540c, 0xf9e8d7c6);
Xil_Out32(0xf61c560c, 0xf9e8d7c6);
Xil_Out32(0xf61c5228, 0x00000006);
Xil_Out32(0xf61c5428, 0x00000006);
Xil_Out32(0xf61c5628, 0x00000006);
Xil_Out32(0xf61c5244, 0x00000006);
Xil_Out32(0xf61c5444, 0x00000006);
Xil_Out32(0xf61c5644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c4028, 0x00000000);
Xil_Out32(0xf61c40dc, 0x00000004);
Xil_Out32(0xf61c4000, 0x00000002);
Xil_Out32(0xf61c4004, 0x00000000);
Xil_Out32(0xf61c4000, 0x00000040);
Xil_Out32(0xf61c4004, 0x00000000);
Xil_Out32(0xf61c4000, 0x00000004);
Xil_Out32(0xf61c4004, 0x00000000);
Xil_Out32(0xf61c4000, 0x00000080);
Xil_Out32(0xf61c4004, 0x00000000);
Xil_Out32(0xf61c4000, 0x00000101);
Xil_Out32(0xf61c4004, 0x00000001);
//------------------------------nibble_5
#endif
Xil_Out32(0xf61c500c, 0xf9e8d7c6);
Xil_Out32(0xf61c501c, 0x000000b0);
Xil_Out32(0xf61c5020, 0x0000020f);
Xil_Out32(0xf61c5034, 0x00001420);
Xil_Out32(0xf61c5058, 0x0000003f);
Xil_Out32(0xf61c504c, 0x00000100);
Xil_Out32(0xf61c5040, 0x0000c03f);
Xil_Out32(0xf61c5048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c5004, 0x000011fe);
Xil_Out32(0xf61c5028, 0x00000000);
Xil_Out32(0xf61c50dc, 0x00000004);
//------------------------------nibble_6
Xil_Out32(0xf61c620c, 0xf9e8d7c6);
Xil_Out32(0xf61c640c, 0xf9e8d7c6);
Xil_Out32(0xf61c660c, 0xf9e8d7c6);
Xil_Out32(0xf61c6228, 0x00000006);
Xil_Out32(0xf61c6428, 0x00000006);
Xil_Out32(0xf61c6628, 0x00000006);
Xil_Out32(0xf61c6244, 0x00000006);
Xil_Out32(0xf61c6444, 0x00000006);
Xil_Out32(0xf61c6644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c5028, 0x00000000);
Xil_Out32(0xf61c50dc, 0x00000004);
Xil_Out32(0xf61c5000, 0x00000002);
Xil_Out32(0xf61c5004, 0x00000000);
Xil_Out32(0xf61c5000, 0x00000040);
Xil_Out32(0xf61c5004, 0x00000000);
Xil_Out32(0xf61c5000, 0x00000004);
Xil_Out32(0xf61c5004, 0x00000000);
Xil_Out32(0xf61c5000, 0x00000080);
Xil_Out32(0xf61c5004, 0x00000000);
Xil_Out32(0xf61c5000, 0x00000101);
Xil_Out32(0xf61c5004, 0x00000001);
//------------------------------nibble_6
#endif
Xil_Out32(0xf61c600c, 0xf9e8d7c6);
Xil_Out32(0xf61c601c, 0x000000b3);
Xil_Out32(0xf61c6020, 0x0000020b);
Xil_Out32(0xf61c6034, 0x00001420);
Xil_Out32(0xf61c602c, 0x0000013f);
Xil_Out32(0xf61c6058, 0x0000003d);
Xil_Out32(0xf61c604c, 0x00000100);
Xil_Out32(0xf61c6040, 0x0000c03f);
Xil_Out32(0xf61c6048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c6004, 0x000011fe);
Xil_Out32(0xf61c6028, 0x00000000);
Xil_Out32(0xf61c60dc, 0x00000004);
//------------------------------nibble_7
Xil_Out32(0xf61c720c, 0xf9e8d7c6);
Xil_Out32(0xf61c740c, 0xf9e8d7c6);
Xil_Out32(0xf61c760c, 0xf9e8d7c6);
Xil_Out32(0xf61c7228, 0x00000006);
Xil_Out32(0xf61c7428, 0x00000006);
Xil_Out32(0xf61c7628, 0x00000006);
Xil_Out32(0xf61c7244, 0x00000006);
Xil_Out32(0xf61c7444, 0x00000006);
Xil_Out32(0xf61c7644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c6028, 0x00000000);
Xil_Out32(0xf61c60dc, 0x00000004);
Xil_Out32(0xf61c6000, 0x00000002);
Xil_Out32(0xf61c6004, 0x00000000);
Xil_Out32(0xf61c6000, 0x00000040);
Xil_Out32(0xf61c6004, 0x00000000);
Xil_Out32(0xf61c6000, 0x00000004);
Xil_Out32(0xf61c6004, 0x00000000);
Xil_Out32(0xf61c6000, 0x00000080);
Xil_Out32(0xf61c6004, 0x00000000);
Xil_Out32(0xf61c6000, 0x00000101);
Xil_Out32(0xf61c6004, 0x00000001);
//------------------------------nibble_7
#endif
Xil_Out32(0xf61c700c, 0xf9e8d7c6);
Xil_Out32(0xf61c701c, 0x000000b0);
Xil_Out32(0xf61c7020, 0x0000020f);
Xil_Out32(0xf61c7034, 0x00001420);
Xil_Out32(0xf61c702c, 0x0000013f);
Xil_Out32(0xf61c7058, 0x0000003f);
Xil_Out32(0xf61c704c, 0x00000100);
Xil_Out32(0xf61c7040, 0x0000c03f);
Xil_Out32(0xf61c7048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c7004, 0x000011fe);
Xil_Out32(0xf61c7028, 0x00000000);
Xil_Out32(0xf61c70dc, 0x00000004);
//------------------------------nibble_8
Xil_Out32(0xf61c820c, 0xf9e8d7c6);
Xil_Out32(0xf61c840c, 0xf9e8d7c6);
Xil_Out32(0xf61c860c, 0xf9e8d7c6);
Xil_Out32(0xf61c8228, 0x00000006);
Xil_Out32(0xf61c8428, 0x00000006);
Xil_Out32(0xf61c8628, 0x00000006);
Xil_Out32(0xf61c8244, 0x00000006);
Xil_Out32(0xf61c8444, 0x00000006);
Xil_Out32(0xf61c8644, 0x00000006);
#else //XPMCFW_HW90
Xil_Out32(0xf61c7028, 0x00000000);
Xil_Out32(0xf61c70dc, 0x00000004);
Xil_Out32(0xf61c7000, 0x00000002);
Xil_Out32(0xf61c7004, 0x00000000);
Xil_Out32(0xf61c7000, 0x00000040);
Xil_Out32(0xf61c7004, 0x00000000);
Xil_Out32(0xf61c7000, 0x00000004);
Xil_Out32(0xf61c7004, 0x00000000);
Xil_Out32(0xf61c7000, 0x00000080);
Xil_Out32(0xf61c7004, 0x00000000);
Xil_Out32(0xf61c7000, 0x00000101);
Xil_Out32(0xf61c7004, 0x00000001);
//------------------------------nibble_8
#endif
Xil_Out32(0xf61c800c, 0xf9e8d7c6);
Xil_Out32(0xf61c8020, 0x000002fc);
Xil_Out32(0xf61c8034, 0x00001420);
Xil_Out32(0xf61c802c, 0x0000013f);
Xil_Out32(0xf61c804c, 0x00000100);
Xil_Out32(0xf61c8040, 0x0000c03f);
Xil_Out32(0xf61c8048, 0x00000010);
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf61c8004, 0x000011fe);
Xil_Out32(0xf61c8028, 0x00000000);
Xil_Out32(0xf61c80dc, 0x00000004);
//****************************************************
//XPIO_0
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf6090000, 0x00000040);
Xil_Out32(0xf6090004, 0x000011be);
Xil_Out32(0xf6090000, 0x00000042);
Xil_Out32(0xf6090004, 0x000011bc);
Xil_Out32(0xf6090000, 0x00000142);
Xil_Out32(0xf6090004, 0x000010bc);
Xil_Out32(0xf6090000, 0x000001c2);
Xil_Out32(0xf6090004, 0x0000103c);
Xil_Out32(0xf6090200, 0x00000106);
Xil_Out32(0xf6090400, 0x00000106);
Xil_Out32(0xf6090600, 0x00000106);
Xil_Out32(0xf6090204, 0x000000f8);
Xil_Out32(0xf6090404, 0x000000f8);
Xil_Out32(0xf6090604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf6091000, 0x00000040);
Xil_Out32(0xf6091004, 0x000011be);
Xil_Out32(0xf6091000, 0x00000042);
Xil_Out32(0xf6091004, 0x000011bc);
Xil_Out32(0xf6091000, 0x00000142);
Xil_Out32(0xf6091004, 0x000010bc);
Xil_Out32(0xf6091000, 0x000001c2);
Xil_Out32(0xf6091004, 0x0000103c);
Xil_Out32(0xf6091200, 0x00000106);
Xil_Out32(0xf6091400, 0x00000106);
Xil_Out32(0xf6091600, 0x00000106);
Xil_Out32(0xf6091204, 0x000000f8);
Xil_Out32(0xf6091404, 0x000000f8);
Xil_Out32(0xf6091604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf6092000, 0x00000040);
Xil_Out32(0xf6092004, 0x000011be);
Xil_Out32(0xf6092000, 0x00000042);
Xil_Out32(0xf6092004, 0x000011bc);
Xil_Out32(0xf6092000, 0x00000142);
Xil_Out32(0xf6092004, 0x000010bc);
Xil_Out32(0xf6092000, 0x000001c2);
Xil_Out32(0xf6092004, 0x0000103c);
Xil_Out32(0xf6092200, 0x00000106);
Xil_Out32(0xf6092400, 0x00000106);
Xil_Out32(0xf6092600, 0x00000106);
Xil_Out32(0xf6092204, 0x000000f8);
Xil_Out32(0xf6092404, 0x000000f8);
Xil_Out32(0xf6092604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf6093000, 0x00000040);
Xil_Out32(0xf6093004, 0x000011be);
Xil_Out32(0xf6093000, 0x00000042);
Xil_Out32(0xf6093004, 0x000011bc);
Xil_Out32(0xf6093000, 0x00000142);
Xil_Out32(0xf6093004, 0x000010bc);
Xil_Out32(0xf6093000, 0x000001c2);
Xil_Out32(0xf6093004, 0x0000103c);
Xil_Out32(0xf6093200, 0x00000106);
Xil_Out32(0xf6093400, 0x00000106);
Xil_Out32(0xf6093600, 0x00000106);
Xil_Out32(0xf6093204, 0x000000f8);
Xil_Out32(0xf6093404, 0x000000f8);
Xil_Out32(0xf6093604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf6094000, 0x00000040);
Xil_Out32(0xf6094004, 0x000011be);
Xil_Out32(0xf6094000, 0x00000042);
Xil_Out32(0xf6094004, 0x000011bc);
Xil_Out32(0xf6094000, 0x00000142);
Xil_Out32(0xf6094004, 0x000010bc);
Xil_Out32(0xf6094000, 0x000001c2);
Xil_Out32(0xf6094004, 0x0000103c);
Xil_Out32(0xf6094200, 0x00000106);
Xil_Out32(0xf6094400, 0x00000106);
Xil_Out32(0xf6094600, 0x00000106);
Xil_Out32(0xf6094204, 0x000000f8);
Xil_Out32(0xf6094404, 0x000000f8);
Xil_Out32(0xf6094604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf6095000, 0x00000040);
Xil_Out32(0xf6095004, 0x000011be);
Xil_Out32(0xf6095000, 0x00000042);
Xil_Out32(0xf6095004, 0x000011bc);
Xil_Out32(0xf6095000, 0x00000142);
Xil_Out32(0xf6095004, 0x000010bc);
Xil_Out32(0xf6095000, 0x000001c2);
Xil_Out32(0xf6095004, 0x0000103c);
Xil_Out32(0xf6095200, 0x00000106);
Xil_Out32(0xf6095400, 0x00000106);
Xil_Out32(0xf6095600, 0x00000106);
Xil_Out32(0xf6095204, 0x000000f8);
Xil_Out32(0xf6095404, 0x000000f8);
Xil_Out32(0xf6095604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf6096000, 0x00000040);
Xil_Out32(0xf6096004, 0x000011be);
Xil_Out32(0xf6096000, 0x00000042);
Xil_Out32(0xf6096004, 0x000011bc);
Xil_Out32(0xf6096000, 0x00000142);
Xil_Out32(0xf6096004, 0x000010bc);
Xil_Out32(0xf6096000, 0x000001c2);
Xil_Out32(0xf6096004, 0x0000103c);
Xil_Out32(0xf6096200, 0x00000106);
Xil_Out32(0xf6096400, 0x00000106);
Xil_Out32(0xf6096600, 0x00000106);
Xil_Out32(0xf6096204, 0x000000f8);
Xil_Out32(0xf6096404, 0x000000f8);
Xil_Out32(0xf6096604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf6097000, 0x00000040);
Xil_Out32(0xf6097004, 0x000011be);
Xil_Out32(0xf6097000, 0x00000042);
Xil_Out32(0xf6097004, 0x000011bc);
Xil_Out32(0xf6097000, 0x00000142);
Xil_Out32(0xf6097004, 0x000010bc);
Xil_Out32(0xf6097000, 0x000001c2);
Xil_Out32(0xf6097004, 0x0000103c);
Xil_Out32(0xf6097200, 0x00000106);
Xil_Out32(0xf6097400, 0x00000106);
Xil_Out32(0xf6097600, 0x00000106);
Xil_Out32(0xf6097204, 0x000000f8);
Xil_Out32(0xf6097404, 0x000000f8);
Xil_Out32(0xf6097604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf6098000, 0x00000040);
Xil_Out32(0xf6098004, 0x000011be);
Xil_Out32(0xf6098000, 0x00000042);
Xil_Out32(0xf6098004, 0x000011bc);
Xil_Out32(0xf6098000, 0x00000142);
Xil_Out32(0xf6098004, 0x000010bc);
Xil_Out32(0xf6098000, 0x000001c2);
Xil_Out32(0xf6098004, 0x0000103c);
Xil_Out32(0xf6098200, 0x00000106);
Xil_Out32(0xf6098400, 0x00000106);
Xil_Out32(0xf6098600, 0x00000106);
Xil_Out32(0xf6098204, 0x000000f8);
Xil_Out32(0xf6098404, 0x000000f8);
Xil_Out32(0xf6098604, 0x000000f8);
//****************************************************
//XPIO_1
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf6160000, 0x00000040);
Xil_Out32(0xf6160004, 0x000011be);
Xil_Out32(0xf6160000, 0x00000042);
Xil_Out32(0xf6160004, 0x000011bc);
Xil_Out32(0xf6160000, 0x00000142);
Xil_Out32(0xf6160004, 0x000010bc);
Xil_Out32(0xf6160000, 0x000001c2);
Xil_Out32(0xf6160004, 0x0000103c);
Xil_Out32(0xf6160200, 0x00000106);
Xil_Out32(0xf6160400, 0x00000106);
Xil_Out32(0xf6160600, 0x00000106);
Xil_Out32(0xf6160204, 0x000000f8);
Xil_Out32(0xf6160404, 0x000000f8);
Xil_Out32(0xf6160604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf6161000, 0x00000040);
Xil_Out32(0xf6161004, 0x000011be);
Xil_Out32(0xf6161000, 0x00000042);
Xil_Out32(0xf6161004, 0x000011bc);
Xil_Out32(0xf6161000, 0x00000142);
Xil_Out32(0xf6161004, 0x000010bc);
Xil_Out32(0xf6161000, 0x000001c2);
Xil_Out32(0xf6161004, 0x0000103c);
Xil_Out32(0xf6161200, 0x00000106);
Xil_Out32(0xf6161400, 0x00000106);
Xil_Out32(0xf6161600, 0x00000106);
Xil_Out32(0xf6161204, 0x000000f8);
Xil_Out32(0xf6161404, 0x000000f8);
Xil_Out32(0xf6161604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf6162000, 0x00000040);
Xil_Out32(0xf6162004, 0x000011be);
Xil_Out32(0xf6162000, 0x00000042);
Xil_Out32(0xf6162004, 0x000011bc);
Xil_Out32(0xf6162000, 0x00000142);
Xil_Out32(0xf6162004, 0x000010bc);
Xil_Out32(0xf6162000, 0x000001c2);
Xil_Out32(0xf6162004, 0x0000103c);
Xil_Out32(0xf6162200, 0x00000106);
Xil_Out32(0xf6162400, 0x00000106);
Xil_Out32(0xf6162600, 0x00000106);
Xil_Out32(0xf6162204, 0x000000f8);
Xil_Out32(0xf6162404, 0x000000f8);
Xil_Out32(0xf6162604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf6163000, 0x00000040);
Xil_Out32(0xf6163004, 0x000011be);
Xil_Out32(0xf6163000, 0x00000042);
Xil_Out32(0xf6163004, 0x000011bc);
Xil_Out32(0xf6163000, 0x00000142);
Xil_Out32(0xf6163004, 0x000010bc);
Xil_Out32(0xf6163000, 0x000001c2);
Xil_Out32(0xf6163004, 0x0000103c);
Xil_Out32(0xf6163200, 0x00000106);
Xil_Out32(0xf6163400, 0x00000106);
Xil_Out32(0xf6163600, 0x00000106);
Xil_Out32(0xf6163204, 0x000000f8);
Xil_Out32(0xf6163404, 0x000000f8);
Xil_Out32(0xf6163604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf6164000, 0x00000040);
Xil_Out32(0xf6164004, 0x000011be);
Xil_Out32(0xf6164000, 0x00000042);
Xil_Out32(0xf6164004, 0x000011bc);
Xil_Out32(0xf6164000, 0x00000142);
Xil_Out32(0xf6164004, 0x000010bc);
Xil_Out32(0xf6164000, 0x000001c2);
Xil_Out32(0xf6164004, 0x0000103c);
Xil_Out32(0xf6164200, 0x00000106);
Xil_Out32(0xf6164400, 0x00000106);
Xil_Out32(0xf6164600, 0x00000106);
Xil_Out32(0xf6164204, 0x000000f8);
Xil_Out32(0xf6164404, 0x000000f8);
Xil_Out32(0xf6164604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf6165000, 0x00000040);
Xil_Out32(0xf6165004, 0x000011be);
Xil_Out32(0xf6165000, 0x00000042);
Xil_Out32(0xf6165004, 0x000011bc);
Xil_Out32(0xf6165000, 0x00000142);
Xil_Out32(0xf6165004, 0x000010bc);
Xil_Out32(0xf6165000, 0x000001c2);
Xil_Out32(0xf6165004, 0x0000103c);
Xil_Out32(0xf6165200, 0x00000106);
Xil_Out32(0xf6165400, 0x00000106);
Xil_Out32(0xf6165600, 0x00000106);
Xil_Out32(0xf6165204, 0x000000f8);
Xil_Out32(0xf6165404, 0x000000f8);
Xil_Out32(0xf6165604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf6166000, 0x00000040);
Xil_Out32(0xf6166004, 0x000011be);
Xil_Out32(0xf6166000, 0x00000042);
Xil_Out32(0xf6166004, 0x000011bc);
Xil_Out32(0xf6166000, 0x00000142);
Xil_Out32(0xf6166004, 0x000010bc);
Xil_Out32(0xf6166000, 0x000001c2);
Xil_Out32(0xf6166004, 0x0000103c);
Xil_Out32(0xf6166200, 0x00000106);
Xil_Out32(0xf6166400, 0x00000106);
Xil_Out32(0xf6166600, 0x00000106);
Xil_Out32(0xf6166204, 0x000000f8);
Xil_Out32(0xf6166404, 0x000000f8);
Xil_Out32(0xf6166604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf6167000, 0x00000040);
Xil_Out32(0xf6167004, 0x000011be);
Xil_Out32(0xf6167000, 0x00000042);
Xil_Out32(0xf6167004, 0x000011bc);
Xil_Out32(0xf6167000, 0x00000142);
Xil_Out32(0xf6167004, 0x000010bc);
Xil_Out32(0xf6167000, 0x000001c2);
Xil_Out32(0xf6167004, 0x0000103c);
Xil_Out32(0xf6167200, 0x00000106);
Xil_Out32(0xf6167400, 0x00000106);
Xil_Out32(0xf6167600, 0x00000106);
Xil_Out32(0xf6167204, 0x000000f8);
Xil_Out32(0xf6167404, 0x000000f8);
Xil_Out32(0xf6167604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf6168000, 0x00000040);
Xil_Out32(0xf6168004, 0x000011be);
Xil_Out32(0xf6168000, 0x00000042);
Xil_Out32(0xf6168004, 0x000011bc);
Xil_Out32(0xf6168000, 0x00000142);
Xil_Out32(0xf6168004, 0x000010bc);
Xil_Out32(0xf6168000, 0x000001c2);
Xil_Out32(0xf6168004, 0x0000103c);
Xil_Out32(0xf6168200, 0x00000106);
Xil_Out32(0xf6168400, 0x00000106);
Xil_Out32(0xf6168600, 0x00000106);
Xil_Out32(0xf6168204, 0x000000f8);
Xil_Out32(0xf6168404, 0x000000f8);
Xil_Out32(0xf6168604, 0x000000f8);
//****************************************************
//XPIO_2
//****************************************************
//------------------------------nibble_0
Xil_Out32(0xf61c0000, 0x00000040);
Xil_Out32(0xf61c0004, 0x000011be);
Xil_Out32(0xf61c0000, 0x00000042);
Xil_Out32(0xf61c0004, 0x000011bc);
Xil_Out32(0xf61c0000, 0x00000142);
Xil_Out32(0xf61c0004, 0x000010bc);
Xil_Out32(0xf61c0000, 0x000001c2);
Xil_Out32(0xf61c0004, 0x0000103c);
Xil_Out32(0xf61c0200, 0x00000106);
Xil_Out32(0xf61c0400, 0x00000106);
Xil_Out32(0xf61c0600, 0x00000106);
Xil_Out32(0xf61c0204, 0x000000f8);
Xil_Out32(0xf61c0404, 0x000000f8);
Xil_Out32(0xf61c0604, 0x000000f8);
//------------------------------nibble_1
Xil_Out32(0xf61c1000, 0x00000040);
Xil_Out32(0xf61c1004, 0x000011be);
Xil_Out32(0xf61c1000, 0x00000042);
Xil_Out32(0xf61c1004, 0x000011bc);
Xil_Out32(0xf61c1000, 0x00000142);
Xil_Out32(0xf61c1004, 0x000010bc);
Xil_Out32(0xf61c1000, 0x000001c2);
Xil_Out32(0xf61c1004, 0x0000103c);
Xil_Out32(0xf61c1200, 0x00000106);
Xil_Out32(0xf61c1400, 0x00000106);
Xil_Out32(0xf61c1600, 0x00000106);
Xil_Out32(0xf61c1204, 0x000000f8);
Xil_Out32(0xf61c1404, 0x000000f8);
Xil_Out32(0xf61c1604, 0x000000f8);
//------------------------------nibble_2
Xil_Out32(0xf61c2000, 0x00000040);
Xil_Out32(0xf61c2004, 0x000011be);
Xil_Out32(0xf61c2000, 0x00000042);
Xil_Out32(0xf61c2004, 0x000011bc);
Xil_Out32(0xf61c2000, 0x00000142);
Xil_Out32(0xf61c2004, 0x000010bc);
Xil_Out32(0xf61c2000, 0x000001c2);
Xil_Out32(0xf61c2004, 0x0000103c);
Xil_Out32(0xf61c2200, 0x00000106);
Xil_Out32(0xf61c2400, 0x00000106);
Xil_Out32(0xf61c2600, 0x00000106);
Xil_Out32(0xf61c2204, 0x000000f8);
Xil_Out32(0xf61c2404, 0x000000f8);
Xil_Out32(0xf61c2604, 0x000000f8);
//------------------------------nibble_3
Xil_Out32(0xf61c3000, 0x00000040);
Xil_Out32(0xf61c3004, 0x000011be);
Xil_Out32(0xf61c3000, 0x00000042);
Xil_Out32(0xf61c3004, 0x000011bc);
Xil_Out32(0xf61c3000, 0x00000142);
Xil_Out32(0xf61c3004, 0x000010bc);
Xil_Out32(0xf61c3000, 0x000001c2);
Xil_Out32(0xf61c3004, 0x0000103c);
Xil_Out32(0xf61c3200, 0x00000106);
Xil_Out32(0xf61c3400, 0x00000106);
Xil_Out32(0xf61c3600, 0x00000106);
Xil_Out32(0xf61c3204, 0x000000f8);
Xil_Out32(0xf61c3404, 0x000000f8);
Xil_Out32(0xf61c3604, 0x000000f8);
//------------------------------nibble_4
Xil_Out32(0xf61c4000, 0x00000040);
Xil_Out32(0xf61c4004, 0x000011be);
Xil_Out32(0xf61c4000, 0x00000042);
Xil_Out32(0xf61c4004, 0x000011bc);
Xil_Out32(0xf61c4000, 0x00000142);
Xil_Out32(0xf61c4004, 0x000010bc);
Xil_Out32(0xf61c4000, 0x000001c2);
Xil_Out32(0xf61c4004, 0x0000103c);
Xil_Out32(0xf61c4200, 0x00000106);
Xil_Out32(0xf61c4400, 0x00000106);
Xil_Out32(0xf61c4600, 0x00000106);
Xil_Out32(0xf61c4204, 0x000000f8);
Xil_Out32(0xf61c4404, 0x000000f8);
Xil_Out32(0xf61c4604, 0x000000f8);
//------------------------------nibble_5
Xil_Out32(0xf61c5000, 0x00000040);
Xil_Out32(0xf61c5004, 0x000011be);
Xil_Out32(0xf61c5000, 0x00000042);
Xil_Out32(0xf61c5004, 0x000011bc);
Xil_Out32(0xf61c5000, 0x00000142);
Xil_Out32(0xf61c5004, 0x000010bc);
Xil_Out32(0xf61c5000, 0x000001c2);
Xil_Out32(0xf61c5004, 0x0000103c);
Xil_Out32(0xf61c5200, 0x00000106);
Xil_Out32(0xf61c5400, 0x00000106);
Xil_Out32(0xf61c5600, 0x00000106);
Xil_Out32(0xf61c5204, 0x000000f8);
Xil_Out32(0xf61c5404, 0x000000f8);
Xil_Out32(0xf61c5604, 0x000000f8);
//------------------------------nibble_6
Xil_Out32(0xf61c6000, 0x00000040);
Xil_Out32(0xf61c6004, 0x000011be);
Xil_Out32(0xf61c6000, 0x00000042);
Xil_Out32(0xf61c6004, 0x000011bc);
Xil_Out32(0xf61c6000, 0x00000142);
Xil_Out32(0xf61c6004, 0x000010bc);
Xil_Out32(0xf61c6000, 0x000001c2);
Xil_Out32(0xf61c6004, 0x0000103c);
Xil_Out32(0xf61c6200, 0x00000106);
Xil_Out32(0xf61c6400, 0x00000106);
Xil_Out32(0xf61c6600, 0x00000106);
Xil_Out32(0xf61c6204, 0x000000f8);
Xil_Out32(0xf61c6404, 0x000000f8);
Xil_Out32(0xf61c6604, 0x000000f8);
//------------------------------nibble_7
Xil_Out32(0xf61c7000, 0x00000040);
Xil_Out32(0xf61c7004, 0x000011be);
Xil_Out32(0xf61c7000, 0x00000042);
Xil_Out32(0xf61c7004, 0x000011bc);
Xil_Out32(0xf61c7000, 0x00000142);
Xil_Out32(0xf61c7004, 0x000010bc);
Xil_Out32(0xf61c7000, 0x000001c2);
Xil_Out32(0xf61c7004, 0x0000103c);
Xil_Out32(0xf61c7200, 0x00000106);
Xil_Out32(0xf61c7400, 0x00000106);
Xil_Out32(0xf61c7600, 0x00000106);
Xil_Out32(0xf61c7204, 0x000000f8);
Xil_Out32(0xf61c7404, 0x000000f8);
Xil_Out32(0xf61c7604, 0x000000f8);
//------------------------------nibble_8
Xil_Out32(0xf61c8000, 0x00000040);
Xil_Out32(0xf61c8004, 0x000011be);
Xil_Out32(0xf61c8000, 0x00000042);
Xil_Out32(0xf61c8004, 0x000011bc);
Xil_Out32(0xf61c8000, 0x00000142);
Xil_Out32(0xf61c8004, 0x000010bc);
Xil_Out32(0xf61c8000, 0x000001c2);
Xil_Out32(0xf61c8004, 0x0000103c);
Xil_Out32(0xf61c8200, 0x00000106);
Xil_Out32(0xf61c8400, 0x00000106);
Xil_Out32(0xf61c8600, 0x00000106);
Xil_Out32(0xf61c8204, 0x000000f8);
Xil_Out32(0xf61c8404, 0x000000f8);
Xil_Out32(0xf61c8604, 0x000000f8);
#else //XPMCFW_HW90
Xil_Out32(0xf61c8028, 0x00000000);
Xil_Out32(0xf61c80dc, 0x00000004);
Xil_Out32(0xf61c8000, 0x00000002);
Xil_Out32(0xf61c8004, 0x00000000);
Xil_Out32(0xf61c8000, 0x00000040);
Xil_Out32(0xf61c8004, 0x00000000);
Xil_Out32(0xf61c8000, 0x00000004);
Xil_Out32(0xf61c8004, 0x00000000);
Xil_Out32(0xf61c8000, 0x00000080);
Xil_Out32(0xf61c8004, 0x00000000);
Xil_Out32(0xf61c8000, 0x00000101);
Xil_Out32(0xf61c8004, 0x00000001);
#endif
//*************************************************************************************************XPHY_CFG : end

//**************Register programming from  xphy_init end----------------

dbg0_pmc(16387);
//**************Register programming from  ddr_ub start----------------

dbg0_pmc(16388);
Xil_Out32(0xf6110000, 0x000000c2);
Xil_Out32(0xf6110004, 0x00000002);
Xil_Out32(0xf6110000, 0x01000000);
Xil_Out32(0xf6110004, 0x00000000);
#ifdef XPMCFW_HW70
prog_reg(0xF6110200, 0x2, 0x4, 0x1);
#else //XPMCFW_HW80 or XPMCFW_HW90
Xil_Out32(0xf6110000, 0x00000400);
Xil_Out32(0xf6110004, 0x00000400);
#endif
prog_reg(0xF6110200, 0x0, 0x3, 0x1);
prog_reg(0xF6110000, 0x1, 0x2, 0x1);
Xil_Out32(0xf6110004, 0x00000000);
Xil_Out32(0xf6130000, 0xb0000002);
Xil_Out32(0xf6130004, 0xb8080050);
Xil_Out32(0xf6130008, 0xb0000002);
Xil_Out32(0xf613000c, 0xb80804ac);
Xil_Out32(0xf6130010, 0xb0000002);
Xil_Out32(0xf6130014, 0xb8080930);
Xil_Out32(0xf6130020, 0xb0000002);
Xil_Out32(0xf6130024, 0xb80804b0);
Xil_Out32(0xf6130050, 0xb0000002);
Xil_Out32(0xf6130054, 0x30201288);
Xil_Out32(0xf6130058, 0xb0000002);
Xil_Out32(0xf613005c, 0x31601298);
Xil_Out32(0xf6130060, 0x940bc802);
Xil_Out32(0xf6130064, 0xb0000002);
Xil_Out32(0xf6130068, 0x31600e98);
Xil_Out32(0xf613006c, 0x940bc800);
Xil_Out32(0xf6130070, 0x20c00000);
Xil_Out32(0xf6130074, 0x20e00000);
Xil_Out32(0xf6130078, 0xb0000000);
Xil_Out32(0xf613007c, 0xb9f403f8);
Xil_Out32(0xf6130080, 0x20a00000);
Xil_Out32(0xf6130084, 0xb9f40008);
Xil_Out32(0xf6130088, 0x30a30000);
Xil_Out32(0xf613008c, 0xb8000000);
Xil_Out32(0xf6130090, 0x3021fff4);
Xil_Out32(0xf6130094, 0xfa610008);
Xil_Out32(0xf6130098, 0x12610000);
Xil_Out32(0xf613009c, 0xf8b30004);
Xil_Out32(0xf61300a0, 0xe8730004);
Xil_Out32(0xf61300a4, 0xe8630000);
Xil_Out32(0xf61300a8, 0x10330000);
Xil_Out32(0xf61300ac, 0xea610008);
Xil_Out32(0xf61300b0, 0x3021000c);
Xil_Out32(0xf61300b4, 0xb60f0008);
Xil_Out32(0xf61300b8, 0x80000000);
Xil_Out32(0xf61300bc, 0x3021ffec);
Xil_Out32(0xf61300c0, 0xfa610010);
Xil_Out32(0xf61300c4, 0x12610000);
Xil_Out32(0xf61300c8, 0xf8b30008);
Xil_Out32(0xf61300cc, 0xf8d3000c);
Xil_Out32(0xf61300d0, 0xe8730008);
Xil_Out32(0xf61300d4, 0xf8730004);
Xil_Out32(0xf61300d8, 0xe8730004);
Xil_Out32(0xf61300dc, 0xe893000c);
Xil_Out32(0xf61300e0, 0xf8830000);
Xil_Out32(0xf61300e4, 0x80000000);
Xil_Out32(0xf61300e8, 0x10330000);
Xil_Out32(0xf61300ec, 0xea610010);
Xil_Out32(0xf61300f0, 0x30210014);
Xil_Out32(0xf61300f4, 0xb60f0008);
Xil_Out32(0xf61300f8, 0x80000000);
Xil_Out32(0xf61300fc, 0x3021ffec);
Xil_Out32(0xf6130100, 0xf9e10000);
Xil_Out32(0xf6130104, 0xfa610010);
Xil_Out32(0xf6130108, 0x12610000);
Xil_Out32(0xf613010c, 0x30c0003c);
Xil_Out32(0xf6130110, 0xb0000008);
Xil_Out32(0xf6130114, 0x30a06c00);
Xil_Out32(0xf6130118, 0xb9f4ffa4);
Xil_Out32(0xf613011c, 0x80000000);
Xil_Out32(0xf6130120, 0x30c0003c);
Xil_Out32(0xf6130124, 0xb0000008);
Xil_Out32(0xf6130128, 0x30a06c04);
Xil_Out32(0xf613012c, 0xb9f4ff90);
Xil_Out32(0xf6130130, 0x80000000);
Xil_Out32(0xf6130134, 0x30c0003c);
Xil_Out32(0xf6130138, 0xb0000008);
Xil_Out32(0xf613013c, 0x30a07000);
Xil_Out32(0xf6130140, 0xb9f4ff7c);
Xil_Out32(0xf6130144, 0x80000000);
Xil_Out32(0xf6130148, 0x30c0003c);
Xil_Out32(0xf613014c, 0xb0000008);
Xil_Out32(0xf6130150, 0x30a07004);
Xil_Out32(0xf6130154, 0xb9f4ff68);
Xil_Out32(0xf6130158, 0x80000000);
Xil_Out32(0xf613015c, 0x30c0003c);
Xil_Out32(0xf6130160, 0xb0000008);
Xil_Out32(0xf6130164, 0x30a07400);
Xil_Out32(0xf6130168, 0xb9f4ff54);
Xil_Out32(0xf613016c, 0x80000000);
Xil_Out32(0xf6130170, 0x30c0003c);
Xil_Out32(0xf6130174, 0xb0000008);
Xil_Out32(0xf6130178, 0x30a07404);
Xil_Out32(0xf613017c, 0xb9f4ff40);
Xil_Out32(0xf6130180, 0x80000000);
Xil_Out32(0xf6130184, 0x30c0003c);
Xil_Out32(0xf6130188, 0xb0000008);
Xil_Out32(0xf613018c, 0x30a07800);
Xil_Out32(0xf6130190, 0xb9f4ff2c);
Xil_Out32(0xf6130194, 0x80000000);
Xil_Out32(0xf6130198, 0x30c0003c);
Xil_Out32(0xf613019c, 0xb0000008);
Xil_Out32(0xf61301a0, 0x30a07804);
Xil_Out32(0xf61301a4, 0xb9f4ff18);
Xil_Out32(0xf61301a8, 0x80000000);
Xil_Out32(0xf61301ac, 0x30c0003c);
Xil_Out32(0xf61301b0, 0xb0000008);
Xil_Out32(0xf61301b4, 0x30a07c00);
Xil_Out32(0xf61301b8, 0xb9f4ff04);
Xil_Out32(0xf61301bc, 0x80000000);
Xil_Out32(0xf61301c0, 0x30c0003c);
Xil_Out32(0xf61301c4, 0xb0000008);
Xil_Out32(0xf61301c8, 0x30a07c04);
Xil_Out32(0xf61301cc, 0xb9f4fef0);
Xil_Out32(0xf61301d0, 0x80000000);
Xil_Out32(0xf61301d4, 0x30c0003c);
Xil_Out32(0xf61301d8, 0xb0000008);
Xil_Out32(0xf61301dc, 0x30a08000);
Xil_Out32(0xf61301e0, 0xb9f4fedc);
Xil_Out32(0xf61301e4, 0x80000000);
Xil_Out32(0xf61301e8, 0x30c0003c);
Xil_Out32(0xf61301ec, 0xb0000008);
Xil_Out32(0xf61301f0, 0x30a08004);
Xil_Out32(0xf61301f4, 0xb9f4fec8);
Xil_Out32(0xf61301f8, 0x80000000);
Xil_Out32(0xf61301fc, 0x30c0ffff);
Xil_Out32(0xf6130200, 0xb000000a);
Xil_Out32(0xf6130204, 0x30a00000);
Xil_Out32(0xf6130208, 0xb9f4feb4);
Xil_Out32(0xf613020c, 0x80000000);
Xil_Out32(0xf6130210, 0x30c0ffff);
Xil_Out32(0xf6130214, 0xb000000a);
Xil_Out32(0xf6130218, 0x30a00004);
Xil_Out32(0xf613021c, 0xb9f4fea0);
Xil_Out32(0xf6130220, 0x80000000);
Xil_Out32(0xf6130224, 0x30c0ffff);
Xil_Out32(0xf6130228, 0xb000000a);
Xil_Out32(0xf613022c, 0x30a00008);
Xil_Out32(0xf6130230, 0xb9f4fe8c);
Xil_Out32(0xf6130234, 0x80000000);
Xil_Out32(0xf6130238, 0x30c0ffff);
Xil_Out32(0xf613023c, 0xb000000a);
Xil_Out32(0xf6130240, 0x30a0000c);
Xil_Out32(0xf6130244, 0xb9f4fe78);
Xil_Out32(0xf6130248, 0x80000000);
Xil_Out32(0xf613024c, 0x30c0ffff);
Xil_Out32(0xf6130250, 0xb000000a);
Xil_Out32(0xf6130254, 0x30a00010);
Xil_Out32(0xf6130258, 0xb9f4fe64);
Xil_Out32(0xf613025c, 0x80000000);
Xil_Out32(0xf6130260, 0x30c0ffff);
Xil_Out32(0xf6130264, 0xb000000a);
Xil_Out32(0xf6130268, 0x30a00014);
Xil_Out32(0xf613026c, 0xb9f4fe50);
Xil_Out32(0xf6130270, 0x80000000);
Xil_Out32(0xf6130274, 0x30c0ffff);
Xil_Out32(0xf6130278, 0xb000000a);
Xil_Out32(0xf613027c, 0x30a00018);
Xil_Out32(0xf6130280, 0xb9f4fe3c);
Xil_Out32(0xf6130284, 0x80000000);
Xil_Out32(0xf6130288, 0x30c0ffff);
Xil_Out32(0xf613028c, 0xb000000a);
Xil_Out32(0xf6130290, 0x30a0001c);
Xil_Out32(0xf6130294, 0xb9f4fe28);
Xil_Out32(0xf6130298, 0x80000000);
Xil_Out32(0xf613029c, 0xb0000009);
Xil_Out32(0xf61302a0, 0x30a0000c);
Xil_Out32(0xf61302a4, 0xb9f4fdec);
Xil_Out32(0xf61302a8, 0x80000000);
Xil_Out32(0xf61302ac, 0xf8730004);
Xil_Out32(0xf61302b0, 0xe8730004);
Xil_Out32(0xf61302b4, 0xa0630020);
Xil_Out32(0xf61302b8, 0xf8730008);
Xil_Out32(0xf61302bc, 0xe8d30008);
Xil_Out32(0xf61302c0, 0xb0000009);
Xil_Out32(0xf61302c4, 0x30a0000c);
Xil_Out32(0xf61302c8, 0xb9f4fdf4);
Xil_Out32(0xf61302cc, 0x80000000);
Xil_Out32(0xf61302d0, 0xe8d30004);
Xil_Out32(0xf61302d4, 0xb0000009);
Xil_Out32(0xf61302d8, 0x30a0000c);
Xil_Out32(0xf61302dc, 0xb9f4fde0);
Xil_Out32(0xf61302e0, 0x80000000);
Xil_Out32(0xf61302e4, 0xb0000009);
Xil_Out32(0xf61302e8, 0x30a000ac);
Xil_Out32(0xf61302ec, 0xb9f4fda4);
Xil_Out32(0xf61302f0, 0x80000000);
Xil_Out32(0xf61302f4, 0xf873000c);
Xil_Out32(0xf61302f8, 0xe873000c);
Xil_Out32(0xf61302fc, 0xa463fff7);
Xil_Out32(0xf6130300, 0xf873000c);
Xil_Out32(0xf6130304, 0xe8d3000c);
Xil_Out32(0xf6130308, 0xb0000009);
Xil_Out32(0xf613030c, 0x30a000ac);
Xil_Out32(0xf6130310, 0xb9f4fdac);
Xil_Out32(0xf6130314, 0x80000000);
Xil_Out32(0xf6130318, 0x30c0003c);
Xil_Out32(0xf613031c, 0xb0000008);
Xil_Out32(0xf6130320, 0x30a06c00);
Xil_Out32(0xf6130324, 0xb9f4fd98);
Xil_Out32(0xf6130328, 0x80000000);
Xil_Out32(0xf613032c, 0x10c00000);
Xil_Out32(0xf6130330, 0xb0000008);
Xil_Out32(0xf6130334, 0x30a06c04);
Xil_Out32(0xf6130338, 0xb9f4fd84);
Xil_Out32(0xf613033c, 0x80000000);
Xil_Out32(0xf6130340, 0x30c0003c);
Xil_Out32(0xf6130344, 0xb0000008);
Xil_Out32(0xf6130348, 0x30a07000);
Xil_Out32(0xf613034c, 0xb9f4fd70);
Xil_Out32(0xf6130350, 0x80000000);
Xil_Out32(0xf6130354, 0x10c00000);
Xil_Out32(0xf6130358, 0xb0000008);
Xil_Out32(0xf613035c, 0x30a07004);
Xil_Out32(0xf6130360, 0xb9f4fd5c);
Xil_Out32(0xf6130364, 0x80000000);
Xil_Out32(0xf6130368, 0x30c0003c);
Xil_Out32(0xf613036c, 0xb0000008);
Xil_Out32(0xf6130370, 0x30a07c00);
Xil_Out32(0xf6130374, 0xb9f4fd48);
Xil_Out32(0xf6130378, 0x80000000);
Xil_Out32(0xf613037c, 0x10c00000);
Xil_Out32(0xf6130380, 0xb0000008);
Xil_Out32(0xf6130384, 0x30a07c04);
Xil_Out32(0xf6130388, 0xb9f4fd34);
Xil_Out32(0xf613038c, 0x80000000);
Xil_Out32(0xf6130390, 0x30c0003c);
Xil_Out32(0xf6130394, 0xb0000008);
Xil_Out32(0xf6130398, 0x30a08000);
Xil_Out32(0xf613039c, 0xb9f4fd20);
Xil_Out32(0xf61303a0, 0x80000000);
Xil_Out32(0xf61303a4, 0x10c00000);
Xil_Out32(0xf61303a8, 0xb0000008);
Xil_Out32(0xf61303ac, 0x30a08004);
Xil_Out32(0xf61303b0, 0xb9f4fd0c);
Xil_Out32(0xf61303b4, 0x80000000);
Xil_Out32(0xf61303b8, 0x30c0003c);
Xil_Out32(0xf61303bc, 0xb0000008);
Xil_Out32(0xf61303c0, 0x30a07400);
Xil_Out32(0xf61303c4, 0xb9f4fcf8);
Xil_Out32(0xf61303c8, 0x80000000);
Xil_Out32(0xf61303cc, 0x10c00000);
Xil_Out32(0xf61303d0, 0xb0000008);
Xil_Out32(0xf61303d4, 0x30a07404);
Xil_Out32(0xf61303d8, 0xb9f4fce4);
Xil_Out32(0xf61303dc, 0x80000000);
Xil_Out32(0xf61303e0, 0x30c0003c);
Xil_Out32(0xf61303e4, 0xb0000008);
Xil_Out32(0xf61303e8, 0x30a07800);
Xil_Out32(0xf61303ec, 0xb9f4fcd0);
Xil_Out32(0xf61303f0, 0x80000000);
Xil_Out32(0xf61303f4, 0x10c00000);
Xil_Out32(0xf61303f8, 0xb0000008);
Xil_Out32(0xf61303fc, 0x30a07804);
Xil_Out32(0xf6130400, 0xb9f4fcbc);
Xil_Out32(0xf6130404, 0x80000000);
Xil_Out32(0xf6130408, 0xb000feed);
Xil_Out32(0xf613040c, 0x30c00001);
Xil_Out32(0xf6130410, 0xb0000001);
Xil_Out32(0xf6130414, 0x30a0fffc);
Xil_Out32(0xf6130418, 0xb9f4fca4);
Xil_Out32(0xf613041c, 0x80000000);
Xil_Out32(0xf6130420, 0x80000000);
Xil_Out32(0xf6130424, 0xe9e10000);
Xil_Out32(0xf6130428, 0x10330000);
Xil_Out32(0xf613042c, 0xea610010);
Xil_Out32(0xf6130430, 0x30210014);
Xil_Out32(0xf6130434, 0xb60f0008);
Xil_Out32(0xf6130438, 0x80000000);
Xil_Out32(0xf613043c, 0x3021ffe0);
Xil_Out32(0xf6130440, 0xf9e10000);
Xil_Out32(0xf6130444, 0xfa61001c);
Xil_Out32(0xf6130448, 0x12610000);
Xil_Out32(0xf613044c, 0x30a00009);
Xil_Out32(0xf6130450, 0xb9f4fcac);
Xil_Out32(0xf6130454, 0x80000000);
Xil_Out32(0xf6130458, 0x80000000);
Xil_Out32(0xf613045c, 0xe9e10000);
Xil_Out32(0xf6130460, 0x10330000);
Xil_Out32(0xf6130464, 0xea61001c);
Xil_Out32(0xf6130468, 0x30210020);
Xil_Out32(0xf613046c, 0xb60f0008);
Xil_Out32(0xf6130470, 0x80000000);
Xil_Out32(0xf6130474, 0x3021ffe0);
Xil_Out32(0xf6130478, 0xf9e10000);
Xil_Out32(0xf613047c, 0xfa61001c);
Xil_Out32(0xf6130480, 0x12610000);
Xil_Out32(0xf6130484, 0xb000ffff);
Xil_Out32(0xf6130488, 0xb9f4ffb4);
Xil_Out32(0xf613048c, 0x80000000);
Xil_Out32(0xf6130490, 0x10600000);
Xil_Out32(0xf6130494, 0xe9e10000);
Xil_Out32(0xf6130498, 0x10330000);
Xil_Out32(0xf613049c, 0xea61001c);
Xil_Out32(0xf61304a0, 0x30210020);
Xil_Out32(0xf61304a4, 0xb60f0008);
Xil_Out32(0xf61304a8, 0x80000000);
#ifdef XPMCFW_HW70
prog_reg(0xF6110200, 0x2, 0x4, 0x0);
#else //XPMCFW_HW80 or XPMCFW_HW90
Xil_Out32(0xf6110000, 0x00000400);
Xil_Out32(0xf6110004, 0x00000000);
#endif
prog_reg(0xF6110200, 0x0, 0x3, 0x0);
prog_reg(0xF6070758, 0x0, 0x1, 0x1);
dbg0_pmc(16392);
poll_for(0xf612fffc, 0xffffffff, 0x00000000, 0xfeed0001);
dbg0_pmc(16393);
//**************Register programming from  ddr_ub end----------------

dbg0_pmc(16389);
#if defined(XPMCFW_DDR64) || defined(XPMCFW_HW70)
//**************Register programming from ddrmc_init start----------------

dbg0_pmc(16390);
Xil_Out32(0xf6070034, 0x00000000); //reg_adec0
Xil_Out32(0xf6070038, 0x00000000); //reg_adec1
Xil_Out32(0xf607003c, 0x00100080); //reg_adec2
Xil_Out32(0xf6070040, 0x00000078); //reg_adec3
Xil_Out32(0xf6070044, 0x00000021); //reg_adec4
Xil_Out32(0xf6070048, 0x15513491); //reg_adec5
Xil_Out32(0xf607004c, 0x1a6585d6); //reg_adec6
Xil_Out32(0xf6070050, 0x1f79d71b); //reg_adec7
Xil_Out32(0xf6070054, 0x03000020); //reg_adec8
Xil_Out32(0xf6070058, 0x09207144); //reg_adec9
Xil_Out32(0xf607005c, 0x0f34c2ca); //reg_adec10
Xil_Out32(0xf6070060, 0x0000e190); //reg_adec11
#ifdef XPMCFW_HW70
Xil_Out32(0xf6070440, 0x000000f0); //reg_qos0
#else //XPMCFW_HW80 or XPMCFW_HW90
Xil_Out32(0xf6070440, 0x001e01e0); //reg_qos0
#endif
Xil_Out32(0xf6070444, 0x00100401); //reg_qos1
Xil_Out32(0xf6070448, 0x00000401); //reg_qos2
Xil_Out32(0xf60704b0, 0x01084210); //reg_cmdq_ctrl1
Xil_Out32(0xf6150200, 0x1a410404); //reg_safe_config0
Xil_Out32(0xf6150204, 0x0081c207); //reg_safe_config1
Xil_Out32(0xf6150208, 0x4252c3cf); //reg_safe_config2
Xil_Out32(0xf615020c, 0x02044899); //reg_safe_config3
Xil_Out32(0xf6150210, 0x250050e6); //reg_safe_config4
Xil_Out32(0xf6150214, 0x00c830c0); //reg_safe_config6
Xil_Out32(0xf6150218, 0x00040230); //reg_safe_config7
Xil_Out32(0xf615021c, 0x00000200); //reg_safe_config8
#if defined(XPMCFW_HW70) || defined(XPMCFW_HW80)
Xil_Out32(0xf6150220, 0x08ca4855); //reg_retry_0
Xil_Out32(0xf6150224, 0x145f6ac0); //reg_retry_1
#else //XPMCFW_HW90
Xil_Out32(0xf6150220, 0x074a4855); //reg_retry_0
Xil_Out32(0xf6150224, 0x14076ac0); //reg_retry_1
#endif
Xil_Out32(0xf615022c, 0x00009002); //reg_ref_3
Xil_Out32(0xf6150230, 0x00014248); //reg_com_3
Xil_Out32(0xf6150234, 0x0020079e); //reg_mrs_0
Xil_Out32(0xf615023c, 0x00000000); //reg_mrs_7
Xil_Out32(0xf615024c, 0x1001001d); //reg_rd_config
Xil_Out32(0xf6150250, 0x00006071); //reg_pt_config
Xil_Out32(0xf6150258, 0x00404210); //reg_config0
Xil_Out32(0xf615025c, 0x00000009); //reg_pinout
Xil_Out32(0xf6150268, 0x00000013); //seq_init_cmd_valid
Xil_Out32(0xf615026c, 0x0001901f); //seq_init_cmd0
Xil_Out32(0xf6150270, 0x0002585f); //seq_init_cmd1
Xil_Out32(0xf6150274, 0x0004807f); //seq_init_cmd2
Xil_Out32(0xf6150278, 0x00000268); //seq_init_cmd3
Xil_Out32(0xf615027c, 0x00001468); //seq_init_cmd4
Xil_Out32(0xf6150280, 0x00000268); //seq_init_cmd5
Xil_Out32(0xf6150284, 0x00001468); //seq_init_cmd6
Xil_Out32(0xf6150288, 0x00000268); //seq_init_cmd7
Xil_Out32(0xf615028c, 0x00001468); //seq_init_cmd8
Xil_Out32(0xf6150290, 0x00000268); //seq_init_cmd9
Xil_Out32(0xf6150294, 0x00001468); //seq_init_cmd10
Xil_Out32(0xf6150298, 0x00000268); //seq_init_cmd11
Xil_Out32(0xf615029c, 0x00001468); //seq_init_cmd12
Xil_Out32(0xf61502a0, 0x00000268); //seq_init_cmd13
Xil_Out32(0xf61502a4, 0x00001468); //seq_init_cmd14
Xil_Out32(0xf61502a8, 0x00000268); //seq_init_cmd15
Xil_Out32(0xf61502ac, 0x00003068); //seq_init_cmd16
Xil_Out32(0xf61502b0, 0x0000026e); //seq_init_cmd17
Xil_Out32(0xf61502b4, 0x0008006e); //seq_init_cmd18
Xil_Out32(0xf61502b8, 0x00000000); //seq_init_cmd19
Xil_Out32(0xf61502bc, 0x00000000); //seq_init_cmd20
Xil_Out32(0xf61502c0, 0x00000000); //seq_init_cmd21
Xil_Out32(0xf61502c4, 0x00000000); //seq_init_cmd22
Xil_Out32(0xf61502c8, 0x00000000); //seq_init_cmd23
Xil_Out32(0xf61502cc, 0x00000000); //seq_init_cmd24
Xil_Out32(0xf61502d0, 0x00000000); //seq_init_cmd25
Xil_Out32(0xf61502d4, 0x00000000); //seq_init_cmd26
Xil_Out32(0xf61502d8, 0x00000000); //seq_init_cmd27
Xil_Out32(0xf61502dc, 0x00000000); //seq_init_cmd28
Xil_Out32(0xf61502e0, 0x00000000); //seq_init_cmd29
Xil_Out32(0xf61502e4, 0x00000000); //seq_init_cmd30
Xil_Out32(0xf61502ec, 0x0000007f); //seq_init_cmd_set
Xil_Out32(0xf61502f0, 0x00000000); //seq_init_addr0
Xil_Out32(0xf61502f4, 0x00000000); //seq_init_addr1
Xil_Out32(0xf61502f8, 0x00000000); //seq_init_addr2
Xil_Out32(0xf61502fc, 0x03000400); //seq_init_addr3
Xil_Out32(0xf6150300, 0x03000400); //seq_init_addr4
Xil_Out32(0xf6150304, 0x06001000); //seq_init_addr5
Xil_Out32(0xf6150308, 0x06001000); //seq_init_addr6
Xil_Out32(0xf615030c, 0x05000500); //seq_init_addr7
Xil_Out32(0xf6150310, 0x05000500); //seq_init_addr8
Xil_Out32(0xf6150314, 0x04000000); //seq_init_addr9
Xil_Out32(0xf6150318, 0x04000000); //seq_init_addr10
Xil_Out32(0xf615031c, 0x02000028); //seq_init_addr11
Xil_Out32(0xf6150320, 0x02000028); //seq_init_addr12
Xil_Out32(0xf6150324, 0x01000001); //seq_init_addr13
Xil_Out32(0xf6150328, 0x01000001); //seq_init_addr14
Xil_Out32(0xf615032c, 0x00000d50); //seq_init_addr15
Xil_Out32(0xf6150330, 0x00000d50); //seq_init_addr16
Xil_Out32(0xf6150334, 0x00000524); //seq_init_addr17
Xil_Out32(0xf6150338, 0x00000524); //seq_init_addr18
Xil_Out32(0xf615033c, 0x00000000); //seq_init_addr19
Xil_Out32(0xf6150340, 0x00000000); //seq_init_addr20
Xil_Out32(0xf6150344, 0x00000000); //seq_init_addr21
Xil_Out32(0xf6150348, 0x00000000); //seq_init_addr22
Xil_Out32(0xf615034c, 0x00000000); //seq_init_addr23
Xil_Out32(0xf6150350, 0x00000000); //seq_init_addr24
Xil_Out32(0xf6150354, 0x00000000); //seq_init_addr25
Xil_Out32(0xf6150358, 0x00000000); //seq_init_addr26
Xil_Out32(0xf615035c, 0x00000000); //seq_init_addr27
Xil_Out32(0xf6150360, 0x00000000); //seq_init_addr28
Xil_Out32(0xf6150364, 0x00000000); //seq_init_addr29
Xil_Out32(0xf6150368, 0x00000000); //seq_init_addr30
Xil_Out32(0xf6150378, 0x00000001); //seq_init_cntrl0
Xil_Out32(0xf615037c, 0x00000001); //seq_init_cntrl1
Xil_Out32(0xf6150380, 0x00000000); //seq_init_cntrl2
Xil_Out32(0xf6150384, 0x00000010); //seq_init_cntrl3
Xil_Out32(0xf6150388, 0x00000022); //seq_init_cntrl4
Xil_Out32(0xf615038c, 0x00000010); //seq_init_cntrl5
Xil_Out32(0xf6150390, 0x00000022); //seq_init_cntrl6
Xil_Out32(0xf6150394, 0x00000010); //seq_init_cntrl7
Xil_Out32(0xf6150398, 0x00000022); //seq_init_cntrl8
Xil_Out32(0xf615039c, 0x00000010); //seq_init_cntrl9
Xil_Out32(0xf61503a0, 0x00000022); //seq_init_cntrl10
Xil_Out32(0xf61503a4, 0x00000010); //seq_init_cntrl11
Xil_Out32(0xf61503a8, 0x00000022); //seq_init_cntrl12
Xil_Out32(0xf61503ac, 0x00000010); //seq_init_cntrl13
Xil_Out32(0xf61503b0, 0x00000022); //seq_init_cntrl14
Xil_Out32(0xf61503b4, 0x00000010); //seq_init_cntrl15
Xil_Out32(0xf61503b8, 0x00000022); //seq_init_cntrl16
Xil_Out32(0xf61503bc, 0x00000010); //seq_init_cntrl17
Xil_Out32(0xf61503c0, 0x00000022); //seq_init_cntrl18
Xil_Out32(0xf61503c4, 0x00000010); //seq_init_cntrl19
Xil_Out32(0xf61503c8, 0x00000022); //seq_init_cntrl20
Xil_Out32(0xf61503cc, 0x00000010); //seq_init_cntrl21
Xil_Out32(0xf61503d0, 0x00000022); //seq_init_cntrl22
Xil_Out32(0xf61503d4, 0x00000010); //seq_init_cntrl23
Xil_Out32(0xf61503d8, 0x00000022); //seq_init_cntrl24
Xil_Out32(0xf61503dc, 0x00000010); //seq_init_cntrl25
Xil_Out32(0xf61503e0, 0x00000022); //seq_init_cntrl26
Xil_Out32(0xf61503e4, 0x00000010); //seq_init_cntrl27
Xil_Out32(0xf61503e8, 0x00000022); //seq_init_cntrl28
Xil_Out32(0xf61503ec, 0x00000010); //seq_init_cntrl29
Xil_Out32(0xf61503f0, 0x00000022); //seq_init_cntrl30
#ifdef XPMCFW_HW70
Xil_Out32(0xf6150734, 0x00101010); //txnq_rd_priority
Xil_Out32(0xf6150738, 0x00101010); //txnq_wr_priority
#else //XPMCFW_HW80 or XPMCFW_HW90
Xil_Out32(0xf6150734, 0x03202020); //txnq_rd_priority
Xil_Out32(0xf6150738, 0x01202020); //txnq_wr_priority
#endif
Xil_Out32(0xf6150800, 0x00001016); //reg_safe_config5
Xil_Out32(0xf6150820, 0x00000001); //reg_scrub9
Xil_Out32(0xf6150824, 0x00000006); //reg_config1
Xil_Out32(0xf615082c, 0x00000001); //cal_mode
Xil_Out32(0xf6150830, 0x00000016); //phy_rden0
Xil_Out32(0xf6150834, 0x00000016); //phy_rden1
Xil_Out32(0xf6150838, 0x00000016); //phy_rden2
Xil_Out32(0xf615083c, 0x00000016); //phy_rden3
Xil_Out32(0xf6150840, 0x00000016); //phy_rden4
Xil_Out32(0xf6150844, 0x00000016); //phy_rden5
Xil_Out32(0xf6150848, 0x00000016); //phy_rden6
Xil_Out32(0xf615084c, 0x00000016); //phy_rden7
Xil_Out32(0xf6150850, 0x00000016); //phy_rden8
Xil_Out32(0xf6150854, 0x00000016); //phy_rden9
Xil_Out32(0xf6150858, 0x00000016); //phy_rden10
Xil_Out32(0xf615085c, 0x00000016); //phy_rden11
Xil_Out32(0xf6150860, 0x00000016); //phy_rden12
Xil_Out32(0xf6150864, 0x00000016); //phy_rden13
Xil_Out32(0xf6150868, 0x00000016); //phy_rden14
Xil_Out32(0xf615086c, 0x00000016); //phy_rden15
Xil_Out32(0xf6150870, 0x00000016); //phy_rden16
Xil_Out32(0xf6150874, 0x00000016); //phy_rden17
Xil_Out32(0xf6150878, 0x00000016); //phy_rden18
Xil_Out32(0xf615087c, 0x00000016); //phy_rden19
Xil_Out32(0xf6150880, 0x00000016); //phy_rden20
Xil_Out32(0xf6150884, 0x00000016); //phy_rden21
Xil_Out32(0xf6150888, 0x00000016); //phy_rden22
Xil_Out32(0xf615088c, 0x00000016); //phy_rden23
Xil_Out32(0xf6150890, 0x00000016); //phy_rden24
Xil_Out32(0xf6150894, 0x00000016); //phy_rden25
Xil_Out32(0xf6150898, 0x00000016); //phy_rden26
Xil_Out32(0xf615089c, 0x0000000f); //fifo_rden
Xil_Out32(0xf61508ac, 0x00000000); //seq_mode
Xil_Out32(0xf61508b0, 0x0000007f); //seq_cmd_default
Xil_Out32(0xf61508d0, 0x00000000); //seq_ck_cal
Xil_Out32(0xf6150944, 0x00000410); //xpi_oe_all_nib
Xil_Out32(0xf6150948, 0x00000410); //xpi_wrdata_all_nib
Xil_Out32(0xf615094c, 0x00000101); //xpi_oe
Xil_Out32(0xf6150950, 0x000000c0); //xpi_pmi_config
Xil_Out32(0xf6150958, 0x00000001); //xpi_write_dm_dbi
Xil_Out32(0xf6150960, 0x000000a5); //default_pattern
Xil_Out32(0xf6150964, 0x00000000); //t_txbit
Xil_Out32(0xf6150c0c, 0x0000005f); //xpi_dqs
//**************Register programming from ddrmc_init end----------------

#endif
#if defined(XPMCFW_HW80) || defined(XPMCFW_HW90)
#ifdef XPMCFW_DDR32_ECC
//**************Register programming from ddrmc_init start----------------

dbg0_pmc(16390);
Xil_Out32(0xf6070034, 0x00000000); //reg_adec0
Xil_Out32(0xf6070038, 0x00000000); //reg_adec1
Xil_Out32(0xf607003c, 0x00100080); //reg_adec2
Xil_Out32(0xf6070040, 0x00000078); //reg_adec3
Xil_Out32(0xf6070044, 0x00000020); //reg_adec4
Xil_Out32(0xf6070048, 0x144d2450); //reg_adec5
Xil_Out32(0xf607004c, 0x19617595); //reg_adec6
Xil_Out32(0xf6070050, 0x1e75c6da); //reg_adec7
Xil_Out32(0xf6070054, 0x0200001f); //reg_adec8
Xil_Out32(0xf6070058, 0x081c6103); //reg_adec9
Xil_Out32(0xf607005c, 0x0e30b289); //reg_adec10
Xil_Out32(0xf6070060, 0x0000d14f); //reg_adec11
Xil_Out32(0xf6070440, 0x001e01e0); //reg_qos0
Xil_Out32(0xf6070444, 0x00100401); //reg_qos1
Xil_Out32(0xf6070448, 0x00000401); //reg_qos2
Xil_Out32(0xf60704b0, 0x01084210); //reg_cmdq_ctrl1
Xil_Out32(0xf6150200, 0x1a410404); //reg_safe_config0
Xil_Out32(0xf6150204, 0x0081c207); //reg_safe_config1
Xil_Out32(0xf6150208, 0x4252c3cf); //reg_safe_config2
Xil_Out32(0xf615020c, 0x02044899); //reg_safe_config3
Xil_Out32(0xf6150210, 0x250050e6); //reg_safe_config4
Xil_Out32(0xf6150214, 0x00c830c0); //reg_safe_config6
Xil_Out32(0xf6150218, 0x00040230); //reg_safe_config7
Xil_Out32(0xf615021c, 0x00000200); //reg_safe_config8
Xil_Out32(0xf6150220, 0x094a4855); //reg_retry_0
Xil_Out32(0xf6150224, 0x145f6ac0); //reg_retry_1
Xil_Out32(0xf615022c, 0x00009002); //reg_ref_3
Xil_Out32(0xf6150230, 0x00014248); //reg_com_3
Xil_Out32(0xf6150234, 0x0020079e); //reg_mrs_0
Xil_Out32(0xf615023c, 0x00000000); //reg_mrs_7
Xil_Out32(0xf615024c, 0x1001001d); //reg_rd_config
Xil_Out32(0xf6150250, 0x00006071); //reg_pt_config
Xil_Out32(0xf6150258, 0x00444210); //reg_config0
Xil_Out32(0xf615025c, 0x000000e9); //reg_pinout
Xil_Out32(0xf6150268, 0x00000013); //seq_init_cmd_valid
Xil_Out32(0xf615026c, 0x0001901f); //seq_init_cmd0
Xil_Out32(0xf6150270, 0x0002585f); //seq_init_cmd1
Xil_Out32(0xf6150274, 0x0004807f); //seq_init_cmd2
Xil_Out32(0xf6150278, 0x00000268); //seq_init_cmd3
Xil_Out32(0xf615027c, 0x00001468); //seq_init_cmd4
Xil_Out32(0xf6150280, 0x00000268); //seq_init_cmd5
Xil_Out32(0xf6150284, 0x00001468); //seq_init_cmd6
Xil_Out32(0xf6150288, 0x00000268); //seq_init_cmd7
Xil_Out32(0xf615028c, 0x00001468); //seq_init_cmd8
Xil_Out32(0xf6150290, 0x00000268); //seq_init_cmd9
Xil_Out32(0xf6150294, 0x00001468); //seq_init_cmd10
Xil_Out32(0xf6150298, 0x00000268); //seq_init_cmd11
Xil_Out32(0xf615029c, 0x00001468); //seq_init_cmd12
Xil_Out32(0xf61502a0, 0x00000268); //seq_init_cmd13
Xil_Out32(0xf61502a4, 0x00001468); //seq_init_cmd14
Xil_Out32(0xf61502a8, 0x00000268); //seq_init_cmd15
Xil_Out32(0xf61502ac, 0x00003068); //seq_init_cmd16
Xil_Out32(0xf61502b0, 0x0000026e); //seq_init_cmd17
Xil_Out32(0xf61502b4, 0x0008006e); //seq_init_cmd18
Xil_Out32(0xf61502b8, 0x00000000); //seq_init_cmd19
Xil_Out32(0xf61502bc, 0x00000000); //seq_init_cmd20
Xil_Out32(0xf61502c0, 0x00000000); //seq_init_cmd21
Xil_Out32(0xf61502c4, 0x00000000); //seq_init_cmd22
Xil_Out32(0xf61502c8, 0x00000000); //seq_init_cmd23
Xil_Out32(0xf61502cc, 0x00000000); //seq_init_cmd24
Xil_Out32(0xf61502d0, 0x00000000); //seq_init_cmd25
Xil_Out32(0xf61502d4, 0x00000000); //seq_init_cmd26
Xil_Out32(0xf61502d8, 0x00000000); //seq_init_cmd27
Xil_Out32(0xf61502dc, 0x00000000); //seq_init_cmd28
Xil_Out32(0xf61502e0, 0x00000000); //seq_init_cmd29
Xil_Out32(0xf61502e4, 0x00000000); //seq_init_cmd30
Xil_Out32(0xf61502ec, 0x0000007f); //seq_init_cmd_set
Xil_Out32(0xf61502f0, 0x00000000); //seq_init_addr0
Xil_Out32(0xf61502f4, 0x00000000); //seq_init_addr1
Xil_Out32(0xf61502f8, 0x00000000); //seq_init_addr2
Xil_Out32(0xf61502fc, 0x03000400); //seq_init_addr3
Xil_Out32(0xf6150300, 0x03000400); //seq_init_addr4
Xil_Out32(0xf6150304, 0x06001000); //seq_init_addr5
Xil_Out32(0xf6150308, 0x06001000); //seq_init_addr6
Xil_Out32(0xf615030c, 0x05000500); //seq_init_addr7
Xil_Out32(0xf6150310, 0x05000500); //seq_init_addr8
Xil_Out32(0xf6150314, 0x04000000); //seq_init_addr9
Xil_Out32(0xf6150318, 0x04000000); //seq_init_addr10
Xil_Out32(0xf615031c, 0x02000028); //seq_init_addr11
Xil_Out32(0xf6150320, 0x02000028); //seq_init_addr12
Xil_Out32(0xf6150324, 0x01000001); //seq_init_addr13
Xil_Out32(0xf6150328, 0x01000001); //seq_init_addr14
Xil_Out32(0xf615032c, 0x00000d50); //seq_init_addr15
Xil_Out32(0xf6150330, 0x00000d50); //seq_init_addr16
Xil_Out32(0xf6150334, 0x00000524); //seq_init_addr17
Xil_Out32(0xf6150338, 0x00000524); //seq_init_addr18
Xil_Out32(0xf615033c, 0x00000000); //seq_init_addr19
Xil_Out32(0xf6150340, 0x00000000); //seq_init_addr20
Xil_Out32(0xf6150344, 0x00000000); //seq_init_addr21
Xil_Out32(0xf6150348, 0x00000000); //seq_init_addr22
Xil_Out32(0xf615034c, 0x00000000); //seq_init_addr23
Xil_Out32(0xf6150350, 0x00000000); //seq_init_addr24
Xil_Out32(0xf6150354, 0x00000000); //seq_init_addr25
Xil_Out32(0xf6150358, 0x00000000); //seq_init_addr26
Xil_Out32(0xf615035c, 0x00000000); //seq_init_addr27
Xil_Out32(0xf6150360, 0x00000000); //seq_init_addr28
Xil_Out32(0xf6150364, 0x00000000); //seq_init_addr29
Xil_Out32(0xf6150368, 0x00000000); //seq_init_addr30
Xil_Out32(0xf6150378, 0x00000001); //seq_init_cntrl0
Xil_Out32(0xf615037c, 0x00000001); //seq_init_cntrl1
Xil_Out32(0xf6150380, 0x00000000); //seq_init_cntrl2
Xil_Out32(0xf6150384, 0x00000010); //seq_init_cntrl3
Xil_Out32(0xf6150388, 0x00000022); //seq_init_cntrl4
Xil_Out32(0xf615038c, 0x00000010); //seq_init_cntrl5
Xil_Out32(0xf6150390, 0x00000022); //seq_init_cntrl6
Xil_Out32(0xf6150394, 0x00000010); //seq_init_cntrl7
Xil_Out32(0xf6150398, 0x00000022); //seq_init_cntrl8
Xil_Out32(0xf615039c, 0x00000010); //seq_init_cntrl9
Xil_Out32(0xf61503a0, 0x00000022); //seq_init_cntrl10
Xil_Out32(0xf61503a4, 0x00000010); //seq_init_cntrl11
Xil_Out32(0xf61503a8, 0x00000022); //seq_init_cntrl12
Xil_Out32(0xf61503ac, 0x00000010); //seq_init_cntrl13
Xil_Out32(0xf61503b0, 0x00000022); //seq_init_cntrl14
Xil_Out32(0xf61503b4, 0x00000010); //seq_init_cntrl15
Xil_Out32(0xf61503b8, 0x00000022); //seq_init_cntrl16
Xil_Out32(0xf61503bc, 0x00000010); //seq_init_cntrl17
Xil_Out32(0xf61503c0, 0x00000022); //seq_init_cntrl18
Xil_Out32(0xf61503c4, 0x00000010); //seq_init_cntrl19
Xil_Out32(0xf61503c8, 0x00000022); //seq_init_cntrl20
Xil_Out32(0xf61503cc, 0x00000010); //seq_init_cntrl21
Xil_Out32(0xf61503d0, 0x00000022); //seq_init_cntrl22
Xil_Out32(0xf61503d4, 0x00000010); //seq_init_cntrl23
Xil_Out32(0xf61503d8, 0x00000022); //seq_init_cntrl24
Xil_Out32(0xf61503dc, 0x00000010); //seq_init_cntrl25
Xil_Out32(0xf61503e0, 0x00000022); //seq_init_cntrl26
Xil_Out32(0xf61503e4, 0x00000010); //seq_init_cntrl27
Xil_Out32(0xf61503e8, 0x00000022); //seq_init_cntrl28
Xil_Out32(0xf61503ec, 0x00000010); //seq_init_cntrl29
Xil_Out32(0xf61503f0, 0x00000022); //seq_init_cntrl30
Xil_Out32(0xf6150734, 0x03202020); //txnq_rd_priority
Xil_Out32(0xf6150738, 0x01202020); //txnq_wr_priority
Xil_Out32(0xf6150800, 0x00001016); //reg_safe_config5
Xil_Out32(0xf6150820, 0x00000001); //reg_scrub9
Xil_Out32(0xf6150824, 0x00000006); //reg_config1
Xil_Out32(0xf615082c, 0x00000001); //cal_mode
Xil_Out32(0xf6150830, 0x00000016); //phy_rden0
Xil_Out32(0xf6150834, 0x00000016); //phy_rden1
Xil_Out32(0xf6150838, 0x00000016); //phy_rden2
Xil_Out32(0xf615083c, 0x00000016); //phy_rden3
Xil_Out32(0xf6150840, 0x00000016); //phy_rden4
Xil_Out32(0xf6150844, 0x00000016); //phy_rden5
Xil_Out32(0xf6150848, 0x00000016); //phy_rden6
Xil_Out32(0xf615084c, 0x00000016); //phy_rden7
Xil_Out32(0xf6150850, 0x00000016); //phy_rden8
Xil_Out32(0xf6150854, 0x00000016); //phy_rden9
Xil_Out32(0xf6150858, 0x00000016); //phy_rden10
Xil_Out32(0xf615085c, 0x00000016); //phy_rden11
Xil_Out32(0xf6150860, 0x00000016); //phy_rden12
Xil_Out32(0xf6150864, 0x00000016); //phy_rden13
Xil_Out32(0xf6150868, 0x00000016); //phy_rden14
Xil_Out32(0xf615086c, 0x00000016); //phy_rden15
Xil_Out32(0xf6150870, 0x00000016); //phy_rden16
Xil_Out32(0xf6150874, 0x00000016); //phy_rden17
Xil_Out32(0xf6150878, 0x00000016); //phy_rden18
Xil_Out32(0xf615087c, 0x00000016); //phy_rden19
Xil_Out32(0xf6150880, 0x00000016); //phy_rden20
Xil_Out32(0xf6150884, 0x00000016); //phy_rden21
Xil_Out32(0xf6150888, 0x00000016); //phy_rden22
Xil_Out32(0xf615088c, 0x00000016); //phy_rden23
Xil_Out32(0xf6150890, 0x00000016); //phy_rden24
Xil_Out32(0xf6150894, 0x00000016); //phy_rden25
Xil_Out32(0xf6150898, 0x00000016); //phy_rden26
Xil_Out32(0xf615089c, 0x0000000f); //fifo_rden
Xil_Out32(0xf61508ac, 0x00000000); //seq_mode
Xil_Out32(0xf61508b0, 0x0000007f); //seq_cmd_default
Xil_Out32(0xf61508d0, 0x00000000); //seq_ck_cal
Xil_Out32(0xf6150944, 0x00000410); //xpi_oe_all_nib
Xil_Out32(0xf6150948, 0x00000410); //xpi_wrdata_all_nib
Xil_Out32(0xf615094c, 0x00000101); //xpi_oe
Xil_Out32(0xf6150950, 0x000000c0); //xpi_pmi_config
Xil_Out32(0xf6150958, 0x00000001); //xpi_write_dm_dbi
Xil_Out32(0xf6150960, 0x000000a5); //default_pattern
Xil_Out32(0xf6150964, 0x00000000); //t_txbit
Xil_Out32(0xf6150c0c, 0x0000005f); //xpi_dqs
//**************Register programming from ddrmc_init end----------------
#endif
#endif
dbg0_pmc(16391);
Xil_Out32(0xf6150968, 0x00000640);

dbg0_pmc(16402);
prog_reg(0xF6150E3C, 0x0, 0x1, 0x1);
dbg0_pmc(16396);
poll_for(0xf6150e40, 0x00000001, 0x00000000, 0x00000001);
dbg0_pmc(16397);
dbg0_pmc(16399);
dbg0_pmc(16403);
Xil_Out32(0xf6150cbc, 0x00000002);
Xil_Out32(0xf6110000, 0x000000fe);
Xil_Out32(0xf6110004, 0x00000d00);
#if defined(XPMCFW_HW80) || defined(XPMCFW_HW90)
// change axi width to 512
// VNOC NMU 4
Xil_Out32(0xf6f1000c, 0xf9e8d7c6);
Xil_Out32(0xf6f102c8, 0x60c2);
Xil_Out32(0xf6f10454, 0x6);
Xil_Out32(0xf6f1000c, 0x0);
 // VNOC NSU 4
Xil_Out32(0xf6f1200c, 0xf9e8d7c6);
Xil_Out32(0xf6f12104, 0x6);
Xil_Out32(0xf6f1200c, 0x0);
 // VNOC NMU 6
 Xil_Out32(0xf6f6000c, 0xf9e8d7c6);
Xil_Out32(0xf6f602c8, 0x60e6);
Xil_Out32(0xf6f60454, 0x6);
Xil_Out32(0xf6f6000c, 0x0);
 // VNOC NSU 6
Xil_Out32(0xf6f6200c, 0xf9e8d7c6);
Xil_Out32(0xf6f62104, 0x6);
Xil_Out32(0xf6f6200c, 0x0);
#endif
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// msoc_driver.c ver. 1.26, Sep  7 2018 , 22:10:01 ********

//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// ---- INPUT PARAMETERS ----
// sim mode    = 1
#ifdef XPMCFW_DDR64
// ecc         = 0
// bus width   = 64
#endif
#ifdef XPMCFW_DDR32_ECC
// ecc         = 1
// bus width   = 32
#endif
// dram width  = 8
// memory type = DDR4
// speed bin   = 3200
// frequency   = 1600.000 MHz
// device capacity  = 8192 Mbit
// rank   addr cnt  = 1
// bank group  addr cnt  = 0
// bank   addr cnt  = 0
// row    addr cnt  = 0
// column addr cnt  = 10
// temp ctrl ref mode = 0
// temp ctrl ref range = 0
// CL   = 22 cycles
// CWL  = 16 cycles
// tRCD = 22 cycles
// tRP  = 22 cycles
// AL   = 0 cycles
// BL   = 8 (burst length)
// tRC  = 45.750 nsec
// tRAS = 32.000 nsec
// tFAW = 21.000 nsec
// clock_stop_en = 0
// rd_dbi = 0
// wr_dbi = 0
// ecc_scrub = 0
// ecc_poison = 0
// parity = 0
// ca_parity_latency = 0
// geardown = 0
// cal_mode_en = 0
// udimm = 0
// rdimm = 0
// addr_mirror = 0
// dimm_addr_mirror = 0
// crc = 0
// brc_mapping = 0
// wr_preamble = 0
// rd_preamble = 0
// wr_postamble = 0
// rd_postamble = 0


}
#endif



#ifdef XPMCFW_ME

#define SS_PS_ME_BASE_ADDR 0x200000000
#define PSCR_LOCK 0xF9E8D7C6
#define ROW_ADDR_SHIFT 0x12
#define COL_ADDR_SHIFT 0x17

#define TILE_ADDR(col,row) (SS_PS_ME_BASE_ADDR | (col << COL_ADDR_SHIFT) | (row << ROW_ADDR_SHIFT))

unsigned int ME_In(unsigned long Addr)
{
// asm("dsb sy \n");
// asm("isb \n");
  return *(volatile unsigned int *) Addr;
}

void ME_Out(unsigned long OutAddress, unsigned int Value)
{
  *(volatile unsigned int *) OutAddress = Value;
// asm("dsb sy \n");
// asm("isb \n");
}

void me_npi_unlock() {
  Xil_Out32(ME_NPI_REG_PCSR_LOCK, PSCR_LOCK);
}


void remove_me_array_reset() {
  unsigned int regval;
  regval = Xil_In32(ME_NPI_ME_RESET_CTRL);
  regval = regval & (~ME_NPI_ME_RESET_CTRL_ME_ARRAY_RESET_MASK);
  regval = regval | (0x0 << ME_NPI_ME_RESET_CTRL_ME_ARRAY_RESET_SHIFT);
  Xil_Out32(ME_NPI_ME_RESET_CTRL, regval);
}

void me_setup_pll() {
  unsigned int regval;
  //Remove PCSR.INITSTATE_MASK
  regval = Xil_In32(ME_NPI_REG_PCSR_MASK);
  regval = regval & (~ME_NPI_REG_PCSR_MASK_GATEREG_MASK);
  regval = regval | (0x1 << ME_NPI_REG_PCSR_MASK_GATEREG_SHIFT);
  Xil_Out32(ME_NPI_REG_PCSR_MASK, regval);

  //Remove PCSR.INITSTATE_CTRL
  regval = Xil_In32(ME_NPI_REG_PCSR_CONTROL);
  regval = regval & (~ME_NPI_REG_PCSR_CONTROL_GATEREG_MASK);
  regval = regval | (0x0 << ME_NPI_REG_PCSR_CONTROL_GATEREG_SHIFT);
  Xil_Out32(ME_NPI_REG_PCSR_CONTROL, regval);

  //MPLL CTRL FBDIV = 48 and Internal Divider = 2
  regval = Xil_In32(ME_NPI_MPLL_CTRL);
  regval = regval & (~ME_NPI_MPLL_CTRL_CLKOUTDIV_MASK);
  regval = regval | (0x1 << ME_NPI_MPLL_CTRL_CLKOUTDIV_SHIFT);
  regval = regval & (~ME_NPI_MPLL_CTRL_FBDIV_MASK);
  regval = regval | (0x30 << ME_NPI_MPLL_CTRL_FBDIV_SHIFT);
  Xil_Out32(ME_NPI_MPLL_CTRL, regval);

  //set divisor to 2
  regval = Xil_In32(ME_NPI_ME_CORE_REF_CTRL);
  regval = regval & (~ME_NPI_ME_CORE_REF_CTRL_DIVISOR0_MASK);
  regval = regval | (0x2 << ME_NPI_ME_CORE_REF_CTRL_DIVISOR0_SHIFT);
  Xil_Out32(ME_NPI_ME_CORE_REF_CTRL, regval);

  //MPLL CFG config
  regval = Xil_In32(ME_NPI_MPLL_CFG);
  regval = regval & (~ME_NPI_MPLL_CFG_RES_MASK);
  regval = regval | (0x2 << ME_NPI_MPLL_CFG_RES_SHIFT);
  regval = regval & (~ME_NPI_MPLL_CFG_CP_MASK);
  regval = regval | (0x3 << ME_NPI_MPLL_CFG_CP_SHIFT);
  regval = regval & (~ME_NPI_MPLL_CFG_LFHF_MASK);
  regval = regval | (0x3 << ME_NPI_MPLL_CFG_LFHF_SHIFT);
  regval = regval & (~ME_NPI_MPLL_CFG_LOCK_CNT_MASK);
  regval = regval | (0x2EE << ME_NPI_MPLL_CFG_LOCK_CNT_SHIFT);
  regval = regval & (~ME_NPI_MPLL_CFG_LOCK_DLY_MASK);
  regval = regval | (0x3F << ME_NPI_MPLL_CFG_LOCK_DLY_SHIFT);
  Xil_Out32(ME_NPI_MPLL_CFG, regval);

  //MPLL CTRL BYPASS enable
  regval = Xil_In32(ME_NPI_MPLL_CTRL);
  regval = regval & (~ME_NPI_MPLL_CTRL_BYPASS_MASK);
  regval = regval | (0x1 << ME_NPI_MPLL_CTRL_BYPASS_SHIFT);
  Xil_Out32(ME_NPI_MPLL_CTRL, regval);

  //Remove PCSR.INITSTATE_MASK
  regval = Xil_In32(ME_NPI_REG_PCSR_MASK);
  regval = regval & (~ME_NPI_REG_PCSR_MASK_INITSTATE_MASK);
  regval = regval | (0x1 << ME_NPI_REG_PCSR_MASK_INITSTATE_SHIFT);
  Xil_Out32(ME_NPI_REG_PCSR_MASK, regval);

  //Remove PCSR.INITSTATE_CTRL
  regval = Xil_In32(ME_NPI_REG_PCSR_CONTROL);
  regval = regval & (~ME_NPI_REG_PCSR_CONTROL_INITSTATE_MASK);
  regval = regval | (0x0 << ME_NPI_REG_PCSR_CONTROL_INITSTATE_SHIFT);
  regval = regval & (~ME_NPI_REG_PCSR_CONTROL_GATEREG_MASK);
  regval = regval | (0x0 << ME_NPI_REG_PCSR_CONTROL_GATEREG_SHIFT);
  Xil_Out32(ME_NPI_REG_PCSR_CONTROL, regval);

  //Release MPLL_CTRL.RESET
  regval = Xil_In32(ME_NPI_MPLL_CTRL);
  regval = regval & (~ME_NPI_MPLL_CTRL_RESET_MASK);
  regval = regval | (0x0 << ME_NPI_MPLL_CTRL_RESET_SHIFT);
  Xil_Out32(ME_NPI_MPLL_CTRL, regval);

  //Poll for PLL lock and stable
  regval = 0x0;
  while((regval & 0x5) != 0x5) {
  XPmcFw_Printf(DEBUG_INFO, "waiting \n\r");
    regval = Xil_In32(ME_NPI_ME_PLL_STATUS);
  }

  //MPLL CTRL BYPASS remove
  regval = Xil_In32(ME_NPI_MPLL_CTRL);
  regval = regval & (~ME_NPI_MPLL_CTRL_BYPASS_MASK);
  regval = regval | (0x0 << ME_NPI_MPLL_CTRL_BYPASS_SHIFT);
  Xil_Out32(ME_NPI_MPLL_CTRL, regval);

}

void me_smid_config() {
  Xil_Out32(ME_NPI_ME_SMID_REG, 0x1F);
}

void me_init_seq() {
	XPmcFw_Printf(DEBUG_INFO, "ME init seq start..");
  //apply_sys_rst1();
  me_npi_unlock();
  me_setup_pll();
  me_smid_config();
  //remove_sys_rst1();
  remove_me_array_reset();
	XPmcFw_Printf(DEBUG_INFO, "Done \n\r");
}

#endif

#ifdef XPMCFW_SSIT

void reg_rmw(unsigned int addr,unsigned int mask,unsigned int shift,unsigned int data) {
    unsigned int reg_val;
    reg_val = Xil_In32(addr);
    reg_val = reg_val & (0xFFFFFFFF ^ mask);
    reg_val = reg_val | (data << shift);
    Xil_Out32(addr,reg_val);
}

void n_pll_prog(int n_pll_fbdiv, int n_pll_clkoutdiv)
{

    unsigned int pll_status_regval;

    // Set CLOCKOUTDIV
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_CLKOUTDIV_MASK, CRP_NOCPLL_CTRL_CLKOUTDIV_SHIFT, n_pll_clkoutdiv);

    // Set FBDIV
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_FBDIV_MASK, CRP_NOCPLL_CTRL_FBDIV_SHIFT, n_pll_fbdiv);

    // Setting PLL BYPASS
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_BYPASS_MASK, CRP_NOCPLL_CTRL_BYPASS_SHIFT, 1);

    // Setting PLL RESET
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_RESET_MASK, CRP_NOCPLL_CTRL_RESET_SHIFT, 1);

    // Clearing PLL RESET
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_RESET_MASK, CRP_NOCPLL_CTRL_RESET_SHIFT, 0);

    // Checking PLL lock
    pll_status_regval = 0x0;
    while((pll_status_regval & CRP_PLL_STATUS_NOCPLL_LOCK_MASK) != CRP_PLL_STATUS_NOCPLL_LOCK_MASK) {
        pll_status_regval = Xil_In32(CRP_PLL_STATUS);
    }

    // Clearing PLL BYPASS
    reg_rmw(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_BYPASS_MASK, CRP_NOCPLL_CTRL_BYPASS_SHIFT, 0);

    // CRP_NPLL_TO_XPD_CTRL
    reg_rmw(CRP_NPLL_TO_XPD_CTRL, CRP_NPLL_TO_XPD_CTRL_CLKACT_MASK, CRP_NPLL_TO_XPD_CTRL_CLKACT_SHIFT, 1);

}

void nps_turn(unsigned int npi_addr, unsigned int Req_ID, unsigned int Resp_ID, unsigned int input_port, unsigned int output_port)
{
    unsigned int temp_addr;
    unsigned int high_id;
    unsigned int mid_id;
    unsigned int low_id;
    unsigned int wr_data;
    unsigned int wr_halfword;

    temp_addr = npi_addr + 0xC; //REG_PCSR_LOCK
    Xil_Out32(temp_addr,0xf9e8d7c6);
    temp_addr = npi_addr; //REG_PCSR_MASK
    Xil_Out32(temp_addr,0x23e);
    temp_addr = npi_addr + 0x4; //REG_PCSR_CONTROL
    Xil_Out32(temp_addr,0x200);

    //Configuring for Request path
    high_id = (Req_ID & 0xFC0)>>6;
    mid_id = (Req_ID & 0x030)>>4;
    low_id = (Req_ID & 0x00F);

    if (high_id != 0) {
        //staring HIGHID0 addr
        temp_addr = npi_addr + 0x100 + high_id * 8; // REG_HIGH_ID16_P23
    } else if (mid_id != 0) {
        //staring MIDID0 addr
        temp_addr = npi_addr + 0x300 + mid_id * 8; // REG_HIGH_ID16_P23
    } else {
        //staring LOWID0 addr
        temp_addr = npi_addr + 0x320 + low_id * 8; // REG_HIGH_ID16_P23
    }


    if (output_port == 0 ) {
        wr_halfword = 0x0;
    }
    if (output_port == 1) {
        wr_halfword = 0x5555;
    }
    if (output_port == 2) {
        wr_halfword = 0xAAAA;
    }
    if (output_port == 3) {
        wr_halfword = 0xFFFF;
    }

    if (input_port == 0) {
        //better to do read-modify write
        wr_data = 0xFFFFAAAA; //default
        wr_data = (wr_data & 0xFFFF0000) + wr_halfword;
    }

    if (input_port == 1) {
        //better to do read-modify write
        wr_data = 0xFFFFAAAA; //default
        wr_data = (wr_data & 0x0000FFFF) + (wr_halfword << 16);
    }

    if (input_port == 2) {
        //better to do read-modify write
        wr_data = 0x55550000; //default
        wr_data = (wr_data & 0xFFFF0000) + wr_halfword;
        temp_addr = temp_addr + 4;
    }

    if (input_port == 3) {
        //better to do read-modify write
        wr_data = 0x55550000; //default
        wr_data = (wr_data & 0x0000FFFF) + (wr_halfword << 16);
        temp_addr = temp_addr + 4;
    }

    Xil_Out32(temp_addr,wr_data); // REG_HIGH_ID16_PXX

    //Configuring for Respnse path
    high_id = (Resp_ID & 0xFC0)>>6;
    mid_id = (Resp_ID & 0x030)>>4;
    low_id = (Resp_ID & 0x00F);

    if (high_id != 0) {
        //staring HIGHID0 addr
        temp_addr = npi_addr + 0x100 + high_id * 8; // REG_HIGH_ID16_P23
    } else if (mid_id != 0) {
        //staring MIDID0 addr
        temp_addr = npi_addr + 0x300 + mid_id * 8; // REG_HIGH_ID16_P23
    } else {
        //staring LOWID0 addr
        temp_addr = npi_addr + 0x320 + low_id * 8; // REG_HIGH_ID16_P23
    }

    if (input_port == 0 ) {
        wr_halfword = 0x0;
    }
    if (input_port == 1) {
        wr_halfword = 0x5555;
    }
    if (input_port == 2) {
        wr_halfword = 0xAAAA;
    }
    if (input_port == 3) {
        wr_halfword = 0xFFFF;
    }

    if (output_port == 0) {
        //better to do read-modify write
        wr_data = 0xFFFFAAAA; //default
        wr_data = (wr_data & 0xFFFF0000) + wr_halfword;
    }

    if (output_port == 1) {
        //better to do read-modify write
        wr_data = 0xFFFFAAAA; //default
        wr_data = (wr_data & 0x0000FFFF) + (wr_halfword << 16);
    }

    if (output_port == 2) {
        //better to do read-modify write
        wr_data = 0x55550000; //default
        wr_data = (wr_data & 0xFFFF0000) + wr_halfword;
        temp_addr = temp_addr + 4;
    }

    if (output_port == 3) {
        //better to do read-modify write
        wr_data = 0x55550000; //default
        wr_data = (wr_data & 0x0000FFFF) + (wr_halfword << 16);
        temp_addr = temp_addr + 4;
    }

    Xil_Out32(temp_addr,wr_data); // REG_LOW_ID0_PXX

    temp_addr = npi_addr + 0xC; //REG_PCSR_LOCK
    Xil_Out32(temp_addr,0x0);
}

void mst_slr_nidb_cfg() {

    unsigned int nir_adr, temp_adr;

    nir_adr = 0xf6230000;
    //Xil_In32
    //Unlock change
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0xf9e8d7c6); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
    //This is to de-asert pdisable field
    temp_adr = nir_adr + 0x0;
    Xil_Out32(temp_adr,0x4); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_MASK
    temp_adr = nir_adr + 0x4;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_CONTROL
    //Lock again
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK


    nir_adr = 0xf6240000;


    //Xil_In32
    //Unlock change
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0xf9e8d7c6); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
    //This is to de-asert pdisable field
    temp_adr = nir_adr + 0x0;
    Xil_Out32(temp_adr,0x4); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_MASK
    temp_adr = nir_adr + 0x4;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_CONTROL
    //Lock again
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
}
// NOC Prog
void mst_slr_mst_pmc_slv_pmc_noc_cfg(unsigned int nir_adr) {
	unsigned int pmc_nmu_adr;
	unsigned int temp_adr;

	//NIR setup
	//Unlock change
	temp_adr = nir_adr + 0xc;
	Xil_Out32(temp_adr,0xf9e8d7c6); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
	//This is to de-asert gate_reg field
	temp_adr = nir_adr + 0x0;
	Xil_Out32(temp_adr,0x2); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_MASK
	temp_adr = nir_adr + 0x4;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_CONTROL
	//Need to understand below
	temp_adr = nir_adr + 0x100;
	Xil_Out32(temp_adr,0x6541888); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_TIMEBASE_SEL
	//Lock again
	temp_adr = nir_adr + 0xc;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK

	//PMC NMU setup
	pmc_nmu_adr = nir_adr + 0x016000;
	//unlock
	temp_adr = pmc_nmu_adr + 0xc;
	Xil_Out32(temp_adr,0xf9e8d7c6); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_LOCK
	//src id
	temp_adr = pmc_nmu_adr + 0x450;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_SRC
	//remove mask for  system rst
	temp_adr = pmc_nmu_adr + 0x0;
	Xil_Out32(temp_adr,0x10382cc); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_MASK
	//reset assertion/deassertion (?)
	temp_adr = pmc_nmu_adr + 0x4;
	Xil_Out32(temp_adr,0x38200); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_CONTROL
	//priority bit setting for rd/wr
	temp_adr = pmc_nmu_adr + 0x464;
	Xil_Out32(temp_adr,0x1); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PRIORITY
	temp_adr = pmc_nmu_adr + 0x430;
	Xil_Out32(temp_adr,0x1); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_ADR_MAP_PMC_ALIAS_0
	temp_adr = pmc_nmu_adr + 0x434;
	Xil_Out32(temp_adr,0x401); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_ADR_MAP_PMC_ALIAS_1
	temp_adr = pmc_nmu_adr + 0x438;
	Xil_Out32(temp_adr,0x801); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_ADR_MAP_PMC_ALIAS_2
	temp_adr = pmc_nmu_adr + 0x43c;
	Xil_Out32(temp_adr,0xC01); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_ADR_MAP_PMC_ALIAS_3
	//mask removed for gatereg
	temp_adr = pmc_nmu_adr + 0x0;
	Xil_Out32(temp_adr,0x2); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_MASK
	//address remap entry, same as default
	temp_adr = pmc_nmu_adr + 0x2c4;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_ADDR_REMAP
	//gate_reg drive to '0
	temp_adr = pmc_nmu_adr + 0x4;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_CONTROL
	temp_adr = pmc_nmu_adr + 0xc;
	Xil_Out32(temp_adr,0x0); // reg_model_global.u_pmc_noc_nmu.noc_nmu_bank.REG_PCSR_LOCK

	mst_slr_nidb_cfg();
}

void slv0_slr_nidb_top_egress_cfg()
{
	unsigned int nir_adr, temp_adr;

	nir_adr = 0xf6110000;
    //Unlock change
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0xf9e8d7c6); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
    //This is to de-asert pdisable field
    temp_adr = nir_adr + 0x0;
    Xil_Out32(temp_adr,0x4); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_MASK
    temp_adr = nir_adr + 0x4;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_CONTROL
    //Lock again
    temp_adr = nir_adr + 0xc;
    Xil_Out32(temp_adr,0x0); // reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
}

void XPmcFw_SsitNocInit(void)
{
	unsigned int slr_type = 0;
	unsigned int error_cnt = 0;
	u64 pmc_alias_dest = 0x108000000;
	u64 pmc_alias_dest_pmc_ram = pmc_alias_dest + 0x2000000;
    int j;

    slr_type = Xil_In32(PMC_TAP_SLR_TYPE);

    switch (slr_type) {
    case SSIT_MONOLITIC:
    {
    }
    break;

    case SSIT_MASTER_SLR:
    {
	XPmcFw_Printf(DEBUG_INFO, "SSIT NOC Config \n\r");
	n_pll_prog(72,3);

	// CRP_NPI_REF_CTRL : clkact, npi_ref_clk = 300Mhz, src=NPLL(1.2Ghz)
	Xil_Out32(CRP_NPI_REF_CTRL, 0x02000403);

	// Release NoC Reset, NoC POR, NPI Reset
	reg_rmw(CRP_RST_NONPS, 0x00000077, 0, 0x0);
	Xil_Out32(CRP_RST_NONPS, 0x0);

	// Configuration of NoC Paths
	mst_slr_mst_pmc_slv_pmc_noc_cfg(0xF6000000);

	// Initiate transfer to Master PMC alias region
	// Writes
	for(j=0; j < 0x10 ; j++){
		XPmcFw_Out64(pmc_alias_dest_pmc_ram+j*4, 0xABCD0000+j);
	}
	// Reads
	for(j=0; j < 0x10 ; j++){
		if( XPmcFw_In64(pmc_alias_dest_pmc_ram+j*4) != 0xABCD0000+j ) { error_cnt++; }
	}

	Xil_Out32(CRP_PMCPLL_CFG,       0x7E4B0C82); // PMCPLL CFG
	Xil_Out32(CRP_PMCPLL_CTRL,      0x00025800); // PMCPLL output 1320 MHz
	Xil_Out32(CRP_QSPI_REF_CTRL,    0x01000400); // QSPI REF_CLK 330 MHz
    }
    break;

    case SSIT_SLAVE_SLR1_TOP_SLR:
    {
    }
    break;

    case SSIT_SLAVE_SLR1_NON_TOP_SLR:
    {
	Xil_Out32(CRP_RST_NONPS,0x0);

        // Configuration of NoC Paths
        slv0_slr_nidb_top_egress_cfg();

        /*
         * Indicate to Master that NoC config done on slave
         * and clear the source
         */
        Xil_Out32(PMC_GLOBAL_SSIT_ERR, SSIT_INTR_MASK);
        Xil_Out32(PMC_GLOBAL_SSIT_ERR, SSIT_INTR_CLEAR);

     }break;

    case SSIT_SLAVE_SLR2_TOP_SLR:
    {
    }
    break;

    case SSIT_SLAVE_SLR2_NON_TOP_SLR:
    {
    }
    break;

    case SSIT_SLAVE_SLR3_TOP_SLR:
    {
    }
    break;

    default:
    {
    }
    break;

    }

    XPmcFw_Printf(DEBUG_INFO, "Configuration done \n\r");
}
#endif

int npi_init( ) {

#ifndef XPMCFW_SSIT
	msoc_hsr_init();
#endif
        Xil_Out32(CRP_RST_PL,0x0);
#ifdef XPMCFW_ME
	me_init_seq();
#endif

#ifdef XPMCFW_SSIT
	XPmcFw_SsitNocInit();
#endif

	return 0;
}
