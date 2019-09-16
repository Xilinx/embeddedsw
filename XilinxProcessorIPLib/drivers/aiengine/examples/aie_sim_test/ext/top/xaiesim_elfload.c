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

/*****************************************************************************/
/**
* @file xaiesim_elfload.c
* @{
*
* This file contains the API for ELF loading. Applicable only for the
* AIE simulation environment execution on linux.
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
* 1.6  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.7  Hyun    09/13/2019  Used global IO accessors and added more __AIESIM__
* 1.8  Hyun    09/13/2019  Added XAieSim_LoadElfMem()
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "xaiesim.h"
#include "xaiesim_elfload.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_Config XAieGbl_ConfigTable[];

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This routine is used to write to the specified section by reading the
* corresponding data from the ELF buffer.
*
* @param	TileInstPtr: Pointer to the Tile instance structure.
* @param        SectName: Name of the section.
* @param        SectPtr: Poiner to the section entry in the ELF buffer.
* @param	ElfPtr: Pointer to the ELF buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
static void XAieSim_WriteSectionMem(XAieSim_Tile *TileInstPtr, uint8 *SectName,
		Elf32_Shdr *SectPtr, uint8 *ElfPtr)
{
	uint32 Idx;
	uint32 DmbOff;
	uint32 SectAddr;
	uint32 RemSize;
	uint32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;

	if(strstr(SectName, "data.DMb") != NULL) {
		TgtTileAddr = XAieSim_GetTargetTileAddr(TileInstPtr,
				SectPtr->sh_addr);

		SectAddr = SectPtr->sh_addr;
		RemSize = SectPtr->sh_size;
		DoneSize = 0U;

		/* Use 32 bit loads to match sim output */
		for(Idx = 0U; Idx < RemSize; Idx += 4U){
			/* Mask address as per the Data memory offset*/
			DmbOff = (SectAddr &
					XAIESIM_ELF_TILEADDR_DMB_MASK) + Idx;

			if(DmbOff & 0x8000U) {
				/*
				 * 32 KB boundary cross. Data section
				 * moving to the memory bank of next
				 * cardinal direction. Change the target
				 * tile address
				 */
				SectPtr->sh_addr += 0x8000U;
				TgtTileAddr =
					XAieSim_GetTargetTileAddr(TileInstPtr,
							(SectPtr->sh_addr &
							 0x1FFFFU));
				SectAddr = 0x0U;
				Idx = 0U;
				DmbOff = 0U;
				RemSize -= DoneSize;
			}

			DmbAddr = TgtTileAddr +
				XAIESIM_ELF_TILECORE_DATMEM + DmbOff;
			XAieGbl_Write32(DmbAddr, *ElfPtr++);
			DoneSize += 4U;
		}
	} else {
		TgtTileAddr = TileInstPtr->TileAddr+XAIESIM_ELF_TILECORE_PRGMEM +
			SectPtr->sh_addr;

		for(Idx = 0U; Idx < SectPtr->sh_size; Idx += 4U) {
			XAieGbl_Write32((TgtTileAddr + Idx), *ElfPtr++);
		}
	}
}

