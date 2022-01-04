cmake_minimum_required(VERSION 3.5)
project(obs-websocket-compat VERSION 4.9.1)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DASIO_STANDALONE)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
    set(CMAKE_CXX_FLAGS "-mfpu=neon")
endif()

if (WIN32 OR APPLE)
	include(external/FindLibObs.cmake)
endif()

find_package(LibObs REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(obs-websocket-compat_SOURCES
	src/obs-websocket.cpp
	src/WSServer.cpp
	src/ConnectionProperties.cpp
	src/WSRequestHandler.cpp
	src/WSRequestHandler_General.cpp
	src/WSRequestHandler_Profiles.cpp
	src/WSRequestHandler_Recording.cpp
	src/WSRequestHandler_ReplayBuffer.cpp
	src/WSRequestHandler_SceneCollections.cpp
	src/WSRequestHandler_Scenes.cpp
	src/WSRequestHandler_SceneItems.cpp
	src/WSRequestHandler_Sources.cpp
	src/WSRequestHandler_Streaming.cpp
	src/WSRequestHandler_VirtualCam.cpp
	src/WSRequestHandler_StudioMode.cpp
	src/WSRequestHandler_Transitions.cpp
	src/WSRequestHandler_Outputs.cpp
	src/WSRequestHandler_MediaControl.cpp
	src/WSEvents.cpp
	src/Config.cpp
	src/Utils.cpp
	src/rpc/RpcRequest.cpp
	src/rpc/RpcResponse.cpp
	src/rpc/RpcEvent.cpp
	src/protocol/OBSRemoteProtocol.cpp
	src/forms/settings-dialog.cpp)

set(obs-websocket-compat_HEADERS
	src/obs-websocket.h
	src/WSServer.h
	src/ConnectionProperties.h
	src/WSRequestHandler.h
	src/WSEvents.h
	src/Config.h
	src/Utils.h
	src/rpc/RpcRequest.h
	src/rpc/RpcResponse.h
	src/rpc/RpcEvent.h
	src/protocol/OBSRemoteProtocol.h
	src/forms/settings-dialog.h)

# --- Platform-independent build settings ---
add_library(obs-websocket-compat MODULE
	${obs-websocket-compat_SOURCES}
	${obs-websocket-compat_HEADERS})

include_directories(
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES}
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/asio/asio/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/websocketpp")

target_link_libraries(obs-websocket-compat
	libobs
	Qt5::Core
	Qt5::Widgets)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(WIN32)
	set(OBS_FRONTEND_LIB "NOTFOUND" CACHE FILEPATH "OBS frontend library")
	if(OBS_FRONTEND_LIB STREQUAL "NOTFOUND")
		message(FATAL_ERROR "OBS Frontend API library not found!")
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

	target_link_libraries(obs-websocket-compat
		"${OBS_FRONTEND_LIB}")

	# --- Release package helper ---
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

	add_custom_command(TARGET obs-websocket-compat POST_BUILD
		# If config is Release or RelWithDebInfo, package release files
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${RELEASE_DIR}/data/obs-plugins/obs-websocket-compat"
		)

		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>==1 (
			"${CMAKE_COMMAND}" -E copy
			"$<TARGET_FILE:obs-websocket-compat>"
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
			"$<TARGET_PDB_FILE:obs-websocket-compat>"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
		)

		# In the Debug configuration, copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-websocket-compat>"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
			"$<TARGET_PDB_FILE:obs-websocket-compat>"
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket-compat"
		)

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket-compat"
		)
	)
	# --- End of sub-section ---

endif()
# --- End of section ---

# --- Linux-specific build settings and tasks ---
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	set_target_properties(obs-websocket-compat PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket-compat obs-frontend-api)

	file(GLOB locale_files data/locale/*.ini)

    set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
		OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_WRITE GROUP_EXECUTE
		WORLD_READ WORLD_EXECUTE
	)

	if(${USE_UBUNTU_FIX})
		install(TARGETS obs-websocket-compat LIBRARY
			DESTINATION "/usr/lib/obs-plugins"
			PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	endif()
	install(TARGETS obs-websocket-compat LIBRARY
		DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/obs-plugins"
		PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/obs/obs-plugins/obs-websocket-compat/locale")
endif()
# --- End of section ---

# -- OS X specific build settings and tasks --
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fvisibility=default")

	set(CMAKE_SKIP_RPATH TRUE)
	set_target_properties(obs-websocket-compat PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket-compat "${OBS_FRONTEND_LIB}")
endif()
# -- End of section --
