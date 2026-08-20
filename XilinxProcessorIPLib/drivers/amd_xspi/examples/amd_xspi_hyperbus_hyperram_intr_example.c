/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_hyperbus_hyperram_intr_example.c
*
* This file contains an interrupt-driven example for the AXI XSPI driver
* operating in AXI-Lite mode with a HyperBus (xSPI Profile 2.0) HyperRAM
* device. It is vendor-neutral: any HyperBus HyperRAM returning a valid
* Identification Register 0 is accepted (tested with an Infineon device;
* datasheet citations use the Infineon HyperRAM datasheet as a reference).
* It performs the same flow as the polled example:
* - Initialization of the AXI XSPI controller and interrupt system
* - Power-up verification of HyperRAM
* - Reading HyperRAM Identification Registers (IR0, IR1) and CR0
* - Memory RAM test: a fresh per-run pattern written across a one row region
*   in chunks and read back
*   (write/read/verify with data integrity and address decode checks)
*
* Interrupt usage: transactions use the message-based AmdXspi_InterruptTransfer
* API. TXN_COMPLETE/TXN_ERROR are routed through the GIC/INTC to
* AmdXspi_InterruptHandler, which drives the message queue and invokes the
* status callback (AmdXspi_SetStatusHandler) with XST_SPI_TRANSFER_DONE.
* The callback only sets flags (no prints) so it does not perturb bus timing.
*
* NOTE: HyperRAM is self-refreshing DRAM, not flash - there is no flash-style
* erase command and no defined "erased" value. This example does a RAM test
* instead: it writes a pattern that mixes each word's address into its value
* and changes every run, so that a bit stuck at either level, a chunk written
* to the wrong address and a write that fails silently are all caught by the
* one pass.
*
* This example requires a GIC or INTC interrupt controller in the system.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "amd_xspi.h"
#include "amd_xspi_hyperbus_config.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xinterrupt_wrap.h"

/************************** Constant Definitions *****************************/

#define AMD_XSPI_BASEADDR	XPAR_XAMD_XSPI_0_BASEADDR	/**< Controller base address */

/*
 * Register-space word addresses, ID/CR0 field encodings, per-device latency
 * and burst tables, density and timing all come from the HyperRAM device
 * table in amd_xspi_hyperbus_config.h. Supporting a new HyperRAM part means
 * adding a row to that table; the one value that cannot come from there is
 * below, because it is needed before the device is known.
 */

/*
 * Initial latency, in clocks, that the fitted HyperRAM comes out of reset at.
 * Take it from the vendor datasheet for the part on the board - it is the
 * reset value of CR0[7:4], 7 clocks on the Infineon S27KL/S27KS family.
 *
 * The read that identifies the device needs a latency, and the device is not
 * known yet, so this one value has to be supplied by hand. It is only used
 * until CR0 has been read; from then on the latency comes from the device's
 * own code table. Once the part has been identified this value is checked
 * against its table entry and a mismatch is reported.
 *
 * Set it correctly. The device begins driving data the moment its own latency
 * expires, so any other count reads the wrong position in the burst, and one
 * that is too short can wedge the part until RESET# or a power cycle - see
 * "The bootstrap problem" in amd_xspi_hyperbus_config.h.
 */
#define HYPERRAM_DEFAULT_LATENCY_CLOCKS	7U	/**< CR0[7:4] reset value of the
						     fitted part, in clocks */

/*
 * Memory test region (HyperRAM memory space, 16-bit word addressing).
 *
 * The region is covered in chunks rather than in one transaction, because
 * tCSM limits how long chip select may stay low, not how much data may be
 * moved in total: chip select rises between chunks and the device refreshes
 * as normal. Enlarging the region is therefore free of tCSM risk, while
 * enlarging the chunk is not.
 *
 * The region is one row (512 words on the fitted part), so the test exercises
 * address decode across a row rather than sitting in a handful of words.
 * Keeping the chunk a power of two and the start address chunk-aligned means
 * no linear burst ever crosses a row boundary part-way through.
 */
#define HYPERRAM_TEST_WORD_ADDR		0x00000000U	/**< First word address of
							     the test region */
#define HYPERRAM_CHUNK_WORDS		16U		/**< 16 words = 32 bytes per
							     chip-select window */
#define HYPERRAM_CHUNK_BYTES		(HYPERRAM_CHUNK_WORDS * 2U)
						/**< Chunk size in bytes */
#define HYPERRAM_TEST_NUM_WORDS		512U		/**< 512 words = 1 KB = one row */
#define HYPERRAM_TEST_NUM_CHUNKS	(HYPERRAM_TEST_NUM_WORDS / HYPERRAM_CHUNK_WORDS)
						/**< Transactions per pass */

/*
 * Step added to the value already stored in the region (from the previous
 * run) to derive a FRESH write pattern on every run. Writing new data each
 * run guarantees the write path is genuinely exercised (a silently failed
 * write would read back the previous run's data and no longer match).
 *
 * NOTE (maintainers): whenever this example is updated, also change this
 * step (or the base pattern) so a rebuilt binary writes a different data set.
 */
#define HYPERRAM_SEED_STEP		0x0111U	/**< Added to the stored value to
						     derive this run's base pattern */

/*
 * Odd multiplier that mixes the word address into the per-run pattern, so
 * every word in the region carries a different value. Without this a chunk
 * written to the wrong address would still read back as expected and an
 * address decode fault would pass unnoticed. Odd, so the mapping is one to
 * one over 16 bits and no two addresses in the region can collide.
 */
