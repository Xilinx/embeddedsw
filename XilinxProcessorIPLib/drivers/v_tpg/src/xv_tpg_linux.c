// ==============================================================
// Copyright (c) 2015 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_tpg_linux.c
 * @addtogroup v_tpg Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/

#include "xv_tpg.h"

/************************** Constant Definitions *****************************/

/**
 * Maximum size for UIO device path string
 */
#define MAX_UIO_PATH_SIZE       256

/**
 * Maximum size for UIO device name string
 */
#define MAX_UIO_NAME_SIZE       64

/**
 * Maximum number of memory maps per UIO device
 */
#define MAX_UIO_MAPS            5

/**
 * Invalid UIO address marker
 */
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions *******************************/

typedef struct {
    u32 addr;  /**< Base address of the memory map */
    u32 size;  /**< Size of the memory map in bytes */
} XV_tpg_uio_map;

typedef struct {
    int  uio_fd;                          /**< File descriptor for UIO device */
    int  uio_num;                         /**< UIO device number */
    char name[ MAX_UIO_NAME_SIZE ];       /**< Name of the UIO device */
    char version[ MAX_UIO_NAME_SIZE ];    /**< Version string of the UIO device */
    XV_tpg_uio_map maps[ MAX_UIO_MAPS ];  /**< Array of memory maps */
} XV_tpg_uio_info;


static XV_tpg_uio_info uio_info;

/*****************************************************************************/
/**
 * @brief Read a single line from a file
 *
 * This function opens a file, reads the first line into a buffer, and removes
 * any trailing newline character.
 *
 * @param  filename Path to the file to read
 * @param  linebuf Buffer to store the line read from file
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if reading fails
 *
 * @note Buffer must be at least MAX_UIO_NAME_SIZE bytes
 *
 ******************************************************
/************************** Function Implementation *************************/
static int line_from_file(char* filename, char* linebuf) {
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;
    s = fgets(linebuf, MAX_UIO_NAME_SIZE, fp);
    fclose(fp);
    if (!s) return -2;
    for (i=0; (*s)&&(i<MAX_UIO_NAME_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return 0;
}

/*****************************************************************************/
/**
 * @brief Read UIO device name from sysfs
 *
 * This function reads the name attribute of a UIO device from the sysfs
 * filesystem.
 *
 * @param  info Pointer to XV_tpg_uio_info structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_name(XV_tpg_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Read UIO device version from sysfs
 *
 * This function reads the version attribute of a UIO device from the sysfs
 * filesystem.
 *
 * @param  info Pointer to XV_tpg_uio_info structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_version(XV_tpg_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Read memory map base address for a UIO device
 *
 * This function reads the base address of a specific memory map (mapN) from
 * the sysfs filesystem for a UIO device.
 *
 * @param  info Pointer to XV_tpg_uio_info structure
 * @param  n Index of the memory map to read
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if reading fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_addr(XV_tpg_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    info->maps[n].addr = UIO_INVALID_ADDR;
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/addr", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].addr);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/*****************************************************************************/
/**
 * @brief Read memory map size for a UIO device
 *
 * This function reads the size of a specific memory map (mapN) from the sysfs
 * filesystem for a UIO device.
 *
 * @param  info Pointer to XV_tpg_uio_info structure
 * @param  n Index of the memory map to read
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if reading fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_size(XV_tpg_uio_info* info, int n) {
    int ret;
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/maps/map%d/size", info->uio_num, n);
    FILE* fp = fopen(file, "r");
    if (!fp) return -1;
    ret = fscanf(fp, "0x%x", &info->maps[n].size);
    fclose(fp);
    if (ret < 0) return -2;
    return 0;
}

/*****************************************************************************/
/**
 * @brief Initialize XV_tpg instance for Linux using UIO
 *
 * This function initializes the XV_tpg driver instance by locating and opening
 * the UIO device with the specified name, reading its configuration from sysfs,
 * and memory mapping the control interface.
 *
 * @param  InstancePtr Pointer to XV_tpg instance to initialize
 * @param  InstanceName Name of the UIO device to open
 *
 * @return XST_SUCCESS if initialization succeeds
 *         XST_DEVICE_NOT_FOUND if UIO device cannot be found
 *         XST_OPEN_DEVICE_FAILED if device file cannot be opened
 *
 * @note The control interface (slave interface 'Ctrl') is mapped to uioX/map0
 *
 *******************************************************************************/
int XV_tpg_Initialize(XV_tpg *InstancePtr, const char* InstanceName) {
	XV_tpg_uio_info *InfoPtr = &uio_info;
	struct dirent **namelist;
    int i, n;
    char* s;
    char file[ MAX_UIO_PATH_SIZE ];
    char name[ MAX_UIO_NAME_SIZE ];
    int flag = 0;

    assert(InstancePtr != NULL);

    n = scandir("/sys/class/uio", &namelist, 0, alphasort);
    if (n < 0)  return XST_DEVICE_NOT_FOUND;
    for (i = 0;  i < n; i++) {
	strcpy(file, "/sys/class/uio/");
	strcat(file, namelist[i]->d_name);
	strcat(file, "/name");
        if ((line_from_file(file, name) == 0) && (strcmp(name, InstanceName) == 0)) {
            flag = 1;
            s = namelist[i]->d_name;
            s += 3; // "uio"
            InfoPtr->uio_num = atoi(s);
            break;
        }
    }
    if (flag == 0)  return XST_DEVICE_NOT_FOUND;

    uio_info_read_name(InfoPtr);
    uio_info_read_version(InfoPtr);
    for (n = 0; n < MAX_UIO_MAPS; ++n) {
        uio_info_read_map_addr(InfoPtr, n);
        uio_info_read_map_size(InfoPtr, n);
    }

    sprintf(file, "/dev/uio%d", InfoPtr->uio_num);
    if ((InfoPtr->uio_fd = open(file, O_RDWR)) < 0) {
        return XST_OPEN_DEVICE_FAILED;
    }

    // NOTE: slave interface 'Ctrl' should be mapped to uioX/map0
    InstancePtr->Config.BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Config.BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Release XV_tpg instance resources for Linux
 *
 * This function releases resources allocated during initialization, including
 * unmapping the memory-mapped control interface and closing the UIO device
 * file descriptor.
 *
 * @param  InstancePtr Pointer to XV_tpg instance to release
 *
 * @return XST_SUCCESS on successful release
 *
 * @note Instance must be initialized before calling this function
 *
 *******************************************************************************/
int XV_tpg_Release(XV_tpg *InstancePtr) {
	XV_tpg_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
