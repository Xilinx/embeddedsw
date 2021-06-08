/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule.c
* @addtogroup iomodule_v2_12
* @{
*
* Contains required functions for the XIomodule driver for the Xilinx
* IO Module Interrupt Controller. See xiomodule.h for a detailed
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- --------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.01a sa   04/10/12 Updated with fast interrupt
* 2.1   bss  05/02/14 Modified XIOModule_IsExpired to check for all 1's instead
*		      of 0 in CounterReg.(CR#794167)
* 2.4   mi   09/20/16 Fixed compilation warnings
* 2.5   ms   08/07/17 Fixed compilation warnings
* 2.7   mus  11/09/18 Updated XIOModule_Initialize and
*                     XIOModule_ConnectFastHandler to deal with the
*                     vector address > 32bit.
* 2.12	sk   06/08/21 Update XIOModule_DiscreteRead and XIOModule_DiscreteWrite
*		      API's argument(Channel) datatype to fix the coverity warning.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiomodule.h"
#include "xiomodule_l.h"
#include "xiomodule_i.h"
#include "xiomodule_io.h"
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Variable Definitions *****************************/

/*
 * Array of masks associated with the bit position, improves performance
 * in the ISR and acknowledge functions, this table is shared between all
 * instances of the driver, this table is not statically initialized because
 * the size of the table is based upon the maximum used interrupt id
 */
u32 XIOModule_BitPosMask[XPAR_IOMODULE_INTC_MAX_INTR_SIZE];

/************************** Function Prototypes ******************************/

static void StubHandler(void *CallBackRef);

/*****************************************************************************/
/**
*
* Initialize a specific interrupt controller instance/driver. The
* initialization entails:
*
*	- Initialize fields of the XIOModule structure
*	- Initial vector table with stub function calls
*	- All interrupt sources are disabled
*	- Interrupt output is disabled
*	- All timers are initialized
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*		worked on.
* @param	DeviceId is the unique id of the device controlled by this
*		XIOModule instance.  Passing in a device id associates the
*		generic XIOModule instance to a specific device, as chosen
*		by the caller or application developer.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_DEVICE_IS_STARTED if the device has already been started
*		- XST_DEVICE_NOT_FOUND if device configuration information was
*		not found for a device with the supplied device ID.
*
* @note		None.
*
******************************************************************************/
int XIOModule_Initialize(XIOModule * InstancePtr, u16 DeviceId)
{
	u8 Id;
	XIOModule_Config *CfgPtr;
	u32 NextBitMask = 1;
        int i;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * If the device is started, disallow the initialize and return a status
	 * indicating it is started.  This allows the user to stop the device
	 * and reinitialize, but prevents a user from inadvertently initializing
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_READY) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Lookup the device configuration in the CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	CfgPtr = XIOModule_LookupConfig(DeviceId);
	if (CfgPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Set some default values
	 */
	InstancePtr->IsReady = 0;
	InstancePtr->IsStarted = 0;	/* not started */
	InstancePtr->CfgPtr = CfgPtr;

	InstancePtr->CfgPtr->Options = XIN_SVC_SGL_ISR_OPTION;

	/*
	 * Initialize GPO value from INIT parameter
	 */
        for (i = 0; i < XGPO_DEVICE_COUNT; i++)
		InstancePtr->GpoValue[i] = CfgPtr->GpoInit[i];

	/*
	 * Save the base address pointer such that the registers of the
	 * IO Module can be accessed
	 */
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;

	/*
	 * Initialize all the data needed to perform interrupt processing for
	 * each interrupt ID up to the maximum used
	 */
	for (Id = 0; Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; Id++) {
		/*
		 * Initialize the handler to point to a stub to handle an
		 * interrupt which has not been connected to a handler. Only
		 * initialize it if the handler is 0 or XNullHandler, which
		 * means it was not initialized statically by the tools/user.
		 * Set the callback reference to this instance so that
		 * unhandled interrupts can be tracked.
		 */
		if ((InstancePtr->CfgPtr->HandlerTable[Id].Handler == 0) ||
		    (InstancePtr->CfgPtr->HandlerTable[Id].Handler ==
		     XNullHandler)) {
			InstancePtr->CfgPtr->HandlerTable[Id].Handler =
				StubHandler;
		}
		InstancePtr->CfgPtr->HandlerTable[Id].CallBackRef = InstancePtr;

		/*
		 * Initialize the bit position mask table such that bit
		 * positions are lookups only for each interrupt id, with 0
		 * being a special case
		 * (XIOModule_BitPosMask[] = { 1, 2, 4, 8, ... })
		 */
		XIOModule_BitPosMask[Id] = NextBitMask;
		NextBitMask *= 2;
	}

	/*
	 * Disable all interrupt sources
	 * Acknowledge all sources
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, 0);
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IMR_OFFSET, 0);
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IAR_OFFSET, 0xFFFFFFFF);

	InstancePtr->CurrentIER = 0;
	InstancePtr->CurrentIMR = 0;

	/*
	 * If the fast Interrupt mode is enabled then set all the
	 * interrupts as normal mode and initialize the interrupt hardware
	 * vector table to default ((BaseVector & 0xFFFFFFFFFFFFFF80) | 0x10).
	 */
	if (InstancePtr->CfgPtr->FastIntr == TRUE) {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IMR_OFFSET, 0);

		for (Id = 0; Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE; Id++) {
			if (InstancePtr->CfgPtr->VectorAddrWidth >
				XIOMODULE_STANDARD_VECTOR_ADDRESS_WIDTH)
			{
					XIomodule_Out64(InstancePtr->BaseAddress +
						XIN_IVEAR_OFFSET + Id * 8,
						(InstancePtr->CfgPtr->BaseVector &
					0xFFFFFFFFFFFFFF80ULL) | 0x10);
			} else {
					XIomodule_Out32(InstancePtr->BaseAddress +
						XIN_IVAR_OFFSET + Id * 4,
						(InstancePtr->CfgPtr->BaseVector &
						0xFFFFFF80) | 0x10);
			}
		}
	}

	/*
	 * Initialize all Programmable Interrupt Timers
	 */
        XIOModule_Timer_Initialize(InstancePtr, DeviceId);

	/*
	 * Initialize all UART related status
	 */
        XIOModule_CfgInitialize(InstancePtr, CfgPtr, 0);

	/*
	 * Save the IO Bus base address pointer such that the memory mapped
	 * IO can be accessed
	 */
	InstancePtr->IoBaseAddress = CfgPtr->IoBaseAddress;

	/*
	 * Indicate the instance is now ready to use, successfully initialized
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Starts the IO Module. Interrupts may be generated by the IO Module after this
* function is called.
*
* It is necessary for the caller to connect the interrupt handler of this
* component to the proper interrupt source.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*		worked on.
*
* @return
*		- XST_SUCCESS if the device was started successfully
*		- XST_FAILURE if simulation mode was specified and it could not
*		be set because real mode has already been entered.
*
* @note 	Must be called after XIOModule initialization is completed.
*
******************************************************************************/
int XIOModule_Start(XIOModule * InstancePtr)
{
	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Indicate the instance is ready to be used and is started before we
	 * enable the device.
	 */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Stops the interrupt controller by disabling the output from the controller
* so that no interrupts will be caused by the interrupt controller.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Stop(XIOModule * InstancePtr)
{
	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->IsStarted = 0;
}

/*****************************************************************************/
/**
*
* Makes the connection between the Id of the interrupt source and the
* associated handler that is to run when the interrupt is recognized. The
* argument provided in this call as the Callbackref is used as the argument
* for the handler when it is called.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
* @param	Id contains the ID of the interrupt source and should be in the
*		range of 0 to XPAR_IOMODULE_INTC_MAX_INTR_SIZE - 1 with 0
*               being the highest priority interrupt.
* @param	Handler to the handler for that interrupt.
* @param	CallBackRef is the callback reference, usually the instance
*		pointer of the connecting driver.
*
* @return
* 		- XST_SUCCESS if the handler was connected correctly.
*
* @note		Only used with normal interrupt mode.
*		Does not restore normal interrupt mode.
*
* WARNING: The handler provided as an argument will overwrite any handler
* that was previously connected.
*
****************************************************************************/
int XIOModule_Connect(XIOModule * InstancePtr, u8 Id,
		  XInterruptHandler Handler, void *CallBackRef)
{
	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertNonvoid(Handler != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The Id is used as an index into the table to select the proper
	 * handler
	 */
	InstancePtr->CfgPtr->HandlerTable[Id].Handler = Handler;
	InstancePtr->CfgPtr->HandlerTable[Id].CallBackRef = CallBackRef;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Updates the interrupt table with the Null Handler and NULL arguments at the
* location pointed at by the Id. This effectively disconnects that interrupt
* source from any handler. The interrupt is disabled also.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
* @param	Id contains the ID of the interrupt source and should be in the
*		range of 0 to XPAR_IOMODULE_INTC_MAX_INTR_SIZE - 1 with 0
*               being the highest priority interrupt.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XIOModule_Disconnect(XIOModule * InstancePtr, u8 Id)
{
	u32 NewIER;
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Disable the interrupt such that it won't occur while disconnecting
	 * the handler, only disable the specified interrupt id without
	 * modifying the other interrupt ids
	 */
	Mask = XIOModule_BitPosMask[Id]; /* convert integer id to bit mask */

	NewIER = InstancePtr->CurrentIER & ~Mask;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;

	/*
	 * Disconnect the handler and connect a stub, the callback reference
	 * must be set to this instance to allow unhandled interrupts to be
	 * tracked
	 */
	InstancePtr->CfgPtr->HandlerTable[Id].Handler = StubHandler;
	InstancePtr->CfgPtr->HandlerTable[Id].CallBackRef = InstancePtr;
}

/*****************************************************************************/
/**
*
* Enables the interrupt source provided as the argument Id. Any pending
* interrupt condition for the specified Id will occur after this function is
* called.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*		worked on.
* @param	Id contains the ID of the interrupt source and should be in the
*		range of 0 to XPAR_IOMODULE_INTC_MAX_INTR_SIZE - 1 with 0
*		being the highest priority interrupt.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XIOModule_Enable(XIOModule * InstancePtr, u8 Id)
{
	u32 NewIER;
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The Id is used to create the appropriate mask for the
	 * desired bit position. Id currently limited to 0 - 31
	 */
	Mask = XIOModule_BitPosMask[Id];

	/*
	 * Enable the selected interrupt source by using the interrupt enable
	 * current value and then modifying only the specified interrupt id
	 */
	NewIER = InstancePtr->CurrentIER | Mask;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;
}

/*****************************************************************************/
/**
*
* Disables the interrupt source provided as the argument Id such that the
* interrupt controller will not cause interrupts for the specified Id. The
* interrupt controller will continue to hold an interrupt condition for the
* Id, but will not cause an interrupt.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
* @param	Id contains the ID of the interrupt source and should be in
*		the range of 0 to XPAR_IOMODULE_INTC_MAX_INTR_SIZE - 1
*		with 0 being the highest priority interrupt.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XIOModule_Disable(XIOModule * InstancePtr, u8 Id)
{
	u32 NewIER;
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The Id is used to create the appropriate mask for the
	 * desired bit position. Id currently limited to 0 - 31
	 */
	Mask = XIOModule_BitPosMask[Id];

	/*
	 * Disable the selected interrupt source by using the interrupt enable
	 * current value and then modifying only the specified interrupt id
	 */
	NewIER = InstancePtr->CurrentIER & ~Mask;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET, NewIER);
	InstancePtr->CurrentIER = NewIER;
}

/*****************************************************************************/
/**
*
* Acknowledges the interrupt source provided as the argument Id. When the
* interrupt is acknowledged, it causes the interrupt controller to clear its
* interrupt condition.
*
* @param	InstancePtr is a pointer to the XIOModule instance to be
*               worked on.
* @param	Id contains the ID of the interrupt source and should be in
*		the range of 0 to XPAR_IOMODULE_INTC_MAX_INTR_SIZE - 1
*		with 0 being the highest priority interrupt.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XIOModule_Acknowledge(XIOModule * InstancePtr, u8 Id)
{
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * The Id is used to create the appropriate mask for the
	 * desired bit position. Id currently limited to 0 - 31
	 */
	Mask = XIOModule_BitPosMask[Id];

	/*
	 * Acknowledge the selected interrupt source, no read of the acknowledge
	 * register is necessary since only the bits set in the mask will be
	 * affected by the write
	 */
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IAR_OFFSET, Mask);
}

