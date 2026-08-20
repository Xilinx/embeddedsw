/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_hyperbus_config.h
*
* This file contains the HyperBus device configuration table and the related
* defines. It should be included by the AXI XSPI examples and compiled along
* with them (*.c), in the same way xqspipsu_flash_config.h is used by the
* QSPIPSU examples.
*
* This file supports HyperRAM over HyperBus. The table and its accessors are
* arranged so that further HyperRAM parts, from Infineon or any other vendor,
* are added as data alone.
*
* <b>Why a device table</b>
* The AXI XSPI driver is protocol-agnostic: in AXI-Lite mode it only moves
* Command/Address and data bytes. Everything that is device specific therefore
* lives in the application.
*
* Only the parameters that NO register reports belong here. Anything a device
* can be asked about is read from it at runtime:
*
*   Read from the device, never tabulated:
*     Manufacturer ID and geometry from IR0, device type from IR1, and the
*     current latency/burst/fixed-latency from CR0. Density, array rows and
*     the address limit are arithmetic on the IR0 row and column bit counts.
*
*   Tabulated here, because no register conveys it:
*     - What a latency code MEANS in clocks, and the power-on default
*     - What a burst code MEANS in bytes
*     - Power-up time, tCSM and the refresh interval
*     - A human readable part name
*
* Because geometry is never stored, one entry covers every density in a family.
*
* <b>The bootstrap problem</b>
* A HyperBus read uses the device's current initial latency - including the
* read that discovers what the device is. Unlike SFDP, which NOR flash exposes
* through a command with a spec-fixed dummy count, HyperBus offers no
* known-latency entry point.
*
* The host does not choose this number freely, it has to reproduce the one the
* device is programmed to. Per datasheet 002-24692 section 4.2, once the
* latency clocks named by CR0[7:4] have elapsed "the memory starts to
* simultaneously transition the RWDS and output the target data", and it keeps
* streaming for as long as the clock runs. The device never waits for the host,
* so counting too few clocks samples before the data arrives and counting too
* many samples past the words that have already gone by. Both give wrong data.
*
* What makes the identifying read possible is that CR0[7:4] comes out of reset
* at the family maximum, a value the datasheet states and the integrator can
* therefore supply before any register has been read. Section 6.3.2.2 adds that
* this reset value satisfies tACC all the way to the part's top frequency, so
* no bus clock a design can legally run needs it lowered first. Lowering it
* afterwards is a throughput optimisation, not a requirement, and the frequency
* the design runs at never enters into the identifying read.
*
* That value is not taken from this table, because the table cannot be consulted
* until the device is known. Each example names it as
* HYPERRAM_DEFAULT_LATENCY_CLOCKS and the integrator sets it for the part fitted
* to the board. The table does not record it per entry: the HyperBus
* specification fixes the reset value at 7 clocks for every part, and a
* successful identifying read is itself proof that the configured value matched
* the device, so a per entry copy could only ever disagree by being stale.
*
* Getting it wrong in the short direction is worse than wrong data. Measurement
* on an S27KL0642 shows every later transaction returning zeros, with neither
* aborting the transfer nor cycling the controller bringing the part back: chip
* select held past tCSM starves the array of refresh, and only RESET# or a
* power cycle clears it. A part whose reset default is unknown therefore cannot
* be identified by trial - a wrong guess is unrecoverable in software, and there
* is no second attempt to spend. Hence the value is configured rather than
* searched for.
*
* C_TGT_LATENCY is no help here, which is why neither the driver nor these
* examples read it. A Lite mode design cannot set it, so it stays at the IP
* default of 4 - a value outside the parameter's own documented range of 7,
* and one no supported device uses at power-on. The same applies to
* C_TGT_MEM_SIZE and C_TGT_BURST_LENGTH: geometry, latency and burst length
* come from the identified device, not from the design.
*
* <b>Adding a new device</b>
* Append one entry to HyperBus_Config_Table[]. Nothing else needs to change,
* because the entry holds only the identifiers, a name and tCSM: no latency or
* burst code is decoded, so nothing in it is a per part encoding. A board
* fitted with a part whose reset latency differs from the one the example is
* set for needs HYPERRAM_DEFAULT_LATENCY_CLOCKS changed to match, and the
* bootstrap note above explains why trying each in turn instead is not an
* option on a board that cannot assert RESET# between attempts.
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

