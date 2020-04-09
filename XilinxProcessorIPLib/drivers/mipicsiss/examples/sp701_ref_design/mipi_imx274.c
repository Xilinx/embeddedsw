/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file mipi_imx274.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* X.XX  XX     YY/MM/DD
* 1.00  RHe    19/09/20 Initial release.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "function_prototype.h"
#include "imx274_cfgs.h"
#include "xstatus.h"
#include "sleep.h"
#include "xiic_l.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xv_tpg.h"


#ifdef XPAR_INTC_0_DEVICE_ID
 #include "xintc.h"
#else
 #include "xscugic.h"
#endif



/************************** Constant Definitions *****************************/

/* Uncomment or comment the following depending on the board used*/
#define KC705
//#define ZC702
//#define ZCU102

#ifdef KC705
    //#define MIG_7SERIES_BASEADDRESS0  XPAR_MIG_7SERIES_0_BASEADDR
   //#define MIG_7SERIES_BASEADDRESS1  XPAR_MIG_7SERIES_0_BASEADDR + 1FFFFFFF
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_0_IIC_1_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x3C
	#define SENSOR_ADDRESS 		0x3C

	#define IIC_MUX_ADDRESS 	0x74
	#define IIC_FMC_CHANNEL		0x07

	#define IIC_MUX_ENABLE		0
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_CSISS_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 1920
	#define HEIGHT 1080
	#define LANES 2
#endif


#ifdef ZC702
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_1_IIC_2_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_1_IIC_2_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_AXI_IIC_0_IIC2INTC_IRPT_INTR
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_AXI_IIC_1_IIC2INTC_IRPT_INTR
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x20
	#define SENSOR_ADDRESS 		0x10

	#define IIC_MUX_ADDRESS 	0x74
	#define IIC_FMC_CHANNEL		0x3F

	#define IIC_MUX_ENABLE
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_MIPI_CSI2_RX_SUBSYSTEM_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 640
	#define HEIGHT 480
	#define LANES 4
#endif


#ifdef ZCU102
	#define IIC_FMC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
	#define IIC_ADAPTER_DEVICE_ID	XPAR_IIC_1_DEVICE_ID

	#ifdef XPAR_INTC_0_DEVICE_ID
	 #define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_INTC_0_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_INTC_0_IIC_1_VEC_ID
	 #define INTC			XIntc
	 #define INTC_HANDLER		XIntc_InterruptHandler
	#else
	 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
	 #define IIC_FMC_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define IIC_ADAPTER_INTR_ID	XPAR_FABRIC_IIC_0_VEC_ID
	 #define INTC			XScuGic
	 #define INTC_HANDLER		XScuGic_InterruptHandler
	#endif

	#define FMC_ADDRESS 		0x3C
	#define ADAPTER_ADDRESS 	0x20
	#define SENSOR_ADDRESS 		0x10

	#define IIC_MUX_ADDRESS 	0x75
	#define IIC_FMC_CHANNEL		0x03

	#define IIC_MUX_ENABLE
	#define PAGE_SIZE   16
	#define FMC_TEST_START_ADDRESS   	128
	#define ADAPTER_TEST_START_ADDRESS   	01

	#define CSI_SS_BOARD 0
	#define CSI_SS_SENSOR 		XPAR_CSISS_0_BASEADDR
	#define CSI_TIMER_SENSOR 	XPAR_AXI_TIMER_0_BASEADDR
	#define WIDTH 640
	#define HEIGHT 480
	#define LANES 4

#endif

#define IIC_BASE_ADDRESS	XPAR_IIC_2_BASEADDR

#define EEPROM_TEST_START_ADDRESS	0x80

 #define EEPROM_ADDRESS		0x50

#define PAGE_SIZE	16

#define IIC_SLAVE_ADDRESS	1

#define IIC_SWITCH_ADDRESS 0x74
#define IIC_SI570_ADDRESS  0x5D
#define IIC_FMC_HPC_ADDRESS 0x70
#define IIC_FMC_LPC_ADDRESS 0x70
#define IIC_EEPROM_ADDRESS 0x54
#define IIC_SFP_ADDRESS 0x50
#define IIC_ADV7511_ADDRESS 0x39
#define IIC_DDR3_SPD_ADDRESS 0x50
#define IIC_DDR3_TEMP_ADDRESS 0x18
#define IIC_SI5326_ADDRESS 0x50

