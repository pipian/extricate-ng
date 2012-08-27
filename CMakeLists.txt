CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Suppress CYGWIN warning (until 2.8.4 is required)
SET(CMAKE_LEGACY_CYGWIN_WIN32 0)

PROJECT(extricate-ng)
SET(PROJECT_VERSION 0.0.1)

SET(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH
    "Installation prefix for executables and object code libraries" FORCE)
SET(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH
    "Installation prefix for executables" FORCE)

ADD_SUBDIRECTORY(src)
