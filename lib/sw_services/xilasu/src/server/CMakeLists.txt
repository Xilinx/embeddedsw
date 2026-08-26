# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS Rsa.h)

# liboqs (Open Quantum Safe) prebuilt drop for ML-DSA (FIPS 204) and
# ML-KEM (FIPS 203). oqs_mldsa_internal.h exposes the lower-level ML-DSA entry points used
# for the full FIPS 204 surface (HashML-DSA, deterministic signing, seed-based KeyGen).
# oqs_mlkem_internal.h exposes the native ML-KEM entry points, which the firmware calls in
# place of the generic OQS_KEM API so that the ML-KEM path needs no allocator.
if(XILASU_mldsa_support)
  collect (PROJECT_LIB_HEADERS oqs_mldsa_internal.h)
endif()
if(XILASU_mlkem_support)
  collect (PROJECT_LIB_HEADERS oqs_mlkem_internal.h)
endif()

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze_riscv")
  # A single liboqs drop serves both ML-DSA and ML-KEM, so it is required as soon as either
  # one is enabled and must not be demanded when both are off.
  set(asu_required_archives libxilasu.a)
  if(XILASU_mldsa_support OR XILASU_mlkem_support)
    list(APPEND asu_required_archives liboqs.a)
  endif()

  # cmake -E copy only warns when a source archive is absent and still exits 0, so the
  # failure would otherwise surface much later as an unresolved -lxilasu_mbv/-loqs_mbv.
  foreach(asu_archive ${asu_required_archives})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${asu_archive}")
      message(FATAL_ERROR
        "xilasu server: prebuilt archive '${asu_archive}' not found in "
        "${CMAKE_CURRENT_SOURCE_DIR}; the ASU firmware cannot link without it.")
    endif()
  endforeach()

  execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilasu.a ${CMAKE_LIBRARY_PATH}/libxilasu_mbv.a)
  if(XILASU_mldsa_support OR XILASU_mlkem_support)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/liboqs.a ${CMAKE_LIBRARY_PATH}/liboqs_mbv.a)
  else()
    # Drop what an earlier PQC enabled configuration left behind, so a stale archive cannot
    # be picked up by the firmware link.
    file(REMOVE ${CMAKE_LIBRARY_PATH}/liboqs_mbv.a)
  endif()
endif()
