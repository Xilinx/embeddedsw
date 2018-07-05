/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 *
 * @file xsrio_dma_loopback_example.c
 *
 * This file demonstrates how to use xsrio driver on the Xilinx SRIO Gen2 Core.
 * The SRIO Gen2 comprised of phy, logical and transport and buffer layers.
 * Using this IP We can generate both messaging and read/write semantics.
 *
 * This example demonstartes how to generate SWRITE(Streaming Write)
 * transactions on the core.
 * 
 * Inorder to test this example external loopback is required at the boardlevel
 * Between the SRIO Tx and Rx pins.
 * 
 * H/W Requirments:
 * Inorder to test this example at the h/w level the the SRIO Initiator Request 
 * is connected to the AXI DMA MM2S Channel and SRIO Target Request is connected
 * to the AXI DMA S2MM Channel.
 * 
 * S/W Flow:
 * 1) The system consists of two different memories.Processor runs this example
 *    Code in one memory and the SRIO packet is formed in another memory. 
 * 2) The SRIO Packet is framed in the Memory
 * 3) Configure the AXI DMA MM2S source address and S2MM for Destiantion address
 *    and specify the byte count for both the channels and then start the dma.
 * 4) Compare the Data.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   adk  16/04/14	Initial release
 * 1.1   ms   01/23/17  Modified xil_printf statement in main function to
 *                      ensure that "Successfully ran" and "Failed" strings
 *                      are available in all examples. This is a fix for
 *                      CR-965028.
 *       ms   04/05/17  Added tabspace for return statements in functions for
 *                      proper documentation while generating doxygen.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xparameters.h"	
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xsrio.h"
#include "xbram.h"
#include "xaxidma.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define SRIO_DEVICE_ID   XPAR_SRIO_0_DEVICE_ID
#define MEM_ADDR         XPAR_AXI_BRAM_CTRL_1_S_AXI_BASEADDR
#define DMA_DEV_ID       XPAR_AXIDMA_0_DEVICE_ID
#define DATA_SIZE 	 256

/******************** Variable Definitions **********************************/
XSrio Srio;		 /* Instance of the XSrio */
XAxiDma AxiDma;          /* Instance of the XAxiDma */

