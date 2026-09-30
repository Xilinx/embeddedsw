/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xiic_pmbus_intr_example.c
* @brief Shows interrupt-mode PMBus transfers over IIC.
*
* This file consists of an interrupt mode design example which uses the Xilinx
* IIC device and low-level driver to communicate with a PMBus device.
*
* The example demonstrates how to read voltage and temperature from a
* PMBus-compliant device using the IIC interface. It includes conversion
* routines for PMBus Linear11 and Linear16 formats.
*
* The XIic_MasterSend() API is used to transmit the data and XIic_MasterRecv() API
* is used to receive the data. Interrupt handlers are used for send, receive, and
* status events.
*
* The define PMBUS_SLAVE_ADDR in this file needs to be changed depending on
* the PMBus device and board configuration on which this example is to be run.
*
* This code assumes that no Operating System is being used.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ----------  -----------------------------------------------
* 3.15  vlt  02/18/26    Initial version
* 3.16  tb   09/22/26    Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xiic.h"
#include "xil_printf.h"
#include "xiic_l.h"
#include "xil_util.h"

#ifdef SDT
#include "xinterrupt_wrap.h"
#endif
/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
/** Base address of the IIC device. */
#define	XIIC_BASEADDRESS		XPAR_XIIC_0_BASEADDR

/*
 * The ZCU102 and ZCU106 boards use a PMBus MUX address of 0x75 with
 * channel address 0x05 and PMBus slave address 0x13.
 * The ZC702 and ZC706 boards use a PMBus MUX address of 0x74 with channel
 * address 0x80 and PMBus slave address 0x52.
 * The KCU105 board uses a PMBus MUX address of 0x74 or 0x75 with channel
 * address 0x04 and PMBus slave address 0x0A.
 * Refer to the User Guide of the respective board and the PMBus
 * device datasheet for further information about the correct addresses and
 * channel to use for PMBus communication.
 * The defines below must be updated to match the board in use.
 */
#define IIC_MUX_ADDRESS         0x75	/**< 7-bit IIC MUX address. */
#define IIC_PMBUS_CHANNEL       0x04	/**< IIC MUX channel for PMBus. */

/*
 * The following constant defines the address of the PMBus device on the
 * IIC bus. Note that this is a 7-bit address. The PMBus slave address
 * may vary depending on the board and the PMBus device used. For
 * example, the KCU105 board uses 0x0A as the PMBus slave address for its
 * power controller. Refer to the User Guide of the respective board and
 * the PMBus device datasheet for further information about the correct
 * slave address to use.
 */
#define PMBUS_SLAVE_ADDR	0x0A	/**< 7-bit PMBus slave address. */

/*
 * This define should be uncommented if there is an IIC MUX on the board
 * to which the PMBus device is connected. For example, the KCU105 board
 * uses an IIC MUX to access PMBus devices.
 */
#define IIC_MUX_ENABLE	/**< Enables the IIC MUX. */

/** Command to read output voltage (VOUT). */
#define PMBUS_CMD_READ_VOUT	0x8B
/** Command to read temperature (TEMP). */
#define PMBUS_CMD_READ_TEMP	0x8D

/*
 * PMBus READ_VOUT uses LINEAR16 format. As per PMBus specification, the
 * exponent is defined by VOUT_MODE. For the PMBus device used in this
 * example, VOUT_MODE exponent is fixed at -12 (device/board specific).
 */
/** VOUT_MODE exponent for LINEAR16 conversion. */
#define PMBUS_VOUT_EXPONENT	(-12)

/*
 * Maximum delay count used for timeout handling
 */
/** Maximum delay count for timeout handling. */
#define MAX_DELAY_CNT               1000

/*
 * Internal IIC event mask
 */
#define EventMask                   0xFF	/**< Internal IIC event mask. */

/*
 * Internal event value used for IIC event checking
 */
/** Internal event value for IIC event checking. */
#define Event_Value                 1

/*
 * Shift to get exponent bits
 */
#define EXPONENT_SHIFT   11	/**< Shift to get exponent bits. */

/*
 * Mask for 5-bit exponent
 */
#define EXPONENT_MASK    0x1F	/**< Mask for 5-bit exponent. */

/*
 * Mask for 11-bit mantissa
 */
#define MANTISSA_MASK    0x7FF	/**< Mask for 11-bit mantissa. */

/*
 * Exponent sign bit
 */
#define EXP_SIGN_BIT    0x10	/**< Exponent sign bit. */

/*
 * Mantissa sign bit
 */
