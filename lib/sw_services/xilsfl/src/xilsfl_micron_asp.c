/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilsfl_micron_asp.c
 * @brief Implements Micron advanced sector protection for OCTAL and QUAD NOR.
 *
 * This file implements Micron Advanced Sector Protection (ASP) for OCTAL OSPI
 * and QUAD QSPI NOR. It covers volatile locks, nonvolatile (NV) locks, the
 * Sector Protection Register, password register sequences, and erasing
 * nonvolatile lock bits. It uses the framing expected when
 * XSfl_AspMicronOctalAspOpsTable binds at initialization. The Flash
 * Configuration Table selects the transfer protocol. The same ASP sequences
 * therefore drive both OCTAL (Micron ID 0x2C) and QUAD (Micron ID 0x20) parts.
 *
 * This module never caches the password supplied by the application in static
 * RAM. Verification reads the on-device password with READ PASSWORD and
 * compares it to the caller transient buffer for each authenticated operation.
 *
 * NOTE ON RETENTION (datasheet): Micron documents PASSWORD PROGRAM timing
 * (tPASSP) and PASSWORD PROTECTION PROGRAM operations. Password bits can remain
 * programmed across power cycles depending on device configuration (factory
 * default is typically erased). Confirm retention on your silicon step before
 * relying on power-off clearing. Volatile sector locks (E1/E0) still clear at
 * reset.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.3   sb  07/20/26  Initial support for Advanced Sector Protection.
 * 1.4   vn  09/30/26  Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 ******************************************************************************/

/** @addtogroup xilsfl overview
 * @{
 */

#include "xilsfl_config.h"

#if CONFIG_ADVANCED_SECTOR_PROTECTION

/***************************** Include Files *********************************/
#include "xilsfl_control.h"
#include "xilsfl_micron_asp.h"
#include <string.h>

extern XSfl SflInstance; /**< SFL library instance. */


