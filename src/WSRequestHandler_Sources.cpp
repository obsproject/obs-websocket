#include <QtCore/QString>
#include <QtCore/QBuffer>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>
#include <QtGui/QImageWriter>

#include "Utils.h"

#include "WSRequestHandler.h"

bool isTextGDIPlusSource(const QString& sourceKind)
{
	return (sourceKind == "text_gdiplus" || sourceKind == "text_gdiplus_v2");
}

bool isTextFreeType2Source(const QString& sourceKind)
{
	return (sourceKind == "text_ft2_source" || sourceKind == "text_ft2_source_v2");
}

/**
 * Create a source and add it as a sceneitem to a scene.
 *
 * @param {String} `sourceName` Source name.
 * @param {String} `sourceKind` Source kind, Eg. `vlc_source`.
 * @param {String} `sceneName` Scene to add the new source to.
 * @param {Object (optional)} `sourceSettings` Source settings data.
 * @param {boolean (optional)} `setVisible` Set the created SceneItem as visible or not. Defaults to true
 *
 * @return {int} `itemId` ID of the SceneItem in the scene.
 *
 * @api requests
 * @name CreateSource
 * @category sources
 * @since 4.9.0
 */
RpcResponse WSRequestHandler::CreateSource(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("sourceKind") || !request.hasField("sceneName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	QString sourceKind = obs_data_get_string(request.parameters(), "sourceKind");
	if (sourceName.isEmpty() || sourceKind.isEmpty()) {
		return request.failed("empty sourceKind or sourceName parameters");
	}
	
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (source) {
		return request.failed("a source with that name already exists");
	}
	
	const char* sceneName = obs_data_get_string(request.parameters(), "sceneName");
	OBSSourceAutoRelease sceneSource = obs_get_source_by_name(sceneName);
	OBSScene scene = obs_scene_from_source(sceneSource);
	if (!scene) {
		return request.failed("requested scene is invalid or doesnt exist");
	}

	OBSDataAutoRelease sourceSettings = nullptr;
	if (request.hasField("sourceSettings")) {
		sourceSettings = obs_data_get_obj(request.parameters(), "sourceSettings");
	}

	OBSSourceAutoRelease newSource = obs_source_create(sourceKind.toUtf8(), sourceName.toUtf8(), sourceSettings, nullptr);

	if (!newSource) {
		return request.failed("failed to create the source");
	}
	obs_source_set_enabled(newSource, true);

	Utils::AddSourceData data;
	data.source = newSource;
	data.setVisible = true;
	if (request.hasField("setVisible")) {
		data.setVisible = obs_data_get_bool(request.parameters(), "setVisible");
	}
	
	obs_enter_graphics();
	obs_scene_atomic_update(scene, Utils::AddSourceHelper, &data);
	obs_leave_graphics();
	
	OBSDataAutoRelease responseData = obs_data_create();
	obs_data_set_int(responseData, "itemId", obs_sceneitem_get_id(data.sceneItem));

	return request.success(responseData);
}

/**
* List all sources available in the running OBS instance
*
* @return {Array<Object>} `sources` Array of sources
* @return {String} `sources.*.name` Unique source name
* @return {String} `sources.*.typeId` Non-unique source internal type (a.k.a kind)
* @return {String} `sources.*.type` Source type. Value is one of the following: "input", "filter", "transition", "scene" or "unknown"
*
* @api requests
* @name GetSourcesList
* @category sources
* @since 4.3.0
*/
RpcResponse WSRequestHandler::GetSourcesList(const RpcRequest& request)
{
	OBSDataArrayAutoRelease sourcesArray = obs_data_array_create();

	auto sourceEnumProc = [](void* privateData, obs_source_t* source) -> bool {
		obs_data_array_t* sourcesArray = (obs_data_array_t*)privateData;

		OBSDataAutoRelease sourceData = obs_data_create();
		obs_data_set_string(sourceData, "name", obs_source_get_name(source));
		obs_data_set_string(sourceData, "typeId", obs_source_get_id(source));

		QString typeString = "";
		enum obs_source_type sourceType = obs_source_get_type(source);
		switch (sourceType) {
		case OBS_SOURCE_TYPE_INPUT:
			typeString = "input";
			break;

		case OBS_SOURCE_TYPE_FILTER:
			typeString = "filter";
			break;

		case OBS_SOURCE_TYPE_TRANSITION:
			typeString = "transition";
			break;

		case OBS_SOURCE_TYPE_SCENE:
			typeString = "scene";
			break;

		default:
			typeString = "unknown";
			break;
		}
		obs_data_set_string(sourceData, "type", typeString.toUtf8());

		obs_data_array_push_back(sourcesArray, sourceData);
		return true;
	};
	obs_enum_sources(sourceEnumProc, sourcesArray);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "sources", sourcesArray);
	return request.success(response);
}

/**
* Get a list of all available sources types
*
* @return {Array<Object>} `types` Array of source types
* @return {String} `types.*.typeId` Non-unique internal source type ID
* @return {String} `types.*.displayName` Display name of the source type
* @return {String} `types.*.type` Type. Value is one of the following: "input", "filter", "transition" or "other"
* @return {Object} `types.*.defaultSettings` Default settings of this source type
* @return {Object} `types.*.caps` Source type capabilities
* @return {Boolean} `types.*.caps.isAsync` True if source of this type provide frames asynchronously
* @return {Boolean} `types.*.caps.hasVideo` True if sources of this type provide video
* @return {Boolean} `types.*.caps.hasAudio` True if sources of this type provide audio
* @return {Boolean} `types.*.caps.canInteract` True if interaction with this sources of this type is possible
* @return {Boolean} `types.*.caps.isComposite` True if sources of this type composite one or more sub-sources
* @return {Boolean} `types.*.caps.doNotDuplicate` True if sources of this type should not be fully duplicated
* @return {Boolean} `types.*.caps.doNotSelfMonitor` True if sources of this type may cause a feedback loop if it's audio is monitored and shouldn't be
*
* @api requests
* @name GetSourceTypesList
* @category sources
* @since 4.3.0
*/
RpcResponse WSRequestHandler::GetSourceTypesList(const RpcRequest& request)
{
	OBSDataArrayAutoRelease idsArray = obs_data_array_create();

	const char* id;
	size_t idx = 0;

	QHash<QString, QString> idTypes;

	idx = 0;
	while (obs_enum_input_types(idx++, &id)) {
		idTypes.insert(id, "input");
	}

	idx = 0;
	while (obs_enum_filter_types(idx++, &id)) {
		idTypes.insert(id, "filter");
	}

	idx = 0;
	while (obs_enum_transition_types(idx++, &id)) {
		idTypes.insert(id, "transition");
	}

	idx = 0;
	while (obs_enum_source_types(idx++, &id)) {
		OBSDataAutoRelease item = obs_data_create();

		obs_data_set_string(item, "typeId", id);
		obs_data_set_string(item, "displayName", obs_source_get_display_name(id));
		obs_data_set_string(item, "type", idTypes.value(id, "other").toUtf8());

		uint32_t caps = obs_get_source_output_flags(id);
		OBSDataAutoRelease capsData = obs_data_create();
		obs_data_set_bool(capsData, "isAsync", caps & OBS_SOURCE_ASYNC);
		obs_data_set_bool(capsData, "hasVideo", caps & OBS_SOURCE_VIDEO);
		obs_data_set_bool(capsData, "hasAudio", caps & OBS_SOURCE_AUDIO);
		obs_data_set_bool(capsData, "canInteract", caps & OBS_SOURCE_INTERACTION);
		obs_data_set_bool(capsData, "isComposite", caps & OBS_SOURCE_COMPOSITE);
		obs_data_set_bool(capsData, "doNotDuplicate", caps & OBS_SOURCE_DO_NOT_DUPLICATE);
		obs_data_set_bool(capsData, "doNotSelfMonitor", caps & OBS_SOURCE_DO_NOT_SELF_MONITOR);
		obs_data_set_bool(capsData, "isDeprecated", caps & OBS_SOURCE_DEPRECATED);

		obs_data_set_obj(item, "caps", capsData);

		OBSDataAutoRelease defaultSettings = obs_get_source_defaults(id);
		obs_data_set_obj(item, "defaultSettings", defaultSettings);

		obs_data_array_push_back(idsArray, item);
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "types", idsArray);
	return request.success(response);
}

