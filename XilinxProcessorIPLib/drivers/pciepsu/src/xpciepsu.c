/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/
/******************************************************************************/
/**
* @file xpciepsu.c
*
* Implements all of functions for psu_pci IP driver except interrupts and
* initialization
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
/******************************** Include Files *******************************/
#include "xpciepsu.h"
#include "xpciepsu_ht.h"
#include "xpciepsu_common.h"
#include "sleep.h"
/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/


/* Command register offsets */

/* Memory access enable */
#define PCIE_CFG_CMD_MEM_EN 		0x00000002

/* Bus master enable */
#define PCIE_CFG_CMD_BUSM_EN 		0x00000004

/* PCIe Configuration registers offsets */

/* Vendor ID/Device ID offset */
#define PCIE_CFG_ID_REG 			0x0000

/* Command/Status Register Offset */
#define PCIE_CFG_CMD_STATUS_REG 	0x0001

/* Primary/Sec.Bus Register Offset */
#define PCIE_CFG_PRI_SEC_BUS_REG 	0x0006

/* Cache Line/Latency Timer / Header Type / BIST Register Offset */
#define PCIE_CFG_CAH_LAT_HD_REG 	0x0003

#define PCIE_CFG_BAR_MEM_TYPE_MASK 	0x1

/* PCIe Base Addr */
#define PCIE_CFG_BAR_BASE_OFFSET 	0x0004

/* PCIe Base Addr 0 */
#define PCIE_CFG_BAR_0_REG 			0x0004

/* PCIe Base Addr 1 */
#define PCIE_CFG_BAR_1_REG 			0x0005

/* PCIe Base Addr 2 */
#define PCIE_CFG_BAR_2_REG 			0x0006

/* PCIe Base Addr 3 */
#define PCIE_CFG_BAR_3_REG 			0x0007

/* PCIe Base Addr 4 */
#define PCIE_CFG_BAR_4_REG 			0x0008

#define PCIE_CFG_BUS_NUMS_T1_REG 	0X0006
#define PCIE_CFG_NP_MEM_T1_REG 		0X0008
#define PCIE_CFG_P_MEM_T1_REG 		0X0009
#define PCIE_CFG_P_UPPER_MEM_T1_REG 0X000A
#define PCIE_CFG_P_LIMIT_MEM_T1_REG 0X000B

#define PCIE_CFG_FUN_NOT_IMP_MASK 	0xFFFF
#define PCIE_CFG_HEADER_TYPE_MASK 	0x00EF0000
#define PCIE_CFG_MUL_FUN_DEV_MASK 	0x00800000

#define PCIE_CFG_MAX_NUM_OF_BUS 	256
#define PCIE_CFG_MAX_NUM_OF_DEV 	32
#define PCIE_CFG_MAX_NUM_OF_FUN 	8

#define PCIE_CFG_HEADER_O_TYPE 		0x0000

#define PCI_BAR_IO_MEM 				1
#define PCI_BAR_ADDR_MEM 			0

#define PCI_BAR_MEM_TYPE_64 		1
#define PCI_BAR_MEM_TYPE_32 		0
#define PCI_PRIMARY_BUS   			0x18

#define MB_SHIFT 					20
#define HEX_NIBBLE 					4
#define TWO_HEX_NIBBLES 			8
#define FOUR_HEX_NIBBLES 			16
#define EIGHT_HEX_NIBBLES 			32

#define LINKUP_SUCCESS				1
#define LINKUP_FAIL					0

#define DATA_MASK_32				(0xFFFFFFFF)

/**************************** Variable Definitions ****************************/


/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function looks for phy link is up or not
*
* @param   InstancePtr pointer to XPciePsu_Config Instance
*
* @return  1 if link is up
*          0 if link is down
*
*******************************************************************************/
static int psu_phy_link_up(XPciePsu_Config *CfgPtr)
{
	if (XPciepsu_ReadReg(CfgPtr->PciReg, PS_LINKUP_OFFSET)
	    & PHY_RDY_LINKUP_BIT)
		return LINKUP_SUCCESS;
	return LINKUP_FAIL;
}