#ifndef AMD_XSPI_HYPERBUS_CONFIG_H_	/**< prevent circular inclusions */
#define AMD_XSPI_HYPERBUS_CONFIG_H_	/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "amd_xspi.h"

/************************** Constant Definitions *****************************/

/*
 * HyperBus Command/Address (CA) field definitions.
 *
 * These are defined by the HyperBus (xSPI Profile 2.0) protocol and are the
 * same for every vendor, so they are NOT part of the per-device table.
 *   CA[47]    = R/W#   (1 = Read, 0 = Write)
 *   CA[46]    = Address Space (0 = Memory, 1 = Register)
 *   CA[45]    = Burst type (1 = Linear, 0 = Wrapped)
 *   CA[44:16] = Row and upper column address (word address bits [31:3])
 *   CA[15:3]  = Reserved (must be 0)
 *   CA[2:0]   = Lower column address (word address bits [2:0])
 *
 * A register write to word address 0x555, for example, encodes as
 * CA = 0x600000AA0005: CA[44:16] = 0xAA gives address bits [31:3] (0x550) and
 * CA[2:0] = 5 supplies the low three bits.
 */
#define AMD_XSPI_HYPERBUS_CA_BYTES			6U	/**< A HyperBus command/address
							     word is always 6 bytes */

/** 8D-8D-8D moves a byte on each edge of SCK, so two bytes per clock */
#define AMD_XSPI_HYPERBUS_BYTES_PER_CLOCK		2U

/*
 * tVCS: the self-initialisation the device performs once VCC and VCCQ are
 * valid and RESET# is high, during which no transaction is allowed. The
 * HyperBus parts all quote the same 150 us maximum, so this is not per-device
 * data. It is timed from the supplies coming up rather than from anything
 * software does, so a design whose rails settled long before the processor
 * started has already served it; the wait costs nothing and covers the cold
 * boot case where configuration finishes first.
 */
#define AMD_XSPI_HYPERBUS_TVCS_US			150U

/*
 * HyperRAM register space word addresses. These are HyperBus-standard and
 * common to all vendors, so they are not per-device either.
 */
#define AMD_XSPI_HYPERRAM_IR0_WORD_ADDR		0x00000000U	/**< Identification
								     Register 0 */
#define AMD_XSPI_HYPERRAM_IR1_WORD_ADDR		0x00000001U	/**< Identification
								     Register 1 */
#define AMD_XSPI_HYPERRAM_CR0_WORD_ADDR		0x00000800U	/**< Configuration
								     Register 0 */

/*
 * ID Register 0 field definitions (HyperBus standard layout).
 * The manufacturer VALUE is vendor specific and lives in the table below.
 *
 * NOTE: the row and column fields are encoded as (bit count - 1). Infineon
 * datasheet 002-24692 Table 7 spells this out: 00000b means one row address
 * bit and 11111b means thirty-two, 0000b means one column address bit and
 * 1111b means sixteen. A 64 Mb part therefore reports 01100b/1000b for its 13
 * row and 9 column address bits. Always go through the accessors below rather
 * than masking these fields by hand.
 */
#define AMD_XSPI_HYPERRAM_IR0_MFR_MASK		0x000FU	/**< Manufacturer ID mask */
#define AMD_XSPI_HYPERRAM_IR0_MFR_SHIFT		0U	/**< Manufacturer ID shift */
#define AMD_XSPI_HYPERRAM_IR0_COL_BITS_MASK	0x00F0U	/**< Column bit count mask */
#define AMD_XSPI_HYPERRAM_IR0_COL_BITS_SHIFT	4U	/**< Column bit count shift */
#define AMD_XSPI_HYPERRAM_IR0_ROW_BITS_MASK	0x1F00U	/**< Row bit count mask */
#define AMD_XSPI_HYPERRAM_IR0_ROW_BITS_SHIFT	8U	/**< Row bit count shift */

/*
 * Plausible range for the total word address bit count (row + column) decoded
 * from IR0. Used to reject a garbled ID read. The upper bound is 32 because
 * the HyperBus Command/Address field carries a 32-bit word address
 * (CA[44:16] plus CA[2:0]).
 */
#define AMD_XSPI_HYPERRAM_MIN_WORD_ADDR_BITS	16U	/**< Fewest row+column bits
							     a sane IR0 can report */
