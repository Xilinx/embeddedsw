/*!
 * @file te_fsbl_hooks.c
 * @author Antti Lukats
 * @copyright 2015 Trenz Electronic GmbH
 */

#include "fsbl.h"
#include "xstatus.h"

#include "xparameters.h"


#include "te_fsbl_config.h"
#include "te_fsbl_hooks.h"

#include "xemacps.h"


#include "xdevcfg.h"
#include "xil_cache.h"
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define DCFG_DEVICE_ID		XPAR_XDCFG_0_DEVICE_ID

/**
 * @name Configuration Type1 packet headers masks
 * @{
 */
#define XDC_TYPE_SHIFT			29
#define XDC_REGISTER_SHIFT		13
#define XDC_OP_SHIFT			27
#define XDC_TYPE_1			1
#define OPCODE_READ			1
/* @} */

/*
 * Addresses of the Configuration Registers
 */
#define CRC		0	/* Status Register */
#define FAR		1	/* Frame Address Register */
#define FDRI		2	/* FDRI Register */
#define FDRO		3	/* FDRO Register */
#define CMD		4	/* Command Register */
#define CTL0		5	/* Control Register 0 */
#define MASK		6	/* MASK Register */
#define STAT		7	/* Status Register */
#define LOUT		8	/* LOUT Register */
#define COR0		9	/* Configuration Options Register 0 */
#define MFWR		10	/* MFWR Register */
#define CBC		11	/* CBC Register */
#define IDCODE		12	/* IDCODE Register */
#define AXSS		13	/* AXSS Register */
#define COR1		14	/* Configuration Options Register 1 */
#define WBSTAR		15	/* Warm Boot Start Address Register */
#define TIMER		16	/* Watchdog Timer Register */
#define BOOTSTS		17	/* Boot History Status Register */
#define CTL1		18	/* Control Register 1 */

/*
 * Mask For IDCODE
 */
#define IDCODE_MASK   0x0FFFFFFF

/************************** Variable Definitions *****************************/

XDcfg DcfgInstance;		/* Device Configuration Interface Instance */

/*****************************************************************************/


#ifdef TE_INIT_VIDEO


#endif


#ifdef UBOOT_ENV_MAGIC


/*
 * read IDCODE over PCAP
 *
 */

u32 XDcfg_RegAddr(u8 Register, u8 OpCode, u8 Size)
{
	/*
	 * Type 1 Packet Header Format
	 * The header section is always a 32-bit word.
	 *
	 * HeaderType | Opcode | Register Address | Reserved | Word Count
	 * [31:29]	[28:27]		[26:13]	     [12:11]     [10:0]
	 * --------------------------------------------------------------
	 *   001 	  xx 	  RRRRRRRRRxxxxx	RR	xxxxxxxxxxx
	 *
	 * �R� means the bit is not used and reserved for future use.
	 * The reserved bits should be written as 0s.
	 *
	 * Generating the Type 1 packet header which involves sifting of Type 1
	 * Header Mask, Register value and the OpCode which is 01 in this case
	 * as only read operation is to be carried out and then performing OR
	 * operation with the Word Length.
	 */
	return ( ((XDC_TYPE_1 << XDC_TYPE_SHIFT) |
		(Register << XDC_REGISTER_SHIFT) |
		(OpCode << XDC_OP_SHIFT)) | Size);
}


