/*!
 * @file te_fsbl_config.c
 * @author Antti Lukats
 * @copyright 2015 Trenz Electronic GmbH
 */

#define TE0720

// uncomment to add default video init into FSBL
//#define TE_INIT_VIDEO



/* 
 * Settings to copy MAC address into OCM for u-boot usage in environment
 */

#define UBOOT_ENV_MAGIC 0xCAFEBABE
#define UBOOT_ENV_MAGIC_ADDR 0xFFFFFC00
#define UBOOT_ENV_ADDR 0xFFFFFC04