/**
* Get the volume of the specified source. Default response uses mul format, NOT SLIDER PERCENTAGE.
*
* @param {String} `source` Source name.
* @param {boolean (optional)} `useDecibel` Output volume in decibels of attenuation instead of amplitude/mul.
*
* @return {String} `name` Source name.
* @return {double} `volume` Volume of the source. Between `0.0` and `20.0` if using mul, under `26.0` if using dB.
* @return {boolean} `muted` Indicates whether the source is muted.
*
* @api requests
* @name GetVolume
* @category sources
* @since 4.0.0
*/
RpcResponse WSRequestHandler::GetVolume(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	float volume = obs_source_get_volume(source);

	bool useDecibel = obs_data_get_bool(request.parameters(), "useDecibel");
	if (useDecibel) {
		volume = obs_mul_to_db(volume);
	}
	
	if (volume == -INFINITY) {
		volume = -100.0;
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name", obs_source_get_name(source));
	obs_data_set_double(response, "volume", volume);
	obs_data_set_bool(response, "muted", obs_source_muted(source));
	return request.success(response);
}

/**
* Set the volume of the specified source. Default request format uses mul, NOT SLIDER PERCENTAGE.
*
* @param {String} `source` Source name.
* @param {double} `volume` Desired volume. Must be between `0.0` and `20.0` for mul, and under 26.0 for dB. OBS will interpret dB values under -100.0 as Inf. Note: The OBS volume sliders only reach a maximum of 1.0mul/0.0dB, however OBS actually supports larger values.
* @param {boolean (optional)} `useDecibel` Interperet `volume` data as decibels instead of amplitude/mul.
*
* @api requests
* @name SetVolume
* @category sources
* @since 4.0.0
*/
RpcResponse WSRequestHandler::SetVolume(const RpcRequest& request)
 {
	if (!request.hasField("source") || !request.hasField("volume")) {
		return request.failed("missing request parameters");
	}

	bool useDecibel = obs_data_get_bool(request.parameters(), "useDecibel");

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	float sourceVolume = obs_data_get_double(request.parameters(), "volume");

	bool isNotValidDecibel = (useDecibel && sourceVolume > 26.0);
	bool isNotValidMul = (!useDecibel && (sourceVolume < 0.0 || sourceVolume > 20.0));
	if (sourceName.isEmpty() || isNotValidDecibel || isNotValidMul) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	if (useDecibel) {
		sourceVolume = obs_db_to_mul(sourceVolume);
	}
	obs_source_set_volume(source, sourceVolume);

	return request.success();
}

/**
* Changes whether an audio track is active for a source.
*
* @param {String} `sourceName` Source name.
* @param {int} `track` Audio tracks 1-6.
* @param {boolean} `active` Whether audio track is active or not.
*
* @api requests
* @name SetAudioTracks
* @category sources
* @since 4.9.1
*/
RpcResponse WSRequestHandler::SetAudioTracks(const RpcRequest& request)
 {
	if (!request.hasField("sourceName") || !request.hasField("track") || !request.hasField("active")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	bool active = obs_data_get_bool(request.parameters(), "active");
	int track = obs_data_get_int(request.parameters(), "track")-1;

	if (sourceName.isEmpty() || track > 5 || track < 0) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	uint32_t mixers = obs_source_get_audio_mixers(source);

	if (active && !(mixers & (1 << track)))
		mixers |= (1 << track);
	else if (mixers & (1 << track))
		mixers &= ~(1 << track);

	obs_source_set_audio_mixers(source, mixers);

	return request.success();
}


/**
* Gets whether an audio track is active for a source.
*
* @param {String} `sourceName` Source name.
* 
* @return {boolean} `track1`
* @return {boolean} `track2`
* @return {boolean} `track3`
* @return {boolean} `track4`
* @return {boolean} `track5`
* @return {boolean} `track6`
*
* @api requests
* @name GetAudioTracks
* @category sources
* @since 4.9.1
*/
RpcResponse WSRequestHandler::GetAudioTracks(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	uint32_t mixers = obs_source_get_audio_mixers(source);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name", obs_source_get_name(source));
	obs_data_set_bool(response, "track1", mixers & (1 << 0));
	obs_data_set_bool(response, "track2", mixers & (1 << 1));
	obs_data_set_bool(response, "track3", mixers & (1 << 2));
	obs_data_set_bool(response, "track4", mixers & (1 << 3));
	obs_data_set_bool(response, "track5", mixers & (1 << 4));
	obs_data_set_bool(response, "track6", mixers & (1 << 5));
	return request.success(response);
}

/**
* Get the mute status of a specified source.
*
* @param {String} `source` Source name.
*
* @return {String} `name` Source name.
* @return {boolean} `muted` Mute status of the source.
*
* @api requests
* @name GetMute
* @category sources
* @since 4.0.0
*/
RpcResponse WSRequestHandler::GetMute(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name", obs_source_get_name(source));
	obs_data_set_bool(response, "muted", obs_source_muted(source));

	return request.success(response);
}

/**
 * Sets the mute status of a specified source.
 *
 * @param {String} `source` Source name.
 * @param {boolean} `mute` Desired mute status.
 *
 * @api requests
 * @name SetMute
 * @category sources
 * @since 4.0.0
 */
RpcResponse WSRequestHandler::SetMute(const RpcRequest& request)
{
	if (!request.hasField("source") || !request.hasField("mute")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	bool mute = obs_data_get_bool(request.parameters(), "mute");

	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_set_muted(source, mute);
	return request.success();
}

/**
* Inverts the mute status of a specified source.
*
* @param {String} `source` Source name.
*
* @api requests
* @name ToggleMute
* @category sources
* @since 4.0.0
*/
RpcResponse WSRequestHandler::ToggleMute(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("invalid request parameters");
	}

	obs_source_set_muted(source, !obs_source_muted(source));
	return request.success();
}

/**
* Get the source's active status of a specified source (if it is showing in the final mix).
*
* @param {String} `sourceName` Source name.
*
* @return {boolean} `sourceActive` Source active status of the source.
*
* @api requests
* @name GetSourceActive
* @category sources
* @since 4.9.1
*/
RpcResponse WSRequestHandler::GetSourceActive(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_bool(response, "sourceActive", obs_source_active(source));

	return request.success(response);
}

/**
* Get the audio's active status of a specified source.
*
* @param {String} `sourceName` Source name.
*
* @return {boolean} `audioActive` Audio active status of the source.
*
* @api requests
* @name GetAudioActive
* @category sources
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetAudioActive(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_bool(response, "audioActive", obs_source_audio_active(source));

	return request.success(response);
}

/**
* Sets (aka rename) the name of a source. Also works with scenes since scenes are technically sources in OBS.
*
* Note: If the new name already exists as a source, obs-websocket will return an error.
*
* @param {String} `sourceName` Source name.
* @param {String} `newName` New source name.
*
* @api requests
* @name SetSourceName
* @category sources
* @since 4.8.0
*/
RpcResponse WSRequestHandler::SetSourceName(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("newName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	QString newName = obs_data_get_string(request.parameters(), "newName");
	if (sourceName.isEmpty() || newName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease existingSource = obs_get_source_by_name(newName.toUtf8());
	if (!existingSource) { // OBS is supposed to automatically rename colliding source names, but it doesn't. So this gets to be the solution for now.
		obs_source_set_name(source, newName.toUtf8());

		return request.success();
	} else {
		return request.failed("a source with that name already exists");
	}
}

/**
 * Set the audio sync offset of a specified source.
 *
 * @param {String} `source` Source name.
 * @param {int} `offset` The desired audio sync offset (in nanoseconds).
 *
 * @api requests
 * @name SetSyncOffset
 * @category sources
 * @since 4.2.0
 */
RpcResponse WSRequestHandler::SetSyncOffset(const RpcRequest& request)
{
	if (!request.hasField("source") || !request.hasField("offset")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	int64_t sourceSyncOffset = (int64_t)obs_data_get_int(request.parameters(), "offset");

	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	obs_source_set_sync_offset(source, sourceSyncOffset);
	return request.success();
}

/**
 * Get the audio sync offset of a specified source.
 *
 * @param {String} `source` Source name.
 *
 * @return {String} `name` Source name.
 * @return {int} `offset` The audio sync offset (in nanoseconds).
 *
 * @api requests
 * @name GetSyncOffset
 * @category sources
 * @since 4.2.0
 */
RpcResponse WSRequestHandler::GetSyncOffset(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "source");
	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "name", obs_source_get_name(source));
	obs_data_set_int(response, "offset", obs_source_get_sync_offset(source));

	return request.success(response);
}

/**
* Get settings of the specified source
*
* @param {String} `sourceName` Source name.
* @param {String (optional)} `sourceType` Type of the specified source. Useful for type-checking if you expect a specific settings schema.
*
* @return {String} `sourceName` Source name
* @return {String} `sourceType` Type of the specified source
* @return {Object} `sourceSettings` Source settings (varies between source types, may require some probing around).
*
* @api requests
* @name GetSourceSettings
* @category sources
* @since 4.3.0
*/
RpcResponse WSRequestHandler::GetSourceSettings(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);

	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	if (request.hasField("sourceType")) {
		QString actualSourceType = obs_source_get_id(source);
		QString requestedType = obs_data_get_string(request.parameters(), "sourceType");

		if (actualSourceType != requestedType) {
			return request.failed("specified source exists but is not of expected type");
		}
	}

	OBSDataAutoRelease sourceSettings = obs_source_get_settings(source);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "sourceName", obs_source_get_name(source));
	obs_data_set_string(response, "sourceType", obs_source_get_id(source));
	obs_data_set_obj(response, "sourceSettings", sourceSettings);

	return request.success(response);
}

/**
* Set settings of the specified source.
*
* @param {String} `sourceName` Source name.
* @param {String (optional)} `sourceType` Type of the specified source. Useful for type-checking to avoid settings a set of settings incompatible with the actual source's type.
* @param {Object} `sourceSettings` Source settings (varies between source types, may require some probing around).
*
* @return {String} `sourceName` Source name
* @return {String} `sourceType` Type of the specified source
* @return {Object} `sourceSettings` Updated source settings
*
* @api requests
* @name SetSourceSettings
* @category sources
* @since 4.3.0
*/
RpcResponse WSRequestHandler::SetSourceSettings(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("sourceSettings")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	if (request.hasField("sourceType")) {
		QString actualSourceType = obs_source_get_id(source);
		QString requestedType = obs_data_get_string(request.parameters(), "sourceType");

		if (actualSourceType != requestedType) {
			return request.failed("specified source exists but is not of expected type");
		}
	}

	OBSDataAutoRelease newSettings = obs_data_get_obj(request.parameters(), "sourceSettings");

	obs_source_update(source, newSettings);
	obs_source_update_properties(source);

	OBSDataAutoRelease updatedSettings = obs_source_get_settings(source);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "sourceName", obs_source_get_name(source));
	obs_data_set_string(response, "sourceType", obs_source_get_id(source));
	obs_data_set_obj(response, "sourceSettings", updatedSettings);
	return request.success(response);
}

