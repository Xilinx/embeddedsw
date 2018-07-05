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
* 6.0   cog    02/21/19 Removed unnecessary register writes.
*              02/21/19 Set frequency and sample rate to appropriate values
*                       for MTS.
*              02/21/19 Set metal log level to DEBUG.
* 7.0   cog    07/25/19 Updated example for new metal register API.
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
#ifdef __BAREMETAL__
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#else
#define RFDC_DEVICE_ID 	0
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
	struct metal_device *deviceptr;
#endif

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XRFDC_FAILURE;
	}
	metal_set_log_level(METAL_LOG_DEBUG);
    ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
    if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}

#ifndef __BAREMETAL__
	status = XRFdc_RegisterMetal(RFdcInstPtr, RFdcDeviceId, &deviceptr);
	if (status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
#endif

    status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
    if (status != XRFDC_SUCCESS) {
        printf("RFdc Init Failure\n\r");
    }

#ifdef XPS_BOARD_ZCU111
    /*Setting Frequency & Sample Rate to Appropriate Values for MTS*/
    printf("Configuring Clock Frequency and Sampling Rate\n");
    status = XRFdc_DynamicPLLConfig(RFdcInstPtr, XRFDC_DAC_TILE, 0, XRFDC_EXTERNAL_CLK, 122.88,3932.16);
	if (status != XRFDC_SUCCESS) {
		printf("ERROR: Could not configure PLL For DAC 0");
		return XRFDC_FAILURE;
	}
	status = XRFdc_DynamicPLLConfig(RFdcInstPtr, XRFDC_DAC_TILE, 1, XRFDC_EXTERNAL_CLK, 122.88,3932.16);
	if (status != XRFDC_SUCCESS) {
		printf("ERROR: Could not configure PLL For DAC 1");
		return XRFDC_FAILURE;
	}
	status = XRFdc_DynamicPLLConfig(RFdcInstPtr, XRFDC_ADC_TILE, 0, XRFDC_EXTERNAL_CLK, 122.88,3932.16);
	if (status != XRFDC_SUCCESS) {
		printf("ERROR: Could not configure PLL For ADC 0");
		return XRFDC_FAILURE;
	}
	status = XRFdc_DynamicPLLConfig(RFdcInstPtr, XRFDC_ADC_TILE, 2, XRFDC_EXTERNAL_CLK, 122.88,3932.16);
	if (status != XRFDC_SUCCESS) {
		printf("ERROR: Could not configure PLL For ADC 2");
		return XRFDC_FAILURE;
	}
#endif

    printf("=== RFdc Initialized - Running Multi-tile Sync ===\n");

    /* ADC MTS Settings */
    XRFdc_MultiConverter_Sync_Config ADC_Sync_Config;

    /* DAC MTS Settings */
    XRFdc_MultiConverter_Sync_Config DAC_Sync_Config;

    /* Run MTS for the ADC & DAC */
    printf("\n=== Run DAC Sync ===\n");

    /* Initialize DAC MTS Settings */
    XRFdc_MultiConverter_Init (&DAC_Sync_Config, 0, 0);
    DAC_Sync_Config.Tiles = 0x3;	/* Sync DAC tiles 0 and 1 */

    status_dac = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_DAC_TILE,
					&DAC_Sync_Config);
    if(status_dac == XRFDC_MTS_OK){
	printf("INFO : DAC Multi-Tile-Sync completed successfully\n");
    }else{
	printf("ERROR : DAC Multi-Tile-Sync did not complete successfully. Error code is %u \n",status_dac);
	return status_dac;
    }

    printf("\n=== Run ADC Sync ===\n");

    /* Initialize ADC MTS Settings */
    XRFdc_MultiConverter_Init (&ADC_Sync_Config, 0, 0);
    ADC_Sync_Config.Tiles = 0x5;	/* Sync ADC tiles 0, 2 */

    status_adc = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_ADC_TILE,
					&ADC_Sync_Config);
    if(status_adc == XRFDC_MTS_OK){
	printf("INFO : ADC Multi-Tile-Sync completed successfully\n");
    }else{
		printf("ERROR : ADC Multi-Tile-Sync did not complete successfully. Error code is %u \n",status_adc);
		return status_adc;
    }

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

    return XRFDC_MTS_OK;
}
