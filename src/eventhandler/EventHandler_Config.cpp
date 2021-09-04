#include "EventHandler.h"

void EventHandler::HandleCurrentSceneCollectionChanged()
{
	json eventData;
	eventData["sceneCollectionName"] = Utils::Obs::StringHelper::GetCurrentSceneCollection();
	BroadcastEvent(EventSubscription::Config, "CurrentSceneCollectionChanged", eventData);
}

void EventHandler::HandleSceneCollectionListChanged()
{
	json eventData;
	eventData["sceneCollections"] = Utils::Obs::ListHelper::GetSceneCollectionList();
	BroadcastEvent(EventSubscription::Config, "SceneCollectionListChanged", eventData);
}

void EventHandler::HandleCurrentProfileChanged()
{
	json eventData;
	eventData["profileName"] = Utils::Obs::StringHelper::GetCurrentProfile();
	BroadcastEvent(EventSubscription::Config, "CurrentProfileChanged", eventData);
}

void EventHandler::HandleProfileListChanged()
{
	json eventData;
	eventData["profiles"] = Utils::Obs::ListHelper::GetProfileList();
	BroadcastEvent(EventSubscription::Config, "ProfileListChanged", eventData);
}