#define IIC_BUS_0 0x01
#define IIC_BUS_1 0x02
#define IIC_BUS_2 0x04
#define IIC_BUS_3 0x08
#define IIC_BUS_4 0x10
#define IIC_BUS_5 0x20
#define IIC_BUS_6 0x40
#define IIC_BUS_7 0x80

typedef u8 AddressType;

typedef struct {
	u8 addr;
	u8 data;
	u8 init;
} HDMI_REG;

	#define NUMBER_OF_HDMI_REGS  16
HDMI_REG hdmi_iic[NUMBER_OF_HDMI_REGS] = {
	{0x41, 0x00, 0x10},
	{0x98, 0x00, 0x03},
	{0x9A, 0x00, 0xE0},
	{0x9C, 0x00, 0x30},
	{0x9D, 0x00, 0x61},
	{0xA2, 0x00, 0xA4},
	{0xA3, 0x00, 0xA4},
	{0xE0, 0x00, 0xD0},
	{0xF9, 0x00, 0x00},
	{0x18, 0x00, 0xE7},
        {0x55, 0x00, 0x00},
        {0x56, 0x00, 0x28},
        {0xD6, 0x00, 0xC0},
        {0xAF, 0x00, 0x4},
	{0xF9, 0x00, 0x00}


};

u8 EepromIicAddr;		/* Variable for storing Eeprom IIC address */

int IicLowLevelDynEeprom();

u8 EepromReadByte(AddressType Address, u8 *BufferPtr, u8 ByteCount);
u8 EepromWriteByte(AddressType Address, u8 *BufferPtr, u8 ByteCount);


u8 WriteBuffer[PAGE_SIZE];	/* Write buffer for writing a page. */

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page.  */

u8 DataBuf[PAGE_SIZE];
/****************i************ Type Definitions *******************************/

typedef u8 AddressType;

/************************** Variable Definitions *****************************/

extern XIic IicFmc, IicAdapter, IicSensor ;	/*  IIC device. */
extern INTC IntcFmc, IntcAdapter, IntcSensor;

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

extern u8 FmcIicAddr;		/* Variable for storing FMC IIC address */
extern u8 AdapterIicAddr;	/* Variable for storing Adapter IIC address */
extern u8 SensorIicAddr;	/* Variable for storing Sensor IIC address */

//HDMI IIC
int IicLowLevelDynEeprom()
{
	u8 BytesRead;
	u32 StatusReg;
	u8 Index;
	int Status, i;
	EepromIicAddr = IIC_SWITCH_ADDRESS;
	Status = XIic_DynInit(IIC_BASE_ADDRESS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("\r\n after XIic_DynInit\r\n");
	while (((StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS,
				XIIC_SR_REG_OFFSET)) &
				(XIIC_SR_RX_FIFO_EMPTY_MASK |
				XIIC_SR_TX_FIFO_EMPTY_MASK |
				XIIC_SR_BUS_BUSY_MASK)) !=
				(XIIC_SR_RX_FIFO_EMPTY_MASK |
				XIIC_SR_TX_FIFO_EMPTY_MASK)) {

	}

	for (Index = 0; Index < PAGE_SIZE; Index++) {
		WriteBuffer[Index] = Index;
		ReadBuffer[Index] = 0;
		DataBuf[Index] = 0;
	}
	EepromIicAddr = IIC_ADV7511_ADDRESS;
	for ( Index = 0; Index < NUMBER_OF_HDMI_REGS; Index++)
	{
		  EepromWriteByte(hdmi_iic[Index].addr, &hdmi_iic[Index].init, 1);

	}

	for ( Index = 0; Index < NUMBER_OF_HDMI_REGS; Index++)
	{
    BytesRead = EepromReadByte(hdmi_iic[Index].addr, &hdmi_iic[Index].data, 1);
      for(i=0;i<1000;i++);	// IIC delay
	  if (BytesRead != 1) {
	    return XST_FAILURE;
	  }
	}


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC serial EEPROM.
*
* @param	BufferPtr contains the address of the data to write.
* @param	ByteCount contains the number of bytes in the buffer to be
*		written. Note that this should not exceed the page size of the
*		EEPROM as noted by the constant PAGE_SIZE.
*
* @return	The number of bytes written, a value less than that which was
*		specified as an input indicates an error.
*
* @note		one.
*
******************************************************************************/
u8 EepromWriteByte(AddressType Address, u8 *BufferPtr,
					   u8 ByteCount)
{
	u8 SentByteCount;
	u8 WriteBuffer[sizeof(Address) + PAGE_SIZE];
	u8 Index;

	/*
	 * A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first based upon the
	 * size of the address for the EEPROM
	 */
	if (sizeof(AddressType) == 2) {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	} else if (sizeof(AddressType) == 1) {
		WriteBuffer[0] = (u8) (Address);
		EepromIicAddr |= (EEPROM_TEST_START_ADDRESS >> 8) & 0x7;
	}

	/*
	 * Put the data in the write buffer following the address.
	 */
	for (Index = 0; Index < ByteCount; Index++) {
		WriteBuffer[sizeof(Address) + Index] = BufferPtr[Index];
	}

	/*
	 * Write a page of data at the specified address to the EEPROM.
	 */
	SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS, EepromIicAddr,
				WriteBuffer, sizeof(Address) + ByteCount,
				XIIC_STOP);

	/*
	 * Return the number of bytes written to the EEPROM.
	 */
	return SentByteCount - sizeof(Address);
}