int XDcfg_GetConfigReg(XDcfg *DcfgInstancePtr, u32 ConfigReg, u32 *RegData)
{
	u32 IntrStsReg;
	u32 StatusReg;
	unsigned int CmdIndex;
	unsigned int CmdBuf[18];

	/*
	 * Clear the interrupt status bits
	 */
	XDcfg_IntrClear(DcfgInstancePtr, (XDCFG_IXR_PCFG_DONE_MASK |
			XDCFG_IXR_D_P_DONE_MASK | XDCFG_IXR_DMA_DONE_MASK));

	/* Check if DMA command queue is full */
	StatusReg = XDcfg_ReadReg(DcfgInstancePtr->Config.BaseAddr,
				XDCFG_STATUS_OFFSET);
	if ((StatusReg & XDCFG_STATUS_DMA_CMD_Q_F_MASK) ==
			XDCFG_STATUS_DMA_CMD_Q_F_MASK) {
		return XST_FAILURE;
	}

	/*
	 * Register Readback in non secure mode
	 * Create the data to be written to read back the
	 * Configuration Registers from PL Region.
	 */
	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x000000BB; 	/* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x11220044; 	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0xFFFFFFFF; 	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0xAA995566; 	/* Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = XDcfg_RegAddr(ConfigReg,OPCODE_READ,0x1);
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */
	CmdBuf[CmdIndex++] = 0x20000000; 	/* Type 1 NOOP Word 0 */

	XDcfg_Transfer(&DcfgInstance, (&CmdBuf[0]),
			CmdIndex, RegData, 1, XDCFG_PCAP_READBACK);

	/* Poll IXR_DMA_DONE */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) !=
			XDCFG_IXR_DMA_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	/* Poll IXR_D_P_DONE */
	while ((IntrStsReg & XDCFG_IXR_D_P_DONE_MASK) !=
			XDCFG_IXR_D_P_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	CmdIndex = 0;
	CmdBuf[CmdIndex++] = 0x30008001;	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x0000000D;	/* Bus Width Sync Word */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Dummy Word */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Bus Width Detect */
	CmdBuf[CmdIndex++] = 0x20000000;	/* Dummy Word */

	XDcfg_InitiateDma(DcfgInstancePtr, (u32)(&CmdBuf[0]),
				XDCFG_DMA_INVALID_ADDRESS, CmdIndex, 0);

	/* Poll IXR_DMA_DONE */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) !=
			XDCFG_IXR_DMA_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	/* Poll IXR_D_P_DONE */
	while ((IntrStsReg & XDCFG_IXR_D_P_DONE_MASK) !=
			XDCFG_IXR_D_P_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstancePtr);
	}

	return XST_SUCCESS;
}


u32 te_read_IDCODE(void)
{
		int Status;
		unsigned int ValueBack;
		unsigned int tmval;

		XDcfg_Config *ConfigPtr;

		/*
		 * Initialize the Device Configuration Interface driver.
		 */
		ConfigPtr = XDcfg_LookupConfig(XPAR_XDCFG_0_DEVICE_ID);

		/*
		 * This is where the virtual address would be used, this example
		 * uses physical address.
		 */
		Status = XDcfg_CfgInitialize(&DcfgInstance, ConfigPtr,
						ConfigPtr->BaseAddr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Run the Self test.
		 */
		Status = XDcfg_SelfTest(&DcfgInstance);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}


		if (XDcfg_GetConfigReg(&DcfgInstance, IDCODE, (u32 *)&ValueBack) !=
			XST_SUCCESS) {
			return XST_FAILURE;
		}
		xil_printf("\t\r\n Device IDCODE -> \t %x \t\r\n", ValueBack );

		tmval=(ValueBack & 0xF0000000)>>28;
		xil_printf(" Revision -> \t %x \t\r\n", tmval);
		// tmval=(ValueBack & 0x0FE00000)>>21;
		// xil_printf(" Family -> \t %x \t\r\n", tmval);
		// tmval=(ValueBack & 0x001E0000)>>17;
		// xil_printf(" SubFamily -> \t %x \t\r\n", tmval);
		tmval=(ValueBack & 0x0001F000)>>12;
		if (((ValueBack & 0x0001F000)>>12)==0x08) { xil_printf(" Device -> \t %x (7z014s)\t\r\n", tmval);}
		else if (((ValueBack & 0x0001F000)>>12)==0x07) {  xil_printf(" Device -> \t %x (7z020)\t\r\n", tmval);}
    else {  xil_printf(" Device -> \t %x (wrong type for TE0720 FSBL!)\t\r\n", tmval);}
		
		// tmval=(ValueBack & 0x00000FFE)>>1;
		// xil_printf(" MANUFACTURER_ID -> \t %x \t\r\n", tmval);

		return XST_SUCCESS;
}

/******************************************************************************
* Convert a 4-bit value to hexadecimal representation (lowercase letters).
*
* @param x 4-bit value to be converted.
* @return Hexadecimal representation.
****************************************************************************/
static char Uint4ToHex(const unsigned char x)
{
	if (x<10) {
		return x + '0';
	} else {
		return x + ('a' - 10);
	}
}

