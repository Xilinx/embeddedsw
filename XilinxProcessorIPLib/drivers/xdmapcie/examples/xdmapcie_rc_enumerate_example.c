/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_rc_enumerate_example.c
*
* @brief Demonstrates the use of the XDMA PCIe IP and its driver when the IP
*        is configured as a Root Port.
*
* The example initializes the XDMA PCIe IP and shows how to enumerate the PCIe
* system.
*
* @note This example should be used only when the XDMA PCIe IP is configured
*       as a root complex.
*
* This code will illustrate how the XDmaPcie IP and its standalone driver can
* be used to:
*	- Initialize a XDMA PCIe IP core built as a root complex
*	- Enumerate PCIe end points in the system
*	- Assign BARs to end points
*	- find capabilities on end point
*
* Please note that this example enumerates and initializes PCIe end points
* only.
*
* On CPM6 (versal_2vp) the example also drives the PCIe PERST# fundamental
* reset before enumerating. PERST# is active low and is routed to two pins per
* controller: a PS MIO pin, driven through the PS_GPIO registers directly
* because that device tree node is disabled, and a PMC GPIO pin driven through
* the XGpioPs driver. The pins for the controller in use are selected from the
* device tree compatible string.
*
* We tried to use as much of the driver's API calls as possible to show the
* reader how each call could be used and that probably made the example not
* the shortest way of doing the tasks shown as they could be done.
*
*<pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	Initial version of XDMA PCIe root complex example
* 3.3		10/24/2026	Added the CPM6 (versal_2vp) GPIO PERST# sequence
*</pre>
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"	/* Defines for XPAR constants */
#include "xdmapcie.h"		/* XDmaPcie level 1 interface */
#include "stdio.h"
#include "xil_printf.h"
#include "sleep.h"

#if defined(versal_2vp)
#include "xdmapcie_common.h"	/* CPM6 helper inline functions */
#include "xgpiops.h"		/* PMC GPIO used for the PCIe PERST# signal */
#endif


/************************** Constant Definitions ****************************/

/* Parameters for the waiting for link up routine */
#define XDMAPCIE_LINK_WAIT_MAX_RETRIES 		10	/**< Maximum link up poll attempts */
#define XDMAPCIE_LINK_WAIT_USLEEP_MIN 		90000	/**< Delay between link up polls, in microseconds */

#ifndef SDT
#define XDMAPCIE_DEVICE_ID 	XPAR_XDMAPCIE_0_DEVICE_ID	/**< Device ID of the PCIe IP */
#endif

/**
 * @name Command register bit definitions
 * @{
 */
#define PCIE_CFG_CMD_IO_EN	0x00000001 /**< I/O access enable */
#define PCIE_CFG_CMD_MEM_EN	0x00000002 /**< Memory access enable */
#define PCIE_CFG_CMD_BUSM_EN	0x00000004 /**< Bus master enable */
#define PCIE_CFG_CMD_PARITY	0x00000040 /**< Parity error response */
#define PCIE_CFG_CMD_SERR_EN	0x00000100 /**< SERR report enable */
/** @} */

#if defined(versal_2vp)
/**
 * @name PCIe PERST# pin assignments
 *
 * Each CPM6 controller drives PERST# on two pins, one PS MIO and one PMC
 * GPIO, and both have to be pulsed. Adjust these to match the board wiring.
 * @{
 */
#define PERST_PS_MIO_CTRL0		18U /**< Controller 0 PERST#, PS MIO */
#define PERST_PS_MIO_CTRL1		19U /**< Controller 1 PERST#, PS MIO */
#define PERST_PMC_PIN_CTRL0		40U /**< Controller 0 PERST#, PMC GPIO */
#define PERST_PMC_PIN_CTRL1		41U /**< Controller 1 PERST#, PMC GPIO */
/** @} */

