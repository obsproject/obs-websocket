project(obs-websocket VERSION 5.0.1)
set(OBS_WEBSOCKET_RPC_VERSION 1)

if(NOT ENABLE_UI)
  message(STATUS "OBS:  DISABLED   obs-websocket")
  return()
endif()

# Plugin tests flag
option(PLUGIN_TESTS "Enable plugin runtime tests" OFF)

# Qt build stuff
set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # For resources.qrc

# Find Qt
if(NOT COMMAND find_qt)
  if(NOT QT_VERSION)
    set(QT_VERSION
        AUTO
        CACHE STRING "obs-websocket Qt version [AUTO, 5, 6]" FORCE)
    set_property(CACHE QT_VERSION PROPERTY STRINGS AUTO 5 6)
  endif()

  macro(find_qt)
    set(multiValueArgs COMPONENTS COMPONENTS_WIN COMPONENTS_MAC
                       COMPONENTS_LINUX)
    cmake_parse_arguments(FIND_QT "" "${oneValueArgs}" "${multiValueArgs}"
                          ${ARGN})
    set(QT_NO_CREATE_VERSIONLESS_TARGETS ON)
    find_package(
      Qt5
      COMPONENTS Core
      QUIET)
    find_package(
      Qt6
      COMPONENTS Core
      QUIET)

    if(NOT _QT_VERSION AND QT_VERSION STREQUAL AUTO)
      if(TARGET Qt5::Core)
        set(_QT_VERSION
            5
            CACHE INTERNAL "")
      elseif(TARGET Qt6::Core)
        set(_QT_VERSION
            6
            CACHE INTERNAL "")
      endif()
    elseif(NOT _QT_VERSION)
      if(TARGET Qt${QT_VERSION}::Core)
        set(_QT_VERSION
            ${QT_VERSION}
            CACHE INTERNAL "")
      else()
        if(QT_VERSION EQUAL 6)
          set(FALLBACK_QT_VERSION 5)
        else()
          set(FALLBACK_QT_VERSION 6)
        endif()
        message(
          WARNING
            "Qt${QT_VERSION} was not found, falling back to Qt${FALLBACK_QT_VERSION}"
        )

        if(TARGET Qt${FALLBACK_QT_VERSION}::Core)
          set(_QT_VERSION
              ${FALLBACK_QT_VERSION}
              CACHE INTERNAL "")
        endif()
      endif()
    endif()

    set(QT_NO_CREATE_VERSIONLESS_TARGETS OFF)

    if(NOT _QT_VERSION)
      message(FATAL_ERROR "Neither Qt5 or Qt6 were found")
    endif()

    if(OS_WINDOWS)
      find_package(
        Qt${_QT_VERSION}
        COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_WIN}
        REQUIRED)
    elseif(OS_MACOS)
      find_package(
        Qt${_QT_VERSION}
        COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_MAC}
        REQUIRED)
    else()
      find_package(
        Qt${_QT_VERSION}
        COMPONENTS ${FIND_QT_COMPONENTS} ${FIND_QT_COMPONENTS_LINUX}
        REQUIRED)
    endif()

    list(APPEND FIND_QT_COMPONENTS "Core")

    if("Gui" IN_LIST FIND_QT_COMPONENTS_LINUX)
      list(APPEND FIND_QT_COMPONENTS_LINUX "GuiPrivate")
    endif()

    foreach(_COMPONENT IN LISTS FIND_QT_COMPONENTS FIND_QT_COMPONENTS_WIN
                                FIND_QT_COMPONENTS_MAC FIND_QT_COMPONENTS_LINUX)
      if(NOT TARGET Qt::${_COMPONENT} AND TARGET
                                          Qt${_QT_VERSION}::${_COMPONENT})

        add_library(Qt::${_COMPONENT} INTERFACE IMPORTED)
        set_target_properties(
          Qt::${_COMPONENT} PROPERTIES INTERFACE_LINK_LIBRARIES
                                       "Qt${_QT_VERSION}::${_COMPONENT}")
      endif()
    endforeach()
  endmacro()
endif()

find_qt(COMPONENTS Core Widgets Svg Network)

# Find nlohmann
set(JSON_BuildTests
    OFF
    CACHE INTERNAL "")
add_subdirectory(deps/json)

# Tell websocketpp not to use system boost
add_definitions(-DASIO_STANDALONE)