/******************************************************************************
* Convert an octet to the hexadecimal representation (lowercase letters). A null byte is not appended.
*
* @param s Buffer to store hexadecimal representation. At least two bytes must be available.
* @param x Octet to be converted.
****************************************************************************/
static void Uint8ToHex(char* s, const unsigned char x)
{
	s[0] = Uint4ToHex((x >> 4) & 0x0F);
	s[1] = Uint4ToHex((x >> 0) & 0x0F);
}

/******************************************************************************
* Convert MAC address to the environment string for the U-Boot to read the MAC address from.
* Example: "ethaddr=00:0a:35:00:00:05\n"
*
* @param env Buffer to store the environment string. Must have at least 27 bytes available.
* @param mac_addr MAC address, 6 bytes.
****************************************************************************/
static void MacToUbootEnvironment(char* env, const unsigned char* mac_addr)
{
	// end of "ethaddr=".
	const int idx0 = 8;
	const int N_MAC = 6;
	int i;

	strcpy(env, "ethaddr=");
	for (i=0; i<N_MAC; ++i) {
		const int idx = idx0 + 3*i;
		Uint8ToHex(&env[idx], mac_addr[i]);
		env[idx+2] = ':';
	}
	env[idx0 + 3 * N_MAC - 1] = '\n';
	env[idx0 + 3 * N_MAC - 0] = 0;
}

#endif











/*
 * Add this function to FsblHookBeforeHandoff in fsbl_hooks.c
 *
 */
