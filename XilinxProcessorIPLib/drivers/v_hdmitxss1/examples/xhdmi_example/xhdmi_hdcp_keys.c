/******************************************************************************
* Copyright (C) 2018 – 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
#if defined (XPAR_XUARTPSV_NUM_INSTANCES )
#define XHDCP_UART_BASEADDR XPAR_XUARTPSV_0_BASEADDR
#elif defined (XPAR_XUARTLITE_NUM_INSTANCES)
#define XHDCP_UART_BASEADDR XPAR_MB_SS_0_AXI_UARTLITE_BASEADDR
#else
#define XHDCP_UART_BASEADDR XPAR_XUARTPS_0_BASEADDR
#endif

#if defined (XPS_BOARD_VEK280)
#define XHDCP_EEPROM_ADDRESS	  0x50	 /* 0xA0 as an 8 bit number */
#else
#define XHDCP_EEPROM_ADDRESS	  0x53	 /* 0xA0 as an 8 bit number */
#endif
#define XHDCP_EEPROM_PAGE_SIZE    16
#define SIGNATURE_OFFSET          0
#define HDCP22_LC128_OFFSET       16
#define HDCP22_CERTIFICATE_OFFSET 32
#define HDCP14_KEY1_OFFSET        1024
#define HDCP14_KEY2_OFFSET        1536

#if defined (XPS_BOARD_ZCU102) || \
	defined (XPS_BOARD_ZCU104) || \
	defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
#define I2C_REPEATED_START 0x01
#define I2C_STOP 0x00
#else
#define I2C_REPEATED_START XIIC_REPEATED_START
#define I2C_STOP XIIC_STOP
#endif

/************************** Function Prototypes ******************************/
static unsigned XHdcp_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option);
static unsigned XHdcp_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option);
static u16 Round16(u16 Size);
static void Decrypt(u8 *CipherBufferPtr, u8 *PlainBufferPtr, u8 *Key, u16 Length);
static u16 EepromGet(void *IicPtr, u16 Address, u8 *BufferPtr, u16 Length);
u8 EepromReadByte(void *IicPtr, u16 Address, u8 *BufferPtr, u16 ByteCount);
static u8 EnterPassword (u8 *Password);

/*****************************************************************************/
/**
*
* This function send the IIC data to EEPROM
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
static unsigned XHdcp_I2cSend(void *IicPtr, u16 SlaveAddr, u8 *MsgPtr,
							unsigned ByteCount, u8 Option)
{
#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_VEK280)
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

	usleep(10000);

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
* This function send the IIC data to EEPROM
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
static unsigned XHdcp_I2cRecv(void *IicPtr, u16 SlaveAddr, u8 *BufPtr,
							unsigned ByteCount, u8 Option)
{
#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_VEK280)
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
 * This function loads the HDCP keys from the eeprom
 *
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
int XHdcp_LoadKeys(void *IicPtr,
		u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size, u8 *Hdcp22RxPrivateKey, u32 Hdcp22RxPrivateKeySize,
		u8 *Hdcp14KeyA, u32 Hdcp14KeyASize, u8 *Hdcp14KeyB, u32 Hdcp14KeyBSize)
{

#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
#else
    XIic *Iic_Ptr = IicPtr;
#endif

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
	EepromGet(Iic_Ptr, SIGNATURE_OFFSET, &Buffer[0], sizeof(HdcpSignature));
	Decrypt(&Buffer[0], HdcpSignatureBuffer, Key, sizeof(HdcpSignature));

	SignatureOk = TRUE;
	for (i=0; i<sizeof(HdcpSignature); i++)
	{
		if (HdcpSignature[i] != HdcpSignatureBuffer[i])
			SignatureOk = FALSE;
	}

	if (SignatureOk)
	{
		xil_printf("The Password is valid.\r\n");
		xil_printf("Loading HDCP keys from EEPROM...\r\n");

		// HDCP 2.2 LC128
		// Read from EEPROM
		EepromGet(Iic_Ptr, HDCP22_LC128_OFFSET, &Buffer[0], Round16(Hdcp22Lc128Size));

		// Decrypt
		Decrypt(&Buffer[0], Hdcp22Lc128, Key, Hdcp22Lc128Size);

		// Certificate
		// Read from EEPROM
		EepromGet(Iic_Ptr, HDCP22_CERTIFICATE_OFFSET, &Buffer[0], Round16(Hdcp22RxPrivateKeySize));

		// Decrypt
		Decrypt(&Buffer[0], Hdcp22RxPrivateKey, Key, Hdcp22RxPrivateKeySize);

		// HDCP 1.4 key A
		// Read from EEPROM
		EepromGet(Iic_Ptr, HDCP14_KEY1_OFFSET, &Buffer[0], Round16(Hdcp14KeyASize));

		// Decrypt
		Decrypt(&Buffer[0], Hdcp14KeyA, Key, Hdcp14KeyASize);

		// HDCP 1.4 key B
		// Read from EEPROM
		EepromGet(Iic_Ptr, HDCP14_KEY2_OFFSET, &Buffer[0], Round16(Hdcp14KeyBSize));

		// Decrypt
		Decrypt(&Buffer[0], Hdcp14KeyB, Key, Hdcp14KeyBSize);
		xil_printf("Enabling HDCP Functionality\r\n");

		return XST_SUCCESS;
	}

	else
	{
		xil_printf("The password is not valid.\r\n");
		xil_printf("Is the EEPROM programmed and is the password correct ?\r\n");
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
#if defined (XPAR_XUARTPSV_NUM_INSTANCES)
		if (XUartPsv_IsReceiveData(XHDCP_UART_BASEADDR)) {
			/* Read data from uart */
			Data = XUartPsv_RecvByte(XHDCP_UART_BASEADDR);

			/* Send response to user */
			XUartPsv_SendByte(XHDCP_UART_BASEADDR, '.');