/**
 * @name PS_GPIO registers used to drive PERST#
 *
 * PS_GPIO bank 0 drives MIO[25:0]. The PS wizard tags the PS PERST pin as
 * PCIe reset rather than GPIO, so the PS_GPIO device tree node is disabled
 * and XGpioPs_LookupConfig() has no entry for it. The PDI still configures
 * the pin as a GPIO output, so the controller registers are driven directly.
 * @{
 */
#define PS_GPIO_BASEADDR		0xFF0B0000U /**< PS_GPIO register base */
#define PS_GPIO_MASK_DATA_0_MSW		0x004U /**< Masked data, MIO[25:16] */
#define PS_GPIO_DIRM_0			0x204U /**< Direction mode, bank 0 */
#define PS_GPIO_OEN_0			0x208U /**< Output enable, bank 0 */

#define PS_GPIO_MSW_FIRST_MIO		16U /**< First MIO in the MSW half bank */
#define PS_GPIO_MSW_DATA_MASK		0x3FFU /**< Data field, MIO[25:16] */
#define PS_GPIO_MSW_MASK_SHIFT		16U /**< Write mask field position */

/**
 * Builds a MASK_DATA_0_MSW value that drives only the target pin.
 *
 * Bits [9:0] are the output data for MIO[25:16] and bits [25:16] are the
 * per pin write mask, where 0 is write-through and 1 is masked.
 */
#define PS_GPIO_MSW_VALUE(Bit, Level)					\
	((((u32)~((u32)1U << (Bit)) & PS_GPIO_MSW_DATA_MASK) <<		\
	  PS_GPIO_MSW_MASK_SHIFT) | ((u32)(Level) << (Bit)))

#define PERST_DIR_OUTPUT		1U /**< Drive the pin as an output */
#define PERST_OUTPUT_ENABLE		1U /**< Enable the pin output buffer */
#define PERST_ASSERT_LEVEL		0U /**< PERST# is active low */
#define PERST_DEASSERT_LEVEL		1U /**< Release PERST# */

#define PERST_ASSERT_DELAY_US		100000U /**< PERST# low pulse width */
#define PERST_DEASSERT_DELAY_US		100000U /**< Settle time after release */
/** @} */
#endif /* versal_2vp */

/**
 * @name PCIe configuration space register offsets, in doublewords
 * @{
 */
#define PCIE_CFG_ID_REG			0x0000 /**< Vendor ID and Device ID */
#define PCIE_CFG_CMD_STATUS_REG		0x0001 /**<
						* Command and Status Register
						* Offset
						*/
#define PCIE_CFG_PRI_SEC_BUS_REG	0x0006 /**<
						* Primary and Secondary Bus
						* Register Offset
						*/
#define PCIE_CFG_CAH_LAT_HD_REG		0x0003 /**<
						* Cache Line, Latency Timer,
						* Header Type and
						* BIST Register Offset
						*/
#define PCIE_CFG_BAR_0_REG		0x0004 /**< Base Address Register 0 */
/** @} */

/**
 * @name Configuration header field masks and enumeration limits
 * @{
 */
#define PCIE_CFG_FUN_NOT_IMP_MASK	0xFFFF		/**< Marks a function that is not implemented */
#define PCIE_CFG_HEADER_TYPE_MASK	0x00EF0000	/**< Header type field */
#define PCIE_CFG_MUL_FUN_DEV_MASK	0x00800000	/**< Multi function device */

#define PCIE_CFG_MAX_NUM_OF_BUS		256	/**< Maximum number of buses */
#define PCIE_CFG_MAX_NUM_OF_DEV		1	/**< Maximum number of devices per bus */
#define PCIE_CFG_MAX_NUM_OF_FUN		8	/**< Maximum number of functions per device */

#define PCIE_CFG_PRIM_SEC_BUS		0x00070100	/**< Primary and secondary bus numbers */
#define PCIE_CFG_BAR_0_ADDR		0x00001111	/**< Default address programmed into BAR 0 */
/** @} */



/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

