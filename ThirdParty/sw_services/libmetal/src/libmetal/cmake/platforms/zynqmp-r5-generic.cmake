set (CMAKE_SYSTEM_PROCESSOR "arm"              CACHE STRING "")
set (MACHINE "zynqmp_r5" CACHE STRING "")

set (CROSS_PREFIX           "armr5-none-eabi-" CACHE STRING "")
set (CMAKE_C_FLAGS          "-mfloat-abi=soft -mcpu=cortex-r5 -O2 -g" CACHE STRING "")

include (cross-generic-gcc)

# vim: expandtab:ts=2:sw=2:smartindent
