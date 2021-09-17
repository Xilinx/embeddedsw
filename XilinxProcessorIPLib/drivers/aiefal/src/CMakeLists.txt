###############################################################################
# Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################

file(GLOB_RECURSE _sources *.cpp)
file(GLOB_RECURSE _headers *.hpp)

set (_dest_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach (hf ${_headers})
  get_filename_component(f "${hf}" NAME)
  get_filename_component(d "${hf}" DIRECTORY)
  get_filename_component(d "${d}" NAME)
  if ("${d}" STREQUAL "src")
	  set (d "")
  endif()

  configure_file ("./${d}/${f}" include/${PROJECT_NAME}/${d}/${f} @ONLY)
  install (FILES ${_dest_inc_dir}/${PROJECT_NAME}/${d}/${f}
           DESTINATION include RENAME ${PROJECT_NAME}/${d}/${f})
endforeach (hf)

collect (PROJECT_LIB_DIRS "${CMAKE_CURRENT_BINARY_DIR}")
collect (PROJECT_INC_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include")

# vim: expandtab:ts=2:sw=2:smartindent
