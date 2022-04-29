cmake_minimum_required(VERSION 3.16...3.20)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(
    FATAL_ERROR
      "obs-websocket: You cannot build in a source directory (or any directory with "
      "CMakeLists.txt file). Either create and run from a build subdirectory or instruct "
      "CMake to create one for you (using the '-B' parameter). You can remove "
      "CMakeCache.txt and CMakeFiles in the project directory to clean up.")
endif()

project(obs-websocket VERSION 5.0.0)
add_library(obs-websocket MODULE)

# Set correct version string
if(DEFINED OBS_WEBSOCKET_VERSION_SUFFIX AND NOT OBS_WEBSOCKET_VERSION_SUFFIX
                                            STREQUAL "")
  set(OBS_WEBSOCKET_VERSION
      "${CMAKE_PROJECT_VERSION}${OBS_WEBSOCKET_VERSION_SUFFIX}")
  message(
    WARNING
      "-----------------------------------\n"
      "Version Suffix provided. OBS_WEBSOCKET_VERSION is now ${OBS_WEBSOCKET_VERSION}\n"
      "-----------------------------------")
else()
  set(OBS_WEBSOCKET_VERSION "${CMAKE_PROJECT_VERSION}")
endif()
set(OBS_WEBSOCKET_RPC_VERSION 1)
set(MACOS_BUNDLEID "fr.palakis.obs-websocket")
set(LINUX_MAINTAINER_EMAIL "tt2468@gmail.com")

option(PLUGIN_TESTS "Enable plugin runtime tests" OFF)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
include(cmake/ObsPluginHelpers.cmake)
find_qt(VERSION ${QT_VERSION} COMPONENTS Core Widgets Svg Network)

set(JSON_BuildTests
    OFF
    CACHE INTERNAL "")
add_subdirectory(deps/json)

configure_file(src/plugin-macros.h.in
               ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_include_directories(obs-websocket PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(
  obs-websocket
  OBS::libobs
  OBS::obs-frontend-api
  Qt::Core
  Qt::Widgets
  Qt::Network
  Qt::Svg
  nlohmann_json::nlohmann_json)

target_compile_features(obs-websocket PRIVATE cxx_std_17)
target_compile_definitions(obs-websocket PRIVATE ASIO_STANDALONE)

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm[^64]")
  set(CMAKE_CXX_FLAGS "-mfpu=neon")
endif()

set_target_properties(
  obs-websocket
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_sources(
  obs-websocket
  PRIVATE src/obs-websocket.cpp
          src/obs-websocket.h
          src/Config.cpp
          src/Config.h
          src/WebSocketApi.cpp
          src/WebSocketApi.h
          lib/obs-websocket-api.h
          src/websocketserver/WebSocketServer.cpp
          src/websocketserver/WebSocketServer.h
          src/websocketserver/WebSocketServer_Protocol.cpp
          src/websocketserver/rpc/WebSocketSession.cpp
          src/websocketserver/rpc/WebSocketSession.h
          src/websocketserver/types/WebSocketCloseCode.h
          src/websocketserver/types/WebSocketOpCode.h)

target_sources(
  obs-websocket
  PRIVATE src/forms/ConnectInfo.cpp src/forms/ConnectInfo.h
          src/forms/SettingsDialog.cpp src/forms/SettingsDialog.h
          src/forms/resources.qrc)

target_sources(
  obs-websocket
  PRIVATE src/utils/Crypto.cpp
          src/utils/Crypto.h
          src/utils/Json.cpp
          src/utils/Json.h
          src/utils/Obs.cpp
          src/utils/Obs.h
          src/utils/Obs_StringHelper.cpp
          src/utils/Obs_EnumHelper.cpp
          src/utils/Obs_NumberHelper.cpp
          src/utils/Obs_ArrayHelper.cpp
          src/utils/Obs_ObjectHelper.cpp
          src/utils/Obs_SearchHelper.cpp
          src/utils/Obs_ActionHelper.cpp
          src/utils/Obs_VolumeMeter.cpp
          src/utils/Obs_VolumeMeter.h
          src/utils/Obs_VolumeMeter_Helpers.h
          src/utils/Platform.cpp
          src/utils/Platform.h
          src/utils/Compat.cpp
          src/utils/Compat.h
          src/utils/Utils.h
          deps/qr/cpp/QrCode.cpp
          deps/qr/cpp/QrCode.hpp)

target_sources(
  obs-websocket
  PRIVATE src/eventhandler/EventHandler.cpp
          src/eventhandler/EventHandler.h
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
          src/eventhandler/types/EventSubscription.h)

target_sources(
  obs-websocket
  PRIVATE src/requesthandler/RequestHandler.cpp
          src/requesthandler/RequestHandler.h
          src/requesthandler/RequestBatchHandler.cpp
          src/requesthandler/RequestBatchHandler.h
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
          src/requesthandler/rpc/Request.h
          src/requesthandler/rpc/RequestBatchRequest.cpp
          src/requesthandler/rpc/RequestBatchRequest.h
          src/requesthandler/rpc/RequestResult.cpp
          src/requesthandler/rpc/RequestResult.h
          src/requesthandler/types/RequestStatus.h
          src/requesthandler/types/RequestBatchExecutionType.h)

target_include_directories(
  obs-websocket
  PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/deps/asio/asio/include
          ${CMAKE_SOURCE_DIR}/deps/websocketpp)

if(PLUGIN_TESTS)
  target_compile_definitions(obs-websocket PRIVATE PLUGIN_TESTS)
endif()

if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(obs-websocket PRIVATE /MP /d2FH4-)
  endif()

  target_compile_definitions(obs-websocket PRIVATE _WEBSOCKETPP_CPP11_STL_)
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macOS.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macOS.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(
    obs-websocket PRIVATE -Wall -Wextra -Werror-implicit-function-declaration
                          -stdlib=libc++ -fvisibility=default)

  set_target_properties(obs-websocket PROPERTIES PREFIX "")

else()
  target_compile_options(obs-websocket PRIVATE -Wall -Wextra -Wpedantic -Werror
                                               -Wno-missing-field-initializers)

  set_target_properties(obs-websocket PROPERTIES PREFIX "")
endif()

setup_plugin_target(obs-websocket)
