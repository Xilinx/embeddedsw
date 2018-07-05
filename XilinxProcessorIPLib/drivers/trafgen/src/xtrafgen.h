/******************************************************************************
*
* Copyright (C) 2013 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xtrafgen.h
* @addtogroup trafgen_v4_2
* @{
* @details
*
* This file contains the implementation of the AXI Traffic Generator driver.
* User documentation for the driver functions is contained in this file in the 
* form of comment blocks at the front of each function.
*
* The AXI Traffic Generator IP is designed to generate AXI4 traffic which can 
* be used to stress different modules/interconnect connected in the system. 
* Different configurable options allow the user to generate a wide variety of
* traffic based on their requirements.  The core is broadly separated into a
* master and slave block, each of which contains the write block and read 
* block. Other support features are provided by the Control registers and
* Internal RAMs.
*
* The commands to be issued by the AXI traffic generator are loaded in a 
* 128-bit wide, 512 deep command RAM through AXI Slave interface. After the
* core is enabled, control logic issues the write/read commands based on the
* command settings programmed. The core updates the Status registers and
* asserts interrupts on the completion of issuing programmed commands.
*
* The Axi Traffic Genrator has five different modes:
*
* - Advanced Mode: Advanced Mode allows full control over the traffic generation
*   Control registers are provided to you to program the core to generate
*   different AXI4 transactions.
*
* - Basic Mode: Basic Mode allows basic AXI4 traffic generation with less
*   resource overhead.
*
* - Static Mode: Static Mode allows you to generate a simple AXI4 traffic with
*   very less resource and minimum processor intervention.In this Mode the core
*   continuously generates fixed address and fixed length INCR type read and
*   write transfers.
*
* - System Init Mode: System Init Mode is a special Mode where core provides
*   only AXI4-Lite Master write interface.This mode can be used in a system
*   without a processor to initialize the system peripherals with preconfigured
*   values on system reset.
*
* - Streaming Mode: In Streaming Mode the core can be configured to generate 
*   traffic based on the register configuration. 
*
* <b>Initialization & Configuration</b>
*
* The XTrafGen_Config structure is used by the driver to configure itself.
* This configuration structure is typically created by the tool-chain based
* on HW build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in the
* following way:
*
* - XTrafGen_LookupConfig(DeviceId) - Use the divide identifier to find the
*   static configuration structure defined in xtrafgen_g.c. This is setup
*   by the tools. For some operating systems the config structure will be
*   initialized by the software and this call is not needed.
*
* - XTrafGen_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*   configuration structure provided by the caller. If running in a system
*   with address translation, the provided virtual memory base address
*   replaces the physical address present in the configuration structure.
*
* <b>Command Handling</b>
*
* AXI Traffic Generator core operates based on the commands programmed into
* Command and Parameter RAMs. The CMDRAM and PARAMRAM is divided into two
* regions: write and read. Each region can hold 256 entries.  Once the core
* is enabled, the internal control logic issues write/read commands. To
* handle command programming efficiently, we are maintaining a software 
* list of commands. Following APIs are provided to handle this mechanism:
*
* - XTrafGen_AddCommand(): This function prepares the Command Words and 
*   Parameter Word from the Command structure passed from the user 
*   application.  It then adds to a software list of commands.
*
* - XTrafGen_WriteCmdsToHw(): This function writes the prepared list of 
*   Command and Parameter Words prepared to CMDRAM and PARAMRAM.
*
* - XTrafGen_GetLastValidIndex(): This function gets last Valid Command
*   Index of Write/Read region. The last valid command index is used to
*   set 'my_depend' and 'other_depend' fields of the Command RAM. 
*
* - XTrafGen_EraseAllCommands(): This function clears the list of commands
*   maintained in software and also updates the respective RAMs.
*
* - XTrafGen_PrintAllCmds(): This function displays the list of commands.  
*
* <b>Master RAM Handling</b>
*
* AXI Traffic Generator uses MSTRAM to 
* - Take data from this RAM for write transactions
* - Store data to this RAM for read transaction
* User need to call this API to write/read to/from Master RAM,
*
* - XTrafGen_AccessMasterRam() - This function programs the Master RAM
*   with the data which is used in master logic.  The amount of the data
*   is limited by the size of master RAM.
*
* <b>Interrupts</b>
*
* The driver defaults to no interrupts at initialization such that interrupts
* must be enabled if desired. An interrupt is generated for one of the
* following conditions:
* - Master Logic Completion Interrupt
* - Error Interrupt (For Master and Slave Errors)
*
* The application can control which interrupts are enabled using these
* functions:
* - XTrafGen_EnableMasterCmpInterrupt()
* - XTrafGen_MasterErrIntrEnable() 
* - XTrafGen_SlaveErrIntrEnable()
*
* The interrupt system has to be set up and if the interrupts are enabled,
* Traffic Generator notifies the software either about the completion or an 
* error in transfer through interrupts.
*
* <b> Examples </b>
*
* We provided two examples to show how to use the driver API:
* - One for interrupt mode (xtrafgen_interrupt_example.c)
* - One for polling mode (xtrafgen_polling_example.c)
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent.  It works with
* physical addresses only.  Any needs for dynamic memory management, threads or
* thread mutual exclusion, virtual memory, or cache control must be satisfied
* by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a srt  01/24/13 First release
* 1.01a adk  03/09/13 Updated Driver to Support Streaming and Static Mode
* 2.00a adk  16/09/13 Fixed CR:737291
* 2.01a adk  21/10/13 Fixed CR:740522 Updated the MasterRam offset as per latest
*		      IP.This driver is valid only for IP(v2.0) onwards. The 
*		      XTG_MASTER_RAM_OFFSET has been changed from 
*		      0x10000 to 0xc000.
* 2.01a adk  15/11/13 Fixed CR:760808 added Macro's for the New bit fields added
* 		      in the latest tarfgen IP(v2.0).
* 3.0   adk  12/10/13 Updated as per the New Tcl API's
* 3.1   adk  28/04/14 Fixed CR:782131 Incorrect mask value for the
*		      loopenable bit.
* 3.2   adk  05/08/14 Fixed CR:798742 The last word of 8KB Master RAM in
*		      axi traffic generator can't access and CR:799554
* 		      Some incorrect parameter in axi traffic generator driver.
* 4.0   sd  19/08/15 Fixed CR:876564 Added 64-bit Support to axi traffic generator
*		     driver.
* 4.1   ms  01/23/17 Modified xil_printf statement in main function for all
*                    examples to ensure that "Successfully ran" and "Failed"
*                    strings are available in all examples. This is a fix
*                    for CR-965028.
*       ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                    generation.
*       ms  04/05/17 Added tabspace for return statements in functions of
*                    trafgen examples for proper documentation while
*                    generating doxygen and also modified filename tag in
*                    master streaming example file to include it in doxygen
*                    examples.
* 4.2   ms  04/18/17 Modified tcl file to add suffix U for all macros
*                    definitions of trafgen in xparameters.h
* </pre>
******************************************************************************/

