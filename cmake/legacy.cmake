project(obs-websocket VERSION 5.3.0)
set(OBS_WEBSOCKET_RPC_VERSION 1)

option(ENABLE_WEBSOCKET "Enable building OBS with websocket plugin" ON)

if(NOT ENABLE_WEBSOCKET OR NOT ENABLE_UI)
  message(STATUS "OBS:  DISABLED   obs-websocket")
  return()
endif()

# Plugin tests flag
option(PLUGIN_TESTS "Enable plugin runtime tests" OFF)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)

# Find nlohmann JSON
find_package(nlohmann_json 3 REQUIRED)

# Find qrcodegencpp
find_package(Libqrcodegencpp REQUIRED)

# Find WebSocket++
find_package(Websocketpp 0.8 REQUIRED)

# Find Asio
find_package(Asio 1.12.1 REQUIRED)

# Tell websocketpp not to use system boost
add_definitions(-DASIO_STANDALONE)

# Configure files
configure_file(src/plugin-macros.h.in plugin-macros.generated.h)

# Setup target
add_library(obs-websocket MODULE)
add_library(OBS::websocket ALIAS obs-websocket)

set_target_properties(
  obs-websocket
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

if(_QT_VERSION EQUAL 6 AND OS_WINDOWS)
  set_target_properties(obs-websocket PROPERTIES AUTORCC_OPTIONS "--format-version;1")
endif()

target_include_directories(obs-websocket PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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
          src/utils/Utils.h)

target_link_libraries(
  obs-websocket
  PRIVATE OBS::libobs
          OBS::frontend-api
          Qt::Core
          Qt::Widgets
          Qt::Svg
          Qt::Network
          nlohmann_json::nlohmann_json
          Websocketpp::Websocketpp
          Asio::Asio
          Libqrcodegencpp::Libqrcodegencpp)

target_compile_features(obs-websocket PRIVATE cxx_std_17)

set_target_properties(obs-websocket PROPERTIES FOLDER "plugins/obs-websocket")

if(PLUGIN_TESTS)
  target_compile_definitions(obs-websocket PRIVATE PLUGIN_TESTS)
endif()

# Random other things
if(WIN32)
  add_definitions(-D_WEBSOCKETPP_CPP11_STL_)
endif()

if(MSVC)
  target_compile_options(obs-websocket PRIVATE /wd4267 /wd4996)
else()
  target_compile_options(
    obs-websocket
    PRIVATE
      -Wall
      "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-error=format-overflow>"
      "$<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-error=null-pointer-subtraction;-Wno-error=deprecated-declarations>"
  )
endif()

# Final CMake helpers
setup_plugin_target(obs-websocket)
setup_target_resources(obs-websocket "obs-plugins/obs-websocket")
