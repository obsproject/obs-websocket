#pragma once

#include "RequestStatus.h"
#include "../../WebSocketSession.h"
#include "../../utils/Json.h"

enum ObsWebSocketSceneFilter {
	OBS_WEBSOCKET_SCENE_FILTER_SCENE_ONLY,
	OBS_WEBSOCKET_SCENE_FILTER_GROUP_ONLY,
	OBS_WEBSOCKET_SCENE_FILTER_SCENE_OR_GROUP,
};

struct Request
{
	Request(SessionPtr session, const std::string requestType, const json requestData = nullptr);

	const bool HasRequestData() const
	{
		return RequestData.is_object() && !RequestData.empty();
	}

	const bool ValidateBasic(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	const bool ValidateNumber(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const double minValue = -INFINITY, const double maxValue = INFINITY) const;
	const bool ValidateString(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;
	const bool ValidateBoolean(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	const bool ValidateObject(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;
	const bool ValidateArray(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const bool allowEmpty = false) const;

	// All return values have incremented refcounts
	obs_source_t *ValidateSource(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	obs_source_t *ValidateScene(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const ObsWebSocketSceneFilter filter = OBS_WEBSOCKET_SCENE_FILTER_SCENE_ONLY) const;
	obs_source_t *ValidateInput(const std::string keyName, RequestStatus::RequestStatus &statusCode, std::string &comment) const;
	obs_sceneitem_t *ValidateSceneItem(const std::string sceneKeyName, const std::string sceneItemIdKeyName, RequestStatus::RequestStatus &statusCode, std::string &comment, const ObsWebSocketSceneFilter filter = OBS_WEBSOCKET_SCENE_FILTER_SCENE_ONLY) const;

	SessionPtr Session;
	const uint8_t RpcVersion;
	const bool IgnoreNonFatalRequestChecks;
	const std::string RequestType;
	const json RequestData;
};