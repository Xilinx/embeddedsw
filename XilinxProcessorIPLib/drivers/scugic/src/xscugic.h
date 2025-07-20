/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xscugic.h
* @addtogroup scugic_api SCUGIC APIs
* @{
* @details
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a drg  01/19/00 First release
* 1.01a sdm  11/09/11 The XScuGic and XScuGic_Config structures have changed.
*		      The HandlerTable (of type XScuGic_VectorTableEntry) is
*		      moved to XScuGic_Config structure from XScuGic structure.
*
*		      The "Config" entry in XScuGic structure is made as
*		      pointer for better efficiency.
*
*		      A new file named as xscugic_hw.c is now added. It is
*		      to implement low level driver routines without using
*		      any xscugic instance pointer. They are useful when the
*		      user wants to use xscugic through device id or
*		      base address. The driver routines provided are explained
*		      below.
*		      XScuGic_DeviceInitialize that takes device id as
*		      argument and initializes the device (without calling
*		      XScuGic_CfgInitialize).
*		      XScuGic_DeviceInterruptHandler that takes device id
*		      as argument and calls appropriate handlers from the
*		      HandlerTable.
*		      XScuGic_RegisterHandler that registers a new handler
*		      by taking xscugic hardware base address as argument.
*		      LookupConfigByBaseAddress is used to return the
*		      corresponding config structure from XScuGic_ConfigTable
*		      based on the scugic base address passed.
* 1.02a sdm  12/20/11 Removed AckBeforeService from the XScuGic_Config
*		      structure.
* 1.03a srt  02/27/13 Moved Offset calculation macros from *.c and *_hw.c to
*		      *_hw.h
*		      Added APIs
*			- XScuGic_SetPriTrigTypeByDistAddr()
*			- XScuGic_GetPriTrigTypeByDistAddr()
*		      (CR 702687)
*			Added support to direct interrupts to the appropriate CPU. Earlier
*			  interrupts were directed to CPU1 (hard coded). Now depending
*			  upon the CPU selected by the user (xparameters.h), interrupts
*			  will be directed to the relevant CPU. This fixes CR 699688.
* 1.04a hk   05/04/13 Assigned EffectiveAddr to CpuBaseAddress in
*			  XScuGic_CfgInitialize. Fix for CR#704400 to remove warnings.
*			  Moved functions XScuGic_SetPriTrigTypeByDistAddr and
*             XScuGic_GetPriTrigTypeByDistAddr to xscugic_hw.c.
*			  This is fix for CR#705621.
* 1.05a hk   06/26/13 Modified tcl to export external interrupts correctly to
*                     xparameters.h. Fix for CR's 690505, 708928 & 719359.
* 2.0   adk  12/10/13 Updated as per the New Tcl API's
* 2.1   adk  25/04/14 Fixed the CR:789373 changes are made in the driver tcl file.
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.2   asa  02/29/16 Modified DistributorInit function for Zynq AMP case. The
*			  distributor is left uninitialized for Zynq AMP. It is assumed
*             that the distributor will be initialized by Linux master. However
*             for CortexR5 case, the earlier code is left unchanged where the
*             the interrupt processor target registers in the distributor is
*             initialized with the corresponding CPU ID on which the application
*             built over the scugic driver runs.
*             These changes fix CR#937243.
*
* 3.4   asa  04/07/16 Created a new static function DoDistributorInit to simplify
*            the flow and avoid code duplication. Changes are made for
*            USE_AMP use case for R5. In a scenario (in R5 split mode) when
*            one R5 is operating with A53 in open amp config and other
*            R5 running baremetal app, the existing code
*            had the potential to stop the whole AMP solution to work (if
*            for some reason the R5 running the baremetal app tasked to
*            initialize the Distributor hangs or crashes before initializing).
*            Changes are made so that the R5 under AMP first checks if
*            the distributor is enabled or not and if not, it does the
*            standard Distributor initialization.
*            This fixes the CR#952962.
* 3.6   ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       kvn  02/17/17 Add support for changing GIC CPU master at run time.
*       kvn  02/28/17 Make the CpuId as static variable and Added new
*                     XScugiC_GetCpuId to access CpuId.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 3.7   ms   04/11/17 Modified tcl file to add suffix U for all macro
*                     definitions of scugic in xparameters.h
* 3.8   mus  07/05/17 Updated scugic.tcl to add support for interrupts connected
*                     through util_reduced_vector IP(OR gate)
*       mus  07/05/17 Updated xdefine_zynq_canonical_xpars proc to initialize
*                     the HandlerTable in XScuGic_ConfigTable to 0, it removes
*                     the compilation warning in xscugic_g.c. Fix for CR#978736.
*       mus  07/25/17 Updated xdefine_gic_params proc to export correct canonical
*                     definitions for pl to ps interrupts.Fix for CR#980534
* 3.9   mus  02/21/18 Added new API's XScuGic_UnmapAllInterruptsFromCpu and
*                     XScuGic_InterruptUnmapFromCpu, These API's can be used
*                     by applications to unmap specific/all interrupts from
*                     target CPU.
* 3.10  aru  08/23/18 Resolved MISRA-C:2012 compliance mandatory violations
* 4.0   mus  11/22/18 Fixed bugs in software interrupt generation through
*                      XScuGic_SoftwareIntr API
* 4.1   asa  03/30/19 Made changes not to direct each interrupt to all
*                     available CPUs by default. This was breaking AMP
*                     behavior. Instead every time an interrupt enable
*                     request is received, the interrupt was mapped to
*                     the respective CPU. There were several other changes
*                     made to implement this. This set of changes was to
*                     fix CR-1024716.
* 4.1   mus  06/19/19 Added API's XScuGic_MarkCoreAsleep and
*                     XScuGic_MarkCoreAwake to mark processor core as
*                     asleep or awake. Fix for CR#1027220.
* 4.5   asa  03/07/21 Included a header file xil_spinlock.h to ensure that
*                     GIC driver can use newly introduced spinlock
*                     functionality.
* 4.6	sk   08/05/21 Fix scugic misrac violations.
* 4.7   dp   11/22/21 Added new API XScuGic_IsInitialized() to check and return
*                     the GIC initialization status.
* 5.0   mus  22/02/22 Add support for VERSAL NET
* 	adk  04/18/22 Replace infinite while loops in the examples with
* 		      Xil_WaitForEventSet() API.
*       dp   04/25/22 Correct Trigger index calculation in macro
*                     XScuGic_Get_Rdist_Int_Trigger_Index
* 5.0   dp   11/07/22 Add macros for accessing the GIC Binary Point and
*                     Running Priority registers of Cortex-R52.
* 5.1   mus  02/13/23 Updated XScuGic_CfgInitialize, XScuGic_Enable and
*                     XScuGic_Disable to support interrupts on each core
*                     of all CortexA78/CortexR52 clusters in VERSAL NET SoC.
*                     While at it, modified interrupt routing logic to make
*                     use of CPU affinity register instead of XPAR_CPU_ID macro.
*                     Also, XScuGic_CfgInitialize has been updated to find
*                     redistributor base address of core on which API is
*                     executed, redistributor address will be stored in newly
*                     added member of XScuGic data structure "RedistBaseAddr".
*                     It fixes CR#1150432.
* 5.2   ml   03/02/23 Add description to fix Doxygen warnings.
* 5.2   adk  04/14/23 Added support for system device-tree flow.
* 5.5   ml   01/08/25 Update datatype of distributor and cpu base address in
*                     scugic config structure.
* 5.6   ml   07/21/25 Fix GCC warnings
* </pre>
*
******************************************************************************/

#ifndef XSCUGIC_H /**< prevent circular inclusions */
#define XSCUGIC_H /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xil_io.h"
#include "xscugic_hw.h"
#include "xil_exception.h"
#include "xil_spinlock.h"

/************************** Constant Definitions *****************************/

/**
 * @name EFUSE status Register information
 * EFUSE Status Register
 * @{
 */
#define EFUSE_STATUS_OFFSET   0x10
#define EFUSE_STATUS_CPU_MASK 0x80

#if !defined (ARMR5) && !defined (__aarch64__) && !defined (ARMA53_32)
#define ARMA9 /**< ARMA9 macro to identify cortexA9 */
#endif

/**
 * @name GICD_CTLR Register information
 * GICD_CTLR Status Register
 * @{
 */
#define XSCUGIC500_DCTLR_ARE_NS_ENABLE  0x20
#define XSCUGIC500_DCTLR_ARE_S_ENABLE  0x10

#if defined (VERSAL_NET)
#define XSCUGIC_CLUSTERID_MASK 0xF0U
#define XSCUGIC_COREID_MASK 0xFU
#define XSCUGIC_CLUSTERID_SHIFT 4U
#define XSCUGIC_COREID_SHIFT 0U

#define XSCUGIC_SGI1R_AFFINITY1_SHIFT 16U
#define XSCUGIC_SGI1R_AFFINITY2_SHIFT 32U

#define XSCUGIC_IROUTER_AFFINITY1_SHIFT 8U
#define XSCUGIC_IROUTER_AFFINITY2_SHIFT 16U

#define XSCUGIC_IROUTER_IRM_MASK 0x80000000U
#endif

/**************************** Type Definitions *******************************/

/* The following data type defines each entry in an interrupt vector table.
 * The callback reference is the base address of the interrupting device
 * for the low level driver and an instance pointer for the high level driver.
 */
typedef struct
{
       Xil_InterruptHandler Handler; /**< Interrupt Handler */
       void *CallBackRef; /**< CallBackRef is the callback reference passed in
                               by the upper layer when setting the Interrupt
                               handler for specific interrupt ID, and it will
                               passed back to Interrupt handler when it is
                               invoked. */
} XScuGic_VectorTableEntry;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
#ifndef SDT
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR CpuBaseAddress;	/**< CPU Interface Register base address */
	UINTPTR DistBaseAddress;	/**< Distributor Register base address */
#else
	char *Name;		/**< Compatible string */
	UINTPTR DistBaseAddress;	/**< Distributor Register base address */
	UINTPTR CpuBaseAddress;	/**< CPU Interface Register base address */
#endif
	XScuGic_VectorTableEntry HandlerTable[XSCUGIC_MAX_NUM_INTR_INPUTS];/**<
				 Vector table of interrupt handlers */
} XScuGic_Config;

