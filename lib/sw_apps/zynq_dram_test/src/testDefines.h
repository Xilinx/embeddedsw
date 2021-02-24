#ifndef _TEST_DEFINES_H_
#define _TEST_DEFINES_H_

#define RESERVED		0x0fffff00
#define TblBase1		0x80004000
#define TblBase			0x00004000

#define L2CCWay		    	(0xF8F02000 + 0x077C)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CACHE_INVLD_WAY_OFFSET)*/
#define L2CCSync	 	(0xF8F02000 + 0x0730)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CACHE_SYNC_OFFSET)*/
#define L2CCCrtl		(0xF8F02000 + 0x0100)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CNTRL_OFFSET)*/
#define L2CCAuxCrtl	  	(0xF8F02000 + 0x0104)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_AUX_CNTRL_OFFSET)*/
#define L2CCTAGLatReg		(0xF8F02000 + 0x0108)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_TAG_RAM_CNTRL_OFFSET)*/
#define L2CCDataLatReg  	(0xF8F02000 + 0x010C)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_DATA_RAM_CNTRL_OFFSET)*/
#define L2CCIntClear		(0xF8F02000 + 0x0220)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_IAR_OFFSET)*/
#define L2CCIntRaw	  	(0xF8F02000 + 0x021C)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_ISR_OFFSET)*/
#define L2CCEvCntCfg1 		(0xF8F02000 + 0x0204)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_ISR_OFFSET)*/

#define CRValMmuCac		0b01000000000101	/* Enable IDC and Disable MMU */
#define CRValHiVectorAddr	0b00000000000000	/* Set the Vector address to high 0xFFFF0000 */

#define L2CCAuxControl		0x21400			/* Way Size (16 KB) and High Priority for SO and Dev Reads Enable */
#define L2CCControl   		0x01			/* Enable L2CC */
#define L2CCTAGLatency		0x0666			/* 7 Cycle Latency for the time being */
#define L2CCDataLatency		0x0666			/* 7 Cycle Latency for the time being */


#define FPEXC_EN		0x40000000		/* FPU enable bit (1 << 30) */

/*Defines for Base Addresses*/

// SCU Registers
#define SCU_PERIPHERAL_BASE 	0xFEF00000

#define SCU_CONTROL 		0xFEF00000
#define SCU_CONFIG 		0xFEF00004
#define SCU_NON_SECURE_ACCESS 	0xFEF00054
#define SCU_FILTER_START	0xFEF00040
#define SCU_FILTER_END		0xFEF00044

#define SCU_GLOBAL_TIMER_COUNT_L32	0xF8F00200
#define SCU_GLOBAL_TIMER_COUNT_U32	0xF8F00204
#define SCU_GLOBAL_TIMER_CONTROL	0xF8F00208
#define SCU_GLOBAL_TIMER_AUTO_INC	0xF8F00218

#define L2CACHE_CNTRL          	0xF8F02000
#define SLCR_LOW_BASE          	0xF8000000
#define SLCR_UNLOCK_VALUE      	0x0000DF0D
#define SLCR_LOCK_VALUE        	0x0000767B
#define SLCR_ADDR_CLKCTRL       SLCR_LOW_BASE+0x00000100
#ifdef BRAM_BOOT
#define OCM_RAM_LOW_BASE       	0x00000000
#else
#define OCM_RAM_LOW_BASE       	0x00040000
#endif
#ifdef BRAM_BOOT
#define OCM_ROM_LOW_BASE       	0x00040000
#else
#define OCM_ROM_LOW_BASE        0x00000000
#endif
#define OCM_APB_LOW_BASE        0xF800C000
#define AFI_APB_LOW_BASE        0xF8008000
#define DEV_CFG_APB_LOW_BASE    0xF8007000
#define DMAC_NS_BASE            0xF8004000
#define DMAC_SEC_BASE           0xF8003000
#define DMAC_APB_LOW_BASE       0xF8004000
#define DDR_LOW_BASE            0xF8006000
#define DDR_BASE_NOMAP          0x00100000
#define TTC0_BASE               0xF8001000
#define TTC1_BASE               0xF8002000
#define SWDT_BASE               0xF8005000
#define EFUSE_APB_LOW_BASE      0xF800D000


/*AFI BASE ADDRESSES*/
#define AFI0_APB_LOW_BASE       0xF8008000
#define AFI1_APB_LOW_BASE       0xF8009000
#define AFI2_APB_LOW_BASE       0xF800A000
#define AFI3_APB_LOW_BASE       0xF800B000