/*****************************************************************************/
/**
*
* This is the API to load the specified ELF to the target AIE Tile program
* memory followed by clearing of the BSS sections.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ElfPtr: Path to the ELF memory
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_LoadElfMem(XAieSim_Tile *TileInstPtr, uint8 *ElfPtr,
		uint8 LoadSym)
{
	Elf32_Ehdr *ElfHdr = ElfPtr;
	Elf32_Shdr *SectHdr;

	uint8 Count = 0U;
	uint8 ShNameIdx = 0U;

	uint32 Idx;
	uint8 *DataPtr;
	uint32 DmbOff;
	uint32 SectAddr;
	uint32 RemSize;
	uint32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;

	XAieSim_print("**** ELF HEADER ****\n");
	XAieSim_print("e_type\t\t : %08x\ne_machine\t : %08x\ne_version\t : "
			"%08x\ne_entry\t\t : %08x\ne_phoff\t\t : %08x\n",
			ElfHdr->e_type, ElfHdr->e_machine, ElfHdr->e_version,
			ElfHdr->e_entry, ElfHdr->e_phoff);
	XAieSim_print("e_shoff\t\t : ""%08x\ne_flags\t\t : %08x\ne_ehsize\t"
			" : %08x\ne_phentsize\t : %08x\ne_phnum\t\t : %08x\n"
			"e_shentsize\t : %08x\ne_shnum\t\t : %08x\ne_shstrndx\t : "
			"%08x\n\n", ElfHdr->e_shoff, ElfHdr->e_flags,
			ElfHdr->e_ehsize, ElfHdr->e_phentsize, ElfHdr->e_phnum,
			ElfHdr->e_shentsize, ElfHdr->e_shnum, ElfHdr->e_shstrndx);

	SectHdr = ElfPtr + ElfHdr->e_shoff;

	/*
	 * Get the section names from the .shstrtab section. First find the
	 * index of this section among the section header entries
	 */
	while(Count < ElfHdr->e_shnum) {
		if(SectHdr[Count].sh_type == SHT_STRTAB) {
			ShNameIdx = Count;
			break;
		}
		Count++;
	}

	/* Now point to the .shstrtab section in the ELF file */
	DataPtr = ElfPtr + SectHdr[ShNameIdx].sh_offset;

	XAieSim_print("**** SECTION HEADERS ****\n");
	XAieSim_print("Idx\tsh_name\t\tsh_type\t\tsh_flags\tsh_addr\t\tsh_offset"
			"\tsh_size\t\tsh_link\t\tsh_addralign\tsection_name\n");
	Count = 0U;
	while(Count < ElfHdr->e_shnum) {
		XAieSim_print("%d\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t"
				"%08x\t%s\n", Count, SectHdr[Count].sh_name,
				SectHdr[Count].sh_type, SectHdr[Count].sh_flags,
				SectHdr[Count].sh_addr, SectHdr[Count].sh_offset,
				SectHdr[Count].sh_size, SectHdr[Count].sh_link,
				SectHdr[Count].sh_addralign,
				DataPtr + SectHdr[Count].sh_name);
		Count++;
	}

	Count = 0U;
	while(Count < ElfHdr->e_shnum) {
		/* Copy the program data sections to memory */
		if((SectHdr[Count].sh_type == SHT_PROGBITS) &&
				((SectHdr[Count].sh_flags &
				  (SHF_ALLOC | SHF_EXECINSTR)) ||
				 (SectHdr[Count].sh_flags &
				  (SHF_ALLOC | SHF_WRITE)))) {

			XAieSim_WriteSectionMem(TileInstPtr,
					DataPtr + SectHdr[Count].sh_name,
					&SectHdr[Count],
					ElfPtr + SectHdr[Count].sh_offset);
		}

		/* Zero out the bss sections */
		if((SectHdr[Count].sh_type == SHT_NOBITS) &&
				(strstr(DataPtr + SectHdr[Count].sh_name,
					"bss.DMb") != NULL)) {
			XAieSim_print("Zeroing out the bss sections\n");

			TgtTileAddr = XAieSim_GetTargetTileAddr(TileInstPtr,
					SectHdr[Count].sh_addr);
			SectAddr = SectHdr[Count].sh_addr;
			/*
			 * Use the section header size. The AIE compiler uses
			 * the section header for load size.
			 */
			RemSize = SectHdr[Count].sh_size;
			DoneSize = 0U;

			/* Use 32 bit loads to match sim output */
			for(Idx = 0U; Idx < RemSize; Idx += 4U){
				/* Mask address as per the Data memory offset*/
				DmbOff = (SectAddr &
						XAIESIM_ELF_TILEADDR_DMB_MASK) + Idx;

				if(DmbOff & 0x8000U) {
					/*
					 * 32 KB boundary cross. Data section
					 * moving to the memory bank of next
					 * cardinal direction. Change the target
					 * tile address
					 */
					SectHdr[Count].sh_addr += 0x8000U;
					TgtTileAddr =
						XAieSim_GetTargetTileAddr(TileInstPtr,
								(SectHdr[Count].sh_addr &
								 0x1FFFFU));
					SectAddr = 0x0U;
					Idx = 0U;
					DmbOff = 0U;
					RemSize -= DoneSize;
					DoneSize = 0U;
				}

				DmbAddr = TgtTileAddr +
					XAIESIM_ELF_TILECORE_DATMEM + DmbOff;
				XAieGbl_Write32(DmbAddr, 0U);
				DoneSize += 4U;
			}
		}
		Count++;
	}

	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the API to load the specified ELF to the target AIE Tile program
