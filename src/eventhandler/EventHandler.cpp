#include "EventHandler.h"

EventHandler::EventHandler(WebSocketServerPtr webSocketServer) :
	_webSocketServer(webSocketServer)
{
	_cpuUsageInfo = os_cpu_usage_info_start();

	obs_frontend_add_event_callback(EventHandler::OnFrontendEvent, this);
}

EventHandler::~EventHandler()
{
	os_cpu_usage_info_destroy(_cpuUsageInfo);

	obs_frontend_remove_event_callback(EventHandler::OnFrontendEvent, this);
}

std::string EventHandler::GetCalldataString(const calldata_t *data, const char* name)
{
	const char* value = nullptr;
	calldata_get_string(data, name, &value);
	return value;
}

void EventHandler::OnFrontendEvent(enum obs_frontend_event event, void *private_data) {
	auto eventHandler = reinterpret_cast<EventHandler*>(private_data);

	switch (event) {
		// General
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			break;
		case OBS_FRONTEND_EVENT_EXIT:
			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED:
			break;
		case OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED:
			break;

		// Config
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_LIST_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_PROFILE_LIST_CHANGED:
			break;

		// Scenes
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED:
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