#ifndef XTRAFGEN_H          /* prevent circular inclusions */
#define XTRAFGEN_H          /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include <string.h>
#include "xstatus.h"
#include "xil_assert.h"
#include "xtrafgen_hw.h"

/************************** Constant Definitions *****************************/

#define MAX_NUM_ENTRIES	256	/**< Number of command entries per region */	
#define NUM_BLOCKS	2	/**< Number of Read and write regions */

/* Direction Flags */
#define XTG_WRITE	1	/**< Write Direction Flag */	 
#define XTG_READ	0	/**< Read Direction Flag */

/* Operating Mode flags */	
#define XTG_MODE_FULL		0	/**< Full Mode */
#define XTG_MODE_BASIC		1	/**< Basic Mode */
#define XTG_MODE_STATIC		2	/**< Static Mode */
#define XTG_MODE_STREAMING	3	/**< Streaming Mode */
#define XTG_MODE_SYS_INIT	4	/**< System Init Mode */

/* Master Width Flags */
#define XTG_MWIDTH_32	0	/**< Master Width - 32 */
#define XTG_MWIDTH_64	1	/**< Master Width - 64 */
	
/* Slave Width Flags */
#define XTG_SWIDTH_32	0	/**< Slave Width - 32 */
#define XTG_SWIDTH_64	1	/**< Slave Width - 64 */
	
