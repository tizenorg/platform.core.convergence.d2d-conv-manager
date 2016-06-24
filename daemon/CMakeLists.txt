INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/daemon/lib)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/msf_tizen_client/include)

SET(PTHREAD "pthread")

FILE(GLOB DAEMON_SRCS *.c *.cpp )
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} discovery_provider/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} service_provider/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} access_control/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} discovery_provider/iotcon/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} discovery_provider/smartview/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} ../msf_tizen_client/src/*.cpp)

SET(provider_deps "glib-2.0 dlog json-glib-1.0 iotcon capi-appfw-app-manager iotcon vconf capi-network-bluetooth capi-network-wifi-direct capi-appfw-application bundle capi-network-connection cynara-creds-gdbus cynara-client cynara-session capi-appfw-package-manager")
SET(provider_deps "${provider_deps} openssl libwebsockets libcurl nsd-dns-sd")
#	SET(provider_deps "${provider_deps} security-server")
# Mobile profile
IF("${PROFILE}" STREQUAL "mobile")
	ADD_DEFINITIONS("-D_MOBILE_")
#	SET(provider_deps "${provider_deps} tapi capi-messaging-messages")
ENDIF("${PROFILE}" STREQUAL "mobile")

# TV profile
IF("${PROFILE}" STREQUAL "tv")
	ADD_DEFINITIONS("-D_TV_")
ENDIF("${PROFILE}" STREQUAL "tv")

pkg_check_modules(daemon_pkgs REQUIRED ${provider_deps})

INCLUDE_DIRECTORIES(${daemon_pkgs_INCLUDE_DIRS})
LINK_DIRECTORIES(${daemon_pkgs_LIBRARY_DIRS})

ADD_DEFINITIONS("-fPIE")
ADD_DEFINITIONS("-DCONV_DBUS_INTERFACE=\"${DBUS_INTERFACE}\"")

ADD_EXECUTABLE(${DAEMON} ${DAEMON_SRCS})

TARGET_LINK_LIBRARIES(${DAEMON} ${daemon_pkgs_LIBRARIES} -lpthread -pie)

INSTALL(TARGETS ${DAEMON} DESTINATION ${BIN_INSTALL_DIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d2d-conv-manager-iotcon-server.dat DESTINATION
       /usr/share/d2d-conv-manager)
#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/d2d-conv-manager-iotcon-server.dat DESTINATION
#       ${SHARE_INSTALL_PREFIX}/d2d-conv-manager)
