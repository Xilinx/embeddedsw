/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file LMK04906.c
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI   07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#include "LMK04906.h"

volatile int TransferInProgress;

void LMK04906_init(XSpi *SPI_LMK04906){
    int Status    = 0;
    u32 SPI_Option = 0;
    XSpi_Config *SPI_LMK04906_Conf;

	// SPI Setting Load
	SPI_LMK04906_Conf = (XSpi_LookupConfig(LMK04906_DEVICE_ID));

	if (SPI_LMK04906_Conf == NULL) {
		xil_printf("Error : SPI Device ConfSetting Not Found !\n\r");
        exit(-1);
	}

    // SPI Device init
    Status = XSpi_CfgInitialize(SPI_LMK04906, SPI_LMK04906_Conf,
								SPI_LMK04906_Conf->BaseAddress);
    if(Status != XST_SUCCESS){
	xil_printf("Error : SPI Device Not Found ( Status Num : %d )!\n\r",
							Status);
	exit(-1);
    }

    //Reset
    Soft_Reset();
    // Get Option
    SPI_Option = Get_Option();
    // Set Option
    SPI_Option = ((SPI_Option|0x00000066)&0xFFFFFEFE);
//    xil_printf("SPI Set option %x\n\r",SPI_Option);
    Set_Option(SPI_Option);
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0xFFFF);
}

int LMK04906_RegWrite(XSpi *SPI_LMK04906 , u32 RegData ,u32 RegAddr){

	const u32 WriteData = (RegData << 5)|RegAddr ;
	volatile u32 wait_cnt = 0;
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0x0000);

//	printf("TX Data : [%08x]\n\r",WriteData);
    Tx_Data_Write(WriteData);
    while(wait_cnt++ != 200);
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x70, 0xFFFF);

	return XST_SUCCESS;
}

int IF_LoopBack_Test(XSpi *SPI_LMK04906) {

    u32 SPI_Option;
    int Count;

    SPI_Option = Get_Option();
    SPI_Option  |= (u32)0x00000001;
    xil_printf("SPI Set option %x\n\r",SPI_Option);
    Set_Option(SPI_Option);

    xil_printf("Internal Loop Back Mode Set \n\r");
	for (Count = 0; Count < 15; Count++) {
		// TX DATA
		Tx_Data_Write(Count);
	}
	for (Count = 0; Count < 15; Count++) {
		// RX DATA
	    if(Rx_Data_Read() != Count){
	        SPI_Option  &= (u32)0xFFFFFFFE;
	        Set_Option( SPI_Option);
		xil_printf("Internal LoopBack FAILD \n\r");
		return 1;
	    }
	}
    SPI_Option  &= (u32)0xFFFFFFFE;
    Set_Option( SPI_Option);
	xil_printf("Internal LoopBack Success \n\r");
    return XST_SUCCESS;
}

void Soft_Reset(){
	//Reset
	XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x40, 0xa);
	usleep(100);
	XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x40, 0x0);
}
void Set_Option(u32 Option){
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x60, Option);
}
u32 Get_Option(){
    return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x60);
}

u32 Get_Status(){
	return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x64);
}

u32 Rx_Data_Read(){
	u8 timer = 0;
    while((Get_Status()&0x1) && timer < 250){
		timer++;
    }
	return XSpi_ReadReg(LMK04906_DEVICE_BASEADDR,0x6C);

}
void Tx_Data_Write(u32 WriteData){
	while(Get_Status()&0x8);
    XSpi_WriteReg(LMK04906_DEVICE_BASEADDR,0x68, WriteData);

}
