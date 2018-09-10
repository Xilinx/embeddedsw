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
/****************************************************************************/
/**
*
* @file xrfdc_mts_example.c
*
* RFSoC MultiTile Sync Example test application
*
* This example calls the RFdc Multi-tile-sync (MTS) API with the
* following configuration:
* Tiles to Sync: DAC0, DAC1, ADC0, ADC1, ADC2, ADC3.
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 3.1   jm     01/24/18 First release
* 5.0   sk     09/05/18 Rename XRFdc_MTS_RMW_DRP as XRFdc_ClrSetReg.
* 5 0   mus    08/18/18 Updated to remove xparameters.h dependency for linux
*                       platform.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#ifdef __BAREMETAL__
#include "xparameters.h"
#endif
#include "xrfdc.h"
#include "xrfdc_mts.h"
/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#ifndef __BAREMETAL__
#define BUS_NAME        "platform"
#define RFDC_DEVICE_ID	0U
#else
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/

int RFdcMTS_Example(u16 RFdcDeviceId);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */

/****************************************************************************/
/**
*
* Main function that invokes the MTS example in this file.
*
* @param	None.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int Status;

	printf("RFdc MTS Example Test\r\n");
	/*
	 * Specify the Device ID that is generated in xparameters.h.
	 */
	Status = RFdcMTS_Example(RFDC_DEVICE_ID);
	if (Status != XRFDC_SUCCESS) {
		printf("MTS Example Test failed\r\n");
		return XRFDC_FAILURE;
	}

	printf("Successfully ran MTS Example\r\n");
	return XRFDC_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a MTS test on the RFSoC data converter device using the
* driver APIs.
* This function does the following tasks:
*	- Initialize the RFdc device driver instance
*	- Test MTS feature.
*
* @param	RFdcDeviceId is the XPAR_<XRFDC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int RFdcMTS_Example(u16 RFdcDeviceId)
{
	int status, status_adc, status_dac, i;
	u32 factor;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;
#ifndef __BAREMETAL__
	struct metal_device *device;
	struct metal_io_region *io;
	int ret = 0;
	char DeviceName[NAME_MAX];
#endif

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XRFDC_FAILURE;
	}

    ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
    if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}

    status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
    if (status != XRFDC_SUCCESS) {
        printf("RFdc Init Failure\n\r");
    }

#ifndef __BAREMETAL__
	status = XRFdc_GetDeviceNameByDeviceId(DeviceName, RFDC_DEVICE_ID);
	if (status < 0) {
		printf("ERROR: Failed to find rfdc device with device id %d\n",
				RFDC_DEVICE_ID);
		return XRFDC_FAILURE;
	}
	ret = metal_device_open(BUS_NAME, DeviceName, &device);
	if (ret) {
		printf("ERROR: Failed to open device %s.\n", DeviceName);
		return XRFDC_FAILURE;
	}

	/* Map RFDC device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		printf("ERROR: Failed to map RFDC regio for %s.\n",
			  device->name);
		return XRFDC_FAILURE;
	}
	RFdcInstPtr->device = device;
	RFdcInstPtr->io = io;
#endif

    printf("=== RFdc Initialized - Running Multi-tile Sync ===\n");

    /* ADC MTS Settings */
    XRFdc_MultiConverter_Sync_Config ADC_Sync_Config;

    /* DAC MTS Settings */
    XRFdc_MultiConverter_Sync_Config DAC_Sync_Config;

    /* Run MTS for the ADC & DAC */
    printf("\n=== Run DAC Sync ===\n");

    XRFdc_ClrSetReg(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(1) + XRFDC_HSCOM_ADDR,  0xB0, 0x0F, 0x01);
    XRFdc_ClrSetReg(RFdcInstPtr, XRFDC_ADC_TILE_DRP_ADDR(3) + XRFDC_HSCOM_ADDR,  0xB0, 0x0F, 0x01);

    /* Initialize DAC MTS Settings */
    XRFdc_MultiConverter_Init (&DAC_Sync_Config, 0, 0);
    DAC_Sync_Config.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    status_dac = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_DAC_TILE,
					&DAC_Sync_Config);
    if(status_dac == XRFDC_MTS_OK)
	printf("INFO : DAC Multi-Tile-Sync completed successfully\n");


    printf("\n=== Run ADC Sync ===\n");

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config, 0, 0);
    ADC_Sync_Config.Tiles = 0x5;	/* Sync ADC tiles 0, 2 */

    status_adc = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_ADC_TILE,
					&ADC_Sync_Config);
    if(status_adc == XRFDC_MTS_OK)
	printf("INFO : ADC Multi-Tile-Sync completed successfully\n");


    /*
     * Report Overall Latency in T1 (Sample Clocks) and
     * Offsets (in terms of PL words) added to each FIFO
     */
     printf("\n\n=== Multi-Tile Sync Report ===\n");
     for(i=0; i<4; i++) {
         if((1<<i)&DAC_Sync_Config.Tiles) {
                 XRFdc_GetInterpolationFactor(RFdcInstPtr, i, 0, &factor);
                 printf("DAC%d: Latency(T1) =%3d, Adjusted Delay"
				 "Offset(T%d) =%3d\n", i, DAC_Sync_Config.Latency[i],
						 factor, DAC_Sync_Config.Offset[i]);
         }
     }
     for(i=0; i<4; i++) {
         if((1<<i)&ADC_Sync_Config.Tiles) {
                 XRFdc_GetDecimationFactor(RFdcInstPtr, i, 0, &factor);
                 printf("ADC%d: Latency(T1) =%3d, Adjusted Delay"
				 "Offset(T%d) =%3d\n", i, ADC_Sync_Config.Latency[i],
						 factor, ADC_Sync_Config.Offset[i]);
         }
     }

    return 0;
}
