This directory contains the following files:
readme.txt:		This file
xilmfs.h:		Header file for using Memory File System
mfs_filesys.c:		C Source code for Memory File System

mfs_filesys_util.c:	Additional functions to support the Memory File System - use if needed


Usage of Memory File System:
1. Read/write file system on RAM in a MicroBlaze based platform
	a. Compile mfs_filesys.c along with your application code. mfs_filesys.c include xilmfs.h
	b. Invoke mfs_init(NUM_BYTES, BASE_ADDRESS, MFSINIT_NEW) within your application 
		before calling any MFS functions.
	 - NUM_BYTES is the number of bytes allocated to the MFS
	 - BASE_ADDRESS is the starting address of the memory block allocated to MFS 
		- this can be obtained by calling a memory allocator such as malloc() 
		  or by directly reserving this in the system MHS file
	 - MFSINIT_NEW is a constant defined in xilmfs.h
	 - NUM_BYTES and BASE_ADDRESS can optionally be generated through libgen 
		- in which case they are placed in the mfs_config.h file and
                - the mfs_filesys functions are part of the standard libraries
	

2. Read-only file system on SRAM/Flash or pre-initialized file read/write system on SRAM
	- See the readme.txt file in the utils directory for more info
	
