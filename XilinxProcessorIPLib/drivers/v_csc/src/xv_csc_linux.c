// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

#ifdef __linux__

/***************************** Include Files *********************************/
#include "xv_csc.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define MAX_UIO_PATH_SIZE       256
#define MAX_UIO_NAME_SIZE       64
#define MAX_UIO_MAPS            5
#define UIO_INVALID_ADDR        0

/**************************** Type Definitions ******************************/
/**
 * This Structure representing a memory map for the XV_csc UIO device.
 *
 * This structure holds the base address
*/
typedef struct {
    u32 addr;
    u32 size;
} XV_csc_uio_map;


/**
 * @struct XV_csc_uio_info
 * @brief Structure to store UIO (Userspace I/O) device information for XV_csc.
 *
 * This structure holds file descriptor, device number, device name, version,
 * and memory map information for a UIO device used by the XV_csc driver.
 *
 * @var uio_fd
 *      File descriptor for the opened UIO device.
 * @var uio_num
 *      UIO device number.
 * @var name
 *      Name of the UIO device.
 * @var version
 *      Version string of the UIO device.
 * @var maps
 *      Array of memory map information for the UIO device.
 */

typedef struct {
    int  uio_fd;
    int  uio_num;
    char name[ MAX_UIO_NAME_SIZE ];
    char version[ MAX_UIO_NAME_SIZE ];
    XV_csc_uio_map maps[ MAX_UIO_MAPS ];
} XV_csc_uio_info;

/***************** Variable Definitions **************************************/
static XV_csc_uio_info uio_info;

/************************** Function Implementation *************************/
/**
 * This Function Reads a single line from a file into a buffer.
 *
 * This function opens the specified file, reads one line into the provided buffer,
 * and removes the trailing newline character if present. The buffer should be at least
 * MAX_UIO_NAME_SIZE bytes long.
 *
 * @param filename The path to the file to read from.
 * @param linebuf Pointer to the buffer where the line will be stored.
 * @return 0 on success,
 *         -1 if the file could not be opened,
 *         -2 if no line could be read from the file.
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
 * Reads the name of the UIO device corresponding to the given info structure.
 *
 * This function constructs the path to the 'name' file in the sysfs for the
 * specified UIO device (using its uio_num), and reads the device name from it.
 * The name is stored in the 'name' field of the provided XV_csc_uio_info struct.
 *
 * @param info Pointer to an XV_csc_uio_info structure containing the UIO device number.
 * @return 0 on success, or a negative error code on failure.
 */

static int uio_info_read_name(XV_csc_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/name", info->uio_num);
    return line_from_file(file, info->name);
}


/**
 * This Function Reads the version information of a UIO device.
 *
 * This function constructs the path to the version file for the specified
 * UIO device and reads its contents into the provided info structure.
 *
 * @param info Pointer to an XV_csc_uio_info structure containing the UIO device number
 *             and a buffer to store the version string.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int uio_info_read_version(XV_csc_uio_info* info) {
    char file[ MAX_UIO_PATH_SIZE ];
    sprintf(file, "/sys/class/uio/uio%d/version", info->uio_num);
    return line_from_file(file, info->version);
}


/**
 * Reads the physical address of the specified UIO memory map and stores it in the info structure.
 *
 * @param info Pointer to the XV_csc_uio_info structure containing UIO device information.
 * @param n    Index of the memory map to read the address for.
 * @return     0 on success,
 *            -1 if the address file could not be opened,
 *            -2 if the address could not be read from the file.
 *
 * This function constructs the path to the sysfs file containing the physical address
 * of the specified UIO memory map, opens the file, reads the address, and stores it
 * in the corresponding entry of the info->maps array. If any step fails, an error code
 * is returned.
 */
static int uio_info_read_map_addr(XV_csc_uio_info* info, int n) {
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
 * Reads the size of the specified memory map for a UIO device and stores it in the info structure.
 *
 * @param info Pointer to an XV_csc_uio_info structure containing UIO device information.
 * @param n    Index of the memory map to read the size for.
 * @return     0 on success,
 *            -1 if the size file could not be opened,
 *            -2 if the size could not be read from the file.
 *
 * This function constructs the path to the size file for the specified map of the UIO device,
 * opens the file, reads the size (in hexadecimal format), and stores it in the corresponding
 * map entry of the info structure.
 */
static int uio_info_read_map_size(XV_csc_uio_info* info, int n) {
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
 * XV_csc_Initialize - Initialize the XV_csc instance for use with UIO devices in Linux.
 *
 * This function scans the /sys/class/uio directory to find a UIO device whose name matches
 * the provided InstanceName. If found, it reads the device's information, opens the corresponding
 * /dev/uio device file, and maps the control interface into user space. The function sets up
 * the XV_csc instance structure for further use.
 *
 * @param InstancePtr   Pointer to the XV_csc instance to be initialized.
 * @param InstanceName  Name of the UIO device instance to match.
 *
 * @return
 *   - XST_SUCCESS if initialization is successful.
 *   - XST_DEVICE_NOT_FOUND if the specified device is not found.
 *   - XST_OPEN_DEVICE_FAILED if the device file cannot be opened.
 *
 * @note
 *   - The function asserts that InstancePtr is not NULL.
 *   - The function assumes that the slave interface 'Ctrl' is mapped to uioX/map0.
 *   - The function uses mmap to map the device memory into user space.
 *   - The function relies on several helper functions to read UIO device information.
 */

int XV_csc_Initialize(XV_csc *InstancePtr, const char* InstanceName) {
    XV_csc_uio_info *InfoPtr = &uio_info;
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

/**
 * Releases resources associated with the XV_csc instance.
 *
 * This function unmaps the memory region mapped for the XV_csc hardware instance
 * and closes the associated UIO file descriptor. It should be called when the
 * XV_csc instance is no longer needed to free system resources.
 *
 * @param    InstancePtr is a pointer to the XV_csc instance to be released.
 *
 * @return   XST_SUCCESS upon successful release of resources.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before proceeding.
 */
int XV_csc_Release(XV_csc *InstancePtr) {
    XV_csc_uio_info *InfoPtr = &uio_info;

    assert(InstancePtr != NULL);
    assert(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    munmap((void*)InstancePtr->Config.BaseAddress, InfoPtr->maps[0].size);

    close(InfoPtr->uio_fd);

    return XST_SUCCESS;
}

#endif