/**
 * Get the current properties of a Text GDI Plus source.
 *
 * @param {String} `source` Source name.
 *
 * @return {String} `source` Source name.
 * @return {String} `align` Text Alignment ("left", "center", "right").
 * @return {int} `bk_color` Background color.
 * @return {int} `bk_opacity` Background opacity (0-100).
 * @return {boolean} `chatlog` Chat log.
 * @return {int} `chatlog_lines` Chat log lines.
 * @return {int} `color` Text color.
 * @return {boolean} `extents` Extents wrap.
 * @return {int} `extents_cx` Extents cx.
 * @return {int} `extents_cy` Extents cy.
 * @return {String} `file` File path name.
 * @return {boolean} `read_from_file` Read text from the specified file.
 * @return {Object} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @return {String} `font.face` Font face.
 * @return {int} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @return {int} `font.size` Font text size.
 * @return {String} `font.style` Font Style (unknown function).
 * @return {boolean} `gradient` Gradient enabled.
 * @return {int} `gradient_color` Gradient color.
 * @return {float} `gradient_dir` Gradient direction.
 * @return {int} `gradient_opacity` Gradient opacity (0-100).
 * @return {boolean} `outline` Outline.
 * @return {int} `outline_color` Outline color.
 * @return {int} `outline_size` Outline size.
 * @return {int} `outline_opacity` Outline opacity (0-100).
 * @return {String} `text` Text content to be displayed.
 * @return {String} `valign` Text vertical alignment ("top", "center", "bottom").
 * @return {boolean} `vertical` Vertical text enabled.
 *
 * @api requests
 * @name GetTextGDIPlusProperties
 * @category sources
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::GetTextGDIPlusProperties(const RpcRequest& request)
 {
	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceKind = obs_source_get_id(source);
	if (!isTextGDIPlusSource(sourceKind)) {
		return request.failed("not a text gdi plus source");
	}

	OBSDataAutoRelease response = obs_source_get_settings(source);
	obs_data_set_string(response, "source", obs_source_get_name(source));

	return request.success(response);
}

/**
 * Set the current properties of a Text GDI Plus source.
 *
 * @param {String} `source` Name of the source.
 * @param {String (optional)} `align` Text Alignment ("left", "center", "right").
 * @param {int (optional)} `bk_color` Background color.
 * @param {int (optional)} `bk_opacity` Background opacity (0-100).
 * @param {boolean (optional)} `chatlog` Chat log.
 * @param {int (optional)} `chatlog_lines` Chat log lines.
 * @param {int (optional)} `color` Text color.
 * @param {boolean (optional)} `extents` Extents wrap.
 * @param {int (optional)} `extents_cx` Extents cx.
 * @param {int (optional)} `extents_cy` Extents cy.
 * @param {String (optional)} `file` File path name.
 * @param {boolean (optional)} `read_from_file` Read text from the specified file.
 * @param {Object (optional)} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @param {String (optional)} `font.face` Font face.
 * @param {int (optional)} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @param {int (optional)} `font.size` Font text size.
 * @param {String (optional)} `font.style` Font Style (unknown function).
 * @param {boolean (optional)} `gradient` Gradient enabled.
 * @param {int (optional)} `gradient_color` Gradient color.
 * @param {float (optional)} `gradient_dir` Gradient direction.
 * @param {int (optional)} `gradient_opacity` Gradient opacity (0-100).
 * @param {boolean (optional)} `outline` Outline.
 * @param {int (optional)} `outline_color` Outline color.
 * @param {int (optional)} `outline_size` Outline size.
 * @param {int (optional)} `outline_opacity` Outline opacity (0-100).
 * @param {String (optional)} `text` Text content to be displayed.
 * @param {String (optional)} `valign` Text vertical alignment ("top", "center", "bottom").
 * @param {boolean (optional)} `vertical` Vertical text enabled.
 * @param {boolean (optional)} `render` Visibility of the scene item.
 *
 * @api requests
 * @name SetTextGDIPlusProperties
 * @category sources
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::SetTextGDIPlusProperties(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceKind = obs_source_get_id(source);
	if (!isTextGDIPlusSource(sourceKind)) {
		return request.failed("not a text gdi plus source");
	}

	OBSDataAutoRelease settings = obs_source_get_settings(source);

	if (request.hasField("align")) {
		obs_data_set_string(settings, "align", obs_data_get_string(request.parameters(), "align"));
	}

	if (request.hasField("bk_color")) {
		obs_data_set_int(settings, "bk_color", obs_data_get_int(request.parameters(), "bk_color"));
	}

	if (request.hasField("bk-opacity")) {
		obs_data_set_int(settings, "bk_opacity", obs_data_get_int(request.parameters(), "bk_opacity"));
	}

	if (request.hasField("chatlog")) {
		obs_data_set_bool(settings, "chatlog", obs_data_get_bool(request.parameters(), "chatlog"));
	}

	if (request.hasField("chatlog_lines")) {
		obs_data_set_int(settings, "chatlog_lines", obs_data_get_int(request.parameters(), "chatlog_lines"));
	}

	if (request.hasField("color")) {
		obs_data_set_int(settings, "color", obs_data_get_int(request.parameters(), "color"));
	}

	if (request.hasField("extents")) {
		obs_data_set_bool(settings, "extents", obs_data_get_bool(request.parameters(), "extents"));
	}

	if (request.hasField("extents_wrap")) {
		obs_data_set_bool(settings, "extents_wrap", obs_data_get_bool(request.parameters(), "extents_wrap"));
	}

	if (request.hasField("extents_cx")) {
		obs_data_set_int(settings, "extents_cx", obs_data_get_int(request.parameters(), "extents_cx"));
	}

	if (request.hasField("extents_cy")) {
		obs_data_set_int(settings, "extents_cy", obs_data_get_int(request.parameters(), "extents_cy"));
	}

	if (request.hasField("file")) {
		obs_data_set_string(settings, "file", obs_data_get_string(request.parameters(), "file"));
	}

	if (request.hasField("font")) {
		OBSDataAutoRelease font_obj = obs_data_get_obj(settings, "font");
		if (font_obj) {
			OBSDataAutoRelease req_font_obj = obs_data_get_obj(request.parameters(), "font");

			if (obs_data_has_user_value(req_font_obj, "face")) {
				obs_data_set_string(font_obj, "face", obs_data_get_string(req_font_obj, "face"));
			}

			if (obs_data_has_user_value(req_font_obj, "flags")) {
				obs_data_set_int(font_obj, "flags", obs_data_get_int(req_font_obj, "flags"));
			}

			if (obs_data_has_user_value(req_font_obj, "size")) {
				obs_data_set_int(font_obj, "size", obs_data_get_int(req_font_obj, "size"));
			}

			if (obs_data_has_user_value(req_font_obj, "style")) {
				obs_data_set_string(font_obj, "style", obs_data_get_string(req_font_obj, "style"));
			}
		}
	}

	if (request.hasField("gradient")) {
		obs_data_set_bool(settings, "gradient", obs_data_get_bool(request.parameters(), "gradient"));
	}

	if (request.hasField("gradient_color")) {
		obs_data_set_int(settings, "gradient_color", obs_data_get_int(request.parameters(), "gradient_color"));
	}

	if (request.hasField("gradient_dir")) {
		obs_data_set_double(settings, "gradient_dir", obs_data_get_double(request.parameters(), "gradient_dir"));
	}

	if (request.hasField("gradient_opacity")) {
		obs_data_set_int(settings, "gradient_opacity", obs_data_get_int(request.parameters(), "gradient_opacity"));
	}

	if (request.hasField("outline")) {
		obs_data_set_bool(settings, "outline", obs_data_get_bool(request.parameters(), "outline"));
	}

	if (request.hasField("outline_size")) {
		obs_data_set_int(settings, "outline_size", obs_data_get_int(request.parameters(), "outline_size"));
	}

	if (request.hasField("outline_color")) {
		obs_data_set_int(settings, "outline_color", obs_data_get_int(request.parameters(), "outline_color"));
	}

	if (request.hasField("outline_opacity")) {
		obs_data_set_int(settings, "outline_opacity", obs_data_get_int(request.parameters(), "outline_opacity"));
	}

	if (request.hasField("read_from_file")) {
		obs_data_set_bool(settings, "read_from_file", obs_data_get_bool(request.parameters(), "read_from_file"));
	}

	if (request.hasField("text")) {
		obs_data_set_string(settings, "text", obs_data_get_string(request.parameters(), "text"));
	}

	if (request.hasField("valign")) {
		obs_data_set_string(settings, "valign", obs_data_get_string(request.parameters(), "valign"));
	}

	if (request.hasField("vertical")) {
		obs_data_set_bool(settings, "vertical", obs_data_get_bool(request.parameters(), "vertical"));
	}

	obs_source_update(source, settings);

	return request.success();
}

/**
 * Get the current properties of a Text Freetype 2 source.
 *
 * @param {String} `source` Source name.
 *
 * @return {String} `source` Source name
 * @return {int} `color1` Gradient top color.
 * @return {int} `color2` Gradient bottom color.
 * @return {int} `custom_width` Custom width (0 to disable).
 * @return {boolean} `drop_shadow` Drop shadow.
 * @return {Object} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @return {String} `font.face` Font face.
 * @return {int} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @return {int} `font.size` Font text size.
 * @return {String} `font.style` Font Style (unknown function).
 * @return {boolean} `from_file` Read text from the specified file.
 * @return {boolean} `log_mode` Chat log.
 * @return {boolean} `outline` Outline.
 * @return {String} `text` Text content to be displayed.
 * @return {String} `text_file` File path.
 * @return {boolean} `word_wrap` Word wrap.
 *
 * @api requests
 * @name GetTextFreetype2Properties
 * @category sources
 * @since 4.5.0
 */