/*****************************************************************************/
/**
*
* A stub for the asynchronous callback. The stub is here in case the upper
* layers forget to set the handler.
*
* @param	CallBackRef is a pointer to the upper layer callback reference
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubHandler(void *CallBackRef)
{
	/*
	 * Verify that the inputs are valid
	 */
	Xil_AssertVoid(CallBackRef != NULL);

	/*
	 * Indicate another unhandled interrupt for stats
	 */
	((XIOModule *) CallBackRef)->UnhandledInterrupts++;
}

/*****************************************************************************/
/**
*
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId is the unique identifier for a device.
*
* @return	A pointer to the XIOModule configuration structure for the
*		specified device, or NULL if the device was not found.
*
* @note		None.
*
******************************************************************************/
XIOModule_Config *XIOModule_LookupConfig(u16 DeviceId)
{
	XIOModule_Config *CfgPtr = NULL;
	u32 i;

	for (i = 0U; i < XPAR_XIOMODULE_NUM_INSTANCES; i++) {
		if (XIOModule_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XIOModule_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}



/*****************************************************************************/
/**
*
* Makes the connection between the Id of the interrupt source and the
* associated handler that is to run when the interrupt is recognized.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	Id contains the ID of the interrupt source and should be in the
*		range of 0 to XPAR_INTC_MAX_NUM_INTR_INPUTS - 1 with 0 being the
*		highest priority interrupt.
* @param	Handler to the handler for that interrupt.
*
* @return
* 		- XST_SUCCESS if the handler was connected correctly.
*
* @note		Only used with fast interrupt mode.
*
* WARNING: The handler provided as an argument will overwrite any handler
* that was previously connected.
*
****************************************************************************/
int XIOModule_ConnectFastHandler(XIOModule *InstancePtr, u8 Id,
				    XFastInterruptHandler Handler)
{
	u32 CurrentIER, NewIMR;
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertNonvoid(Handler != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->CfgPtr->FastIntr == TRUE);

	/*
	 * The Id is used to create the appropriate mask for the
	 * desired bit position. Id currently limited to 0 - 31
	 */
	Mask = XIOModule_BitPosMask[Id];

	/*
	 * Get the Enabled Interrupts and disable the Interrupt if it was
	 * enabled before calling this function
	 */
	CurrentIER = InstancePtr->CurrentIER;
	if (CurrentIER & Mask) {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
				CurrentIER & ~Mask);
	}

	/*
	 * Assign the handler information and set the hardware vector
	 */
	InstancePtr->CfgPtr->HandlerTable[Id].Handler = NULL;
	InstancePtr->CfgPtr->HandlerTable[Id].CallBackRef = InstancePtr;

	if (InstancePtr->CfgPtr->VectorAddrWidth > XIOMODULE_STANDARD_VECTOR_ADDRESS_WIDTH)
	{
		XIomodule_Out64(InstancePtr->BaseAddress + XIN_IVEAR_OFFSET + (Id * 8),
			(UINTPTR) Handler);
	} else {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IVAR_OFFSET + (Id * 4),
			(UINTPTR) Handler);
	}

	/*
	 * Set the selected interrupt source to use fast interrupt
	 */
	NewIMR = InstancePtr->CurrentIMR | Mask;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IMR_OFFSET, NewIMR);
	InstancePtr->CurrentIMR = NewIMR;

	/*
	 * Enable Interrupt if it was enabled before calling this function
	 */
	if (CurrentIER & Mask) {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
				CurrentIER);
	}

	return XST_SUCCESS;
}



