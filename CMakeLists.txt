cmake_minimum_required(VERSION 3.16...3.20)

# Version variables
project(obs-websocket VERSION 5.0.0)
set(OBS_WEBSOCKET_RPC_VERSION 1)


# Set correct version string
if(DEFINED OBS_WEBSOCKET_VERSION_SUFFIX AND NOT OBS_WEBSOCKET_VERSION_SUFFIX STREQUAL "")
	set(OBS_WEBSOCKET_VERSION "${CMAKE_PROJECT_VERSION}${OBS_WEBSOCKET_VERSION_SUFFIX}")
	message(WARNING "-----------------------------------\nVersion Suffix provided. OBS_WEBSOCKET_VERSION is now ${OBS_WEBSOCKET_VERSION}\n-----------------------------------")
else()
	set(OBS_WEBSOCKET_VERSION "${CMAKE_PROJECT_VERSION}")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Prohibit in-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" _LOC_PATH)
if(EXISTS "${LOC_PATH}")
	message(FATAL_ERROR "obs-websocket: You cannot build in a source directory (or any directory with "
						"CMakeLists.txt file). Please make a build subdirectory. Feel free to "
						"remove CMakeCache.txt and CMakeFiles.")
endif()
unset(_LOC_PATH)


# Allow selection of common build types via UI
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
		"OBS build type [Release, RelWithDebInfo, Debug, MinSizeRel]" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release RelWithDebInfo Debug MinSizeRel)
endif()


# Plugin tests flag
option(PLUGIN_TESTS "Enable plugin runtime tests" OFF)


# Qt build stuff
set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON) # For resources.qrc

# Tell websocketpp not to use system boost
add_definitions(-DASIO_STANDALONE)


# Arm build fixes
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	set(CMAKE_CXX_FLAGS "-mfpu=neon")
endif()


# Find libobs
if (WIN32 OR APPLE)
	include(cmake/FindLibObs.cmake)
endif()
find_package(LibObs REQUIRED)


# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Svg Network)


# Find nlohmann
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(deps/json)


# Configure files
configure_file(
	src/plugin-macros.h.in
	../src/plugin-macros.generated.h
)
configure_file(
	installer/installer-windows.iss.in
	../installer/installer-windows.generated.iss
)


# Inlude sources
set(obs-websocket_SOURCES
	src/obs-websocket.cpp
	src/Config.cpp
	src/WebSocketApi.cpp
	src/websocketserver/WebSocketServer.cpp
	src/websocketserver/WebSocketServer_Protocol.cpp
	src/websocketserver/rpc/WebSocketSession.cpp
	src/eventhandler/EventHandler.cpp
	src/eventhandler/EventHandler_General.cpp
	src/eventhandler/EventHandler_Config.cpp
	src/eventhandler/EventHandler_Scenes.cpp
	src/eventhandler/EventHandler_Inputs.cpp
	src/eventhandler/EventHandler_Transitions.cpp
	src/eventhandler/EventHandler_Filters.cpp
	src/eventhandler/EventHandler_Outputs.cpp
	src/eventhandler/EventHandler_SceneItems.cpp
	src/eventhandler/EventHandler_MediaInputs.cpp
	src/eventhandler/EventHandler_Ui.cpp
	src/requesthandler/RequestHandler.cpp
	src/requesthandler/RequestBatchHandler.cpp
	src/requesthandler/RequestHandler_General.cpp
	src/requesthandler/RequestHandler_Config.cpp
	src/requesthandler/RequestHandler_Sources.cpp
	src/requesthandler/RequestHandler_Scenes.cpp
	src/requesthandler/RequestHandler_Inputs.cpp
	src/requesthandler/RequestHandler_Transitions.cpp
	src/requesthandler/RequestHandler_Filters.cpp
	src/requesthandler/RequestHandler_SceneItems.cpp
	src/requesthandler/RequestHandler_Outputs.cpp
	src/requesthandler/RequestHandler_Stream.cpp
	src/requesthandler/RequestHandler_Record.cpp
	src/requesthandler/RequestHandler_MediaInputs.cpp
	src/requesthandler/RequestHandler_Ui.cpp
	src/requesthandler/rpc/Request.cpp
	src/requesthandler/rpc/RequestBatchRequest.cpp
	src/requesthandler/rpc/RequestResult.cpp
	src/forms/SettingsDialog.cpp
	src/forms/ConnectInfo.cpp
	src/forms/resources.qrc
	src/utils/Crypto.cpp
	src/utils/Json.cpp
	src/utils/Obs.cpp
	src/utils/Obs_StringHelper.cpp
	src/utils/Obs_EnumHelper.cpp
	src/utils/Obs_NumberHelper.cpp
	src/utils/Obs_ArrayHelper.cpp
	src/utils/Obs_ObjectHelper.cpp
	src/utils/Obs_SearchHelper.cpp
	src/utils/Obs_ActionHelper.cpp
	src/utils/Obs_VolumeMeter.cpp
	src/utils/Platform.cpp
	src/utils/Compat.cpp
	deps/qr/cpp/QrCode.cpp)