RpcResponse WSRequestHandler::GetTextFreetype2Properties(const RpcRequest& request)
{
	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceKind = obs_source_get_id(source);
	if (!isTextFreeType2Source(sourceKind)) {
		return request.failed("not a freetype 2 source");
	}

	OBSDataAutoRelease response = obs_source_get_settings(source);
	obs_data_set_string(response, "source", sourceName);

	return request.success(response);
}

/**
 * Set the current properties of a Text Freetype 2 source.
 *
 * @param {String} `source` Source name.
 * @param {int (optional)} `color1` Gradient top color.
 * @param {int (optional)} `color2` Gradient bottom color.
 * @param {int (optional)} `custom_width` Custom width (0 to disable).
 * @param {boolean (optional)} `drop_shadow` Drop shadow.
 * @param {Object (optional)} `font` Holds data for the font. Ex: `"font": { "face": "Arial", "flags": 0, "size": 150, "style": "" }`
 * @param {String (optional)} `font.face` Font face.
 * @param {int (optional)} `font.flags` Font text styling flag. `Bold=1, Italic=2, Bold Italic=3, Underline=5, Strikeout=8`
 * @param {int (optional)} `font.size` Font text size.
 * @param {String (optional)} `font.style` Font Style (unknown function).
 * @param {boolean (optional)} `from_file` Read text from the specified file.
 * @param {boolean (optional)} `log_mode` Chat log.
 * @param {boolean (optional)} `outline` Outline.
 * @param {String (optional)} `text` Text content to be displayed.
 * @param {String (optional)} `text_file` File path.
 * @param {boolean (optional)} `word_wrap` Word wrap.
 *
 * @api requests
 * @name SetTextFreetype2Properties
 * @category sources
 * @since 4.5.0
 */