/*IOU APB DEFINES*/
#define IOU_APB_LOW_BASE_UART0  0xE0000000
#define IOU_APB_LOW_BASE_UART1  0xE0001000
#define IOU_APB_LOW_BASE_USB0   0xE0002000
#define IOU_APB_LOW_BASE_USB1   0xE0003000
#define IOU_APB_LOW_BASE_I2C0   0xE0004000
#define IOU_APB_LOW_BASE_I2C1   0xE0005000
#define IOU_APB_LOW_BASE_SPI0   0xE0006000
#define IOU_APB_LOW_BASE_SPI1   0xE0007000
#define IOU_APB_LOW_BASE_CAN0   0xE0008000
#define IOU_APB_LOW_BASE_CAN1   0xE0009000
#define IOU_APB_LOW_BASE_GPIO   0xE000A000
#define IOU_APB_LOW_BASE_ETH0   0xE000B000
#define IOU_APB_LOW_BASE_ETH1   0xE000C000
#define IOU_APB_LOW_BASE_QSPI   0xE000D000
#define IOU_APB_LOW_BASE_PPCTRL 0xE000E000
#define IOU_APB_LOW_BASE_SDIO0  0xE0100000
#define IOU_APB_LOW_BASE_SDIO1  0xE0101000
#define IOU_APB_LOW_BASE_GPV    0xE0200000

#define NAND_LOW_BASE           0xE1000000
#define PPORT_0_LOW_BASE        0xE2000000
#define PPORT_1_LOW_BASE        0xE4000000

#define IOU_AXI_LOW_BASE_LQSPI  0xFC000000

/*Farbic addresses*/
#define FABS0_LOW_BASE        	0x40000000
#define FABS1_LOW_BASE        	0x80000000



/*CPU PRIVATE PERIPHERAL BASE ADDRESSES*/
#define PERIPHBASE              0xF8F00000
#define PERIPH_SCU_BASE         0xF8F00000
#define PERIPH_GIC_BASE         0xF8F00100
#define PERIPH_GTIMERS_BASE     0xF8F00200
#define PERIPH_PTIMERS_BASE     0xF8F00600


/*CORESIGHT BASE ADDRESSES*/
#define CS_BASE                 0xF8800000
#define CS_DAPROM_BASE          0xF8800000
#define CS_ETB_BASE             0xF8801000
#define CS_CTI_BASE             0xF8802000
#define CS_TPIU_BASE            0xF8803000
#define CS_FUN_BASE             0xF8804000
#define CS_ITM_BASE             0xF8805000
#define CS_CTIF_BASE            0xF8809000
#define CS_CTIA_BASE            0xF880A000
#define CS_FTM_BASE             0xF880B000
#define CS_AXIM_BASE            0xF880C000
#define CS_A9ROM_BASE           0xF8880000
#define CS_CPU_DEBUG0_BASE      0xF8890000
#define CS_CPU_PMU0_BASE        0xF8891000
#define CS_CPU_DEBUG1_BASE      0xF8892000
#define CS_CPU_PMU1_BASE        0xF8893000
#define CS_CPU_CTI0_BASE        0xF8898000
#define CS_CPU_CTI1_BASE        0xF8899000
#define CS_CPU_PTM0_BASE        0xF889C000
#define CS_CPU_PTM1_BASE        0xF889D000


/*SLCR register of#defines*/
#define SCL                  0x000 /*Secure Configuration Lock*/
#define SLCR_LOCK            0x004 /*SLCR Write Protection Lock*/
#define SLCR_UNLOCK          0x008 /*SLCR Write Protection Unlock*/
#define SLCR_LOCKSTA         0x00C /*SLCR Write Protection Status*/

