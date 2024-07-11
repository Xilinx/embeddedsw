/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xilffs_polled_ufs_example.c
*
*
* @note This example uses file system with UFS to write to and read from
* an UFS device in polled mode.
* To test this example File System should not be in Read Only mode.
* To test this example USE_MKFS option should be true.
*
* This example was tested using UFS device.
*
* To test with different logical drives, drive number should be mentioned in
* both FileName and Path variables. By default, it will take drive 0 if drive
* number is not mentioned in the FileName variable.
* For example, to test logical drive 2
* FileName =  "2:/<file_name>" and Path = "2:/"
* Similarly to test logical drive N, FileName = "N:/<file_name>" and
* Path = "N:/"
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   sk  07/11/24 First release
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/


#include "xparameters.h"	/* SDK generated parameters */
#include "xil_printf.h"
#include "ff.h"
#include "xil_cache.h"
#include "xplatform_info.h"
#include "xufspsxc.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FfsUfsPolledExample(void);

/************************** Variable Definitions *****************************/
static FIL fil;		/* File object */
static FATFS fatfs;

/*
 * TEST PROCEDURE:
 *
 * To test logical drive 0, FileName should be "0:/<File name>" or
 * "<file_name>". For logical drive 1, FileName should be "1:/<file_name>"
 *
 * ****************************************************************************
 * SD0/SD1 Interface
 * ****************************************************************************
 * 	Multi-Partition Support enabled:
 * 		=> User need to use Logical drive numbers 0 to 9.
 * 			- To access partition [Y] in SD[X] -> Logical drive number = (X*5) + Y.
 * 				For example to access SD0 1st partition, Log drv number = 0 + 1 = 1.
 *
 * 	Multi-partition support disabled:
 * 		=> User need to use Logical drive numbers 0 to 1.
 * 			- To access SD0 -> Logical drive number is 0
 * 			- To access SD1 -> Logical drive number is 1
 *
 * ****************************************************************************
 * UFS Interface
 * ****************************************************************************
 * 	Multi-Partition Support enabled:
 * 		=> User need to use Logical drive numbers 10 to 174.
 * 			=> 15 to 174 used for direct LUN access.
 * 				- To access partition [Y] in LUN[X] -> Logical drive number = 15 + (X*5) + Y.
 * 					For example to access LUN2 1st partition, Log drv number = 15 + 10 + 1 = 26.
 * 			=> 10 to 14 used for Boot-LUN access.
 * 				- To access partition [Y] in B-LUN -> Logical drive number = 10 + Y.
 *
 * 	Multi-partition support disabled:
 * 		=> User need to use Logical drive numbers 2 to 34.
 * 			=> 3 to 34 used for direct LUN access.
 * 				- To access LUN[X] -> Logical drive number = X + 3.
 * 			=> 2 used for direct Boot-LUN access.
 *
 */
static char FileName[32] = "2:/BOOT.BIN";
static char *UFS_File;

#ifdef __ICCARM__
#pragma data_alignment = 32
u8 DestinationAddress[10*1024];
#pragma data_alignment = 32
u8 SourceAddress[10*1024];
#else
u8 DestinationAddress[50*1024] __attribute__ ((aligned(32)));
u8 SourceAddress[50*1024] __attribute__ ((aligned(32)));
#endif

/* Macros to get the Logical drive numbers for SD/eMMC/UFS */
#if FF_MULTI_PARTITION
#define SD_DRV_NUM(SDPdrv, Partition)	((SDPdrv * 5) + Partition)	/* SD/eMMC */
#define UFS_DRV_NUM(LUN, Partition)		(15 + (LUN * 5) + Partition)	/* UFS LUN access */
#define UFS_BLUN_DRV_NUM(Partition)		(10 + Partition)	/* UFS Boot LUN access */
#else
#define SD_DRV_NUM(SDPdrv, Partition)	(SDPdrv)	/* SD/eMMC */
#define UFS_DRV_NUM(LUN, Partition)		(LUN + 3)	/* UFS LUN access */
#define UFS_BLUN_DRV_NUM(Partition)		2		/* UFS Boot LUN access */
#endif