RpcResponse WSRequestHandler::SetTextFreetype2Properties(const RpcRequest& request)
{
	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceKind = obs_source_get_id(source);
	if (!isTextFreeType2Source(sourceKind)) {
		return request.failed("not text freetype 2 source");
	}

	OBSDataAutoRelease settings = obs_source_get_settings(source);

	if (request.hasField("color1")) {
		obs_data_set_int(settings, "color1", obs_data_get_int(request.parameters(), "color1"));
	}

	if (request.hasField("color2")) {
		obs_data_set_int(settings, "color2", obs_data_get_int(request.parameters(), "color2"));
	}

	if (request.hasField("custom_width")) {
		obs_data_set_int(settings, "custom_width", obs_data_get_int(request.parameters(), "custom_width"));
	}

	if (request.hasField("drop_shadow")) {
		obs_data_set_bool(settings, "drop_shadow", obs_data_get_bool(request.parameters(), "drop_shadow"));
	}

	if (request.hasField("font")) {
		OBSDataAutoRelease font_obj = obs_data_get_obj(settings, "font");
		if (font_obj) {
			OBSDataAutoRelease req_font_obj = obs_data_get_obj(request.parameters(), "font");

			if (obs_data_has_user_value(req_font_obj, "face")) {
				obs_data_set_string(font_obj, "face", obs_data_get_string(req_font_obj, "face"));
			}

			if (obs_data_has_user_value(req_font_obj, "flags")) {
				obs_data_set_int(font_obj, "flags", obs_data_get_int(req_font_obj, "flags"));
			}

			if (obs_data_has_user_value(req_font_obj, "size")) {
				obs_data_set_int(font_obj, "size", obs_data_get_int(req_font_obj, "size"));
			}

			if (obs_data_has_user_value(req_font_obj, "style")) {
				obs_data_set_string(font_obj, "style", obs_data_get_string(req_font_obj, "style"));
			}
		}
	}

	if (request.hasField("from_file")) {
		obs_data_set_bool(settings, "from_file", obs_data_get_bool(request.parameters(), "from_file"));
	}

	if (request.hasField("log_mode")) {
		obs_data_set_bool(settings, "log_mode", obs_data_get_bool(request.parameters(), "log_mode"));
	}

	if (request.hasField("outline")) {
		obs_data_set_bool(settings, "outline", obs_data_get_bool(request.parameters(), "outline"));
	}

	if (request.hasField("text")) {
		obs_data_set_string(settings, "text", obs_data_get_string(request.parameters(), "text"));
	}

	if (request.hasField("text_file")) {
		obs_data_set_string(settings, "text_file", obs_data_get_string(request.parameters(), "text_file"));
	}

	if (request.hasField("word_wrap")) {
		obs_data_set_bool(settings, "word_wrap", obs_data_get_bool(request.parameters(), "word_wrap"));
	}

	obs_source_update(source, settings);

	return request.success();
}

/**
 * Get current properties for a Browser Source.
 *
 * @param {String} `source` Source name.
 *
 * @return {String} `source` Source name.
 * @return {boolean} `is_local_file` Indicates that a local file is in use.
 * @return {String} `local_file` file path.
 * @return {String} `url` Url.
 * @return {String} `css` CSS to inject.
 * @return {int} `width` Width.
 * @return {int} `height` Height.
 * @return {int} `fps` Framerate.
 * @return {boolean} `shutdown` Indicates whether the source should be shutdown when not visible.
 *
 * @api requests
 * @name GetBrowserSourceProperties
 * @category sources
 * @since 4.1.0
 * @deprecated Since 4.8.0. Prefer the use of GetSourceSettings. Will be removed in v5.0.0
 */
RpcResponse WSRequestHandler::GetBrowserSourceProperties(const RpcRequest& request)
{
	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceId = obs_source_get_id(source);
	if (sourceId != "browser_source" && sourceId != "linuxbrowser-source") {
		return request.failed("not a browser source");
	}

	OBSDataAutoRelease response = obs_source_get_settings(source);
	obs_data_set_string(response, "source", obs_source_get_name(source));

	return request.success(response);
}