set(obs-websocket_HEADERS
	src/obs-websocket.h
	src/Config.h
	src/WebSocketApi.h
	src/websocketserver/WebSocketServer.h
	src/websocketserver/types/WebSocketCloseCode.h
	src/websocketserver/types/WebSocketOpCode.h
	src/websocketserver/rpc/WebSocketSession.h
	src/eventhandler/EventHandler.h
	src/eventhandler/types/EventSubscription.h
	src/requesthandler/RequestHandler.h
	src/requesthandler/RequestBatchHandler.h
	src/requesthandler/types/RequestStatus.h
	src/requesthandler/types/RequestBatchExecutionType.h
	src/requesthandler/rpc/Request.h
	src/requesthandler/rpc/RequestBatchRequest.h
	src/requesthandler/rpc/RequestResult.h
	src/forms/SettingsDialog.h
	src/forms/ConnectInfo.h
	src/utils/Crypto.h
	src/utils/Json.h
	src/utils/Obs.h
	src/utils/Obs_VolumeMeter.h
	src/utils/Obs_VolumeMeter_Helpers.h
	src/utils/Platform.h
	src/utils/Compat.h
	src/utils/Utils.h
	lib/obs-websocket-api.h
	deps/qr/cpp/QrCode.hpp)


# Platform-independent build settings
add_library(obs-websocket MODULE
	${obs-websocket_SOURCES}
	${obs-websocket_HEADERS})

include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES}
	${Qt5Svg_INCLUDES}
	${Qt5Network_INCLUDES}
	"${CMAKE_SOURCE_DIR}/deps/asio/asio/include"
	"${CMAKE_SOURCE_DIR}/deps/websocketpp")

target_link_libraries(obs-websocket
	libobs
	Qt5::Core
	Qt5::Widgets
	Qt5::Svg
	Qt5::Network
	nlohmann_json::nlohmann_json)

if(PLUGIN_TESTS)
  target_compile_definitions(obs-websocket PRIVATE PLUGIN_TESTS)
endif()


# Windows-specific build settings and tasks
if(WIN32)
	if(NOT DEFINED OBS_FRONTEND_LIB)
		set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
		message(FATAL_ERROR "Could not find OBS Frontend API's library!")
	endif()

	if(MSVC)
		# Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL)
		add_definitions(/MP /d2FH4-)
	endif()

	add_definitions(-D_WEBSOCKETPP_CPP11_STL_)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
	endif()

	include_directories(
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/UI"
	)

	target_link_libraries(obs-websocket
		"${OBS_FRONTEND_LIB}")

	# Release package helper
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

	add_custom_command(TARGET obs-websocket POST_BUILD
		# If config is Release or RelWithDebInfo, package release files
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${RELEASE_DIR}/data/obs-plugins/obs-websocket"
		)

		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:obs-websocket>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
		)

		# In Release or RelWithDebInfo mode, copy Qt image format plugins
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/bin/${ARCH_NAME}/imageformats"
		)
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E copy
			"${QTDIR}/plugins/imageformats/qicns.dll"
			"${QTDIR}/plugins/imageformats/qico.dll"
			"${QTDIR}/plugins/imageformats/qjpeg.dll"
			"${QTDIR}/plugins/imageformats/qtiff.dll"
			"${QTDIR}/plugins/imageformats/qwbmp.dll"
			"${QTDIR}/plugins/imageformats/qwebp.dll"
			"${RELEASE_DIR}/bin/${ARCH_NAME}/imageformats"
		)

		# If config is RelWithDebInfo, package PDB file for target
		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
			"${CMAKE_COMMAND}" -E copy
			"$<TARGET_PDB_FILE:obs-websocket>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
		)

		# In the Debug configuration, copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-websocket>"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
			"$<TARGET_PDB_FILE:obs-websocket>"
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket"
		)
	)
endif()


# Linux-specific build settings and tasks
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	target_compile_options(obs-websocket PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers)

	set_target_properties(obs-websocket PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket obs-frontend-api)

	# Manually set permissions for locales
	file(GLOB locale_files data/locale/*.ini)
		set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
		OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	# Manually set file permissions for binary
	install(TARGETS obs-websocket LIBRARY
		DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/obs-plugins"
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	# OBS on Ubuntu installs into a different directory than most linux distros
	if(${USE_UBUNTU_FIX})
		install(TARGETS obs-websocket LIBRARY
			DESTINATION "/usr/lib/obs-plugins"
			PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	endif()

	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/obs-websocket/locale")
endif()

# MacOS-specific build settings and tasks
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fvisibility=default")

	set(CMAKE_SKIP_RPATH TRUE)
	set_target_properties(obs-websocket PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket "${OBS_FRONTEND_LIB}")
endif()