/*****************************************************************************/
/**
*
* Sets the normal interrupt mode for the specified interrupt in the Interrupt
* Mode Register, by resetting the vector to (BaseVector & 0xFFFFFF80) | 0x10
* and selecting normal mode.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	Id contains the ID of the interrupt source and should be in the
*		range of 0 to XPAR_INTC_MAX_NUM_INTR_INPUTS - 1 with 0 being the
*		highest priority interrupt.
*
* @return	None.
*
* @note		Only used with fast interrupt mode.
*
****************************************************************************/
void XIOModule_SetNormalIntrMode(XIOModule *InstancePtr, u8 Id)
{
	u32 CurrentIER, NewIMR;
	u32 Mask;

	/*
	 * Assert the arguments
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Id < XPAR_IOMODULE_INTC_MAX_INTR_SIZE);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->CfgPtr->FastIntr == TRUE);

	/*
	 * The Id is used to create the appropriate mask for the
	 * desired bit position. Id currently limited to 0 - 31
	 */
	Mask = XIOModule_BitPosMask[Id];

	/*
	 * Get the Enabled Interrupts and disable the Interrupt if it was
	 * enabled before calling this function
	 */
	CurrentIER = InstancePtr->CurrentIER;
	if (CurrentIER & Mask) {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
				CurrentIER & ~Mask);
	}

	/*
	 * Set the selected interrupt source to use normal interrupt
	 */
	NewIMR = InstancePtr->CurrentIMR & ~Mask;
	XIomodule_Out32(InstancePtr->BaseAddress + XIN_IMR_OFFSET, NewIMR);
	InstancePtr->CurrentIMR = NewIMR;

	if (InstancePtr->CfgPtr->VectorAddrWidth > XIOMODULE_STANDARD_VECTOR_ADDRESS_WIDTH)
	{
		XIomodule_Out64(InstancePtr->BaseAddress + XIN_IVEAR_OFFSET + (Id * 8),
			(InstancePtr->CfgPtr->BaseVector & 0xFFFFFF80) | 0x10);
	} else {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IVAR_OFFSET + (Id * 4),
		    	(InstancePtr->CfgPtr->BaseVector & 0xFFFFFF80) | 0x10);
	}

	/*
	 * Disconnect the handler and connect a stub, the callback reference
	 * must be set to this instance to allow unhandled interrupts to be
	 * tracked
	 */
	InstancePtr->CfgPtr->HandlerTable[Id].Handler =	StubHandler;
	InstancePtr->CfgPtr->HandlerTable[Id].CallBackRef = InstancePtr;

	/*
	 * Enable Interrupt if it was enabled before calling this function
	 */
	if (CurrentIER & Mask) {
		XIomodule_Out32(InstancePtr->BaseAddress + XIN_IER_OFFSET,
				CurrentIER);
	}
}