#ifndef SDT
int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, u16 DeviceId);
#else
int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, UINTPTR BaseAddress);
#endif

#if defined(versal_2vp)
static void XDmaPcie_DoPsMioPERST(u32 Mio);
static int XDmaPcie_DoPmcGpioPERST(u32 GpioPin);
#endif

/************************** Variable Definitions ****************************/

/* Allocate PCIe Root Complex IP Instance */
XDmaPcie XdmaPcieInstance;	/**< Instance of the PCIe root complex IP */

#if defined(versal_2vp)
/* PMC-side GPIO instance used for the PCIe PERST# signal. */
static XGpioPs GpioInstance;
#endif


/****************************************************************************/
/**
* @brief Runs the PCIe root complex enumeration example.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful.
*
*****************************************************************************/
int main(void)
{

	int Status;
#if defined(versal_2vp)
	u32 PerstPsMio;
	u32 PerstPmcPin;
#endif

#if defined(XPS_BOARD_VEK385) || defined(XPS_BOARD_VEK385_1)
	/* Initialize I2C and perform GPIO reset sequence */
	Status = XDmaPcie_IicInit();
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: I2C initialization failed\n");
		return XST_FAILURE;
	}
#endif

#if defined(versal_2vp)
	/*
	 * Select the two PERST# pins of this CPM6 controller (0 or 1) from the
	 * device tree compatible string, then pulse both of them: the PS MIO by
	 * direct register access, because the PS_GPIO device tree node is
	 * disabled, and the PMC GPIO through the XGpioPs driver.
	 */
	PerstPsMio  = XDmaPcie_Cpm6GetPerstPin(XPAR_XXDMAPCIE_0_COMPATIBLE,
					PERST_PS_MIO_CTRL0,
					PERST_PS_MIO_CTRL1);
	PerstPmcPin = XDmaPcie_Cpm6GetPerstPin(XPAR_XXDMAPCIE_0_COMPATIBLE,
					PERST_PMC_PIN_CTRL0,
					PERST_PMC_PIN_CTRL1);

	XDmaPcie_DoPsMioPERST(PerstPsMio);

	Status = XDmaPcie_DoPmcGpioPERST(PerstPmcPin);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: PMC GPIO PERST initialization failed\r\n");
		return XST_FAILURE;
	}
#endif


	/* Initialize Root Complex */
#ifndef SDT
	Status = PcieInitRootComplex(&XdmaPcieInstance, XDMAPCIE_DEVICE_ID);
