This directory contains the following files:
readme.txt:		This file

flash.tcl:		TCL script to be used in conjuction with xmd to download MFS image to 
			flash. Warning: The script has to be modified depending on the board,
			flash memory type, etc.


test_mfs_filesys.c:	Simple test case that can be natively compiled with the files 
			in the src directory to test the MFS library

testmfs.c:
testmfsrom.c:
testmfsflashrom.c:	Simple test case that loads  a preconfigured MFS file 
			(created using mfsgen) to SRAM, ROM, and FlashROM 
			and then accesses the file system
			The testcases are written for a V2MB1000 development board and 
			may need modification to use  on other boards


Usage of pre configured Memory File System with mfsgen:


1. Creating a Read-only file system on Flash or pre-initialized file read/write system on flash/SRAM
	a. Create a directory on the host machine file system that contains 
	   all the files/directories to be loaded on the MFS
	b. Run mfsgen to create mfs image on disk
		- mfsgen -cvf filesys.mfs list_of_files_to_put_on_MFS
	c. Download mfs image filesys.mfs to flash or RAM as described below.

The TCL script flash.tcl, writes the memory image to Flash Memory. 
The Flash writer supports Toshiba TH50VSF2580/2581AASB and TH50VSF3680/3681AASB flash. Downloading the memory image to flash.
	The following steps needs to be followed to download the memory image to flash.				
        a. Run xmd
	b. mbconnect stub -posit 2 
		- Connect to Microblaze 
		** position param depends on particular board being used

	c. (FOR SRAM)
	   xdownload 0 -data filesystem.mfs 0xffe00000
		- download data file to a specific SRAM address
		** SRAM address depends on board and platform configuration

	c. (FOR FLASH)
	   source flash.tcl 
		- load the flash TCL script
	d. flash_init 0x1f800000 0 
		- Create and initialize the flash target. 
		  Specify the flash start address (0x1f800000) and existing xmd target (0).
		*** Depends on the address assinged to flash memory in your system
	e. flash_data_dow  filesys.mfs 0x1f800000
		- Download the mfs image file called filesys.mfs from host to flash ROM

2. Notes
 * It is possible to verify the correctness of an MFS file image by extracting all the files 
   in a different area and comparing with the original:
  	a. Create the mfs image:
      		mfsgen -cvf tmp.mfs original.elf
  	b. Extract the file in a different area:
      		mkdir tmp; cd tmp; cp ../tmp.mfs .; ../mfsgen -xvf tmp.mfs
  	c. Compare the two files:
      		diff original.elf ../original.elf

3. Error Handling and Optimization

  When mfsgen successfully creates an MFS image it issues diagnostics of the form:
	MFS block usage (used / free / total) = 75 / 4925 / 5000
  	Size of memory is 2660000 bytes
  	Block size is 532
  The first line says that 75 out of 5000 blocks are used for the image so NUM_BLOCKS can be 
  reduced from 5000 to 75 for a read-only image, and 75+desired_free_blocks for a read-write image.
  The second line says the memory size is 2660000 bytes. The actual size of the ROM/RAM/Flash  
  should be at least this much, in the target system.
 