/****************************************************************************/
/**
* Read state of discretes for the specified GPI channel.
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*               worked on.
* @param	Channel contains the channel of the GPI (1, 2, 3 or 4) to
*               operate on.
*
* @return	Current copy of the discretes register.
*
*****************************************************************************/
u32 XIOModule_DiscreteRead(XIOModule * InstancePtr, u32 Channel)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Channel >= 1) && (Channel <= XGPI_DEVICE_COUNT));

	return XIOModule_ReadReg(InstancePtr->BaseAddress,
			((Channel - 1) * XGPI_CHAN_OFFSET) + XGPI_DATA_OFFSET);
}

/****************************************************************************/
/**
* Write to discretes register for the specified GPO channel.
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*               worked on.
* @param	Channel contains the channel of the GPO (1, 2, 3 or 4) to
*               operate on.
* @param	Data is the value to be written to the discretes register.
*
* @return	None.
*
*****************************************************************************/
void XIOModule_DiscreteWrite(XIOModule * InstancePtr,
			     u32 Channel,
			     u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Channel >= 1) && (Channel <= XGPO_DEVICE_COUNT));

	XIOModule_WriteReg(InstancePtr->BaseAddress,
			((Channel - 1) * XGPO_CHAN_OFFSET) + XGPO_DATA_OFFSET,
			Data);
	InstancePtr->GpoValue[Channel - 1] = Data;
}