/* Internal RAM Sizes */
#define XTG_PRM_RAM_BLOCK_SIZE	0x400	/**< PARAM Block Size (1KB) */	
#define XTG_CMD_RAM_BLOCK_SIZE	0x1000 	/**< Cmd RAM Block Size (4KB) */
#define XTG_EXTCMD_RAM_BLOCK_SIZE 0x400	/**< Extended CMDRAM Block Size (1KB) */
#define XTG_PARAM_RAM_SIZE	0x800	/**< Parameter RAM (2KB) */
#define XTG_COMMAND_RAM_SIZE	0x2000	/**< Command RAM (8KB) */
#define XTG_MASTER_RAM_SIZE	0x2000	/**< Master RAM (8KB) */

/************************** Type Definitions *****************************/

/**
 *  Command Ram word fields 
 *
 */
typedef struct XTrafGen_CRamCmd
{
	UINTPTR Address;		/**< Address Driven to a*_addr line */
	u32 ValidCmd;		/**< Valid Command */
	u32 LastAddress;	/**< Last address */
	u32 Prot;		/**< Driven to a*_prot line */
	u32 Id;			/**< Driven to a*_id line */
	u32 Size;		/**< Driven to a*_size line */
	u32 Burst;		/**< Driven to a*_burst line */
	u32 Lock;		/** Driven to a*_lock line */	
	u32 Length;		/**< Driven to a*_len line  */
	u32 MyDepend;		/**< My depend command no */
	u32 OtherDepend;	/**< Other depend Command no */
	u32 MasterRamIndex;	/**< Master Ram Index */
	u32 Qos;		/**< Driven to a*_qos line */
	u32 User;		/**< Driven to a*_user line */
	u32 Cache;		/**< Driven to a*_cache line */
	u32 ExpectedResp;	/**< Expected response */
} XTrafGen_CRamCmd;

/**
 *  Parameter Ram word fields 
 *
 */
typedef struct XTrafGen_PRamCmd
{
	u32 OpCntl0;		/**< Control field 0 */ 	
	u32 OpCntl1;		/**< Control field 1 */
	u32 OpCntl2;		/**< Control field 2 */
	u32 AddrMode;		/**< Address mode */
	u32 IntervalMode;	/**< Interval mode */
	u32 IdMode;		/**< Id mode */
	u32 Opcode;		/**< Opcode */
} XTrafGen_PRamCmd;

/**
 *  Command structure exposed to user
 *
 *  This structure should be updated by user with required
 *  configuration
 */ 
typedef struct XTrafGen_Cmd
{
	XTrafGen_CRamCmd CRamCmd; /**< Command RAM struct */ 
	XTrafGen_PRamCmd PRamCmd; /**< Param RAM struct */ 

	u8 RdWrFlag;		/**< Write/Read region? */ 
} XTrafGen_Cmd;

/**
 * Command Entry structure
 *
 * This structure denotes each entry of 256 commands.
 */
typedef struct XTrafGen_CmdEntry
{
	u32 CmdWords[5];	/**< Command Ram words */
	u32 ParamWord;		/**< Parameter Ram word */
} XTrafGen_CmdEntry;

/**
 * The configuration structure for Traffic Generator device
 *
 * This structure passes the hardware building information to the driver
 */
typedef struct XTrafGen_Config {
	u16 DeviceId;		/**< Device Id */
	UINTPTR BaseAddress;	/**< Base Address */
	u32 BusType;		/**< Atgmode */
	u32 Mode;		/**< Atgmode_l2 */
	u32 ModeType;		/**< Axismode */
	u32 AddressWidth;	/**< AddressWidth */
} XTrafGen_Config;

/**
 * Command Information Structure 
 *
 * This structure is maintained by the driver 
 */
