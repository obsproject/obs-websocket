#include "EventHandler.h"

#define CASE(x) case x: return #x;

std::string GetMediaInputActionString(ObsMediaInputAction action) {
	switch (action) {
		default:
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE)
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY)
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART)
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP)
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT)
		CASE(OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS)
	}
}

void EventHandler::SourceMediaPauseMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PAUSE);
}

void EventHandler::SourceMediaPlayMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PLAY);
}

void EventHandler::SourceMediaRestartMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_RESTART);
}

void EventHandler::SourceMediaStopMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_STOP);
}

void EventHandler::SourceMediaNextMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_NEXT);
}

void EventHandler::SourceMediaPreviousMultiHandler(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	eventHandler->HandleMediaInputActionTriggered(source, OBS_WEBSOCKET_MEDIA_INPUT_ACTION_PREVIOUS);
}

void EventHandler::HandleMediaInputPlaybackStarted(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventHandler->BroadcastEvent(EventSubscription::MediaInputs, "MediaInputPlaybackStarted", eventData);
}

void EventHandler::HandleMediaInputPlaybackEnded(void *param, calldata_t *data)
{
	auto eventHandler = reinterpret_cast<EventHandler*>(param);

	obs_source_t *source = GetCalldataPointer<obs_source_t>(data, "source");
	if (!source)
		return;

	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT)
		return;

	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventHandler->BroadcastEvent(EventSubscription::MediaInputs, "MediaInputPlaybackEnded", eventData);
}

void EventHandler::HandleMediaInputActionTriggered(obs_source_t *source, ObsMediaInputAction action)
{
	json eventData;
	eventData["inputName"] = obs_source_get_name(source);
	eventData["mediaAction"] = GetMediaInputActionString(action);
	BroadcastEvent(EventSubscription::MediaInputs, "MediaInputActionTriggered", eventData);
}