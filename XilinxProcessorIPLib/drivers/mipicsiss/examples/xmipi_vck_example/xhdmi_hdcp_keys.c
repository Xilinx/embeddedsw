/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_hdcp_keys.c
*
* This file contains the Xilinx HDCP key loading utility implementation
* as used in the HDMI example design. Please see xhdmi_hdcp_keys.h for
* more details.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* 1.0   MG   26-01-2016 Initial release
* 1.1   YH   04-08-2016 Bypass HDCP Key password for VIPER run in board farm
* 1.2   GM   12-07-2017 Changed printf usage to xil_printf
*                       Changed "\n\r" in xil_printf calls to "\r\n"
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xhdmi_hdcp_keys.h"

/************************** Constant Definitions *****************************/
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define XHDCP_UART_BASEADDR       XPAR_UARTLITE_0_BASEADDR
#else
#define XHDCP_UART_BASEADDR       XPAR_XUARTPS_0_BASEADDR
#endif

#define XHDCP_IIC_BASEADDR		  XPAR_IIC_0_BASEADDR
#define XHDCP_EEPROM_ADDRESS      0x50 /* 0xA0 as an 8 bit number */
#define XHDCP_EEPROM_PAGE_SIZE    16
#define SIGNATURE_OFFSET          0
#define HDCP22_LC128_OFFSET       16
#define HDCP22_CERTIFICATE_OFFSET 32
#define HDCP14_KEY1_OFFSET        1024
#define HDCP14_KEY2_OFFSET        1536

/************************** Function Prototypes ******************************/
static u16 Round16(u16 Size);
static void Decrypt(u8 *CipherBufferPtr,u8 *PlainBufferPtr,u8 *Key,u16 Length);
static u16 EepromGet(u16 Address, u8 *BufferPtr, u16 Length);
static u8 EepromReadByte(u16 Address, u8 *BufferPtr, u16 ByteCount);
static u8 EnterPassword (u8 *Password);