typedef struct XTrafGen_CmdInfo {
	u32 WrIndex;	/**< Write Region Command Index */
	u32 RdIndex;	/**< Read Region Command Index */
	
	u8 WrIndexEnd;	/**< Write Index End */
	u8 RdIndexEnd;	/**< Read Index End */

	int LastWrValidIndex;	/**< Write Last Valid Command Index */ 
	int LastRdValidIndex;	/**< Read Last Valid Command Index */

	XTrafGen_CmdEntry CmdEntry[2][MAX_NUM_ENTRIES];
				/**< Software array of Commands */
} XTrafGen_CmdInfo;

/**
 * The XTrafGen driver instance data. An instance must be allocated for 
 * each Traffic Generator device in use.
 */
typedef struct XTrafGen {
	XTrafGen_Config Config; /**< Config Structure */

	u8 OperatingMode;	/**< Operating mode */
	u8 MasterWidth; 	/**< Master Width */
	u8 SlaveWidth;		/**< Slave Width */

	XTrafGen_CmdInfo CmdInfo; /**< Command Info structure */	
	
	int IsReady;	/* Device is initialized and ready */	
} XTrafGen;

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* XTrafGen_ReadCoreRevision reads revision of core.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	Core Revision Value
*
* @note         C-style signature:
*               u8 XTrafGen_ReadCoreRevision(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_ReadCoreRevision(InstancePtr)	\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET) & XTG_MCNTL_REV_MASK) >> \
		XTG_MCNTL_REV_SHIFT)

/****************************************************************************/
/**
*
* XTrafGen_ReadIdWidth reads M_ID_WIDTH.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	Value of M_ID_WIDTH
*
* @note         C-style signature:
*               u8 XTrafGen_ReadIdWidth(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_ReadIdWidth(InstancePtr)	\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET) & XTG_MCNTL_MSTID_MASK) >>	\
		XTG_MCNTL_MSTID_SHIFT)

/****************************************************************************/
/**
*
* XTrafGen_StartMasterLogic starts traffic generator master logic.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_StartMasterLogic(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StartMasterLogic(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_MCNTL_OFFSET) | XTG_MCNTL_MSTEN_MASK))

/****************************************************************************/
/**
*
* XTrafGen_IsMasterLogicDone checks for traffic generator master logic
* completed bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	TRUE if master logic completed.
*		FALSE if master logic not completed.
*
* @note         C-style signature:
*               u8 XTrafGen_IsMasterLogicDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_IsMasterLogicDone(InstancePtr)	\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET) & XTG_MCNTL_MSTEN_MASK) ? \
		FALSE : TRUE)

/****************************************************************************/
/**
*
* XTrafGen_LoopEnable loops through the command set created using CMDRAM and 
* PARAMRAM indefinitely in Advanced mode/Basic mode of ATG.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_LoopEnable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_LoopEnable(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_MCNTL_OFFSET) | XTG_MCNTL_LOOPEN_MASK))

/****************************************************************************/
/**
*
* XTrafGen_LoopDisable Disables the loop bit in Master control regiset in
* Advanced mode/Basic mode of ATG.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_LoopDisable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_LoopDisable(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_MCNTL_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_MCNTL_OFFSET) & ~XTG_MCNTL_LOOPEN_MASK))
			
/****************************************************************************/
/**
*
* XTrafGen_WriteSlaveControlReg enables control bits of Slave Control
* Register. This API will write the value passed from the user. 
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the Slave Control Register value to set 
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_WriteSlaveControlReg(XTrafGen *InstancePtr, 
*						u32 Value)
*****************************************************************************/
#define XTrafGen_WriteSlaveControlReg(InstancePtr, Value)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_SCNTL_OFFSET, Value)

