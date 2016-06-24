CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(d2d-conv-manager)

INCLUDE(FindPkgConfig)

SET(EXTRA_CFLAGS "-Wall -Werror-implicit-function-declaration -fPIC -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(EXTRA_CXXFLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")

SET(CMAKE_EXE_LINKER_FLAGS " -Wl,--hash-style=both")

SET(CLIENT "d2d-conv-manager")
SET(DAEMON "d2d-conv-managerd")
SET(DBUS_INTERFACE "org.tizen.d2dconv")

INSTALL(FILES ${CMAKE_SOURCE_DIR}/99_d2d-conv-manager.post DESTINATION
       ${SYSCONF_INSTALL_DIR}/gumd/useradd.d/)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(daemon)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(msf_tizen_client/test)
