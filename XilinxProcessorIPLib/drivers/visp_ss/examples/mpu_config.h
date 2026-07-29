#ifdef ARMR52
#include "trace.h"
#include "xil_mpu.h"
#endif



#if ARMR52

/*************************************mpu**********************/
// #define RPU0_FW_START_ADDR                  (0x12000000)
// #define RPU0_FW_SIZE                        (0x10000000) // 256MB

#define RPU0_FW_START_ADDR                  (0x31000000)
#define RPU0_FW_SIZE                        (0x10000000) // 256MB

#define RPU0_LOAD_CALIB_START_ADDR          (0xF800000U)
#define RPU0_LOAD_CALIB_SIZE                (0x28000U)

#define RPU_MBOX_START_ADDR		    	    (0x1E0A8000U)
#define RPU_MBOX_SIZE                       (0x400000U)

/*User shall not modify the below MACROS*/
#define RPU_SHM_START_ADDR		             (0x1E4A8000U)
#define RPU_SHM_SIZE                         (0X40000U)
#define RPU_SHM_END_ADDR                     (RPU_SHM_START_ADDR+RPU_SHM_SIZE)
#define RPU_MBOX_RPUSHM_SIZE        	     (RPU_MBOX_SIZE + RPU_SHM_SIZE - 1)
#define RPU_FW_START_ADDR                    RPU0_FW_START_ADDR
#define RPU_FW_SIZE                          RPU0_FW_SIZE
#define RPU_LOAD_CALIB_START_ADDR            RPU0_LOAD_CALIB_START_ADDR
#define RPU_LOAD_CALIB_SIZE                  RPU0_LOAD_CALIB_SIZE

#define RPU_TDATABASE_BIN_ADDR               0x30000000
#define RPU_TDATABASE_BIN_SIZE               0xe5e50
XMpuConfig_Initial InitialMpu_Config __attribute__((section(".bootdata"))) = {
{
		/* TCM  */
		0x00000000U,
		0x2000,
		NORM_NSHARED_WT_NWA | PRIV_RO_USER_RO,
	},
	{
		/* BootData */
		0x2000,
		0x1E000,
		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW,
	},
	{
	// 	/* TEXT */
		RPU_TDATABASE_BIN_ADDR,
		RPU_TDATABASE_BIN_SIZE,
		STRONG_ORDERD_SHARED | PRIV_RW_USER_RW,
	},
	{
		/* TEXT */
		RPU_FW_START_ADDR,
		RPU_FW_SIZE,
		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW,
	},
	{
		RPU0_LOAD_CALIB_START_ADDR,
	RPU0_LOAD_CALIB_SIZE,
		 STRONG_ORDERD_SHARED | PRIV_RW_USER_RW,

	},
	{
	 RPU_MBOX_START_ADDR,
		RPU_MBOX_SIZE,
	//	 NORM_SHARED_NCACHE || PRIV_RW_USER_RW,
		STRONG_ORDERD_SHARED | PRIV_RW_USER_RW,
	},
	{
		/* 512 MB LPD to AFI fabric slave port */
		0x80000000U,
		0x1FFFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		0xA0000000U,
		0x17FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 2 MB OCM */
		0xBBE00000U,
		0x1FFFFF,
		NORM_NSHARED_WT_NWA | PRIV_RW_USER_RW,
	},
	{
		/* 512 MB xSPI + 16 MB Coresight */
		0xC0000000U,
		0x20FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 2MB RPU GIC */
		0xE2000000U,
		0x1FFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		 /* 8MB VCU and ISP */
		0xE8000000U,
		0x7FFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},

	{
		/* 16MB FPD + 32MB LPD + 16MB MMI */
		0xEA000000U,
		0x3FFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	{
		/* 128MB PMC + 64MB PS_FPD_CMN */
		0xF0000000U,
		0xBFFFFFF,
		DEVICE_NONSHARED | PRIV_RW_USER_RW,
	},
	/* A total of 14 MPU regions are allocated with another 2 being free for users */
	{
		0U
	}
};
/**************************************************************/
#endif