/****************************************************************************/
/**
*
* XTrafGen_CheckforMasterComplete checks for master complete.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return      	TRUE if master complete bit is set. 
*               FALSE if master complete bit is not set.
*
* @note         C-style signature:
*               u8 XTrafGen_CheckforMasterComplete(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_CheckforMasterComplete(InstancePtr)	\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_STS_OFFSET) & XTG_ERR_MSTCMP_MASK) ? TRUE : FALSE)

/****************************************************************************/
/**
*
* XTrafGen_ReadErrors read master and slave errors.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	Both Master and Slave error value.
*
* @note         C-style signature:
*               u32 XTrafGen_ReadErrors(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_ReadErrors(InstancePtr)	\
	(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_STS_OFFSET) & XTG_ERR_ALL_ERR_MASK)

/****************************************************************************/
/**
*
* XTrafGen_EnableMasterCmpInterrupt enables Master logic complete bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_EnableMasterCmpInterrupt(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_EnableMasterCmpInterrupt(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_EN_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_ERR_EN_OFFSET) |	\
			XTG_ERR_MSTCMP_MASK))

/****************************************************************************/
/**
*
* XTrafGen_ClearMasterCmpInterrupt clear Master logic complete interrupt bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               u8 XTrafGen_ClearMasterCmpInterrupt(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_ClearMasterCmpInterrupt(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_STS_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_ERR_STS_OFFSET) |	\
			XTG_ERR_MSTCMP_MASK))

/****************************************************************************/
/**
*
* XTrafGen_ClearErrors clear errors specified in <i>Mask</i>.
* The corresponding error for each bit set to 1 in <i>Mask</i>, will be
* enabled.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param        Mask contains a bit mask of the errors to clear. The mask
*               can be formed using a set of bit wise or'd values from the
*               definitions in xtrafgen_hw.h file.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_ClearErrors(XTrafGen *InstancePtr,
*                                               u32 Mask)
*
*****************************************************************************/
#define XTrafGen_ClearErrors(InstancePtr, Mask)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_STS_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_ERR_STS_OFFSET) | Mask))

/****************************************************************************/
/**
*
* XTrafGen_EnableErrors enable errors specified in <i>Mask</i>.  The 
* corresponding error for each bit set to 1 in <i>Mask</i>, will be
* enabled.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param        Mask contains a bit mask of the errors to enable. The mask
*               can be formed using a set of bit wise or'd values from the
*               definitions in xtrafgen_hw.h file.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_EnableErrors(XTrafGen *InstancePtr,
*                                               u32 Mask)
*
*****************************************************************************/
#define XTrafGen_EnableErrors(InstancePtr, Mask)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_ERR_EN_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_ERR_EN_OFFSET) | Mask))

/*****************************************************************************/
/**
*
* XTrafGen_MasterErrIntrEnable enables Global Master error bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_MasterErrIntrEnable(XTrafGen *InstancePtr)
*
******************************************************************************/
#define XTrafGen_MasterErrIntrEnable(InstancePtr)                     \
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,  \
		XTG_MSTERR_INTR_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
			XTG_MSTERR_INTR_OFFSET) |	\
			XTG_MSTERR_INTR_MINTREN_MASK)) 

/*****************************************************************************/
/**
*
* XTrafGen_MasterErrIntrDisable disables Global Master error bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_MasterErrIntrDisable(XTrafGen *InstancePtr)
*
******************************************************************************/
#define XTrafGen_MasterErrIntrDisable(InstancePtr)                     \
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,  \
		XTG_MSTERR_INTR_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
			XTG_MSTERR_INTR_OFFSET) &	\
			~XTG_MSTERR_INTR_MINTREN_MASK)) 

/*****************************************************************************/
/**
*
* XTrafGen_SlaveErrIntrEnable enables Global Slave error bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_SlaveErrIntrEnable(XTrafGen *InstancePtr)
*
******************************************************************************/
#define XTrafGen_SlaveErrIntrEnable(InstancePtr)                     \
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,  \
		XTG_SCNTL_OFFSET, \
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
			XTG_SCNTL_OFFSET) | XTG_SCNTL_ERREN_MASK)) 

/*****************************************************************************/
/**
*
* XTrafGen_SlaveErrIntrDisable disables Global Slave error bit.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	None
*
* @note         C-style signature:
*               void XTrafGen_SlaveErrIntrDisable(XTrafGen *InstancePtr)
*
******************************************************************************/
#define XTrafGen_SlaveErrIntrDisable(InstancePtr)	\
	XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,  \
		XTG_SCNTL_OFFSET, \
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
			XTG_SCNTL_OFFSET) & 	\
			~XTG_SCNTL_ERREN_MASK)) 

