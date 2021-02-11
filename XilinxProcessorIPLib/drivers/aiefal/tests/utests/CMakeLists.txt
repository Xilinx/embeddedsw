if (CPPUTEST_DIR)
  find_path(CPPUTEST_INC_DIR  NAMES "CppUTest/Utest.h" PATHS ${CPPUTEST_DIR}/include)
  find_library(CPPUTEST_FOUND_LIB NAMES CppUTest PATHS ${CPPUTEST_DIR}/src/CppUTest)
  if (NOT CPPUTEST_FOUND_LIB)
    message (FATAL_ERROR "No CppUtest Library is found in ${CPPUTEST_DIR}")
  endif (NOT CPPUTEST_FOUND_LIB)
  get_filename_component(CPPUTEST_LIB_DIR ${CPPUTEST_FOUND_LIB} DIRECTORY)
  message ("CPPUTEST_INC_DIR=${CPPUTEST_INC_DIR}")
else()
  ExternalProject_Add(cpputest
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpputest"
    CMAKE_ARGS -DTESTS=OFF -DEXAMPLES=OFF
    INSTALL_COMMAND ""
  )
  set (CPPUTEST_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cpputest/include")
  set (CPPUTEST_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpputest-prefix/src/cpputest-build/src/CppUTest")
endif (CPPUTEST_DIR)

get_property (_ec_flags GLOBAL PROPERTY "PROJECT_EC_FLAGS")

collector_list  (_list PROJECT_INC_DIRS)
list (APPEND _list ${CPPUTEST_INC_DIR})
list (APPEND _list ${CMAKE_CURRENT_SOURCE_DIR}/tc)
include_directories (${_list})

collector_list  (_list PROJECT_LIB_DIRS)
list (APPEND _list ${CPPUTEST_LIB_DIR})
link_directories (${_list})

collector_list (_deps PROJECT_LIB_DEPS)
list (APPEND _deps "CppUTest")

file(GLOB _sources tc/*.cpp)
set (EXEPREX "run-test")
set (EXETEST "${EXEPREX}-aie")
add_executable (${EXETEST} ${_sources})
set_target_properties(${EXETEST} PROPERTIES CXX_STANDARD 11)
if (NOT CPPUTEST_FOUND_LIB)
  add_dependencies (${EXETEST} cpputest)
endif (NOT CPPUTEST_FOUND_LIB)
target_compile_options (${EXETEST} PUBLIC ${_ec_flags} -DAIE_GEN=1)
target_link_libraries (${EXETEST} ${_ec_flags} ${_deps})
install (TARGETS ${EXETEST} RUNTIME DESTINATION bin)

#set library search PATH
if (WITH_TESTS_EXEC)
  collector_list  (_list PROJECT_LIB_DIRS)
  string(REPLACE " " ":" _libs_paths "${_list}")
  if (APPLE)
    set (_env "DYLD_LIBRARY_PATH=${_libs_paths}")
  else()
    set (_env "LD_LIBRARY_PATH=${_libs_paths}")
  endif(APPLE)

  set (_tests ${EXEPREX}-aie)
  foreach (_test ${_tests})
    add_test(${_test} ${_test})
    set_property(TEST ${_test} PROPERTY ENVIRONMENT "${_env}")
    add_custom_target(run_unit_test_${_test} ALL
      COMMENT "Run Tests"
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      POST_BUILD
      DEPENDS ${_test})

    if(CMAKE_BUILD_TYPE STREQUAL "coverage" OR CODE_COVERAGE)
      if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        add_custom_target(coverage_report_${_test} ALL
          COMMENT "Generate coverage report"
          POST_BUILD
          DEPENDS run_unit_test_${_test}
          COMMAND llvm-profdata merge -sparse default.profraw -o ${_test}.profdata
          COMMAND llvm-cov report ${_test} -instr-profile=${_test}.profdata
       )
      else()
        add_custom_target(coverage_report_${_test} ALL
          COMMENT "Generate coverage report"
          POST_BUILD
          DEPENDS run_unit_test_${_test}
          COMMAND lcov -c --rc lcov_branch_coverage=1 -d . -o coverage.info
          COMMAND lcov -l --rc lcov_branch_coverage=1 coverage.info
        )
      endif("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    endif(CMAKE_BUILD_TYPE STREQUAL "coverage" OR CODE_COVERAGE)
  endforeach(_test)
endif (WITH_TESTS_EXEC)
