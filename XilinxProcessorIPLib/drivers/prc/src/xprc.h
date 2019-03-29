/******************************************************************************
*
* Copyright (C) 2016-2019 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be
* used in advertising or otherwise to promote the sale, use or other dealings
* in this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xprc.h
* @addtogroup prc_v1_2
* @{
* @details
*
* Xilinx PRC driver component provides management function for self-controlling
* partially reconfigurable designs.
*
* It is intended for enclosed systems where all of the Reconfigurable Modules
* are known to the controller. The optional AXI4-Lite register interface allows
* the core to be reconfigured at run time, so it can also be used in systems
* where the Reconfigurable Modules(RM) can change in the field.
*
* The PRC supports the following features:
* - 32 Virtual Sockets.
* - 128 Reconfigurable modules per Virtual Socket.
* - Optional hardware and software shutdown of Reconfigurable Modules
*    (configurable per Reconfigurable Module).
* - Optional software start-up of Reconfigurable Modules (configurable per
*    Reconfigurable Module).
* - Optional reset of Reconfigurable Modules after loading (configurable per
*    Reconfigurable Module).
* - Virtual Socket Managers can be shutdown and restarted by the user to allow
*    external controllers to partially reconfigure the device.
* - User control of Virtual Socket Manager output signals is supported in the
*    shutdown state.
*
* The Partial Reconfiguration Controller consists of one or more Virtual Socket
* Managers(VSMs) which connect to a single fetch path.
* Virtual Socket Managers operate in parallel waiting for triggers to occur.
* When a trigger occurs, the appropriate VSM maps the trigger to an RM and
* manages the reconfiguration of that RM. VSM's queue for access to the
* fetch path.
*
* VSMs exist in two States
*	- Active State
*	- Shutdown State
*
* <b>Initialization & Configuration</b>
*
* The XPrc_Config structure is used by the driver to configure itself.
* This configuration structure is created based on HW build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
*    - XPrc_LookupConfig(DeviceId) - Use the device identifier to find the
*      static configuration structure defined in xprc_g.c. This is setup by
*      the tools. For some operating systems the config structure will be
*      initialized by the software and this call is not needed.
*
*    - XPrc_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) is used for
*      initialization. The user needs to first call the XPrc_LookupConfig()
*      API which returns the Configuration structure pointer which is passed
*      as a parameter to the XPrc_CfgInitialize() API.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XPrc driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <b> Debug prints </b>
*
* XPrc driver is having debug prints.
*	- To get the debug print statements of the driver, please define
*	  XPRC_DEBUG as shown below.
*		"#define XPRC_DEBUG"
*	- To get the debug print statements of the core, please define
*	  XPRC_DEBUG_CORE as shown below.
*		"#define XPRC_DEBUG_CORE"
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ---- -----  ------------  ----------------------------------------------
* 1.0   ms     07/18/2016   First release
*       ms     03/17/17     Added readme.txt file in examples folder for
*                           doxygen generation.
*       ms     04/05/2017   Modified comment lines notation in functions
*                           of prc examples to avoid unnecessary description
*                           which was displayed while generating doxygen.
* 1.1   ms     04/18/17     Modified tcl file to add suffix U for all macros
*                           definitions of prc in xparameters.h
*       ms     08/01/17     Added a new parameter "Cp_Compression" to
*                           XPrc_Config structure and also inline function
*                           related to this paramter.
*                           Modified xprc.c, prc.tcl, xprc_hw.h to add a
*                           new parameter "Cp_Compression" and status error
*                           flags. Added the Updated api.tcl to data folder.
* 1.2  Nava   29/03/19      Updated the tcl logic to generated the
*                           XPrc_ConfigTable properly.
* </pre>
*
******************************************************************************/

