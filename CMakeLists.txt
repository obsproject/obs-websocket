cmake_minimum_required(VERSION 3.5)
project(obs-websocket)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include(external/FindLibObs.cmake)
find_package(LibObs REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5WebSockets REQUIRED)

set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
if(OBS_FRONTEND_LIB EQUAL "OBS_FRONTEND_LIB-NOTFOUND")
	message(FATAL_ERROR "Could not find OBS Frontend API\'s library !")
endif()

set(obs-websocket_SOURCES 
	obs-websocket.cpp
	WSServer.cpp
	WSRequestHandler.cpp
	WSEvents.cpp
	Config.cpp
	Utils.cpp)

set(obs-websocket_HEADERS
	obs-websocket.h
	WSServer.h
	WSRequestHandler.h
	WSEvents.h
	Config.h
	Utils.h)

add_library(obs-websocket MODULE 
	${obs-websocket_SOURCES}
	${obs-websocket_HEADERS})
qt5_use_modules(obs-websocket
	Core WebSockets)
include_directories( 
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5WebSockets_INCLUDES})
target_link_libraries(obs-websocket 
	libobs
	${OBS_FRONTEND_LIB}
	Qt5::Core
	Qt5::WebSockets)