#define HYPERRAM_ADDR_MIX		0x9E37U	/**< Odd multiplier mixing the word
						     address into the pattern */

/*
 * Fallback base pattern used only when the pre-run read of the region fails
 * (for example first-ever run). Normal runs advance from the stored value.
 */
#define HYPERRAM_SEED_FALLBACK		0xA500U	/**< Base pattern used when the
						     pre-run read fails */

/************************** Function Prototypes ******************************/

static s32 AmdXspiIntrExample(UINTPTR BaseAddress);

static void AmdXspiStatusHandler(void *CallBackRef, u32 StatusEvent,
			      u32 ByteCount);
static s32 WaitForTransfer(void);
static void HyperRAM_RecoverController(AmdXspi *InstancePtr);

static void PrintSckFreq(AmdXspi *InstancePtr);
static void BuildHyperBusCA_Register(u8 *CaBuf, u32 WordAddr, u8 IsRead);
static void BuildHyperBusCA_Memory(u8 *CaBuf, u32 WordAddr, u8 IsRead);
static s32 HyperRAM_ReadRegister(AmdXspi *InstancePtr, u32 RegWordAddr, u16 *Data);
static s32 HyperRAM_WriteMemory(AmdXspi *InstancePtr, u32 WordAddr,
			       const u16 *Data, u32 NumWords, u8 Latency);
static s32 HyperRAM_ReadMemory(AmdXspi *InstancePtr, u32 WordAddr,
			      u16 *Data, u32 NumWords, u8 Latency);
static u16 HyperRAM_ExpectedWord(u16 Base, u16 AddrMix, u32 WordAddr);
static s32 HyperRAM_Write(AmdXspi *InstancePtr, u32 StartWordAddr,
			 const u16 *Data, u32 NumWords, u8 Latency);
static s32 HyperRAM_Read(AmdXspi *InstancePtr, u32 StartWordAddr,
			u16 *Data, u32 NumWords, u8 Latency);
static s32 HyperRAM_Compare(AmdXspi *InstancePtr, u32 StartWordAddr,
			   const u16 *Written, const u16 *Read, u32 NumWords,
			   const char *Label, u8 Latency);

/************************** Variable Definitions *****************************/

static AmdXspi AmdXspiInstance;

static u8 CmdAddrBuffer[AMD_XSPI_HYPERBUS_CA_BYTES];
static u8 DataBuffer[2];	/* 2-byte data buffer for register access */

/*
 * Device table entry for the attached part, resolved from the ID registers.
 * NULL until the device has been identified.
 */
static const AmdXspi_HyperBusConfig *HyperRamCfg;

/*
 * Latency used for register-space reads, set from the device table before the
 * first read. This example only reads CR0, never writes it, so the device
 * stays at its power-on latency throughout and this value never changes.
 */
static u8 RegLatency;

/*
 * Bus-facing buffers for one transaction. A chip-select window has to close
 * before tCSM, so a single transaction never carries more than a chunk.
 */
static u8 MemTxBuffer[HYPERRAM_CHUNK_BYTES];
static u8 MemRxBuffer[HYPERRAM_CHUNK_BYTES];

/*
 * The pattern written to the test region and the data read back from it. The
 * verify step compares these two directly, so both span the whole region:
 * enlarging HYPERRAM_TEST_NUM_WORDS grows them by 2 bytes per word each.
 */
static u16 WriteWords[HYPERRAM_TEST_NUM_WORDS];
static u16 ReadWords[HYPERRAM_TEST_NUM_WORDS];

/* Holds the chunk re-read after a mismatch, to separate a bad write from a
 * bad read. */
static u16 RereadWords[HYPERRAM_CHUNK_WORDS];

/*
 * Set from the status callback (runs in interrupt context). The main
 * flow launches a transfer with AmdXspi_InterruptTransfer, then spins on these.
 */
static volatile int TransferDone;
static volatile int TransferError;
static volatile u32 TxnCompleteCount;	/* number of completed transfers */
static volatile u32 ErrorCount;		/* number of error interrupts */

/*
 * Status event from the most recent callback. A failed transfer reports it, so
 * a short receive (XST_SPI_RECEIVE_OVERRUN, 1154) is told apart from a fault
 * the controller raised, since the two need different fixes.
 */
static volatile u32 LastStatusEvent;

/** Spin timeout for waiting on an interrupt-driven transfer to finish */
#define AMD_XSPI_TRANSFER_WAIT_LIMIT	100000000U

