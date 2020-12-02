/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_sys.c
*
* This file contains System Board APIs used to read/update image related
* information.
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xbir_sys.h"
#include "xbir_qspi.h"
#include "xbir_i2c.h"
#include "xbir_qspimap.h"
#include "string.h"
#include "xbir_config.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XBIR_SYS_QSPI_MAX_SUB_SECTOR_SIZE	(8192U)	/* 8KB */
#define XBR_SYS_NUM_REDUNDANT_COPY		(2U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void Xbir_SysReadAndCorrectBootImgInfo (void);
static int Xbir_SysReadSysInfoFromEeprom (Xbir_SysHeaderInfo* SysBoardHeaderInfo,
		Xbir_SysBoardInfo* SysBoardBoardInfo, Xbir_SysHeaderInfo* CCHeaderInfo,
		Xbir_SysBoardInfo* CCBoardInfo);
static int Xbir_SysWriteBootImageInfo (Xbir_SysBootImgInfo *BootImgInfo,
	u32 Offset);
static int Xbir_SysValidateBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo,
	u32 *Checksum);
static u32 Xbir_SysCalcBootImgInfoChecksum (Xbir_SysBootImgInfo *BootImgInfo);
static int Xbir_SysWrvBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo, u32 Offset);
static void Xbir_SysShowBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo);
#if defined(XPAR_XIICPS_NUM_INSTANCES)
static int Xbir_SysReadSysBoardInfoFromEeprom (Xbir_SysHeaderInfo* SysBoardHeaderInfo,
		Xbir_SysBoardInfo* SysBoardBoardInfo);
static int Xbir_SysReadCCInfoFromEeprom (Xbir_SysHeaderInfo* CCHeaderInfo,
		Xbir_SysBoardInfo* CCBoardInfo);
#endif

/************************** Variable Definitions *****************************/
static const u32 Xbir_UtilCrcTable[] = {
	0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU, 0x076DC419U, 0x706AF48FU, 0xE963A535U, 0x9E6495A3U,
	0x0EDB8832U, 0x79DCB8A4U, 0xE0D5E91EU, 0x97D2D988U, 0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U, 0x90BF1D91U,
	0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU, 0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U,
	0x136C9856U, 0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU, 0x14015C4FU, 0x63066CD9U, 0xFA0F3D63U, 0x8D080DF5U,
	0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U, 0xA2677172U, 0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
	0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U, 0x32D86CE3U, 0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U,
	0x26D930ACU, 0x51DE003AU, 0xC8D75180U, 0xBFD06116U, 0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U, 0xB8BDA50FU,
	0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U, 0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU,
	0x76DC4190U, 0x01DB7106U, 0x98D220BCU, 0xEFD5102AU, 0x71B18589U, 0x06B6B51FU, 0x9FBFE4A5U, 0xE8B8D433U,
	0x7807C9A2U, 0x0F00F934U, 0x9609A88EU, 0xE10E9818U, 0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
	0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU, 0x6C0695EDU, 0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U,
	0x65B0D9C6U, 0x12B7E950U, 0x8BBEB8EAU, 0xFCB9887CU, 0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U, 0xFBD44C65U,
	0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U, 0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU,
	0x4369E96AU, 0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U, 0x44042D73U, 0x33031DE5U, 0xAA0A4C5FU, 0xDD0D7CC9U,
	0x5005713CU, 0x270241AAU, 0xBE0B1010U, 0xC90C2086U, 0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
	0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U, 0x59B33D17U, 0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU,
	0xEDB88320U, 0x9ABFB3B6U, 0x03B6E20CU, 0x74B1D29AU, 0xEAD54739U, 0x9DD277AFU, 0x04DB2615U, 0x73DC1683U,
	0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U, 0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U,
	0xF00F9344U, 0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU, 0xF762575DU, 0x806567CBU, 0x196C3671U, 0x6E6B06E7U,
	0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU, 0x67DD4ACCU, 0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
	0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U, 0xD1BB67F1U, 0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU,
	0xD80D2BDAU, 0xAF0A1B4CU, 0x36034AF6U, 0x41047A60U, 0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU, 0x4669BE79U,
	0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U, 0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU,
	0xC5BA3BBEU, 0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U, 0xC2D7FFA7U, 0xB5D0CF31U, 0x2CD99E8BU, 0x5BDEAE1DU,
	0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU, 0x026D930AU, 0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
	0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U, 0x92D28E9BU, 0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U,
	0x86D3D2D4U, 0xF1D4E242U, 0x68DDB3F8U, 0x1FDA836EU, 0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U, 0x18B74777U,
	0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU, 0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U,
	0xA00AE278U, 0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U, 0xA7672661U, 0xD06016F7U, 0x4969474DU, 0x3E6E77DBU,
	0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U, 0x37D83BF0U, 0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
	0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U, 0xBAD03605U, 0xCDD70693U, 0x54DE5729U, 0x23D967BFU,
	0xB3667A2EU, 0xC4614AB8U, 0x5D681B02U, 0x2A6F2B94U, 0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU, 0x2D02EF8DU,
};