#define AMD_XSPI_HYPERRAM_MAX_WORD_ADDR_BITS	32U	/**< Most row+column bits the
							     CA field can address */

/* ID Register 1 field definitions */
#define AMD_XSPI_HYPERRAM_IR1_DEV_TYPE_MASK	0x000FU	/**< Device type mask */
#define AMD_XSPI_HYPERRAM_IR1_DEV_TYPE_SHIFT	0U	/**< Device type shift */
#define AMD_XSPI_HYPERRAM_DEV_TYPE_HYPERRAM_2_0	0x1U	/**< Device type value
							     for HyperRAM 2.0 */

/*
 * Latency code field definition.
 *
 * The bit POSITION is fixed by HyperBus: Configuration Register 0 carries the
 * latency code at [7:4]. What a given code means in clocks is per device and
 * is not decoded here, because the examples run at the reset latency
 * throughout: the identifying read only succeeds at that value, and neither
 * example writes CR0 afterwards, so the code read back can only be the reset
 * one. The raw code is reported as-is.
 */
#define AMD_XSPI_HYPERBUS_LATENCY_MASK		0x00F0U	/**< CR0 latency code mask */
#define AMD_XSPI_HYPERBUS_LATENCY_SHIFT		4U	/**< CR0 latency code shift */

/**************************** Type Definitions *******************************/

/**
 * HyperBus device description.
 *
 * One entry per supported device, looked up at runtime from the manufacturer
 * and device identifiers read out of the part - IR0 and IR1 for HyperRAM.
 *
 * This holds ONLY what the device cannot report about itself:
 *
 *   - Geometry is absent. Density, row/column bits, array rows and the address
 *     limit are all decoded from IR0 by the accessors below, so one entry
 *     serves every density in a family.
 *   - Directly readable register fields are not duplicated.
 *
 * Adding a device is therefore data, not code: append an entry and everything
 * else follows.
 */
typedef struct {
	/* Identity - ManufacturerId and DeviceType form the lookup key */
	u8 ManufacturerId;	/**< IR0[3:0] */
	u8 DeviceType;		/**< IR1[3:0] */
	const char *Name;	/**< Human readable device name */

	u32 CsLowMaxUs;		/**< tCSM: max CS# LOW time, imposed by
				     refresh */
} AmdXspi_HyperBusConfig;

/************************** Variable Definitions *****************************/

/*
 * Supported HyperBus devices.
 *
 * To add a device, append an entry with that part's tCSM taken from its
 * datasheet.
 */
static const AmdXspi_HyperBusConfig HyperBus_Config_Table[] = {
	/*
	 * Infineon S27KL0642 / S27KS0642 - 64Mb HYPERRAM self-refresh DRAM.
	 * Datasheet 002-24692 Rev. *I. A 64 Mb part reports ID0 = 0x0C81
	 * (13 row address bits, 9 column address bits, manufacturer 0x1); other
	 * densities in the family differ only in the IR0 geometry fields.
	 */
	{
		.ManufacturerId		= 0x1U,
		.DeviceType		= AMD_XSPI_HYPERRAM_DEV_TYPE_HYPERRAM_2_0,
		.Name			= "Infineon HYPERRAM (S27KL/S27KS family)",

		.CsLowMaxUs		= 4U	/* tCSM at TCASE <= 85C,
						   1us at 105C */
	}
};

/** Number of entries in HyperBus_Config_Table */
#define AMD_XSPI_HYPERBUS_NUM_SUPPORTED_DEVICES \
	(sizeof(HyperBus_Config_Table) / sizeof(HyperBus_Config_Table[0]))

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* Extract the manufacturer ID from an ID Register 0 value.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Manufacturer code from IR0[3:0].
*
******************************************************************************/
static INLINE u8 AmdXspi_HyperRamGetManufacturerId(u16 Ir0Value)
{
	return (u8)((Ir0Value & AMD_XSPI_HYPERRAM_IR0_MFR_MASK) >>
		    AMD_XSPI_HYPERRAM_IR0_MFR_SHIFT);
}

/*****************************************************************************/
/**
*
* Extract the device type from an ID Register 1 value.
*
* @param[in] Ir1Value is the raw IR1 register value.
*
* @return	Device type from IR1[3:0].
*
******************************************************************************/
static INLINE u8 AmdXspi_HyperRamGetDeviceType(u16 Ir1Value)
{
	return (u8)((Ir1Value & AMD_XSPI_HYPERRAM_IR1_DEV_TYPE_MASK) >>
		    AMD_XSPI_HYPERRAM_IR1_DEV_TYPE_SHIFT);
}