/*****************************************************************************/
/**
*
* Main function to run the AXI XSPI interrupt example.
*
* @return
*		- XST_SUCCESS if the example ran successfully.
*		- XST_FAILURE if the example failed.
*
******************************************************************************/
int main(void)
{
	s32 Status;

	xil_printf("\r\n");
	xil_printf("##############################################################\r\n");
	xil_printf("#     AXI XSPI HyperRAM Interrupt Example                    #\r\n");
	xil_printf("#     xSPI Profile 2.0 (HyperBus)                            #\r\n");
	xil_printf("##############################################################\r\n");

	Status = AmdXspiIntrExample(AMD_XSPI_BASEADDR);

	if (Status != XST_SUCCESS) {
		xil_printf("\r\nHyperBus HyperRAM Interrupt Ex Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\r\nSuccessfully ran HyperBus HyperRAM Interrupt Ex\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Report the bus clock, which is the external clock divided by the core's
* SCK ratio. Worth printing separately because the two are easily confused,
* and it is the bus clock that the tCSM budget is measured against.
*
* A BSP generated before the divider parameters were described in the driver
* metadata reports them as zero, in which case the frequency is unknown rather
* than zero, and saying so is more useful than printing a wrong number.
*
* @param	InstancePtr is a pointer to the AmdXspi instance.
*
******************************************************************************/
static void PrintSckFreq(AmdXspi *InstancePtr)
{
	u32 SckFreq = AmdXspi_GetSckFreqMHz(InstancePtr);

	if (SckFreq == 0U) {
		xil_printf("    SckFreq:    unknown (divider not reported by "
			   "hardware)\r\n");
	} else {
		xil_printf("    SckFreq:    %u MHz (bus clock, ext / %u)\r\n",
			   SckFreq, InstancePtr->Config.SckRatio);
	}
}

/*****************************************************************************/
/**
*
* Returns the controller to a state where a fresh transaction can start.
*
* AmdXspi_Abort() restores the controller, but the aborted transfer may
* already have set the flags the status handler drives. Clearing them here
* keeps the abort and the flag reset together, so a stale completion cannot
* satisfy the next WaitForTransfer().
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
*
******************************************************************************/
static void HyperRAM_RecoverController(AmdXspi *InstancePtr)
{
	AmdXspi_Abort(InstancePtr);

	TransferDone = 0U;
	TransferError = 0U;
}

/*****************************************************************************/
/**
*
* Build HyperBus Command/Address (CA) bytes for Register space access.
* CA[46]=1 (register), CA[45]=1 (linear), MSB-first, big-endian.
*
******************************************************************************/
static void BuildHyperBusCA_Register(u8 *CaBuf, u32 WordAddr, u8 IsRead)
{
	u64 Ca = 0ULL;

	if (IsRead != 0U) {
		Ca |= (1ULL << 47);		/* CA[47] = R/W# */
	}
	Ca |= (1ULL << 46);			/* CA[46] = 1 (Register space) */
	Ca |= (1ULL << 45);			/* CA[45] = 1 (Linear burst) */
	Ca |= ((u64)(WordAddr >> 3) & 0x1FFFFFFFUL) << 16;	/* CA[44:16] */
	Ca |= ((u64)(WordAddr & 0x7U));		/* CA[2:0] */

	CaBuf[0] = (u8)((Ca >> 40) & 0xFFU);
	CaBuf[1] = (u8)((Ca >> 32) & 0xFFU);
	CaBuf[2] = (u8)((Ca >> 24) & 0xFFU);
	CaBuf[3] = (u8)((Ca >> 16) & 0xFFU);
	CaBuf[4] = (u8)((Ca >> 8) & 0xFFU);
	CaBuf[5] = (u8)(Ca & 0xFFU);
}

/*****************************************************************************/
/**
*
* Build HyperBus Command/Address (CA) bytes for Memory space access.
* Identical to the register builder except CA[46]=0 (memory space).
*
******************************************************************************/
static void BuildHyperBusCA_Memory(u8 *CaBuf, u32 WordAddr, u8 IsRead)
{
	u64 Ca = 0ULL;

	if (IsRead != 0U) {
		Ca |= (1ULL << 47);		/* CA[47] = R/W# */
	}
	/* CA[46] = 0 (Memory space) - left as zero */
	Ca |= (1ULL << 45);			/* CA[45] = 1 (Linear burst) */
	Ca |= ((u64)(WordAddr >> 3) & 0x1FFFFFFFUL) << 16;	/* CA[44:16] */
	Ca |= ((u64)(WordAddr & 0x7U));		/* CA[2:0] */

	CaBuf[0] = (u8)((Ca >> 40) & 0xFFU);
	CaBuf[1] = (u8)((Ca >> 32) & 0xFFU);
	CaBuf[2] = (u8)((Ca >> 24) & 0xFFU);
	CaBuf[3] = (u8)((Ca >> 16) & 0xFFU);
	CaBuf[4] = (u8)((Ca >> 8) & 0xFFU);
	CaBuf[5] = (u8)(Ca & 0xFFU);
}

/*****************************************************************************/
/**
*
* Read from a HyperRAM register (2 bytes, big-endian, current initial latency).
*
******************************************************************************/
/*****************************************************************************/
/**
*
* Read a single 16-bit register from HyperRAM register space using an
* interrupt-driven transfer. Register reads take the probe latency currently
* held in RegLatency, which is the value being validated during the
* identification sweep. Data is big-endian on the bus, so the first byte
* received is bits [15:8].
*
* @param[in]  InstancePtr is a pointer to the AmdXspi instance.
* @param[in]  RegWordAddr is the register word address to read.
* @param[out] Data is the register value, or 0 if the transfer failed.
*
* @return
*		- XST_SUCCESS if the register was read.
*		- XST_FAILURE or a driver error code otherwise.
*
******************************************************************************/
static s32 HyperRAM_ReadRegister(AmdXspi *InstancePtr, u32 RegWordAddr, u16 *Data)
{
	s32 Status;
	AmdXspi_Msg Msg;

	BuildHyperBusCA_Register(CmdAddrBuffer, RegWordAddr, 1U);

	DataBuffer[0] = 0;
	DataBuffer[1] = 0;

	Msg.CmdAddrBufPtr = CmdAddrBuffer;
	Msg.CmdAddrBytes = AMD_XSPI_HYPERBUS_CA_BYTES;
	Msg.TxBfrPtr = NULL;
	Msg.RxBfrPtr = DataBuffer;
	Msg.ByteCount = 2U;
	Msg.DummyCycles = RegLatency;
	Msg.FrameFormat = AMD_XSPI_FRAME_READ_WITH_LATENCY;
	Msg.IoFormat = 0U;
	Msg.Flags = AMD_XSPI_MSG_FLAG_RX;

	TransferDone = 0;
	TransferError = 0;
	Status = AmdXspi_InterruptTransfer(InstancePtr, &Msg, 1U);
	if (Status == XST_SUCCESS) {
		Status = WaitForTransfer();
	}

	if (Status == XST_SUCCESS) {
		*Data = ((u16)DataBuffer[0] << 8) | (u16)DataBuffer[1];
	} else {
		*Data = 0;
		/* TXN_ERROR latches until reset, so clear it before returning */
		HyperRAM_RecoverController(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* Write a block of 16-bit words to HyperRAM memory space (linear burst).
* Memory writes use the same initial latency as reads; big-endian on the bus.
*
******************************************************************************/
static s32 HyperRAM_WriteMemory(AmdXspi *InstancePtr, u32 WordAddr,
			       const u16 *Data, u32 NumWords, u8 Latency)
{
	s32 Status;
	u32 i;
	AmdXspi_Msg Msg;

	if (NumWords > HYPERRAM_CHUNK_WORDS) {
		return XST_INVALID_PARAM;
	}

	for (i = 0U; i < NumWords; i++) {
		MemTxBuffer[2U * i]      = (u8)((Data[i] >> 8) & 0xFFU);
		MemTxBuffer[2U * i + 1U] = (u8)(Data[i] & 0xFFU);
	}

	BuildHyperBusCA_Memory(CmdAddrBuffer, WordAddr, 0U);

	Msg.CmdAddrBufPtr = CmdAddrBuffer;
	Msg.CmdAddrBytes = AMD_XSPI_HYPERBUS_CA_BYTES;
	Msg.TxBfrPtr = MemTxBuffer;
	Msg.RxBfrPtr = NULL;
	Msg.ByteCount = NumWords * 2U;
	Msg.DummyCycles = Latency;
	Msg.FrameFormat = AMD_XSPI_FRAME_WRITE_WITH_LATENCY;
	Msg.IoFormat = 0U;
	Msg.Flags = AMD_XSPI_MSG_FLAG_TX;

	TransferDone = 0;
	TransferError = 0;
	Status = AmdXspi_InterruptTransfer(InstancePtr, &Msg, 1U);
	if (Status == XST_SUCCESS) {
		Status = WaitForTransfer();
	}

	if (Status != XST_SUCCESS) {
		xil_printf("      Memory write FAILED at word addr 0x%08X "
			   "(status event %u)\r\n", WordAddr, LastStatusEvent);
		/* TXN_ERROR latches until reset, so clear it before returning */
		HyperRAM_RecoverController(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* Read a block of 16-bit words from HyperRAM memory space (linear burst).
*
******************************************************************************/
static s32 HyperRAM_ReadMemory(AmdXspi *InstancePtr, u32 WordAddr,
			      u16 *Data, u32 NumWords, u8 Latency)
{
	s32 Status;
	u32 i;
	AmdXspi_Msg Msg;

	if (NumWords > HYPERRAM_CHUNK_WORDS) {
		return XST_INVALID_PARAM;
	}

	BuildHyperBusCA_Memory(CmdAddrBuffer, WordAddr, 1U);

	Msg.CmdAddrBufPtr = CmdAddrBuffer;
	Msg.CmdAddrBytes = AMD_XSPI_HYPERBUS_CA_BYTES;
	Msg.TxBfrPtr = NULL;
	Msg.RxBfrPtr = MemRxBuffer;
	Msg.ByteCount = NumWords * 2U;
	Msg.DummyCycles = Latency;
	Msg.FrameFormat = AMD_XSPI_FRAME_READ_WITH_LATENCY;
	Msg.IoFormat = 0U;
	Msg.Flags = AMD_XSPI_MSG_FLAG_RX;

	TransferDone = 0;
	TransferError = 0;
	Status = AmdXspi_InterruptTransfer(InstancePtr, &Msg, 1U);
	if (Status == XST_SUCCESS) {
		Status = WaitForTransfer();
	}

	if (Status != XST_SUCCESS) {
		xil_printf("      Memory read FAILED at word addr 0x%08X "
			   "(status event %u, %u of %u bytes drained)\r\n",
			   WordAddr, LastStatusEvent, InstancePtr->MsgRxCnt,
			   NumWords * 2U);
		/* TXN_ERROR latches until reset, so clear it before returning */
		HyperRAM_RecoverController(InstancePtr);
		return Status;
	}

	for (i = 0U; i < NumWords; i++) {
		Data[i] = ((u16)MemRxBuffer[2U * i] << 8) |
			  (u16)MemRxBuffer[2U * i + 1U];
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Expected value of one word, derived from its address.
*
* AddrMix is odd and non-zero, which makes every word in the region distinct
* so that a chunk written to the wrong address cannot read back as correct.
*
* @param	Base is the run's base pattern.
* @param	AddrMix is the address multiplier.
* @param	WordAddr is the absolute word address.
*
* @return	Expected 16-bit value at that address.
*
******************************************************************************/
static u16 HyperRAM_ExpectedWord(u16 Base, u16 AddrMix, u32 WordAddr)
{
	return (u16)(Base ^ (u16)(WordAddr * (u32)AddrMix));
}

/*****************************************************************************/
/**
*
* Write a block of words across the test region.
*
* The region is covered in chunk-sized transactions so that each chip-select
* window stays well inside tCSM.
*
* @param	InstancePtr is a pointer to the AmdXspi instance.
* @param	StartWordAddr is the first word address of the region.
* @param	Data is the block of words to write.
* @param	NumWords is the region length in words; must be a multiple of
*		HYPERRAM_CHUNK_WORDS.
* @param	Latency is the initial latency in clocks.
*
* @return	XST_SUCCESS if the whole region was written, otherwise a
*		failure code.
*
******************************************************************************/
static s32 HyperRAM_Write(AmdXspi *InstancePtr, u32 StartWordAddr,
			 const u16 *Data, u32 NumWords, u8 Latency)
{
	s32 Status;
	u32 Offset;

	if ((NumWords % HYPERRAM_CHUNK_WORDS) != 0U) {
		return XST_INVALID_PARAM;
	}

	for (Offset = 0U; Offset < NumWords; Offset += HYPERRAM_CHUNK_WORDS) {
		Status = HyperRAM_WriteMemory(InstancePtr,
					      StartWordAddr + Offset,
					      &Data[Offset],
					      HYPERRAM_CHUNK_WORDS, Latency);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Read a block of words back from the test region.
*
* Chunked for the same reason as the write path: one chip-select window per
* transaction.
*
* @param	InstancePtr is a pointer to the AmdXspi instance.
* @param	StartWordAddr is the first word address of the region.
* @param	Data receives the words read.
* @param	NumWords is the region length in words; must be a multiple of
*		HYPERRAM_CHUNK_WORDS.
* @param	Latency is the initial latency in clocks.
*
* @return	XST_SUCCESS if the whole region was read, otherwise a failure
*		code.
*
******************************************************************************/
static s32 HyperRAM_Read(AmdXspi *InstancePtr, u32 StartWordAddr,
			u16 *Data, u32 NumWords, u8 Latency)
{
	s32 Status;
	u32 Offset;

	if ((NumWords % HYPERRAM_CHUNK_WORDS) != 0U) {
		return XST_INVALID_PARAM;
	}

	for (Offset = 0U; Offset < NumWords; Offset += HYPERRAM_CHUNK_WORDS) {
		Status = HyperRAM_ReadMemory(InstancePtr,
					     StartWordAddr + Offset,
					     &Data[Offset],
					     HYPERRAM_CHUNK_WORDS, Latency);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Compare the data written to the region against the data read back from it,
* reporting the first few mismatches and a pass/fail summary.
*
* Every reported mismatch is followed by a fresh read of the chunk holding it.
* Nothing is written in between, so what comes back says where the corruption
* happened: the expected value means the device holds the right data and the
* first read misdelivered it, while the same wrong value again means the write
* put the wrong data in the array and every read since has faithfully returned
* it. That distinction cannot be drawn from the first read alone, and it
* decides whether the transmit or the receive direction is at fault.
*
* @param	InstancePtr is a pointer to the AmdXspi instance, used for the
*		re-read.
* @param	StartWordAddr is the first word address of the region, used to
*		report a mismatch by device address rather than by index.
* @param	Written is the block of words handed to HyperRAM_Write().
* @param	Read is the block of words returned by HyperRAM_Read().
* @param	NumWords is the region length in words.
* @param	Label names the test in the log.
* @param	Latency is the initial latency in clocks, for the re-read.
*
* @return	XST_SUCCESS if every word matched, otherwise XST_FAILURE.
*
******************************************************************************/
static s32 HyperRAM_Compare(AmdXspi *InstancePtr, u32 StartWordAddr,
			   const u16 *Written, const u16 *Read, u32 NumWords,
			   const char *Label, u8 Latency)
{
	u32 i;
	u32 MismatchCount = 0U;
	u32 ChunkStart;
	u32 ChunkOffset;
	s32 RereadStatus;

	for (i = 0U; i < NumWords; i++) {
		if (Read[i] != Written[i]) {
			MismatchCount++;
			if (MismatchCount <= 8U) {
				xil_printf("    MISMATCH %s word 0x%08X: wrote 0x%04X read 0x%04X\r\n",
					   Label, StartWordAddr + i, Written[i],
					   Read[i]);

				ChunkOffset = i % HYPERRAM_CHUNK_WORDS;
				ChunkStart = StartWordAddr + (i - ChunkOffset);

				RereadStatus = HyperRAM_ReadMemory(InstancePtr,
						ChunkStart, RereadWords,
						HYPERRAM_CHUNK_WORDS, Latency);
				if (RereadStatus != XST_SUCCESS) {
					xil_printf("      re-read FAILED\r\n");
				} else if (RereadWords[ChunkOffset] ==
					   Written[i]) {
					xil_printf("      re-read 0x%04X - matches, "
						   "device holds the right data, "
						   "so the READ path lost it\r\n",
						   RereadWords[ChunkOffset]);
				} else if (RereadWords[ChunkOffset] ==
					   Read[i]) {
					xil_printf("      re-read 0x%04X - same wrong "
						   "value, so the WRITE path stored "
						   "it\r\n",
						   RereadWords[ChunkOffset]);
				} else {
					xil_printf("      re-read 0x%04X - differs from "
						   "both, corruption is not stable\r\n",
						   RereadWords[ChunkOffset]);
				}
			}
		}
	}

	if (MismatchCount != 0U) {
		xil_printf("    *** %s FAILED (%u/%u words differ) ***\r\n",
			   Label, MismatchCount, NumWords);
		return XST_FAILURE;
	}

	xil_printf("    %s PASSED: all %u words match\r\n", Label, NumWords);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Interrupt-driven HyperRAM example: ID read + zeros/ones/fresh RAM test.
*
******************************************************************************/
static s32 AmdXspiIntrExample(UINTPTR BaseAddress)
{
	s32 Status;
	AmdXspi_Config *ConfigPtr;
	u16 Ir0Value = 0U, Ir1Value = 0U, Cr0Value = 0U;
	u8 DeviceFound = 0U;
	u32 MaxPayload;	/* Largest transaction tCSM allows at this bus clock */
	u32 DeviceWords;
	u16 RunSeed;
	u32 Index;

	/*
	 * Step 1: Initialize XSPI controller and interrupt system
	 */
	xil_printf("\r\n[1] Initializing XSPI controller and interrupts...\r\n");

	ConfigPtr = AmdXspi_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		xil_printf("    ERROR: LookupConfig failed\r\n");
		return XST_FAILURE;
	}

	Status = AmdXspi_CfgInitialize(&AmdXspiInstance, ConfigPtr,
				     ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: CfgInitialize failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("    BaseAddress: 0x%08X\r\n", AmdXspiInstance.Config.BaseAddress);
	xil_printf("    SpiMode: %u (5=xSPI Profile 2.0/HyperBus)\r\n",
		   AmdXspiInstance.Config.SpiMode);
	xil_printf("    FifoDepth: %u\r\n", AmdXspiInstance.Config.FifoDepth);
	xil_printf("    NumSlaves: %u\r\n", AmdXspiInstance.Config.NumSlaves);
	xil_printf("    ExtClkFreq: %u MHz (clock into the core)\r\n",
		   AmdXspiInstance.Config.SpiClkFreq);
	PrintSckFreq(&AmdXspiInstance);

	Status = XSetupInterruptSystem(&AmdXspiInstance,
				       &AmdXspi_InterruptHandler,
				       ConfigPtr->IntrId,
				       ConfigPtr->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: XSetupInterruptSystem failed\r\n");
		return XST_FAILURE;
	}

	AmdXspi_SetStatusHandler(&AmdXspiInstance, &AmdXspiInstance, AmdXspiStatusHandler);

	Status = AmdXspi_SetClkMode(&AmdXspiInstance, AMD_XSPI_CLK_MODE_0);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: SetClkMode failed\r\n");
		return XST_FAILURE;
	}

	Status = AmdXspi_SetSlaveSelect(&AmdXspiInstance, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: SetSlaveSelect failed\r\n");
		return XST_FAILURE;
	}

	TxnCompleteCount = 0U;
	ErrorCount = 0U;
	TransferDone = 0;
	TransferError = 0;

	/*
	 * Per-transfer completion/error interrupts are enabled inside
	 * AmdXspi_InterruptTransfer; the GIC/INTC and exception path were wired
	 * up above by the interrupt-system setup.
	 */
	xil_printf("    Interrupt handler registered\r\n");

	/*
	 * Step 2: Wait for HyperRAM power-up. tVCS is the same for every
	 * HyperBus part, so the wait needs no knowledge of which one is fitted
	 * and can precede identification.
	 */
	xil_printf("\r\n[2] Waiting for HyperRAM power-up (tVCS <= %u us)...\r\n",
		   AMD_XSPI_HYPERBUS_TVCS_US);
	usleep(AMD_XSPI_HYPERBUS_TVCS_US + 50U);

	/*
	 * Step 3: Read HyperRAM Identification Registers
	 */
	/*
	 * Reading IR0 needs an initial latency, and the device has not been
	 * identified yet, so the reset value configured for the fitted part is
	 * used. Per datasheet 002-24692 section 6.3.2.2 that reset value also
	 * satisfies tACC up to the part's top frequency, so it holds at
	 * whatever clock the design runs.
	 *
	 * Only the reset default may be used, and there is no second chance. The
	 * device starts driving data the moment its own latency expires, so any
	 * other count reads the wrong position in the burst, and one that is too
	 * short wedges the HyperRAM outright: measurement on an S27KL0642 shows
	 * every later transaction returns zeros no matter what the controller
	 * does, as abort, enable cycling and FIFO resets all leave the part
	 * unresponsive for the rest of the run. The likely cause is chip select
	 * held low past tCSM, which starves the array of refresh, and recovering
	 * from that needs RESET# or a power cycle. So the read is made once: a
	 * device that does not answer is absent or wired wrong, and repeating
	 * the attempt cannot change either.
	 */
	xil_printf("\r\n[3] Reading HyperRAM Identification Registers...\r\n");

	RegLatency = HYPERRAM_DEFAULT_LATENCY_CLOCKS;

	xil_printf("    Reading at the power-on default initial latency of %u clocks,\r\n",
		   RegLatency);
	xil_printf("    which is valid at every supported bus frequency.\r\n\r\n");

	xil_printf("    IR0 at %u latency clocks: ", RegLatency);
	Status = HyperRAM_ReadRegister(&AmdXspiInstance,
				       AMD_XSPI_HYPERRAM_IR0_WORD_ADDR,
				       &Ir0Value);

	if ((Status == XST_SUCCESS) &&
	    (AmdXspi_HyperRamIsValidId(Ir0Value) == TRUE)) {
		xil_printf("0x%04X - VALID, Manufacturer ID [3:0] = 0x%X\r\n",
			   Ir0Value,
			   AmdXspi_HyperRamGetManufacturerId(Ir0Value));
		DeviceFound = 1U;
	} else {
		xil_printf("no valid response (0x%04X)\r\n", Ir0Value);
	}

	if (!DeviceFound) {
		xil_printf("\r\n*** HyperRAM NOT DETECTED ***\r\n");
		xil_printf("If the part powers on at a different initial latency, "
			   "add it to HyperBus_Config_Table[].\r\n");
		return XST_FAILURE;
	}

	/*
	 * Step 4: Read remaining ID/config registers
	 */
	xil_printf("\r\n[4] Reading Additional Registers...\r\n");

	Status = HyperRAM_ReadRegister(&AmdXspiInstance,
				       AMD_XSPI_HYPERRAM_IR1_WORD_ADDR, &Ir1Value);
	if (Status == XST_SUCCESS) {
		xil_printf("    IR1 = 0x%04X, Device Type [3:0] = 0x%X\r\n",
			   Ir1Value, AmdXspi_HyperRamGetDeviceType(Ir1Value));
	}

	/*
	 * Geometry comes from the device itself: IR0 reports the row and column
	 * address bit counts, and density and the address limit are arithmetic
	 * on those. Nothing here needs the device table.
	 */
	DeviceWords = AmdXspi_HyperRamNumWords(Ir0Value);
	xil_printf("    Geometry from IR0: %u/%u row/col bits, %u Mbit (%u words),"
		   " %u rows to refresh\r\n",
		   AmdXspi_HyperRamGetRowAddrBits(Ir0Value),
		   AmdXspi_HyperRamGetColAddrBits(Ir0Value),
		   AmdXspi_HyperRamDensityMbits(Ir0Value), DeviceWords,
		   AmdXspi_HyperRamArrayRows(Ir0Value));

	/*
	 * The device table supplies only what no register reports: what the CR0
	 * latency and burst codes MEAN, and the timing parameters. An unlisted
	 * device still runs using the generic HyperBus defaults, just without
	 * device-specific decoding, so this is a warning rather than a failure.
	 */
	HyperRamCfg = AmdXspi_HyperBusLookupConfig(
			AmdXspi_HyperRamGetManufacturerId(Ir0Value),
			AmdXspi_HyperRamGetDeviceType(Ir1Value));
	xil_printf("    Device table lookup: %s\r\n", AmdXspi_HyperBusName(HyperRamCfg));
	if (HyperRamCfg != NULL) {
		xil_printf("    tVCS %u us, tCSM %u us\r\n",
			   AMD_XSPI_HYPERBUS_TVCS_US, HyperRamCfg->CsLowMaxUs);
	} else {
		xil_printf("    WARNING: device not in amd_xspi_hyperbus_config.h;"
			   " using generic HyperBus defaults\r\n");
	}

	Status = HyperRAM_ReadRegister(&AmdXspiInstance,
				       AMD_XSPI_HYPERRAM_CR0_WORD_ADDR, &Cr0Value);
	if (Status == XST_SUCCESS) {
		xil_printf("    CR0 = 0x%04X\r\n", Cr0Value);
		xil_printf("    [7:4] Latency Code: 0x%X, [3] Fixed: %u,"
			   " [1:0] Burst: %u\r\n",
			   (Cr0Value & AMD_XSPI_HYPERBUS_LATENCY_MASK) >>
			   AMD_XSPI_HYPERBUS_LATENCY_SHIFT,
			   (Cr0Value >> 3) & 1, Cr0Value & 0x3);
	}

	/*
	 * A chunk that overruns tCSM still reports success, so check the
	 * budget rather than waiting for corruption to show up.
	 */
	MaxPayload = AmdXspi_HyperBusMaxPayloadBytes(HyperRamCfg,
					AmdXspi_GetSckFreqMHz(&AmdXspiInstance),
					RegLatency);
	if (MaxPayload == 0U) {
		xil_printf("    tCSM budget: unknown (device or bus clock "
			   "not reported)\r\n");
	} else {
		xil_printf("    tCSM budget: %u bytes per transaction, "
			   "chunk is %u bytes\r\n", MaxPayload,
			   HYPERRAM_CHUNK_WORDS * 2U);
		if ((HYPERRAM_CHUNK_WORDS * 2U) > MaxPayload) {
			xil_printf("    ERROR: chunk exceeds tCSM; refresh is "
				   "starved and data will be lost\r\n");
			return XST_FAILURE;
		}
	}

	/* Keep the test region inside the density the device reported */
	if (((u64)HYPERRAM_TEST_WORD_ADDR + HYPERRAM_TEST_NUM_WORDS) > DeviceWords) {
		xil_printf("\r\n    ERROR: test region exceeds device size\r\n");
		return XST_FAILURE;
	}

	/*
	 * Derive a FRESH write pattern for this run from the value currently
	 * stored in the region, then advance it (new data every run).
	 */
	Status = HyperRAM_ReadMemory(&AmdXspiInstance, HYPERRAM_TEST_WORD_ADDR,
				     ReadWords, HYPERRAM_CHUNK_WORDS,
				     RegLatency);
	if (Status == XST_SUCCESS) {
		RunSeed = (u16)(ReadWords[0] + HYPERRAM_SEED_STEP);
	} else {
		RunSeed = (u16)(HYPERRAM_SEED_FALLBACK + HYPERRAM_SEED_STEP);
	}
	xil_printf("\r\n    This run's write seed (base pattern): 0x%04X\r\n",
		   RunSeed);

	/*
	 * Step 5: RAM test - fresh per-run pattern (data integrity)
	 */
	xil_printf("\r\n[5] RAM test: fresh address-dependent pattern"
		   " (base 0x%04X), %u words at word 0x%08X"
		   " (%u chunks of %u words)...\r\n",
		   RunSeed, HYPERRAM_TEST_NUM_WORDS, HYPERRAM_TEST_WORD_ADDR,
		   HYPERRAM_TEST_NUM_CHUNKS, HYPERRAM_CHUNK_WORDS);
	for (Index = 0U; Index < HYPERRAM_TEST_NUM_WORDS; Index++) {
		WriteWords[Index] = HyperRAM_ExpectedWord(RunSeed,
					HYPERRAM_ADDR_MIX,
					HYPERRAM_TEST_WORD_ADDR + Index);
		ReadWords[Index] = 0U;
	}

	/*
	 * Write the whole region before reading any of it. Verifying each
	 * chunk as it was written would not detect a later write landing on an
	 * address an earlier chunk already occupied, which is the failure this
	 * address-dependent pattern exists to catch.
	 */
	Status = HyperRAM_Write(&AmdXspiInstance, HYPERRAM_TEST_WORD_ADDR,
				WriteWords, HYPERRAM_TEST_NUM_WORDS,
				RegLatency);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: Fresh pattern write failed\r\n");
		return XST_FAILURE;
	}

	Status = HyperRAM_Read(&AmdXspiInstance, HYPERRAM_TEST_WORD_ADDR,
			       ReadWords, HYPERRAM_TEST_NUM_WORDS, RegLatency);
	if (Status != XST_SUCCESS) {
		xil_printf("    ERROR: Fresh pattern read-back failed\r\n");
		return XST_FAILURE;
	}

	Status = HyperRAM_Compare(&AmdXspiInstance, HYPERRAM_TEST_WORD_ADDR,
				  WriteWords, ReadWords,
				  HYPERRAM_TEST_NUM_WORDS, "Fresh pattern",
				  RegLatency);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Step 6: Summary (including interrupt activity)
	 */
	xil_printf("\r\n[6] Summary:\r\n");
	xil_printf("    HyperRAM Device: DETECTED\r\n");
	xil_printf("    Device:          %s\r\n", AmdXspi_HyperBusName(HyperRamCfg));
	xil_printf("    Manufacturer:    0x%X\r\n",
		   AmdXspi_HyperRamGetManufacturerId(Ir0Value));
	xil_printf("    IR0:             0x%04X\r\n", Ir0Value);
	xil_printf("    IR1:             0x%04X\r\n", Ir1Value);
	xil_printf("    CR0:             0x%04X\r\n", Cr0Value);
	xil_printf("    Memory test:     FRESH PATTERN WRITE-READ-VERIFY PASSED\r\n");
	xil_printf("    Interrupts seen: %u transaction-complete, %u error\r\n",
		   TxnCompleteCount, ErrorCount);

	if (ErrorCount != 0U) {
		xil_printf("    WARNING: error interrupt(s) were observed\r\n");
		return XST_FAILURE;
	}

	/*
	 * Step 7: Disable interrupts
	 */
	xil_printf("\r\n[7] Disabling interrupts...\r\n");
	AmdXspi_IntrDisable(&AmdXspiInstance, AMD_XSPI_INTR_ALL_MASK);
	xil_printf("    Interrupts disabled\r\n");

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* Status callback for the message-based transfer API. Runs in
* interrupt context; it only sets flags/counters (no prints) so it does not
* perturb bus timing. XST_SPI_TRANSFER_DONE marks a successful queue
* completion; any other event is treated as an error.
******************************************************************************/
static void AmdXspiStatusHandler(void *CallBackRef, u32 StatusEvent,
			      u32 ByteCount)
{
	(void)CallBackRef;
	(void)ByteCount;

	LastStatusEvent = StatusEvent;

	if (StatusEvent == (u32)XST_SPI_TRANSFER_DONE) {
		TxnCompleteCount++;
		TransferDone = 1;
	} else {
		ErrorCount++;
		TransferError = 1;
	}
}

/*****************************************************************************/
/**
* Wait for the in-flight interrupt-driven transfer to finish, spinning on the
* flags set by the status callback. Returns XST_SUCCESS on completion,
* XST_FAILURE on an error event or timeout.
******************************************************************************/
static s32 WaitForTransfer(void)
{
	u32 Timeout = AMD_XSPI_TRANSFER_WAIT_LIMIT;

	while ((TransferDone == 0) && (TransferError == 0)) {
		if (Timeout == 0U) {
			return XST_FAILURE;
		}
		Timeout--;
	}

	if (TransferError != 0) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
