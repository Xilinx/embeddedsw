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

/*****************************************************************************/
/**
* @file xmesim_elfload.c
* @{
*
* This file contains the API for ELF loading. Applicable only for the
* ME simulation environment execution on linux.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  05/07/2018  Fixed CR#1001944, CR#1002101
* 1.2  Naresh  06/13/2018  Fixed CR#1003905
* 1.3  Naresh  07/11/2018  Updated copyright info
* 1.4  Naresh  07/26/2018  Fixed CR#1007367
* 1.5  Hyun    08/27/2018  Fixed the incorrect remaining bytes, CR-1009665
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "xmesim.h"
#include "xmesim_elfload.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XMeGbl_Config XMeGbl_ConfigTable[];

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the API to load the specified ELF to the target ME Tile program
* memory followed by clearing of the BSS sections.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ElfPtr: Path to the ELF file to be loaded into memory.
*
* @return	XMESIM_SUCCESS on success, else XMESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XMeSim_LoadElf(XMeSim_Tile *TileInstPtr, uint8 *ElfPtr, uint8 LoadSym)
{
	FILE *Fd;
	Elf32_Ehdr ElfHdr;
	Elf32_Shdr SectHdr[XMESIM_ELF_SECTION_NUMMAX];
	XMeSim_StackSz StackSz;

	uint8 MapPath[256U];
	uint8 Count = 0U;
	uint8 ShNameIdx = 0U;
	uint8 ShName[XMESIM_ELF_SECTION_NUMMAX][XMESIM_ELF_SECTION_NAMEMAXLEN];

	uint32 Idx = 0U;
	uint32 Status;
	uint32 *DataPtr;
	uint32 *CurrPtr;
	uint32 NumBytesRead;
	uint32 NumBytesRem;
	uint32 PrevPos;
	uint32 CurrPos;
        uint32 DmbOff;
        uint32 SectAddr;
        uint32 RemSize;
        uint32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;

	/* Get the stack range */
	strcpy(MapPath, ElfPtr);
	strcat(MapPath, ".map");
	Status = XMeSim_GetStackRange(MapPath, &StackSz);
	XMeSim_print("Stack start:%08x, end:%08x\n",
					StackSz.start, StackSz.end);
	if(Status != XMESIM_SUCCESS) {
		XMeSim_print("ERROR: Stack range definition failed\n");
		return Status;
	}

	/* Send the stack range set command */
	XMeSim_WriteCmd(XMESIM_CMDIO_CMD_SETSTACK, TileInstPtr->ColId,
		TileInstPtr->RowId, StackSz.start, StackSz.end, XME_NULL);

	/* Load symbols if enabled */
	if(LoadSym == XME_ENABLE) {
		XMeSim_LoadSymbols(TileInstPtr, ElfPtr);
	}

	/* Open the ELF file for reading */
	Fd = fopen(ElfPtr, "r");
	if(Fd == NULL) {
		XMeSim_print("ERROR: Invalid ELF file\n");
		return XMESIM_FAILURE;
	}

	/* Read the ELF header */
        NumBytesRem = sizeof(ElfHdr);
	NumBytesRead = 0U;
	PrevPos = ftell(Fd);
	CurrPtr = (uint8 *)&ElfHdr;

	/* Read complete data of ELF headers until all bytes  are read */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(uint8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((uint8 *)CurrPtr, NumBytesRem, Fd);
		}

		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (uint8 *)CurrPtr + NumBytesRead;

		XMeSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
						NumBytesRead, NumBytesRem);
	}

	XMeSim_print("**** ELF HEADER ****\n");
	XMeSim_print("e_type\t\t : %08x\ne_machine\t : %08x\ne_version\t : "
		"%08x\ne_entry\t\t : %08x\ne_phoff\t\t : %08x\n",
		ElfHdr.e_type, ElfHdr.e_machine, ElfHdr.e_version,
		ElfHdr.e_entry, ElfHdr.e_phoff);
	XMeSim_print("e_shoff\t\t : ""%08x\ne_flags\t\t : %08x\ne_ehsize\t"
		" : %08x\ne_phentsize\t : %08x\ne_phnum\t\t : %08x\n"
		"e_shentsize\t : %08x\ne_shnum\t\t : %08x\ne_shstrndx\t : "
		"%08x\n\n", ElfHdr.e_shoff, ElfHdr.e_flags,
		ElfHdr.e_ehsize, ElfHdr.e_phentsize, ElfHdr.e_phnum,
		ElfHdr.e_shentsize, ElfHdr.e_shnum, ElfHdr.e_shstrndx);

	/* Point to the section headers in the file */
	fseek(Fd, ElfHdr.e_shoff, SEEK_SET);
	/* Read the section header entries */
	//fgets(&SectHdr[0], (sizeof(SectHdr[0]) * ElfHdr.e_shnum), Fd);

	NumBytesRem = (sizeof(SectHdr[0]) * ElfHdr.e_shnum);
	NumBytesRead = 0U;
	PrevPos = ftell(Fd);
	CurrPtr = (uint8 *)&SectHdr[0];

	/* Read complete data of section headers until all bytes  are read */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(uint8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((uint8 *)CurrPtr, NumBytesRem, Fd);
		}

		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (uint8 *)CurrPtr + NumBytesRead;

		XMeSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
						NumBytesRead, NumBytesRem);
	}

	/*
	 * Get the section names from the .shstrtab section. First find the
	 * index of this section among the section header entries
	 */
	while(Count < ElfHdr.e_shnum) {
		if(SectHdr[Count].sh_type == SHT_STRTAB) {
			ShNameIdx = Count;
			break;
		}
		Count++;
	}

	/* Now point to the .shstrtab section in the ELF file */
	fseek(Fd, SectHdr[ShNameIdx].sh_offset, SEEK_SET);
	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		/*
		 * Read each section's name string from respective offsets
		 * from the .shstrtab section
		 */
		fseek(Fd, SectHdr[Count].sh_name, SEEK_CUR);
		fgets(ShName[Count], XMESIM_ELF_SECTION_NAMEMAXLEN, Fd);

		/*
		 * Again point to the start of the section .shstrtab for the
		 * next section to get its name from its corresponding offset
		 */
		fseek(Fd, SectHdr[ShNameIdx].sh_offset, SEEK_SET);
		Count++;
	}

	XMeSim_print("**** SECTION HEADERS ****\n");
	XMeSim_print("Idx\tsh_name\t\tsh_type\t\tsh_flags\tsh_addr\t\tsh_offset"
		"\tsh_size\t\tsh_link\t\tsh_addralign\tsection_name\n");
	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		XMeSim_print("%d\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t"
			"%08x\t%s\n", Count, SectHdr[Count].sh_name,
			SectHdr[Count].sh_type, SectHdr[Count].sh_flags,
			SectHdr[Count].sh_addr, SectHdr[Count].sh_offset,
			SectHdr[Count].sh_size, SectHdr[Count].sh_link,
			SectHdr[Count].sh_addralign, ShName[Count]);
		Count++;
	}

	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		/* Copy the program data sections to memory */
		if((SectHdr[Count].sh_type == SHT_PROGBITS) &&
			((SectHdr[Count].sh_flags &
				(SHF_ALLOC | SHF_EXECINSTR)) ||
			(SectHdr[Count].sh_flags &
				(SHF_ALLOC | SHF_WRITE)))) {
			
			XMeSim_WriteSection(TileInstPtr, ShName[Count],
							&SectHdr[Count], Fd);
		}

		/* Zero out the bss sections */
		if((SectHdr[Count].sh_type == SHT_NOBITS) &&
				(strstr(ShName[Count], "bss.DMb") != NULL)) {
			XMeSim_print("Zeroing out the bss sections\n");

			TgtTileAddr = XMeSim_GetTargetTileAddr(TileInstPtr,
						SectHdr[Count].sh_addr);
                        SectAddr = SectHdr[Count].sh_addr;
                        RemSize = SectHdr[Count].sh_size;
                        DoneSize = 0U;

			/* Use 32 bit loads to match sim output */
			for(Idx = 0U; Idx < RemSize; Idx += 4U){
				/* Mask address as per the Data memory offset*/
				DmbOff = (SectAddr &
                                        XMESIM_ELF_TILEADDR_DMB_MASK) + Idx;

                                if(DmbOff & 0x8000U) {
                                        /*
                                         * 32 KB boundary cross. Data section 
                                         * moving to the memory bank of next
                                         * cardinal direction. Change the target
                                         * tile address
                                         */
                                        SectHdr[Count].sh_addr += 0x8000U;
                                        TgtTileAddr =
                                        XMeSim_GetTargetTileAddr(TileInstPtr,
						(SectHdr[Count].sh_addr &
                                                0x1FFFFU));
                                        SectAddr = 0x0U;
                                        Idx = 0U;
                                        DmbOff = 0U;
                                        RemSize -= DoneSize;
                                        DoneSize = 0U;
                                }

                                DmbAddr = TgtTileAddr +
                                        XMESIM_ELF_TILECORE_DATMEM + DmbOff;
				XMeSim_Write32(DmbAddr, 0U);
                                DoneSize += 4U;
			}
		}
		Count++;
	}
	fclose(Fd);

	return XMESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the routine to derive the stack start and end addresses from the