/*****************************************************************************/
/**
*
* Extract the row address bit count from an ID Register 0 value.
*
* IR0[12:8] is encoded as (count - 1), so 01100b reports thirteen row address
* bits.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Row address bit count (1 to 32).
*
******************************************************************************/
static INLINE u8 AmdXspi_HyperRamGetRowAddrBits(u16 Ir0Value)
{
	return (u8)(((Ir0Value & AMD_XSPI_HYPERRAM_IR0_ROW_BITS_MASK) >>
		     AMD_XSPI_HYPERRAM_IR0_ROW_BITS_SHIFT) + 1U);
}

/*****************************************************************************/
/**
*
* Extract the column address bit count from an ID Register 0 value.
*
* IR0[7:4] is encoded as (count - 1), so 1000b reports nine column address
* bits.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Column address bit count (1 to 16).
*
******************************************************************************/
static INLINE u8 AmdXspi_HyperRamGetColAddrBits(u16 Ir0Value)
{
	return (u8)(((Ir0Value & AMD_XSPI_HYPERRAM_IR0_COL_BITS_MASK) >>
		     AMD_XSPI_HYPERRAM_IR0_COL_BITS_SHIFT) + 1U);
}

/*****************************************************************************/
/**
*
* Return the total number of word address bits reported by ID Register 0.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Row plus column address bit count.
*
******************************************************************************/
static INLINE u8 AmdXspi_HyperRamGetWordAddrBits(u16 Ir0Value)
{
	return (u8)(AmdXspi_HyperRamGetRowAddrBits(Ir0Value) +
		    AmdXspi_HyperRamGetColAddrBits(Ir0Value));
}

/*****************************************************************************/
/**
*
* Check whether an ID Register 0 value looks like a real HyperRAM response.
*
* Used to decide whether the identifying read found a device at all. An absent
* or mis-wired part, or one left unresponsive by an earlier bad access, shows
* up as an all zeros or all ones word, an unassigned manufacturer nibble, or a
* nonsensical density.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	TRUE if the value is plausible, FALSE otherwise.
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperRamIsValidId(u16 Ir0Value)
{
	u8 ManufacturerId;
	u8 WordAddrBits;

	/* Classic "device absent or held in reset" bus patterns */
	if ((Ir0Value == 0x0000U) || (Ir0Value == 0xFFFFU)) {
		return (u32)FALSE;
	}

	ManufacturerId = AmdXspi_HyperRamGetManufacturerId(Ir0Value);
	if ((ManufacturerId == 0x0U) || (ManufacturerId == 0xFU)) {
		return (u32)FALSE;
	}

	WordAddrBits = AmdXspi_HyperRamGetWordAddrBits(Ir0Value);
	if ((WordAddrBits < AMD_XSPI_HYPERRAM_MIN_WORD_ADDR_BITS) ||
	    (WordAddrBits > AMD_XSPI_HYPERRAM_MAX_WORD_ADDR_BITS)) {
		return (u32)FALSE;
	}

	return (u32)TRUE;
}

/*****************************************************************************/
/**
*
* Return the number of array rows that must be refreshed, decoded from IR0.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Row count (2 ^ row address bits).
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperRamArrayRows(u16 Ir0Value)
{
	return (u32)1U << AmdXspi_HyperRamGetRowAddrBits(Ir0Value);
}

/*****************************************************************************/
/**
*
* Find the table entry describing an attached HyperBus device.
*
* @param[in] ManufacturerId is IR0[3:0].
* @param[in] DeviceType is IR1[3:0].
*
* @return	Pointer to the matching entry, or NULL if the device is not in
*		the table. A NULL return does not mean the part is absent, only
*		that its parameters are unknown to this build.
*
******************************************************************************/
static INLINE const AmdXspi_HyperBusConfig *AmdXspi_HyperBusLookupConfig(
		u8 ManufacturerId, u8 DeviceType)
{
	u32 Index;

	for (Index = 0U; Index < AMD_XSPI_HYPERBUS_NUM_SUPPORTED_DEVICES; Index++) {
		if ((HyperBus_Config_Table[Index].ManufacturerId == ManufacturerId) &&
		    (HyperBus_Config_Table[Index].DeviceType == DeviceType)) {
			return &HyperBus_Config_Table[Index];
		}
	}

	return NULL;
}