/******************************************************************************/
/**
* This function looks for PCIe link is up or not
*
* @param   InstancePtr pointer to XPciePsu_Config Instance
*
* @return  1 if link is up
*          0 if link is down
*
*******************************************************************************/
static int psu_pcie_link_up(XPciePsu_Config *CfgPtr)
{
	if (XPciepsu_ReadReg(CfgPtr->PciReg, PS_LINKUP_OFFSET)
	    & PCIE_PHY_LINKUP_BIT)
		return LINKUP_SUCCESS;
	return LINKUP_FAIL;
}

/******************************************************************************/
/**
* This function waits for phy link to come up till LINK_WAIT_MAX_RETRIES times.
*
* @param   InstancePtr pointer to XPciePsu_Config Instance
*
* @return  0 if link is up
*          -1 if link never came up
*
*******************************************************************************/
static int psu_wait_for_link(XPciePsu_Config *CfgPtr)
{
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		if (psu_phy_link_up(CfgPtr))
			return LINKUP_SUCCESS;

		usleep(LINK_WAIT_USLEEP_MIN);
	}

	pciepsu_dbg("PHY link never came up\n");

	return LINKUP_FAIL;
}

/******************************************************************************/
/**
* This function initializes PCIe bridge. Enables/Disables required channels,
* checks for links, ECAM, Interrupts.
*
* @param   InstancePtr pointer to XPciePsu Instance
*
* @return  0 if success
*          error value on failure
*
*******************************************************************************/
static int psu_pci_bridge_init(XPciePsu *InstancePtr)
{
	XPciePsu_Config *CfgPtr;
	u32 breg_val, ecam_val, first_busno = 0, ecam_val_min, ecam_sz;
	int err;

	CfgPtr = &(InstancePtr->Config);

	Xil_AssertNonvoid(CfgPtr != NULL);

	breg_val = XPciepsu_ReadReg(CfgPtr->BrigReg, E_BREG_CAPABILITIES)
		   & BREG_PRESENT;
	if (!breg_val) {
		pciepsu_dbg("BREG is not present\n");
		return breg_val;
	}

	/* Write bridge_off to breg base */
	XPciepsu_WriteReg(CfgPtr->BrigReg, E_BREG_BASE_LO,
			  LOWER_32_BITS(CfgPtr->BrigReg));

	XPciepsu_WriteReg(CfgPtr->BrigReg, E_BREG_BASE_HI,
			  UPPER_32_BITS(CfgPtr->BrigReg));

	/* Enable BREG */
	XPciepsu_WriteReg(CfgPtr->BrigReg, E_BREG_CONTROL,
			  ((~BREG_ENABLE_FORCE) & BREG_ENABLE));

	/* Disable DMA channel registers */
	XPciepsu_WriteReg(CfgPtr->BrigReg, BRCFG_PCIE_RX0,
			  (XPciepsu_ReadReg(CfgPtr->BrigReg, BRCFG_PCIE_RX0)
			   | CFG_DMA_REG_BAR));

	/* Enable Ingress subtractive decode translation */
	XPciepsu_WriteReg(CfgPtr->BrigReg, I_ISUB_CONTROL, SET_ISUB_CONTROL);

	/* Enable msg filtering details */
	XPciepsu_WriteReg(CfgPtr->BrigReg, BRCFG_PCIE_RX_MSG_FILTER,
			  CFG_ENABLE_MSG_FILTER_MASK);

	/* Check for linkup */
	err = psu_wait_for_link(CfgPtr);
	if (err != LINKUP_SUCCESS)
		return err;

        ecam_val = XPciepsu_ReadReg(CfgPtr->BrigReg, E_ECAM_CAPABILITIES) &
                E_ECAM_PRESENT;
	if (!ecam_val) {
		pciepsu_dbg("ECAM is not present\n");
		return ecam_val;
	}

	/* Enable ECAM */
	XPciepsu_WriteReg(CfgPtr->BrigReg, E_ECAM_CONTROL,
                          XPciepsu_ReadReg(CfgPtr->BrigReg, E_ECAM_CONTROL) |
                          E_ECAM_CR_ENABLE);

	XPciepsu_WriteReg(
		CfgPtr->BrigReg, E_ECAM_CONTROL,
                XPciepsu_ReadReg(CfgPtr->BrigReg, E_ECAM_CONTROL) |
                    (PSU_ECAM_VALUE_DEFAULT << E_ECAM_SIZE_SHIFT));

	XPciepsu_WriteReg(CfgPtr->BrigReg, E_ECAM_BASE_LO,
			  LOWER_32_BITS(CfgPtr->Ecam));

	XPciepsu_WriteReg(CfgPtr->BrigReg, E_ECAM_BASE_HI,
			  UPPER_32_BITS(CfgPtr->Ecam));

	/* Get bus range */
	ecam_val = XPciepsu_ReadReg(CfgPtr->BrigReg, E_ECAM_CONTROL);
	ecam_val_min = XPciepsu_ReadReg(CfgPtr->BrigReg, E_ECAM_CAPABILITIES);

        ecam_sz = 0x1 << (((ecam_val & E_ECAM_SIZE_LOC) >> E_ECAM_SIZE_SHIFT) +
                          ((ecam_val_min & E_ECAM_SIZE_MIN) >>
                           E_ECAM_SIZE_SHIFT));

	/* Configure last bus numbers as max possible bus number */
	InstancePtr->last_busno = GET_MAX_BUS_NO(ecam_sz);

	/* Write primary, secondary and subordinate bus numbers */
	ecam_val = first_busno;
	ecam_val |= (first_busno + 1) << 8;
	ecam_val |= (InstancePtr->last_busno << E_ECAM_SIZE_SHIFT);

	XPciepsu_WriteReg(CfgPtr->Ecam, PCI_PRIMARY_BUS, ecam_val);

	/* check link up */
	if (psu_pcie_link_up(CfgPtr) == LINKUP_SUCCESS) {
		pciepsu_dbg("Link is UP\n");
	} else {
		pciepsu_dbg("Link is DOWN\n");
		return XST_FAILURE;
	}

	/* Disable all misc interrupts */
	XPciepsu_WriteReg(CfgPtr->BrigReg, MSGF_MISC_MASK,
			  (u32)~MSGF_MISC_SR_MASKALL);

	/* Disable all legacy interrupts */
	XPciepsu_WriteReg(CfgPtr->BrigReg, MSGF_LEG_MASK,
			  (u32)~MSGF_LEG_SR_MASKALL);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function Composes configuration space location
*
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset
*
* @return  location address of the composed address
*
*******************************************************************************/
u32 XPciePsu_ComposeExternalConfigAddress(u8 Bus, u8 Device, u8 Function,
					  u16 Offset)
{
	u32 Location = 0;

        Location |= ((((u32)Bus) << PCIEPSU_ECAM_BUS_SHIFT) &
                     PCIEPSU_ECAM_BUS_MASK);

        Location |= ((((u32)Device) << PCIEPSU_ECAM_DEV_SHIFT) &
                     PCIEPSU_ECAM_DEV_MASK);

        Location |= ((((u32)Function) << PCIEPSU_ECAM_FUN_SHIFT) &
                     PCIEPSU_ECAM_FUN_MASK);

        Location |= ((((u32)Offset) << PCIEPSU_ECAM_REG_SHIFT) &
                     PCIEPSU_ECAM_REG_MASK);

	Location &= PCIEPSU_ECAM_MASK;

	return Location;
}

/******************************************************************************/
/**
* This function read from remote configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset	location of the address to read data from.
* @param   DataPtr	pointer store date available in the offset
*
* @return  none
*
*******************************************************************************/
void XPciePsu_ReadRemoteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				    u8 Function, u16 Offset, u32 *DataPtr)
{
	u32 Location = 0;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(DataPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (((Bus == 0) && !((Device == 0) && (Function == 0)))
	    || (Bus > InstancePtr->last_busno)) {
		*DataPtr = DATA_MASK_32;
		return;
	}

	/* Compose function configuration space location */
	Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
							 Offset);

	/* Read data from that location */
	Data = XPciepsu_ReadReg((InstancePtr->Config.Ecam), Location);

	*DataPtr = Data;
}