#else
	Status = PcieInitRootComplex(&XdmaPcieInstance, XPAR_XXDMAPCIE_0_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("XdmaPcie rc enumerate Example Failed\r\n");
		return XST_FAILURE;
	}

	/* Scan PCIe Fabric */
	XDmaPcie_EnumerateFabric(&XdmaPcieInstance);

	xil_printf("Successfully ran XdmaPcie rc enumerate Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief Initializes an XDMA PCIe IP that is built as a root complex.
*
* When SDT is defined the second parameter is the base address of the IP
* instead of its device ID.
*
* @param[in,out] XdmaPciePtr Pointer to an instance of the XDmaPcie data
*                structure that represents a root complex IP.
* @param[in] DeviceId Unique ID of the XDMA PCIe IP.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE if unsuccessful.
*
******************************************************************************/
#ifndef SDT
int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, u16 DeviceId)
#else
int PcieInitRootComplex(XDmaPcie *XdmaPciePtr, UINTPTR BaseAddress)
#endif
{
	int Status;
	u32 HeaderData;
	u32 InterruptMask;
	u8  BusNumber;
	u8  DeviceNumber;
	u8  FunNumber;
	u8  PortNumber;

	XDmaPcie_Config *ConfigPtr;

#ifndef SDT
	ConfigPtr = XDmaPcie_LookupConfig(DeviceId);
#else
	ConfigPtr = XDmaPcie_LookupConfig(BaseAddress);
#endif

	Status = XDmaPcie_CfgInitialize(XdmaPciePtr, ConfigPtr,
						ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize PCIe Root Complex"
							"IP Instance\r\n");
		return XST_FAILURE;
	}

	if(!XdmaPciePtr->Config.IncludeRootComplex) {
		xil_printf("Failed to initialize...XDMA PCIE is configured"
							" as endpoint\r\n");
		return XST_FAILURE;
	}


	/* See what interrupts are currently enabled */
	XDmaPcie_GetEnabledInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	/* Make sure all interrupts disabled. */
	XDmaPcie_DisableInterrupts(XdmaPciePtr, XDMAPCIE_IM_ENABLE_ALL_MASK);


	/* See what interrupts are currently pending */
	XDmaPcie_GetPendingInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Just if there is any pending interrupt then clear it.*/
	XDmaPcie_ClearPendingInterrupts(XdmaPciePtr,
						XDMAPCIE_ID_CLEAR_ALL_MASK);

	/*
	 * Read enabled interrupts and pending interrupts
	 * to verify the previous two operations and also
	 * to test those two API functions
	 */

	XDmaPcie_GetEnabledInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently enabled are %8X\r\n", InterruptMask);

	XDmaPcie_GetPendingInterrupts(XdmaPciePtr, &InterruptMask);
	xil_printf("Interrupts currently pending are %8X\r\n", InterruptMask);

	/* Make sure link is up. */
	int Retries;
	Status = FALSE;
	/* check if the link is up or not */
        for (Retries = 0; Retries < XDMAPCIE_LINK_WAIT_MAX_RETRIES; Retries++) {
		if (XDmaPcie_IsLinkUp(XdmaPciePtr)){
			Status = TRUE;
		}
                usleep(XDMAPCIE_LINK_WAIT_USLEEP_MIN);
	}
	if (Status != TRUE ) {
		xil_printf("Link is not up\r\n");
		return XST_FAILURE;
	}

	xil_printf("Link is up\r\n");

	/*
	 * Read back requester ID.
	 */
	XDmaPcie_GetRequesterId(XdmaPciePtr, &BusNumber,
				&DeviceNumber, &FunNumber, &PortNumber);

	xil_printf("Bus Number is %02X\r\n"
			"Device Number is %02X\r\n"
				"Function Number is %02X\r\n"
					"Port Number is %02X\r\n",
			BusNumber, DeviceNumber, FunNumber, PortNumber);


	/* Set up the PCIe header of this Root Complex */
	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, &HeaderData);

	HeaderData |= (PCIE_CFG_CMD_BUSM_EN | PCIE_CFG_CMD_MEM_EN |
				PCIE_CFG_CMD_IO_EN | PCIE_CFG_CMD_PARITY |
							PCIE_CFG_CMD_SERR_EN);

	XDmaPcie_WriteLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, HeaderData);

	/*
	 * Read back local config reg.
	 * to verify the write.
	 */

	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_CMD_STATUS_REG, &HeaderData);

	xil_printf("PCIe Local Config Space is %8X at register"
					" CommandStatus\r\n", HeaderData);

	/*
	 * Set up Bus number
	 */

	HeaderData = PCIE_CFG_PRIM_SEC_BUS;

	XDmaPcie_WriteLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_PRI_SEC_BUS_REG, HeaderData);

	/*
	 * Read back local config reg.
	 * to verify the write.
	 */
	XDmaPcie_ReadLocalConfigSpace(XdmaPciePtr,
					PCIE_CFG_PRI_SEC_BUS_REG, &HeaderData);

	xil_printf("PCIe Local Config Space is %8X at register "
					"Prim Sec. Bus\r\n", HeaderData);

	/* Now it is ready to function */

	xil_printf("Root Complex IP Instance has been successfully"
							" initialized\r\n");

	return XST_SUCCESS;
}