/*****************************************************************************/
/**
*
* Initializes a specific timer instance/driver. Initialize fields of the
* XIOModule structure, then reset the timer
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	DeviceId is the unique id of the device controlled by this
*		XIOModule component.  Passing in a device id associates the
*		generic XIOModule component to a specific device, as chosen by
*		the caller or application developer.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_DEVICE_IS_STARTED if the device has already been started
*		- XST_DEVICE_NOT_FOUND if the device doesn't exist
*
* @note		None.
*
******************************************************************************/
int XIOModule_Timer_Initialize(XIOModule * InstancePtr, u16 DeviceId)
{
	XIOModule_Config *IOModuleConfigPtr;
	int TimerNumber;
	u32 TimerOffset;
	u32 StatusReg;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component.
	 */
	IOModuleConfigPtr = XIOModule_LookupConfig(DeviceId);

	if (IOModuleConfigPtr == (XIOModule_Config *) NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Check each of the timers of the device, if any are already
	 * running, then the device should not be initialized. This allows the
	 * user to stop the device and reinitialize, but prevents a user from
	 * inadvertently initializing.
	 */
	for (TimerNumber = 0;
	     TimerNumber < XTC_DEVICE_TIMER_COUNT; TimerNumber++) {
		TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

		/*
		 * Use the current register contents and check if the timer
		 * counter is started and running, note that this is not
		 * destructive if the timer counter is already started
		 */
		StatusReg = InstancePtr->CurrentTCSR[TimerNumber];
		if (StatusReg & XTC_CSR_ENABLE_TMR_MASK) {
			continue;
		}

		/*
		 * Set some default values, including setting the callback
		 * handlers to stubs.
		 */
		InstancePtr->BaseAddress = IOModuleConfigPtr->BaseAddress;
		InstancePtr->Handler = NULL;
		InstancePtr->CallBackRef = NULL;

		/*
		 * Clear the statistics for this driver
		 */
		InstancePtr->Timer_Stats[TimerNumber].Interrupts = 0;

		/* Initialize the registers of each timer in the device */

		/*
		 * Set the Load register to 0
		 */
		XIOModule_WriteReg(InstancePtr->BaseAddress,
				   TimerOffset + XTC_TLR_OFFSET, 0);
		InstancePtr->CurrentTLR[TimerNumber] = 0;

		/*
		 * Set the control/status register to complete initialization
		 * by clearing the reset bit which was just set
		 */
		XIOModule_WriteReg(InstancePtr->BaseAddress,
				   TimerOffset + XTC_TCSR_OFFSET, 0);
		InstancePtr->CurrentTCSR[TimerNumber] = 0;
	}

	/*
	 * Indicate the instance is ready to use, successfully initialized
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Starts the specified timer counter of the device such that it starts running.
* The timer counter is reset before it is started and the reset value is
* loaded into the timer counter.
*
* If interrupt mode is specified in the options, it is necessary for the caller
* to connect the interrupt handler of the timer to the interrupt source,
* typically an interrupt controller, and enable the interrupt within the
* interrupt controller.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer of the device to operate on.
*		Each device may contain multiple timers. The timer
*		number is a zero based number with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Timer_Start(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 NewControlStatus;
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set the new value the current contents such that only the necessary
	 * bits of the register are modified in the following operations
	 */
	NewControlStatus = InstancePtr->CurrentTCSR[TimerNumber] |
			   XTC_CSR_ENABLE_TMR_MASK;

	/*
	 * Remove the reset condition such that the timer starts
	 * running with the value loaded from the compare register
	 */
	XIOModule_WriteReg(InstancePtr->BaseAddress,
			   TimerOffset + XTC_TCSR_OFFSET, NewControlStatus);
	InstancePtr->CurrentTCSR[TimerNumber] = NewControlStatus;
}

/*****************************************************************************/
/**
*
* Stops the timer by disabling it.
*
* It is the callers' responsibility to disconnect the interrupt handler of the
* timer from the interrupt source, typically an interrupt controller,
* and disable the interrupt within the interrupt controller.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Timer_Stop(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 NewControlStatus;
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set the new value based on the current contents
	 */
	NewControlStatus = InstancePtr->CurrentTCSR[TimerNumber];

	/*
	 * Disable the timer counter such that it's not running
	 */
	NewControlStatus &= ~(XTC_CSR_ENABLE_TMR_MASK);

	/*
	 * Write out the updated value to the actual register.
	 */
	XIOModule_WriteReg(InstancePtr->BaseAddress,
			   TimerOffset + XTC_TCSR_OFFSET, NewControlStatus);
	InstancePtr->CurrentTCSR[TimerNumber] = NewControlStatus;
}

