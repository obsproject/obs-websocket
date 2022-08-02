cmake_minimum_required(VERSION 3.16...3.21)

project(obs-websocket-compat VERSION 4.9.1)
add_library(${CMAKE_PROJECT_NAME} MODULE)

set(PLUGIN_AUTHOR "OBS Project")

set(MACOS_BUNDLEID "com.obsproject.${CMAKE_PROJECT_NAME}")

set(LINUX_MAINTAINER_EMAIL "tt2468@irltoolkit.com")

target_compile_definitions(obs-websocket-compat PRIVATE ASIO_STANDALONE)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/obs-websocket.cpp
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
          src/forms/settings-dialog.cpp
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

target_include_directories(
  obs-websocket-compat PRIVATE ${CMAKE_SOURCE_DIR}/deps/asio/asio/include
                               ${CMAKE_SOURCE_DIR}/deps/websocketpp)

# /!\ TAKE NOTE: No need to edit things past this point /!\

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
include(cmake/ObsPluginHelpers.cmake)
find_qt(COMPONENTS Widgets Core)

# --- Platform-independent build settings ---

target_include_directories(${CMAKE_PROJECT_NAME}
                           PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(
  ${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs OBS::obs-frontend-api Qt::Core
                                Qt::Widgets)

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /MP /d2FH4-)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE -Werror-implicit-function-declaration
                                  -stdlib=libc++ -fvisibility=default)

  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})
