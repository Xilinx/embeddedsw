/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*/

#include "xparameters.h"
#include "xiicps.h"
#include "xspi.h"
#include "sleep.h"
#include "clk_set.h"

//XIicPs  IicInstance;
XIicPs  IicPtr;

#define two_to_37 (double) 137438953472.0
#define two_to_32 (double)   4294967296.0
#define two_to_28 (double)    268435456.0
#define two_to_24 (double)     16777216.0
#define two_to_16 (double)        65536.0
#define two_to_8  (double)          256.0



#define do_iic_until_pass(msg, foo) \
{ \
	u8 RetryCount = 0;						\
	do {								\
		Status = foo;						\
		RetryCount++;						\
	} while ((Status != XST_SUCCESS) && (RetryCount < 15));		\
}

static void si570_read_cal(XIicPs *Iic, u8 Addr, u8 *RFreq_Cal);
static void si570_rfreq_calc(double Freq, u8 *RFreq_Cal, u8 *RFreq_Set,
											u8 *HSDIV_Set, u8 *N1_Set);
static void si570_write_rfreq(XIicPs *Iic, u8 Addr, u8 *RFreq_Set,
										u8 HSDIV_Set,  u8 N1_Set);
static void double2hex(double dnum, u8 *hnum);
extern int LMK04906_RegWrite(XSpi *SPI_LMK04906 , u32 RegData ,u32 RegAddr);


u32 clk_set(u8 i2c_mux_addr, u8 i2c_dev_addr, double set_freq){

	XIicPs_Config *IicCfgPtr;
	u32 Status;
	u8 WriteBuffer[16];

	/* Initialize I2C driver. */
	IicCfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
	XIicPs_CfgInitialize(&IicPtr, IicCfgPtr, IicCfgPtr->BaseAddress);
	/* Set serial clock rate. */
	Status = XIicPs_SetSClk(&IicPtr, 400000);
	if (Status != XST_SUCCESS)
		xil_printf("XIicPs_SetSClk failed.\r\n");

	xil_printf("Select I2C channel.\r\n");
	WriteBuffer[0] = 1 << 2;
	Status = XIicPs_MasterSendPolled(&IicPtr, WriteBuffer, 1, i2c_mux_addr);
	if (Status != XST_SUCCESS)
		xil_printf("I2C channel select failed.\r\n");

	u8 RFreq_Cal[5];
    u8 RFreq_Set[5];
    u8 HSDIV_Set;
    u8 N1_Set;

	si570_read_cal(&IicPtr, i2c_dev_addr, RFreq_Cal);

	/* Calculate New Frequency Settings */
	si570_rfreq_calc(set_freq, RFreq_Cal, RFreq_Set, &HSDIV_Set, &N1_Set);

	/* Write New Frequency Settings */
	si570_write_rfreq(&IicPtr, i2c_dev_addr, RFreq_Set, HSDIV_Set, N1_Set);

	return XST_SUCCESS;

}

static void si570_read_cal(XIicPs *Iic, u8 Addr, u8 *RFreq_Cal)
{
	int Status;
	u16 i=0;

	/*
	 * Wait until bus is idle and then read calibration values
	 */
	while (XIicPs_BusIsBusy(Iic) && i++ < 10000)
		;

	do_iic_until_pass(">>> Fail (RST & RECALL) write",
		XIicPs_MasterSendPolled(Iic, (u8[]){135,0x01}, 2, Addr)
	);  // RST & RECALL
	do_iic_until_pass(">>> Fail 8 set\r\n",
		XIicPs_MasterSendPolled(Iic, (u8[]){  8,0xff}, 1, Addr)
	);  //
	do_iic_until_pass(">>> Fail 8 receive\r\n",
		XIicPs_MasterRecvPolled(Iic, RFreq_Cal+0,      1, Addr)
	);  // N1[1:0],RF[37:32]
	do_iic_until_pass(">>> Fail 9 set\r\n",
		XIicPs_MasterSendPolled(Iic, (u8[]){  9,0xff}, 1, Addr)
	);  //
	do_iic_until_pass(">>> Fail 9 receive\r\n",
		XIicPs_MasterRecvPolled(Iic, RFreq_Cal+1,      1, Addr)
	);  // RF[31:24]
	do_iic_until_pass(">>> Fail 10 set\r\n",
		XIicPs_MasterSendPolled(Iic, (u8[]){ 10,0xff}, 1, Addr)
	);  //
	do_iic_until_pass(">>> Fail 10 receive\r\n",
		XIicPs_MasterRecvPolled(Iic, RFreq_Cal+2,      1, Addr)
	);  // RF[23:16]
	do_iic_until_pass(">>> Fail 11 set\r\n",
		XIicPs_MasterSendPolled(Iic, (u8[]){ 11,0xff}, 1, Addr)
	);  //
	do_iic_until_pass(">>> Fail 11 receive\r\n",
		XIicPs_MasterRecvPolled(Iic, RFreq_Cal+3,      1, Addr)
	);  // RF[15:8]
	do_iic_until_pass(">>> Fail 12 set\r\n",
		XIicPs_MasterSendPolled(Iic, (u8[]){ 12,0xff}, 1, Addr)
	);  //
	do_iic_until_pass(">>> Fail 12 receive\r\n",
		XIicPs_MasterRecvPolled(Iic, RFreq_Cal+4,      1, Addr)
	);  // RF[7:0]


	*RFreq_Cal = (*(RFreq_Cal+0) & (u8)0x3f);

	if (Status ==0) return;
}


