if (DOXYGEN_FOUND)

  if (EXISTS ${CMAKE_SOURCE_DIR}/../_doxygen/openamp/Doxyfile-openamp.in)
    set (OAMP_DOX_DIR ${CMAKE_SOURCE_DIR}/../_doxygen/openamp)
    configure_file (Doxyfile.in Doxyfile1 @ONLY)
    configure_file (${OAMP_DOX_DIR}/Doxyfile-openamp.in Doxyfile2 @ONLY)

    add_custom_target (doc ALL
      COMMAND cat Doxyfile1 Doxyfile2 >Doxyfile
      COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  else ()
    configure_file (Doxyfile.in Doxyfile @ONLY)

    add_custom_target (doc ALL
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif ()

install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
  DESTINATION share/doc/${PROJECT_NAME})
install (FILES ${PROJECT_SOURCE_DIR}/README.md
  DESTINATION share/doc/${PROJECT_NAME})
install (FILES ${PROJECT_SOURCE_DIR}/LICENSE.md
  DESTINATION share/doc/${PROJECT_NAME})

endif (DOXYGEN_FOUND)