/******************************************************************************/
/**
* This function write to remote configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance
* @param   Bus
* @param   Device
* @param   Function
* @param   Offset	location of the address to write data.
* @param   Data to be written on to the offset
*
* @return  none
*
*******************************************************************************/
void XPciePsu_WriteRemoteConfigSpace(XPciePsu *InstancePtr, u8 Bus, u8 Device,
				     u8 Function, u16 Offset, u32 Data)
{
	u32 Location = 0;
	u32 TestWrite = 0;
	u8 Count = 3;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((Bus == 0) || (Bus > InstancePtr->last_busno))
		return;

	/* Compose function configuration space location */
        Location = XPciePsu_ComposeExternalConfigAddress(Bus, Device, Function,
                                                         Offset);

	/* Write data to that location */
	XPciepsu_WriteReg((InstancePtr->Config.Ecam), Location, Data);

	/* Read data from that location to verify write */
	while (Count) {
		TestWrite =
			XPciepsu_ReadReg((InstancePtr->Config.Ecam), Location);

		if (TestWrite == Data)
			break;

		Count--;
	}
}

static int PositionRightmostSetbit(u64 size)
{
	int position = 0;
	int bit = 1;

	/* ignore 4 bits */
	size = size & (~(0xf));

	while (!(size & bit)) {
		bit = bit << 1;
		position++;
	}

	return position;
}
/******************************************************************************/
/**
* This function reserves bar memory address.
*
* @param   InstancePtr pointer to XPciePsu Instance
* @param   mem_type type of bar memory. address mem or IO.
* @param   mem_as bar memory tpye 32 or 64 bit
* @param   size	u64 size to increase
*
* @return  bar address
*
*******************************************************************************/
static u64 reserve_bar_mem(XPciePsu *InstancePtr, u8 mem_type, u8 mem_as,
			   u64 size)
{
	u64 ret = 0;

	if (mem_type == PCI_BAR_IO_MEM)
		return XST_FAILURE;

	if (mem_as == PCI_BAR_MEM_TYPE_64) {
		ret = InstancePtr->Config.P_mem;
		InstancePtr->Config.P_mem = InstancePtr->Config.P_mem + size;
		Xil_AssertNonvoid(InstancePtr->Config.P_mem <=
				InstancePtr->Config.P_mem_max);
	} else {
		ret = InstancePtr->Config.NP_mem;
		InstancePtr->Config.NP_mem = InstancePtr->Config.NP_mem + size;
		Xil_AssertNonvoid(InstancePtr->Config.NP_mem <=
				InstancePtr->Config.NP_mem_max);
	}

	return ret;
}