static void si570_rfreq_calc(double Freq, u8 *RFreq_Cal, u8 *RFreq_Set,
											u8 *HSDIV_Set, u8 *N1_Set){
	int	HSDIV_Valid[6] = {4,5,6,7,9,11};
	int	N1_Valid[65] = {1,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,
						38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,
						72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,
						104,106,108,110,112,114,116,118,120,122,124,126,128};

   int   i,j;
   double DCO;
   double RFreq_Val;

   /*
    * Find the lowest HS_DIV and N1 settings
    */
   *HSDIV_Set=255;
   *N1_Set=255;
   for (i=0;i<6;i++) {
      for (j=0;j<65;j++) {
        DCO = Freq * HSDIV_Valid[i] * N1_Valid[j];
        if ( (DCO >= 4850.0 && DCO <= 5670.0)
		&& ((HSDIV_Valid[i] * N1_Valid[j]) < ((*HSDIV_Set) * (*N1_Set)))) {
            *HSDIV_Set = HSDIV_Valid[i];
            *N1_Set    = N1_Valid[j];
        }
      }
   }
   if (*HSDIV_Set==255) {
       return;
   }
   DCO = Freq * (*HSDIV_Set) * (*N1_Set);

   /*
    * Calculate New RFreq Setting
    */

   RFreq_Val= ((DCO / 114.285) * two_to_28);

   double2hex(RFreq_Val, RFreq_Set);

}

static void si570_write_rfreq(XIicPs *Iic, u8 Addr, u8 *RFreq_Set,
										u8 HSDIV_Set,  u8 N1_Set){
	int Status;
	u8 reg7,reg8;
	u16 i=0;

	/*
	 * Change from human readable settings to Si570 programming values
	 */

	// Valid 4=000, 5=001, 6=010, 7=011, 9=101, 11=111
	HSDIV_Set = HSDIV_Set - 4;
	N1_Set    = N1_Set-1;      // Subtract one from N1 value

	// HSDIV[2:0] & N1[6:2];
	reg7 = ( HSDIV_Set & 0x07)*32 + ((N1_Set & 0x7C)/4);
	// N1[1:0] & RFREQ[37:32];
	reg8 = ( N1_Set    & 0x03)*64 + (*RFreq_Set & 0x3F);

	/*
	 * Wait until bus is idle and then write all values
	 */
	while (XIicPs_BusIsBusy(Iic) && i++ < 10000);

	do_iic_until_pass(">>> Fail freeze DCO\r\n",
			XIicPs_MasterSendPolled(Iic, (u8[]){137,0x18}, 2, Addr)
	);            // 137:Freeze DCO
	do_iic_until_pass(">>> Fail 7 HSDIV N1\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){  7,reg7}, 2, Addr)
	);            //   7:HSDIV[2:0], N1[6:2]
	do_iic_until_pass(">>> Fail 8 N1 RFREQ4.5\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){  8,reg8}, 2, Addr)
	);            //   8:N1[1:0],RFREQ[37:32]
	do_iic_until_pass(">>> Fail RFREQ3\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){  9,*(RFreq_Set+1)}, 2, Addr)
	);  //   9:RFREQ[31:24]
	do_iic_until_pass(">>> Fail RFREQ2\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){ 10,*(RFreq_Set+2)}, 2, Addr)
	);  //  10:RFREQ[23:16]
	do_iic_until_pass(">>> Fail RFREQ1\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){ 11,*(RFreq_Set+3)}, 2, Addr)
	);  //  11:RFREQ[15:8]
	do_iic_until_pass(">>> Fail RFREQ0\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){ 12,*(RFreq_Set+4)}, 2, Addr)
	);  //  12:RFREQ[ 7:0]
	do_iic_until_pass(">>> Fail unfreeze DCO\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){137,0x08}, 2, Addr)
	);            // 137:Unfreeze DCO
	do_iic_until_pass(">>> Fail reset\n\r",
			XIicPs_MasterSendPolled(Iic, (u8[]){135,0x40}, 2, Addr)
	);            // 135:Reset



	if (Status == 0) return;
}

