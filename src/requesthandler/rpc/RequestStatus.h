#pragma once

namespace RequestStatus {
	enum RequestStatus {
		/**
		* @api
		* For internal use to signify an unknown parameter check 
		*/
		Unknown = 0,
		/**
		* @api
		* For internal use to signify a no error parameter check 
		*/
		NoError = 10,
		/**
		* @api
		* For internal use to signify a successful parameter check 
		*/
		Success = 100,

		/**
		* @api
		* The `requestType` field is missing from the request data 
		*/
		MissingRequestType = 203,
		/**
		* @api
		* The request type is invalid (does not exist) 
		*/
		UnknownRequestType = 204,
		/**
		* @api
		* Generic error code (comment is expected to be provided) 
		*/
		GenericError = 205,

		/**
		* @api
		* A required request parameter is missing 
		*/
		MissingRequestParameter = 300,
		/**
		* @api
		* The request does not have a valid requestData object. 
		*/
		MissingRequestData = 301,

		/**
		* @api
		* Generic invalid request parameter message 
		*/
		InvalidRequestParameter = 400,
		/**
		* @api
		* A request parameter has the wrong data type 
		*/
		InvalidRequestParameterDataType = 401,
		/**
		* @api
		* A request parameter (float or int) is out of valid range 
		*/
		RequestParameterOutOfRange = 402,
		/**
		* @api
		* A request parameter (string or array) is empty and cannot be 
		*/
		RequestParameterEmpty = 403,
		/**
		* @api
		* There are too many request parameters (eg. a request takes two optionals, where only one is allowed at a time) 
		*/
		TooManyRequestParameters = 404,

		/**
		* @api
		* An output is running and cannot be in order to perform the request (generic) 
		*/
		OutputRunning = 500,
		/**
		* @api
		* An output is not running and should be 
		*/
		OutputNotRunning = 501,
		/**
		* @api
		* Stream is running and cannot be 
		*/
		StreamRunning = 502,
		/**
		* @api
		* Stream is not running and should be 
		*/
		StreamNotRunning = 503,
		/**
		* @api
		* Record is running and cannot be 
		*/
		RecordRunning = 504,
		/**
		* @api
		* Record is not running and should be 
		*/
		RecordNotRunning = 505,
		/**
		* @api
		* Record is paused and cannot be 
		*/
		RecordPaused = 506,
		/**
		* @api
		* Replay buffer is running and cannot be 
		*/
		ReplayBufferRunning = 507,
		/**
		* @api
		* Replay buffer is not running and should be 
		*/
		ReplayBufferNotRunning = 508,
		/**
		* @api
		* Replay buffer is disabled and cannot be 
		*/
		ReplayBufferDisabled = 509,
		/**
		* @api
		* Studio mode is active and cannot be 
		*/
		StudioModeActive = 510,
		/**
		* @api
		* Studio mode is not active and should be 
		*/
		StudioModeNotActive = 511,
		/**
		* @api
		* Virtualcam is running and cannot be 
		*/
		VirtualcamRunning = 512,
		/**
		* @api
		* Virtualcam is not running and should be 
		*/
		VirtualcamNotRunning = 513,

		/**
		* @api
		* The specified source (obs_source_t) was of the invalid type (Eg. input instead of scene) 
		*/
		InvalidSourceType = 600,
		/**
		* @api
		* The specified source (obs_source_t) was not found (generic for input, filter, transition, scene) 
		*/
		SourceNotFound = 601,
		/**
		* @api
		* The specified source (obs_source_t) already exists. Applicable to inputs, filters, transitions, scenes 
		*/
		SourceAlreadyExists = 602,
		/**
		* @api
		* The specified input (obs_source_t-OBS_SOURCE_TYPE_FILTER) was not found 
		*/
		InputNotFound = 603,
		/**
		* @api
		* The specified input (obs_source_t-OBS_SOURCE_TYPE_INPUT) had the wrong kind 
		*/
		InvalidInputKind = 604,
		/**
		* @api
		* The specified filter (obs_source_t-OBS_SOURCE_TYPE_FILTER) was not found 
		*/
		FilterNotFound = 605,
		/**
		* @api
		* The specified transition (obs_source_t-OBS_SOURCE_TYPE_TRANSITION) was not found 
		*/
		TransitionNotFound = 606,
		/**
		* @api
		* The specified transition (obs_source_t-OBS_SOURCE_TYPE_TRANSITION) does not support setting its position (transition is of fixed type) 
		*/
		TransitionDurationFixed = 607,
		/**
		* @api
		* The specified scene (obs_source_t-OBS_SOURCE_TYPE_SCENE), (obs_scene_t) was not found 
		*/
		SceneNotFound = 608,
		/**
		* @api
		* The specified scene item (obs_sceneitem_t) was not found 
		*/
		SceneItemNotFound = 609,
		/**
		* @api
		* The specified scene collection was not found 
		*/
		SceneCollectionNotFound = 610,
		/**
		* @api
		* The specified profile was not found 
		*/
		ProfileNotFound = 611,
		/**
		* @api
		* The specified output (obs_output_t) was not found 
		*/
		OutputNotFound = 612,
		/**
		* @api
		* The specified encoder (obs_encoder_t) was not found 
		*/
		EncoderNotFound = 613,
		/**
		* @api
		* The specified service (obs_service_t) was not found 
		*/
		ServiceNotFound = 614,
		/**
		* @api
		* The specified hotkey was not found 
		*/
		HotkeyNotFound = 615,
		/**
		* @api
		* The specified directory was not found 
		*/
		DirectoryNotFound = 616,
		/**
		* @api
		* The specified config item (config_t) was not found. Could be section or parameter name 
		*/
		ConfigParameterNotFound = 617,
		/**
		* @api
		* The specified property (obs_properties_t) was not found 
		*/
		PropertyNotFound = 618,
		/**
		* @api
		* The specififed key (OBS_KEY_*) was not found 
		*/
		KeyNotFound = 619,

		/**
		* @api
		* Processing the request failed unexpectedly 
		*/
		RequestProcessingFailed = 700,
		/**
		* @api
		* Starting the Output failed 
		*/
		OutputStartFailed = 701,
		/**
		* @api
		* Duplicating the scene item failed 
		*/
		SceneItemDuplicationFailed = 702,
		/**
		* @api
		* Rendering the screenshot failed 
		*/
		ScreenshotRenderFailed = 703,
		/**
		* @api
		* Encoding the screenshot failed 
		*/
		ScreenshotEncodeFailed = 704,
		/**
		* @api
		* Saving the screenshot failed 
		*/
		ScreenshotSaveFailed = 705,
		/**
		* @api
		* Creating the directory failed 
		*/
		DirectoryCreationFailed = 706,
		/**
		* @api
		* The combination of request parameters cannot be used to perform an action 
		*/
		CannotAct = 707,
	};
};