/******************** Function Prototypes ************************************/
int XSrioDmaLoopbackExample(XSrio *InstancePtr, u16 DeviceId);

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the SRIO DMA Loopback test. 
*
* @param	None
*
* @return
*		- XST_SUCCESS if tests pass
* 		- XST_FAILURE if fails.
*
* @note		None
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("Entering main\n\r");

	Status = XSrioDmaLoopbackExample(&Srio, SRIO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("SRIO DMA Loopback Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran SRIO DMA Loopback Test\n\r");
	xil_printf("--- Exiting main() ---\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XSrioDmaLoopbackExample This function does a minimal test on the XSrio device
* and driver as a design example. The purpose of this function is to illustrate
* how to use the XSrio Component.
* 
* This function does the following:
*	- Initialize the SRIO device
* 	- Initialize the DMA engine
*	- Clearing the Memory
* 	- Framing the SRIO Packet in the Memory
*	- Configuring the SRIO
* 	- Configuring the DMA
*	- Verifying the Data
*
* @param	InstancePtr is a pointer to the instance of the
*		XSrio driver.
* @param	DeviceId is Device ID of the SRIO Gen2 Device.
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
int XSrioDmaLoopbackExample(XSrio *InstancePtr, u16 DeviceId)
{
	XSrio_Config *SrioConfig;
	XAxiDma_Config *DmaConfig;
	int Status = XST_SUCCESS;
	int Count = 0;
	
	/* Initialize the SRIO Device Configuration Interface driver */
	SrioConfig = XSrio_LookupConfig(DeviceId);
	if (!SrioConfig) {
		xil_printf("No SRIO config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	/**< This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XSrio_CfgInitialize(InstancePtr, SrioConfig,
				SrioConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed for SRIO\n\r");
		return Status;
	}
	 
	 /* Check for PE Configuration */
	Status = XSrio_GetPEType(InstancePtr); 
	if (Status != XSRIO_IS_MEMORY) {
		xil_printf("SRIO is not configured as the Memory \n\r");
		return XST_FAILURE;
	}
		
	/* Clearing the Memory */
	for(Count=0; Count<(128*1024); Count += 4) {
		*(u32 *)(MEM_ADDR + Count) = 0;
	}
	
	/**< Check whether Streaming Write Operation is Supported by the 
	 * Core or not Since it is a loopback Example Checking at the both 
	 * Target and source Operations.
	 */
	Status = XSrio_IsOperationSupported(InstancePtr, XSRIO_OP_MODE_SWRITE,
						XSRIO_DIR_TX);
	if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
	
	Status = XSrio_IsOperationSupported(InstancePtr, XSRIO_OP_MODE_SWRITE,
						XSRIO_DIR_RX);
	if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
	
	/**< Frame the SRIO Write-Stream Packet in the Memory 
	 * The Packet-format used here is HELLO Packet format
	 * More details look into pg 3.1 version:73 page(HELLO PACKET FORMAT).
	 */
	*(u32 *)(MEM_ADDR + 0x00) = 0x50000600;
		/* Lower word of the HELLO Packet */
	*(u32 *)(MEM_ADDR + 0x04) = 0x08602F74;
		/* Upper word of the HELLO packet */
	Count = 8;

	while(Count<(DATA_SIZE * 2)) {
		*(u32 *)(MEM_ADDR + Count) = Count;
		 Count += 4;
	}
	
	/* SRIO Configuration */
	/* Set the Local Configuration Space Base Address */
	XSrio_SetLCSBA(InstancePtr, 0xFFF);
        /* Set the Water Mark Level to transfer priority 0,1,2 packet */
	XSrio_SetWaterMark(InstancePtr, 0x5, 0x4, 0x3);
        /* Set the Port Response timeout value */
	XSrio_SetPortRespTimeOutValue(InstancePtr, 0x010203);
	
	/* DMA Configuration */
	DmaConfig = XAxiDma_LookupConfig(DMA_DEV_ID);
    	if (!DmaConfig) {
           xil_printf("No DMA config found for %d\r\n", DMA_DEV_ID);
           return XST_FAILURE;
    	}

	/* Initialize DMA engine */
    	Status = XAxiDma_CfgInitialize(&AxiDma, DmaConfig);
	if (Status != XST_SUCCESS) {
           xil_printf("Initialization failed %d\r\n", Status);
           return XST_FAILURE;
     	}
	
	/**< Configure the DMA Tx Side 
         * MEM_ADDR is the address where Tx packet is formed
	 */
	XAxiDma_WriteReg(DmaConfig->BaseAddr, XAXIDMA_CR_OFFSET,
				XAXIDMA_CR_RUNSTOP_MASK);
	XAxiDma_IntrEnable(&AxiDma,XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_WriteReg(DmaConfig->BaseAddr, XAXIDMA_SRCADDR_OFFSET, MEM_ADDR);

	/*
	 * Configure the DMA Rx Side
     * MEM_ADDR+0x5000 is the address where Rx packet is formed
	 */
	XAxiDma_WriteReg(DmaConfig->BaseAddr,
		XAXIDMA_RX_OFFSET+XAXIDMA_CR_OFFSET, XAXIDMA_CR_RUNSTOP_MASK);
	XAxiDma_IntrEnable(&AxiDma, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_WriteReg(DmaConfig->BaseAddr,
		XAXIDMA_RX_OFFSET + XAXIDMA_DESTADDR_OFFSET, MEM_ADDR+0x5000);
	
	
	for(Count=8; Count<DATA_SIZE; Count += 4) {
		*(u32 *)(MEM_ADDR + Count) = Count;
	}
	
	XAxiDma_WriteReg(DmaConfig->BaseAddr, XAXIDMA_BUFFLEN_OFFSET, 256);
	/* Wait till DMA MM2S Transfer Complete */
	while(!(XAxiDma_ReadReg(DmaConfig->BaseAddr, XAXIDMA_SR_OFFSET)
			& 0x1000));
	XAxiDma_WriteReg(DmaConfig->BaseAddr,
			XAXIDMA_RX_OFFSET+XAXIDMA_BUFFLEN_OFFSET, 256);
	 /* Wait till S2MM Transfer Complete */
	  while(!( XAxiDma_ReadReg(DmaConfig->BaseAddr,
			XAXIDMA_RX_OFFSET+XAXIDMA_SR_OFFSET) & 0x1000));
	  	
	/* Verifying the Data */
	for(Count=8; Count<DATA_SIZE; Count += 4) {
		if(*(u32 *)(MEM_ADDR + 0x5000 + Count) 
				!= *(u32 *)(MEM_ADDR + Count)) {
			xil_printf("\n ERROR in Transfer\n\r");
			return XST_FAILURE;
		}
	}	

	/* Clearing the Memory */
	for(Count=0x5000; Count<(0x6000); Count += 4) {
		*(u32 *)(MEM_ADDR + Count) = 0;
	}

	for(Count=0; Count<DATA_SIZE; Count += 4) {
		if(*(u32 *)(MEM_ADDR + 0x5000+ Count) != 0x00) {
			xil_printf("\n ERROR in Clearing Memory\n\r");
			return XST_FAILURE;
		}
	}	

	return Status;
}



	
