
set (_app "func_test_suite")
set (_cflags "${CMAKE_C_FLAGS} ${APP_EXTRA_C_FLAGS}")
set (_fw_dir "${APPS_SHARE_DIR}")

collector_list (_list PROJECT_INC_DIRS)
collector_list (_app_list APP_INC_DIRS)
include_directories (${_list} ${_app_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list (_list PROJECT_LIB_DIRS)
collector_list (_app_list APP_LIB_DIRS)
link_directories (${_list} ${_app_list})

collector_list (_list APP_COMMON_SOURCES)
if (WITH_REMOTEPROC_MASTER)
list (APPEND _list "${CMAKE_CURRENT_SOURCE_DIR}/func_test_suite_remoteproc_master.c")
set (_cflags "${_cflags} -DBAREMETAL_MASTER=1")
else (WITH_REMOTEPROC_MASTER)
list (APPEND _list "${CMAKE_CURRENT_SOURCE_DIR}/func_test_suite.c")
endif (WITH_REMOTEPROC_MASTER)
add_executable (${_app}.out ${_list})

set_source_files_properties(${_list} PROPERTIES COMPILE_FLAGS "${_cflags}")

get_property (_linker_opt GLOBAL PROPERTY APP_LINKER_OPT)
get_property (_deps GLOBAL PROPERTY APP_LIB_DEPS)
set (_deps "${_deps} ${PLATFORM_LIB_DEPS}")
string (STRIP "${_deps}" _deps)

if (WITH_REMOTEPROC_MASTER)
target_link_libraries(${_app}.out -Wl,-Map=${_app}.map -Wl,--gc-sections ${_linker_opt} -Wl,--start-group ${_fw_dir}/firmware1.o ${_fw_dir}/firmware2.o ${_deps} -Wl,--end-group)
add_custom_target (${_app}.bin ALL
  ${CROSS_PREFIX}objcopy -O binary ${_app}.out ${_app}.bin
  DEPENDS ${_app}.out)

add_dependencies (${_app}.out ${_fw_dir}/firmware1.o ${_fw_dir}/firmware2.o)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/${_app}.bin" DESTINATION bin)
else (WITH_REMOTEPROC_MASTER)
target_link_libraries(${_app}.out -Wl,-Map=${_app}.map -Wl,--gc-sections ${_linker_opt} -Wl,--start-group ${_deps} -Wl,--end-group)

install (TARGETS ${_app}.out RUNTIME DESTINATION bin)
endif (WITH_REMOTEPROC_MASTER)
