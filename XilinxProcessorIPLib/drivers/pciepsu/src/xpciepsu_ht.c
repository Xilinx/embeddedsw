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
*******************************************************************************/
/******************************************************************************/
/**
* @file xpciepsu_ht.c
*
* Implements functions to store bar address with config_data (vendor id and
* device id) and bar number.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/

/******************************** Include Files *******************************/
#include "xpciepsu_ht.h"
#include "xpciepsu_common.h"

/**************************** Variable Definitions ****************************/

XEntry_t *XHtHeadPtr;

/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function initializes a new entry to add to list with
* deviceIdVendorId, Bar number and corresponding Bar Addresses.
*
* @param	venDevId u32 value of vendor id and device id.
* @param	bar_address[] array of bar address of size MAX_BARS. Filled with
* bar addresses if implemented else set as 0
* @param   Bus
* @param   Device
* @param   Function
*
* @return	none
*
*******************************************************************************/
void ht_set(u32 venDevId, u64 bar_address[], u8 Bus, u8 Device,
	    u8 Function)
{
	XEntry_t *newpairPtr = NULL;
	XEntry_t *nextPtr = NULL;
	XEntry_t *lastPtr = NULL;

	nextPtr = XHtHeadPtr;

	while (nextPtr != NULL) {
		lastPtr = nextPtr;
		nextPtr = nextPtr->next;
	}

	newpairPtr = (XEntry_t *)malloc(sizeof(XEntry_t));
	newpairPtr->vendorIdDeviceId = venDevId;
	newpairPtr->bus = Bus;
	newpairPtr->device = Device;
	newpairPtr->function = Function;
	for(int i = 0; i < MAX_BARS; i++){
		newpairPtr->barAddress[i] = bar_address[i];
	}
	newpairPtr->next = NULL;

	if (XHtHeadPtr == NULL)
		XHtHeadPtr = newpairPtr;
	else
		lastPtr->next = newpairPtr;
}

/******************************************************************************/
/**
* This function returns Bar Address from the list for bdf and bar no
*
* @param   	Bus
* @param   	Device
* @param   	Function
* @param	bar_no value of BAR number to get bar address
*
* @return	u64 bar_address if available
*			0 if not available
*
*******************************************************************************/
u64 ht_get_bar_addr(u8 Bus, u8 Device, u8 Function, u8 bar_no)
{
	XEntry_t *IteratorPtr;

	IteratorPtr = XHtHeadPtr;

	while (IteratorPtr != NULL && IteratorPtr->vendorIdDeviceId != 0) {
		if (IteratorPtr->bus == Bus && IteratorPtr->device == Device
		    && IteratorPtr->function == Function) {
			return IteratorPtr->barAddress[bar_no];
		}
		IteratorPtr = IteratorPtr->next;
	}
	return 0;
}

/******************************************************************************/
/**
* This function returns deviceIdVendorId of BDF
*
* @param   	Bus
* @param   	Device
* @param   	Function
*
* @return	u32 vendorDeviceId if available
*			0 if not available
*
*******************************************************************************/
u32 ht_get_device_vendor_id(u8 Bus, u8 Device, u8 Function)
{
	XEntry_t *IteratorPtr;

	IteratorPtr = XHtHeadPtr;

	while (IteratorPtr != NULL && IteratorPtr->vendorIdDeviceId != 0) {
		if (IteratorPtr->bus == Bus && IteratorPtr->device == Device
		    && IteratorPtr->function == Function) {
			return IteratorPtr->vendorIdDeviceId;
		}
		IteratorPtr = IteratorPtr->next;
	}
	return 0;
}
/******************************************************************************/
/**
* This function prints all BDFs, Bars and deviceIdVendorId
*
* @return	none
*
*******************************************************************************/
void ht_print_all()
{
	XEntry_t *IteratorPtr;

	IteratorPtr = XHtHeadPtr;

	while (IteratorPtr != NULL && IteratorPtr->vendorIdDeviceId != 0) {
		pciepsu_dbg("bus : 0x%X\r\nDevice : 0x%X\r\nFunction : 0x%X\r\n"
				"Vendor : 0x%X\r\n", IteratorPtr->bus, IteratorPtr->device,
				IteratorPtr->function, IteratorPtr->vendorIdDeviceId);
		pciepsu_dbg("BARs :: ");
		for(int i = 0; i < MAX_BARS; i++){
			pciepsu_dbg("0x%p, ", IteratorPtr->barAddress[i]);
		}
		pciepsu_dbg("\r\n");
		IteratorPtr = IteratorPtr->next;
	}
}
/******************************************************************************/
/**
* This function releases all the initialized data of hash table.
*
* @param	none
*
* @return	none
*
*******************************************************************************/
void ht_deinit(void)
{
	XEntry_t *e = XHtHeadPtr;
	XEntry_t *tempPtr;

	while (e != NULL) {
		tempPtr = e;
		e = e->next;
		free(tempPtr);
	}
	e = NULL;
}