#define ARM_PLL_CTRL         0x100 /*ARM PLL Control*/
#define DDR_PLL_CTRL         0x104 /*DDR PLL Control*/
#define IO_PLL_CTRL          0x108 /*IO PLL Control*/
#define PLL_STATUS           0x10C /*PLL Status*/
#define ARM_PLL_CFG          0x110 /*ARM PLL Configuration*/
#define DDR_PLL_CFG          0x114 /*DDR PLL Configuration*/
#define IO_PLL_CFG           0x118 /*IO PLL Configuration*/
#define PLL_BG_CTRL          0x11C /*PLL Bandgap control*/
#define CPU_CLK_CTRL         0x120 /*ARM CPU Clock Control*/
#define DDR_CLK_CTRL         0x124 /*DDR Clock Control*/
#define DCI_CLK_CTRL         0x128 /*DCI Clock Control*/
#define APER_CLK_CTRL        0x12C /*AMBA Peripheral Clock Control*/
#define USB0_CLK_CTRL        0x130 /*USB 0 ULPI Clock Control*/
#define USB1_CLK_CTRL        0x134 /*USB 1 ULPI Clock Control*/
#define GEM0_RCLK_CTRL       0x138 /*Ethernet 0 RX Clock Control*/
#define GEM1_RCLK_CTRL       0x13C /*Ethernet 1 RX Clock Control*/
#define GEM0_CLK_CTRL        0x140 /*Ethernet 0 Reference Clock Control*/
#define GEM1_CLK_CTRL        0x144 /*Ethernet 1 Reference Clock Control*/
#define SMC_CLK_CTRL         0x148 /*SMC Reference Clock Control*/
#define LQSPI_CLK_CTRL       0x14C /*Quad-SPI Reference Clock Control*/
#define SDIO_CLK_CTRL        0x150 /*SDIO Reference Clock Control*/
#define UART_CLK_CTRL        0x154 /*UART Reference Clock Control*/
#define SPI_CLK_CTRL         0x158 /*SPI Reference Clock Control*/
#define CAN_CLK_CTRL         0x15C /*CAN Reference Clock Control*/
#define CAN_MIOCLK_CTRL      0x160 /*CAN MIO Clock Control*/
#define DBG_CLK_CTRL         0x164 /*Debug Clock Control*/
#define PCAP_CLK_CTRL        0x168 /*PCAP 2X Clock Contol*/
#define TOPSW_CLK_CTRL       0x16C /*TOPSW CPU Clock Contol*/
#define FPGA0_CLK_CTRL       0x170 /*FPGA 0 Output Clock Control*/
#define FPGA0_THR_CTRL       0x174 /*FPGA 0 Clock Throttle Control*/
#define FPGA0_THR_CNT        0x178 /*FPGA 0 Clock Throttle Count*/
#define FPGA0_THR_STA        0x17C /*FPGA 0 Clock Throttle Status*/
#define FPGA1_CLK_CTRL       0x180 /*FPGA 1 Output Clock Control*/
#define FPGA1_THR_CTRL       0x184 /*FPGA 1 Clock Throttle Control*/
#define FPGA1_THR_CNT        0x188 /*FPGA 1 Clock Throttle Count*/
#define FPGA1_THR_STA        0x18C /*FPGA 1 Clock Throttle Status*/
#define FPGA2_CLK_CTRL       0x190 /*FPGA 2 Output Clock Control*/
#define FPGA2_THR_CTRL       0x194 /*FPGA 2 Clock Throttle Control*/
#define FPGA2_THR_CNT        0x198 /*FPGA 2 Clock Throttle Count*/
#define FPGA2_THR_STA        0x19C /*FPGA 2 Clock Throttle Status*/
#define FPGA3_CLK_CTRL       0x1A0 /*FPGA 3 Output Clock Control*/
#define FPGA3_THR_CTRL       0x1A4 /*FPGA 3 Clock Throttle Control*/
#define FPGA3_THR_CNT        0x1A8 /*FPGA 3 Clock Throttle Count*/
#define FPGA3_THR_STA        0x1AC /*FPGA 3 Clock Throttle Status*/
#define SYNC_CTRL            0x1B0 /*Clock Synchronisation Mode Control*/
#define SYNC_STATUS          0x1B4 /*Clock Synchronisation Mode Status*/
#define BANDGAP_TRIM         0x1B8 /*Bandgap trim register*/
#define CC_TEST              0x1BC /*Test register*/
#define PLL_PREDIVISOR       0x1C0 /*PLL pre-divisor*/
#define CLK_621              0x1C4 /*6:3:2:1/4:2:2:1 ratio clock*/
#define PICTURE_DBG          0x1D0 /*Picture debug Configuration register*/
#define PICTURE_DBG_UCNT     0x1D4 /*Picture debug count register upper-32-bit*/
#define PICTURE_DBG_LCNT     0x1D8 /*Picture debug count register lower-32-bit*/