static Xbir_SysBootImgInfo BootImgStatus = \
		{{0x41U, 0x42U, 0x55U, 0x4DU}, 1U, 1U, 0xAFA2BCBCU, \
		{0U, 1U, 0U, 0U}, 0x200000U, 0x1000000U, 0x1E80000U};
static u8 QspiBuffer[XBIR_SYS_QSPI_MAX_SUB_SECTOR_SIZE];
static Xbir_SysBoardInfo SysBoardBoardInfo = {0U};
static Xbir_SysBoardInfo CCBoardInfo = {0U};

/*****************************************************************************/
/**
 * @brief
 * This function carries out the required initialization for the APIs to work
 * and also displays system information.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successful intialization of the system devices
 * 		Error code on failure
 *
 *****************************************************************************/
int Xbir_SysInit (void)
{
	int Status = XST_FAILURE;
	Xbir_SysHeaderInfo SysBoardHeaderInfo = {0U};
	Xbir_SysHeaderInfo CCHeaderInfo = {0U};

	Status = Xbir_QspiInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_IicInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_SysReadSysInfoFromEeprom (&SysBoardHeaderInfo, &SysBoardBoardInfo,
			&CCHeaderInfo, &CCBoardInfo);
	if (Status != XST_SUCCESS) {
		Status = XST_SUCCESS;
		//goto END;
	}

	Xbir_SysReadAndCorrectBootImgInfo();
	Xbir_SysShowBootImgInfo(&BootImgStatus);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function return the SysBoard board information read from EEPROM. Note that
 * function Xbir_SysReadSysInfoFromEeprom should be called before calling this
 * function.
 *
 * @param	None
 *
 * @return	Pointer to SysBoard board information
 *
 *****************************************************************************/
const Xbir_SysBoardInfo* Xbir_SysGetSysBoardInfo (void)
{
	return &SysBoardBoardInfo;
}

/*****************************************************************************/
/**
 * @brief
 * This function return the carrier card information read from EEPROM. Note that
 * function Xbir_SysReadSysInfoFromEeprom should be called before calling this
 * function.
 *
 * @param	None
 *
 * @return	Pointer to carrier card information
 *
 *****************************************************************************/
const Xbir_SysBoardInfo* Xbir_SysGetCcInfo (void)
{
	return &CCBoardInfo;
}

/*****************************************************************************/
/**
 * @brief
 * This function reads and returns boot persistent state register from boot
 * image information structure.
 *
 * @param	None
 *
 * @return	Boot persistent state register content
 *
 *****************************************************************************/
const Xbir_SysPersistentState * Xbir_SysGetBootImgStatus (void)
{
	return &BootImgStatus.PersistentState;
}

/*****************************************************************************/
/**
 * @brief
 * This function returns specified boot image offset mentioned in boot image
 * information structure.
 *
 * @param	BootImgId	Boot Image ID
 * @param	Offset		Pointer to memory for returning boot image
 *				offset
 *
 * @return	XST_SUCCESS on success
 *		XST_FAILUE on invalid request
 *
 *****************************************************************************/
int Xbir_SysGetBootImgOffset (Xbir_SysBootImgId BootImgId, u32 *Offset)
{
	int Status = XST_FAILURE;

	if (XBIR_SYS_BOOT_IMG_A_ID == BootImgId) {
		*Offset = BootImgStatus.BootImgAOffset;
	}
	else if (XBIR_SYS_BOOT_IMG_B_ID == BootImgId){
		*Offset = BootImgStatus.BootImgBOffset;
	}
	else {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function updates boot persistent state register.
 *
 * @param	ImgABootable	0: Non bootable, 1: Bootable
 * @param	ImgBBootable	0: Non bootable, 1: Bootable
 * @param	ReqBootImg	0: Image A, 1: Image B
 *
 * @return	XST_SUCCESS on successful update of persistent state register
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_SysUpdateBootImgStatus (u8 ImgABootable, u8 ImgBBootable,
	u8 ReqBootImg)
{
	int Status = XST_FAILURE;
	Xbir_SysBootImgInfo BootImgInfo;
	Xbir_SysBootImgInfo RdBootImgInfo;
	u32 Addr;
	u8 Idx = 0U;

	if ((BootImgStatus.PersistentState.ImgABootable == ImgABootable) &&
		(BootImgStatus.PersistentState.ImgBBootable == ImgBBootable) &&
		(BootImgStatus.PersistentState.RequestedBootImg == ReqBootImg)) {
		Status = XST_SUCCESS;
		goto END;
	}

	memcpy ((void *)&BootImgInfo, (void *)&BootImgStatus,
		sizeof(BootImgInfo));

	BootImgInfo.PersistentState.ImgABootable = ImgABootable;
	BootImgInfo.PersistentState.ImgBBootable = ImgBBootable;
	BootImgInfo.PersistentState.RequestedBootImg = ReqBootImg;

	BootImgInfo.Checksum = Xbir_SysCalcBootImgInfoChecksum(&BootImgInfo);

	Addr = XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR;
	for (Idx = 0U; Idx < XBR_SYS_NUM_REDUNDANT_COPY; Idx++) {
		Status = Xbir_SysWriteBootImageInfo(&BootImgInfo, Addr);
		if (Status != XST_SUCCESS) {
			Xbir_Printf("Failed During Write\r\n");
			goto END;
		}

		Status = Xbir_QspiRead(Addr, (u8 *)&RdBootImgInfo,
			sizeof(RdBootImgInfo));
		if (Status != XST_SUCCESS) {
			Xbir_Printf("Failed During readback\r\n");
			goto END;
		}

		Status = Xbir_SysValidateBootImgInfo(&RdBootImgInfo, NULL);
		if (Status != XST_SUCCESS) {
			Xbir_Printf("Failed During validation\r\n");
			goto END;
		}

		Addr = XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR;
	}

END:
	if (Idx > 0U) {
		memcpy ((void *)&BootImgStatus, (void *)&BootImgInfo,
			sizeof(BootImgInfo));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes data to QSPI.
 *
 * @param	Offset	QSPI address where data is to be written
 * @param	Data	Pointer to Data to be written
 * @param	Size	Size of data to be written
 * @param	IsLast	Is this last partial data of the image
 *
 * @return	XST_SUCCESS on successful flash update with input partial data
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_SysWriteFlash (u32 Offset, u8 *Data, u32 Size,
	Xbir_ImgDataStatus IsLast)
{
	int Status = XST_FAILURE;
	u32 DataSize = Size;
	u32 WrAddr = Offset;
	u8 *WrBuff = Data;
	static u32 PrevPendingDataLen = 0U;
	u16 PageSize = Xbir_QspiGetPageSize();

	if (PrevPendingDataLen > 0U) {
		if ((Size + PrevPendingDataLen) > sizeof(QspiBuffer)) {
			goto END;
		}
		memcpy (&QspiBuffer[PrevPendingDataLen], Data, Size);
		DataSize += PrevPendingDataLen;
		WrAddr -= PrevPendingDataLen;
		WrBuff = QspiBuffer;
	}

	while (DataSize >= PageSize) {
		Status = Xbir_QspiWrite(WrAddr, WrBuff, PageSize);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		WrAddr += PageSize;
		WrBuff += PageSize;
		DataSize -= PageSize;
	}

	if (XBIR_SYS_PARTIAL_DATA_CHUNK == IsLast) {
		memcpy(QspiBuffer, WrBuff, DataSize);
		PrevPendingDataLen = DataSize;
	}
	else {
		PrevPendingDataLen = 0U;
		if (DataSize > 0U) {
			Status = Xbir_QspiWrite(WrAddr, WrBuff, DataSize);
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function erases the QSPI region reserved for image.
 *
 * @param	BootImgId	Boot Image ID
 *
 * @return	XST_SUCCESS on successfule erasing of specified QSPI region
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_SysEraseBootImg (Xbir_SysBootImgId BootImgId)
{
	int Status = XST_FAILURE;
	u32 Offset;

	if (XBIR_SYS_BOOT_IMG_A_ID == BootImgId) {
		Offset = BootImgStatus.BootImgAOffset;
	}
	else if (XBIR_SYS_BOOT_IMG_B_ID == BootImgId){
		Offset = BootImgStatus.BootImgBOffset;
	}
	else {
		goto END;
	}

	Status = Xbir_QspiFlashErase(Offset, XBIR_QSPI_MAX_BOOT_IMG_SIZE);
	if (Status != XST_SUCCESS) {
		Xbir_Printf("\r\nERROR: Flash Erase failed (%08X)\r\n",
			 Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reads the boot image info from QSPI. It also try to correct the
 * corrupted boot image information stored in QSPI.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successful read of boot image information
 * 		Error code on failure
 *
 *****************************************************************************/
static void Xbir_SysReadAndCorrectBootImgInfo (void)
{
	int Status = XST_FAILURE;
	int StatusA = XST_FAILURE;
	int StatusB = XST_FAILURE;
	u32 ChksumA;
	u32 ChksumB;
	Xbir_SysBootImgInfo BootImgInfoA __attribute__ ((aligned(32U))) = {0U};
	Xbir_SysBootImgInfo BootImgInfoB __attribute__ ((aligned(32U))) = {0U};

	/* Read Persistent State Registers */
	StatusA = Xbir_QspiRead(XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR,
		(u8 *)&BootImgInfoA, sizeof(BootImgInfoA));
	if (XST_SUCCESS == StatusA) {
		StatusA = Xbir_SysValidateBootImgInfo(&BootImgInfoA, &ChksumA);
	}

	StatusB = Xbir_QspiRead(XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR,
		(u8 *)&BootImgInfoB, sizeof(BootImgInfoB));
	if (XST_SUCCESS == StatusB) {
		StatusB = Xbir_SysValidateBootImgInfo(&BootImgInfoB, &ChksumB);
	}

	/* Try to update Image B info if it is corrupted */
	if (XST_SUCCESS == StatusA) {
		memcpy(&BootImgStatus, &BootImgInfoA, sizeof(BootImgInfoA));
		if ((StatusB != XST_SUCCESS) || (ChksumB != ChksumA)) {
			Xbir_Printf("Boot image backup info corrupted...\r\n");
			Status = Xbir_SysWrvBootImgInfo(&BootImgStatus,
					XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR);
			if (Status == XST_SUCCESS) {
				Xbir_Printf("Boot image backup info recovered...\r\n");
			}
			else {
				Xbir_Printf("ERROR: Boot image backup recovery failed...\r\n");
			}
		}
	}
	else if (XST_SUCCESS == StatusB) {
		memcpy(&BootImgStatus, &BootImgInfoB, sizeof(BootImgInfoB));
		Xbir_Printf("Boot image info corrupted...\r\n");
		Status = Xbir_SysWrvBootImgInfo(&BootImgStatus,
			XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR);
		if (Status == XST_SUCCESS) {
			Xbir_Printf("Boot image info recovered...\r\n");
		}
		else {
			Xbir_Printf("ERROR: Boot image recovery failed...\r\n");
		}
	}
	else {
		Xbir_Printf("Use default Boot image info\r\n");
		memcpy(&BootImgInfoA, &BootImgStatus, sizeof(BootImgStatus));
		Status = Xbir_SysWrvBootImgInfo(&BootImgStatus,
			XBIR_QSPI_MM_BOOT_IMG_INFO_ADDR);
		if (Status == XST_SUCCESS) {
			Xbir_Printf("Boot image info recovered...\r\n");
		}
		else {
			Xbir_Printf("ERROR: Boot image recovery failed...\r\n");
		}

		Status = Xbir_SysWrvBootImgInfo(&BootImgStatus,
					XBIR_QSPI_MM_BOOT_IMG_INFO_BKP_ADDR);
		if (Status == XST_SUCCESS) {
			Xbir_Printf("Boot image backup info recovered...\r\n");
		}
		else {
			Xbir_Printf("ERROR: Boot image backup recovery failed...\r\n");
		}
	}

	return;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes the boot image information to QSPI.
 *
 * @param	BootImgInfo	Pointer to boot image information
 * @param	Offset		QSPI offset where bBoot image information to be
 * 				weritten
 * @return	XST_SUCCESS on success write to boot image information
 * 		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysWriteBootImageInfo (Xbir_SysBootImgInfo *BootImgInfo,
	u32 Offset)
{
	int Status = XST_FAILURE;
	u16 PageSize = Xbir_QspiGetPageSize();
	u8 WriteBuffer[XBIR_SYS_QSPI_MAX_SUB_SECTOR_SIZE] = {0U};

	memcpy((void *)WriteBuffer, (void *)BootImgInfo, sizeof(Xbir_SysBootImgInfo));

	Status = Xbir_QspiFlashErase(Offset, XBIR_QSPI_MAX_BOOT_IMG_INFO_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_QspiWrite(Offset, WriteBuffer, PageSize);
	if (Status != XST_SUCCESS) {
		Xbir_Printf("ERROR: Boot Img Info write failed\r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function validates the boot image information. It checks header ID,
 * and checksum
 *
 * @param	BootImgInfo	Pointer to boot image information
 * @param	Checksum	If not NULL, function returns calculated
 * 				checksum
 *
 * @return	XST_SUCCESS on success
 * 		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysValidateBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo,
	u32 *Checksum)
{
	int Status = XST_FAILURE;
	u32 ChkSum;

	if ((BootImgInfo->IdStr[0U] == 'A') &&
		(BootImgInfo->IdStr[1U] == 'B') &&
		(BootImgInfo->IdStr[2U] == 'U') &&
		(BootImgInfo->IdStr[3U] == 'M')) {
		ChkSum = Xbir_SysCalcBootImgInfoChecksum (BootImgInfo);
		if (ChkSum == BootImgInfo->Checksum) {
			Status = XST_SUCCESS;
		}
		else {
			Xbir_Printf("ERROR: Boot Img Info validation failed\r\n");
		}

		if(Checksum != NULL) {
			*Checksum = ChkSum;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the checksum field of the Boot Image information.
 *
 * @param	BootImgInfo	Pointer to boot image information
 *
 * @return	32-bit of calculated checksum
 *
 *****************************************************************************/
static u32 Xbir_SysCalcBootImgInfoChecksum (Xbir_SysBootImgInfo *BootImgInfo)
{
	u32 Idx;
	u32 Checksum = 0U;
	u32 *Data = (u32 *) BootImgInfo;
	u32 BootImgInfoSize = sizeof(Xbir_SysBootImgInfo) / 4U;

	BootImgInfo->Checksum = 0U;
	for (Idx = 0U; Idx < BootImgInfoSize; Idx++) {
		Checksum += Data[Idx];
	}

	BootImgInfo->Checksum =  0xFFFFFFFFU - Checksum;

	return BootImgInfo->Checksum;
}

/*****************************************************************************/
/**
 * @brief
 * This function writes, reads and validates checksum of the Boot Image
 * information.
 *
 * @param	BootImgInfo	Pointer to boot image information
 * @param	Offset		Offset where boot image information is to be
 * 				written
 *
 * @return	XST_SUCCESS on successful write of boot image information
 * 		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysWrvBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo, u32 Offset)
{
	int Status = XST_FAILURE;
	Xbir_SysBootImgInfo RdBootImgInfo __attribute__ ((aligned(32U)));

	Status = Xbir_SysWriteBootImageInfo(BootImgInfo, Offset);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_QspiRead(Offset, (u8 *)&RdBootImgInfo,
		sizeof(RdBootImgInfo));

	if (XST_SUCCESS == Status) {
		Status = Xbir_SysValidateBootImgInfo(&RdBootImgInfo, NULL);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function displays the boot image information.
 *
 * @param	BootImgInfo	Pointer to boot image information
 *
 * @return	None
 *
 *****************************************************************************/
static void Xbir_SysShowBootImgInfo (Xbir_SysBootImgInfo *BootImgInfo)
{
	Xbir_Printf("[Boot Image Info]\r\n");
	Xbir_Printf("\t                 Ver: %u\r\n", BootImgInfo->Ver);
	Xbir_Printf("\t              Length: %u\r\n", BootImgInfo->Len);
	Xbir_Printf("\t            Checksum: 0x%08x\r\n", BootImgInfo->Checksum);
	Xbir_Printf("\tPersistent State Reg: 0x%08X\r\n",
		 *(u32 *)&BootImgInfo->PersistentState);
	Xbir_Printf("\t        Img A Offset: 0x%08X\r\n",
		 BootImgInfo->BootImgAOffset);
	Xbir_Printf("\t        Img B Offset: 0x%08X\r\n",
		 BootImgInfo->BootImgBOffset);
	Xbir_Printf("\t Recovery Img Offset: %08x\r\n\r\n",
		 BootImgInfo->RecoveryImgOffset);
}

#if defined(XPAR_XIICPS_NUM_INSTANCES)
/*****************************************************************************/
/**
 * @brief
 * This function reads the system information (SysBoard and carrier card) from
 * EEPROM and stores it into global variable for later access.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successfully reading system information from
 * 		EEPEOM.
 *		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysReadSysBoardInfoFromEeprom (Xbir_SysHeaderInfo* SysBoardHeaderInfo,
		Xbir_SysBoardInfo* SysBoardBoardInfo)
{
	int Status = XST_FAILURE;

	Status = Xbir_IicEepromReadData((u8 *)SysBoardHeaderInfo, sizeof(Xbir_SysHeaderInfo),
			XBIR_IIC_SYS_BOARD_EEPROM_ADDRESS + XBIR_IIC_EEPROM_HEADER_OFFSET);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	Status = Xbir_IicEepromReadData((u8 *)SysBoardBoardInfo, sizeof(Xbir_SysBoardInfo),
			XBIR_IIC_SYS_BOARD_EEPROM_ADDRESS + XBIR_IIC_EEPROM_BOARD_OFFSET);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reads the system information (SysBoard and carrier card) from
 * EEPROM and stores it into global variable for later access.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successfully reading system information from
 * 		EEPEOM.
 *		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysReadCCInfoFromEeprom (Xbir_SysHeaderInfo* CCHeaderInfo,
		Xbir_SysBoardInfo* CCBoardInfo)
{
	int Status = XST_FAILURE;

	Status = Xbir_IicEepromReadData((u8 *)CCHeaderInfo, sizeof(Xbir_SysHeaderInfo),
			XBIR_IIC_CC_EEPROM_ADDRESS + XBIR_IIC_EEPROM_HEADER_OFFSET);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	Status = Xbir_IicEepromReadData((u8 *)CCBoardInfo, sizeof(Xbir_SysBoardInfo),
			XBIR_IIC_CC_EEPROM_ADDRESS + XBIR_IIC_EEPROM_BOARD_OFFSET);
	if (XST_SUCCESS != Status) {
		goto END;
	}

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief
 * This function reads the system information (SysBoard and carrier card) from
 * EEPROM and stores it into global variable for later access.
 *
 * @param	None
 *
 * @return	XST_SUCCESS on successfully reading system information from
 * 		EEPEOM.
 *		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysReadSysInfoFromEeprom (Xbir_SysHeaderInfo* SysBoardHeaderInfo,
		Xbir_SysBoardInfo* SysBoardBoardInfo, Xbir_SysHeaderInfo* CCHeaderInfo,
		Xbir_SysBoardInfo* CCBoardInfo)
{
	int Status = XST_FAILURE;

#if defined(XPAR_XIICPS_NUM_INSTANCES)
	Status = Xbir_SysReadSysBoardInfoFromEeprom (SysBoardHeaderInfo, SysBoardBoardInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_SysReadCCInfoFromEeprom (CCHeaderInfo, CCBoardInfo);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
#else
	Status = XST_SUCCESS;
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the CRC32 of the data stored in specified QSPI
 * memory region.
 *
 * @param	Offset	QSPI Memory region offset
 * @param	Size	Size of the region on which CRC32 is to be calculated
 * @param	Crc	Memory location where calculated CRC32 will be returned
 *
 * @return	XST_SUCCESS on successfully calculating the CRC32.
 *		Error code on failure
 *
 *****************************************************************************/
static int Xbir_SysCalculateCrc32 (u32 Offset, u32 Size, u32* Crc)
{
	int Status = XST_FAILURE;
	u32 CalcCrc = 0xFFFFFFFFU;
	u32 Idx;
	u32 Len;
	u32 Addr = Offset;
	u32 RemainingSize = Size;
	u8 ReadBuffer[XBIR_SYS_QSPI_MAX_SUB_SECTOR_SIZE]
			__attribute__ ((aligned(32U))) = {0U};

	while (RemainingSize > 0U) {
		if (RemainingSize > sizeof(ReadBuffer)) {
			Len = sizeof(ReadBuffer);
		}
		else {
			Len = RemainingSize;
		}

		Status = Xbir_QspiRead(Addr, ReadBuffer, Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		for (Idx = 0U; Idx < Len; Idx++) {
			CalcCrc = (CalcCrc >> 8U) ^
				Xbir_UtilCrcTable[(CalcCrc ^ ReadBuffer[Idx]) & 0xFFU];
		}

		RemainingSize -= Len;
		Addr += Len;
	}

	*Crc = CalcCrc ^ 0xFFFFFFFFU;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function calculates the CRC32 of specified image and compares it to
 * input Crc32.
 *
 * @param	BootImgId	ID of the image
 * @param	Size		Size of the image
 * @param	InCrc		Expected CRC32 of the image
 *
 * @return	XST_SUCCESS on successfully calculating the CRC32.
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_SysValidateCrc (Xbir_SysBootImgId BootImgId, u32 Size, u32 InCrc)
{
	u32 Status = XST_FAILURE;
	u32 Crc = 0U;
	u32 Offset;

	if (XBIR_SYS_BOOT_IMG_A_ID == BootImgId) {
		Offset = BootImgStatus.BootImgAOffset;
	}
	else if (XBIR_SYS_BOOT_IMG_B_ID == BootImgId){
		Offset = BootImgStatus.BootImgBOffset;
	}
	else {
		goto END;
	}

	Status = Xbir_SysCalculateCrc32(Offset, Size, &Crc);
	if ((Status == XST_SUCCESS) && (Crc == InCrc)) {
		Xbir_Printf("CRC matches\r\n");
		Status = XST_SUCCESS;
	}
	else {
		Xbir_Printf("ERROR: Crc mismatch\r\n");
		Status = XST_FAILURE;
	}

	Xbir_Printf("Flash Img CRC = %08X, Sender Crc = %08X\r\n", Crc, InCrc);

END:
	return Status;
}