/*****************************************************************************/
/**
*
* Get the current value of the specified timer counter.  The timer counter
* may be either incrementing or decrementing based upon the current mode of
* operation.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number  with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	The current value for the timer counter.
*
* @note		None.
*
******************************************************************************/
u32 XIOModule_GetValue(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XIOModule_ReadReg(InstancePtr->BaseAddress,
				 TimerOffset + XTC_TCR_OFFSET);
}

/*****************************************************************************/
/**
*
* Set the reset value for the specified timer counter. This is the value
* that is loaded into the timer counter when it is reset. This value is also
* loaded when the timer counter is started.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number  with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
* @param	ResetValue contains the value to be used to reset the timer
*		counter.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_SetResetValue(XIOModule * InstancePtr, u8 TimerNumber,
			     u32 ResetValue)
{
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XIOModule_WriteReg(InstancePtr->BaseAddress,
			   TimerOffset + XTC_TLR_OFFSET,
			   ResetValue);
	InstancePtr->CurrentTLR[TimerNumber] = ResetValue;
}

/*****************************************************************************/
/**
*
* Returns the timer counter value that was captured the last time the external
* capture input was asserted.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number  with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	The current capture value for the indicated timer counter.
*
* @note		None.
*
*******************************************************************************/
u32 XIOModule_GetCaptureValue(XIOModule * InstancePtr, u8 TimerNumber)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return InstancePtr->CurrentTLR[TimerNumber];
}

