set (_cflags "${CMAKE_C_FLAGS} ${APP_EXTRA_C_FLAGS}")
set (_fw_dir "${APPS_SHARE_DIR}")

collector_list (_list PROJECT_INC_DIRS)
collector_list (_app_list APP_INC_DIRS)
include_directories (${_list} ${_app_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list (_list PROJECT_LIB_DIRS)
collector_list (_app_list APP_LIB_DIRS)
link_directories (${_list} ${_app_list})

get_property (_linker_opt GLOBAL PROPERTY APP_LINKER_OPT)
collector_list (_deps PROJECT_LIB_DEPS)

set (OPENAMP_LIB open_amp)
foreach (_app matrix_multiply matrix_multiplyd)
  collector_list (_sources APP_COMMON_SOURCES)
  set (build_app 1)
  if (WITH_REMOTEPROC_MASTER)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_app}_remoteproc_master.c")
      list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/${_app}_remoteproc_master.c")
      set (_cflags "${_cflags} -DBAREMETAL_MASTER=1")
    else ()
      set (build_app 0)
    endif ()
  else (WITH_REMOTEPROC_MASTER)
    list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/${_app}.c")
  endif (WITH_REMOTEPROC_MASTER)

  if (${build_app} EQUAL "1")
    if (WITH_SHARED_LIB)
      add_executable (${_app}-shared ${_sources})
      target_link_libraries (${_app}-shared ${OPENAMP_LIB}-shared ${_deps})
      install (TARGETS ${_app}-shared RUNTIME DESTINATION bin)
    endif (WITH_SHARED_LIB)

    if (WITH_STATIC_LIB)
      if (${PROJECT_SYSTEM} STREQUAL "linux")
        add_executable (${_app}-static ${_sources})
        target_link_libraries (${_app}-static ${OPENAMP_LIB}-static ${_deps})
        install (TARGETS ${_app}-static RUNTIME DESTINATION bin)
      else (${PROJECT_SYSTEM})
        add_executable (${_app}.out ${_sources})
        set_source_files_properties(${_sources} PROPERTIES COMPILE_FLAGS "${_cflags}")

        if (WITH_REMOTEPROC_MASTER)
          target_link_libraries(${_app}.out -Wl,-Map=${_app}.map -Wl,--gc-sections ${_linker_opt} -Wl,--start-group ${_fw_dir}/firmware1.o ${_fw_dir}/firmware2.o ${OPENAM__LIB}-static ${_deps} -Wl,--end-group)
          add_custom_target (${_app}.bin ALL
          ${CROSS_PREFIX}objcopy -O binary ${_app}.out ${_app}.bin
          DEPENDS ${_app}.out)

          add_dependencies (${_app}.out ${_fw_dir}/firmware1.o ${_fw_dir}/firmware2.o)

          install (FILES "${CMAKE_CURRENT_BINARY_DIR}/${_app}.bin" DESTINATION bin)

        else (WITH_REMOTEPROC_MASTER)

          target_link_libraries(${_app}.out -Wl,-Map=${_app}.map -Wl,--gc-sections ${_linker_opt} -Wl,--start-group ${OPENAMP_LIB}-static ${_deps} -Wl,--end-group)

          install (TARGETS ${_app}.out RUNTIME DESTINATION bin)
        endif (WITH_REMOTEPROC_MASTER)
      endif (${PROJECT_SYSTEM} STREQUAL "linux" )
    endif (WITH_STATIC_LIB)
  endif (${build_app} EQUAL "1")
endforeach(_app)
