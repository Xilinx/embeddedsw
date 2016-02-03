set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (MACHINE                "zynq7" CACHE STRING "")
set (CROSS_PREFIX           "arm-xilinx-eabi-" CACHE STRING "")
set (CMAKE_C_FLAGS          "-gdwarf-2 -mthumb-interwork -mcpu=cortex-a9 -ffunction-sections -fdata-sections" CACHE STRING "")
set (CMAKE_ASM_FLAGS        "-gdwarf-2 -mthumb-interwork -mcpu=cortex-a9" CACHE STRING "")
set (PLATFORM_LIB_DEPS      "-lbaremetal -lc -lm -lcs3 -lcs3arm -lcs3unhosted" CACHE STRING "")
set (APP_EXTRA_C_FLAGS      "-DZYNQ7_BAREMETAL" CACHE STRING "")

include (cross_generic_gcc)

# vim: expandtab:ts=2:sw=2:smartindent
