/******************************************************************************
* Copyright (C) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file si570drv.c
 *
 * This file contains low-level driver functions for controlling the
 * SiliconLabs Si570 clock generator as mounted on the ZCU106 demo board.
 * The user should refer to the hardware device specification for more details
 * of the device operation.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date         Changes
 * ----- --- ----------   -----------------------------------------------
 * 1.0   ssh 07/27/2021	  Initial release.
 * </pre>
 *
 ****************************************************************************/

#include "si570drv.h"
#include "stdio.h"
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (versal)
#include "xiicps.h"
#else
#include "xiic.h"
#endif
#include "sleep.h"

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (versal)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

static void double2hex(double dnum, u8 *hnum);
static unsigned Si570_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option);
static unsigned Si570_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option);

void double2hex(double dnum, u8 *hnum)
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

/*****************************************************************************/
/**
*
* This function send the IIC data to Si570
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param MsgPtr points to the data to be sent.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
* 		  transmitting the data.
*
* @return	The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned Si570_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (versal)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	/* Set operation to 7-bit mode */
	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);

	/* Set Repeated Start option */
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterSendPolled(Iic_Ptr, MsgPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	/* This delay prevents IIC access from hanging */
	usleep(500);

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	return XIic_Send(Iic_Ptr->BaseAddress, SlaveAddr, MsgPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function send the IIC data to Si570
*
* @param  IicPtr IIC instance pointer.
* @param  SlaveAddr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param BufPtr points to the memory to write the data.
* @param ByteCount is the number of bytes to be sent.
* @param Option indicates whether to hold or free the bus after
* 		  transmitting the data.
*
* @return	The number of bytes sent.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static unsigned Si570_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option)
{
#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
	defined (versal)
	XIicPs *Iic_Ptr = IicPtr;
	u32 Status;

	XIicPs_SetOptions(Iic_Ptr, XIICPS_7_BIT_ADDR_OPTION);
	XIicPs_ClearOptions(Iic_Ptr, XIICPS_10_BIT_ADDR_OPTION);
	if (Option == I2C_REPEATED_START) {
		XIicPs_SetOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	} else {
		XIicPs_ClearOptions(Iic_Ptr, XIICPS_REP_START_OPTION);
	}

	Status = XIicPs_MasterRecvPolled(Iic_Ptr, BufPtr, ByteCount, SlaveAddr);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	if (!(Iic_Ptr->IsRepeatedStart)) {
		while (XIicPs_BusIsBusy(Iic_Ptr));
	}

	if (Status == XST_SUCCESS) {
		return ByteCount;
	} else {
		return 0;
	}
#else
	XIic *Iic_Ptr = IicPtr;
	return XIic_Recv(Iic_Ptr->BaseAddress, SlaveAddr, BufPtr,
					ByteCount, Option);
#endif
}

/*****************************************************************************/
/**
*
* This function the frequency output of Si570
*
* @param  IicPtr IIC instance pointer.
* @param  Si_Addr contains the 7 bit IIC address of the device to send the
*		   specified data to.
* @param Freq is the requested frequency output in MHz (floating)
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void Si570_SetFreq(void *IicPtr, u8 Si_Addr, double Freq)
{
    u8 RFreq_Cal[5];
    u8 HSDIV_Cal;
    u8 N1_Cal;
    u8 RFreq_Set[5];
    u8 HSDIV_Set;
    u8 N1_Set;
    double XtalFreq;


	/* Read the NVM RFreq_Cal Data */
	Si570_ReadCal(IicPtr, Si_Addr, RFreq_Cal, &HSDIV_Cal, &N1_Cal);

	Si570_XtalFreqCalc(&XtalFreq, RFreq_Cal, &HSDIV_Cal, &N1_Cal);

    /* Calculate New Frequency Settings */
	Si570_RfreqCalc(Freq, RFreq_Cal, RFreq_Set, &HSDIV_Set,
											&N1_Set, &XtalFreq);
	/* Write New Frequency Settings */
	Si570_WriteRfreq(IicPtr, Si_Addr, RFreq_Set, HSDIV_Set, N1_Set);

}

