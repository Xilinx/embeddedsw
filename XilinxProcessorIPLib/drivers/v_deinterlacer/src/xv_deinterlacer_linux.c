// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_deinterlacer.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
// Structure for UIO memory map: holds physical address and size.
// Structure for UIO device info: holds file descriptor, device number, name, version, and memory maps.
/**
 * @struct XV_deinterlacer_uio_map
 * @brief Represents a memory map for the UIO device.
 *
 * @var XV_deinterlacer_uio_map::addr
 * Physical base address of the mapped region.
 * @var XV_deinterlacer_uio_map::size
 * Size (in bytes) of the mapped region.
 */

/**
 * @struct XV_deinterlacer_uio_info
 * @brief Contains information about the UIO device instance.
 *
 * @var XV_deinterlacer_uio_info::uio_fd
 * File descriptor for the opened UIO device.
 * @var XV_deinterlacer_uio_info::uio_num
 * UIO device number.
 * @var XV_deinterlacer_uio_info::name
 * Name of the UIO device.
 * @var XV_deinterlacer_uio_info::version
 * Version string of the UIO device.
 * @var XV_deinterlacer_uio_info::maps
 * Array of memory maps associated with the UIO device.
 */

typedef struct {
    u32 addr;
    u32 size;
} XV_deinterlacer_uio_map;

typedef struct {
    int  uio_fd;
    int  uio_num;
    char name[ MAX_UIO_NAME_SIZE ];
    char version[ MAX_UIO_NAME_SIZE ];
    XV_deinterlacer_uio_map maps[ MAX_UIO_MAPS ];
} XV_deinterlacer_uio_info;

/***************** Variable Definitions **************************************/
static XV_deinterlacer_uio_info uio_info;

/************************** Function Implementation *************************/


/**
 * @brief Reads a single line from a file into the provided buffer.
 *
 * Opens the specified file, reads up to MAX_UIO_NAME_SIZE characters from the first line,
 * and stores them in the buffer pointed to by linebuf. If a newline character is encountered,
 * it is replaced with a null terminator. The file is closed before returning.
 *
 * @param filename Path to the file to read from.
 * @param linebuf Pointer to the buffer where the line will be stored.
 *                The buffer must be at least MAX_UIO_NAME_SIZE bytes long.
 * @return 0 on success,
 *         -1 if the file could not be opened,
 *         -2 if reading from the file failed.
 */
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

/**
 * @brief Reads the name of the UIO device and stores it in the provided info structure.
 *
 * This function constructs the path to the 'name' file for the specified UIO device,
 * reads the device name from the file, and stores it in the 'name' field of the
 * XV_deinterlacer_uio_info structure.
 *
 * @param info Pointer to an XV_deinterlacer_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_name(XV_deinterlacer_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}

/**
 * @brief Reads the version information of the specified UIO device.
 *
 * This function constructs the path to the version file of the UIO device
 * identified by the given info structure, reads the version string from the file,
 * and stores it in the info structure.
 *
 * @param info Pointer to an XV_deinterlacer_uio_info structure containing the UIO device number
 *             and a buffer to store the version string.
 * @return Returns 0 on success, or a negative value if an error occurs while reading the file.
 */
static int uio_info_read_version(XV_deinterlacer_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}

/**
 * Reads the physical address of the specified UIO memory map and stores it in the provided info structure.
 *
 * @param info Pointer to an XV_deinterlacer_uio_info structure where the map address will be stored.
 * @param n    Index of the map to read the address for.
 * @return     0 on success,
 *            -1 if the address file could not be opened,
 *            -2 if the address could not be read from the file.
 *
 * This function constructs the path to the sysfs file containing the physical address
 * of the specified UIO map, opens the file, reads the address, and stores it in the
 * corresponding entry of the info->maps array. If any step fails, an error code is returned.
 */
static int uio_info_read_map_addr(XV_deinterlacer_uio_info* info, int n) {
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

/**
 * Reads the size of the specified UIO memory map from sysfs and stores it in the info structure.
 *
 * @param info Pointer to the XV_deinterlacer_uio_info structure containing UIO device information.
 * @param n    Index of the memory map to read the size for.
 * @return     0 on success,
 *            -1 if the size file could not be opened,
 *            -2 if the size could not be read from the file.
 *
 * The function constructs the sysfs path for the given UIO device and map index,
 * opens the corresponding "size" file, reads the hexadecimal size value, and stores
 * it in the info->maps[n].size field. Proper error codes are returned on failure.
 */
static int uio_info_read_map_size(XV_deinterlacer_uio_info* info, int n) {
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

/**
 * Initializes the XV_deinterlacer instance for use in a Linux environment.
 *
 * This function sets up the XV_deinterlacer instance by associating it with
 * the specified device instance name and initializing the underlying UIO
 * (Userspace I/O) information structure.
 *
 * @param InstancePtr   Pointer to the XV_deinterlacer instance to initialize.
 * @param InstanceName  Name of the device instance to associate with this driver.
 *
 * @return
 *   - XST_SUCCESS on successful initialization.
 */
int XV_deinterlacer_Initialize(XV_deinterlacer *InstancePtr, const char* InstanceName) {
    XV_deinterlacer_uio_info *InfoPtr = &uio_info;
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

    // NOTE: slave interface 'Axilites' should be mapped to uioX/map0
    InstancePtr->Config.BaseAddress = (u32)mmap(NULL, InfoPtr->maps[0].size, PROT_READ|PROT_WRITE, MAP_SHARED, InfoPtr->uio_fd, 0 * getpagesize());
    assert(InstancePtr->Config.BaseAddress);

    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}

/**
 * Releases resources associated with the given XV_deinterlacer instance.
 *
 * This function should be called when the XV_deinterlacer instance is no longer needed,
 * to free any resources or memory allocated during its operation.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance to be released.
 * @return
 *   - XST_SUCCESS on success.
 */
int XV_deinterlacer_Release(XV_deinterlacer *InstancePtr) {
    XV_deinterlacer_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