u32 te_FsblHookBeforeHandoff(void) {


	u32 Status;
	Status = XST_SUCCESS;
  u32 device;

	u16 rval16;
	u8 speed_grade;
	u8 pcb_rev;
	unsigned char temp_grade;
	unsigned char model1;
	unsigned char model2;
	unsigned char model3;

	XEmacPs Emac;
	XEmacPs_Config *Mac_Config;

	unsigned char mac_addr[6];
	int i = 0;


#ifdef TE_VIDEO_INIT

//	Status = adv7511_init(XPAR_XIICPS_0_DEVICE_ID, 0x00, ADV7511_ADDR);

	Status = tpg_init(XPAR_V_TPG_0_DEVICE_ID);	
	Status = vtc_init(XPAR_VTC_0_DEVICE_ID);
	Status = vdma_init(XPAR_AXI_VDMA_0_DEVICE_ID);

#endif


#ifdef TE0720
	Status = te_read_IDCODE();


	Mac_Config = XEmacPs_LookupConfig(XPAR_PS7_ETHERNET_0_DEVICE_ID); if(Mac_Config == NULL) { return XST_FAILURE; }

	Status = XEmacPs_CfgInitialize(&Emac, Mac_Config, Mac_Config->BaseAddress); if(Status != XST_SUCCESS){ return XST_FAILURE; }
    /*
     * Read out MAC Address bytes
	 */
	Status = XEmacPs_PhyRead(&Emac, 0x1A,  9, &rval16); if(Status != XST_SUCCESS){ return XST_FAILURE; }
	mac_addr[0] = (unsigned char)(rval16 >> 8);	
        mac_addr[1] = (unsigned char)(rval16 & 0xFF);
	Status = XEmacPs_PhyRead(&Emac, 0x1A,  10, &rval16); if(Status != XST_SUCCESS){	return XST_FAILURE; }
	mac_addr[2] = (unsigned char)(rval16 >> 8);	
        mac_addr[3] = (unsigned char)(rval16 & 0xFF);
	Status = XEmacPs_PhyRead(&Emac, 0x1A,  11, &rval16); if(Status != XST_SUCCESS){	return XST_FAILURE; }
	mac_addr[4] = (unsigned char)(rval16 >> 8);	
        mac_addr[5] = (unsigned char)(rval16 & 0xFF);

    /*
     * Decode SoM model and version information!
	 */
	// Read register 3
	Status = XEmacPs_PhyRead(&Emac, 0x1A,  3, &rval16); if(Status != XST_SUCCESS){	return XST_FAILURE; }
	pcb_rev = (rval16 >>10) & 0x7;

	// Read register 4
	Status = XEmacPs_PhyRead(&Emac, 0x1A,  4, &rval16); if(Status != XST_SUCCESS){	return XST_FAILURE; }


	speed_grade = (rval16 >> 14) & 3;
	/* 0=C, 1=E, 2=I, 3=A */
	if ((rval16 & 0x3000)==0x0000) { temp_grade = 0x43; }
	else if ((rval16 & 0x3000)==0x1000) { temp_grade = 0x45; }
	else if ((rval16 & 0x3000)==0x2000) { temp_grade = 0x49; }
	else if ((rval16 & 0x3000)==0x3000) { temp_grade = 0x41; }
  else { temp_grade = 0x20; }
  
  if ((rval16 & 0x0F00)==0x000) { model1 = 0x20;model2 = 0x20;model3 = 0x46; } 
  else if ((rval16 & 0x0F00)==0x100) { model1 = 0x20;model2 = 0x20;model3 = 0x52; }
  else if ((rval16 & 0x0F00)==0x200) { model1 = 0x20;model2 = 0x4C;model3 = 0x46; }
  else if ((rval16 & 0x0F00)==0x300) { model1 = 0x31;model2 = 0x34;model3 = 0x53; }
  else { model1 = 0x31;model2 = 0x31;model3 = 0x31; }
  
  
	xil_printf("\n\rSoM: TE0720-0%d-%d%c%c%c%c SC REV:%02x", pcb_rev, speed_grade, temp_grade, model1, model2, model3, rval16 & 0xFF);
	xil_printf("\n\rMAC: ");

	for(i = 0; i < 6; i++) {
		xil_printf("%02x ", mac_addr[i]);
	}
	xil_printf("\n\r");
#endif

	/*
	 * Write MAC Address to OCM memory for u-boot to import!
	 *
	 */

	//strcpy(0xFFFFFC04, "ethaddr=00:0a:35:00:00:05\n" );
#ifdef UBOOT_ENV_MAGIC
	Xil_Out32(UBOOT_ENV_MAGIC_ADDR, UBOOT_ENV_MAGIC); // Magic!
	MacToUbootEnvironment((char*)UBOOT_ENV_ADDR, mac_addr);
#endif


    /*
     * Set MAC Address in PS7 IP Core registers
	 */
	Status = XEmacPs_SetMacAddress(&Emac, mac_addr, 1); if(Status != XST_SUCCESS){ return XST_FAILURE; }



    /*
     * Marvell PHY Config
	 */

        /* Select Page 18 */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  0x16, 0x0012); if(Status != XST_SUCCESS){ return XST_FAILURE; }
        /* Disable fiber/SGMII Autodetect */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  0x14, 0x8210); if(Status != XST_SUCCESS){ return XST_FAILURE; }

        /* Select Page 3 */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  0x16, 0x0003); if(Status != XST_SUCCESS){ return XST_FAILURE; }
        /* LED0: On link, Blink activity, LED1 Receive, LED2 Transmit */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  16, 0x0501); if(Status != XST_SUCCESS){ return XST_FAILURE; }
        /* LED polarity positive, push-pull on=high */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  17, 0x4415); if(Status != XST_SUCCESS){ return XST_FAILURE; }


        /* Select Page 0 */
	Status = XEmacPs_PhyWrite(&Emac, 0x00,  0x16, 0x0000); if(Status != XST_SUCCESS){ return XST_FAILURE; }
	
#ifdef TE0720
    /*
     * SC LED remap
     * Green = ETH LED 0, Red = MIO7, NOSEQ output = ETH LED0
	 */
	Status = XEmacPs_PhyWrite(&Emac, 0x1A,  5, 0x0041); if(Status != XST_SUCCESS){ return XST_FAILURE; }

    /*
     * Reset pulse to USB PHY
	 */
	Status = XEmacPs_PhyWrite(&Emac, 0x1A,  7, 0x0010); if(Status != XST_SUCCESS){ return XST_FAILURE; }
	Status = XEmacPs_PhyWrite(&Emac, 0x1A,  7, 0x0000); if(Status != XST_SUCCESS){ return XST_FAILURE; }
#endif


	return (Status);


}