void Si570_ReadCal(void *IicPtr, u8 Addr, u8 *RFreq_Cal,
                        u8 *HSDIV_Cal, u8 *N1_Cal)
{
	int Status = XST_SUCCESS;
    u8 retry = 0;
    u8 N1_HS_DIV;

    do {
        Status = XST_SUCCESS;

        /* RST & RECALL */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){135,0x01}, 2, I2C_STOP);
        /* N1_HS_DIV */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  7,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, &N1_HS_DIV,       1, I2C_STOP);
        /* N1[1:0],RF[37:32] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  8,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, RFreq_Cal+0,      1, I2C_STOP);
        /* RF[31:24] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  9,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, RFreq_Cal+1,      1, I2C_STOP);
        /* RF[23:16] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 10,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, RFreq_Cal+2,      1, I2C_STOP);
        /* RF[15:8] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 11,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, RFreq_Cal+3,      1, I2C_STOP);
        /* RF[7:0] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 12,0xff}, 1, I2C_STOP);
        Status |= Si570_I2cRecv(IicPtr, Addr, RFreq_Cal+4,      1, I2C_STOP);

        *HSDIV_Cal = (N1_HS_DIV >> 5) & 0x7;
        *N1_Cal    = ((N1_HS_DIV & 0x1f) << 2) | ((*(RFreq_Cal+0) >> 6) & 0x3);
        *RFreq_Cal = (*(RFreq_Cal+0) & (u8)0x3f);

        retry++;
    } while ((Status != XST_SUCCESS) && (retry < 5));
}

void Si570_XtalFreqCalc(double *XtalFreq, u8 *RFreq_Cal,
                        u8 *HSDIV_Set, u8 *N1_Set)
{
    int HS_DIV;
    int N1;
    double RFREQ;

    HS_DIV = *HSDIV_Set + 4;

    if (*N1_Set == 0x0) {
        N1 = 1;
    } else {
        N1 = (*N1_Set % 2) ? *N1_Set+1 : *N1_Set;
    }

    RFREQ = ((RFreq_Cal[0] * two_to_32) + (RFreq_Cal[1] * two_to_24) +
                (RFreq_Cal[2] * two_to_16) + (RFreq_Cal[3] * two_to_8) +
                RFreq_Cal[4]) / two_to_28;

    *XtalFreq = (SI570_DEFAULT_CLKOUT_FREQ * HS_DIV * N1) / RFREQ;
}

void Si570_RfreqCalc(double Freq, u8 *RFreq_Cal, u8 *RFreq_Set,
                        u8 *HSDIV_Set, u8 *N1_Set, double *XtalFreq)
{
   int   HSDIV_Valid[6] = {4,5,6,7,9,11};
   int   N1_Valid[65] = {1,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,
                         38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,
                         72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,
                         104,106,108,110,112,114,116,118,120,122,124,126,128};

   int   i,j;
   double Rval_Std, Rval_Cal;
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
        if ((DCO >= 4850.0 && DCO <= 5670.0) &&
            ((HSDIV_Valid[i] * N1_Valid[j]) < ((*HSDIV_Set) * (*N1_Set)))) {
           *HSDIV_Set = HSDIV_Valid[i];
           *N1_Set    = N1_Valid[j];
        }
      }
   }
   if (*HSDIV_Set==255) {
	   xil_printf(ANSI_COLOR_RED "Si570 Error! Configuration not found for "
                    "frequency %13.10lf\n" ANSI_COLOR_RESET, Freq);
	   return;
   }
   DCO = Freq * (*HSDIV_Set) * (*N1_Set);
   /*
    * Calculate New RFreq Setting
    */
   Rval_Std = 5000.0 / (*XtalFreq) * two_to_28;  // 0x2BC011EB8;
   Rval_Cal = (RFreq_Cal[0] * two_to_32) + (RFreq_Cal[1] * two_to_24) +
                (RFreq_Cal[2] * two_to_16) + (RFreq_Cal[3] * two_to_8) +
                RFreq_Cal[4];
   RFreq_Val= ((DCO / (*XtalFreq)) * (Rval_Cal / Rval_Std)) * two_to_28;
   /*RFreq_Val= ((DCO / (*XtalFreq))) * two_to_28;*/

   double2hex(RFreq_Val, RFreq_Set);

}


void Si570_WriteRfreq(void *IicPtr, u8 Addr, u8 *RFreq_Set,
                        u8 HSDIV_Set, u8 N1_Set)
{
	int Status = XST_SUCCESS;
	u8 reg7,reg8;
    u8 retry = 0;

	/*
	 * Change from human readable settings to Si570 programming values
	 */
    /* Valid 4=000, 5=001, 6=010, 7=011, 9=101, 11=111 */
	HSDIV_Set = HSDIV_Set - 4;
    /* Subtract one from N1 value */
	N1_Set    = N1_Set-1;

    /* HSDIV[2:0] & N1[6:2] */
	reg7 = ( HSDIV_Set & 0x07)*32 + ((N1_Set & 0x7C)/4);
    /* N1[1:0] & RFREQ[37:32] */
	reg8 = ( N1_Set    & 0x03)*64 + (*RFreq_Set & 0x3F);

    do {
        Status = XST_SUCCESS;

        /* 137:Freeze DCO */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){137,0x18},
								2, I2C_STOP);
        /* 7:HSDIV[2:0], N1[6:2] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  7,reg7},
								2, I2C_STOP);
        /* 8:N1[1:0],RFREQ[37:32] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  8,reg8},
								2, I2C_STOP);
        /* 9:RFREQ[31:24] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){  9,*(RFreq_Set+1)},
								2, I2C_STOP);
        /* 10:RFREQ[23:16] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 10,*(RFreq_Set+2)},
								2, I2C_STOP);
        /* 11:RFREQ[15:8] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 11,*(RFreq_Set+3)},
								2, I2C_STOP);
        /* 12:RFREQ[ 7:0] */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){ 12,*(RFreq_Set+4)},
								2, I2C_STOP);
        /* 137:Unfreeze DCO */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){137,0x08},
								2, I2C_STOP);
        /* 135:Reset */
        Status |= Si570_I2cSend(IicPtr, Addr, (u8[]){135,0x40},
								2, I2C_STOP);

        retry++;
    } while ((Status != XST_SUCCESS) && (retry < 5));
}