/**
 * Set current properties for a Browser Source.
 *
 * @param {String} `source` Name of the source.
 * @param {boolean (optional)} `is_local_file` Indicates that a local file is in use.
 * @param {String (optional)} `local_file` file path.
 * @param {String (optional)} `url` Url.
 * @param {String (optional)} `css` CSS to inject.
 * @param {int (optional)} `width` Width.
 * @param {int (optional)} `height` Height.
 * @param {int (optional)} `fps` Framerate.
 * @param {boolean (optional)} `shutdown` Indicates whether the source should be shutdown when not visible.
 * @param {boolean (optional)} `render` Visibility of the scene item.
 *
 * @api requests
 * @name SetBrowserSourceProperties
 * @category sources
 * @deprecated Since 4.8.0. Prefer the use of SetSourceSettings. Will be removed in v5.0.0
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::SetBrowserSourceProperties(const RpcRequest& request)
{
	if (!request.hasField("source")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "source");
	if (!sourceName) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	QString sourceId = obs_source_get_id(source);
	if(sourceId != "browser_source" && sourceId != "linuxbrowser-source") {
		return request.failed("not a browser source");
	}

	OBSDataAutoRelease settings = obs_source_get_settings(source);

	if (request.hasField("restart_when_active")) {
		obs_data_set_bool(settings, "restart_when_active", obs_data_get_bool(request.parameters(), "restart_when_active"));
	}

	if (request.hasField("shutdown")) {
		obs_data_set_bool(settings, "shutdown", obs_data_get_bool(request.parameters(), "shutdown"));
	}

	if (request.hasField("is_local_file")) {
		obs_data_set_bool(settings, "is_local_file", obs_data_get_bool(request.parameters(), "is_local_file"));
	}

	if (request.hasField("local_file")) {
		obs_data_set_string(settings, "local_file", obs_data_get_string(request.parameters(), "local_file"));
	}

	if (request.hasField("url")) {
		obs_data_set_string(settings, "url", obs_data_get_string(request.parameters(), "url"));
	}

	if (request.hasField("css")) {
		obs_data_set_string(settings, "css", obs_data_get_string(request.parameters(), "css"));
	}

	if (request.hasField("width")) {
		obs_data_set_int(settings, "width", obs_data_get_int(request.parameters(), "width"));
	}

	if (request.hasField("height")) {
		obs_data_set_int(settings, "height", obs_data_get_int(request.parameters(), "height"));
	}

	if (request.hasField("fps")) {
		obs_data_set_int(settings, "fps", obs_data_get_int(request.parameters(), "fps"));
	}

	obs_source_update(source, settings);

	return request.success();
}

/**
 * Get configured special sources like Desktop Audio and Mic/Aux sources.
 *
 * @return {String (optional)} `desktop-1` Name of the first Desktop Audio capture source.
 * @return {String (optional)} `desktop-2` Name of the second Desktop Audio capture source.
 * @return {String (optional)} `mic-1` Name of the first Mic/Aux input source.
 * @return {String (optional)} `mic-2` Name of the second Mic/Aux input source.
 * @return {String (optional)} `mic-3` NAme of the third Mic/Aux input source.
 *
 * @api requests
 * @name GetSpecialSources
 * @category sources
 * @since 4.1.0
 */
RpcResponse WSRequestHandler::GetSpecialSources(const RpcRequest& request)
 {
	OBSDataAutoRelease response = obs_data_create();

	QMap<const char*, int> sources;
	sources["desktop-1"] = 1;
	sources["desktop-2"] = 2;
	sources["mic-1"] = 3;
	sources["mic-2"] = 4;
	sources["mic-3"] = 5;

	QMapIterator<const char*, int> i(sources);
	while (i.hasNext()) {
		i.next();

		const char* id = i.key();
		OBSSourceAutoRelease source = obs_get_output_source(i.value());
		if (source) {
			obs_data_set_string(response, id, obs_source_get_name(source));
		}
	}

	return request.success(response);
}

/**
* List filters applied to a source
*
* @param {String} `sourceName` Source name
*
* @return {Array<Object>} `filters` List of filters for the specified source
* @return {Boolean} `filters.*.enabled` Filter status (enabled or not)
* @return {String} `filters.*.type` Filter type
* @return {String} `filters.*.name` Filter name
* @return {Object} `filters.*.settings` Filter settings
*
* @api requests
* @name GetSourceFilters
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::GetSourceFilters(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataArrayAutoRelease filters = Utils::GetSourceFiltersList(source, true);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_array(response, "filters", filters);
	return request.success(response);
}

/**
* List filters applied to a source
*
* @param {String} `sourceName` Source name
* @param {String} `filterName` Source filter name
*
* @return {Boolean} `enabled` Filter status (enabled or not)
* @return {String} `type` Filter type
* @return {String} `name` Filter name
* @return {Object} `settings` Filter settings
*
* @api requests
* @name GetSourceFilterInfo
* @category sources
* @since 4.7.0
*/
RpcResponse WSRequestHandler::GetSourceFilterInfo(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist on specified source");
	}

	OBSDataAutoRelease response = Utils::GetSourceFilterInfo(filter, true);
	return request.success(response);
}

/**
* Add a new filter to a source. Available source types along with their settings properties are available from `GetSourceTypesList`.
*
* @param {String} `sourceName` Name of the source on which the filter is added
* @param {String} `filterName` Name of the new filter
* @param {String} `filterType` Filter type
* @param {Object} `filterSettings` Filter settings
*
* @api requests
* @name AddFilterToSource
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::AddFilterToSource(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName") ||
		!request.hasField("filterType") || !request.hasField("filterSettings"))
	{
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	const char* filterType = obs_data_get_string(request.parameters(), "filterType");
	OBSDataAutoRelease filterSettings = obs_data_get_obj(request.parameters(), "filterSettings");

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease existingFilter = obs_source_get_filter_by_name(source, filterName);
	if (existingFilter) {
		return request.failed("filter name already taken");
	}

	OBSSourceAutoRelease filter = obs_source_create_private(filterType, filterName, filterSettings);
	if (!filter) {
		return request.failed("filter creation failed");
	}
	if (obs_source_get_type(filter) != OBS_SOURCE_TYPE_FILTER) {
		return request.failed("invalid filter type");
	}

	obs_source_filter_add(source, filter);

	return request.success();
}

/**
* Remove a filter from a source
*
* @param {String} `sourceName` Name of the source from which the specified filter is removed
* @param {String} `filterName` Name of the filter to remove
*
* @api requests
* @name RemoveFilterFromSource
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::RemoveFilterFromSource(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	const char* filterName = obs_data_get_string(request.parameters(), "filterName");

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist");
	}

	obs_source_filter_remove(source, filter);

	return request.success();
}

/**
* Move a filter in the chain (absolute index positioning)
*
* @param {String} `sourceName` Name of the source to which the filter belongs
* @param {String} `filterName` Name of the filter to reorder
* @param {Integer} `newIndex` Desired position of the filter in the chain
*
* @api requests
* @name ReorderSourceFilter
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::ReorderSourceFilter(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName") || !request.hasField("newIndex")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	int newIndex = obs_data_get_int(request.parameters(), "newIndex");

	if (newIndex < 0) {
		return request.failed("invalid index");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist");
	}

	struct filterSearch {
		int i;
		int filterIndex;
		obs_source_t* filter;
	};
	struct filterSearch ctx = { 0, 0, filter };
	obs_source_enum_filters(source, [](obs_source_t *parent, obs_source_t *child, void *param)
	{
		struct filterSearch* ctx = (struct filterSearch*)param;
		if (child == ctx->filter) {
			ctx->filterIndex = ctx->i;
		}
		ctx->i++;
	}, &ctx);

	int lastFilterIndex = ctx.i + 1;
	if (newIndex > lastFilterIndex) {
		return request.failed("index out of bounds");
	}

	int currentIndex = ctx.filterIndex;
	if (newIndex > currentIndex) {
		int downSteps = newIndex - currentIndex;
		for (int i = 0; i < downSteps; i++) {
			obs_source_filter_set_order(source, filter, OBS_ORDER_MOVE_DOWN);
		}
	}
	else if (newIndex < currentIndex) {
		int upSteps = currentIndex - newIndex;
		for (int i = 0; i < upSteps; i++) {
			obs_source_filter_set_order(source, filter, OBS_ORDER_MOVE_UP);
		}
	}

	return request.success();
}

/**
* Move a filter in the chain (relative positioning)
*
* @param {String} `sourceName` Name of the source to which the filter belongs
* @param {String} `filterName` Name of the filter to reorder
* @param {String} `movementType` How to move the filter around in the source's filter chain. Either "up", "down", "top" or "bottom".
*
* @api requests
* @name MoveSourceFilter
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::MoveSourceFilter(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName") || !request.hasField("movementType")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	QString movementType(obs_data_get_string(request.parameters(), "movementType"));

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist");
	}

	obs_order_movement movement;
	if (movementType == "up") {
		movement = OBS_ORDER_MOVE_UP;
	}
	else if (movementType == "down") {
		movement = OBS_ORDER_MOVE_DOWN;
	}
	else if (movementType == "top") {
		movement = OBS_ORDER_MOVE_TOP;
	}
	else if (movementType == "bottom") {
		movement = OBS_ORDER_MOVE_BOTTOM;
	}
	else {
		return request.failed("invalid value for movementType: must be either 'up', 'down', 'top' or 'bottom'.");
	}

	obs_source_filter_set_order(source, filter, movement);

	return request.success();
}

/**
* Update settings of a filter
*
* @param {String} `sourceName` Name of the source to which the filter belongs
* @param {String} `filterName` Name of the filter to reconfigure
* @param {Object} `filterSettings` New settings. These will be merged to the current filter settings.
*
* @api requests
* @name SetSourceFilterSettings
* @category sources
* @since 4.5.0
*/
RpcResponse WSRequestHandler::SetSourceFilterSettings(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName") || !request.hasField("filterSettings")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	OBSDataAutoRelease newFilterSettings = obs_data_get_obj(request.parameters(), "filterSettings");

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist");
	}

	OBSDataAutoRelease settings = obs_source_get_settings(filter);
	obs_data_apply(settings, newFilterSettings);
	obs_source_update(filter, settings);

	return request.success();
}

