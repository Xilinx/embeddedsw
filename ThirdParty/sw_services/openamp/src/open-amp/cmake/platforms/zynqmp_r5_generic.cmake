set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (MACHINE                "zynqmp_r5" CACHE STRING "")
set (CROSS_PREFIX           "armr5-none-eabi-" CACHE STRING "")
set (CMAKE_C_FLAGS          "-mfloat-abi=soft -mcpu=cortex-r5 -O2" CACHE STRING "")
set (CMAKE_ASM_FLAGS        "-mfloat-abi=soft -mcpu=cortex-r5" CACHE STRING "")
set (PLATFORM_LIB_DEPS      "-lxil -lc -lm" CACHE STRING "")

include (cross_generic_gcc)

# vim: expandtab:ts=2:sw=2:smartindent