#if defined(versal_2vp)
/****************************************************************************/
/**
* @brief Performs the PCIe PERST# sequence on a PS MIO pin.
*
* Drives the PS_GPIO bank 0 registers directly. The PS_GPIO device tree node
* is disabled on this design, so the XGpioPs driver cannot be used for this
* pin.
*
* @param[in] Mio PS MIO pin number, 16 to 25, wired to PERST#.
*
*****************************************************************************/
static void XDmaPcie_DoPsMioPERST(u32 Mio)
{
	u32 Bit = Mio - PS_GPIO_MSW_FIRST_MIO;

	/* Configure the pin as an output and enable the output buffer. */
	XDmaPcie_WriteReg(PS_GPIO_BASEADDR, PS_GPIO_DIRM_0,
		XDmaPcie_ReadReg(PS_GPIO_BASEADDR, PS_GPIO_DIRM_0) |
			((u32)PERST_DIR_OUTPUT << Mio));
	XDmaPcie_WriteReg(PS_GPIO_BASEADDR, PS_GPIO_OEN_0,
		XDmaPcie_ReadReg(PS_GPIO_BASEADDR, PS_GPIO_OEN_0) |
			((u32)PERST_OUTPUT_ENABLE << Mio));

	/* Assert PERST# (drive low), hold, then deassert (drive high). */
	XDmaPcie_WriteReg(PS_GPIO_BASEADDR, PS_GPIO_MASK_DATA_0_MSW,
			PS_GPIO_MSW_VALUE(Bit, PERST_ASSERT_LEVEL));
	usleep(PERST_ASSERT_DELAY_US);
	XDmaPcie_WriteReg(PS_GPIO_BASEADDR, PS_GPIO_MASK_DATA_0_MSW,
			PS_GPIO_MSW_VALUE(Bit, PERST_DEASSERT_LEVEL));
	usleep(PERST_DEASSERT_DELAY_US);

	xil_printf("PCIe PS_MIO%u PERST# sequence completed\r\n", Mio);
}

/****************************************************************************/
/**
* @brief Performs the PCIe PERST# sequence on a PMC GPIO pin using the XGpioPs
*        driver.
*
* @param[in] GpioPin PMC GPIO pin number wired to PERST#.
*
* @return
*		- XST_SUCCESS if the reset sequence was driven successfully.
*		- XST_FAILURE if the GPIO lookup or initialization failed.
*
* @note The PMC GPIO is the only enabled gpiops node, so it is instance 0.
*
*****************************************************************************/
static int XDmaPcie_DoPmcGpioPERST(u32 GpioPin)
{
	XGpioPs_Config *GpioConfig;
	int Status;

	GpioConfig = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_BASEADDR);
	if (GpioConfig == NULL) {
		xil_printf("ERROR: PMC GPIO LookupConfig failed\r\n");
		return XST_FAILURE;
	}

	/* Selects the PMC bank map, must be set before initializing. */
	GpioInstance.PmcGpio = (u32)TRUE;

	Status = XGpioPs_CfgInitialize(&GpioInstance, GpioConfig,
					GpioConfig->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR: PMC GPIO CfgInitialize failed\r\n");
		return XST_FAILURE;
	}

	XGpioPs_SetDirectionPin(&GpioInstance, GpioPin, PERST_DIR_OUTPUT);
	XGpioPs_SetOutputEnablePin(&GpioInstance, GpioPin,
					PERST_OUTPUT_ENABLE);

	XGpioPs_WritePin(&GpioInstance, GpioPin, PERST_ASSERT_LEVEL);
	usleep(PERST_ASSERT_DELAY_US);
	XGpioPs_WritePin(&GpioInstance, GpioPin, PERST_DEASSERT_LEVEL);
	usleep(PERST_DEASSERT_DELAY_US);

	xil_printf("PCIe PMC_MIO%u PERST# sequence completed\r\n", GpioPin);

	return XST_SUCCESS;
}
#endif /* versal_2vp */