#define ENABLE_SWITCH_BLUN 0

#define TEST 7
MKFS_PARM mkfs_parm;

/*****************************************************************************/
/**
*
* Main function to call the UFS example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("UFS Polled File System Example Test \r\n");

	Status = FfsUfsPolledExample();
	if (Status != XST_SUCCESS) {
		xil_printf("UFS Polled File System Example Test failed \r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UFS Polled File System Example Test \r\n");

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* File system example using UFS driver to write to and read from an UFS card
* in polled mode.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int FfsUfsPolledExample(void)
{
	FRESULT Res;
	UINT NumBytesRead;
	UINT NumBytesWritten;
	u32 BuffCnt;
	BYTE work[FF_MAX_SS];
	u32 FileSize = (30 * 1024);
	u32 PowerMode;

	/*
	 * To test logical drive 0, Path should be "0:/"
	 * For logical drive 1, Path should be "1:/"
	 * Please refer the "TEST PROCEDURE" mentioned above.
	 */
	TCHAR *Path = "2:/";

	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		SourceAddress[BuffCnt] = TEST + BuffCnt;
		DestinationAddress[BuffCnt] = 0x0;
	}

	/*
	 * Register volume work area, initialize device
	 */
	Res = f_mount(&fatfs, Path, 0);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	 mkfs_parm.fmt = FM_FAT32;
	/*
	 * Path - Path to logical driver, 0 - FDISK format.
	 * 0 - Cluster size is automatically determined based on Vol size.
	 */
	Res = f_mkfs(Path, &mkfs_parm , work, sizeof work);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	PowerMode = XUFSPSXC_PWM_G1;
	Res = f_ioctl(Path, XUFSPSXC_SPEED_CHANGE, &PowerMode);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	/*
	 * Open file with required permissions.
	 * Here - Creating new file with read/write permissions. .
	 * To open file with write permissions, file system should not
	 * be in Read Only mode.
	 */
	UFS_File = (char *)FileName;

	Res = f_open(&fil, UFS_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}
	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Write data to file.
	 */
	Res = f_write(&fil, (const void*)SourceAddress, FileSize,
			&NumBytesWritten);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Read data from file.
	 */
	Res = f_read(&fil, (void*)DestinationAddress, FileSize,
			&NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}

	/*
	 * Close file.
	 */
	Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}

#if ENABLE_SWITCH_BLUN

	Res = f_ioctl(Path, XUFSPSXC_SWITCH_BLUN, NULL);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	Res = f_mount(&fatfs, Path, 0);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	mkfs_parm.fmt = FM_FAT32;
	/*
	 * Path - Path to logical driver, 0 - FDISK format.
	 * 0 - Cluster size is automatically determined based on Vol size.
	 */
	Res = f_mkfs(Path, &mkfs_parm , work, sizeof work);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}

	/*
	 * Open file with required permissions.
	 * Here - Creating new file with read/write permissions. .
	 * To open file with write permissions, file system should not
	 * be in Read Only mode.
	 */
	UFS_File = (char *)FileName;

	Res = f_open(&fil, UFS_File, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Write data to file.
	 */
	Res = f_write(&fil, (const void*)SourceAddress, FileSize,
			&NumBytesWritten);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Pointer to beginning of file .
	 */
	Res = f_lseek(&fil, 0);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Read data from file.
	 */
	Res = f_read(&fil, (void*)DestinationAddress, FileSize,
			&NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}

	/*
	 * Data verification
	 */
	for(BuffCnt = 0; BuffCnt < FileSize; BuffCnt++){
		if(SourceAddress[BuffCnt] != DestinationAddress[BuffCnt]){
			return XST_FAILURE;
		}
	}

	/*
	 * Close file.
	 */
	Res = f_close(&fil);
	if (Res) {
		return XST_FAILURE;
	}
#endif

	return XST_SUCCESS;
}
