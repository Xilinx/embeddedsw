/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_reformat_ebdcfi.c
 *
 * This file reformats the structured essential bit data in xsem_ebdintern.c
 * file by mapping only unique frame data in to XSem_EbdBuffer and generates
 * file "xsem_edbgoldendata.c"
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hv    06/27/2022  Initial Creation
 * 0.2	 anv   05/10/2023  Modified to add copyright information in xsem_ebdgoldendata.c
 * </pre>
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "xsem_ebdintern.h"

/* Macro definitions */
#define MAX_ROWS 4
#define MAX_BLOCK_TYPES 6

/* Global Variables */
extern XSem_EbdSet EbdSetBT0Row0[XSEM_BT0ROW0_FRAMES];
extern XSem_EbdSet EbdSetBT0Row1[XSEM_BT0ROW1_FRAMES];
extern XSem_EbdSet EbdSetBT0Row2[XSEM_BT0ROW2_FRAMES];
extern XSem_EbdSet EbdSetBT0Row3[XSEM_BT0ROW3_FRAMES];
extern XSem_EbdSet EbdSetBT3Row0[XSEM_BT3ROW0_FRAMES];
extern XSem_EbdSet EbdSetBT3Row1[XSEM_BT3ROW1_FRAMES];
extern XSem_EbdSet EbdSetBT3Row2[XSEM_BT3ROW2_FRAMES];
extern XSem_EbdSet EbdSetBT3Row3[XSEM_BT3ROW3_FRAMES];
extern XSem_EbdSet EbdSetBT4Row0[XSEM_BT4ROW0_FRAMES];
extern XSem_EbdSet EbdSetBT4Row1[XSEM_BT4ROW1_FRAMES];
extern XSem_EbdSet EbdSetBT4Row2[XSEM_BT4ROW2_FRAMES];
extern XSem_EbdSet EbdSetBT4Row3[XSEM_BT4ROW3_FRAMES];
extern XSem_EbdSet EbdSetBT5Row0[XSEM_BT5ROW0_FRAMES];
extern XSem_EbdSet EbdSetBT5Row1[XSEM_BT5ROW1_FRAMES];
extern XSem_EbdSet EbdSetBT5Row2[XSEM_BT5ROW2_FRAMES];
extern XSem_EbdSet EbdSetBT5Row3[XSEM_BT5ROW3_FRAMES];

/**
 * input_tables - input tables structure
 */
struct {
    u32 bt;
    u32 row;
    u32 frames;
    XSem_EbdSet *sets;
} input_tables[] = {
    { 0, 0, XSEM_BT0ROW0_FRAMES, EbdSetBT0Row0 },
    { 0, 1, XSEM_BT0ROW1_FRAMES, EbdSetBT0Row1 },
    { 0, 2, XSEM_BT0ROW2_FRAMES, EbdSetBT0Row2 },
    { 0, 3, XSEM_BT0ROW3_FRAMES, EbdSetBT0Row3 },
    { 3, 0, XSEM_BT3ROW0_FRAMES, EbdSetBT3Row0 },
    { 3, 1, XSEM_BT3ROW1_FRAMES, EbdSetBT3Row1 },
    { 3, 2, XSEM_BT3ROW2_FRAMES, EbdSetBT3Row2 },
    { 3, 3, XSEM_BT3ROW3_FRAMES, EbdSetBT3Row3 },
    { 4, 0, XSEM_BT4ROW0_FRAMES, EbdSetBT4Row0 },
    { 4, 1, XSEM_BT4ROW1_FRAMES, EbdSetBT4Row1 },
    { 4, 2, XSEM_BT4ROW2_FRAMES, EbdSetBT4Row2 },
    { 4, 3, XSEM_BT4ROW3_FRAMES, EbdSetBT4Row3 },
    { 5, 0, XSEM_BT5ROW0_FRAMES, EbdSetBT5Row0 },
    { 5, 1, XSEM_BT5ROW1_FRAMES, EbdSetBT5Row1 },
    { 5, 2, XSEM_BT5ROW2_FRAMES, EbdSetBT5Row2 },
    { 5, 3, XSEM_BT5ROW3_FRAMES, EbdSetBT5Row3 },
};

/**
 * EbDb - EBD buffer structure
 */
struct EbDb {
    u32 frames;
    u32 offset;
} ebdb[MAX_BLOCK_TYPES][MAX_ROWS];

/**
 * EbdMask - EBD Mask structure
 */
typedef struct EbdMask {
    u32 words[25][4];
} EbdMask;

EbdMask * mask_list = (void *)0;
u32 mask_index = 0;
u32 mask_capacity = 0;

u32 * map_list = (void *)0;
u32 map_index = 0;
u32 map_capacity = 0;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function finds and creates unique frame masks of all frames
 *
 * @param	set[In]	: Set of Essential bit data of each row of a blocktype
 *
 * @return	i : Buffer Index
 *****************************************************************************/
