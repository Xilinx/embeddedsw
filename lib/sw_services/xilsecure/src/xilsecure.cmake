# Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

# Initialize platform and processor flags
set(_IS_PLM_MICROBLAZE FALSE)
set(_IS_PMU_MICROBLAZE FALSE)
set(_IS_ZYNQMP FALSE)
set(_IS_VERSAL FALSE)
set(_IS_VERSAL_2VP FALSE)
set(_IS_VERSAL_2VP_P FALSE)
set(_IS_VERSALNET FALSE)
set(_IS_VERSAL_2VE_2VM FALSE)
set(_IS_SPARTANUPLUS FALSE)
set(_IS_SPARTANUPLUS_AES1 FALSE)
set(_IS_VERSALNET_OR_2VP_OR_2VP_P FALSE)
set(_IS_VERSALNET_OR_2VP_P FALSE)
set(_IS_VERSAL_ONLY FALSE)
set(_IS_SERVER_CONFIG FALSE)

# Evaluate processor types
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
  set(_IS_PLM_MICROBLAZE TRUE)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze")
  set(_IS_PMU_MICROBLAZE TRUE)
endif()

# Evaluate platform types
if("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
  set(_IS_ZYNQMP TRUE)
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
  set(_IS_VERSAL TRUE)
  # 2VP and 2VP_P variants are subsets of Versal
  if("Versal_2VP" IN_LIST VARIANT)
    set(_IS_VERSAL_2VP TRUE)
  endif()
  if("Versal_2VP_P" IN_LIST VARIANT)
    set(_IS_VERSAL_2VP_P TRUE)
  endif()
endif()

if("${CMAKE_MACHINE}" STREQUAL "VersalNet")
  set(_IS_VERSALNET TRUE)
  # 2VE_2VM is a subset of VersalNet
  if("${CMAKE_SUBMACHINE}" STREQUAL "Versal_2VE_2VM")
    set(_IS_VERSAL_2VE_2VM TRUE)
  endif()
endif()

if("${CMAKE_MACHINE}" STREQUAL "spartanuplus")
  set(_IS_SPARTANUPLUS TRUE)
  if("spartanuplusaes1" IN_LIST VARIANT)
    set(_IS_SPARTANUPLUS_AES1 TRUE)
  endif()
endif()

# Combined platform checks for readability
if(_IS_VERSALNET OR _IS_VERSAL_2VP OR _IS_VERSAL_2VP_P)
  set(_IS_VERSALNET_OR_2VP_OR_2VP_P TRUE)
endif()

if(_IS_VERSALNET OR _IS_VERSAL_2VP_P)
  set(_IS_VERSALNET_OR_2VP_P TRUE)
endif()

# 2VP and 2VP_P variants are subsets of Versal, so check before setting Versal-only flag
if(_IS_VERSAL AND (NOT _IS_VERSALNET_OR_2VP_OR_2VP_P))
  set(_IS_VERSAL_ONLY TRUE)
endif()

# Server configuration is required for PLM Microblaze or for other cores in Versal or Spartan UltraScalePlus AES1 variants
if(_IS_PLM_MICROBLAZE OR ((NOT _IS_PLM_MICROBLAZE) AND _IS_VERSAL_ONLY) OR _IS_SPARTANUPLUS_AES1)
  set(_IS_SERVER_CONFIG TRUE)
endif()

# =============================================================================
# ZynqMP Configuration
# =============================================================================
if(_IS_ZYNQMP)
  if(_IS_PMU_MICROBLAZE)
    option(XILSECURE_secure_environment "Enables trusted execution environment to allow device key usage(post boot) in ZynqMP for IPI response/Linux/U-boot calls valid only for PMUFW BSP" OFF)
    if(XILSECURE_secure_environment)
      set(XSECURE_TRUSTED_ENVIRONMENT " ")
    endif()
  else()
    option(XILSECURE_tpm_support "Enables decryption of bitstream to memory and then writes it to PCAP, allows calculation of SHA on decrypted bitstream in chunks valid only for ZynqMP FSBL BSP" OFF)
    if(XILSECURE_tpm_support)
      set(XSECURE_TPM_ENABLE " ")
    endif()
  endif()
endif()

# =============================================================================
# Server Mode Configuration
# =============================================================================
if(_IS_PLM_MICROBLAZE OR _IS_SPARTANUPLUS)
  set(XILSECURE_mode "server")
  set(XILSECURE_INCLUDE_XPLMI_BSP_CONFIG_H "")
endif()

# =============================================================================
# Server Configuration (PLM Microblaze or Versal server mode for other cores)
# =============================================================================
if(_IS_SERVER_CONFIG)
  # Elliptic Curve Configuration
  option(XILSECURE_elliptic_p192_support "Enables/Disables P-192 curve support" OFF)
  if(XILSECURE_elliptic_p192_support)
    set(XSECURE_ECC_SUPPORT_NIST_P192 " ")
  endif()

  option(XILSECURE_elliptic_p224_support "Enables/Disables P-224 curve support" OFF)
  if(XILSECURE_elliptic_p224_support)
    set(XSECURE_ECC_SUPPORT_NIST_P224 " ")
  endif()

  option(XILSECURE_elliptic_p256_support "Enables/Disables P-256 curve support" OFF)
  if(XILSECURE_elliptic_p256_support)
    set(XSECURE_ECC_SUPPORT_NIST_P256 " ")
  endif()

  option(XILSECURE_elliptic_p521_support "Enables/Disables P-521 curve support" ON)
  if(XILSECURE_elliptic_p521_support)
    set(XSECURE_ECC_SUPPORT_NIST_P521 " ")
  endif()

  # Crypto Algorithm Info Configuration for Versal only
  if(_IS_VERSAL_ONLY)
    option(XILSECURE_cryptoalginfo_enabled "Enables/Disables crypto algorithm version information support" ON)
    if(XILSECURE_cryptoalginfo_enabled)
      set(XSECURE_CRYPTOALGINFO_ENABLED " ")
    endif()
  endif()

  # Endianness Configuration
  set(XILSECURE_elliptic_endianness "littleendian" CACHE STRING "Data endianness selection for Elliptic Curve APIs (server mode only)")
  set_property(CACHE XILSECURE_elliptic_endianness PROPERTY STRINGS "littleendian" "bigendian")
  if(XILSECURE_elliptic_endianness STREQUAL "littleendian")
    set(XSECURE_ENDIANNESS "0")
  else()
    set(XSECURE_ENDIANNESS "1")
  endif()

  # TRNG Configuration for VersalNet, Versal 2VP and Spartan UltraScalePlus AES1 variants
  if(_IS_VERSALNET_OR_2VP_OR_2VP_P OR _IS_SPARTANUPLUS_AES1)
    set(XILSECURE_seedlife "256" CACHE STRING "Number of generates required before reseeding and the value ranging from 1 - 2^19 bits")
    set(XSECURE_TRNG_USER_CFG_SEED_LIFE_VAL "${XILSECURE_seedlife}")

    set(XILSECURE_dlen "7" CACHE STRING "Seed length in multiples of TRNG block size i.e. 16 bytes and the value ranging from 7 - 31")
    set(XSECURE_TRNG_USER_CFG_DF_LENGTH_VAL "${XILSECURE_dlen}")
  endif()

  # Additional TRNG settings for VersalNet or Versal 2VP_P or Spartan UltraScalePlus AES1 variants
  if(_IS_VERSALNET_OR_2VP_P OR _IS_SPARTANUPLUS_AES1)
    set(XILSECURE_adaptproptestcutoff "645" CACHE STRING "Cutoff value to run adaptive health tests")
    set(XSECURE_TRNG_USER_CFG_ADAPT_TEST_CUTOFF_VAL "${XILSECURE_adaptproptestcutoff}")

    set(XILSECURE_repcounttestcutoff "66" CACHE STRING "Cutoff value to run repetitive health tests")
    set(XSECURE_TRNG_USER_CFG_REP_TEST_CUTOFF_VAL "${XILSECURE_repcounttestcutoff}")
  endif()
endif()

# =============================================================================
# Client Configuration
# =============================================================================
if((NOT _IS_PLM_MICROBLAZE) AND (NOT _IS_SPARTANUPLUS) AND (NOT _IS_PMU_MICROBLAZE))
  # Versal: mode can be client or server
  if(_IS_VERSAL_ONLY)
    set(XILSECURE_mode "client" CACHE STRING "Enables A72/R5/PL microblaze server and client mode support for XilSecure library for Versal")
    set_property(CACHE XILSECURE_mode PROPERTY STRINGS "client" "server")
  else()
    set(XILSECURE_mode "client")
  endif()

  # Cache disable option while operating in client mode
  option(XILSECURE_cache_disable "Enables/Disables Cache for XilSecure client library" OFF)
  if(XILSECURE_cache_disable AND XILSECURE_mode STREQUAL "client")
    set(XSECURE_CACHE_DISABLE " ")
  endif()
endif()

# =============================================================================
# Common Configuration (Server and client)
# =============================================================================
# RSA Key Wrap Configuration for VersalNet only
if(_IS_VERSALNET AND (NOT _IS_VERSAL_2VE_2VM))
  set(XILSECURE_rsa_key_size_keywrap "RSA_3072_KEY_SIZE" CACHE STRING "RSA key size for key wrap operation. It shall be configured same in both PLM and client BSP.")
  set_property(CACHE XILSECURE_rsa_key_size_keywrap PROPERTY STRINGS "RSA_2048_KEY_SIZE" "RSA_3072_KEY_SIZE" "RSA_4096_KEY_SIZE")

  # Map RSA key size to byte values
  if(XILSECURE_rsa_key_size_keywrap STREQUAL "RSA_2048_KEY_SIZE")
    set(XILSECURE_RSA_KEY_SIZE_KEYWRAP_VAL "256")
  elseif(XILSECURE_rsa_key_size_keywrap STREQUAL "RSA_3072_KEY_SIZE")
    set(XILSECURE_RSA_KEY_SIZE_KEYWRAP_VAL "384")
  elseif(XILSECURE_rsa_key_size_keywrap STREQUAL "RSA_4096_KEY_SIZE")
    set(XILSECURE_RSA_KEY_SIZE_KEYWRAP_VAL "512")
  endif()

  set(XILSECURE_key_slot_addr "0x00000000" CACHE STRING "Key slot address to store unwrapped keys. It shall be configured same in both PLM and client BSP.")
  set(XSECURE_KEY_SLOT_ADDR "${XILSECURE_key_slot_addr}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xsecure_config.h.in ${CMAKE_BINARY_DIR}/include/xsecure_config.h)
