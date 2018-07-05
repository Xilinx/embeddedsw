/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xiic_low_level_tempsensor_example.c
*
* This file contains a polled mode design example which uses the Xilinx IIC
* device and low-level driver to execise the temperature sensor on the ML300
* board. This example only performs read operations (receive) from the IIC
* temperature sensor of the platform.
*
* The XIic_Recv() API is used to receive the data.
*
* @note
*
* 7-bit addressing is used to access the tempsensor.
*
* None
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  10/09/03 Initial Release
* 1.00a sv   05/09/05 Minor changes to comply to Doxygen and coding guidelines
* 1.00a mta  03/09/06 Minor updates due to changes in the low level driver for
*			 supporting repeated start functionality.
* 2.00a sdm  09/22/09 Minor modifications as per coding guidelines.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic_l.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_BASE_ADDRESS	XPAR_IIC_0_BASEADDR


/*
 * The following constant defines the address of the IIC
 * temperature sensor device on the IIC bus.  Note that since
 * the address is only 7 bits, this  constant is the address divided by 2.
 */
#define TEMP_SENSOR_ONCHIP_ADDRESS  0x18 /* The actual address is 0x30 */
#define TEMP_SENSOR_AMBIENT_ADDRESS 0x4B /* The actual address is 0x96 */


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int LowLevelTempSensorExample(u32 IicBaseAddress,
				u8 TempSensorAddress,
				u8 *TemperaturePtr);

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the IIC level 0
* driver to read the temperature.
*
* @param	None
*
* @return	Always 0
*
* @note
*
* The main function is returning an integer to prevent compiler warnings.
*
****************************************************************************/
int main(void)
{
	u8 TemperaturePtr;

	/*
	 * Run the example, specify the Base Address that is generated in
	 * xparameters.h
	 */
	LowLevelTempSensorExample(IIC_BASE_ADDRESS,
					TEMP_SENSOR_ONCHIP_ADDRESS,
					&TemperaturePtr);
	return 0;
}

/*****************************************************************************/
/**
*
* The function reads the temperature of the IIC temperature sensor on the
* IIC bus using the low-level driver.
*
* @param	IicBaseAddress is the base address of the device.
* @param	TempSensorAddress is the address of the Temperature Sensor device
*		on the IIC bus.
* @param	TemperaturePtr is the databyte read from the temperature sensor.
*
* @return	The number of bytes read from the temperature sensor, normally one
*		byte if successful.
*
* @note		None.
*
****************************************************************************/
int LowLevelTempSensorExample(u32 IicBaseAddress,
				u8  TempSensorAddress,
				u8 *TemperaturePtr)
{
	int ByteCount;

	ByteCount = XIic_Recv(IicBaseAddress, TempSensorAddress,
				TemperaturePtr, 1, XIIC_STOP);



	return ByteCount;
}