#elif defined (XPAR_XUARTLITE_NUM_INSTANCES)
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
static void Decrypt(u8 *CipherBufferPtr, u8 *PlainBufferPtr, u8 *Key, u16 Length)
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
static u16 EepromGet(void *IicPtr, u16 Address, u8 *BufferPtr, u16 Length)
{

#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
#else
    XIic *Iic_Ptr = IicPtr;
#endif

	u16 i;
	u8 BytesRead;
	u16 TotalBytesRead;
	u8 *Ptr;

	Ptr = BufferPtr;
	TotalBytesRead = 0;
	for (i=0; i<(Length / XHDCP_EEPROM_PAGE_SIZE); i++)
	{
		BytesRead = EepromReadByte(Iic_Ptr, Address, Ptr, XHDCP_EEPROM_PAGE_SIZE);
		TotalBytesRead += BytesRead;
		Address += BytesRead;
		Ptr += BytesRead;
	}

	if (Length > TotalBytesRead)
	{
		BytesRead = EepromReadByte(Iic_Ptr, Address, Ptr, (Length - TotalBytesRead));
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
u8 EepromReadByte(void *IicPtr, u16 Address, u8 *BufferPtr, u16 ByteCount)
{
#if defined (XPS_BOARD_ZCU102) || \
    defined (XPS_BOARD_ZCU104) || \
    defined (XPS_BOARD_ZCU106) || \
    defined (XPS_BOARD_VCK190)
    XIicPs *Iic_Ptr = IicPtr;
#else
    XIic *Iic_Ptr = IicPtr;
#endif

	volatile unsigned ReceivedByteCount;
#if (!(defined(__arm__) || (__aarch64__))) || \
	defined (XPS_BOARD_VEK280)
	u16 StatusReg;
#endif
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

#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_VEK280)
		if(!(XIicPs_BusIsBusy(Iic_Ptr))) {
#else
		StatusReg = XIic_ReadReg(Iic_Ptr->BaseAddress, XIIC_SR_REG_OFFSET);
		if(!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
#endif
			ReceivedByteCount = XHdcp_I2cSend(Iic_Ptr, XHDCP_EEPROM_ADDRESS,
									(u8*)WriteBuffer, sizeof(Address),
									I2C_STOP);

			if (ReceivedByteCount != sizeof(Address)) {

#if (defined (ARMR5) || (__aarch64__)) && (!defined XPS_BOARD_VEK280)
				XIicPs_Abort(Iic_Ptr);
#else
				/* Send is aborted so reset Tx FIFO */
				XIic_WriteReg(Iic_Ptr->BaseAddress,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_TX_FIFO_RESET_MASK);
				XIic_WriteReg(Iic_Ptr->BaseAddress,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_ENABLE_DEVICE_MASK);
#endif
			}
		}

	} while (ReceivedByteCount != sizeof(Address));

	/*
	 * Read the number of bytes at the specified address from the EEPROM.
	 */
	ReceivedByteCount = XHdcp_I2cRecv(Iic_Ptr, XHDCP_EEPROM_ADDRESS, BufferPtr,
								ByteCount, I2C_STOP);

	/*
	 * Return the number of bytes read from the EEPROM.
	 */
	return ReceivedByteCount;
}