/******************************************************************************
*
* This function reads a number of bytes from the IIC serial EEPROM into a
* specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*		This value is constrained by the page size of the device such
*		that up to 64K may be read in one call.
*
* @return	The number of bytes read. A value less than the specified input
*		value indicates an error.
*
* @note		None.
*
******************************************************************************/
u8 EepromReadByte(AddressType Address, u8 *BufferPtr,
					  u8 ByteCount)
{
	u8 ReceivedByteCount;
	u8 SentByteCount;
	u16 StatusReg;

	/*
	 * Position the Read pointer to specific location in the EEPROM.
	 */
	do {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS, XIIC_SR_REG_OFFSET);
		if (!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			SentByteCount = XIic_DynSend(IIC_BASE_ADDRESS,
							EepromIicAddr,
							(u8 *) &Address,
							sizeof(Address),
								 XIIC_REPEATED_START);
		}

	} while (SentByteCount != sizeof(Address));
		/*
		 * Receive the data.
		 */
		ReceivedByteCount = XIic_DynRecv(IIC_BASE_ADDRESS,
						 EepromIicAddr, BufferPtr,
						 ByteCount);

	/*
	 * Return the number of bytes received from the EEPROM.
	 */
	return ReceivedByteCount;
}


/*****************************************************************************/
/**
 *
 * Main function to initialize interop system and read data from AR0330 sensor

 * @param  None.
 *
 * @return
 *   - XST_SUCCESS if MIPI Interop was successful.
 *   - XST_FAILURE if MIPI Interop failed.
 *
 * @note   None.
 *
 ******************************************************************************/