/**
* Change the visibility/enabled state of a filter
*
* @param {String} `sourceName` Source name
* @param {String} `filterName` Source filter name
* @param {Boolean} `filterEnabled` New filter state
*
* @api requests
* @name SetSourceFilterVisibility
* @category sources
* @since 4.7.0
*/
RpcResponse WSRequestHandler::SetSourceFilterVisibility(const RpcRequest& request)
{
	if (!request.hasField("sourceName") || !request.hasField("filterName") || !request.hasField("filterEnabled")) {
		return request.failed("missing request parameters");
	}

	const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName);
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	const char* filterName = obs_data_get_string(request.parameters(), "filterName");
	OBSSourceAutoRelease filter = obs_source_get_filter_by_name(source, filterName);
	if (!filter) {
		return request.failed("specified filter doesn't exist on specified source");
	}

	bool filterEnabled = obs_data_get_bool(request.parameters(), "filterEnabled");
	obs_source_set_enabled(filter, filterEnabled);

	return request.success();
}

/**
* Get the audio monitoring type of the specified source.
*
* @param {String} `sourceName` Source name.
*
* @return {String} `monitorType` The monitor type in use. Options: `none`, `monitorOnly`, `monitorAndOutput`.
*
* @api requests
* @name GetAudioMonitorType
* @category sources
* @since 4.8.0
*/
RpcResponse WSRequestHandler::GetAudioMonitorType(const RpcRequest& request)
 {
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");

	if (sourceName.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	OBSDataAutoRelease response = obs_data_create();

	QString monitorType;
	enum obs_monitoring_type mtype = obs_source_get_monitoring_type(source);
	switch (mtype) {
	case OBS_MONITORING_TYPE_NONE:
		monitorType = "none";
		break;
	case OBS_MONITORING_TYPE_MONITOR_ONLY:
		monitorType = "monitorOnly";
		break;
	case OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT:
		monitorType = "monitorAndOutput";
		break;
	default:
		monitorType = "unknown";
		break;
	}
	obs_data_set_string(response, "monitorType", monitorType.toUtf8());

	return request.success(response);
}

/**
* Set the audio monitoring type of the specified source.
*
* @param {String} `sourceName` Source name.
* @param {String} `monitorType` The monitor type to use. Options: `none`, `monitorOnly`, `monitorAndOutput`.
*
* @api requests
* @name SetAudioMonitorType
* @category sources
* @since 4.8.0
*/
RpcResponse WSRequestHandler::SetAudioMonitorType(const RpcRequest& request)
 {
	if (!request.hasField("sourceName") || !request.hasField("monitorType")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");
	QString monitorType = obs_data_get_string(request.parameters(), "monitorType");

	if (sourceName.isEmpty() || monitorType.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	if (monitorType == "none") {
		obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_NONE);
	} else if (monitorType == "monitorOnly") {
		obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_ONLY);
	} else if (monitorType == "monitorAndOutput") {
		obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT);
	} else {
		return request.failed("invalid monitorType");
	}
	return request.success();
}

/**
* Get the default settings for a given source type.
*
* @param {String} `sourceKind` Source kind. Also called "source id" in libobs terminology.
*
* @return {String} `sourceKind` Source kind. Same value as the `sourceKind` parameter.
* @return {Object} `defaultSettings` Settings object for source.
*
* @api requests
* @name GetSourceDefaultSettings
* @category sources
* @since 4.9.0
*/
RpcResponse WSRequestHandler::GetSourceDefaultSettings(const RpcRequest& request)
{
	if (!request.hasField("sourceKind")) {
		return request.failed("missing request parameters");
	}

	QString sourceKind = obs_data_get_string(request.parameters(), "sourceKind");
	if (sourceKind.isEmpty()) {
		return request.failed("invalid request parameters");
	}

	OBSDataAutoRelease defaultData = obs_get_source_defaults(sourceKind.toUtf8());
	if (!defaultData) {
		return request.failed("invalid sourceKind");
	}

	OBSDataAutoRelease defaultSettings = Utils::OBSDataGetDefaults(defaultData);

	OBSDataAutoRelease response = obs_data_create();
	obs_data_set_string(response, "sourceKind", sourceKind.toUtf8().constData());
	obs_data_set_obj(response, "defaultSettings", defaultSettings);
	return request.success(response);
}