/*****************************************************************************/
/**
*
* XTrafGen_ReadConfigStatus reads Config status register.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	Config Status Register value
*
* @note         C-style signature:
*               u32 XTrafGen_ReadConfigStatus(XTrafGen *InstancePtr)
*
******************************************************************************/
#define XTrafGen_ReadConfigStatus(InstancePtr)                     \
	(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,  \
	XTG_CFG_STS_OFFSET))

/*****************************************************************************/
/**
* 
* XTrafGen_StaticEnable enable the traffic generation when the core is
* configured Static Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_StaticEnable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StaticEnable(InstancePtr)				\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STATIC_CNTL_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		 XTG_STATIC_CNTL_OFFSET) | XTG_STATIC_CNTL_STEN_MASK)))	

/*****************************************************************************/
/**
* 
* XTrafGen_StaticDisable disables the traffic generation on the Axi TrafGen
* when the core is configured in Static Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_StaticDisable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StaticDisable(InstancePtr)				\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STATIC_CNTL_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		 XTG_STATIC_CNTL_OFFSET) & XTG_STATIC_CNTL_RESET_MASK)))
				
/*****************************************************************************/
/**
* 
* XTrafGen_StaticVersion returns the version value for the Axi TrafGen
* When configured in Static Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Static version value.
*
* @note         C-style signature:
*               u32 XTrafGen_StaticVersion(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StaticVersion(InstancePtr)				\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STATIC_CNTL_OFFSET) & XTG_STATIC_CNTL_VER_MASK) >>  \
		XTG_STATIC_CNTL_VER_SHIFT )
				
/*****************************************************************************/
/**
* 
* XTrafGen_SetStaticBurstLen Configures the Burst Length for AxiTrafGen
* In Static Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the Burst length to set in the Static length register.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStaticBurstLen(XTrafGen *InstancePtr, 
*						u32 Value)
*****************************************************************************/
#define XTrafGen_SetStaticBurstLen(InstancePtr,Value)			\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STATIC_LEN_OFFSET,Value))

/*****************************************************************************/
/**
* 
* XTrafGen_GetStaticBurstLen Gets the Burst Length for AxiTrafGen in StaticMode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Burst length value.
*
* @note         C-style signature:
*               u32 XTrafGen_GetStaticBurstLen(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetStaticBurstLen(InstancePtr)			\
	(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STATIC_LEN_OFFSET))

/*****************************************************************************/
/**
* 
* XTrafGen_GetStaticTransferDone gets the state of Transfer done bit 
* in Control register When the TraficGen is configured in Static Mode.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Value of the Transfer Done bit.
*
* @note         C-style signature:
*               u32 XTrafGen_GetStaticTransferDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetStaticTransferDone(InstancePtr)			\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
			XTG_STATIC_CNTL_OFFSET)) & XTG_STATIC_CNTL_TD_MASK)

/*****************************************************************************/
/**
* 
* XTrafGen_SetStaticTransferDone sets the Transfer done bit in Control register 
* When AxiTrafGen is Configured in Static Mode.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStaticTransferDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_SetStaticTransferDone(InstancePtr)   			\
	XTrafGen_WriteReg(InstancePtr->Config.BaseAddress, 	        \
		XTG_STATIC_CNTL_OFFSET, 				\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STATIC_CNTL_OFFSET) | XTG_STATIC_CNTL_TD_MASK))

/****************************************************************************/
/**
*
* XTrafGen_IsStaticTransferDone checks for reset value  When Static Traffic generation
* Completed by reading Control Register.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	TRUE if reset Success full
*		FALSE if failed to reset
*
* @note         C-style signature:
*               u8 XTrafGen_IsStaticTransferDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_IsStaticTransferDone(InstancePtr)	\
	(((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STATIC_CNTL_OFFSET) & XTG_STATIC_CNTL_TD_MASK) == \
		XTG_STATIC_CNTL_RESET_MASK) ? \
		TRUE : FALSE)	
	
/*****************************************************************************/
/**
* 
* XTrafGen_StreamEnable enable the traffic generation on the Axi TrafGen
* When the core is configured in Streaming Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_StreamEnable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StreamEnable(InstancePtr) 				\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STREAM_CNTL_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CNTL_OFFSET) | XTG_STREAM_CNTL_STEN_MASK)))

/*****************************************************************************/
/**
* 
* XTrafGen_StreamDisable Disable the traffic generation on the Axi TrafGen
* When core is configured in Streaming Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_StreamDisable(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StreamDisable(InstancePtr)			\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CNTL_OFFSET,				\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CNTL_OFFSET) & XTG_STREAM_CNTL_RESET_MASK)))

/*****************************************************************************/
/**
* 
* XTrafGen_StreamVersion returns the version value for the Axi TrafGen 
* When configured in  Streaming Mode
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Streaming Version Value.
*
* @note         C-style signature:
*               u8 XTrafGen_StreamVersion(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_StreamVersion(InstancePtr)		\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CNTL_OFFSET) & XTG_STREAM_CNTL_VER_MASK) \
		>> XTG_STREAM_CNTL_VER_SHIFT )
	
/*****************************************************************************/
/**
* 
* XTrafGen_SetStreamingTransLen Configures the length of transaction for 
* AxiTrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the transfer length to set in the transfer length 
*		Register.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingTransLen(XTrafGen *InstancePtr, 
*						u32 Value)
*****************************************************************************/
#define XTrafGen_SetStreamingTransLen(InstancePtr,Value)		\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STREAM_TL_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_TL_OFFSET) | Value)))