static u32 find_or_create_unique_output_mask(XSem_EbdSet *set) {
    u32 i;
    for (i = 0; i < mask_index; i++) {
        if (memcmp(mask_list[i].words, set->words, sizeof set->words) == 0) break;
    }
    if (i == mask_index) {
        if (i == mask_capacity) {
            mask_capacity = mask_capacity ? mask_capacity * 2 : 1;
            mask_list = realloc(mask_list, mask_capacity * sizeof *mask_list);
            if (mask_list == (void *)0) {
                perror("malloc");
                exit(1);
            }
        }
        memcpy(mask_list[mask_index].words, set->words, sizeof set->words);
        mask_index++;
    }
    return i;
}

/*****************************************************************************/
/**
 * @brief	This function generates xsem_ebdgoldendata.c file with
 * 	XSem_EbdBuffer loaded with frame map offset, frame maps and unique frame
 *	masks.
 *
 * @param	OutFptr[In]	:	file pointer
 *		BlockTypes[In]	:	Block type index
 *		Rows[In]	:	Row index
 *****************************************************************************/
void print_ebdb(FILE * OutFptr, u32 BlockTypes, u32 Rows) {
    u32 bt;
    u32 row;
    u32 hdr_offset = 0;
    u32 index_offset = hdr_offset + 2;
    u32 map_offset = index_offset + BlockTypes*Rows*2;
    u32 mask_offset = map_offset + map_index;
    u32 i;

    fprintf(OutFptr,"/* Total frames: %u */\n", map_index);
    fprintf(OutFptr,"/* Unique frames: %u */\n", mask_index);
    fprintf(OutFptr,"volatile int XSem_EbdBuffer[] ");
	fprintf(OutFptr, "%s" "%c" "%s" "%c" "%s", " __attribute__((section (", '"', ".sem_ebddata", '"', "))) __attribute__((no_reorder)) = {\n");
    fprintf(OutFptr,"  /* %u: Header */\n", hdr_offset);
    fprintf(OutFptr,"  %u, /* Number of block types */\n", BlockTypes);
    fprintf(OutFptr,"  %u, /* Number of rows */\n", Rows);
    fprintf(OutFptr,"\n");
    fprintf(OutFptr,"  /* %u: (Block type, row index) -> (frame count, frame map offset) */\n", index_offset);
    for (bt = 0; bt < BlockTypes; bt++) {
        for (row = 0; row < Rows; row++) {
            u32 frames = ebdb[bt][row].frames;
            u32 offset = frames ? ebdb[bt][row].offset + map_offset : 0;
            fprintf(OutFptr,"  %6u, %6u, /* BT %u Row %u (frames, offset) */\n",
                   frames, offset, bt, row);
        }
    }
    fprintf(OutFptr,"\n");
    fprintf(OutFptr,"  /* Frame maps */\n", map_offset);
    for (i = 0; i < map_index; i++) {
        if ((i % 8) == 0) {
            if (i != 0) fprintf(OutFptr,"\n");
            fprintf(OutFptr,"  /* %6u */", i + map_offset);
        }
        fprintf(OutFptr," %6u,", map_list[i]*25*4 + mask_offset);
    }
    fprintf(OutFptr,"\n");

    fprintf(OutFptr,"\n");
    fprintf(OutFptr,"  /* Frame masks */\n");
    for (i = 0; i < mask_index; i++) {
        u32 j;
        fprintf(OutFptr,"  /* %6u */\n", i*25*4 + mask_offset);
        for (j = 0; j < 25; j++) {
            u32 k;
            fprintf(OutFptr,"  ");
            for (k = 0; k < 4; k++) {
                fprintf(OutFptr," 0x%08x,", mask_list[i].words[j][k]);
            }
            fprintf(OutFptr,"\n");
        }
    }

    fprintf(OutFptr,"};\n");
}

/*****************************************************************************/
/**
 * @brief	Main function, reformats essential bit info from xsem_ebdintern.c
 *
 *****************************************************************************/
int main(int argc, char **argv) {
    u32 block_types = 0;
    u32 rows = 0;
    u32 i;
	FILE *fptr;
	fptr = fopen("xsem_ebdgoldendata.c", "w");
	fprintf(fptr, "%s", "/******************************************************************************\n* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.\n* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.\n* SPDX-License-Identifier: MIT\n******************************************************************************/\n\n");
	fprintf(fptr, "%s", "#if defined (__ICCARM__) \n#pragma language=save \n#pragma language=extended \n#endif\n\n");
    for (i = 0; i < sizeof input_tables/sizeof *input_tables; i++) {
        u32 bt = input_tables[i].bt;
        u32 row = input_tables[i].row;
        u32 frames = input_tables[i].frames;
        XSem_EbdSet *set = input_tables[i].sets;
        if (block_types <= bt) block_types = bt + 1;
        if (rows <= row) rows = row + 1;
        ebdb[bt][row].frames = frames;
        ebdb[bt][row].offset = map_index;
        if (map_index + frames > map_capacity) {
            do {
                map_capacity = map_capacity ? map_capacity * 2 : 1;
            } while(map_index + frames > map_capacity);
            map_list = realloc(map_list, map_capacity * sizeof *map_list);
        }
        while (frames > 0) {
            map_list[map_index++] = find_or_create_unique_output_mask(set);
            set++;
            frames--;
        }
    }
    print_ebdb(fptr, block_types, rows);
	fclose(fptr);
	return 0;
}
