INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/daemon/lib)

SET(PTHREAD "pthread")

FILE(GLOB DAEMON_SRCS *.c *.cpp )
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} discovery_provider/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} service_provider/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} access_control/*.cpp)
FILE(GLOB DAEMON_SRCS ${DAEMON_SRCS} discovery_provider/iotcon/*.cpp)

SET(provider_deps "glib-2.0 dlog json-glib-1.0 iotcon capi-appfw-app-manager msf-api iotcon vconf capi-network-bluetooth capi-network-wifi-direct capi-appfw-application bundle capi-network-connection cynara-creds-gdbus cynara-client cynara-session capi-appfw-package-manager")
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

ADD_DEFINITIONS("-DCONV_DBUS_INTERFACE=\"${DBUS_INTERFACE}\"")

ADD_EXECUTABLE(${DAEMON} ${DAEMON_SRCS})

TARGET_LINK_LIBRARIES(${DAEMON} ${daemon_pkgs_LIBRARIES} -lpthread)


INSTALL(TARGETS ${DAEMON} DESTINATION ${BIN_INSTALL_DIR})

SET(DBUS_SERVICE_FILE "org.tizen.d2dconv")
CONFIGURE_FILE(${DBUS_SERVICE_FILE}.service.in ${DBUS_SERVICE_FILE}.service @ONLY)
INSTALL(FILES ${DBUS_SERVICE_FILE}.service DESTINATION ${SHARE_INSTALL_PREFIX}/dbus-1/services)
