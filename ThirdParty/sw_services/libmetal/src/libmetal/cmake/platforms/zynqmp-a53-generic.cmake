set (CMAKE_SYSTEM_PROCESSOR "arm64"             CACHE STRING "")
set (CROSS_PREFIX           "aarch64-none-elf-" CACHE STRING "")
include (cross-generic-gcc)

# vim: expandtab:ts=2:sw=2:smartindent