/*****************************************************************************/
/**
* 
* XTrafGen_GetStreamingTransLen Gets the length of transaction for 
* AxiTrafGen in Streaming Mode
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Transfer Length value.
*
* @note         C-style signature:
*               u16 XTrafGen_GetStreamingTransLen(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetStreamingTransLen(InstancePtr)		\
	(XTrafGen_ReadReg(InstancePtr->Config.BaseAddress,	\
		XTG_STREAM_TL_OFFSET)& XTG_STREAM_TL_TLEN_MASK)

/*****************************************************************************/
/**
* 
* XTrafGen_GetStreamingTransCnt Gets the transfer count for AxiTrafGen in 
* Streaming Mode
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Transfer Count value.
*
* @note         C-style signature:
*               u16 XTrafGen_GetStreamingTransCnt(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetStreamingTransCnt(InstancePtr)		\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
		XTG_STREAM_TL_OFFSET ) & XTG_STREAM_TL_TCNT_MASK) \
		>> XTG_STREAM_TL_TCNT_SHIFT)

/*****************************************************************************/
/**
* 
* XTrafGen_SetStreamingRandomLen Configures the random transaction length 
* for AxiTrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param 	Value is the random length that's need to be Configure in the
*		Streaming Config register.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingRandomLen(XTrafGen *InstancePtr, 
*						u32 Value)
*
*****************************************************************************/
#define XTrafGen_SetStreamingRandomLen(InstancePtr,Value)		\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STREAM_CFG_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CFG_OFFSET) | Value)))
	
/*****************************************************************************/
/**
* 
* XTrafGen_GetStreamingProgDelay Gets the Programmable Delay for AxiTrafGen in 
* Streaming Mode.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Propagation Delay Value
*
* @note         C-style signature:
*               u16 XTrafGen_GetProgDelay(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetStreamingProgDelay(InstancePtr)	\
	((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
		XTG_STREAM_TL_OFFSET ) \
		& XTG_STREAM_CFG_PDLY_MASK) >> XTG_STREAM_CFG_PDLY_SHIFT)

/*****************************************************************************/
/**
* 
* XTrafGen_SetStreamingTransCnt Configures the transfer count for 
* AxiTrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the transfer length that needs to be configured in
*		Transfer length register.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingTransCnt(XTrafGen *InstancePtr, 
*						u32 Value)
*
*****************************************************************************/
#define XTrafGen_SetStreamingTransCnt(InstancePtr, Value) \
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_TL_OFFSET,	\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		 XTG_STREAM_TL_OFFSET) |((Value << XTG_STREAM_TL_TCNT_SHIFT) \
		 & XTG_STREAM_TL_TCNT_MASK))))
		