#define MANT_SIGN_BIT   0x400	/**< Mantissa sign bit. */


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifdef IIC_MUX_ENABLE
static int MuxInitIntr(void);
#endif
static void SendHandler(void *CallBackRef, int ByteCount);
static void ReceiveHandler(void *CallBackRef, int ByteCount);
static void StatusHandler(XIic *InstancePtr, int Event);
int IicPmbusInterruptSdtExample(void);
static int Pmbus_SendCmd_Intr(u8 Cmd);
static int Pmbus_ReadWord_Intr(u8 Cmd, u8 *ReadBuf);
static float Pmbus_Linear11_ToFloat(u16 Raw);
static float Pmbus_Linear16_ToFloat(u16 Raw, int Vout_Exp);
/************************** Variable Definitions *****************************/
XIic IicInstance;		/**< The instance of the IIC device. */

#ifndef SDT
INTC Intc;
	/**< The instance of the Interrupt Controller Driver. */
#endif

volatile u8 TransmitComplete;
	/**< Flag to check completion of Transmission. */
volatile u8 ReceiveComplete;	/**< Flag to check completion of Reception. */
volatile u8 ErrorEvent;
	/**< Error event flag set by the IIC interrupt handler. */

u8 WriteBuffer[1];
	/**< Write buffer used for IIC data transmission. */

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief Runs the PMBus interrupt example.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
int main(void)
{
	int Status;
	xil_printf("IIC PMBus interrupt Example\r\n");

	/**
	 * Run the PMBus example.
	 */
	Status = IicPmbusInterruptSdtExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC PMBus Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran IIC PMBus Example\r\n");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
* @brief Demonstrates PMBus access in interrupt mode.
*
* It initializes IIC, sets up interrupts, selects the MUX (if enabled), reads
* voltage and temperature from the PMBus device, and prints the results.
*
* @return
*        - XST_SUCCESS if successful.
*        - XST_FAILURE otherwise.
*
******************************************************************************/
int IicPmbusInterruptSdtExample(void)
{
	int Status;
	XIic_Config *ConfigPtr;
	u8 ReadBuffer[2];
	u16 RawData;
	float Voltage;
	float Temperature;
	int Centi;

	/**
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(XIIC_BASEADDRESS);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Setup the Interrupt System.
	 */
	Status = XSetupInterruptSystem(&IicInstance, &XIic_InterruptHandler, ConfigPtr->IntrId, ConfigPtr->IntrParent,XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Set the Handlers for transmit and reception.
	 */
	XIic_SetSendHandler(&IicInstance, &IicInstance, (XIic_Handler)SendHandler);
	XIic_SetRecvHandler(&IicInstance, &IicInstance, (XIic_Handler)ReceiveHandler);
	XIic_SetStatusHandler(&IicInstance, &IicInstance, (XIic_StatusHandler)StatusHandler);

	/**
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Set the Global Interrupt Enable.
	 */
	XIic_IntrGlobalEnable(IicInstance.BaseAddress);
#ifdef IIC_MUX_ENABLE

	/**
	 * Initialize the IIC MUX on boards where the PMBus
	 * devices are connected through the MUX.
	 */
	Status = MuxInitIntr();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/**
	 * Set the Slave address.
	 */
	XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, PMBUS_SLAVE_ADDR);

	/**
	 * Read the output voltage (VOUT) from the PMBus device
	 */
	Status = Pmbus_ReadWord_Intr(PMBUS_CMD_READ_VOUT, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Combine the two bytes into a 16-bit raw value, convert it to voltage using
	 * Linear16 format, scale it to centivolts, and print the result.
	 */
	RawData = ((u16)ReadBuffer[1] << 8) | ReadBuffer[0];
	Voltage = Pmbus_Linear16_ToFloat(RawData,PMBUS_VOUT_EXPONENT);
	/* VOUT is expected to be non-negative for standard PMBus READ_VOUT */
	Centi = (int)(Voltage * 100 + 0.5f);
	xil_printf("VOUT: RAW = 0x%04X, Voltage = %d.%02d V\r\n",RawData, Centi / 100, Centi % 100);
	/**
	 * Read the output temperature from the PMBus device
	 */
	Status =  Pmbus_ReadWord_Intr(PMBUS_CMD_READ_TEMP, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Convert raw PMBus temperature data (Linear11) to centi-degrees Celsius and print
	 */
	RawData = ((u16)ReadBuffer[1] << 8) | ReadBuffer[0];
	Temperature = Pmbus_Linear11_ToFloat(RawData);
	if (Temperature >= 0.0f) {
		Centi = (int)(Temperature * 100.0f + 0.5f);
	} else {
		Centi = (int)(Temperature * 100.0f - 0.5f);
	}
	int TempWhole = Centi / 100;
	int TempFrac  = Centi % 100;
	if (TempFrac < 0) {
		TempFrac = -TempFrac;
	}
	xil_printf("TEMP: RAW = 0x%04X, Temperature = %d.%02d C\r\n",RawData, TempWhole, TempFrac);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Sends a single command byte to the PMBus slave device.
*
* It uses interrupt-driven IIC communication.
*
* @param[in] Cmd The command byte to be sent to the slave.
*
* @return
*        - XST_SUCCESS if the command was sent successfully.
*        - XST_FAILURE if an error occurred during transmission.
*
* @note This function waits for the IIC bus to become free before sending the
*       command and waits for the transmission to complete.
*
******************************************************************************/
static int Pmbus_SendCmd_Intr(u8 Cmd)
{
	int Status;
	TransmitComplete = 0;

	 /** Ensure bus is free before starting */
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/** Send the command byte to the slave device */
	Status = XIic_MasterSend(&IicInstance, &Cmd, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/**
	 * Wait for transmit completion.
	 *
	 * The TransmitComplete flag is set to 1 by the transmit interrupt handler
	 * once all requested bytes are successfully transmitted. Xil_WaitForEvent()
	 * polls this software flag until it matches the expected completion value
	 * or the timeout expires, preventing an infinite wait in case the interrupt
	 * is not serviced.
	 */
	Status = Xil_WaitForEvent((UINTPTR)&TransmitComplete, EventMask, Event_Value, MAX_DELAY_CNT);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Reads a 2-byte word from the PMBus slave device.
*
* It uses interrupt-driven IIC communication. It first sends the command byte,
* then receives the data.
*
* @param[in] Cmd The command byte to be sent to the slave.
* @param[out] ReadBuf Pointer to the buffer to store the received data.
*
* @return
*        - XST_SUCCESS if the word was read successfully.
*        - XST_FAILURE if an error occurred during the operation.
*
* @note This function uses a repeated start condition between the write
*       (command) and read (data) phases. It waits for the receive operation to
*       complete or timeout.
*
******************************************************************************/
static int Pmbus_ReadWord_Intr(u8 Cmd, u8 *ReadBuf)
{
	int Status;
	u32 OldOpts;
	ReceiveComplete = 0;
	TransmitComplete = 0;
	ErrorEvent = 0;

	/** Ensure bus is free before starting */
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	OldOpts = XIic_GetOptions(&IicInstance);
	/**
	 * PMBus requires a repeated start between command write and data read.
	 * Enable repeated start for the command WRITE phase.
	 */
	XIic_SetOptions(&IicInstance, OldOpts | XII_REPEATED_START_OPTION);
	/** Send command byte (write) */
	Status = Pmbus_SendCmd_Intr(Cmd);
	if (Status != XST_SUCCESS) {
		/** Restore options on failure */
		XIic_SetOptions(&IicInstance, OldOpts);
		return XST_FAILURE;
	}
	/** Disable repeated start BEFORE the READ phase so STOP is generated */
	XIic_SetOptions(&IicInstance, OldOpts);
	/** Receive 2 bytes (read) */
	Status = XIic_MasterRecv(&IicInstance, ReadBuf, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/**
	 * Wait for receive completion.
	 *
	 * The ReceiveComplete flag is set to 1 by the receive interrupt handler
	 * once all requested bytes are successfully received. Xil_WaitForEvent()
	 * polls this software flag until it matches the expected completion value
	 * or the timeout expires, preventing an infinite wait in case the interrupt
	 * is not serviced.
	 */
	Status = Xil_WaitForEvent((UINTPTR)&ReceiveComplete,
                          EventMask, Event_Value, MAX_DELAY_CNT);
	if (Status != XST_SUCCESS || ErrorEvent) {
		return XST_FAILURE;
	}

	/** Make sure bus is free for the next command */
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Converts a PMBus Linear11 formatted value to float.
*
* @param[in] Raw The 16-bit raw value from the PMBus device.
*
* @return Floating point value after conversion.
*
******************************************************************************/
static float Pmbus_Linear11_ToFloat(u16 Raw)
{
	/**
	 * Step 1: Extract the exponent and mantissa fields.
	 *
	 * LINEAR11 format:
	 *   Bits[15:11] : 5-bit signed exponent
	 *   Bits[10:0]  : 11-bit signed mantissa
	 */
	int Exponent  = (Raw >> EXPONENT_SHIFT ) & EXPONENT_MASK;
	int Mantissa  = Raw & MANTISSA_MASK;
	/**
	 * Step 2: Sign-extend the 5-bit exponent.
	 * Bit[4] (0x10) is the sign bit of the exponent(2's complement).
	 */
	if (Exponent & EXP_SIGN_BIT) {
		Exponent |= ~EXPONENT_MASK;
	}
	/**
	 * Step 3: Sign-extend the 11-bit mantissa.
	 * Bit[10] (0x400) is the sign bit of the mantissa(2's complement).
	 */
	if (Mantissa & MANT_SIGN_BIT) {
		Mantissa |= ~MANTISSA_MASK;
	}
	/**
	 * Step 4: Apply the Linear11 conversion formula.
	 * Value = Mantissa × 2^Exponent
	 */
	if (Exponent >= 0) {
		return (float)Mantissa * (float)(1 << Exponent);
	} else {
		return (float)Mantissa / (float)(1 << (-Exponent));
	}
}

/*****************************************************************************/
/**
* @brief Converts a PMBus Linear16 formatted value to float.
*
* Used for voltage (READ_VOUT).
*
* @param[in] Raw The 16-bit raw value from the PMBus device.
* @param[in] Vout_Exp The exponent for VOUT.
*
* @return Floating point value after conversion.
*
******************************************************************************/
static float Pmbus_Linear16_ToFloat(u16 Raw, int Vout_Exp)
{
	/**
	 * Step 1: Convert the 5‑bit exponent to a signed integer.
	 * Bit[4] (0x10) is the sign bit in a 5‑bit two's‑complement exponent.
	 */
	if (Vout_Exp & EXP_SIGN_BIT) {
		Vout_Exp |= ~EXPONENT_MASK;
	}
	/**
	 * Step 2: Apply the Linear16 conversion formula.
	 * Value = Raw × 2^Exponent
	 */
	if (Vout_Exp >= 0) {
		return (float)Raw * (float)(1 << Vout_Exp);
	} else {
		return (float)Raw / (float)(1 << (-Vout_Exp));
	}
}

/*****************************************************************************/
/**
* @brief Handles send completion from interrupt context.
*
* This Send handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been sent.
*
* @param[in] CallBackRef Pointer to the IIC device instance.
* @param[in] ByteCount The number of bytes sent.
*
******************************************************************************/
static void SendHandler(void *CallBackRef, int ByteCount)
{
    (void)ByteCount;
    (void)CallBackRef;

    TransmitComplete = 1;
}

/*****************************************************************************/
/**
* @brief Handles receive completion from interrupt context.
*
* This Receive handler is called asynchronously from an interrupt context and
* indicates that data in the specified buffer has been received.
*
* @param[in] CallBackRef Pointer to the IIC device instance.
* @param[in] ByteCount The number of bytes received.
*
******************************************************************************/
static void ReceiveHandler(void *CallBackRef, int ByteCount)
{
    (void)CallBackRef;
    (void)ByteCount;

    ReceiveComplete = 1;
}

/*****************************************************************************/
/**
* @brief Handles IIC status events from interrupt context.
*
* This status handler is invoked from the interrupt context to report IIC status
* events. It checks for error conditions such as arbitration loss or missing
* slave acknowledgment and sets an error flag.
*
* @param[in] InstancePtr Pointer to the IIC driver instance for which the
*                        handler is being called for.
* @param[in] Event Indicates the condition that has occurred.
*
* @note The errorflag is used by the application to detect PMBus transaction
*       failures.
*
******************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{
	(void)InstancePtr;
	if ((Event == XII_ARB_LOST_EVENT) || (Event == XII_SLAVE_NO_ACK_EVENT)) {
		ErrorEvent = 1;
	}
}

#ifdef IIC_MUX_ENABLE
/*****************************************************************************/
/**
* @brief Initializes the IIC MUX to select the PMBus channel.
*
* @return XST_SUCCESS if pass, otherwise XST_FAILURE.
*
****************************************************************************/
static int MuxInitIntr(void)
{
	int Status;
	Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_SEND_TYPE, IIC_MUX_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	WriteBuffer[0] = IIC_PMBUS_CHANNEL;
	TransmitComplete = 0;
	ErrorEvent = 0
	/* Ensure the IIC bus is free before accessing the IIC MUX */;
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIic_MasterSend(&IicInstance, WriteBuffer, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/**
	*  Wait for transmit completion.
	*
	* The TransmitComplete flag is set to 1 by the transmit interrupt handler
	* once all requested bytes are successfully transmitted. Xil_WaitForEvent()
	* polls this software flag until it matches the expected completion value
	* or the timeout expires, preventing an infinite wait in case the interrupt
	* is not serviced.
	*/
	Status = Xil_WaitForEvent((UINTPTR)&TransmitComplete, EventMask, Event_Value, MAX_DELAY_CNT);
	if (Status != XST_SUCCESS || ErrorEvent) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
#endif
