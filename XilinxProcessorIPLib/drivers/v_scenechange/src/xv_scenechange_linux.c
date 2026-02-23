// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_scenechange_linux.c
 * @addtogroup v_scenechange Overview
 */

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_scenechange.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions ******************************/
/** UIO memory map structure containing address and size information */
typedef struct {
    u32 addr;
    u32 size;
} XV_scenechange_uio_map;

/** UIO device information structure for Linux user-space driver */
typedef struct {
    int  uio_fd;
    int  uio_num;
    char name[ MAX_UIO_NAME_SIZE ];
    char version[ MAX_UIO_NAME_SIZE ];
    XV_scenechange_uio_map maps[ MAX_UIO_MAPS ];
} XV_scenechange_uio_info;

/***************** Macros (Inline Functions) Definitions *********************/
/** Maximum size for UIO device path strings */
#define MAX_UIO_PATH_SIZE       256
/** Maximum size for UIO device name strings */
#define MAX_UIO_NAME_SIZE       64
/** Maximum number of memory maps per UIO device */
#define MAX_UIO_MAPS            5
/** Invalid address marker for uninitialized UIO maps */
#define UIO_INVALID_ADDR        0

/************************** Function Prototypes ******************************/
static int line_from_file(char* filename, char* linebuf);
static int uio_info_read_name(XV_scenechange_uio_info* info);
static int uio_info_read_version(XV_scenechange_uio_info* info);
static int uio_info_read_map_addr(XV_scenechange_uio_info* info, int n);
static int uio_info_read_map_size(XV_scenechange_uio_info* info, int n);

/************************** Variable Definitions *****************************/
static XV_scenechange_uio_info uio_info;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Read a single line from a file
 *
 * This function reads the first line from the specified file, removes the
 * newline character, and stores the result in the provided buffer.
 *
 * @param  filename is the path to the file to read
 * @param  linebuf is the buffer to store the line (MAX_UIO_NAME_SIZE bytes)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if file read fails
 *
 * @note The linebuf must be at least MAX_UIO_NAME_SIZE bytes
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
 * @brief Read the UIO device name from sysfs
 *
 * This function reads the device name from the sysfs interface for the
 * specified UIO device number.
 *
 * @param  info is a pointer to the UIO information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_name(XV_scenechange_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/*****************************************************************************/
/**
 * @brief Read the UIO device version from sysfs
 *
 * This function reads the device version from the sysfs interface for the
 * specified UIO device number.
 *
 * @param  info is a pointer to the UIO information structure
 *
 * @return 0 on success, negative value on failure
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_version(XV_scenechange_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/*****************************************************************************/
/**
 * @brief Read the physical address of a UIO memory map
 *
 * This function reads the physical address of the specified memory map
 * from the sysfs interface.
 *
 * @param  info is a pointer to the UIO information structure
 * @param  n is the memory map index (0 to MAX_UIO_MAPS-1)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if read fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_addr(XV_scenechange_uio_info* info, int n) {
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
 * @brief Read the size of a UIO memory map
 *
 * This function reads the size of the specified memory map from the sysfs
 * interface.
 *
 * @param  info is a pointer to the UIO information structure
 * @param  n is the memory map index (0 to MAX_UIO_MAPS-1)
 *
 * @return 0 on success
 *         -1 if file cannot be opened
 *         -2 if read fails
 *
 * @note None
 *
 *******************************************************************************/
static int uio_info_read_map_size(XV_scenechange_uio_info* info, int n) {
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
 * @brief Initialize the scene change driver for Linux user-space
 *
 * This function initializes the scene change driver by locating the UIO
 * device with the specified name, reading its configuration from sysfs,
 * opening the device file, and memory-mapping the control registers.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  InstanceName is the name of the UIO device to initialize
 *
 * @return XST_SUCCESS if initialization succeeds
 *         XST_DEVICE_NOT_FOUND if the UIO device is not found
 *         XST_OPEN_DEVICE_FAILED if device file cannot be opened
 *
 * @note The control interface is mapped to uio map0
 *
 *******************************************************************************/
int XV_scenechange_Initialize(XV_scenechange *InstancePtr, const char* InstanceName) {
	XV_scenechange_uio_info *InfoPtr = &uio_info;
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
    InstancePtr->Ctrl_BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Ctrl_BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Release resources allocated for the scene change driver
 *
 * This function releases the resources allocated during initialization,
 * including unmapping the memory-mapped control registers and closing the
 * UIO device file descriptor.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return XST_SUCCESS on successful release
 *
 * @note The instance must be properly initialized before calling this function
 *
 *******************************************************************************/
int XV_scenechange_Release(XV_scenechange *InstancePtr) {
	XV_scenechange_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Ctrl_BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
