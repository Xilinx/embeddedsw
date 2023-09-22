# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(XILSECURE_secure_environment "Enables trusted execution environment to allow device key usage(post boot) in ZynqMP for IPI response/Linux/U-boot calls valid only for PMUFW BSP" OFF)
if(XILSECURE_secure_environment)
  set(XSECURE_TRUSTED_ENVIRONMENT " ")
endif()

option(XILSECURE_tpm_support "Enables decryption of bitstream to memory and then writes it to PCAP, allows calculation of sha on decrypted bitstream in chunks valid only for ZynqMP FSBL BSP" OFF)
if(XILSECURE_tpm_support)
  set(XSECURE_TPM_ENABLE " ")
endif()

option(XILSECURE_nonsecure_ipi_access "Enables non secure access for Xilsecure IPI commands for Versal" OFF)
if(XILSECURE_nonsecure_ipi_access)
  set(XSECURE_NONSECURE_IPI_ACCESS " ")
endif()

option(XILSECURE_ecc_support_nist_p256 "Enables/Disables P-256 curve support" OFF)
if(XILSECURE_ecc_support_nist_p256)
  set(ECC_SUPPORT_NIST_P256 " ")
endif()

option(XILSECURE_cache_disable "Enables/Disables cache" ON)
if(XILSECURE_cache_disable)
  if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa78") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr52") OR
     ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze"))
      set(XSECURE_CACHE_DISABLE " ")
  endif()
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(XILSECURE_mode "server")
else()
  set(XILSECURE_mode "client" CACHE STRING "Enables A72/R5 server and client mode support for XilSecure library for Versal")
  set_property(CACHE XILSECURE_mode PROPERTY STRINGS "client" "server")
endif()

set(XILSECURE_elliptic_endianness "littleendian" CACHE STRING "Data endianness selection for elliptic curve APIs of Versal and Versal Net this selection is applicable only for server mode")
set_property(CACHE XILSECURE_elliptic_endianness PROPERTY STRINGS "littleendian" "bigendian")
if("${CMAKE_MACHINE}" STREQUAL "Versal")
  if("${XILSECURE_elliptic_endianness}" STREQUAL "littleendian")
    set(XSECURE_ELLIPTIC_ENDIANNESS " ")
    set(XSECURE_ENDIANNESS "0")
  else()
    set(XSECURE_ELLIPTIC_ENDIANNESS " ")
    set(XSECURE_ENDIANNESS "1")
  endif()
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
    set(XILSECURE_seedlife "256" CACHE STRING "Number of generates required before reseeding and the value ranging from 1 - 2^19 bits")
    set(XILSECURE_dlen "7" CACHE STRING "Seed length in multiples of TRNG block size i.e 16 bytes and the value ranging from 7 - 31")
    set(XILSECURE_adaptproptestcutoff "645" CACHE STRING "Cutoff value to run adaptive health tests")
    set(XILSECURE_repcounttestcutoff "66" CACHE STRING "Cutoff value to run repetitive health tests")

    set(XSECURE_TRNG_USER_CFG_SEED_LIFE "${XILSECURE_seedlife}")
    set(XSECURE_TRNG_USER_CFG_DF_LENGTH "${XILSECURE_dlen}")
    set(XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF "${XILSECURE_adaptproptestcutoff}")
    set(XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF "${XILSECURE_repcounttestcutoff}")
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xsecure_config.h.in ${CMAKE_BINARY_DIR}/include/xsecure_config.h)