#ifndef XPRC_H_ /* Prevent circular inclusions */
#define XPRC_H_ /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xprc_hw.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/** @name Please define XPRC_DEBUG macro to get debug prints of the driver and
 *  define XPRC_DEBUG_CORE to get debug prints of the core.
 * @{
 */
#define XPRC_DEBUG_GENERAL	(0x00000001)

#if defined (XPRC_DEBUG)
#define xprc_dbg_current_types (XPRC_DEBUG_GENERAL)
#else
#define xprc_dbg_current_types 0
#endif
#if defined (XPRC_DEBUG_CORE)
#define xprc_dbgcore_current_types (XPRC_DEBUG_GENERAL)
#else
#define xprc_dbgcore_current_types 0
#endif

#ifdef STDOUT_BASEADDRESS
#define xprc_printf(type,...) \
	if (((type) & xprc_dbg_current_types)) { xil_printf (__VA_ARGS__);}
#define xprc_core_printf(type,...) \
	if (((type) & xprc_dbgcore_current_types)) { xil_printf (__VA_ARGS__);}
#else
#define xprc_printf(type, ...)
#define xprc_core_printf(type,...)
#endif
/*@}*/

/** @name Virtual Socket Manager Registers
 * @{
 */
#define XPRC_MAX_NUMBER_OF_VSMS		(32)	/**< Maximum Number Of Vsms */
#define XPRC_VSM_NUM_GENERAL_REGISTERS	(2)	/**< Number Of General
						  *  registers */
#define XPRC_VSM_REGISTERS_PER_TRIGGER	(1)	/**< Registers per
						  *  Trigger */
#define XPRC_VSM_REGISTERS_PER_RM	(2)	/**< Registers per Rm */
#define XPRC_VSM_REGISTERS_PER_BS	(3)	/**< Registers per Bs */
/*@}*/

/** @name Control Registers
 * @{
 */
#define XPRC_CR_VS_FULL			(1)	/**< Virtual Socket In Full
						  *  State */
#define XPRC_CR_VS_EMPTY		(0)	/**< Virtual Socket In Empty
						  *  State */
#define XPRC_CR_DEFAULT_BYTE		(0)	/**< Byte field Information
						  *  for Control register
						  *  Commands */
#define XPRC_CR_DEFAULT_HALFWORD	(0)	/**< Halfword field information
						  *  for Control register
						  *  Commands */
/*@}*/

/** @name Status Registers
 * @{
 */
#define XPRC_SR_SHUTDOWN_ON		(1)	/**< Vsm In Shutdown State */
#define XPRC_SR_SHUTDOWN_OFF		(0)	/**< Vsm not In Shutdown
						  *  State */
/*@}*/

/** @name Software Trigger Registers
 * @{
 */
#define XPRC_SW_TRIGGER_PENDING		(1)	/**< Software Trigger is
						  *  Pending */
#define XPRC_NO_SW_TRIGGER_PENDING	(0)	/**< Software Trigger is not
						  *  Pending */
/*@}*/

/** @name RM Control Registers
 * @{
 */
#define XPRC_RM_CR_MAX_RESETDURATION	(256)	/**< Maximum Number of
						  *  Reset duration */
/*@}*/

/** @name Register Table Row
 * @{
 */
#define XPRC_REG_TABLE_ROW		(0)	/**< Register Table
						  *  Row */
/*@}*/

/** @name Cp_Fifo_Type
 * @{
 */
#define CP_FIFO_TYPE_LUTRAM		(0)	/**< Fifo Value for Lutram */
#define CP_FIFO_TYPE_BLOCKRAM		(1)	/**< Fifo Value for Blockram */
/*@}*/

/**************************** Type Definitions *******************************/

/* This typedef contains configuration information for a device */
typedef struct {
	u16 DeviceId;			/**< Unique ID of device */
	u32 BaseAddress;		/**< Register Base Address */
	u16 NumberOfVsms;		/**< Number of VSMs */
	u8 RequiresClearBitstreams;	/**< Derived from CP_FAMILY */
	u16 Cp_Arbitration_Protocol;	/**< Arbitration protocol */
	u16 Has_Axi_Lite_If;		/**< Axi Lite Interface */
	u16 Reset_Active_Level;		/**< Reset Active Level */
	u16 Cp_Fifo_Depth;		/**< Fifo Depth */
	u16 Cp_Fifo_Type;		/**< Fifo Type */
	u16 Cp_Family;			/**< Device Family being managed */
	u16 Cdc_Stages;			/**< Cdc Stages */
	u16 Cp_Compression;		/**< Cp Compression */

	/* Per VSM information */
	u16 NumberOfRms[XPRC_MAX_NUMBER_OF_VSMS];/**< Number of RMs
						   *  in each VSM */
	u16 NumberOfRmsAllocated[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Number of RMs allocated
						  *  in each VSM */
	u8 Start_In_Shutdown[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Whether each VSM should
						  *  start in the shutdown
						  *  state */
	u16 No_Of_Triggers_Allocated[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Number of triggers for
						  *  each VSM */
	u8 Shutdown_On_Error[XPRC_MAX_NUMBER_OF_VSMS];
					/**< Whether each VSM should enter the
					  *  shutdown state if an error
					  *  occurs */
	u8 Has_Por_Rm[XPRC_MAX_NUMBER_OF_VSMS];	/**< Whether each VSM has
						  *  a POR RM */
	u16 Por_Rm[XPRC_MAX_NUMBER_OF_VSMS];	/**< The POR RM for each VSM */
	u16 Has_Axis_Status[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Whether each VSM has an
						  *  Axi4 Status Channel */
	u16 Has_Axis_Control[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Whether each VSM has an
						  *  Axi4 Control Channel */
	u16 Skip_Rm_Startup_After_Reset[XPRC_MAX_NUMBER_OF_VSMS];
						/**< Whether each VSM shoud
						  *  Skip Rm Startup after
						  *  Reset */
	u16 Num_Hw_Triggers[XPRC_MAX_NUMBER_OF_VSMS];
						/**< The Number of Hardware
						  *  Triggers each VSM has */

	/**
	 * Register information
	 * The address of each register is structured as:
	 * [VSM Select] [Bank Select] [ Register Select]
	 */
	u8 RegVsmMsb;		/**< MSB of register VSM */
	u8 RegVsmLsb;		/**< LSB of register VSM */
	u8 RegBankMsb;		/**< MSB of register bank */
	u8 RegBankLsb;		/**< LSB of register bank */
	u8 RegSelectMsb;	/**< MSB of register select */
	u8 RegSelectLsb;	/**< LSB of register select */
} XPrc_Config;

/**
 * The XPrc instance data structure. A pointer to an instance data structure
 * is passed around by functions to refer to a specific instance.
 */
typedef struct {
	u32 IsReady;		/**< Device is initialized and ready */
	XPrc_Config Config;	/**< Pointer to instance config entry */
} XPrc;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This macro is used to get the Number of Virtual Socket Managers.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Number of Vsms.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetNumberOfVsms(InstancePtr) \
		(InstancePtr)->Config.NumberOfVsms

/*****************************************************************************/
/**
*
* This macro is used to find if Clearing Bitstreams are required.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	0 if Clearing Bitstreams are not required, 1 if they are.
*
* @note		Clearing Bitstreams are only required when the device being
*		managed is an UltraScale device.
*
******************************************************************************/
#define XPrc_GetRequiresClearBitstreams(InstancePtr) \
		(InstancePtr)->Config.RequiresClearBitstreams

/*****************************************************************************/
/**
*
* This macro is used to get the CAP Arbitration protocol.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns the value of the CAP arbitration protocol.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCpArbitrationProtocol(InstancePtr) \
		(InstancePtr)->Config.Cp_Arbitration_Protocol

/*****************************************************************************/
/**
*
* This macro is used to discover if the AXI4-Lite register interface is enabled
* or disabled.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns a value when AXI4-Lite interface is enabled or
*		disabled.
*
* @note		This interface must be enabled for the driver to function.
*
******************************************************************************/
#define XPrc_GetHasAxiLiteIf(InstancePtr) \
		(InstancePtr)->Config.Has_Axi_Lite_If

/*****************************************************************************/
/**
*
* This macro is used to discover if the core's reset is active low (0) or
* active high (1).
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	1 if the core's reset is active high, 0 if it's active low.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetResetActiveLevel(InstancePtr) \
		(InstancePtr)->Config.Reset_Active_Level

/*****************************************************************************/
/**
*
* This macro returns the depth of FIFO in the fetch path.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Return Fifo depth value.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCpFifoDepth(InstancePtr) \
		(InstancePtr)->Config.Cp_Fifo_Depth

/*****************************************************************************/
/**
*
* This macro returns the type of the FIFO in the fetch path.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns
*		- CP_FIFO_TYPE_LUTRAM for Lutram type
*		- CP_FIFO_TYPE_BLOCKRAM for blockram type
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCpFifoType(InstancePtr)	(InstancePtr)->Config.Cp_Fifo_Type

/*****************************************************************************/
/**
*
* This macro returns the family type of the device being managed.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns 7, 8 or 9 for 7 Series, Ultrascale and UltraScale+.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCpFamily(InstancePtr)	(InstancePtr)->Config.Cp_Family

/*****************************************************************************/
/**
*
* This macro returns the number of synchronization stages used when
* crossing between clock domains.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns the number of CDC stages.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCdcStages(InstancePtr)	(InstancePtr)->Config.Cdc_Stages

/*****************************************************************************/
/**
*
* This macro tells the PRC whether partial bitstreams are compressed or not.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Returns 0 if PRC is same as before.
*               Returns 1 if decompression block is added to hardware.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetCpCompression(InstancePtr) (InstancePtr)->Config.Cp_Compression

/*****************************************************************************/
/**
*
* This macro is used to get the Virtual Socket Manager Select MSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Msb of Virtual Socket Manager.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegVsmMsb(InstancePtr)	(InstancePtr)->Config.RegVsmMsb

/*****************************************************************************/
/**
*
* This macro is used to get the Virtual Socket Manager Select LSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Lsb of Virtual Socket Manager.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegVsmLsb(InstancePtr)	(InstancePtr)->Config.RegVsmLsb

/*****************************************************************************/
/**
*
* This macro is used to get the Bank Select MSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Msb of Bank register.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegBankMsb(InstancePtr)	(InstancePtr)->Config.RegBankMsb

/*****************************************************************************/
/**
*
* This macro is used to get the Bank Select LSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Lsb of Bank register.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegBankLsb(InstancePtr)	(InstancePtr)->Config.RegBankLsb

/*****************************************************************************/
/**
*
* This macro is used to get the Register Select MSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Msb of Register selected.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegSelectMsb(InstancePtr) \
		(InstancePtr)->Config.RegSelectMsb

/*****************************************************************************/
/**
*
* This macro is used to get the Register Select LSB.
*
* @param	InstancePtr is a pointer to the PRC instance.
*
* @return	Lsb of Register selected.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetRegSelectLsb(InstancePtr) \
		(InstancePtr)->Config.RegSelectLsb

/*****************************************************************************/
/**
*
* This macro is used to get the number of Reconfigurable Modules in a VSM.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	The number of RMs that this VSM has.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetNumRms(InstancePtr, VsmId) \
		(InstancePtr)->Config.NumberOfRms[VsmId]

/*****************************************************************************/
/**
*
* This macro returns the number of Reconfigurable Modules that have been
* allocated in this VSM.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Number of RMs allocated in the VSM.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetNumRmsAllocated(InstancePtr, VsmId) \
		(InstancePtr)->Config.NumberOfRmsAllocated[VsmId]

/*****************************************************************************/
/**
*
* This macro returns if the VSM starts in the Shutdown state.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	1 if the VSM starts in Shutdown State, 0 if it starts in
*		the active state.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetStartInShutdown(InstancePtr, VsmId) \
		(InstancePtr)->Config.Start_In_Shutdown[VsmId]

/*****************************************************************************/
/**
*
* This macro returns the number of triggers that the Virtual Socket
* Manager has.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Number of triggers allocated in the Virtual Socket Manager.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetNumTriggersAllocated(InstancePtr, VsmId) \
		(InstancePtr)->Config.No_Of_Triggers_Allocated[VsmId]

/*****************************************************************************/
/**
*
* This macro returns whether the VSM will enter the shutdown state if an
* error is detected.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns 1 if the VSM will enter the shutdown state on an error,
*		0 if it doesn't.
*
* @note		None
*
******************************************************************************/
#define XPrc_GetShutdownOnError(InstancePtr, VsmId) \
		(InstancePtr)->Config.Shutdown_On_Error[VsmId]

/*****************************************************************************/
/**
*
* This macro returns whether the VSM contains an RM in the initial
* configuration bitstream.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns 1 if the VSM has an RM in the initial bitstream and
*		0 if it doesn't.
*
* @note		This will always return 1 when the device being managed is an
*		UltraScale device.
*
******************************************************************************/
#define XPrc_GetHasPorRm(InstancePtr, VsmId) \
		(InstancePtr)->Config.Has_Por_Rm[VsmId]

/*****************************************************************************/
/**
*
* This macro returns the RM in the initial configuration bitstream for this
* VSM. The returned value is only valid if XPrc_GetHasPorRm() returns 1 for
* this VSM.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns a RM identifier.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetPorRm(InstancePtr, VsmId) \
		(InstancePtr)->Config.Por_Rm[VsmId]

/*****************************************************************************/
/**
*
* This macro returns 1 if the AXI4-Stream Status Channel is enabled for this
* VSM, and 0 if it's disabled.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns 1 when the AXI4-Stream Status channel is enabled and
*		0 when it is disabled.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetHasAxisStatus(InstancePtr, VsmId) \
		(InstancePtr)->Config.Has_Axis_Status[VsmId]

/*****************************************************************************/
/**
*
* This macro returns 1 if the AXI4-Stream Control Channel is enabled for this
* VSM, and 0 if it's disabled.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns 1 when the AXI4-Stream Control channel is enabled and
*		0 when it is disabled.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetHasAxisControl(InstancePtr, VsmId) \
		(InstancePtr)->Config.Has_Axis_Control[VsmId]

/*****************************************************************************/
/**
*
* This macro returns whether the VSM will skip the Reconfigurable Module
* startup steps after a reset.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns 1 when the RM startup steps are skipped, and 0 when
*		they aren't.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetSkipRmStartupAfterReset(InstancePtr, VsmId) \
		(InstancePtr)->Config.Skip_Rm_Startup_After_Reset[VsmId]

/*****************************************************************************/
/**
*
* This macro returns the number of hardware triggers that the VSM has.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns the number of hardware triggers.
*
* @note		None.
*
******************************************************************************/
#define XPrc_GetNumHwTriggers(InstancePtr, VsmId) \
		(InstancePtr)->Config.Num_Hw_Triggers[VsmId]

/************************** Function Prototypes ******************************/

/* Lookup configuration in xprc_sinit.c */
XPrc_Config *XPrc_LookupConfig(u16 DeviceId);

/* Functions in xprc.c */
s32 XPrc_CfgInitialize(XPrc *InstancePtr, XPrc_Config *ConfigPtr,
				u32 EffectiveAddr);
void XPrc_SendShutdownCommand(XPrc *InstancePtr, u16 VsmId );
void XPrc_SendRestartWithNoStatusCommand(XPrc *InstancePtr, u16 VsmId );
void XPrc_SendRestartWithStatusCommand(XPrc *InstancePtr, u16 VsmId,
				u8 Full, u16 RmId );
void XPrc_SendProceedCommand(XPrc *InstancePtr, u16 VsmId);
void XPrc_SendUserControlCommand(XPrc *InstancePtr, u16 VsmId,
				u8 Rm_Shutdown_Req, u8 Rm_Decouple,
				u8 Sw_Shutdown_Req, u8 Sw_Startup_Req,
				u8 Rm_Reset);
u32 XPrc_ReadStatusReg(XPrc *InstancePtr, u16 VsmId);
u8 XPrc_IsVsmInShutdown(XPrc *InstancePtr, u32 VsmIdOrStatus);
u32 XPrc_GetVsmState(XPrc *InstancePtr, u32 VsmIdOrStatus);
u32 XPrc_GetVsmErrorStatus(XPrc *InstancePtr, u32 VsmIdOrStatus);
u32 XPrc_GetRmIdFromStatus(XPrc *InstancePtr, u32 VsmIdOrStatus);
u32 XPrc_GetBsIdFromStatus(XPrc *InstancePtr, u32 VsmIdOrStatus);
void XPrc_SendSwTrigger(XPrc *InstancePtr, u16 VsmId, u16 Trigger);
u8 XPrc_IsSwTriggerPending(XPrc *InstancePtr, u16 VsmId, u16 *Trigger);
void XPrc_SetTriggerToRmMapping(XPrc *InstancePtr, u16 VsmId, u16 TriggerId,
				u16 RmId);
u32 XPrc_GetTriggerToRmMapping(XPrc *InstancePtr, u16 VsmId, u16 TriggerId);
void XPrc_SetRmBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId, u16 BsIndex);
void XPrc_SetRmClearingBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId,
				u16 ClearingBsIndex);
u32 XPrc_GetRmBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId);
u16 XPrc_GetRmClearingBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId);
void XPrc_SetRmControl(XPrc *InstancePtr, u16 VsmId, u16 RmId,
				u8 ShutdownRequired, u8 StartupRequired,
				u8 ResetRequired, u8 ResetDuration);
void XPrc_GetRmControl(XPrc *InstancePtr, u16 VsmId, u16 RmId,
				u8 *ShutdownRequired, u8 *StartupRequired,
				u8 *ResetRequired, u8 *ResetDuration);
void XPrc_SetBsId(XPrc *InstancePtr, u16 VsmId, u16 BsIndex, u16 BsId);
u32 XPrc_GetBsId(XPrc *InstancePtr, u16 VsmId, u16 BsIndex);
void XPrc_SetBsSize(XPrc *InstancePtr, u16 VsmId, u16 BsIndex, u32 BsSize);
u32 XPrc_GetBsSize(XPrc *InstancePtr, u16 VsmId, u16 BsIndex);
void XPrc_SetBsAddress(XPrc *InstancePtr, u16 VsmId, u16 BsIndex,
				u32 BsAddress);
u32 XPrc_GetBsAddress(XPrc *InstancePtr, u16 VsmId, u16 BsIndex);
u32 XPrc_GetRegisterAddress(XPrc *InstancePtr, u32 VsmId, u8 RegisterType,
				u16 TableRow);
void XPrc_PrintVsmStatus(XPrc *InstancePtr, u32 VsmIdOrStatus, char * Prefix);

/* Functions in xprc_selftest.c */
s32 XPrc_SelfTest(XPrc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