int main() {
	int Status;
		int pcam5c_mode = 1;
	int usr_entry ,prev_sel;
	int dsi_hdmi_select = 0;




	xil_printf("\n\r****************************************************** \n\r");
	Xil_ICacheDisable();
	Xil_DCacheDisable();
	xil_printf("\n\r**             VCU108 - HDMI Colorbar Test            **");

	        Status = IicLowLevelDynEeprom();
	        if (Status != XST_SUCCESS) {
		    xil_printf("ADV7511 IIC programming FAILED\r\n");
		return XST_FAILURE;
	         }
	        xil_printf("ADV7511 IIC programming PASSED\r\n");


			//Initialize FMC, Adapter and Sensor IIC
			Status = InitIIC();
			if (Status != XST_SUCCESS) {
				xil_printf("\n\r IIC initialization Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("IIC Initializtion Done \n\r");

			//Initialize FMC Interrupt System
			Status = SetupFmcInterruptSystem(&IicFmc);
			if (Status != XST_SUCCESS) {
				xil_printf("\n\rInterrupt System Initialization Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("FMC Interrupt System Initialization Done \n\r");

			//Set up IIC Interrupt Handlers
			SetupIICIntrHandlers();
			xil_printf("IIC Interrupt Handlers Setup Done \n\r");

			*(u32 *)(CSI_SS_BOARD +0x4) = LANES-1;

			Status =  SetFmcIICAddress();
			if (Status != XST_SUCCESS) {
				xil_printf("\n\rFMC IIC Address Setup Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("Fmc IIC Address Set\n\r");

			//Initialize Adapter Interrupt System
			Status = SetupAdapterInterruptSystem(&IicAdapter);
			if (Status != XST_SUCCESS) {
				xil_printf("\n\rInterrupt System Initialization Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("Adapter Interrupt System Initialization Done \n\r");

			//Set Address of Adapter IIC
			Status =  SetAdapterIICAddress();
			if (Status != XST_SUCCESS) {
				xil_printf("\n\rAdapter IIC Address Setup Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("Adapter IIC Address Set\n\r");

			        Status = InitializeCsiRxSs();
			        if (Status != XST_SUCCESS) {
			        xil_printf("CSI Rx Ss Init failed status = %x.\r\n", Status);
			                return XST_FAILURE;
			        }


			  /*      Status = SetupDSI();
			        if (Status != XST_SUCCESS) {
			        xil_printf("SetupDSI failed status = %x.\r\n" ,Status);
			        return XST_FAILURE;
			        }*/




while(1) {


		 xil_printf("\r\nPlease press the key corresponding to the desired option:");
		 xil_printf("\r\n    1 -> PCAM 5C to Display Panel");
		 xil_printf("\r\n    2 -> PCAM 5C to HDMI \n");

		 usr_entry = getchar();
   if (prev_sel == usr_entry) {
   xil_printf("\r\nAlready in the selected option. Please try again\n");
   continue;
			   }
             prev_sel = usr_entry;


		 switch(usr_entry) {

		        case '1':
		        dsi_hdmi_select = 1;
		        resetIp();
		       EnableCSI();
		        GPIOSelect(dsi_hdmi_select);

		        Status = demosaic();
		        if (Status != XST_SUCCESS) {
				xil_printf("\n\ demosaic failed \n\r");
				return XST_FAILURE;
		        }
			xil_printf("\n\r demosaic working \n\r");

			CamReset();

			//Preconifgure Sensor
			Status = SensorPreConfig(pcam5c_mode);
			if (Status != XST_SUCCESS) {
				xil_printf("\n\rSensor PreConfiguration Failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("Sensor is PreConfigured\n\r");

			Status = vdma_dsi();
			if (Status != XST_SUCCESS) {
				xil_printf("\n\vdma_dsi failed \n\r");
				return XST_FAILURE;
			}
			xil_printf("\n\r vdma_dsi working \n\r");


		       break;
		        case '2':
		        dsi_hdmi_select = 0;
		        resetIp();
		     EnableCSI();
		       // enablecsi();
		        GPIOSelect(dsi_hdmi_select);

		        Status = demosaic();
		        if (Status != XST_SUCCESS) {
				xil_printf("\n\ demosaic failed \n\r");
				return XST_FAILURE;
		        }
		        xil_printf("\n\r demosaic working \n\r");

		        CamReset();

		        //Preconifgure Sensor
		        Status = SensorPreConfig(pcam5c_mode);
		        if (Status != XST_SUCCESS) {
				xil_printf("\n\rSensor PreConfiguration Failed \n\r");
				return XST_FAILURE;
		        }
		        xil_printf("Sensor is PreConfigured\n\r");

		        Status = vdma_hdmi();
		        if (Status != XST_SUCCESS) {
				xil_printf("\n\vdma_hdmi failed \n\r");
				return XST_FAILURE;
		        }
		        xil_printf("\n\r vdma_hdmi working \n\r");

		        Status = vtpg_hdmi();
		        if (Status != XST_SUCCESS) {
				xil_printf("\n\vtpg failed \n\r");
				return XST_FAILURE;
		        }
		        xil_printf("\n\r hdmi_vtpg working \n\r");
		        break;


		        default:
				xil_printf("\n    Selection is unavailable. Please try again");
				break;
		 }

		 WritetoReg(0x30, 0x08, 0x02);
		 Sensor_Delay();
		 xil_printf("\n\r pipeline completed \n\r");
   }
return XST_SUCCESS;
}
