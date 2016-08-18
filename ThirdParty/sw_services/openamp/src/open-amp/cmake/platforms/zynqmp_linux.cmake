set (CMAKE_SYSTEM_PROCESSOR "arm64")
set (CROSS_PREFIX           "aarch64-linux-gnu-")
set (MACHINE                "zynqmp_a53" CACHE STRING "")

include (cross-linux-gcc)

# vim: expandtab:ts=2:sw=2:smartindent
