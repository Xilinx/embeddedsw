/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiic_pmbus_polled_example.c
*
* This file consists of a polled mode design example which uses the Xilinx
* IIC device and low-level driver to communicate with a PMBus device.
*
* The example demonstrates how to read voltage and temperature from a
* PMBus-compliant device using the IIC interface. It includes conversion
* routines for PMBus Linear11 and Linear16 formats.
*
* The XIic_Send() API is used to transmit the data and XIic_Recv() API is used
* to receive the data.
*
* The define PMBUS_SLAVE_ADDR in this file needs to be changed depending on
* the PMBus device and board configuration on which this example is to be run.
*
* This code assumes that no Operating System is being used.
*
* @note None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ----------  -----------------------------------------------
* 3.15  vlt  02/18/26    Initial version
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xiic.h"
#include "xiic_l.h"
#include "xil_printf.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define XIIC_BASEADDRESS       XPAR_XIIC_0_BASEADDR

/**
 * The ZCU102 and ZCU106 boards use a PMBus MUX address of 0x75 with channel
 * address 0x05 and PMBus slave address 0x13.
 *
 * The ZC702 and ZC706 boards use a PMBus MUX address of 0x74 with channel
 * address 0x80 and PMBus slave address 0x52.
 *
 * The KCU105 board uses a PMBus MUX address of 0x74 or 0x75 with channel
 * address 0x04 and PMBus slave address 0x0A.
 * Please refer to the User Guide of the respective board and the PMBus
 * device datasheet for further information about the correct addresses and
 * channel to use for PMBus communication.
 * The defines below must be updated to match the board in use.
 */
#define IIC_MUX_ADDRESS         0x75
#define IIC_PMBUS_CHANNEL       0x04

/**
 * The following constant defines the address of the PMBus device on the IIC bus.
 * Note that this is a 7-bit address. The PMBus slave address may vary depending
 * on the board and the PMBus device used. For example, the KCU105 board uses
 * 0x0A as the PMBus slave address for its power controller.
 * Please refer to the User Guide of the respective board and the PMBus device
 * datasheet for further information about the correct slave address to use.
 */

#define PMBUS_SLAVE_ADDR	0x0A

/**
 * This define should be uncommented if there is an IIC MUX on the board to which
 * the PMBus device is connected. For example, the KCU105 board uses an IIC MUX
 * to access PMBus devices.
 */

#define IIC_MUX_ENABLE


/**
 * PMBus command definitions:
 * - READ_VOUT uses the Linear16 data format as defined by the PMBus specification.
 * - All other read commands (current, temperature, and frequency) use the
 *   Linear11 data format.
 */
#define PMBUS_CMD_READ_VOUT	0x8B	/** Command to read output voltage (VOUT) */
#define PMBUS_CMD_READ_TEMP	0x8D	/** Command to read temperature (TEMP) */

/**
 * PMBus READ_VOUT uses LINEAR16 format.
 * As per PMBus specification, the exponent is defined by VOUT_MODE.
 * For the PMBus device used in this example, VOUT_MODE exponent
 * is fixed at -12 (device/board specific).
 */
#define PMBUS_VOUT_EXPONENT	(-12)	/** Exponent used in the calculation of vout voltage.*/

/** Shift to get exponent bits */
#define EXPONENT_SHIFT   11

/** Mask for 5-bit exponent */
#define EXPONENT_MASK    0x1F

/** Mask for 11-bit mantissa */
#define MANTISSA_MASK    0x7FF

/** Exponent sign bit */
#define EXP_SIGN_BIT    0x10

/** Mantissa sign bit */
#define MANT_SIGN_BIT   0x400
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static float Pmbus_Linear11_ToFloat(u16 Raw);
static float Pmbus_Linear16_ToFloat(u16 Raw, int Vout_Exp);
#ifdef IIC_MUX_ENABLE
static int MuxInitPolled(void);
#endif
static int Pmbus_ReadWord_Polled(u8 Cmd, u8 *ReadBuf);
int IicPmbusPolledExample(void);

