/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_decode_ebdcfi.c
 *
 * This file will take uncompressed *.ebd_cfi file as input and generates a
 * structured data in file "xsem_ebdintern.c" for all frames
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hv    04/04/2022  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>

#define MAX_QWORDS		25
/* Global Variables */
static unsigned int EIndex, Count;

/*****************************************************************************/
/**
 * @brief	Main function, decodes essential bit info from ebc_cfi file
 *
 *****************************************************************************/
int main( )
{
    /* Declare the file pointer */
    FILE *InputFilePtr, *OutputFilePtr;

    /* Get the data to be read in file */
    char Data[100], EBitsData[100];
	int Index;
	int BlockType[] = {0, 3, 4, 5};
	/**
	 * Below 2-d array is initialized with last frame address in each row of a block
	 * Here we have initialised for vc1902. The last frame address may vary from device
	 * to device.
	 */
	int LFAddr[4][4] = 	{{0x0000853F, 0x00009662, 0x00009662, 0x00009662},
						{0x0000000B, 0x0000000C, 0x0000000C, 0x0000000C},
						{0x00000005, 0x00000006, 0x00000006, 0x00000006},
						{0x00000001, 0x00000002, 0x00000002, 0x00000002}};

    /**
	 * Open the existing file mypdi.ebc_cfi using fopen()
	 * in read mode using "r" attribute
	 */
    InputFilePtr = fopen("mypdi.ebd_cfi", "r") ;
	OutputFilePtr =  fopen("xsem_ebdintern.c", "w") ;

    /**
	 * Check if this InputFilePtr/OutputFilePtr is null
     * which maybe if the file does not exist
	 */
    if ( InputFilePtr == NULL || OutputFilePtr == NULL)
    {
        printf( "Failed to open files\n" );
    }
    else
    {
		/* Read and ignore first 8 lines of the file */
		for (Index = 0; Index < 8; Index++)
		{
			fgets (Data, sizeof(Data), InputFilePtr);
		}
        printf("The file is now opened.\n") ;
		int QwordIndex, BT_Index, RowIndex;
		long int FrameIndex;
		fprintf(OutputFilePtr, "%s" "%c" "%s" "%c", "#include ", '"', "xsem_ebdintern.h", '"');

		/* Generate xsem_ebdintern.c with structured essential bit data for all frames */
		/* Repeat for all block types */
		for(BT_Index = 0; BT_Index < 4; BT_Index++)
		{
			/* Repeat for all Rows */
			for(RowIndex = 0; RowIndex < 4; RowIndex++)
			{
				fprintf(OutputFilePtr, "%s", "\n");

				/**
				 * Create seperate structure for each row of a block type and
				 * initiailise with essentail bit data of all frames in that
				 * row by reading from *.ebd_cfi
				 */
				fprintf(OutputFilePtr, "%s" "%d" "%s" "%d" "%s" "%d" "%s" "%s", "volatile XSem_EbdSet EbdSetBT", BlockType[BT_Index], "Row", RowIndex, "[",  LFAddr[BT_Index][RowIndex], "]", " =");

				fprintf(OutputFilePtr, "%s", "\n{\n");

				/* Repeat for all Frames */
				for(FrameIndex = 0; FrameIndex < LFAddr[BT_Index][RowIndex]; FrameIndex++)
				{
					fprintf(OutputFilePtr, "%s" "%d" "%s","{", FrameIndex, ",\n{");
					/* Repeat for all Qwords */
					for (QwordIndex = 0; QwordIndex < MAX_QWORDS; QwordIndex++){
						fgets (Data, sizeof(Data), InputFilePtr);
						int WordIndex, i;
						fprintf(OutputFilePtr, "%s", "{");

						for (i = 31; i > 0; i--)
						{
							fprintf(OutputFilePtr, "%s", "0x");
							for (WordIndex = 7; WordIndex >= 0; WordIndex--)
							{
								if(WordIndex == 0){
									if (i > 7) {
										fprintf(OutputFilePtr, "%c" "%s", Data[i - WordIndex], ", ");
									} else {
										fprintf(OutputFilePtr, "%c", Data[i - WordIndex]);
									}
									i = (i - 7);
								} else {
									fprintf(OutputFilePtr, "%c", Data[i - WordIndex]);
								}
							}
						}

						if(QwordIndex == 24){
							fprintf(OutputFilePtr, "%s", "}");
						} else {
							fprintf(OutputFilePtr, "%s", "}, ");
						}
						fprintf(OutputFilePtr, "%s","\n");
					}
					if( FrameIndex == (LFAddr[BT_Index][RowIndex] - 1) ){
						fprintf(OutputFilePtr, "%s", "}}\n\n");
					} else {
						fprintf(OutputFilePtr, "%s", "}},\n\n");
					}
				}
				fprintf(OutputFilePtr, "%s", "};\n\n");
			}
		}
        fclose(InputFilePtr);
		fclose(OutputFilePtr);
        printf("The files are now closed.\n");
    }
    return 0;
}
