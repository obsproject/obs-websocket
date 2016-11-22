cmake_minimum_required(VERSION 3.5)
project(obs-websocket)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include(external/FindLibObs.cmake)
find_package(LibObs REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_subdirectory(deps/mbedtls EXCLUDE_FROM_ALL)
set(ENABLED_PROGRAMS false)

set(obs-websocket_SOURCES 
	obs-websocket.cpp
	WSServer.cpp
	WSRequestHandler.cpp
	WSEvents.cpp
	Config.cpp
	Utils.cpp
	forms/settings-dialog.cpp)

set(obs-websocket_HEADERS
	obs-websocket.h
	WSServer.h
	WSRequestHandler.h
	WSEvents.h
	Config.h
	Utils.h
	forms/settings-dialog.h)

add_library(obs-websocket MODULE 
	${obs-websocket_SOURCES}
	${obs-websocket_HEADERS})
add_dependencies(obs-websocket mbedcrypto)
include_directories( 
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5WebSockets_INCLUDES}
	${Qt5Widgets_INCLUDES}
	${mbedcrypto_INCLUDES}
	"${CMAKE_SOURCE_DIR}/deps/mbedtls/include")
target_link_libraries(obs-websocket 
	libobs
	Qt5::Core
	Qt5::WebSockets
	Qt5::Widgets
	mbedcrypto)

if(WIN32)
	set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
	if(OBS_FRONTEND_LIB EQUAL "OBS_FRONTEND_LIB-NOTFOUND")
		message(FATAL_ERROR "Could not find OBS Frontend API\'s library !")
	endif()
	
	target_link_libraries(obs-websocket
		"${OBS_FRONTEND_LIB}")
	add_custom_command(TARGET obs-websocket POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy
		"${QTDIR}/bin/Qt5WebSockets.dll"
		"${QTDIR}/bin/Qt5Network.dll"
		"${CMAKE_BINARY_DIR}/$<CONFIG>")
endif()

if(UNIX)
	target_compile_options(mbedcrypto PRIVATE -fPIC)
	target_link_libraries(obs-websocket
		obs-frontend-api)
endif()

if(UNIX AND NOT APPLE)
	set_target_properties(obs-websocket PROPERTIES PREFIX "")
	install(TARGETS obs-websocket
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
	install(FILES data/locale/en-US.ini data/locale/fr-FR.ini
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-websocket/locale")
endif()
