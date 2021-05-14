#include "EventHandler.h"

#include "../plugin-macros.generated.h"

std::string GetCalldataString(const calldata_t *data, const char* name)
{
	const char* value = nullptr;
	calldata_get_string(data, name, &value);
	return value;
}

EventHandler::EventHandler(WebSocketServerPtr webSocketServer) :
	_webSocketServer(webSocketServer)
{
	blog(LOG_INFO, "[EventHandler::EventHandler] Setting up event handlers...");

	_cpuUsageInfo = os_cpu_usage_info_start();

	obs_frontend_add_event_callback(EventHandler::OnFrontendEvent, this);

	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_connect(coreSignalHandler, "source_create", SourceCreatedMultiHandler, this);
		signal_handler_connect(coreSignalHandler, "source_remove", SourceRemovedMultiHandler, this);
	}

	obs_enum_sources([](void* param, obs_source_t* source) {
		auto eventHandler = reinterpret_cast<EventHandler*>(param);
		eventHandler->ConnectSourceSignals(source);
		return true;
	}, this);

	blog(LOG_INFO, "[EventHandler::EventHandler] Finished.");
}

EventHandler::~EventHandler()
{
	blog(LOG_INFO, "[EventHandler::~EventHandler] Removing event handlers...");

	os_cpu_usage_info_destroy(_cpuUsageInfo);

	obs_frontend_remove_event_callback(EventHandler::OnFrontendEvent, this);

	signal_handler_t* coreSignalHandler = obs_get_signal_handler();
	if (coreSignalHandler) {
		signal_handler_disconnect(coreSignalHandler, "source_destroy", SourceCreatedMultiHandler, this);
		signal_handler_disconnect(coreSignalHandler, "source_remove", SourceRemovedMultiHandler, this);
	}

	obs_enum_sources([](void* param, obs_source_t* source) {
		auto eventHandler = reinterpret_cast<EventHandler*>(param);
		eventHandler->DisconnectSourceSignals(source);
		return true;
	}, this);

	blog(LOG_INFO, "[EventHandler::~EventHandler] Finished.");
}

void EventHandler::ConnectSourceSignals(obs_source_t *source)
{
	if (!source || obs_source_removed(source))
		return;

	DisconnectSourceSignals(source);

	signal_handler_t* sh = obs_source_get_signal_handler(source);

	signal_handler_connect(sh, "rename", SourceRenamedMultiHandler, this);
}
	
void EventHandler::DisconnectSourceSignals(obs_source_t *source)
{
	if (!source)
		return;

	signal_handler_t* sh = obs_source_get_signal_handler(source);

	signal_handler_disconnect(sh, "rename", SourceRenamedMultiHandler, this);
}

void EventHandler::OnFrontendEvent(enum obs_frontend_event event, void *private_data) {
	auto eventHandler = reinterpret_cast<EventHandler*>(private_data);

	switch (event) {
		// General
		case OBS_FRONTEND_EVENT_EXIT:
			eventHandler->HandleExitStarted();
			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
			eventHandler->HandleStudioModeStateChanged(true);
			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED:
			eventHandler->HandleStudioModeStateChanged(false);
			break;

		// Config
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
			eventHandler->HandleCurrentSceneCollectionChanged();
			break;
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED:
			eventHandler->HandleSceneCollectionListChanged();
			break;
		case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
			eventHandler->HandleCurrentProfileChanged();
			break;
		case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED:
			eventHandler->HandleProfileListChanged();
			break;

		// Scenes
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			eventHandler->HandleCurrentSceneChanged();
			break;
		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
			eventHandler->HandleCurrentPreviewSceneChanged();
			break;
		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
			eventHandler->HandleSceneListReindexed();
			break;

		// Transitions
		case OBS_FRONTEND_EVENT_TRANSITION_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_LIST_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED:
			break;

		// Outputs
		case OBS_FRONTEND_EVENT_STREAMING_STARTING:
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STARTED:
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STARTING:
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STARTED:
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING:
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED:
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING:
			break;
		case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED:
			break;
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED:
			break;
		case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED:
			break;

		default:
			break;
	}
}

void EventHandler::SourceCreatedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(data);

	OBSSource source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	obs_source_type sourceType = obs_source_get_type(source);
	switch (sourceType) {
		case OBS_SOURCE_TYPE_INPUT:
			eventHandler->HandleSceneCreated(source);
			break;
		case OBS_SOURCE_TYPE_FILTER:
			break;
		case OBS_SOURCE_TYPE_TRANSITION:
			break;
		case OBS_SOURCE_TYPE_SCENE:
			break;
		default:
			break;
	}
}

void EventHandler::SourceRemovedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(data);

	OBSSource source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	obs_source_type sourceType = obs_source_get_type(source);
	switch (sourceType) {
		case OBS_SOURCE_TYPE_INPUT:
			eventHandler->HandleSceneRemoved(source);
			break;
		case OBS_SOURCE_TYPE_FILTER:
			break;
		case OBS_SOURCE_TYPE_TRANSITION:
			break;
		case OBS_SOURCE_TYPE_SCENE:
			break;
		default:
			break;
	}
}

void EventHandler::SourceRenamedMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(data);

	OBSSource source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	std::string oldSourceName = GetCalldataString(data, "old_name");
	std::string sourceName = GetCalldataString(data, "new_name");
	if (oldSourceName.empty() || sourceName.empty())
		return;

	obs_source_type sourceType = obs_source_get_type(source);
	switch (sourceType) {
		case OBS_SOURCE_TYPE_INPUT:
			eventHandler->HandleSceneNameChanged(source, oldSourceName, sourceName);
			break;
		case OBS_SOURCE_TYPE_FILTER:
			break;
		case OBS_SOURCE_TYPE_TRANSITION:
			break;
		case OBS_SOURCE_TYPE_SCENE:
			break;
		default:
			break;
	}
}