/*****************************************************************************/
/**
 *
 * @brief Reads the Sector Protection Register of the selected chip.
 *
 * The function issues the Micron READ SECTOR PROTECTION command and returns
 * the raw 16-bit Sector Protection Register value from the selected chip.
 *
 * @param[in]   SflInst Pointer to the XSfl_Interface instance.
 * @param[out]  SprWord Pointer that receives the 16-bit SPR value.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
static u32 XSfl_MicronSectorProtectionReadChip(XSfl_Interface *SflInst, u16 *SprWord)
{
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 RxBuf[2];
#else
	u8 RxBuf[2] __attribute__((aligned(4)));
#endif

	Xil_AssertNonvoid(SprWord != NULL);

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_READ_SECTOR_PROT_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	/* Datasheet: 8 dummy cycles required for OSPI (8-0-8); 0 for QSPI (4-0-4). */
	CmdBuffer[XSFL_DUMMY_OFFSET] =
		(SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) ?
		XSFL_MICRON_REG_READ_DUMMY_OCTAL : 0U;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 2;

	Status = XSfl_FlashRegisterReadWrite(SflInst, RxBuf, NULL, CmdBuffer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	memcpy(SprWord, RxBuf, sizeof(*SprWord));
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Erases the nonvolatile lock bits on the currently selected die.
 *
 * The function issues ERASE NONVOLATILE LOCK BITS (E4h) on the currently
 * selected die using the Micron ASP framing path. MT35X octal parts often
 * ignore a bare command transfer, so the function uses the grouped ASP message
 * path instead.
 *
 * @param[in]   SflInst Pointer to the XSfl_Interface instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
static u32 XSfl_MicronEraseNvLockBitsCurrentDie(XSfl_Interface *SflInst)
{
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;

	Status = XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_ERASE_NV_LOCK_BITS_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	CmdBuffer[XSFL_DUMMY_OFFSET] = 0;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 0;

	Status = XSfl_FlashRegisterReadWrite(SflInst, NULL, NULL, CmdBuffer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		(void)XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_DISABLE_CMD);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Reads the password register of the selected chip.
 *
 * The function issues the Micron READ PASSWORD command and stores the device
 * password register contents into the caller-supplied buffer.
 *
 * @param[in]   SflInst Pointer to the XSfl_Interface instance.
 * @param[out]  OutPwd Pointer to the buffer that receives the password bytes.
 * @param[in]   DummyOctal Number of dummy cycles to use for the read transfer.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
static u32 XSfl_MicronPasswordReadChip(XSfl_Interface *SflInst, u8 *OutPwd,
		u8 DummyOctal)
{
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;

	Xil_AssertNonvoid(OutPwd != NULL);

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_READ_PASSWORD_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	CmdBuffer[XSFL_DUMMY_OFFSET] = DummyOctal;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = XSFL_MICRON_ASP_PASSWORD_BYTES;

	Status = XSfl_FlashRegisterReadWrite(SflInst, OutPwd, NULL, CmdBuffer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Issues a password write command on the selected chip.
 *
 * The function issues write-enable followed by a password write command on
 * the selected chip. The opcode supplied in OpcAfterEnable selects between
 * WRITE PASSWORD (program) and UNLOCK PASSWORD operations.
 *
 * @param[in]   SflInst Pointer to the XSfl_Interface instance.
 * @param[in]   Pw Pointer to the buffer that holds the password bytes to
 *              transmit.
 * @param[in]   OpcAfterEnable Command opcode to issue after write-enable.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
static u32 XSfl_MicronPasswordWriteChip(XSfl_Interface *SflInst, const u8 *Pw,
		u8 OpcAfterEnable)
{
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 TxBuf[16];
#else
	u8 TxBuf[16] __attribute__((aligned(4)));
#endif

	Xil_AssertNonvoid(Pw != NULL);

	Status = XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	memcpy(TxBuf, Pw, XSFL_MICRON_ASP_PASSWORD_BYTES);

	CmdBuffer[XSFL_COMMAND_OFFSET] = OpcAfterEnable;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	CmdBuffer[XSFL_DUMMY_OFFSET] = 0;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = XSFL_MICRON_ASP_PASSWORD_BYTES;

	Status = XSfl_FlashRegisterReadWrite(SflInst, NULL, TxBuf, CmdBuffer, 0);
	memset(TxBuf, 0, sizeof(TxBuf));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		(void)XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_DISABLE_CMD);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Reads the Micron Sector Protection Register.
 *
 * The function runs the Micron OCTAL ASP READ SECTOR PROTECTION sequence and
 * stores the raw 16-bit Sector Protection Register value.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  SprWord Pointer that receives the SPR value.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronSectorProtectionRead(u8 SflHandler, u16 *SprWord)
{
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(SprWord != NULL);

	return XSfl_MicronSectorProtectionReadChip(&SflInstance.Instance[SflHandler],
			SprWord);
}

/*****************************************************************************/
/**
 *
 * @brief Programs the Micron Sector Protection Register.
 *
 * The function programs the 16-bit Sector Protection Register field on Micron
 * OCTAL NOR using PROGRAM SECTOR PROTECTION after write-enable. It then waits
 * until the device completes the operation.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SprWord 16-bit Sector Protection Register value to program.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronSectorProtectionWrite(u8 SflHandler, u16 SprWord)
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;
	u16 CurrentSpr;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 TxBuf[2];
#else
	u8 TxBuf[2] __attribute__((aligned(4)));
#endif

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Status = XSfl_MicronSectorProtectionReadChip(SflInst, &CurrentSpr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	if ((CurrentSpr & XSFL_MICRON_SPR_OTP_LOCK_BIT) == 0U) {
		return XST_FAILURE;
	}

	Status = XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	memcpy(TxBuf, &SprWord, sizeof(SprWord));

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_PROGRAM_SECTOR_PROT_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	CmdBuffer[XSFL_DUMMY_OFFSET] = 0;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 2U;

	Status = XSfl_FlashRegisterReadWrite(SflInst, NULL, TxBuf, CmdBuffer, 0);
	memset(TxBuf, 0, sizeof(TxBuf));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		(void)XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_DISABLE_CMD);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Erases the Micron nonvolatile lock bits.
 *
 * The function issues ERASE NONVOLATILE LOCK BITS on the active die using the
 * Micron ASP framing (write-enable followed by grouped transfer path). When the
 * flash is stacked, the function selects each die CS in turn.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronEraseNVLockBits(u8 SflHandler)
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 Status;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	/* Ensure CS0; stacked parts need E4 on each die (see 4B mode entry). */
	Status = SflInst->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_MicronEraseNvLockBitsCurrentDie(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (SflInst->SflFlashInfo.ConnectionMode == XSFL_CONNECTION_MODE_STACKED) {
		Status = SflInst->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS) {
			(void)SflInst->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS0);
			return XST_FAILURE;
		}

		Status = XSfl_MicronEraseNvLockBitsCurrentDie(SflInst);
		if (Status != XST_SUCCESS) {
			(void)SflInst->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS0);
			return XST_FAILURE;
		}

		Status = SflInst->CntrlInfo.SelectFlash(XSFL_SELECT_FLASH_CS0);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Programs the Micron nonvolatile lock bit of a sector.
 *
 * The function programs the Micron OCTAL NOR nonvolatile lock bit covering the
 * sector that contains SectorAddress after sector-boundary validation.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SectorAddress Aligned start address of the sector to lock.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronNVSectorLock(u8 SflHandler, u32 SectorAddress)
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;
	u32 RealAddr;
	u8 AddrSz;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	/* ASP NV lock ops require a sector-aligned address. */
	if ((SectorAddress % SflInst->SflFlashInfo.SectSize) != 0U) {
		return XST_FAILURE;
	}

	AddrSz = SflInst->SflFlashInfo.AddrSize;
	RealAddr = XSfl_GetRealAddr(SflInst, SectorAddress);

	Status = XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_WRITE_NV_LOCK_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = RealAddr;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = AddrSz;
	CmdBuffer[XSFL_DUMMY_OFFSET] = 0;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 0;

	Status = XSfl_FlashRegisterReadWrite(SflInst, NULL, NULL, CmdBuffer, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	switch (SflInst->SflFlashInfo.FlashMake) {
		case XSFL_MICRON_OCTAL_ID_BYTE0:
			(void)XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_DISABLE_CMD);
			break;
		default:
			break;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Reads the Micron nonvolatile lock state of a sector.
 *
 * The function reads the nonvolatile lock indication for SectorAddress through
 * the Micron READ NONVOLATILE LOCK sequence. It translates the inverted device
 * byte into 0 / 1.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   SectorAddress Aligned start address of the sector to query.
 * @param[out]  IsLocked Pointer that receives the normalized lock flag (1
 *              locked).
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronNVLockRead(u8 SflHandler, u32 SectorAddress, u8 *IsLocked)
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;
	u32 RealAddr;
	u8 AddrSz;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 RxBuf[2];
#else
	u8 RxBuf[2] __attribute__((aligned(4)));
#endif

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(IsLocked != NULL);

	/* ASP NV lock ops require a sector-aligned address. */
	if ((SectorAddress % SflInst->SflFlashInfo.SectSize) != 0U) {
		return XST_FAILURE;
	}

	AddrSz = SflInst->SflFlashInfo.AddrSize;
	RealAddr = XSfl_GetRealAddr(SflInst, SectorAddress);

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_READ_NV_LOCK_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = RealAddr;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = AddrSz;
	/* Datasheet: 8 dummy cycles required for OSPI (8-8-8); 0 for QSPI (4-4-4). */
	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		CmdBuffer[XSFL_DUMMY_OFFSET] = XSFL_MICRON_REG_READ_DUMMY_OCTAL;
	}
	/*
	 * Request a single data byte; XSfl_FlashRegisterReadWrite rounds this
	 * up to 2 automatically in DDR-PHY mode.
	 */
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 1;

	Status = XSfl_FlashRegisterReadWrite(SflInst, RxBuf, NULL, CmdBuffer, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Match legacy QSPI stack: inverted byte; normalize to {0,1}. */
	*IsLocked = ((u8)(~RxBuf[0]) != 0U) ? 1U : 0U;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Programs the Micron ASP password register.
 *
 * The function programs the ASP password bytes into the Micron NOR password
 * register using the WRITE PASSWORD command after write-enable.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Password Buffer that holds the password bytes to program.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronPasswordProgram(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(Password != NULL);

	return XSfl_MicronPasswordWriteChip(&SflInstance.Instance[SflHandler],
			Password, XSFL_MICRON_WRITE_PASSWORD_CMD);
}

/*****************************************************************************/
/**
 *
 * @brief Reads the Micron ASP password register.
 *
 * The function reads the Micron ASP password register into PasswordOut using
 * READ PASSWORD. It zero-pads any tail when the API buffer is larger than the
 * device password.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[out]  PasswordOut Buffer where the read password is stored.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronPasswordRead(u8 SflHandler,
		u8 PasswordOut[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 Status;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(PasswordOut != NULL);

	memset(PasswordOut, 0, XSFL_ASP_PASSWORD_MAX_BYTES);

	/*
	 * MT35X datasheet (Table, note 5): READ PASSWORD (27h) dummy cycles are
	 * fixed at 8 for OSPI (8-0-8) and 0 for QSPI (1-0-1). No fallback count
	 * is defined; an all-zero result is the valid factory-erased password.
	 */
	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		Status = XSfl_MicronPasswordReadChip(SflInst, PasswordOut,
				XSFL_MICRON_REG_READ_DUMMY_OCTAL);
	} else {
		Status = XSfl_MicronPasswordReadChip(SflInst, PasswordOut, 0U);
	}
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Unlocks the Micron flash with the ASP password.
 *
 * The function issues UNLOCK PASSWORD on Micron OCTAL NOR with the supplied
 * password buffer after write-enable.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 * @param[in]   Password Buffer that holds the password supplied for unlock.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronPasswordUnlock(u8 SflHandler,
		const u8 Password[XSFL_ASP_PASSWORD_MAX_BYTES])
{
	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);
	Xil_AssertNonvoid(Password != NULL);

	return XSfl_MicronPasswordWriteChip(&SflInstance.Instance[SflHandler],
			Password,
			XSFL_MICRON_UNLOCK_PASSWORD_CMD);
}

/*****************************************************************************/
/**
 *
 * @brief Writes the Micron global freeze bit.
 *
 * The function issues WRITE GLOBAL FREEZE BIT (A6h) on Micron OCTAL NOR. Per
 * MT35X Table 12, the command sets the global freeze bit to 0. This protects
 * the nonvolatile lock bits from further PROGRAM or ERASE operations.
 *
 * @param[in]   SflHandler Index of the SFL interface driver instance.
 *
 * @return
 *              - XST_SUCCESS if successful.
 *              - Error code otherwise.
 *
 ******************************************************************************/
u32 XSfl_AspMicronGlobalFreezeWrite(u8 SflHandler)
{
	XSfl_Interface *SflInst = &SflInstance.Instance[SflHandler];
	u32 CmdBuffer[XSFL_BUFFER_ARRAY_LEN] = {0};
	u32 Status;

	Xil_AssertNonvoid(SflHandler < XSFL_NUM_INSTANCES);

	Status = XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_ENABLE_CMD);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	CmdBuffer[XSFL_COMMAND_OFFSET] = XSFL_MICRON_WRITE_GLOBAL_FREEZE_CMD;
	CmdBuffer[XSFL_ADDRESS_OFFSET] = 0;
	CmdBuffer[XSFL_ADDRESS_SIZE_OFFSET] = 0;
	CmdBuffer[XSFL_DUMMY_OFFSET] = 0;
	CmdBuffer[XSFL_BYTE_COUNT_OFFSET] = 0;

	Status = XSfl_FlashRegisterReadWrite(SflInst, NULL, NULL, CmdBuffer, 0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSfl_WaitforStatusDone(SflInst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (SflInst->SflFlashInfo.FlashMake == XSFL_MICRON_OCTAL_ID_BYTE0) {
		(void)XSfl_FlashCmdTransfer(SflInst, XSFL_WRITE_DISABLE_CMD);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * @brief Holds the Micron OCTAL ASP vendor operations.
 *
 * Micron OCTAL ASP vendor ops table: XSfl_FlashAspAssignVendorOps binds it to
 * XSfl_FlashInfo::AspOps when the maker ID and FCT indicate Micron OCTAL OSPI
 * NOR.
 *
 ******************************************************************************/
const XSfl_AspVendorOps XSfl_AspMicronOctalAspOpsTable = {
	.SectorProtectionRead            = XSfl_AspMicronSectorProtectionRead,
	.SectorProtectionWrite           = XSfl_AspMicronSectorProtectionWrite,
	.EraseNVLockBits       = XSfl_AspMicronEraseNVLockBits,
	.NVSectorLock          = XSfl_AspMicronNVSectorLock,
	.NVLockRead            = XSfl_AspMicronNVLockRead,
	.PasswordProgram                = XSfl_AspMicronPasswordProgram,
	.PasswordRead                   = XSfl_AspMicronPasswordRead,
	.PasswordUnlock                 = XSfl_AspMicronPasswordUnlock,
	.GlobalFreezeWrite              = XSfl_AspMicronGlobalFreezeWrite,
	.AspPasswordLenBytes            = XSFL_MICRON_ASP_PASSWORD_BYTES,
};

#endif /* CONFIG_ADVANCED_SECTOR_PROTECTION */

/** @} */
