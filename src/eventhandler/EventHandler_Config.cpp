#include "EventHandler.h"

#include "../plugin-macros.generated.h"

void EventHandler::HandleCurrentSceneCollectionChanged()
{
	json eventData;
	eventData["sceneCollectionName"] = Utils::Obs::StringHelper::GetCurrentSceneCollection();
	_webSocketServer->BroadcastEvent(EventSubscription::Config, "CurrentSceneCollectionChanged", eventData);
}

void EventHandler::HandleSceneCollectionListChanged()
{
	json eventData;
	eventData["sceneCollections"] = Utils::Obs::ListHelper::GetSceneCollectionList();
	_webSocketServer->BroadcastEvent(EventSubscription::Config, "SceneCollectionListChanged", eventData);
}

void EventHandler::HandleCurrentProfileChanged()
{
	json eventData;
	eventData["profileName"] = Utils::Obs::StringHelper::GetCurrentProfile();
	_webSocketServer->BroadcastEvent(EventSubscription::Config, "CurrentProfileChanged", eventData);
}

void EventHandler::HandleProfileListChanged()
{
	json eventData;
	eventData["profiles"] = Utils::Obs::ListHelper::GetProfileList();
	_webSocketServer->BroadcastEvent(EventSubscription::Config, "ProfileListChanged", eventData);
}