#define PSS_RST_CTRL         0x200 /*PSS Software R#define Control*/
#define DDR_RST_CTRL         0x204 /*DDR Software R#define Control*/
#define AMBA_RST_CTRL        0x208 /*AMBA Software R#define Control*/
#define DMAC_RST_CTRL        0x20C /*DMAC Software R#define Control*/
#define USB_RST_CTRL         0x210 /*USB Software R#define Control*/
#define GEM_RST_CTRL         0x214 /*Ethernet Software R#define Control*/
#define SDIO_RST_CTRL        0x218 /*SDIO Software R#define Control*/
#define SPI_RST_CTRL         0x21C /*SPI Software R#define Control*/
#define CAN_RST_CTRL         0x220 /*CAN Software R#define Control*/
#define I2C_RST_CTRL         0x224 /*I2C Software R#define Control*/
#define UART_RST_CTRL        0x228 /*UART Software R#define Control*/
#define GPIO_RST_CTRL        0x22C /*GPIO Software R#define Control*/
#define LQSPI_RST_CTRL       0x230 /*Quad-SpI Software R#define Control*/
#define SMC_RST_CTRL         0x234 /*SMC Software R#define Control*/
#define OCM_RST_CTRL         0x238 /*OCM Software R#define Control*/
#define DEVCI_RST_CTRL       0x23C /*Device Configuration Software R#define Control*/
#define FPGA_RST_CTRL        0x240 /*FPGA Software R#define Control*/
#define A9_CPU_RST_CTRL      0x244 /*A9 CPU Software R#define Control*/
#define RS_AWDT_CTRL         0x24C /*Watchdog Timer R#define Control*/
#define RST_STATUS           0x250 /*R#define Status*/
#define RST_CLEAR            0x254 /*R#define Status Clear*/
#define REBOOT_STATUS        0x258 /*Reboot Status (persistent through all r#defines except Power-on r#define)*/
#define BOOT_MODE            0x25C /*Boot Mode bootstrap register*/

#define APU_CTRL             0x300 /*APU Control*/
#define WDT_CLK_SEL          0x304 /*APU watchdog timer clock select*/

#define TZ_OCM_MEM0          0x400 /*OCM Mem TrustZone Configuration Register 0*/
#define TZ_OCM_MEM1          0x404 /*OCM Mem TrustZone Configuration Register 1*/
#define TZ_OCM_MEM2          0x408 /*OCM Mem TrustZone Configuration Register 2*/
#define TZ_DDR_RAM           0x430 /*DDR RAM TrustZone Configuration Register*/
#define TZ_DMA_NS            0x440 /*DMAC TrustZone Configuration Register*/
#define TZ_DMA_IRQ_NS        0x444 /*DMAC TrustZone Configuration Register for Interrupts*/
#define TZ_DMA_PERIPH_N      0x448 /*DMAC TrustZone Configuration Register for Peripherals*/
#define TZ_GEM               0x450 /*Ethernet TrustZone Configuration Register*/
#define TZ_SDIO              0x454 /*SDIO TrustZone Configuration Register*/
#define TZ_USB               0x458 /*USB TrustZone Configuration Register*/
#define TZ_FPGA_M            0x484 /*FPGA master ports TrustZone Disable Register*/
#define TZ_FPGA_AFI          0x488 /*FPGA AFI AXI ports TrustZone Disable Register*/

#define DBG_CTRL             0x500 /*SoC Debug Control*/
#define PSS_IDCODE           0x530 /*PSS IDCODE*/

#define DDR_URGENT           0x600 /*DDR Urgent Control*/
#define DDR_CAL_START        0x60C /*DDR Calibration Start Triggers*/
#define DDR_REF_CTRL         0x610 /*DDR Refresh Control*/
#define DDR_REF_START        0x614 /*DDR Refresh Start Triggers*/
#define DDR_CMD_STA          0x618 /*DDR Command Store Status*/
#define DDR_URGENT_SEL       0x61C /*DDR Urgent select*/
#define DDR_DFI_STATUS       0x620 /*DDR DFI status*/