static void double2hex(double dnum, u8 *hnum)
{
   int    i;
   u8     bnum[38];
   double pwr_2;

   /*
    *  Find binary values for [37:0] by successive substraction
    */
   pwr_2 = two_to_37;
   for (i=37;i>=0;i--){
      bnum[i]=0;
      if (dnum >= pwr_2 ) {
        bnum[i]=1;
        dnum = dnum - pwr_2;
      }
      pwr_2=pwr_2/2;
   }
   /*
    * Caculate 5 HEX values to represent [37:0]
    */
   hnum[0]=                             bnum[37]*32 + bnum[36]*16 +
           bnum[35]*8   + bnum[34]*4  + bnum[33]*2  + bnum[32];
   hnum[1]=bnum[31]*128 + bnum[30]*64 + bnum[29]*32 + bnum[28]*16 +
           bnum[27]*8   + bnum[26]*4  + bnum[25]*2  + bnum[24];
   hnum[2]=bnum[23]*128 + bnum[22]*64 + bnum[21]*32 + bnum[20]*16 +
           bnum[19]*8   + bnum[18]*4  + bnum[17]*2  + bnum[16];
   hnum[3]=bnum[15]*128 + bnum[14]*64 + bnum[13]*32 + bnum[12]*16 +
           bnum[11]*8   + bnum[10]*4  + bnum[ 9]*2  + bnum[ 8];
   hnum[4]=bnum[ 7]*128 + bnum[ 6]*64 + bnum[ 5]*32 + bnum[ 4]*16 +
           bnum[ 3]*8   + bnum[ 2]*4  + bnum[ 1]*2  + bnum[ 0];
}



int PLL_init_Seting(XSpi *SPI_LMK04906, u32 div_value) {

    const u32 init_Setting_num [27][2] =
    {     //{ Data      , Addr }
		/* 00 */
		{ 0x0000000, 31 },
            { 0x0001000,  0 }, // Reset
			// CLKOut0_PD   = 0   ,Divider = 18   (2430/18=135)
		{ div_value,  0 },
		{ div_value,  1 }, // CLKOut1_PD   = 0
		{ 0x4000000,  2 }, //    ***

		/* 05 */
		{ 0x4000001,  3 }, //    ***
            { 0x4000000,  4 }, //    ***
		{ 0x4000000,  5 }, // CLKOut5_PD   = 1 (Disable)
			// CLKOut0_TYPE = 0x01 (LVDS) , CLKOut0_ADLY = 0x00 (Analog Dealy 0)
		{ 0x0888000,  6 },
		{ 0x0888000,  7 }, //    ***

	    /* 10 */
			// CLKOut5_TYPE = 0x01 (LVDS) , CLKOut5_ADLY = 0x00 (Analog Dealy 0)
		{ 0x0888000,  8 },
		{ 0x2AAAAAA,  9 }, // === Defaulte ===
		{ 0x08A0200, 10 }, // EN_OSCout0 = 1 (Enable)
		{ 0x02C0881, 11 }, // xxxx MODE = 5'b00000 Dual PLL, Internal VCO
			// LD_MUX = 3(PLL1 & 2 DLD) , LD_TYPE = 3(Output (Push-Pull)) ,
			// SYNC_PLL2_DLD = 0(Nomal),SYNC_PLL1_DLD = 0(Nomal),
			// EN_TRACK = 1 (Enable) HOLDOVER_MODE = 2(Enable)
		{ 0x0D8600B, 12 },

		/* 15 */
			// HOLDOVER_MUX = 7(uWire Readback) ,
			// HOLDOVER_TYPE = Output(Push-Pull) ,
			// CLKin_SELECT_MODE = 0 (CLKin0 Manual)
		{ 0x1D80003, 13 },
			// CLKin1_BUF_TYPE = 1 (CMOS) , CLKin0_BUF_TYPE = 1 (CMOS)
		{ 0x0918000, 14 },
			// HOLDOVER_DLD_CNT = 512 , FORCE_HOLDOVER = 0(Disable)
		{ 0x0000000, 15 },
		{ 0x00AA820, 16 }, // XTAL_LVL = 0 (1.65 Vpp)
		{ 0x0000006, 24 }, // PLL1_WIND_SIZE = 3

		/* 20 */
		{ 0x0080030, 25 },
		{ 0x07D4000, 26 },
		{ 0x0800002, 27 }, // PLL1_R = 1
		{ 0x0008002, 28 }, // PLL2_R = 1 , PLL1_N = 1
		{ 0x000002D, 29 },

		/* 25 */
		{ 0x010002D, 30 }, // PLL2_N = 45
		{ 0x0000001, 31 }

    };
    int i;
    for(i = 0;i < 26 /*NIT_CMD_NUM*/;i++){
        LMK04906_RegWrite(
			SPI_LMK04906,init_Setting_num[i][0],init_Setting_num[i][1]);
    }
    // wait long enough for SPI write operation to be done
    //  as well as the LMK to finish their operations
    usleep(1000000);

    return XST_SUCCESS;
}
