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
*
* @file xtrafgen.c
* @addtogroup trafgen_v4_2
* @{
*
* This file implements AXI Traffic Generator device-wise initialization and 
* control functions.
* For more information on the implementation of this driver, see xtrafgen.h.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a srt  01/24/13 First release
* 1.01a adk  03/09/13 Updated driver to Support Static and Streaming mode.
* 2.00a adk  16/09/13 Fixed CR:737291
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xtrafgen.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Get Command Info pointer
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       Pointer to the Command Info structure
*
* @note         C-style signature:
*               XTrafGen_CmdInfo *XTrafGen_GetCmdInfo(XTrafGen 
*				*InstancePtr)
*
*****************************************************************************/
#define XTrafGen_GetCmdInfo(InstancePtr) \
                        (&((InstancePtr)->CmdInfo))

/************************** Function Prototypes ******************************/
static void XTrafGen_PrepCmdWords(XTrafGen *InstancePtr,
				XTrafGen_Cmd *CmdPtr, u32 *CmdWords);
static void XTrafGen_PrepParamWord(XTrafGen *InstancePtr,
                                XTrafGen_Cmd *CmdPtr, u32 *ParamWord);
static int XTrafGen_ProgramCmdRam(XTrafGen *InstancePtr, u8 RdWrFlag);
static int XTrafGen_ProgramParamRam(XTrafGen *InstancePtr, u8 RdWrFlag);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function initializes a AXI Traffic Generator device.  This function 
 * must be called prior to using a AXI Traffic Generator Device.  Initializing
 * a engine includes setting up the register base address, setting up the 
 * instance data, and ensuring the hardware is in a quiescent state.
 *
 * @param       InstancePtr is a pointer to the Axi Traffic Generator instance 
 *              to be worked on.
 * @param	CfgPtr references the structure holding the hardware
 *		configuration for the Axi Traffic Generator core to initialize.
 * @param 	EffectiveAddr is the device base address in the virtual memory
 *		address	space. The caller is responsible for keeping the
 *		address mapping from EffectiveAddr to the device physical base
 *		address unchanged once this function is invoked. Unexpected
 *		errors may occur if the address mapping changes after this
 *		function is called. If address translation is not used,
 *		use Config->BaseAddress for this parameters, passing the
 *		physical address instead.
 *
 * @return
 *              - XST_SUCCESS for successful initialization
 *              - XST_INVALID_PARAM if pointer to the configuration structure
 *              is NULL
 *
 *****************************************************************************/
int XTrafGen_CfgInitialize(XTrafGen * InstancePtr,
			XTrafGen_Config *Config, UINTPTR EffectiveAddress)
{
	u32 ConfigStatus;

	InstancePtr->IsReady = 0;

	if(!Config) {
		return XST_INVALID_PARAM;
	}

	/* Setup the instance */
	memset(InstancePtr, 0, sizeof(XTrafGen));
	InstancePtr->Config.BaseAddress = EffectiveAddress;
	InstancePtr->Config.DeviceId = Config->DeviceId;
	InstancePtr->Config.AddressWidth = Config->AddressWidth;
		
	if((Config->BusType == 1) && (Config->Mode == 1 || 
						Config->ModeType == 2)) {
	
		ConfigStatus = XTrafGen_ReadConfigStatus(InstancePtr);

		/* Is it operating in Full Mode */
		if (ConfigStatus & XTG_CFG_STS_MFULL_MASK) {
			InstancePtr->OperatingMode = XTG_MODE_FULL;
		}
	
		/* Is it operating in Basic Mode */
		if (ConfigStatus & XTG_CFG_STS_MBASIC_MASK) {
			InstancePtr->OperatingMode = XTG_MODE_BASIC;
		}
	
		/* Master Width */
		InstancePtr->MasterWidth = 
				(ConfigStatus & XTG_CFG_STS_MWIDTH_MASK) >>
					XTG_CFG_STS_MWIDTH_SHIFT;

		/* Slave Width */
		InstancePtr->SlaveWidth = 
				(ConfigStatus & XTG_CFG_STS_SWIDTH_MASK) >>
					XTG_CFG_STS_SWIDTH_SHIFT;

		/* Initialize parameters */
		InstancePtr->CmdInfo.LastWrValidIndex = -1;
		InstancePtr->CmdInfo.LastRdValidIndex = -1;
	}

	/* Is it operating in Static Mode */	
	if((Config->BusType == 1) && (Config->Mode == 3) && 
			(Config->ModeType == 1 || Config->ModeType == 2)) {
			InstancePtr->OperatingMode = XTG_MODE_STATIC;
	}
	
	/* Is it operating in Streaming Mode */
	if((Config->BusType == 3) && (Config->Mode == 1)) {
			InstancePtr->OperatingMode = XTG_MODE_STREAMING;
	}
	
	/* Is it operating in System-init Mode */
	if(Config->BusType == 2) {
			InstancePtr->OperatingMode = XTG_MODE_SYS_INIT;
	}

	/* Initialization is successful */
	InstancePtr->IsReady = 1;

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* Add a command to the software list of commands.
*
* This function prepares the four Command Words and one Parameter Word from 
* the Command structure passed from the user application.  It then adds to a 
* list of commands (maintained in the software). Both CMDRAM and PARAMRAM are 
* divided into two regions, one for reads and one for writes. Each region can
* hold 256 commands with each entry containing four Command RAM words and one
* Parameter RAM word.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	CmdPtr is a pointer to Command structure.
*
* @return       
*		- XST_SUCCESS if successful
*		- XST_FAILURE if reached max number of command entries
*
*****************************************************************************/
int XTrafGen_AddCommand(XTrafGen *InstancePtr, XTrafGen_Cmd *CmdPtr)
{
	XTrafGen_CmdInfo *CmdInfo;
	u32 *Index;
	u32 *CmdWords;
	u32 *ParamWord;

	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);
        Xil_AssertNonvoid(CmdPtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	if ((CmdInfo->WrIndexEnd && CmdPtr->RdWrFlag) ||
		(CmdInfo->RdIndexEnd && !CmdPtr->RdWrFlag)) {
		return XST_FAILURE;
	}

	if (CmdPtr->RdWrFlag)
		Index = &CmdInfo->WrIndex;
	else
		Index = &CmdInfo->RdIndex;

	CmdWords = CmdInfo->CmdEntry[CmdPtr->RdWrFlag][*Index].CmdWords;
	XTrafGen_PrepCmdWords(InstancePtr, CmdPtr, CmdWords);

	ParamWord = &CmdInfo->CmdEntry[CmdPtr->RdWrFlag][*Index].ParamWord;
	XTrafGen_PrepParamWord(InstancePtr, CmdPtr, ParamWord);

	if (CmdPtr->CRamCmd.ValidCmd) { 
		if (CmdPtr->RdWrFlag == XTG_WRITE)
			CmdInfo->LastWrValidIndex = *Index;
		else
			CmdInfo->LastRdValidIndex = *Index;
	}

	if (*Index != MAX_NUM_ENTRIES - 1)
		(*Index)++;
	else {
		if (CmdPtr->RdWrFlag == XTG_WRITE)
			CmdInfo->WrIndexEnd = 1;
		else
			CmdInfo->RdIndexEnd = 1;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Get last Valid Command Index of Write/Read region
*
* The last valid command index is used to set 'my_depend' and 'other_depend'
* fields of the Command RAM (Word 2).

* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	RdWrFlag specifies a Read or Write Region 
*
* @return       
*		- Last Valid Command Index
*
*****************************************************************************/
int XTrafGen_GetLastValidIndex(XTrafGen *InstancePtr, u32 RdWrFlag)
{
	XTrafGen_CmdInfo *CmdInfo;
	u32 Index;
				
	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	if (RdWrFlag == XTG_WRITE)
		Index = CmdInfo->LastWrValidIndex;
	else
		Index = CmdInfo->LastRdValidIndex;

	return Index;
} 

/*****************************************************************************/
/**
* Write Commands to internal Command and Parameter RAMs
*
* This function writes all the prepared commands to hardware.

* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       
*		- XST_SUCCESS if successful
*	        - XST_FAILURE if programming internal RAMs failed
*
*****************************************************************************/
int XTrafGen_WriteCmdsToHw(XTrafGen *InstancePtr)
{
	u32 Index;
	u32 Status;

	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	for (Index = 0; Index < NUM_BLOCKS; Index++) {
		Status = XTrafGen_ProgramCmdRam(InstancePtr, Index);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
					
		Status = XTrafGen_ProgramParamRam(InstancePtr, Index);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Erase all Command Entry values
*
* This function erases all the 256 entries of both write and read regions with
* each entry containing four command words and parameter word.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
* @return       
*		- XST_SUCCESS if successful
*	        - XST_FAILURE if programming internal RAMs failed
*
*****************************************************************************/
int XTrafGen_EraseAllCommands(XTrafGen *InstancePtr)
{
	XTrafGen_CmdInfo *CmdInfo;
	u32 Status;
	u32 Index;

	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	memset(CmdInfo->CmdEntry, 0, sizeof(CmdInfo->CmdEntry));

	CmdInfo->WrIndex = CmdInfo->RdIndex = MAX_NUM_ENTRIES;

	for (Index = 0; Index < NUM_BLOCKS; Index++) {
		Status = XTrafGen_ProgramCmdRam(InstancePtr, Index);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
					
		Status = XTrafGen_ProgramParamRam(InstancePtr, Index);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	CmdInfo->WrIndex = CmdInfo->RdIndex = 0;
	CmdInfo->WrIndexEnd = CmdInfo->RdIndexEnd = 0;
	CmdInfo->LastWrValidIndex = CmdInfo->LastRdValidIndex = -1;
	
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Write or Read Master RAM
*
* The MSTRAM has 8 KB of internal RAM used for the following:
* - Take data from this RAM for write transactions
* - Store data to this RAM for read transaction
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	Offset is the offset value in Master RAM.
* @param	Length is the size of data to write/read.
* @param	RdWrFlag specifies whether to write or read
* @param 	Data is the pointer to array which contains data to write or
*		reads data into.
* 
*****************************************************************************/
void XTrafGen_AccessMasterRam(XTrafGen *InstancePtr, u32 Offset, 
					int Length, u8 RdWrFlag, u32 *Data)
{
	u32 Index = 0;
		
	/* Verify arguments */
        Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Offset + Length) <= XTG_MASTER_RAM_SIZE);

	while (Length > 0) {
		if (RdWrFlag == XTG_WRITE) {
			XTrafGen_WriteMasterRam(
					InstancePtr->Config.BaseAddress, 
					Offset, Data[Index]);
		} else {
			Data[Index] =
				XTrafGen_ReadMasterRam(
					InstancePtr->Config.BaseAddress, 
					Offset);
		}
		Length -= 4;
		Offset += 4;
		Index++;
	}
}

/*****************************************************************************/
/**
* Prepares all the four Command RAM Words
*
* The CMDRAM is divided into two 4 KB regions, one for reads and one for 
* writes. Each region of CMDRAM can hold 256 commands each of 128 bits i.e.
* Four command words each of size 32 bits. This function prepares these
* command words from the user specified configuration.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	CmdPtr is a pointer to Command structure.
* @param	CmdWords is a pointer to an array of four command words.
*
*****************************************************************************/
static void XTrafGen_PrepCmdWords(XTrafGen *InstancePtr,
				XTrafGen_Cmd *CmdPtr, u32 *CmdWords)
{
	XTrafGen_CRamCmd *Cmd;

	/* Verify arguments */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(CmdPtr != NULL);
        Xil_AssertVoid(CmdWords != NULL);

	Cmd = &CmdPtr->CRamCmd;

	/* Command Word 0 */
	if ( InstancePtr->Config.AddressWidth > 32) {
		/* Command Word 4 */
		CmdWords[0] = LOWER_32_BITS(Cmd->Address);
		CmdWords[4] = UPPER_32_BITS(Cmd->Address);
	} else {
		CmdWords[0] = Cmd->Address;
	}

	/* Command Word 1 */
        CmdWords[1] = 0;
        CmdWords[1] |= ((Cmd->Length & XTG_LEN_MASK) << XTG_LEN_SHIFT);
        CmdWords[1] |= ((Cmd->Lock & XTG_LOCK_MASK) << XTG_LOCK_SHIFT);
        CmdWords[1] |= ((Cmd->Burst & XTG_BURST_MASK) << XTG_BURST_SHIFT);
        CmdWords[1] |= ((Cmd->Size & XTG_SIZE_MASK) << XTG_SIZE_SHIFT);
        CmdWords[1] |= ((Cmd->Id & XTG_ID_MASK) << XTG_ID_SHIFT);
        CmdWords[1] |= ((Cmd->Prot & XTG_PROT_MASK) << XTG_PROT_SHIFT);
        CmdWords[1] |= ((Cmd->LastAddress & XTG_LAST_ADDR_MASK) <<
					XTG_LAST_ADDR_SHIFT);
        CmdWords[1] |= ((Cmd->ValidCmd & XTG_VALID_CMD_MASK) <<
					XTG_VALID_CMD_SHIFT);

	/* Command Word 2 */
        CmdWords[2] = 0;
        CmdWords[2] |= ((Cmd->MasterRamIndex & XTG_MSTRAM_INDEX_MASK) <<
					XTG_MSTRAM_INDEX_SHIFT);
        CmdWords[2] |= ((Cmd->OtherDepend & XTG_OTHER_DEPEND_MASK) <<
					XTG_OTHER_DEPEND_SHIFT);
        CmdWords[2] |= ((Cmd->MyDepend & XTG_MY_DEPEND_MASK) <<
					XTG_MY_DEPEND_SHIFT);

	/* Command Word 3 */
        CmdWords[3] = 0;
        CmdWords[3] |= ((Cmd->Qos & XTG_QOS_MASK) << XTG_QOS_SHIFT);
        CmdWords[3] |= ((Cmd->User & XTG_USER_MASK) << XTG_USER_SHIFT);
        CmdWords[3] |= ((Cmd->Cache & XTG_CACHE_MASK) << XTG_CACHE_SHIFT);
        CmdWords[3] |= ((Cmd->ExpectedResp & XTG_EXPECTED_RESP_MASK) <<
					XTG_EXPECTED_RESP_SHIFT);
}

/*****************************************************************************/
/**
* Prepares Parameter RAM Word
*
* The PARAMRAM extends the command programmability provided through command
* RAM, by adding extra 32 bits to the decode of each command. This function
* prepares this 32 bit Parameter RAM word.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	CmdPtr is a pointer to Command structure.
* @param	ParamWord is a pointer to an value of Parameter RAM word.
*
*****************************************************************************/
static void XTrafGen_PrepParamWord(XTrafGen *InstancePtr,
				XTrafGen_Cmd *CmdPtr, u32 *ParamWord)
{
	XTrafGen_PRamCmd *Cmd;

	/* Verify arguments */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(CmdPtr != NULL);
        Xil_AssertVoid(ParamWord != NULL);

	Cmd = &CmdPtr->PRamCmd;

	*ParamWord = 0;
	*ParamWord |= (Cmd->Opcode & XTG_PARAM_OP_MASK) << XTG_PARAM_OP_SHIFT;
	*ParamWord |= (Cmd->AddrMode & XTG_PARAM_ADDRMODE_MASK) <<
					XTG_PARAM_ADDRMODE_SHIFT;
	*ParamWord |= (Cmd->IdMode & XTG_PARAM_IDMODE_MASK) <<
					XTG_PARAM_IDMODE_SHIFT;
	*ParamWord |= (Cmd->IntervalMode & XTG_PARAM_INTERVALMODE_MASK) <<
					XTG_PARAM_INTERVALMODE_SHIFT;

	switch (Cmd->Opcode) {
	case XTG_PARAM_OP_RPT:
	case XTG_PARAM_OP_DELAY:
		*ParamWord |=  (Cmd->OpCntl0 & XTG_PARAM_COUNT_MASK) <<
					XTG_PARAM_COUNT_SHIFT;
		break;

	case XTG_PARAM_OP_FIXEDRPT:
    		*ParamWord |=  (Cmd->OpCntl0 & XTG_PARAM_ADDRRANGE_MASK) <<
					XTG_PARAM_ADDRRANGE_SHIFT;
    		*ParamWord |=  (Cmd->OpCntl1 & XTG_PARAM_DELAY_MASK) <<
					XTG_PARAM_DELAY_SHIFT;
    		*ParamWord |=  (Cmd->OpCntl2 & XTG_PARAM_DELAYRANGE_MASK) <<
					XTG_PARAM_DELAYRANGE_SHIFT;
		break;

	case XTG_PARAM_OP_NOP:
    		*ParamWord = 0;
		break;
	}
}

/*****************************************************************************/
/**
* Program Command RAM
*
* This function write the list of Command Words prepared by using 
* *_AddCommand() to Command RAM till the last command entry added.

* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	RdWrFlag specifies Read or Write Region 
*
* @return       
*		- XST_SUCCESS if successful
*		- XST_FAILURE if no valid commands present
*
*****************************************************************************/
static int XTrafGen_ProgramCmdRam(XTrafGen *InstancePtr, u8 RdWrFlag) 
{
	XTrafGen_CmdInfo *CmdInfo;
	u32 *CmdWords;
	u32 EntryIndex;
	u32 Index;
	u32 Offset;
	u32 CmdWordIndex;
	u32 Offset1;
	int ValidIndex;
	
	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	if (RdWrFlag == XTG_WRITE) {
		EntryIndex = CmdInfo->WrIndex;
		ValidIndex = CmdInfo->LastWrValidIndex;
		Offset = XTG_CMD_RAM_BLOCK_SIZE;
		Offset1 = XTG_EXTCMD_RAM_BLOCK_SIZE;
	} else {
		EntryIndex = CmdInfo->RdIndex;
		ValidIndex = CmdInfo->LastRdValidIndex;
		Offset = 0;
		Offset1 = 0;
	}

	for (Index = 0; Index < EntryIndex; Index++) {
		if (ValidIndex != -1 ) {
			CmdWords = CmdInfo->CmdEntry[RdWrFlag][Index].CmdWords;
			for (CmdWordIndex = 0; CmdWordIndex < 4; CmdWordIndex++) {
				XTrafGen_WriteCmdRam(
					InstancePtr->Config.BaseAddress,
					Offset, CmdWords[CmdWordIndex]);
				Offset += 4;
			}
			if ( InstancePtr->Config.AddressWidth > 32) {
				XTrafGen_WriteCmdRam_Msb(InstancePtr->Config.BaseAddress,
					Offset1, CmdWords[4]);
					Offset1 += 4;
			}

		} else {
			return XST_FAILURE;
		}
	}
 
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Program Parameter RAM
*
* This function write the list of Parameter Words prepared by using 
* *_AddCommand() to Parameter RAM till the last command entry added.

* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
* @param	RdWrFlag specifies Read or Write Region 
*
* @return       
*		- XST_SUCCESS if successful
*		- XST_FAILURE if no valid commands present
*
*****************************************************************************/
static int XTrafGen_ProgramParamRam(XTrafGen *InstancePtr, u8 RdWrFlag)
{
	XTrafGen_CmdInfo *CmdInfo;
	u32 *ParamWord;
	u32 EntryIndex;
	u32 Index;
	u32 Offset;
	int ValidIndex;
	
	/* Verify arguments */
        Xil_AssertNonvoid(InstancePtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	if (RdWrFlag == XTG_WRITE) {
		EntryIndex = CmdInfo->WrIndex;
		ValidIndex = CmdInfo->LastWrValidIndex;
		Offset = XTG_PRM_RAM_BLOCK_SIZE;
	} else {
		EntryIndex = CmdInfo->RdIndex;
		ValidIndex = CmdInfo->LastRdValidIndex;
		Offset = 0;
	}

	for (Index = 0; Index < EntryIndex; Index++) {
		if (ValidIndex != -1 ) {
			ParamWord =
				&CmdInfo->CmdEntry[RdWrFlag][Index].ParamWord;
			XTrafGen_WriteParamRam(
				InstancePtr->Config.BaseAddress,
				Offset, *ParamWord);
			Offset += 4;
		} else {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;	
}

/*****************************************************************************/
/**
* Display Command Entry values
*
* This function prints all the 256 entries of both write and read regions with
* each entry containing four command words and parameter word.
*
* @param        InstancePtr is a pointer to the Axi TrafGen instance to be
*               worked on.
*
*****************************************************************************/
void XTrafGen_PrintCmds(XTrafGen *InstancePtr)
{
	XTrafGen_CmdInfo *CmdInfo;
	int Index1;
	int Index2; 

	/* Verify arguments */
        Xil_AssertVoid(InstancePtr != NULL);

	CmdInfo = XTrafGen_GetCmdInfo(InstancePtr);

	xil_printf("Commands configured for Write Block: \n\r");
	for (Index1 = 0; Index1 < MAX_NUM_ENTRIES; Index1++) {
		xil_printf("Cmd%d:\t", Index1);
		for (Index2 = 0; Index2 < 5; Index2++) {
			xil_printf("0x%08x, ",
				CmdInfo->CmdEntry[1][Index1].CmdWords[Index2]);
		}
		xil_printf("0x%08x\n\r", CmdInfo->CmdEntry[1][Index1].ParamWord);
	}
			
	xil_printf("Commands configured for Read Block: \n\r");
	for (Index1 = 0; Index1 < MAX_NUM_ENTRIES; Index1++) {
		xil_printf("Cmd%d:\t", Index1);
		for (Index2 = 0; Index2 < 5; Index2++) {
			xil_printf("0x%08x, ",
				CmdInfo->CmdEntry[0][Index1].CmdWords[Index2]);
		}
		xil_printf("0x%08x\n\r", CmdInfo->CmdEntry[0][Index1].ParamWord);
	}
}
/** @} */
