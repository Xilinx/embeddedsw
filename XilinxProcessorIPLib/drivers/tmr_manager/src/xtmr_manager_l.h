/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_l.h
* @addtogroup tmr_manager_v1_1
* @{
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  High-level driver functions
* are defined in xtmr_manager.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sa   04/05/17 First release
* </pre>
*
*****************************************************************************/

#ifndef XTMR_MANAGER_L_H /* prevent circular inclusions */
#define XTMR_MANAGER_L_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/* TMR Manager register offsets */

#define XTM_CR_OFFSET		0	/* control reg                 */
#define XTM_FFR_OFFSET		4	/* first failing reg           */
#define XTM_CMR0_OFFSET		8	/* comparison mask reg 0       */
#define XTM_CMR1_OFFSET		12	/* comparison mask reg 1       */
#define XTM_BDIR_OFFSET		16	/* break delay init reg        */
#define XTM_SEMSR_OFFSET	20	/* sem status reg, read        */
#define XTM_SEMSSR_OFFSET	24	/* sem sticky status reg       */
#define XTM_SEMIMR_OFFSET	28	/* sem interrupt mask reg      */
#define XTM_WR_OFFSET		32	/* watchdog reg                */
#define XTM_RFSR_OFFSET		36	/* reset failing state reg     */
#define XTM_CSCR_OFFSET		40	/* comparator status clear reg */
#define XTM_CFIR_OFFSET		44	/* comparator fault inject reg */

/* Control Register bit positions and masks */

#define XTM_CR_MAGIC1     	0	/* magic byte 1 */
#define XTM_CR_MAGIC2       	8  	/* magic bite 2 */
#define XTM_CR_RIR          	16	/* recover is reset */
#define XTM_CR_BFR          	17	/* block fatal reset */
#define XTM_CR_BB           	18	/* block break */
#define XTM_CR_BFO          	19	/* block fatal output */

#define XTM_CR_MAGIC1_MASK	0x000ff	/* magic byte 1 mask */
#define XTM_CR_MAGIC2_MASK	0x0ff00	/* magic byte 2 mask */

/* First Failing Register bit positions */

#define XTM_FFR_LM12		0	/* lockstep mismatch 1-2 */
#define XTM_FFR_LM13		1	/* lockstep mismatch 1-2 */
#define XTM_FFR_LM23		2	/* lockstep mismatch 1-2 */
#define XTM_FFR_REC		3	/* recovery */
#define XTM_FFR_FAT12		16	/* fatal error 1-2 */
#define XTM_FFR_FAT13		17	/* fatal error 1-3 */
#define XTM_FFR_FAT23		18	/* fatal error 2-3 */
#define XTM_FFR_FATV		19	/* fatal voter error */
#define XTM_FFR_FATUE		20	/* fatal uncorr error */
#define XTM_FFR_WE		21	/* watchdog expired */

#define XTM_FFR_LM12_LM13_MASK  0x000003
#define XTM_FFR_LM12_LM23_MASK  0x000005
#define XTM_FFR_LM13_LM23_MASK  0x000006
#define XTM_FFR_REC_MASK        0x000008
#define XTM_FFR_REC_12_13_MASK  0x00000b
#define XTM_FFR_REC_12_23_MASK  0x00000d
#define XTM_FFR_REC_13_23_MASK  0x00000e

/* SEM Status Register, SEM Sticky Status Register, and SEM Interrupt Mask
 * Register bit positions
 */
#define XTM_SEM_HBWE		0	/* heartbeat watchdog expired */
#define XTM_SEM_HB		1	/* heartbeat input */
#define XTM_SEM_INI		2	/* initialization input */
#define XTM_SEM_OBS		3	/* observation input */
#define XTM_SEM_CORR		4	/* correction input */
#define XTM_SEM_CLA		5	/* classification input */
#define XTM_SEM_INJ		6	/* injection input */
#define XTM_SEM_UNC		7	/* uncorrectable input */
#define XTM_SEM_ESS		8	/* essential input */
#define XTM_SEM_DO		9	/* detect only input */
#define XTM_SEM_DS		10	/* diagnostic scan input */

/* Comparator Fault Inject Register bit positions */
#define XTM_CFIR_IE12		0	/* inject error 1-2 */
#define XTM_CFIR_IE23		1	/* inject error 1-2 */
#define XTM_CFIR_IE13		2	/* inject error 1-2 */
#define XTM_CFIR_IVE		3	/* inject voter error */


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

#define XTMR_Manager_In32  Xil_In32
#define XTMR_Manager_Out32 Xil_Out32


/****************************************************************************/
/**
*
* Write a value to a TMRManager register. A 32 bit write is performed.
*
* @param	BaseAddress is the base address of the TMRManager device.
* @param	RegOffset is the register offset from the base to write to.
* @param	Data is the data written to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XTMR_Manager_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 Data)
*
****************************************************************************/
#define XTMR_Manager_WriteReg(BaseAddress, RegOffset, Data) \
	XTMR_Manager_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/****************************************************************************/
/**
*
* Read a value from a TMRManager register. A 32 bit read is performed.
*
* @param	BaseAddress is the base address of the TMRManager device.
* @param	RegOffset is the register offset from the base to read from.
*
* @return	Data read from the register.
*
* @note		C-style signature:
*		u32 XTMR_Manager_ReadReg(u32 BaseAddress, u32 RegOffset)
*
****************************************************************************/
#define XTMR_Manager_ReadReg(BaseAddress, RegOffset) \
	XTMR_Manager_In32((BaseAddress) + (RegOffset))


/****************************************************************************/
/**
*
* Set the contents of the control register. Use the XTM_CR_* constants defined
* above to create the bit-mask to be written to the register.
*
* @param	BaseAddress is the base address of the device
* @param	Mask is the 32-bit value to write to the control register
*
* @return	None.
*
* @note		C-style Signature:
*		void XTMR_Manager_SetControlReg(u32 BaseAddress, u32 Mask);
*
*****************************************************************************/
#define XTMR_Manager_SetControlReg(BaseAddress, Mask) \
	XTMR_Manager_WriteReg((BaseAddress), XTM_CONTROL_OFFSET, (Mask))


/****************************************************************************/
/**
*
* Get the contents of the first failing register. Use the XTM_FFR_* constants
* defined above to interpret the bit-mask returned.
*
* @param	BaseAddress is the base address of the device
*
* @return	A 32-bit value representing the contents of the status register.
*
* @note		C-style Signature:
*		u32 XTMR_Manager_GetFirstFailingReg(u32 BaseAddress);
*
*****************************************************************************/
#define XTMR_Manager_GetFirstFailingReg(BaseAddress) \
		XTMR_Manager_ReadReg((BaseAddress), XTM_FFR_OFFSET)


/****************************************************************************/
/**
*
* Clear the contents of the first failing register.
*
* @param	BaseAddress is the base address of the device
*
* @return	A 32-bit value representing the contents of the status register.
*
* @note		C-style Signature:
*		u32 XTMR_Manager_ClearFirstFailingReg(u32 BaseAddress);
*
*****************************************************************************/
#define XTMR_Manager_ClearFirstFailingReg(BaseAddress)	\
	XTMR_Manager_WriteReg((BaseAddress), XTM_FFR_OFFSET, 0)


/****************************************************************************/
/**
*
* Check to see if a lockstep mismatch has occurred.
*
* @param	BaseAddress is the base address of the device
*
* @return	TRUE if a lockstep mismatch has occurred, FALSE otherwise.
*
* @note		C-style Signature:
*		int XTMR_Manager_IsLockstepMismatch(u32 BaseAddress);
*
*****************************************************************************/
#define XTMR_Manager_IsLockstepMismatch(BaseAddress) \
  ((XTMR_Manager_GetFirstFailingReg((BaseAddress)) & \
    ((1 << XTM_FFR_LM12) | (1 << XTM_FFR_LM13)| (1 << XTM_FFR_LM23)) != 0)


/****************************************************************************/
/**
*
* Enable the SEM interrupt. We cannot read the SEM Interrupt Mask Register,
* so we just write the enabled interrupt bits and clear all others. Since the
* register only defines the interrupt mask, this works without side effects.
*
* @param	BaseAddress is the base address of the device
*
* @return	None.
*
* @note		C-style Signature:
* 		void XTMR_Manager_EnableIntr(u32 BaseAddress);
*
*****************************************************************************/
#define XTMR_Manager_EnableIntr(BaseAddress, Mask)	\
	  XTMR_Manager_WriteReg((BaseAddress), XTM_SEMIMR_OFFSET, Mask)


/****************************************************************************/
/**
*
* Disable the SEM interrupt. We cannot read the SEM Interrupt Mask Register,
* so we just clear all bits. Since the register only defines the interrupt
* mask, this works without side effects.
*
* @param	BaseAddress is the base address of the device
*
* @return	None.
*
* @note		C-style Signature:
* 		void XTMR_Manager_DisableIntr(u32 BaseAddress);
*
*****************************************************************************/
#define XTMR_Manager_DisableIntr(BaseAddress) \
	  XTMR_Manager_WriteReg((BaseAddress), XTM_SEMIMR_OFFSET, 0)

/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */

/** @} */