/**
 * The XScuGic driver instance data. The user is required to allocate a
 * variable of this type for every intc device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct
{
	XScuGic_Config *Config;  /**< Configuration table entry */
#if defined (GICv3)
	UINTPTR RedistBaseAddr;
#endif
	u32 IsReady;		 /**< Device is initialized and ready */
	u32 UnhandledInterrupts; /**< Intc Statistics */
} XScuGic;

/************************** Variable Definitions *****************************/

extern XScuGic_Config XScuGic_ConfigTable[];	/**< Config table */

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Writes the given CPU Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be written.
* @param    Data 32-bit value to write to the register.
*
* @return   None.
*
* @note  C-style signature:
*        void XScuGic_CPUWriteReg(XScuGic *InstancePtr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XScuGic_CPUWriteReg(InstancePtr, RegOffset, Data) \
(XScuGic_WriteReg(((InstancePtr)->Config->CpuBaseAddress), (RegOffset), \
					((u32)(Data))))

/****************************************************************************/
/**
*
* Reads the given CPU Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Rregister offset to be read.
*
* @return   32-bit value of the register
*
* @note  C-style signature:
*        u32 XScuGic_CPUReadReg(XScuGic *InstancePtr, u32 RegOffset)
*
*****************************************************************************/
#define XScuGic_CPUReadReg(InstancePtr, RegOffset) \
	(XScuGic_ReadReg(((InstancePtr)->Config->CpuBaseAddress), (RegOffset)))

/****************************************************************************/
/**
*
* Writes the given Distributor Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be written.
* @param    Data 32-bit value to write to the register.
*
* @return   None.
*
* @note  C-style signature:
*        void XScuGic_DistWriteReg(XScuGic *InstancePtr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XScuGic_DistWriteReg(InstancePtr, RegOffset, Data) \
(XScuGic_WriteReg(((InstancePtr)->Config->DistBaseAddress), (RegOffset), \
					((u32)(Data))))

/****************************************************************************/
/**
*
* Reads the given Distributor Interface register
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be read.
*
* @return   The 32-bit value of the register.
*
* @note  C-style signature:
*        u32 XScuGic_DistReadReg(XScuGic *InstancePtr, u32 RegOffset)
*
*****************************************************************************/
#define XScuGic_DistReadReg(InstancePtr, RegOffset) \
(XScuGic_ReadReg(((InstancePtr)->Config->DistBaseAddress), (RegOffset)))

#if defined (GICv3)
/****************************************************************************/
/**
*
* Writes the given ReDistributor Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be written.
* @param    Data 32-bit value to write to the register.
*
* @return   None.
*
* @note  C-style signature:
*        void XScuGic_DistWriteReg(XScuGic *InstancePtr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XScuGic_ReDistWriteReg(InstancePtr, RegOffset, Data) \
(XScuGic_WriteReg(InstancePtr->RedistBaseAddr, RegOffset, (u32)Data))

/****************************************************************************/
/**
*
* Reads the given ReDistributor Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be read.
*
* @return   32-bit value of the register.
*
* @note   C-style signature:
*         u32 XScuGic_DistReadReg(XScuGic *InstancePtr, u32 RegOffset)
*
*****************************************************************************/
#define XScuGic_ReDistReadReg(InstancePtr, RegOffset) \
(XScuGic_ReadReg(InstancePtr->RedistBaseAddr, RegOffset))

/****************************************************************************/
/**
*
* Writes the given ReDistributor SGI PPI Interface register.
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be written.
* @param    Data 32-bit value to write to the register.
*
* @return   None.
*
* @note  C-style signature:
*        void XScuGic_DistWriteReg(XScuGic *InstancePtr, u32 RegOffset, u32 Data)
*
*****************************************************************************/
#define XScuGic_ReDistSGIPPIWriteReg(InstancePtr, RegOffset, Data) \
(XScuGic_WriteReg(InstancePtr->RedistBaseAddr + \
				   XSCUGIC_RDIST_SGI_PPI_OFFSET, (RegOffset), ((u32)(Data))))

/****************************************************************************/
/**
*
* Reads the given ReDistributor SGI PPI Interface register
*
* @param    InstancePtr Pointer to the instance to be worked on.
* @param    RegOffset Register offset to be read.
*
* @return   The 32-bit value of the register
*
* @note  C-style signature:
*        u32 XScuGic_DistReadReg(XScuGic *InstancePtr, u32 RegOffset)
*
*****************************************************************************/
#define XScuGic_ReDistSGIPPIReadReg(InstancePtr, RegOffset) \
(XScuGic_ReadReg(InstancePtr->RedistBaseAddr + XSCUGIC_RDIST_SGI_PPI_OFFSET, \
		 RegOffset))

#if defined(ARMR52)
#define XREG_ICC_SRE_EL1	"p15, 0, %0,  c12,  c12, 5"
#define XREG_ICC_IGRPEN0_EL1	"p15, 0, %0,  c12,  c12, 6"
#define XREG_ICC_IGRPEN1_EL1	"p15, 0, %0,  c12,  c12, 7"
#define XREG_ICC_SGI0R_EL1	"p15, 2, %0,  %1,  c12"
#define XREG_ICC_SGI1R_EL1	"p15, 0, %0,  %1,  c12"
#define XREG_ICC_PMR_EL1	"p15, 0, %0,  c4,  c6, 0"
#define XREG_ICC_IAR0_EL1	"p15, 0, %0,  c12,  c8, 0"
#define XREG_ICC_IAR1_EL1	"p15, 0, %0,  c12,  c12, 0"
#define XREG_ICC_EOIR0_EL1	"p15, 0, %0,  c12,  c8, 1"
#define XREG_ICC_EOIR1_EL1	"p15, 0, %0,  c12,  c12, 1"
#define XREG_IMP_CBAR		"p15, 1, %0, c15, c3, 0"
#define XREG_ICC_BPR0_EL1	"p15, 0, %0, c12, c8, 3"
#define XREG_ICC_BPR1_EL1	"p15, 0, %0, c12, c12, 3"
#define XREG_ICC_RPR_EL1	"p15, 0, %0, c12, c11, 3"
#else
#define XREG_ICC_SRE_EL1	"S3_0_C12_C12_5"
#define XREG_ICC_SRE_EL3	"S3_6_C12_C12_5"
#define XREG_ICC_IGRPEN0_EL1	"S3_0_C12_C12_6"
#define XREG_ICC_IGRPEN1_EL1	"S3_0_C12_C12_7"
#define XREG_ICC_IGRPEN1_EL3	"S3_6_C12_C12_7"
#define XREG_ICC_SGI0R_EL1	"S3_0_C12_C11_7"
#define XREG_ICC_SGI1R_EL1	"S3_0_C12_C11_5"
#define XREG_ICC_PMR_EL1	"S3_0_C4_C6_0"
#define XREG_ICC_IAR0_EL1	"S3_0_C12_C8_0"
#define XREG_ICC_IAR1_EL1	"S3_0_C12_C12_0"
#define XREG_ICC_EOIR0_EL1	"S3_0_C12_C8_1"
#define XREG_ICC_EOIR1_EL1	"S3_0_C12_C12_1"
#endif
/****************************************************************************/
/**
* Enables system register interface for GIC CPU Interface.
*
* @param	value Value to be written.
*
* @return	None.
*
*
*****************************************************************************/
#if defined (__aarch64__)
#define XScuGic_Enable_SystemReg_CPU_Interface_EL3() mtcpnotoken(XREG_ICC_SRE_EL3, (u64)0xF);
#define XScuGic_Enable_SystemReg_CPU_Interface_EL1() mtcpnotoken(XREG_ICC_SRE_EL1, (u64)0xF);
#elif defined (ARMR52)
#define XScuGic_Enable_SystemReg_CPU_Interface_EL1() mtcp(XREG_ICC_SRE_EL1, 0xF);
#endif
/****************************************************************************/
/**
* Enable Grou0 interrupts.
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#if defined(ARMR52)
#define XScuGic_Enable_Group0_Interrupts() mtcp(XREG_ICC_IGRPEN0_EL1,0x1);
#else
#define XScuGic_Enable_Group0_Interrupts() mtcpnotoken(XREG_ICC_IGRPEN0_EL1,(u64)0x1);
#endif
/****************************************************************************/
/**
* This function enables Group1 interrupts
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#if defined (ARMR52)

#define XScuGic_Enable_Group1_Interrupts() \
		mtcp (XREG_ICC_IGRPEN1_EL1, 0x1 | mfcp(XREG_ICC_IGRPEN1_EL1) );
#elif EL1_NONSECURE
#define XScuGic_Enable_Group1_Interrupts() \
                mtcpnotoken(XREG_ICC_IGRPEN1_EL1, 0x1 | mfcpnotoken(XREG_ICC_IGRPEN1_EL1) );
#else
#define XScuGic_Enable_Group1_Interrupts() \
		mtcpnotoken(XREG_ICC_IGRPEN1_EL3, 0x1 | mfcpnotoken(XREG_ICC_IGRPEN1_EL3) );
#endif
/****************************************************************************/
/**
* Writes to ICC_SGI0R_EL1.
*
* @param	value Value to be written.
*
* @return	None.
*
*
*****************************************************************************/
#if defined(ARMR52)
#define XScuGic_WriteICC_SGI0R_EL1(val) mtcp2(XREG_ICC_SGI0R_EL1,val)
#else
#define XScuGic_WriteICC_SGI0R_EL1(val) mtcpnotoken(XREG_ICC_SGI0R_EL1,val)
#endif

/****************************************************************************/
/**
* Writes to ICC_SGI1R_EL1.
*
* @param	value Value to be written.
*
* @return	None.
*
*
*****************************************************************************/
#if defined(ARMR52)
#define XScuGic_WriteICC_SGI1R_EL1(val) mtcp2(XREG_ICC_SGI1R_EL1,val)
#else
#define XScuGic_WriteICC_SGI1R_EL1(val) mtcpnotoken(XREG_ICC_SGI1R_EL1,val)
#endif

/****************************************************************************/
/**
* Reads ICC_SGI1R_EL1 register.
*
* @param	None
*
* @return	Value of ICC_SGI1R_EL1 register
*
*
*****************************************************************************/
#if defined (ARMR52)
#define XScuGic_ReadICC_SGI1R_EL1() mfcp(XREG_ICC_SGI1R_EL1)
#else
#define XScuGic_ReadICC_SGI1R_EL1() mfcpnotoken(XREG_ICC_SGI1R_EL1)
#endif
/****************************************************************************/
/**
* Sets interrupt priority filter.
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#if defined (ARMR52)
#define XScuGic_set_priority_filter(val)  mtcp(XREG_ICC_PMR_EL1, val)
#else
#define XScuGic_set_priority_filter(val)  mtcpnotoken(XREG_ICC_PMR_EL1, (u64)val)
#endif
/****************************************************************************/
/**
* Returns interrupt ID of highest priority pending interrupt.
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#if defined(ARMR52)
#define XScuGic_get_IntID()  mfcp(XREG_ICC_IAR1_EL1)
#elif EL3
#define XScuGic_get_IntID()  mfcpnotoken(XREG_ICC_IAR0_EL1)
#else
#define XScuGic_get_IntID()  mfcpnotoken(XREG_ICC_IAR1_EL1)
#endif
/****************************************************************************/
/**
* Acknowledges the interrupt.
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#if defined(ARMR52)
#define XScuGic_ack_Int(val)   mtcp(XREG_ICC_EOIR1_EL1,val)
#elif EL3
#define XScuGic_ack_Int(val)   mtcpnotoken(XREG_ICC_EOIR0_EL1,(u64)val)
#else
#define XScuGic_ack_Int(val)   mtcpnotoken(XREG_ICC_EOIR1_EL1,val)
#endif
/****************************************************************************/
/**
* This macro returns bit position for the specific interrupt's trigger type
* configuration within GICR_ICFGR0/GICR_ICFGR1 register
*
* @param	None.
*
* @return	None.
*
*
*****************************************************************************/
#define XScuGic_Get_Rdist_Int_Trigger_Index(IntrId)  ((Int_Id%16) * 2U)
#endif
/************************** Function Prototypes ******************************/

/*
 * Required functions in xscugic.c
 */

s32  XScuGic_Connect(XScuGic *InstancePtr, u32 Int_Id,
			Xil_InterruptHandler Handler, void *CallBackRef);
void XScuGic_Disconnect(XScuGic *InstancePtr, u32 Int_Id);

void XScuGic_Enable(XScuGic *InstancePtr, u32 Int_Id);
void XScuGic_Disable(XScuGic *InstancePtr, u32 Int_Id);

s32  XScuGic_CfgInitialize(XScuGic *InstancePtr, XScuGic_Config *ConfigPtr,
							u32 EffectiveAddr);

s32  XScuGic_SoftwareIntr(XScuGic *InstancePtr, u32 Int_Id, u32 Cpu_Identifier);

void XScuGic_GetPriorityTriggerType(XScuGic *InstancePtr, u32 Int_Id,
					u8 *Priority, u8 *Trigger);
void XScuGic_SetPriorityTriggerType(XScuGic *InstancePtr, u32 Int_Id,
					u8 Priority, u8 Trigger);
void XScuGic_InterruptMaptoCpu(XScuGic *InstancePtr, u8 Cpu_Identifier, u32 Int_Id);
void XScuGic_InterruptUnmapFromCpu(XScuGic *InstancePtr, u8 Cpu_Identifier, u32 Int_Id);
void XScuGic_UnmapAllInterruptsFromCpu(XScuGic *InstancePtr, u8 Cpu_Identifier);
void XScuGic_Stop(XScuGic *InstancePtr);
void XScuGic_SetCpuID(u32 CpuCoreId);
u32 XScuGic_GetCpuID(void);
#ifndef SDT
u8 XScuGic_IsInitialized(u32 DeviceId);
#else
u8 XScuGic_IsInitialized(u32 BaseAddress);
#endif
#ifndef SDT
/*
 * Lookup configuration by using DeviceId
 */
XScuGic_Config *XScuGic_LookupConfig(u16 DeviceId);
/*
 * Lookup configuration by using BaseAddress
 */
XScuGic_Config *XScuGic_LookupConfigBaseAddr(UINTPTR BaseAddress);
#else
XScuGic_Config *XScuGic_LookupConfig(UINTPTR BaseAddr);
#endif

/*
 * Interrupt functions in xscugic_intr.c
 */
void XScuGic_InterruptHandler(XScuGic *InstancePtr);

/*
 * Self-test functions in xscugic_selftest.c
 */
s32  XScuGic_SelfTest(XScuGic *InstancePtr);

#if defined (GICv3)
void XScuGic_MarkCoreAsleep(XScuGic *InstancePtr);
void XScuGic_MarkCoreAwake(XScuGic *InstancePtr);
#endif
#ifdef __cplusplus
}
#endif

#endif
/* end of protection macro */
/** @} */