/************************** Variable Definitions *****************************/
XIic IicInstance;    /** The instance of the IIC device. */


u8 ReadBuffer[2];   /** Buffer used to store data read from the PMBus device */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
* Main function to call the PMBus polled example.
*
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;
	xil_printf("IIC PMBus POLLED Example\r\n");
	/**
	 * Run the PMBus example.
	 */
	Status = IicPmbusPolledExample();
	if (Status != XST_SUCCESS) {
		xil_printf("IIC PMBus Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran IIC PMBus Example\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function demonstrates PMBus polled access: initializes IIC, selects MUX
* (if enabled), and reads VOUT, TEMP, from the PMBus device, printing results.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicPmbusPolledExample(void)
{
	int Status;
	XIic_Config *ConfigPtr;
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
	 * Start the IIC device.
	 */
	Status = XIic_Start(&IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef IIC_MUX_ENABLE
	/**
        * Initialize the IIC MUX on boards where the PMBus
        * devices are connected through the MUX.
        */
	Status = MuxInitPolled();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/**
	 * Combine the two bytes into a 16-bit raw value, convert it to voltage using
	 * Linear16 format, scale it to centivolts, and print the result.
	 */
	Status = Pmbus_ReadWord_Polled(PMBUS_CMD_READ_VOUT, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	RawData = ((u16)ReadBuffer[1] << 8) | ReadBuffer[0];
	Voltage = Pmbus_Linear16_ToFloat(RawData,PMBUS_VOUT_EXPONENT);
	Centi = (int)(Voltage * 100 + 0.5f);
	xil_printf("VOUT: RAW = 0x%04X, Voltage = %d.%02d V\r\n",RawData, Centi / 100, Centi % 100);

	/**
	 * Convert raw PMBus temperature data (Linear11) to centi-degrees Celsius and print
	 */
	Status = Pmbus_ReadWord_Polled(PMBUS_CMD_READ_TEMP, ReadBuffer);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
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

#ifdef IIC_MUX_ENABLE
/*****************************************************************************/
/**
* This function initializes the IIC MUX to select the PMBus channel in polled mode.
*
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int MuxInitPolled(void)
{
	int Bytes;
	u8 MuxVal;
	MuxVal = IIC_PMBUS_CHANNEL;
	/* Ensure the IIC bus is free before accessing the IIC MUX */
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Send the channel selection value to the IIC MUX
	 */
	Bytes = XIic_Send(IicInstance.BaseAddress, IIC_MUX_ADDRESS, &MuxVal, 1, XIIC_STOP);
	if (Bytes != 1) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function reads a 16-bit word from the PMBus device using polled IIC.
*
* @param	Cmd is the PMBus command code to send.
* @param	ReadBuf is a pointer to the buffer to store the 2 bytes read.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int Pmbus_ReadWord_Polled(u8 Cmd, u8 *ReadBuf)
{
	int Bytes;
	/**
	 * Ensure the IIC bus is free before starting a transaction
	 */
	if (XIic_WaitBusFree(IicInstance.BaseAddress) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/**
	 * Send the PMBus command byte with a repeated-start
	 */
	Bytes = XIic_Send(IicInstance.BaseAddress, PMBUS_SLAVE_ADDR, &Cmd, 1, XIIC_REPEATED_START);
	if (Bytes != 1) {
		return XST_FAILURE;
	}

	/**
	 * Read 2 data bytes from the PMBus device and terminate with STOP
	 */
	Bytes = XIic_Recv(IicInstance.BaseAddress, PMBUS_SLAVE_ADDR, ReadBuf, 2, XIIC_STOP);
	if (Bytes != 2) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function converts a PMBus Linear11 formatted value to float.
*
* @param	Raw is the 16-bit raw value from the PMBus device.
*
* @return	Floating point value after conversion.
*
* @note		None.
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
* This function converts a PMBus Linear16 formatted value to float.
* Used for voltage (READ_VOUT).
*
* @param	Raw is the 16-bit raw value from the PMBus device.
* @param	Vout_Exp is the exponent for VOUT.
*
* @return	Floating point value after conversion.
*
* @note		None.
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