/******************************************************************************/
/**
* This function Composes configuration space location
*
* @param   InstancePtr pointer to XPciePsu Instance
* @param   headerType u32 type0 or type1 header
* @param   Bus
* @param   Device
* @param   Function
*
* @return  int XST_SUCCESS on success
*          err on fail
*
*******************************************************************************/
static int alloc_bar_space(XPciePsu *InstancePtr, u32 headertype, u8 Bus,
			   u8 Device, u8 Function)
{
	u32 data = DATA_MASK_32;
	u32 Location = 0, Location_1 = 0;
	u32 size = 0, size_1 = 0, TestWrite;
	u8 mem_as, mem_type;
	u64 bar_addr;
	u32 tmp, *p;
	u8 bar_no;

	u32 ConfigData;
	u8 max_bars = 0;
	u64 bars[MAX_BARS] = {};

	if (headertype == PCIE_CFG_HEADER_O_TYPE) {
		/* For endpoints */
		max_bars = 6;
	} else {
		/* For Bridge*/
		max_bars = 2;
	}

	for (bar_no = 0; bar_no < max_bars; bar_no++) {
		/* Compose function configuration space location */
		Location = XPciePsu_ComposeExternalConfigAddress(
			Bus, Device, Function,
			PCIE_CFG_BAR_BASE_OFFSET + bar_no);

		/* Write data to that location */
		XPciepsu_WriteReg((InstancePtr->Config.Ecam), Location, data);

		size = XPciepsu_ReadReg((InstancePtr->Config.Ecam), Location);
		if ((size & (~(0xf))) == 0x00) {
			/* return saying that BAR is not implemented */
			pciepsu_dbg(
				"bus: %d, device: %d, function: %d: BAR %d is "
				"not implemented\r\n",
				Bus, Device, Function, bar_no);
			continue;
		}

		/* check for IO space or memory space */
		if (size & PCIE_CFG_BAR_MEM_TYPE_MASK) {
			/* Device required IO address space */
			mem_type = PCI_BAR_IO_MEM;
			pciepsu_dbg(
				"bus: %d, device: %d, function: %d: BAR %d "
				"required IO space; it is unassigned\r\n",
				Bus, Device, Function, bar_no);
			continue;
		} else {
			/* Device required memory address space */
			mem_type = PCI_BAR_ADDR_MEM;
		}

		/* check for 32 bit AS or 64 bit AS */
		if ((size & 0x6) == 0x4) {
			/* 64 bit AS is required */
			mem_as = PCI_BAR_MEM_TYPE_64;

			/* Compose function configuration space location */
			Location_1 = XPciePsu_ComposeExternalConfigAddress(
				Bus, Device, Function,
				PCIE_CFG_BAR_BASE_OFFSET + (bar_no + 1));

			/* Write data to that location */
			XPciepsu_WriteReg((InstancePtr->Config.Ecam),
					  Location_1, data);

			/* get next bar if 64 bit address is required */
			size_1 = XPciepsu_ReadReg((InstancePtr->Config.Ecam),
						  Location_1);

			/* Merge two bars for size */
			p = (u32 *)&bar_addr;
			*p = size;
			*(p + 1) = size_1;

			TestWrite = PositionRightmostSetbit(bar_addr);

			/* actual bar size is 2 << TestWrite */
			bar_addr =
				reserve_bar_mem(InstancePtr, mem_type, mem_as,
						(2 << (TestWrite - 1)));

			tmp = (u32)bar_addr;

			/* Write actual bar address here */
			XPciepsu_WriteReg((InstancePtr->Config.Ecam), Location,
					  tmp);

			tmp = (u32)(bar_addr >> 32);

			/* Write actual bar address here */
			XPciepsu_WriteReg((InstancePtr->Config.Ecam),
					  Location_1, tmp);
			pciepsu_dbg(
				"bus: %d, device: %d, function: %d: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, bar_no, bar_addr,
				((2 << (TestWrite - 1)) / 1024));
		} else {
			/* 32 bit AS is required */
			mem_as = PCI_BAR_MEM_TYPE_32;

			TestWrite = PositionRightmostSetbit(size);

			/* actual bar size is 2 << TestWrite */
			bar_addr =
				reserve_bar_mem(InstancePtr, mem_type, mem_as,
						(2 << (TestWrite - 1)));

			tmp = (u32)bar_addr;

			/* Write actual bar address here */
			XPciepsu_WriteReg((InstancePtr->Config.Ecam), Location,
					  tmp);
			pciepsu_dbg(
				"bus: %d, device: %d, function: %d: BAR %d, "
				"ADDR: 0x%p size : %dK\r\n",
				Bus, Device, Function, bar_no, bar_addr,
				((2 << (TestWrite - 1)) / 1024));
		}
		bars[bar_no] = (u64)bar_addr;

		/* no need to probe next bar if present BAR requires 64 bit AS
		 */
		if ((size & 0x6) == 0x4)
			bar_no = bar_no + 1;
	}

	/* write BAR address to hash table */
	XPciePsu_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
				       Function, PCIE_CFG_ID_REG,
				       &ConfigData);
	ht_set(ConfigData, bars, Bus, Device, Function);
	return XST_SUCCESS;
}

