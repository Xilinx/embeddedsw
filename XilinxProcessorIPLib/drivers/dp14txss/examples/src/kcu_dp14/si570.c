/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file si570.c
*
* This file contains Si570 related functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  Kei    08/12/17 Initial release.
* </pre>
*
******************************************************************************/

#include "xbasic_types.h"
#include "dppt.h"

#if ENABLE_AUDIO
volatile u8 TransmitComplete;   /* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;    /* Flag to check completion of Reception */

extern XIic IicInstance;

typedef u8 AddressType;

u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
u8 ReadBuffer[PAGE_SIZE];

static void ReceiveHandler(XIic * InstancePtr)
{
        ReceiveComplete = 0;
}

static void SendHandler(XIic * InstancePtr)
{
        TransmitComplete = 0;
}

static void StatusHandler(XIic * InstancePtr, int Event)
{

}

int iic_write(u16 ByteCount)
{
        int Status;

        /*
         * Set the defaults.
         */
        TransmitComplete = 1;
        IicInstance.Stats.TxErrors = 0;
        XIic_InterruptHandler(&IicInstance);

        /*
         * Start the IIC device.
         */
        //xil_printf("sat0\r\n");
        Status = XIic_Start(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);
        /*
         * Send the Data.
         */
        Status = XIic_MasterSend(&IicInstance, WriteBuffer, ByteCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Wait till the transmission is completed.
         */
        while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
                /*
                 * This condition is required to be checked in the case where we
                 * are writing two consecutive buffers of data to the EEPROM.
                 * The EEPROM takes about 2 milliseconds time to update the data
                 * internally after a STOP has been sent on the bus.
                 * A NACK will be generated in the case of a second write before
                 * the EEPROM updates the data internally resulting in a
                 * Transmission Error.
                 */
                XIic_InterruptHandler(&IicInstance);

                if (IicInstance.Stats.TxErrors != 0) {
                        XIic_InterruptHandler(&IicInstance);

                        /*
                         * Enable the IIC device.
                         */
                        Status = XIic_Start(&IicInstance);
                        if (Status != XST_SUCCESS) {
                                return XST_FAILURE;
                        }
                        //xil_printf("sat3\r\n");
                        XIic_InterruptHandler(&IicInstance);

                        if (!XIic_IsIicBusy(&IicInstance)) {
                                /*
                                 * Send the Data.
                                 */
                                XIic_InterruptHandler(&IicInstance);

                                Status = XIic_MasterSend(&IicInstance,
                                                         WriteBuffer,
                                                         ByteCount);
                                if (Status == XST_SUCCESS) {
                                        IicInstance.Stats.TxErrors = 0;
                                }
                                else {
                                }
                        }
                }
        }

        /*
         * Stop the IIC device.
         */
        //xil_printf("sat4\r\n");
        Status = XIic_Stop(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        //xil_printf("sat5\r\n");

        return XST_SUCCESS;
}

int iic_read(AddressType addr, u8 *BufferPtr, u16 ByteCount)
{
        int Status;
//      AddressType Address = EEPROM_TEST_START_ADDRESS;
        AddressType Address;
        Address = addr;

        /*
         * Set the Defaults.
         */
        ReceiveComplete = 1;
        XIic_InterruptHandler(&IicInstance);

        /*
         * Position the Pointer in EEPROM.
         */
        //xil_printf("st11\r\n");
        if (sizeof(Address) == 1) {
                WriteBuffer[0] = (u8) (Address);
        }
        else {
                WriteBuffer[0] = (u8) (Address >> 8);
                WriteBuffer[1] = (u8) (Address);
        }
        XIic_InterruptHandler(&IicInstance);

        Status = iic_write(sizeof(Address));
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Start the IIC device.
         */
        Status = XIic_Start(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        //xil_printf("start read2\r\n");
        XIic_InterruptHandler(&IicInstance);
        /*
         * Receive the Data.
         */
        Status = XIic_MasterRecv(&IicInstance, BufferPtr, ByteCount);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        /*
         * Wait till all the data is received.
         */
        while ((ReceiveComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
                XIic_InterruptHandler(&IicInstance);

        }
        /*
         * Stop the IIC device.
         */
        Status = XIic_Stop(&IicInstance);
        if (Status != XST_SUCCESS) {
                return XST_FAILURE;
        }
        XIic_InterruptHandler(&IicInstance);

        return XST_SUCCESS;
}

int write_si570(u8 *UpdateBuffer)
{
	u32 Index;
	int Status;
	AddressType Address = EEPROM_TEST_START_ADDRESS;
	AddressType addr;
	
	XIic_SetSendHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance,
			    (XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance,
			      (XIic_StatusHandler) StatusHandler);
	/* Initialize the data to write and the read buffer. */
	if (sizeof(Address) == 1) {
		WriteBuffer[0] = (u8) (Address);
	} else {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
		ReadBuffer[Index] = 0;
	}
	
	/* Set the Slave address to the PCA9543A.*/
	Status = XIic_SetAddress(&IicInstance,
				 XII_ADDR_TO_SEND_TYPE,
				 IIC_SWITCH_ADDRESS);
	//xil_printf("set addr end\r\n");
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	
	/* Write to the IIC Switch. */
	WriteBuffer[0] = 0x01; /* Select Bus0 - U1 */
	Status = iic_write(1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("eeprom write end\r\n");
	
	/* Set the Slave address to the SI570 */
	Status = XIic_SetAddress(&IicInstance,
				 XII_ADDR_TO_SEND_TYPE,
				 IIC_SI570_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("set addr end\r\n");
	
	/* Write to the SI570 */
	/* Set frequency back to default power-up value
	 * In this case 156.250000 MHz
	 * Freeze DCO bit in Reg 137
	 * */
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x10;
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("1 end\r\n");
	
	/* Recall the 156.2500000 value from NVM
	 * by setting RECALL (bit 0) = 1 in Reg 135
	 */
	WriteBuffer[0] = 135;
	WriteBuffer[1] = 0x01;
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("2 end\r\n");

	/* Un-Freeze DCO bit in Reg 137 */
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x00;
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("3 end\r\n");
	
	/* Assert New Frequency bit in Reg 135 */
	WriteBuffer[0] = 135;
	WriteBuffer[1] = 0x40;
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("4 end\r\n");
	
	/* Wait 10 ms */
	//int zz,kk;
	//for(kk= 0; kk<100000; kk++)
	//        zz = kk+1;
	usleep(10000);
	//udelay(1000);
	//xil_printf("5 end\r\n");
	
	/* Update to user requested frequency */
	/* Freeze DCO bit in Reg 137 */
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x10;
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("6 end\r\n");
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("7 end\r\n");
	
	/* Set New Frequency to 400 MHz when starting from 156.25 MHz */
	WriteBuffer[0] = 7;
	WriteBuffer[1] = UpdateBuffer[0];
	WriteBuffer[2] = UpdateBuffer[1];
	WriteBuffer[3] = UpdateBuffer[2];
	WriteBuffer[4] = UpdateBuffer[3];
	WriteBuffer[5] = UpdateBuffer[4];
	WriteBuffer[6] = UpdateBuffer[5];
	
	Status = iic_write(sizeof(Address) + 6);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("8 end\r\n");
	
	/* Un-Freeze DCO bit in Reg 137 */
	WriteBuffer[0] = 137;
	WriteBuffer[1] = 0x00;
	
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("9 end\r\n");
	
	/* Assert New Frequency bit in Reg 135 */
	WriteBuffer[0] = 135;
	WriteBuffer[1] = 0x40;
	
	Status = iic_write(sizeof(Address) + 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("a end\r\n");
	
	/* Read from the SI570 */
	//xil_printf("Reading data from SI570\r\n");
	addr = 7;
	Status = iic_read(addr, ReadBuffer, 6);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//xil_printf("b end\r\n");
	
	/* Display Read Buffer */
	for (Index = 0; Index < 6; Index++) {
	//    xil_printf("ReadBuffer[%02d] = %02X\r\n",
	//    		 Index, ReadBuffer[Index]);
	}
	
	
	return XST_SUCCESS;
}

#endif
