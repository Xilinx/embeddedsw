/////////////////////////////////////////////////////////////////////////-*-C-*- 
//
// Copyright (c) 2002, 2003 Xilinx, Inc.  All rights reserved.
//
// Xilinx, Inc.
//
// XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
// STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.  
// XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Description : 
// Test program to init and access a read only file system that has
// been preloaded into SRAM
// 
//          mb-gcc $OPTIONS testmfsrom.c  mfs_filesys.c mfs_filesys_util.c -o testmfsrom
//
// $Id: testmfsrom.c,v 1.1.16.7 2010/10/01 18:53:25 jece Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "xilmfs.h"

char *fs = (char *)0xffe00000; /* base address of SRAM */
/* NUMBLOCKS must be the same number that is used in mfsgen when creating the
   MFS image that is pre-loaded into RAM */
/* for big file system */
#define NUMBLOCKS 200
/* for small file system */
/* #define NUMBLOCKS 10 */
int main(int argc, char *argv[]) {

  int numbytes;
  
  numbytes = NUMBLOCKS *sizeof(struct mfs_file_block);

  mfs_init_fs(numbytes, fs, MFSINIT_ROM_IMAGE);
 
  mfs_ls_r(-1);
  mfs_cat("xilmfs.h"); /* assuming there is a file called xilmfs.h in the pre-loaded file system */

  return 0;
}


 