/*****************************************************************************/
/**
* 
* XTrafGen_SetStreamingProgDelay Configures the Programmable Delay for 
* AxiTrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the value that's need to be configure in the Stream
*		Config Register.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingProgDelay(XTrafGen *InstancePtr, 
*						u32 Value)
*
*****************************************************************************/
#define XTrafGen_SetStreamingProgDelay(InstancePtr, Value) \
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CFG_OFFSET,  \
	        (XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
		 XTG_STREAM_CFG_OFFSET)|(Value << XTG_STREAM_CFG_PDLY_SHIFT)) \
		 & XTG_STREAM_CFG_PDLY_MASK))

/*****************************************************************************/
/**
* 
* XTrafGen_SetStreamingTdestPort Configures the Value to drive on TDEST port 
* for Axi TrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Value is the Port value that's need to be set.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingTdestPort(XTrafGen *InstancePtr, 
*						u8 Value)
*
*****************************************************************************/
#define XTrafGen_SetStreamingTdestPort(InstancePtr, Value) \
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,  \
		XTG_STREAM_CFG_OFFSET,  \
	        (XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
		XTG_STREAM_CFG_OFFSET)|(Value << XTG_STREAM_CFG_TDEST_SHIFT)) \
		& XTG_STREAM_CFG_TDEST_MASK))
		
/*****************************************************************************/
/**
* 
* XTrafGen_SetTransferDone sets the Transfer done bit in Control register 
* When AxiTrafGen is Configured in  Streaming Mode.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_SetStreamingTransferDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_SetStreamingTransferDone(InstancePtr)   \
	XTrafGen_WriteReg(InstancePtr->Config.BaseAddress,   \
		XTG_STREAM_CNTL_OFFSET, 			\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress, \
		XTG_STREAM_CNTL_OFFSET) | XTG_STREAM_CNTL_TD_MASK))

/****************************************************************************/
/**
*
* XTrafGen_IsStreamingTransferDone checks for reset value  When Streaming Traffic 
* generation is Completed by reading Stream Control Register.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return	TRUE if reset Success full
*		FALSE if failed to reset
*
* @note         C-style signature:
*               u8 XTrafGen_IsStreamingTransferDone(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_IsStreamingTransferDone(InstancePtr)	\
	(((XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CNTL_OFFSET) & XTG_STREAM_CNTL_TD_MASK) == \
		XTG_STREAM_CNTL_RESET_MASK) ? \
		TRUE : FALSE)	
		
/*****************************************************************************/
/**
* 
* XTrafGen_ResetStreamingRandomLen resets the random transaction length 
* for AxiTrafGen in Streaming Mode.
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       None.
*
* @note         C-style signature:
*               void XTrafGen_ResetStreamingRandomLen(XTrafGen *InstancePtr)
*
*****************************************************************************/
#define XTrafGen_ResetStreamingRandomLen(InstancePtr)		\
	(XTrafGen_WriteReg((InstancePtr)->Config.BaseAddress,		\
		XTG_STREAM_CFG_OFFSET,					\
		(XTrafGen_ReadReg((InstancePtr)->Config.BaseAddress,	\
		XTG_STREAM_CFG_OFFSET) & ~XTG_STREAM_CFG_RANDL_MASK)))
			
/************************** Function Prototypes ******************************/

/*
 * Initialization and control functions in xtrafgen.c
 */
int XTrafGen_CfgInitialize(XTrafGen * InstancePtr,
                             XTrafGen_Config *Config, UINTPTR EffectiveAddress);
XTrafGen_Config *XTrafGen_LookupConfig(u32 DeviceId);
int XTrafGen_AddCommand(XTrafGen *InstancePtr, XTrafGen_Cmd *CmdPtr);
int XTrafGen_GetLastValidIndex(XTrafGen *InstancePtr, u32 RdWrFlag);
int XTrafGen_WriteCmdsToHw(XTrafGen *InstancePtr);
void XTrafGen_AccessMasterRam(XTrafGen *InstancePtr, u32 Offset,
                                     int Length, u8 Write, u32 *Data);
void XTrafGen_PrintCmds(XTrafGen *InstancePtr);
int XTrafGen_EraseAllCommands(XTrafGen *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