/*****************************************************************************/
/**
 *
 * This function loads the HDCP keys from the eeprom
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
int XHdcp_LoadKeys(u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size,
             u8 *Hdcp22RxPrivateKey, u32 Hdcp22RxPrivateKeySize,
	u8 *Hdcp14KeyA, u32 Hdcp14KeyASize, u8 *Hdcp14KeyB, u32 Hdcp14KeyBSize)
{
	u8 i;
	u8 HdcpSignature[16] = {"xilinx_hdcp_keys"};
	u8 Buffer[1024];
	u8 Password[32];
	u8 Key[32];
	u8 SignatureOk;
	u8 HdcpSignatureBuffer[16];
	SHA256_CTX Ctx;

	xil_printf("Before the HDCP functionality can be enabled, \r\n");
	xil_printf("the application will load the encrypted HDCP keys\r\n");
	xil_printf("from the HDMI FMC EEPROM.\r\n");
	xil_printf("The HDCP keys are protected with a unique password.\r\n");
	xil_printf("Please enter your password.\r\n");

	EnterPassword(Password);
	xil_printf("\r\n");


	// Generate password hash
	sha256_init(&Ctx);
	sha256_update(&Ctx, Password, sizeof(Password));
	sha256_final(&Ctx, Key);

	// Signature
	EepromGet(SIGNATURE_OFFSET, Buffer, sizeof(HdcpSignature));
	Decrypt(Buffer, HdcpSignatureBuffer, Key, sizeof(HdcpSignature));

	SignatureOk = TRUE;
	for (i=0; i<sizeof(HdcpSignature); i++)
	{
		if (HdcpSignature[i] != HdcpSignatureBuffer[i])
			SignatureOk = FALSE;
	}

	if (SignatureOk)
	{
		xil_printf("Password is valid.\r\n");
		xil_printf("Loading HDCP keys from EEPROM... ");

		// HDCP 2.2 LC128
		// Read from EEPROM
		EepromGet(HDCP22_LC128_OFFSET, Buffer, Round16(Hdcp22Lc128Size));

		// Decrypt
		Decrypt(Buffer, Hdcp22Lc128, Key, Hdcp22Lc128Size);

		// Certificate
		// Read from EEPROM
                EepromGet(HDCP22_CERTIFICATE_OFFSET, Buffer,
                                        Round16(Hdcp22RxPrivateKeySize));

		// Decrypt
		Decrypt(Buffer, Hdcp22RxPrivateKey, Key, Hdcp22RxPrivateKeySize);

		// HDCP 1.4 key A
		// Read from EEPROM
		EepromGet(HDCP14_KEY1_OFFSET, Buffer, Round16(Hdcp14KeyASize));

		// Decrypt
		Decrypt(Buffer, Hdcp14KeyA, Key, Hdcp14KeyASize);

		// HDCP 1.4 key B
		// Read from EEPROM
		EepromGet(HDCP14_KEY2_OFFSET, Buffer, Round16(Hdcp14KeyBSize));

		// Decrypt
		Decrypt(Buffer, Hdcp14KeyB, Key, Hdcp14KeyBSize);
		xil_printf("done\r\n");
		xil_printf("Enabling HDCP functionality\r\n");

		return XST_SUCCESS;
	}

	else
	{
		xil_printf("The password is not valid.\r\n");
	        xil_printf("Is the EEPROM programmed and \\
						  is the password correct?\r\n");
		xil_printf("Disabled HDCP functionality\r\n");
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
 *
 * This function initialize the HDCP 1.4 key manager
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
int XHdcp_KeyManagerInit(u32 BaseAddress, u8 *Hdcp14Key)
{
	u32 RegValue;
	u8 Row;
	u8 i;
	u8 *KeyPtr;
	u8 Status;

	/* Assign key pointer */
	KeyPtr = Hdcp14Key;

	/* Reset */
	Xil_Out32((BaseAddress + 0x0c), (1<<31));

	// There are 41 rows
	for (Row=0; Row<41; Row++)
	{
		/* Set write enable */
		Xil_Out32((BaseAddress + 0x20), 1);

		/* High data */
		RegValue = 0;
		for (i=0; i<4; i++)
		{
			RegValue <<= 8;
			RegValue |= *KeyPtr;
			KeyPtr++;
		}

		/* Write high data */
		Xil_Out32((BaseAddress + 0x2c), RegValue);

		/* Low data */
		RegValue = 0;
		for (i=0; i<4; i++)
		{
			RegValue <<= 8;
			RegValue |= *KeyPtr;
			KeyPtr++;
		}

		/* Write low data */
		Xil_Out32((BaseAddress + 0x30), RegValue);

		/* Table / Row Address */
		Xil_Out32((BaseAddress + 0x28), Row);

		// Write in progress
		do
		{
			RegValue = Xil_In32(BaseAddress + 0x24);
			RegValue &= 1;
		} while (RegValue != 0);
	}

	// Verify

	/* Re-Assign key pointer */
	KeyPtr = Hdcp14Key;

	/* Default Status */
	Status = XST_SUCCESS;

	/* Start at row 0 */
	Row = 0;

	do
	{
		/* Set read enable */
		Xil_Out32((BaseAddress + 0x20), (1<<1));

		/* Table / Row Address */
		Xil_Out32((BaseAddress + 0x28), Row);

		// Read in progress
		do
		{
			RegValue = Xil_In32(BaseAddress + 0x24);
			RegValue &= 1;
		} while (RegValue != 0);

		/* High data */
		RegValue = 0;
		for (i=0; i<4; i++)
		{
			RegValue <<= 8;
			RegValue |= *KeyPtr;
			KeyPtr++;
		}

		if (RegValue != Xil_In32(BaseAddress + 0x2c))
			Status = XST_FAILURE;

		/* Low data */
		RegValue = 0;
		for (i=0; i<4; i++)
		{
			RegValue <<= 8;
			RegValue |= *KeyPtr;
			KeyPtr++;
		}

		if (RegValue != Xil_In32(BaseAddress + 0x30))
			Status = XST_FAILURE;

		/* Increment row */
		Row++;

	} while ((Row<41) && (Status == XST_SUCCESS));

	if (Status == XST_SUCCESS)
	{
		/* Set read lockout */
		Xil_Out32((BaseAddress + 0x20), (1<<31));

		/* Start AXI-Stream */
		Xil_Out32((BaseAddress + 0x0c), (1));
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function gets the password from the uart
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *
 ******************************************************************************/
static u8 EnterPassword(u8 *Password)
{
	u8 Data;
	u8 i;
	u8 *PasswordPtr;

	// Assign pointer
	PasswordPtr = Password;

	// Clear password
	memset(PasswordPtr, 0x00, 32);

	xil_printf("Enter Password ->");

	i = 0;
	while (1) {
		/* Check if the UART has any data */
#if defined (XPAR_XUARTLITE_NUM_INSTANCES)
		if (!XUartLite_IsReceiveEmpty(XHDCP_UART_BASEADDR)) {
			/* Read data from uart */
			Data = XUartLite_RecvByte(XHDCP_UART_BASEADDR);

			/* Send response to user */
			XUartLite_SendByte(XHDCP_UART_BASEADDR, '.');
#else
			if (XUartPs_IsReceiveData(XHDCP_UART_BASEADDR)) {
				/* Read data from uart */
				Data = XUartPs_RecvByte(XHDCP_UART_BASEADDR);

				/* Send response to user */
				XUartPs_SendByte(XHDCP_UART_BASEADDR, '.');
#endif

			/* Execute */
			if ((Data == '\n') || (Data == '\r')) {
				return TRUE;
			}

			/* Store Data */
			else {
				if (i >= 32)
					return TRUE;
				else {
					*(PasswordPtr + i) = Data;
					i++;
				}
			}
		}
	}
}

/*****************************************************************************/
/**
 *
 * This function rounds the length to a 16 bytes boundary
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
static u16 Round16(u16 Size)
{
	if (Size % 16)
		return ((Size/16)+1) * 16;
	else
		return Size;
};

/*****************************************************************************/
/**
 *
 * This function decrypts the HDCP keys
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
static void Decrypt(u8 *CipherBufferPtr,u8 *PlainBufferPtr,u8 *Key, u16 Length)
{
    u8 i;
    u8 *AesBufferPtr;
    u16 AesLength;
    aes256_context ctx;

    // Assign local Pointer
    AesBufferPtr = CipherBufferPtr;

    // Initialize AES256
    aes256_init(&ctx, Key);

    AesLength = Length/16;
    if (Length % 16) {
	AesLength++;
    }

    for (i=0; i<AesLength; i++)
    {
	// Decrypt
	aes256_decrypt_ecb(&ctx, AesBufferPtr);

		// Increment pointer
	AesBufferPtr += 16;	// The aes always encrypts 16 bytes
    }

    // Done
    aes256_done(&ctx);

   // Clear Buffer
    memset(PlainBufferPtr, 0x00, Length);

    // Copy buffers
    memcpy(PlainBufferPtr, CipherBufferPtr, Length);
}

/*****************************************************************************/
/**
 *
 * This function gets a HDCP array from the eeprom
 *
 *
 * @return	The number of bytes read. A value less than the specified input
 *		value indicates an error.
 *
 ******************************************************************************/
static u16 EepromGet(u16 Address, u8 *BufferPtr, u16 Length)
{
	u16 i;
	u8 BytesRead;
	u16 TotalBytesRead;
	u8 *Ptr;

	Ptr = BufferPtr;
	TotalBytesRead = 0;
	for (i=0; i<(Length / XHDCP_EEPROM_PAGE_SIZE); i++)
	{
		BytesRead = EepromReadByte(Address, Ptr, XHDCP_EEPROM_PAGE_SIZE);
		TotalBytesRead += BytesRead;
		Address += BytesRead;
		Ptr += BytesRead;
	}

	if (Length > TotalBytesRead)
	{
		BytesRead = EepromReadByte(Address, Ptr, (Length - TotalBytesRead));
		TotalBytesRead += BytesRead;
	}
	return TotalBytesRead;
}

/*****************************************************************************/
/**
* This function reads a number of bytes from the IIC serial EEPROM into a
* specified buffer.
*
* @param	Address contains the address in the EEPROM to read from.
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*		This value is not constrained by the page size of the device
*		such that up to 64K may be read in one call.
*
* @return	The number of bytes read. A value less than the specified input
*		value indicates an error.
*
* @note		None.
*
****************************************************************************/
u8 EepromReadByte(u16 Address, u8 *BufferPtr, u16 ByteCount)
{
	volatile u8 ReceivedByteCount;
	u16 StatusReg;
	u8 WriteBuffer[sizeof(Address)];

	WriteBuffer[0] = (u8)(Address >> 8);
	WriteBuffer[1] = (u8)(Address);

	/*
	 * Set the address register to the specified address by writing
	 * the address to the device, this must be tried until it succeeds
	 * because a previous write to the device could be pending and it
	 * will not ack until that write is complete.
	 */
	do {
		StatusReg = XIic_ReadReg(XHDCP_IIC_BASEADDR, XIIC_SR_REG_OFFSET);
		if(!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			ReceivedByteCount = XIic_Send(XHDCP_IIC_BASEADDR,
							XHDCP_EEPROM_ADDRESS,
							WriteBuffer,
							sizeof(Address),
							XIIC_STOP);

			if (ReceivedByteCount != sizeof(Address)) {

				/* Send is aborted so reset Tx FIFO */
				XIic_WriteReg(XHDCP_IIC_BASEADDR,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_TX_FIFO_RESET_MASK);
				XIic_WriteReg(XHDCP_IIC_BASEADDR,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_ENABLE_DEVICE_MASK);
			}
		}

	} while (ReceivedByteCount != sizeof(Address));

	/*
	 * Read the number of bytes at the specified address from the EEPROM.
	 */
	ReceivedByteCount = XIic_Recv(XHDCP_IIC_BASEADDR, XHDCP_EEPROM_ADDRESS,
					BufferPtr, ByteCount, XIIC_STOP);

	/*
	 * Return the number of bytes read from the EEPROM.
	 */
	return ReceivedByteCount;
}
