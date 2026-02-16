// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_gamma_lut_linux.c
 * @addtogroup v_gamma_lut Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_gamma_lut.h"


/***************** Macros (Inline Functions) Definitions *********************/
/** Maximum UIO path size */
#define MAX_UIO_PATH_SIZE       256
/** Maximum UIO name size */
#define MAX_UIO_NAME_SIZE       64
/** Maximum number of UIO maps */
#define MAX_UIO_MAPS            5
/** Invalid UIO address marker */
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions *******************************/
typedef struct {
    u32 addr; /**< UIO map address */
    u32 size; /**< UIO map size */
} XV_gamma_lut_uio_map;

typedef struct {
    int  uio_fd;                          /**< UIO file descriptor */
    int  uio_num;                         /**< UIO device number */
    char name[ MAX_UIO_NAME_SIZE ];       /**< UIO device name */
    char version[ MAX_UIO_NAME_SIZE ];    /**< UIO device version */
    XV_gamma_lut_uio_map maps[ MAX_UIO_MAPS ]; /**< UIO memory maps */
} XV_gamma_lut_uio_info;

/************************** Variable Definitions *****************************/
static XV_gamma_lut_uio_info uio_info;

/*****************************************************************************/
/**
 * @brief Reads a single line from a file
 *
 * This function opens the specified file, reads one line into the provided
 * buffer, removes the newline character, and closes the file.
 *
 * @param  filename is the path to the file to read.
 * @param  linebuf is the buffer to store the line read from the file.
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note None
 *
 *******************************************************************************/
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
 * @brief Reads UIO device name from sysfs
 *
 * This function reads the name of the UIO device from the sysfs filesystem
 * and stores it in the provided uio_info structure.
 *
 * @param  info is a pointer to the XV_gamma_lut_uio_info structure.
 *
 * @return 0 on success, error code on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_name(XV_gamma_lut_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Reads UIO device version from sysfs
 *
 * This function reads the version of the UIO device from the sysfs filesystem
 * and stores it in the provided uio_info structure.
 *
 * @param  info is a pointer to the XV_gamma_lut_uio_info structure.
 *
 * @return 0 on success, error code on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_version(XV_gamma_lut_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Reads UIO memory map address from sysfs
 *
 * This function reads the physical address of a specific UIO memory map
 * from the sysfs filesystem and stores it in the provided uio_info structure.
 *
 * @param  info is a pointer to the XV_gamma_lut_uio_info structure.
 * @param  n is the map index to read.
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_addr(XV_gamma_lut_uio_info* info, int n) {
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
 * @brief Reads UIO memory map size from sysfs
 *
 * This function reads the size of a specific UIO memory map from the sysfs
 * filesystem and stores it in the provided uio_info structure.
 *
 * @param  info is a pointer to the XV_gamma_lut_uio_info structure.
 * @param  n is the map index to read.
 *
 * @return 0 on success, -1 if file cannot be opened, -2 if read fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_size(XV_gamma_lut_uio_info* info, int n) {
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
 * @brief Initializes the Gamma LUT driver for Linux UIO
 *
 * This function initializes the Gamma LUT driver by locating the UIO device
 * with the specified name, reading its configuration from sysfs, opening the
 * device file, and mapping its control registers into user space.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  InstanceName is the name of the UIO device to initialize.
 *
 * @return XST_SUCCESS on success, XST_DEVICE_NOT_FOUND if device not found,
 *         XST_OPEN_DEVICE_FAILED if device cannot be opened
 *
 * @note The slave interface 'Ctrl' is mapped to uioX/map0
 *
 *******************************************************************************/
int XV_gamma_lut_Initialize(XV_gamma_lut *InstancePtr, const char* InstanceName) {
    XV_gamma_lut_uio_info *InfoPtr = &uio_info;
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
 * @brief Releases the Gamma LUT driver resources
 *
 * This function releases the resources allocated by the Gamma LUT driver,
 * including unmapping the memory-mapped registers and closing the UIO device.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return XST_SUCCESS on success
 *
 * @note The instance must be initialized before calling this function
 *
 *******************************************************************************/
int XV_gamma_lut_Release(XV_gamma_lut *InstancePtr) {
    XV_gamma_lut_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
