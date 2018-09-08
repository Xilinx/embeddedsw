/******************************************************************************
*
* Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*

 * eeprom.h
 *
 *  Created on: May 5, 2015
 *      Author: yashova
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "xil_types.h"
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xil_exception.h"
#include "keymgmt.h"

#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif

/**************************** Type Definitions *******************************/

/*
 * The AddressType for ML300/ML310/ML410/ML510 boards should be u16
 *  as the address pointer in the on board EEPROM is 2 bytes.
 * The AddressType for ML403/ML501/ML505/ML507/ML605/SP601/SP605/KC705/ZC702
 * /ZC706 boards should be u8 as the address pointer in the on board EEPROM is
 * 1 bytes.
 */
typedef u8 AddressType;

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID

#ifdef XPAR_INTC_0_DEVICE_ID
 #define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
 #define IIC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
 #define INTC			XIntc
 #define INTC_HANDLER	XIntc_InterruptHandler
#else
 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
 #define IIC_INTR_ID		XPAR_FABRIC_IIC_0_VEC_ID
 #define INTC			 	XScuGic
 #define INTC_HANDLER		XScuGic_InterruptHandler
#endif


/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The 7 bit IIC Slave address of the IIC EEPROM on the ML300/ML310/ML403/ML410/
 * ML501/ML505/ML507/ML510 boards is 0x50. The 7 bit IIC Slave address of the
 * IIC EEPROM on the ML605/SP601/SP605/KC705/ZC702/ZC706 boards is 0x54.
 * Please refer the User Guide's of the respective boards for further
 * information about the IIC slave address of IIC EEPROM's.
 */
#define EEPROM_ADDRESS 		0x54	/* 0xA0 as an 8 bit number. */


/*
 * The IIC_MUX_ADDRESS defines the address of the IIC MUX device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 * The IIC Slaves on the KC705/ZC702/ZC706 boards are connected to an
 * IIC MUX.
 * IIC_EEPROM_CHANNEL is the Channel number of EEPROM for IIC Mux. On KC705 it
 * is 0x08 and ZC702 is 0x04.Please refer the User Guide's of the respective
 * boards for further information about the Channel number to use EEPROM.
 */
#define IIC_MUX_ADDRESS 		0x74
#define IIC_EEPROM_CHANNEL		0x08

/*
 * This define should be uncommented if there is IIC MUX on the board to which
 * this EEPROM is connected. The boards that have IIC MUX are KC705/ZC702/ZC706.
 */
#define IIC_MUX_ENABLE

/*
 * The page size determines how much data should be written at a time.
 * The ML310/ML300 board supports a page size of 32 and 16.
 * The write function should be called with this as a maximum byte count.
 */
#define PAGE_SIZE   16
#define DATA_SIZE   754
/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_TEST_START_ADDRESS   0

extern u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
extern u8 MasterWriteBuffer[1024];
extern u32 MasterWriteBufIndex;
extern u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */
extern u8 MasterReadBuffer[1024];
extern u32 MasterReadBufIndex;
extern u16 EepromIicAddr;		/* Variable for storing Eeprom IIC address */

int IicSetHandler();
int IicEepromReadWrite(const u8 *DataToWrite, u32 data_size);
int EepromWriteData(u16 ByteCount);
int EepromReadData(u8 *BufferPtr, u16 ByteCount);
int EepromReadDataOffset(u8 *BufferPtr, u16 ByteCount, u32 offset);
//static int SetupInterruptSystem(XIic *IicInstPtr);


#endif /* EEPROM_H_ */