/******************************************************************************/
/**
* This function increments to next 1Mb page starting position of
* non prefetchable memory
*
* @param   	InstancePtr pointer to XPciePsu Instance
*
*******************************************************************************/
static void increamentNpMem(XPciePsu *InstancePtr)
{
	InstancePtr->Config.NP_mem >>= MB_SHIFT;
	InstancePtr->Config.NP_mem++;
	InstancePtr->Config.NP_mem <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function increments to next 1Mb block starting position of
* prefetchable memory
*
* @param  	InstancePtr pointer to XPciePsu Instance
*
*******************************************************************************/
static void increamentPMem(XPciePsu *InstancePtr)
{
	InstancePtr->Config.P_mem >>= MB_SHIFT;
	InstancePtr->Config.P_mem++;
	InstancePtr->Config.P_mem <<= MB_SHIFT;
}

/******************************************************************************/
/**
* This function starts enumeration of PCIe Fabric on the system.
* Assigns primary, secondary and subordinate bus numbers.
* Assigns memory to prefetchable and non-prefetchable memory locations.
* enables end-points and bridges.
*
* @param   	InstancePtr pointer to XPciePsu Instance
* @param   	bus_num	to scans for connected bridges/endpoints on it.
*
* @return  	none
*
*******************************************************************************/
static void fetchDevicesInBus(XPciePsu *InstancePtr, u32 bus_num)
{
	u32 ConfigData;
	static u32 last_bus_num;

	u16 PCIeVendorID;
	u16 PCIeDeviceID;
	u32 PCIeHeaderType;
	u32 PCIeMultiFun;

	u32 Adr06; /* Latency timer */
	u32 Adr08;
	u32 Adr09;
	u32 Adr0A;
	u32 Adr0B;

	int ret;

	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (bus_num > InstancePtr->last_busno) {
		/* End of bus size */
		return;
	}

	for (u32 PCIeDevNum = 0; PCIeDevNum < PCIE_CFG_MAX_NUM_OF_DEV;
	     PCIeDevNum++) {
		for (u32 PCIeFunNum = 0; PCIeFunNum < PCIE_CFG_MAX_NUM_OF_FUN;
		     PCIeFunNum++) {

			/* Vendor ID */
			XPciePsu_ReadRemoteConfigSpace(
				InstancePtr, bus_num, PCIeDevNum, PCIeFunNum,
				PCIE_CFG_ID_REG, &ConfigData);

			PCIeVendorID = (u16)(ConfigData & 0xFFFF);
			PCIeDeviceID = (u16)((ConfigData >> 16) & 0xFFFF);

			if (PCIeVendorID == PCIE_CFG_FUN_NOT_IMP_MASK) {
				if (PCIeFunNum == 0)
					/*
					 * We don't need to look
					 * any further on this device.
					 */
					break;
			} else {
				pciepsu_dbg(
					"\n\rPCIeBus is %02X\r\nPCIeDev is "
					"%02X\r\nPCIeFunc is %02X\r\n",
					bus_num, PCIeDevNum, PCIeFunNum);

				pciepsu_dbg(
					"Vendor ID is %04X \r\nDevice ID is "
					"%04X\r\n",
					PCIeVendorID, PCIeDeviceID);

				/* Header Type */
				XPciePsu_ReadRemoteConfigSpace(
					InstancePtr, bus_num, PCIeDevNum,
					PCIeFunNum, PCIE_CFG_CAH_LAT_HD_REG,
					&ConfigData);

				PCIeHeaderType =
					ConfigData & PCIE_CFG_HEADER_TYPE_MASK;
				PCIeMultiFun =
					ConfigData & PCIE_CFG_MUL_FUN_DEV_MASK;

				if (PCIeHeaderType == PCIE_CFG_HEADER_O_TYPE) {
					/* This is an End Point */
					pciepsu_dbg("This is an End Point\r\n");

					/*
					 * Write Address to PCIe BAR
					 */
					ret = alloc_bar_space(
						InstancePtr, PCIeHeaderType,
						bus_num, PCIeDevNum,
						PCIeFunNum);
					if (ret != 0)
						return;

					/*
					 * Initialize this end point
					 * and return.
					 */
					XPciePsu_ReadRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (PCIE_CFG_CMD_BUSM_EN
						       | PCIE_CFG_CMD_MEM_EN);

					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_CMD_STATUS_REG,
						ConfigData);

					pciepsu_dbg(
						"End Point has been "
						"enabled\r\n");

					increamentNpMem(InstancePtr);
					increamentPMem(InstancePtr);

				} else {
					/* This is a bridge */
					pciepsu_dbg("This is a Bridge\r\n");

					/* alloc bar space and configure bridge
					 */
					ret = alloc_bar_space(
						InstancePtr, PCIeHeaderType,
						bus_num, PCIeDevNum,
						PCIeFunNum);

					if (ret != 0)
						continue;

					Adr06 = 0x0; /* Latency timer */
					Adr08 = 0x0;
					Adr09 = 0x0;
					Adr0A = 0x0;
					Adr0B = 0x0;

					/* Sets primary and secondary bus
					 * numbers */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= 0xFF; /* sub ordinate bus no 0xF
						     */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= (++last_bus_num); /* secondary
							      bus no */
					Adr06 <<= TWO_HEX_NIBBLES;
					Adr06 |= bus_num; /* Primary bus no */
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_BUS_NUMS_T1_REG,
						Adr06);

					/* Update start values of P and NP MMIO
					 * base */
					Adr08 |= ((InstancePtr->Config.NP_mem
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_NP_MEM_T1_REG, Adr08);

					Adr09 |= ((InstancePtr->Config.P_mem
						   & 0xFFF00000)
						  >> FOUR_HEX_NIBBLES);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_P_MEM_T1_REG, Adr09);
					Adr0A |= (InstancePtr->Config.P_mem
						  >> EIGHT_HEX_NIBBLES);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_P_UPPER_MEM_T1_REG,
						Adr0A);

					/* Searches secondary bus devices. */
					fetchDevicesInBus(InstancePtr,
							  last_bus_num);

					/*
					 * update subordinate bus no
					 * clearing subordinate bus no
					 */
					Adr06 &= (~(0xFF << FOUR_HEX_NIBBLES));
					/* setting subordinate bus no */
					Adr06 |= (last_bus_num
						  << FOUR_HEX_NIBBLES);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_BUS_NUMS_T1_REG,
						Adr06);

					/*
					 * Update end values of MMIO limit
					 */

					/*
					 * Align memory to 1 Mb boundry.
					 *
					 * eg. 0xE000 0000 is the base address. Increments
					 * 1 Mb which gives 0xE010 0000 and writes to limit.
					 * So the final value at DW08(in pcie type 1 header)
					 * is 0xE010 E000.
					 * So the range is 0xE000 0000 to 0xE01F FFFF.
					 *
					 */
					increamentNpMem(InstancePtr);
					Adr08 |= (InstancePtr->Config.NP_mem
						  & 0xFFF00000);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_NP_MEM_T1_REG, Adr08);

					increamentPMem(InstancePtr);
					Adr09 |= (InstancePtr->Config.P_mem
						  & 0xFFF00000);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_P_MEM_T1_REG, Adr09);
					Adr0B |= (InstancePtr->Config.P_mem
						  >> EIGHT_HEX_NIBBLES);
					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_P_LIMIT_MEM_T1_REG,
						Adr0B);

					/* Increment P & NP mem to next aligned starting address.
					 *
					 * Eg: As the range is 0xE000 0000 to 0xE01F FFFF.
					 * the next starting address should be 0xE020 0000.
					 */
					increamentNpMem(InstancePtr);
					increamentPMem(InstancePtr);

					/*
					 * Enable configuration
					 */
					XPciePsu_ReadRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_CMD_STATUS_REG,
						&ConfigData);

					ConfigData |= (PCIE_CFG_CMD_BUSM_EN
						       | PCIE_CFG_CMD_MEM_EN);

					XPciePsu_WriteRemoteConfigSpace(
						InstancePtr, bus_num,
						PCIeDevNum, PCIeFunNum,
						PCIE_CFG_CMD_STATUS_REG,
						ConfigData);
				}
			}
			if ((!PCIeFunNum) && (!PCIeMultiFun)) {
				/*
				 * If it is function 0 and it is not a
				 * multi function device, we don't need
				 * to look any further on this devie
				 */
				break;
			}
		}
	}
}