/*****************************************************************************/
/**
*
* Resets the specified timer counter of the device. A reset causes the timer
* counter to set it's value to the reset value.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number  with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XIOModule_Reset(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 CounterControlReg;
	u32 NewCounterControl;
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read current contents of the register so it won't be destroyed
	 */
	CounterControlReg = InstancePtr->CurrentTCSR[TimerNumber];
	NewCounterControl = CounterControlReg | XTC_CSR_ENABLE_TMR_MASK;

	/*
	 * Reset the timer by toggling the enable bit in the register
	 */
	if ((CounterControlReg & XTC_CSR_ENABLE_TMR_MASK) == 0) {
		XIOModule_WriteReg(InstancePtr->BaseAddress,
				   TimerOffset + XTC_TCSR_OFFSET,
				   NewCounterControl);
		XIOModule_WriteReg(InstancePtr->BaseAddress,
				   TimerOffset + XTC_TCSR_OFFSET,
				   CounterControlReg);
	}
}

/*****************************************************************************/
/**
*
* Checks if the specified timer counter of the device has expired. In capture
* mode, expired is defined as a capture occurred. In compare mode, expired is
* defined as the timer counter rolled over/under for up/down counting.
*
* When interrupts are enabled, the expiration causes an interrupt. This function
* is typically used to poll a timer counter to determine when it has expired.
*
* @param	InstancePtr is a pointer to the XIOModule instance.
* @param	TimerNumber is the timer counter of the device to operate on.
*		Each device may contain multiple timer counters. The timer
*		number is a zero based number  with a range of
*		0 to (XTC_DEVICE_TIMER_COUNT - 1).
*
* @return	TRUE if the timer has expired, and FALSE otherwise.
*
* @note		None.
*
******************************************************************************/
int XIOModule_IsExpired(XIOModule * InstancePtr, u8 TimerNumber)
{
	u32 CounterReg;
	u32 TimerOffset = TimerNumber << XTC_TIMER_COUNTER_SHIFT;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(TimerNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->CfgPtr->PitReadable[TimerNumber]);

	/*
	 * Check if timer is expired
	 */
	if (InstancePtr->CurrentTCSR[TimerNumber] & XTC_CSR_AUTO_RELOAD_MASK) {
		return 1; /* Always expired for reload */
	} else {
		CounterReg = XIOModule_ReadReg(InstancePtr->BaseAddress,
					       TimerOffset + XTC_TCR_OFFSET);

		if ((CounterReg & InstancePtr->CfgPtr->PitMask[TimerNumber]) ==
				InstancePtr->CfgPtr->PitMask[TimerNumber]) {
			return 1;
		} else {
			return 0;
		}
	}
}