* specified map file. This function basically looks for the line
* <b><init_address>..<final_address> ( <num> items) : Stack</b> in the
* map file to derive the stack address range.
*
* @param	MapPtr: Path to the Map file.
* @param	StackSzPtr: Pointer to the stack range structure.
*
* @return	XMESIM_SUCCESS on success, else XMESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XMeSim_GetStackRange(uint8 *MapPtr, XMeSim_StackSz *StackSzPtr)
{
	FILE *Fd;
	uint8 buffer[200U];

	/*
	 * Read map file and look for line:
	 * <init_address>..<final_address> ( <num> items) : Stack
	 */
	StackSzPtr->start = 0xFFFFFFFFU;
	StackSzPtr->end = 0U;

	Fd = fopen(MapPtr, "r");
	if(Fd == NULL) {
		XMeSim_print("ERROR: Invalid Map file\n");
		return XMESIM_FAILURE;
	}

	while(fgets(buffer, 200U, Fd) != NULL) {
		if(strstr(buffer, "items) : Stack") != NULL) {
			sscanf(buffer, "    0x%8x..0x%8x (%*s",
					&StackSzPtr->start, &StackSzPtr->end);
			break;
		}
	}

	if(StackSzPtr->start == 0xFFFFFFFFU) {
		return XMESIM_FAILURE;
	} else {
		return XMESIM_SUCCESS;
	}
	fclose(Fd);
}