#define MIO_PIN_00           0x700 /*MIO Control for Pin 0*/
#define MIO_PIN_01           0x704 /*MIO Control for Pin 1*/
#define MIO_PIN_02           0x708 /*MIO Control for Pin 2*/
#define MIO_PIN_03           0x70C /*MIO Control for Pin 3*/
#define MIO_PIN_04           0x710 /*MIO Control for Pin 4*/
#define MIO_PIN_05           0x714 /*MIO Control for Pin 5*/
#define MIO_PIN_06           0x718 /*MIO Control for Pin 6*/
#define MIO_PIN_07           0x71C /*MIO Control for Pin 7*/
#define MIO_PIN_08           0x720 /*MIO Control for Pin 8*/
#define MIO_PIN_09           0x724 /*MIO Control for Pin 9*/
#define MIO_PIN_10           0x728 /*MIO Control for Pin 10*/
#define MIO_PIN_11           0x72C /*MIO Control for Pin 11*/
#define MIO_PIN_12           0x730 /*MIO Control for Pin 12*/
#define MIO_PIN_13           0x734 /*MIO Control for Pin 13*/
#define MIO_PIN_14           0x738 /*MIO Control for Pin 14*/
#define MIO_PIN_15           0x73C /*MIO Control for Pin 15*/
#define MIO_PIN_16           0x740 /*MIO Control for Pin 16*/
#define MIO_PIN_17           0x744 /*MIO Control for Pin 17*/
#define MIO_PIN_18           0x748 /*MIO Control for Pin 18*/
#define MIO_PIN_19           0x74C /*MIO Control for Pin 19*/
#define MIO_PIN_20           0x750 /*MIO Control for Pin 20*/
#define MIO_PIN_21           0x754 /*MIO Control for Pin 21*/
#define MIO_PIN_22           0x758 /*MIO Control for Pin 22*/
#define MIO_PIN_23           0x75C /*MIO Control for Pin 23*/
#define MIO_PIN_24           0x760 /*MIO Control for Pin 24*/
#define MIO_PIN_25           0x764 /*MIO Control for Pin 25*/
#define MIO_PIN_26           0x768 /*MIO Control for Pin 26*/
#define MIO_PIN_27           0x76C /*MIO Control for Pin 27*/
#define MIO_PIN_28           0x770 /*MIO Control for Pin 28*/
#define MIO_PIN_29           0x774 /*MIO Control for Pin 29*/
#define MIO_PIN_30           0x778 /*MIO Control for Pin 30*/
#define MIO_PIN_31           0x77C /*MIO Control for Pin 31*/
#define MIO_PIN_32           0x780 /*MIO Control for Pin 32*/
#define MIO_PIN_33           0x784 /*MIO Control for Pin 33*/
#define MIO_PIN_34           0x788 /*MIO Control for Pin 34*/
#define MIO_PIN_35           0x78C /*MIO Control for Pin 35*/
#define MIO_PIN_36           0x790 /*MIO Control for Pin 36*/
#define MIO_PIN_37           0x794 /*MIO Control for Pin 37*/
#define MIO_PIN_38           0x798 /*MIO Control for Pin 38*/
#define MIO_PIN_39           0x79C /*MIO Control for Pin 39*/
#define MIO_PIN_40           0x7A0 /*MIO Control for Pin 40*/
#define MIO_PIN_41           0x7A4 /*MIO Control for Pin 41*/
#define MIO_PIN_42           0x7A8 /*MIO Control for Pin 42*/
#define MIO_PIN_43           0x7AC /*MIO Control for Pin 43*/
#define MIO_PIN_44           0x7B0 /*MIO Control for Pin 44*/
#define MIO_PIN_45           0x7B4 /*MIO Control for Pin 45*/
#define MIO_PIN_46           0x7B8 /*MIO Control for Pin 46*/
#define MIO_PIN_47           0x7BC /*MIO Control for Pin 47*/
#define MIO_PIN_48           0x7C0 /*MIO Control for Pin 48*/
#define MIO_PIN_49           0x7C4 /*MIO Control for Pin 49*/
#define MIO_PIN_50           0x7C8 /*MIO Control for Pin 50*/
#define MIO_PIN_51           0x7CC /*MIO Control for Pin 51*/
#define MIO_PIN_52           0x7D0 /*MIO Control for Pin 52*/
#define MIO_PIN_53           0x7D4 /*MIO Control for Pin 53*/

#define MIO_FMIO_SEL         0x800 /*Select function to be routed via MIO or FMIO*/
#define MIO_LOOPBACK         0x804 /*Loopback function within MIO*/
#define MIO_MST_TRI0         0x80C /*Parallel access to the tri-state enables least significantrd*/
#define MIO_MST_TRI1         0x810 /*Parallel access to the tri-state enables most significantrd*/
#define MIO_MST_TRI          0x814 /*Parallel tri-state enables of all MIO pins*/

#define LVL_SHFTR_EN         0x900 /*Level Shifters Enable*/
#define OCM_CFG              0x910 /*OCM Configuration*/

#endif /* _TEST_DEFINES_H_*/
