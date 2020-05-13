/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmutex.c
* @addtogroup mutex_v4_5
* @{
*
* Contains required functions for the XMutex driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00b ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 2.00a hm   04/14/09 Fixed CR 466322, removed extra definitions
*			Also fixed canonical definitions treating an interface
*			as an device instance.
* 3.00a hbm  10/15/09 Migrated to HAL phase 1 to use xil_io, xil_types,
*			and xil_assert.
* 4.00a bss  03/05/14 Modified XMutex_CfgInitialize to fix CR# 770096
* 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XMutex_CfgInitialize API.
* 4.2   mi   09/22/16 Fixed compilation warnings.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>
#include "xmutex.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a specific Mutex instance/driver.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	ConfigPtr is the device configuration structure containing
*		required HW build data.
* @param	EffectiveAddress is the Physical address of the hardware in a
*		Virtual Memory operating system environment.
*		It is the Base Address in a stand alone environment.
*
* @return
*		- XST_SUCCESS if initialization was successful
*
* @note		None.
*
******************************************************************************/
int XMutex_CfgInitialize(XMutex *InstancePtr, XMutex_Config *ConfigPtr,
			 UINTPTR EffectiveAddress)
{
	u32 i;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XMutex));
	memcpy(&InstancePtr->Config, ConfigPtr, sizeof(XMutex_Config));

	InstancePtr->Config.BaseAddress = EffectiveAddress;

	for(i=0; i < ConfigPtr->NumMutex; i++){
		XMutex_Unlock(InstancePtr, i);
	}

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Locks a particular Mutex lock within a Mutex device. Call blocks till the
* Mutex is locked.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number  with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
*
* @return	None
*
* @note
*		- XMutex_Trylock is a blocking call. This call blocks until the
*		  user gets the lock.
*		- Use XMutex_Trylock for a Non-Blocking call. The user gets the
*		  lock if it is available and returns immediately if the lock
*		  is not available.
*
******************************************************************************/
void XMutex_Lock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 LockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | LOCKED_BIT);
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MutexNumber < InstancePtr->Config.NumMutex);

	while (1){
		XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET, LockPattern);
		Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress,
					MutexNumber,
					XMU_MUTEX_REG_OFFSET);
		if (Value == LockPattern) {
			break;
		}
	}
}

/*****************************************************************************/
/**
*
* Locks a particular Mutex lock within a Mutex device. Call returns immediately
* if the Mutex is already locked (This is Non-Blocking call).
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
*
* @return
*		- XST_SUCCESS if locking was successful.
*		- XST_DEVICE_BUSY if the Mutex was found to be already locked
*
* @note
*		- This is Non-Blocking call, the user gets the lock if it is
*		available else XST_DEVICE_BUSY is returned.
*		- Use XMutex_Lock if you need to block until a lock is obtained.
*
******************************************************************************/
int XMutex_Trylock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 LockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | LOCKED_BIT);
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_MUTEX_REG_OFFSET, LockPattern);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);
	if (Value == LockPattern) {
		return XST_SUCCESS;
	} else{
		return XST_DEVICE_BUSY;
	}
}

/*****************************************************************************/
/**
*
* Unlocks a particular Mutex lock within a Mutex device.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
*
* @return
*
*		- XST_SUCCESS if locking was successful.
*		- XST_FAILURE if the Mutex was locked by process with
*		  different ID.
*
* @note		None.
*
******************************************************************************/
int XMutex_Unlock(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 UnLockPattern = ((XPAR_CPU_ID << OWNER_SHIFT) | 0x0);
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);

	/* Verify that the caller actually owns the Mutex */
	if ((Value & OWNER_MASK) != UnLockPattern) {
		return XST_FAILURE;
	}

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_MUTEX_REG_OFFSET, UnLockPattern);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Gets the current lock state of a Mutex lock within a Mutex device.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
*
* @return
*		- TRUE if locked
*		- FALSE if unlocked
*
* @note		None.
*
******************************************************************************/
int XMutex_IsLocked(XMutex *InstancePtr, u8 MutexNumber)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_MUTEX_REG_OFFSET);

	return ((int)(Value & LOCKED_BIT));
}

/*****************************************************************************/
/**
*
* Gets the current status of a Mutex lock within a Mutex device.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
* @param	Locked is a pointer where the current lock status is stored.
*		Sets memory pointed to by 'Locked' to 1 if the Mutex is locked
*		and 0 if it is unlocked.
* @param	Owner is a pointer where the current owner status is stored.
*.		If the Mutex is locked, the memory pointed to by 'Owner' is
*		updated to reflect the CPU ID that has currently locked this
*		Mutex.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XMutex_GetStatus(XMutex *InstancePtr, u8 MutexNumber, u32 *Locked,
			u32 *Owner)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(MutexNumber < InstancePtr->Config.NumMutex);
	Xil_AssertVoid(Locked != NULL);
	Xil_AssertVoid(Owner != NULL);

	Value = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
			       XMU_MUTEX_REG_OFFSET);
	*Locked = (Value & LOCKED_BIT);
	*Owner = (Value & OWNER_MASK) >> OWNER_SHIFT;
}

/*****************************************************************************/
/**
*
* Gets the USER register of a Mutex lock within a Mutex device.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
* @param	User is a pointer to an u32 where the current user register
		value is stored by this function.
* @return
*		- XST_SUCCESS if successful. Memory pointed to by User is
*		  updated to reflect the contents of the user register.
*		- XST_NO_FEATURE if the Mutex was not configured with a USER
*		  register.
*
* @note		None.
*
******************************************************************************/
int XMutex_GetUser(XMutex *InstancePtr, u8 MutexNumber, u32 *User)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);
	Xil_AssertNonvoid(User != NULL);

	if (!(InstancePtr->Config.UserReg)) {
		return XST_NO_FEATURE;
	}

	*User = XMutex_ReadReg(InstancePtr->Config.BaseAddress, MutexNumber,
				XMU_USER_REG_OFFSET);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Sets the USER register of a Mutex lock within a Mutex device.
*
* @param	InstancePtr is a pointer to the XMutex instance to be worked on.
* @param	MutexNumber is the specific Mutex lock within the device to
*		operate on. Each device may contain multiple Mutex locks.
*		The Mutex number is a zero based number with a range of
*		0 - (InstancePtr->Config.NumMutex - 1).
* @param	User is the value to update the USER register with.
*
* @return
*		- XST_SUCCESS if the USER register is written with the
*		  given value .
*		- XST_NO_FEATURE if the Mutex was not configured with a
*		  USER register.
*
* @note		None.

*
******************************************************************************/
int XMutex_SetUser(XMutex *InstancePtr, u8 MutexNumber, u32 User)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(MutexNumber < InstancePtr->Config.NumMutex);

	if (!(InstancePtr->Config.UserReg)) {
		return XST_NO_FEATURE;
	}

	XMutex_WriteReg(InstancePtr->Config.BaseAddress, MutexNumber,
			XMU_USER_REG_OFFSET, User);

	return XST_SUCCESS;
}
/** @} */