/*****************************************************************************/
/**
*
* This routine sends the out of bound command to the sim to load symbols.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ElfPtr: Path to the ELF file.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_LoadSymbols(XMeSim_Tile *TileInstPtr, uint8 *ElfPtr)
{
	XMeSim_WriteCmd(XMESIM_CMDIO_CMD_LOADSYM, TileInstPtr->ColId,
					TileInstPtr->RowId, 0, 0, ElfPtr);
}



/*****************************************************************************/
/**
*
* This routine is used to write to the specified section by reading the
* corresponding data from the ELF file.
*
* @param	TileInstPtr: Pointer to the Tile instance structure.
* @param        SectName: Name of the section.
* @param        SectPtr: Poiner to the section entry in the ELF file.
* @param	Fd: Pointer to the ELF file.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_WriteSection(XMeSim_Tile *TileInstPtr, uint8 *SectName,
						Elf32_Shdr *SectPtr, FILE *Fd)
{
	uint32 *DataPtr;
	uint32 *CurrPtr;
	uint32 NumBytesRead;
	uint32 NumBytesRem;
	uint32 PrevPos;
	uint32 CurrPos;
	uint32 Idx;
        uint32 DmbOff;
        uint32 SectAddr;
        uint32 RemSize;
        uint32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;

	/* Point to the section in the ELF file */
	fseek(Fd, SectPtr->sh_offset, SEEK_SET);

	/* Allocate a memory of the size of this section */
	DataPtr = (uint32 *)malloc(SectPtr->sh_size);
	memset((uint8 *)DataPtr, 0U, SectPtr->sh_size);

	NumBytesRem = SectPtr->sh_size;
	NumBytesRead = 0U;
	CurrPtr = DataPtr;
	PrevPos = ftell(Fd);

	/*
	 * Read complete data of this section until all
	 * bytes of the section are read
	 */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(uint8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((uint8 *)CurrPtr, NumBytesRem, Fd);
		}
		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (uint8 *)CurrPtr + NumBytesRead;

		XMeSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
				NumBytesRead, NumBytesRem);
	}

	CurrPtr = DataPtr;

	if(strstr(SectName, "data.DMb") != NULL) {
                TgtTileAddr = XMeSim_GetTargetTileAddr(TileInstPtr, 
                					SectPtr->sh_addr);

                SectAddr = SectPtr->sh_addr;
                RemSize = SectPtr->sh_size;
                DoneSize = 0U;

		/* Use 32 bit loads to match sim output */
		for(Idx = 0U; Idx < RemSize; Idx += 4U){
			/* Mask address as per the Data memory offset*/
			DmbOff = (SectAddr &
                                       XMESIM_ELF_TILEADDR_DMB_MASK) + Idx;

                        if(DmbOff & 0x8000U) {
                                /*
                                  * 32 KB boundary cross. Data section 
                                  * moving to the memory bank of next
                                  * cardinal direction. Change the target
                                  * tile address
                                  */
                                SectPtr->sh_addr += 0x8000U;
                                TgtTileAddr =
                                        XMeSim_GetTargetTileAddr(TileInstPtr,
						(SectPtr->sh_addr &
                                                0x1FFFFU));
                                SectAddr = 0x0U;
                                Idx = 0U;
                                DmbOff = 0U;
                                RemSize -= DoneSize;
                        }

                        DmbAddr = TgtTileAddr +
                                        XMESIM_ELF_TILECORE_DATMEM + DmbOff;
			XMeSim_Write32(DmbAddr, *CurrPtr++);
                        DoneSize += 4U;
		}
        } else {
                TgtTileAddr = TileInstPtr->TileAddr+XMESIM_ELF_TILECORE_PRGMEM +
                                                        SectPtr->sh_addr;

                for(Idx = 0U; Idx < SectPtr->sh_size; Idx += 4U) {
        		XMeSim_Write32((TgtTileAddr + Idx), *CurrPtr++);
        	}
        }

	/* Free the allocated memory */
	free(DataPtr);
}

