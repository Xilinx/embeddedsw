/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI  07/13/17 Initial release.
*
* </pre>
*
******************************************************************************/

#include <stdlib.h>
#include "xspi.h"
#include "sleep.h"
#include "xparameters.h"

#define  LMK04906_DEVICE_ID  XPAR_SPI_0_DEVICE_ID
#define  LMK04906_DEVICE_BASEADDR  XPAR_SPI_0_BASEADDR

typedef struct {
	u32 SPI_BaseAddr;
} LMK04906_SPI_Info;

volatile int TransferInProgress;

void LMK04906_init(XSpi *SPI_LMK04906);
int  IF_LoopBack_Test();
int  LMK04906_RegWrite(XSpi *SPI_LMK04906 , u32 RegData ,u32 RegAddr);

void Soft_Reset();
static void Set_Option(u32 Option);
static u32  Get_Option();
static u32  Get_Status();
static u32  Rx_Data_Read();
static void Tx_Data_Write(u32 WriteData);


void LMK04906_init(XSpi *SPI_LMK04906){
    int Status    = 0;
    u32 SPI_Option = 0;
    XSpi_Config *SPI_LMK04906_Conf;

	// SPI Setting Load
	SPI_LMK04906_Conf = (XSpi_LookupConfig(LMK04906_DEVICE_ID));

	if (SPI_LMK04906_Conf == NULL) {
		xil_printf("Error : SPI Device ConfSetting Not Found !\r\n");
        exit(-1);
	}

    // SPI Device init
    Status = XSpi_CfgInitialize(
		SPI_LMK04906, SPI_LMK04906_Conf,SPI_LMK04906_Conf->BaseAddress);
	if(Status != XST_SUCCESS){
		xil_printf("Error : SPI Device Not Found ( Status Num : %d )!\r\n",
																	Status);
		exit(-1);
    }

    //Reset
    XSpi_Reset(SPI_LMK04906);
    // Get Option
    SPI_Option = Get_Option();
    // Set Option
    SPI_Option = ((SPI_Option|0x00000066)&0xFFFFFEFE);
    Set_Option(SPI_Option);
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0xFFFF);
}

int LMK04906_RegWrite(XSpi *SPI_LMK04906 , u32 RegData ,u32 RegAddr){

	const u32 WriteData = (RegData << 5)|RegAddr ;
	volatile u32 wait_cnt = 0;
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0x0000);


    Tx_Data_Write(WriteData);

// set long enough timer value to wait for LMK to be ready
    while(wait_cnt++ != 2000)
       ;
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0xFFFF);

	return XST_SUCCESS;
}

int IF_LoopBack_Test(XSpi *SPI_LMK04906) {

    u32 SPI_Option;
    int Count;

    SPI_Option = Get_Option();
    SPI_Option  |= (u32)0x00000001;
    xil_printf("SPI Set option %x\r\n",SPI_Option);
    Set_Option(SPI_Option);

    xil_printf("Internal Loop Back Mode Set \r\n");
	for (Count = 0; Count < 15; Count++) {
		// TX DATA
		Tx_Data_Write(Count);
	}
	for (Count = 0; Count < 15; Count++) {
		// RX DATA
	    if(Rx_Data_Read() != Count){
	        SPI_Option  &= (u32)0xFFFFFFFE;
	        Set_Option( SPI_Option);
		xil_printf("Internal LoopBack FAILD \r\n");
		return 1;
	    }
	}
    SPI_Option  &= (u32)0xFFFFFFFE;
    Set_Option( SPI_Option);
	xil_printf("Internal LoopBack Success \r\n");
    return XST_SUCCESS;
}

void Soft_Reset(){
	//Reset
	XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x40, 0xa);
	// wait long enough for the LMK operations
	usleep(100000);
}
static void Set_Option(u32 Option){
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x60, Option);
}
static u32 Get_Option(){
    return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x60);
}

static u32 Get_Status(){
	return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x64);
}

static u32 Rx_Data_Read(){
	volatile u32 retval=Get_Status();
	volatile u32 wait_cnt = 0;
	while(retval&0x1 && wait_cnt < 250000){
		retval=Get_Status();
		wait_cnt++;
    }
	return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x6C);

}
static void Tx_Data_Write(u32 WriteData){
	volatile u32 wait_cnt = 0;
	while(Get_Status()&0x8 && wait_cnt < 250000)
		wait_cnt++;
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x68, WriteData);

}