/*****************************************************************************/
/**
*
* Largest payload one transaction may carry without holding chip select low
* past tCSM.
*
* CS# stays low for the command/address phase, the initial latency and then
* the data, all clocked at SCK, and HyperBus moves two bytes per clock. The
* latency is counted at 2x because the fixed-latency setting doubles it and
* that is the worst case.
*
* Overrunning tCSM starves the array of refresh. Nothing reports it: no error
* bit is raised and the transaction appears to succeed, so the budget has to
* be checked rather than observed.
*
* @param[in] ConfigPtr is the device entry, or NULL if the device is unknown.
* @param[in] SckFreqMHz is the bus clock, from AmdXspi_GetSckFreqMHz().
* @param[in] LatencyClocks is the initial latency in clocks.
*
* @return	Largest safe payload in bytes, or 0 if the device or bus clock
*		is unknown, or the overhead alone already fills the budget.
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperBusMaxPayloadBytes(const AmdXspi_HyperBusConfig *ConfigPtr,
						u32 SckFreqMHz, u32 LatencyClocks)
{
	u32 BudgetClocks;
	u32 OverheadClocks;

	if ((ConfigPtr == NULL) || (SckFreqMHz == 0U)) {
		return 0U;
	}

	/* Microseconds multiplied by MHz gives clocks */
	BudgetClocks = ConfigPtr->CsLowMaxUs * SckFreqMHz;

	OverheadClocks = (AMD_XSPI_HYPERBUS_CA_BYTES /
			  AMD_XSPI_HYPERBUS_BYTES_PER_CLOCK) +
			 (LatencyClocks * 2U);

	if (BudgetClocks <= OverheadClocks) {
		return 0U;
	}

	return (BudgetClocks - OverheadClocks) *
	       AMD_XSPI_HYPERBUS_BYTES_PER_CLOCK;
}

/*****************************************************************************/
/**
*
* Return the number of addressable words, decoded from IR0.
*
* HyperRAM is word (16-bit) addressable and the total number of word address
* bits is the sum of the row and column address bit counts, so the device holds
* 2^(RowAddrBits + ColAddrBits) words.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Word count, or 0 if the reported geometry does not fit in 32
*		bits (which AmdXspi_HyperRamIsValidId already rejects).
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperRamNumWords(u16 Ir0Value)
{
	u8 Bits = AmdXspi_HyperRamGetWordAddrBits(Ir0Value);

	if (Bits >= 32U) {
		return 0U;
	}

	return (u32)1U << Bits;
}

/*****************************************************************************/
/**
*
* Return the highest valid word address, decoded from IR0.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Maximum word address.
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperRamMaxWordAddr(u16 Ir0Value)
{
	u8 Bits = AmdXspi_HyperRamGetWordAddrBits(Ir0Value);

	if (Bits >= 32U) {
		return 0xFFFFFFFFU;
	}

	return ((u32)1U << Bits) - 1U;
}

/*****************************************************************************/
/**
*
* Return the density in Mbits, decoded from IR0.
*
* @param[in] Ir0Value is the raw IR0 register value.
*
* @return	Density in Mbits, or 0 if the geometry is out of range.
*
******************************************************************************/
static INLINE u32 AmdXspi_HyperRamDensityMbits(u16 Ir0Value)
{
	u8 Bits = AmdXspi_HyperRamGetWordAddrBits(Ir0Value);

	/* words * 16 bits / (1024 * 1024); below 16 address bits rounds to 0 */
	if ((Bits < 16U) || (Bits >= 32U)) {
		return 0U;
	}

	return (u32)1U << (Bits - 16U);
}

/*****************************************************************************/
/**
*
* Return a display name for a device.
*
* @param[in] ConfigPtr is the device entry, or NULL if the device is unknown.
*
* @return	Device name, or a generic string when the device is not in the
*		table.
*
******************************************************************************/
static INLINE const char *AmdXspi_HyperBusName(const AmdXspi_HyperBusConfig *ConfigPtr)
{
	if (ConfigPtr == NULL) {
		return "Unknown/unsupported HyperBus device";
	}

	return ConfigPtr->Name;
}

#ifdef __cplusplus
}
#endif

#endif /* AMD_XSPI_HYPERBUS_CONFIG_H_ */