/*****************************************************************************/
/**
*
* This routine is used to get the actual tile data memory address based on the
* section's loadable address. The actual tile address is derived from the 
* cardinal direction the secton's loadable address points to.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ShAddr: Section's loadable address.
*
* @return	32-bit target tile address.
*
* @note		None.
*
*******************************************************************************/
uint64_t XMeSim_GetTargetTileAddr(XMeSim_Tile *TileInstPtr, uint32 ShAddr)
{
	uint64_t TgtTileAddr;
	uint8 CardDir;
	uint8 TgtRow;
	uint8 TgtCol;
        uint8 RowParity;

	/* Find the cardinal direction and get the tile addr */
	CardDir = (ShAddr & XMESIM_ELF_TILEADDR_DMB_CARD_OFF) >>
				XMESIM_ELF_TILEADDR_DMB_CARD_SHIFT;

	TgtRow = TileInstPtr->RowId;
	TgtCol = TileInstPtr->ColId;
        RowParity = TgtRow % 2U;

	switch(CardDir) {
	case 0U:
                /* South */
		if(TgtRow > 0U) {
			TgtRow -= 1U;
		}
		break;
	case 1U:
                /*
                 * West - West I/F could be same tile or adjacent tile based on
                 * the row number
                 */
		if(RowParity == 1U) {
                        /* Adjacent tile */
                        if(TgtCol > 0U) {
			        TgtCol -= 1U;
                        }
		}
		break;
	case 2U:
                /* North */
		TgtRow += 1U;
		break;
	case 3U:
		/*
                 * East - East I/F could be same tile or adjacent tile based on
                 * the row number
                 */
		if(RowParity == 0U) {
                        /* Adjacent tile */
                        TgtCol += 1U;
                }
                break;
	}

        /* Restore orig values if we have exceeded the array boundary limits */
	if(TgtRow > XMeGbl_ConfigTable->NumRows) {
		TgtRow = TileInstPtr->RowId;
	}
	if(TgtCol >= XMeGbl_ConfigTable->NumCols) {
		TgtCol = TileInstPtr->ColId;
	}

	TgtTileAddr = ((TileInstPtr->TileAddr &
                                (~(uint64_t)XMESIM_ELF_TILEBASE_ADDRMASK)) |
				(TgtCol << XMESIM_ELF_TILEADDR_COL_SHIFT) |
				(TgtRow << XMESIM_ELF_TILEADDR_ROW_SHIFT));

	return TgtTileAddr;
}

/** @} */

