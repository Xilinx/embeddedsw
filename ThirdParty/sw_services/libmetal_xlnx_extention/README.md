## Introduction

This module provides xlnx specific implementation of libmetal interfaces.
It creates static library that should be used with OpenAMP demos to work.

## ZynqMP Toolchain file example:

```
set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (MACHINE "zynqmp_r5" CACHE STRING "")
set (CMAKE_C_COMPILER "armr5-none-eabi-gcc" CACHE STRING "")

set (CMAKE_C_FLAGS    "\
        -O2 -c -mcpu=cortex-r5 -g -DARMR5 -Wall -Wextra -mfloat-abi=hard \
        -mfpu=vfpv3-d16 -fno-tree-loop-distribute-patterns \
        -DSDT \
        -I$ENV{LIBMETAL_BUILD_DIR}/usr/local/include \
        -I$ENV{XLNX_BSP_PATH}/include"
    CACHE STRING ""
)
link_directories(
    $ENV{XLNX_BSP_PATH}/lib/
)

link_directories(
    $ENV{LIBMETAL_BUILD_DIR}/usr/local/lib/
)

SET(CMAKE_INSTALL_PREFIX "." CACHE STRING "")
SET(CMAKE_AR  "armr5-none-eabi-ar" CACHE STRING "")
SET(CMAKE_C_ARCHIVE_FINISH   true)
```

## ZynqMP Build Command

```
cmake <top dir of this module> -DCMAKE_TOOLCHAIN_FILE="<path to toolchain file>"
cmake --build <build dir> --target install
```

## Dependency

libmetal header files and xlnx BSP header files are needed build
this static library module. The final library archive created:
`libmetal_xlnx_extention.a`

## Contribution

Coding style as per open-amp library coding style:
https://github.com/OpenAMP/open-amp/tree/main?tab=readme-ov-file#code-style