* memory followed by clearing of the BSS sections.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ElfPtr: Path to the ELF file to be loaded into memory.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_LoadElf(XAieSim_Tile *TileInstPtr, uint8 *ElfPtr, uint8 LoadSym)
{
	FILE *Fd;
	Elf32_Ehdr ElfHdr;
	Elf32_Shdr SectHdr[XAIESIM_ELF_SECTION_NUMMAX];
	XAieSim_StackSz StackSz;

	uint8 MapPath[256U];
	uint8 Count = 0U;
	uint8 ShNameIdx = 0U;
	uint8 ShName[XAIESIM_ELF_SECTION_NUMMAX][XAIESIM_ELF_SECTION_NAMEMAXLEN];

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

#ifdef __AIESIM__
	/* Get the stack range */
	strcpy(MapPath, ElfPtr);
	strcat(MapPath, ".map");
	Status = XAieSim_GetStackRange(MapPath, &StackSz);
	XAieSim_print("Stack start:%08x, end:%08x\n",
					StackSz.start, StackSz.end);
	if(Status != XAIESIM_SUCCESS) {
		XAieSim_print("ERROR: Stack range definition failed\n");
		return Status;
	}

	/* Send the stack range set command */
	XAieSim_WriteCmd(XAIESIM_CMDIO_CMD_SETSTACK, TileInstPtr->ColId,
		TileInstPtr->RowId, StackSz.start, StackSz.end, XAIE_NULL);

	/* Load symbols if enabled */
	if(LoadSym == XAIE_ENABLE) {
		XAieSim_LoadSymbols(TileInstPtr, ElfPtr);
	}
#endif

	/* Open the ELF file for reading */
	Fd = fopen(ElfPtr, "r");
	if(Fd == NULL) {
		XAieSim_print("ERROR: Invalid ELF file\n");
		return XAIESIM_FAILURE;
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

		XAieSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
						NumBytesRead, NumBytesRem);
	}

	XAieSim_print("**** ELF HEADER ****\n");
	XAieSim_print("e_type\t\t : %08x\ne_machine\t : %08x\ne_version\t : "
		"%08x\ne_entry\t\t : %08x\ne_phoff\t\t : %08x\n",
		ElfHdr.e_type, ElfHdr.e_machine, ElfHdr.e_version,
		ElfHdr.e_entry, ElfHdr.e_phoff);
	XAieSim_print("e_shoff\t\t : ""%08x\ne_flags\t\t : %08x\ne_ehsize\t"
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

		XAieSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
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
		fgets(ShName[Count], XAIESIM_ELF_SECTION_NAMEMAXLEN, Fd);

		/*
		 * Again point to the start of the section .shstrtab for the
		 * next section to get its name from its corresponding offset
		 */
		fseek(Fd, SectHdr[ShNameIdx].sh_offset, SEEK_SET);
		Count++;
	}

	XAieSim_print("**** SECTION HEADERS ****\n");
	XAieSim_print("Idx\tsh_name\t\tsh_type\t\tsh_flags\tsh_addr\t\tsh_offset"
		"\tsh_size\t\tsh_link\t\tsh_addralign\tsection_name\n");
	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		XAieSim_print("%d\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t"
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
			
			XAieSim_WriteSection(TileInstPtr, ShName[Count],
							&SectHdr[Count], Fd);
		}

		/* Zero out the bss sections */
		if((SectHdr[Count].sh_type == SHT_NOBITS) &&
				(strstr(ShName[Count], "bss.DMb") != NULL)) {
			XAieSim_print("Zeroing out the bss sections\n");

			TgtTileAddr = XAieSim_GetTargetTileAddr(TileInstPtr,
						SectHdr[Count].sh_addr);
                        SectAddr = SectHdr[Count].sh_addr;
                        RemSize = SectHdr[Count].sh_size;
                        DoneSize = 0U;

			/* Use 32 bit loads to match sim output */
			for(Idx = 0U; Idx < RemSize; Idx += 4U){
				/* Mask address as per the Data memory offset*/
				DmbOff = (SectAddr &
                                        XAIESIM_ELF_TILEADDR_DMB_MASK) + Idx;

                                if(DmbOff & 0x8000U) {
                                        /*
                                         * 32 KB boundary cross. Data section 
                                         * moving to the memory bank of next
                                         * cardinal direction. Change the target
                                         * tile address
                                         */
                                        SectHdr[Count].sh_addr += 0x8000U;
                                        TgtTileAddr =
                                        XAieSim_GetTargetTileAddr(TileInstPtr,
						(SectHdr[Count].sh_addr &
                                                0x1FFFFU));
                                        SectAddr = 0x0U;
                                        Idx = 0U;
                                        DmbOff = 0U;
                                        RemSize -= DoneSize;
                                        DoneSize = 0U;
                                }

                                DmbAddr = TgtTileAddr +
                                        XAIESIM_ELF_TILECORE_DATMEM + DmbOff;
				XAieGbl_Write32(DmbAddr, 0U);
                                DoneSize += 4U;
			}
		}
		Count++;
	}
	fclose(Fd);

	return XAIESIM_SUCCESS;
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
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_GetStackRange(uint8 *MapPtr, XAieSim_StackSz *StackSzPtr)
{
#ifdef __AIESIM__
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
		XAieSim_print("ERROR: Invalid Map file\n");
		return XAIESIM_FAILURE;
	}

	while(fgets(buffer, 200U, Fd) != NULL) {
		if(strstr(buffer, "items) : Stack") != NULL) {
			sscanf(buffer, "    0x%8x..0x%8x (%*s",
					&StackSzPtr->start, &StackSzPtr->end);
			break;
		}
	}

	if(StackSzPtr->start == 0xFFFFFFFFU) {
		return XAIESIM_FAILURE;
	} else {
		return XAIESIM_SUCCESS;
	}
	fclose(Fd);