/****************************************************************************/
/**
* Read 32-bit word from the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
*		IO Bus address area
*
* @return	Value read from the IO Bus - 32-bit word
*
*****************************************************************************/
u32 XIOModule_IoReadWord(XIOModule * InstancePtr, u32 ByteOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XIomodule_In32((InstancePtr->IoBaseAddress + ByteOffset));
}

/****************************************************************************/
/**
* Read 16-bit halfword from the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
*		IO Bus address area
*
* @return	Value read from the IO Bus - 16-bit halfword
*
*****************************************************************************/
u16 XIOModule_IoReadHalfword(XIOModule * InstancePtr, u32 ByteOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XIomodule_In16((InstancePtr->IoBaseAddress + ByteOffset));
}

/****************************************************************************/
/**
* Read byte from the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
*		IO Bus address area
*
* @return	Value read from the IO Bus - 8-bit byte
*
*****************************************************************************/
u8 XIOModule_IoReadByte(XIOModule * InstancePtr, u32 ByteOffset)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XIomodule_In8((InstancePtr->IoBaseAddress + ByteOffset));
}

/****************************************************************************/
/**
* Write 32-bit word to the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
* 		IO Bus address area
* @param	Data is the value to be written to the IO Bus - 32-bit
*
* @return	None.
*
*****************************************************************************/
void XIOModule_IoWriteWord(XIOModule * InstancePtr,
			   u32 ByteOffset,
			   u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XIomodule_Out32((InstancePtr->IoBaseAddress + ByteOffset), Data);
}

/****************************************************************************/
/**
* Write 16-bit word to the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
* 		IO Bus address area
* @param	Data is the value to be written to the IO Bus - 16-bit
*
* @return	None.
*
*****************************************************************************/
void XIOModule_IoWriteHalfword(XIOModule * InstancePtr,
			       u32 ByteOffset,
			       u16 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XIomodule_Out16((InstancePtr->IoBaseAddress + ByteOffset), Data);
}

/****************************************************************************/
/**
* Write 8-bit word to the IO Bus memory mapped IO
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*		worked on.
* @param	ByteOffset is a byte offset from the beginning of the
* 		IO Bus address area
* @param	Data is the value to be written to the IO Bus - 8-bit
*
* @return	None.
*
*****************************************************************************/
void XIOModule_IoWriteByte(XIOModule * InstancePtr,
			   u32 ByteOffset,
			   u8 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XIomodule_Out8((InstancePtr->IoBaseAddress + ByteOffset), Data);
}
/** @} */
