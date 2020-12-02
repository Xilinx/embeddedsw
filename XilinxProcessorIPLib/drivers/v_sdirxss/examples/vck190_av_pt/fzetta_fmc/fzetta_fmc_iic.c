/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/
/**
 *
 * @file fzetta_fmc_iic.c
 *
 * FMC configuration file
 *
 * This file configures the FMC card for KCU116 SDI Tx to SDI Rx loopback
 * design
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  jsr   03/07/2018 Initial version
 * </pre>
 *
 ******************************************************************************/
#include <stdio.h>
#include "xiic.h"
#include "fzetta_fmc_iic.h"


/*****************************************************************************/
/**
 *
 * This function  Initializes IIC IP and its corresponding drivers and configptr instances.
 *
 * @param	Dev_ID  Device ID.
 *
 * @return	XST_SUCCESS if initialization is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

int fzetta_fmc_iic_init(u8 Dev_ID) {
	int Status;
	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
   fzetta_fmc_Iic_ConfigPtr = XIic_LookupConfig(Dev_ID);
	if (fzetta_fmc_Iic_ConfigPtr == NULL) {
		return XST_FAILURE;
	}

    /*
     * Initialize the IIC device driver such that it's ready to use,
     * if it didn't initialize properly, abort and return the status.
     */
    Status = XIic_Initialize(&fzetta_fmc_Iic, Dev_ID);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    /*
     * Reset the Instance.
     */
    XIic_Reset(&fzetta_fmc_Iic);

    return XST_SUCCESS;
}


/*****************************************************************************/
/**
 *
 * This function  Fidus Zetta FMC Xbar Switch (DS10CP15A)
 * IIC Register Write Sequence:
 * 		START --> SMB ADDR + W --> ACK --> Xbar Reg ADDR \
 * 				         	   --> ACK --> Xbar Reg DATA --> ACK --> STOP
 *
 * @param	RegAddr  Register Address.
 * @param       RegData  Register Data
 *
 * @return	XST_SUCCESS if register write is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/


int fzetta_fmc_iic_xbar_register_write(u8 RegAddr, u8 RegData)
{
   int Status;
   u8 byte_count = 2;
   static u8 WriteBuffer[2];

    WriteBuffer[0] = RegAddr; //buffer 0 is sent first
    WriteBuffer[1] = RegData;

    //Send write buffer to IIC device
    Status = XIic_Send(fzetta_fmc_Iic.BaseAddress, XBAR_IIC_WRITE_ADDR, WriteBuffer, byte_count, XIIC_STOP);
    if (Status != byte_count)
    {
        return XST_FAILURE;
    }

    return XST_SUCCESS;

}

/*****************************************************************************/
/**
 *
 * This function  Fidus Zetta FMC Xbar Switch DS10CP15A
 * Register Read IIC Sequence:
 * 		START --> SMB ADDR + W --> ACK --> Xbar Reg ADDR --> ACK \
 * 	--> START --> SMB ADDR + R --> ACK --> Xbar Reg DATA --> NACK -->STOP
 *	NOT WORKING YET
 *
 * @param	RegAddr  Register Address.
 *
 * @return	XST_SUCCESS if register read is successful else XST_FAILURE
 *
 * @note	None.
 *
 ******************************************************************************/

u8 fzetta_fmc_iic_xbar_register_read(u8 RegAddr){
		return 0;
		int Status = XST_SUCCESS;
		u32 CntlReg;
		u8 WriteBuffer[1];
		u8 ReadBuffer[1];
	    WriteBuffer[0] = RegAddr; // Reg Addr
	   // WriteBuffer[1] = 0x0A; // Reg Data


	   Status = XIic_DynInit(fzetta_fmc_Iic.BaseAddress);
	   if (Status != XST_SUCCESS) {
	             return XST_FAILURE;
	   }

	  /*
	    * Make sure all the Fifo's are cleared and Bus is Not busy.
	    */
	   while (((Status = XIic_ReadReg(fzetta_fmc_Iic.BaseAddress,
	                           XIIC_SR_REG_OFFSET)) &
	                           (XIIC_SR_RX_FIFO_EMPTY_MASK |
	                           XIIC_SR_TX_FIFO_EMPTY_MASK |
	                         XIIC_SR_BUS_BUSY_MASK)) !=
	                         (XIIC_SR_RX_FIFO_EMPTY_MASK |
	                           XIIC_SR_TX_FIFO_EMPTY_MASK)) {
	   }


	   XIic_DynSend7BitAddress(fzetta_fmc_Iic.BaseAddress, XBAR_IIC_WRITE_ADDR, XIIC_WRITE_OPERATION);
	   XIic_WriteReg(fzetta_fmc_Iic.BaseAddress,  XIIC_DTR_REG_OFFSET, WriteBuffer[0]);
		CntlReg = XIic_ReadReg(fzetta_fmc_Iic.BaseAddress,  XIIC_CR_REG_OFFSET);
		CntlReg |= XIIC_CR_NO_ACK_MASK;
		XIic_WriteReg(fzetta_fmc_Iic.BaseAddress,  XIIC_CR_REG_OFFSET, CntlReg);
	   XIic_DynSend7BitAddress(fzetta_fmc_Iic.BaseAddress,XBAR_IIC_WRITE_ADDR, XIIC_READ_OPERATION);
	   XIic_DynSendStop(fzetta_fmc_Iic.BaseAddress, 1);
	   ReadBuffer[0] = XIic_ReadReg(fzetta_fmc_Iic.BaseAddress, XIIC_DRR_REG_OFFSET);



	return ReadBuffer[0];
}
