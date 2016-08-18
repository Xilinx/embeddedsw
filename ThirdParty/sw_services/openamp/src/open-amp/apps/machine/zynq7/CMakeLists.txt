collect (APP_INC_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")

if (WITH_REMOTEPROC_MASTER)
  collect (APP_COMMON_SOURCES platform_info_remoteproc_master.c)
else (WITH_REMOTEPROC_MASTER)
  collect (APP_COMMON_SOURCES platform_info.c)
  collect (APP_COMMON_SOURCES rsc_table.c)
endif (WITH_REMOTEPROC_MASTER)