#endif
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
void XAieSim_LoadSymbols(XAieSim_Tile *TileInstPtr, uint8 *ElfPtr)
{
#ifdef __AIESIM__
	XAieSim_WriteCmd(XAIESIM_CMDIO_CMD_LOADSYM, TileInstPtr->ColId,
					TileInstPtr->RowId, 0, 0, ElfPtr);
#endif
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
void XAieSim_WriteSection(XAieSim_Tile *TileInstPtr, uint8 *SectName,
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

		XAieSim_print("NumBytesRead : %d, NumBytesRem : %d\n",
				NumBytesRead, NumBytesRem);
	}

	CurrPtr = DataPtr;

	if(strstr(SectName, "data.DMb") != NULL) {
                TgtTileAddr = XAieSim_GetTargetTileAddr(TileInstPtr, 
                					SectPtr->sh_addr);

                SectAddr = SectPtr->sh_addr;
                RemSize = SectPtr->sh_size;
                DoneSize = 0U;

		/* Use 32 bit loads to match sim output */
		for(Idx = 0U; Idx < RemSize; Idx += 4U){
			/* Mask address as per the Data memory offset*/
			DmbOff = (SectAddr &
                                       XAIESIM_ELF_TILEADDR_DMB_MASK) + Idx;

                        if(DmbOff & 0x8000U) {
                                /*
                                  * 32 KB boundary cross. Data section 
                                  * moving to the memory bank of next
                                  * cardinal direction. Change the target
                                  * tile address
                                  */
                                SectPtr->sh_addr += 0x8000U;
                                TgtTileAddr =
                                        XAieSim_GetTargetTileAddr(TileInstPtr,
						(SectPtr->sh_addr &
                                                0x1FFFFU));
                                SectAddr = 0x0U;
                                Idx = 0U;
                                DmbOff = 0U;
                                RemSize -= DoneSize;
                        }

                        DmbAddr = TgtTileAddr +
                                        XAIESIM_ELF_TILECORE_DATMEM + DmbOff;
			XAieGbl_Write32(DmbAddr, *CurrPtr++);
                        DoneSize += 4U;
		}
        } else {
                TgtTileAddr = TileInstPtr->TileAddr+XAIESIM_ELF_TILECORE_PRGMEM +
                                                        SectPtr->sh_addr;

                for(Idx = 0U; Idx < SectPtr->sh_size; Idx += 4U) {
			XAieGbl_Write32((TgtTileAddr + Idx), *CurrPtr++);
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
uint64_t XAieSim_GetTargetTileAddr(XAieSim_Tile *TileInstPtr, uint32 ShAddr)
{
	uint64_t TgtTileAddr;
	uint8 CardDir;
	uint8 TgtRow;
	uint8 TgtCol;
        uint8 RowParity;

	/* Find the cardinal direction and get the tile addr */
	CardDir = (ShAddr & XAIESIM_ELF_TILEADDR_DMB_CARD_OFF) >>
				XAIESIM_ELF_TILEADDR_DMB_CARD_SHIFT;

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
	if(TgtRow > XAieGbl_ConfigTable->NumRows) {
		TgtRow = TileInstPtr->RowId;
	}
	if(TgtCol >= XAieGbl_ConfigTable->NumCols) {
		TgtCol = TileInstPtr->ColId;
	}

	TgtTileAddr = ((TileInstPtr->TileAddr &
                                (~(uint64_t)XAIESIM_ELF_TILEBASE_ADDRMASK)) |
				(TgtCol << XAIESIM_ELF_TILEADDR_COL_SHIFT) |
				(TgtRow << XAIESIM_ELF_TILEADDR_ROW_SHIFT));

	return TgtTileAddr;
}

/** @} */