/******************************************************************************/
/**
* This function starts PCIe enumeration.
*
* @param    InstancePtr pointer to XPciePsu Instance
*
* @return 	none
*
*******************************************************************************/
void XPciePsuEnumerate_Fabric(XPciePsu *InstancePtr)
{
	fetchDevicesInBus(InstancePtr, 0);
}

/******************************************************************************/
/**
* This function initializes the config space and PCIe bridge.
*
* @param   InstancePtr pointer to XPciePsu Instance
* @param   CfgPtr pointer to XPciePsu_Config instrance.
* @param   EffectiveBrgAddress config brigReg address
*
* @return  XST_SUCCESS on success
*          err on failure
*
*******************************************************************************/
u32 XPciePsu_CfgInitialize(XPciePsu *InstancePtr, XPciePsu_Config *CfgPtr,
			   UINTPTR EffectiveBrgAddress)
{
	u32 ret;

	/* Assert arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Clear instance memory and make copy of configuration */
	memset(InstancePtr, 0, sizeof(XPciePsu));
	memcpy(&(InstancePtr->Config), CfgPtr, sizeof(XPciePsu_Config));

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	InstancePtr->Config.BrigReg = EffectiveBrgAddress;

	/* Initialize AXI-PCIe bridge */
	ret = psu_pci_bridge_init(InstancePtr);
	if (ret != XST_SUCCESS) {
		pciepsu_dbg("Pciepsu rc enumeration failed\n");
		return ret;
	}

	return XST_SUCCESS;
}
