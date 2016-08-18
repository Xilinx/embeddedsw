
file (GLOB _c_srcs *.c)
file (GLOB _asm_srcs *.S *.s)

set (_fw_dir "${APPS_SHARE_DIR}")

collector_list (_list PROJECT_INC_DIRS)
include_directories (${_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list (_list PROJECT_LIB_DIRS)
link_directories (${_list})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (firmware-lnx ${_c_srcs} ${_asm_srcs})
set_target_properties (firmware-lnx
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set (_cflags "${CMAKE_C_FLAGS}")
set_source_files_properties(${_asm_srcs} PROPERTIES COMPILE_FLAGS "${_cflags}")

target_link_libraries(firmware-lnx -nostartfiles -T\"${CMAKE_CURRENT_SOURCE_DIR}/link_remote.ld\" -Wl,--start-group -lc -lm -lstdc++ -lzlib -lfdt -Wl,--end-group)

add_dependencies (firmware-lnx image.ub)

add_custom_command (OUTPUT image.ub
  COMMAND cp ${OPENAMP_ROOT_DIR}/obsolete/system/${PROJECT_SYSTEM}/machine/${PROJECT_MACHINE}/image.ub image.ub)

add_custom_command (OUTPUT ${_fw_dir}
  COMMAND mkdir -p ${_fw_dir})

add_custom_target (lnx-fw-bin-gen ALL
  DEPENDS firmware-lnx ${_fw_dir}/firmware1.o ${_fw_dir}/firmware2.o)

add_custom_target (firmware1.o ALL cp firmware-lnx firmware1
  COMMAND ${CROSS_PREFIX}objcopy -I binary -O elf32-littlearm -B arm firmware1 ${_fw_dir}/firmware1.o
  DEPENDS firmware-lnx ${_fw_dir})

add_custom_target (firmware2.o ALL cp firmware-lnx firmware2
  COMMAND ${CROSS_PREFIX}objcopy -I binary -O elf32-littlearm -B arm firmware2 ${_fw_dir}/firmware2.o
  DEPENDS firmware-lnx ${_fw_dir})