# Configure files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/plugin-macros.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin-macros.generated.h)

# Setup target
add_library(obs-websocket MODULE)
add_library(OBS::websocket ALIAS obs-websocket)

target_sources(
  obs-websocket
  PRIVATE src/obs-websocket.cpp
          src/obs-websocket.h
          src/Config.cpp
          src/Config.h
          lib/obs-websocket-api.h
          src/forms/SettingsDialog.cpp
          src/forms/SettingsDialog.h
          src/forms/ConnectInfo.cpp
          src/forms/ConnectInfo.h
          src/forms/resources.qrc
          src/WebSocketApi.cpp
          src/WebSocketApi.h
          src/websocketserver/WebSocketServer.cpp
          src/websocketserver/WebSocketServer_Protocol.cpp
          src/websocketserver/WebSocketServer.h
          src/websocketserver/rpc/WebSocketSession.cpp
          src/websocketserver/rpc/WebSocketSession.h
          src/websocketserver/types/WebSocketCloseCode.h
          src/websocketserver/types/WebSocketOpCode.h
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
          src/eventhandler/EventHandler.h
          src/eventhandler/types/EventSubscription.h
          src/requesthandler/RequestHandler.cpp
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
          src/requesthandler/RequestHandler.h
          src/requesthandler/RequestBatchHandler.cpp
          src/requesthandler/RequestBatchHandler.h
          src/requesthandler/rpc/Request.cpp
          src/requesthandler/rpc/Request.h
          src/requesthandler/rpc/RequestBatchRequest.cpp
          src/requesthandler/rpc/RequestBatchRequest.h
          src/requesthandler/rpc/RequestResult.cpp
          src/requesthandler/rpc/RequestResult.h
          src/requesthandler/types/RequestStatus.h
          src/requesthandler/types/RequestBatchExecutionType.h
          src/utils/Crypto.cpp
          src/utils/Crypto.h
          src/utils/Json.cpp
          src/utils/Json.h
          src/utils/Obs.cpp
          src/utils/Obs_StringHelper.cpp
          src/utils/Obs_NumberHelper.cpp
          src/utils/Obs_ArrayHelper.cpp
          src/utils/Obs_ObjectHelper.cpp
          src/utils/Obs_SearchHelper.cpp
          src/utils/Obs_ActionHelper.cpp
          src/utils/Obs.h
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

target_include_directories(
  obs-websocket
  PRIVATE ${Qt5Core_INCLUDES} ${Qt5Widgets_INCLUDES} ${Qt5Svg_INCLUDES}
          ${Qt5Network_INCLUDES} "deps/asio/asio/include" "deps/websocketpp")

target_link_libraries(
  obs-websocket
  PRIVATE OBS::libobs
          OBS::frontend-api
          Qt::Core
          Qt::Widgets
          Qt::Svg
          Qt::Network
          nlohmann_json::nlohmann_json)

target_compile_features(obs-websocket PRIVATE cxx_std_17)

set_target_properties(obs-websocket PROPERTIES FOLDER "plugins/obs-websocket")

if(PLUGIN_TESTS)
  target_compile_definitions(obs-websocket PRIVATE PLUGIN_TESTS)
endif()

# Random other things
if(WIN32)
  add_definitions(-D_WEBSOCKETPP_CPP11_STL_)
  target_compile_options(obs-websocket PRIVATE /wd4267)

  # Set the correct Windows preprocessor var for asio
  macro(get_WIN32_WINNT version)
    if(WIN32 AND CMAKE_SYSTEM_VERSION)
      set(ver ${CMAKE_SYSTEM_VERSION})
      string(REPLACE "." "" ver ${ver})
      string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

      set(${version} "0x${ver}")
    endif()
  endmacro()

  get_win32_winnt(ver)
  add_definitions(-D_WIN32_WINNT=${ver})
elseif(UNIX AND NOT APPLE)
  target_compile_options(
    obs-websocket PRIVATE -Wall -Wextra -Wno-missing-field-initializers
                          -Wno-variadic-macros -Wno-error=format-overflow)
elseif(APPLE)
  target_compile_options(
    obs-websocket PRIVATE -Wno-error=null-pointer-subtraction
                          -Wno-error=deprecated-declarations)
endif()

# Final CMake helpers
setup_plugin_target(obs-websocket)
setup_target_resources(obs-websocket "obs-plugins/obs-websocket")