/**
* Takes a picture snapshot of a source and then can either or both:
*	- Send it over as a Data URI (base64-encoded data) in the response (by specifying `embedPictureFormat` in the request)
*	- Save it to disk (by specifying `saveToFilePath` in the request)
*
* At least `embedPictureFormat` or `saveToFilePath` must be specified.
*
* Clients can specify `width` and `height` parameters to receive scaled pictures. Aspect ratio is
* preserved if only one of these two parameters is specified.
*
* @param {String (optional)} `sourceName` Source name. Note: Since scenes are also sources, you can also provide a scene name. If not provided, the currently active scene is used.
* @param {String (optional)} `embedPictureFormat` Format of the Data URI encoded picture. Can be "png", "jpg", "jpeg" or "bmp" (or any other value supported by Qt's Image module)
* @param {String (optional)} `saveToFilePath` Full file path (file extension included) where the captured image is to be saved. Can be in a format different from `pictureFormat`. Can be a relative path.
* @param {String (optional)} `fileFormat` Format to save the image file as (one of the values provided in the `supported-image-export-formats` response field of `GetVersion`). If not specified, tries to guess based on file extension.
* @param {int (optional)} `compressionQuality` Compression ratio between -1 and 100 to write the image with. -1 is automatic, 1 is smallest file/most compression, 100 is largest file/least compression. Varies with image type.
* @param {int (optional)} `width` Screenshot width. Defaults to the source's base width.
* @param {int (optional)} `height` Screenshot height. Defaults to the source's base height.
*
* @return {String} `sourceName` Source name
* @return {String} `img` Image Data URI (if `embedPictureFormat` was specified in the request)
* @return {String} `imageFile` Absolute path to the saved image file (if `saveToFilePath` was specified in the request)
*
* @api requests
* @name TakeSourceScreenshot
* @category sources
* @since 4.6.0
*/
RpcResponse WSRequestHandler::TakeSourceScreenshot(const RpcRequest& request) {
	if (!request.hasField("embedPictureFormat") && !request.hasField("saveToFilePath")) {
		return request.failed("At least 'embedPictureFormat' or 'saveToFilePath' must be specified");
	}

	OBSSourceAutoRelease source;
	if (!request.hasField("sourceName")) {
		source = obs_frontend_get_current_scene();
	} else {
		const char* sourceName = obs_data_get_string(request.parameters(), "sourceName");
	 	source = obs_get_source_by_name(sourceName);
		if (!source) {
			return request.failed("specified source doesn't exist");;
		}
	}

	const uint32_t sourceWidth = obs_source_get_base_width(source);
	const uint32_t sourceHeight = obs_source_get_base_height(source);
	const double sourceAspectRatio = ((double)sourceWidth / (double)sourceHeight);

	uint32_t imgWidth = sourceWidth;
	uint32_t imgHeight = sourceHeight;

	if (request.hasField("width")) {
		imgWidth = obs_data_get_int(request.parameters(), "width");

		if (!request.hasField("height")) {
			imgHeight = ((double)imgWidth / sourceAspectRatio);
		}
	}

	if (request.hasField("height")) {
		imgHeight = obs_data_get_int(request.parameters(), "height");

		if (!request.hasField("width")) {
			imgWidth = ((double)imgHeight * sourceAspectRatio);
		}
	}

	QImage sourceImage(imgWidth, imgHeight, QImage::Format::Format_RGBA8888);
	sourceImage.fill(0);

	uint8_t* videoData = nullptr;
	uint32_t videoLinesize = 0;

	obs_enter_graphics();

	gs_texrender_t* texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	gs_stagesurf_t* stagesurface = gs_stagesurface_create(imgWidth, imgHeight, GS_RGBA);

	bool renderSuccess = false;
	gs_texrender_reset(texrender);
	if (gs_texrender_begin(texrender, imgWidth, imgHeight)) {
		vec4 background;
		vec4_zero(&background);

		gs_clear(GS_CLEAR_COLOR, &background, 0.0f, 0);
		gs_ortho(0.0f, (float)sourceWidth, 0.0f, (float)sourceHeight, -100.0f, 100.0f);

		gs_blend_state_push();
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

		obs_source_inc_showing(source);
		obs_source_video_render(source);
		obs_source_dec_showing(source);

		gs_blend_state_pop();
		gs_texrender_end(texrender);

		gs_stage_texture(stagesurface, gs_texrender_get_texture(texrender));
		if (gs_stagesurface_map(stagesurface, &videoData, &videoLinesize)) {
			int linesize = sourceImage.bytesPerLine();
			for (uint y = 0; y < imgHeight; y++) {
			 	memcpy(sourceImage.scanLine(y), videoData + (y * videoLinesize), linesize);
			}
			gs_stagesurface_unmap(stagesurface);
			renderSuccess = true;
		}
	}

	gs_stagesurface_destroy(stagesurface);
	gs_texrender_destroy(texrender);

	obs_leave_graphics();

	if (!renderSuccess) {
		return request.failed("Source render failed");
	}

	OBSDataAutoRelease response = obs_data_create();

	int compressionQuality {-1};
	if (request.hasField("compressionQuality")) {
		compressionQuality = obs_data_get_int(request.parameters(), "compressionQuality");

		if (compressionQuality < -1 || compressionQuality > 100) {
			QString errorMessage = QString("compression quality out of range: %1").arg(compressionQuality);
			return request.failed(errorMessage.toUtf8());
		}
	}

	if (request.hasField("embedPictureFormat")) {
		const char* pictureFormat = obs_data_get_string(request.parameters(), "embedPictureFormat");

		QByteArrayList supportedFormats = QImageWriter::supportedImageFormats();
		if (!supportedFormats.contains(pictureFormat)) {
			QString errorMessage = QString("unsupported picture format: %1").arg(pictureFormat);
			return request.failed(errorMessage.toUtf8());
		}

		QByteArray encodedImgBytes;
		QBuffer buffer(&encodedImgBytes);
		buffer.open(QBuffer::WriteOnly);
		if (!sourceImage.save(&buffer, pictureFormat, compressionQuality)) {
			return request.failed("embed image encoding failed");
		}
		buffer.close();

		QString imgBase64(encodedImgBytes.toBase64());
		imgBase64.prepend(
			QString("data:image/%1;base64,").arg(pictureFormat)
		);

		obs_data_set_string(response, "img", imgBase64.toUtf8());
	}

	if (request.hasField("saveToFilePath")) {
		QString filePathStr = obs_data_get_string(request.parameters(), "saveToFilePath");
		QFileInfo filePathInfo(filePathStr);
		QString absoluteFilePath = filePathInfo.absoluteFilePath();

		const char* fileFormat = nullptr;
		if (request.hasField("fileFormat")) {
			fileFormat = obs_data_get_string(request.parameters(), "fileFormat");
			QByteArrayList supportedFormats = QImageWriter::supportedImageFormats();

			if (!supportedFormats.contains(fileFormat)) {
				QString errorMessage = QString("unsupported file format: %1").arg(fileFormat);
				return request.failed(errorMessage.toUtf8());
			}
		}

		if (!sourceImage.save(absoluteFilePath, fileFormat, compressionQuality)) {
			return request.failed("Image save failed");
		}
		obs_data_set_string(response, "imageFile", absoluteFilePath.toUtf8());
	}

	obs_data_set_string(response, "sourceName", obs_source_get_name(source));
	return request.success(response);
}

/**
* Refreshes the specified browser source.
*
* @param {String} `sourceName` Source name.
*
* @api requests
* @name RefreshBrowserSource
* @category sources
* @since 4.9.0
*/
RpcResponse WSRequestHandler::RefreshBrowserSource(const RpcRequest& request)
{
	if (!request.hasField("sourceName")) {
		return request.failed("missing request parameters");
	}

	QString sourceName = obs_data_get_string(request.parameters(), "sourceName");

	OBSSourceAutoRelease source = obs_get_source_by_name(sourceName.toUtf8());
	if (!source) {
		return request.failed("specified source doesn't exist");
	}

	if (strcmp(obs_source_get_id(source), "browser_source") != 0) {
		return request.failed("specified source is not a browser");
	}

	obs_properties_t *sourceProperties = obs_source_properties(source);
	obs_property_t *property = obs_properties_get(sourceProperties, "refreshnocache");
	obs_property_button_clicked(property, source); // This returns a boolean but we ignore it because the browser plugin always returns `false`.
	obs_properties_destroy(sourceProperties);

	return request.success();
}
