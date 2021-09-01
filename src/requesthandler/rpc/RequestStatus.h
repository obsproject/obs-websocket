#pragma once

namespace RequestStatus {
	enum RequestStatus {
		Unknown = 0,

		// For internal use to signify a successful parameter check
		NoError = 10,

		Success = 100,

		// The `requestType` field is missing from the request data
		MissingRequestType = 203,
		// The request type is invalid or does not exist
		UnknownRequestType = 204,
		// Generic error code (comment required)
		GenericError = 205,

		// A required request parameter is missing
		MissingRequestParameter = 300,
		// The request does not have a valid requestData object.
		MissingRequestData = 301,

		// Generic invalid request parameter message (comment required)
		InvalidRequestParameter = 400,
		// A request parameter has the wrong data type
		InvalidRequestParameterType = 401,
		// A request parameter (float or int) is out of valid range
		RequestParameterOutOfRange = 402,
		// A request parameter (string or array) is empty and cannot be
		RequestParameterEmpty = 403,
		// There are too many request parameters (eg. a request takes two optionals, where only one is allowed at a time)
		TooManyRequestParameters = 404,

		// An output is running and cannot be in order to perform the request (generic)
		OutputRunning = 500,
		// An output is not running and should be
		OutputNotRunning = 501,
		// An output is paused and should not be
		OutputPaused = 502,
		// An output is disabled and should not be
		OutputDisabled = 503,
		// Studio mode is active and cannot be
		StudioModeActive = 504,
		// Studio mode is not active and should be
		StudioModeNotActive = 505,

		// The resource was not found
		ResourceNotFound = 600,
		// The resource already exists
		ResourceAlreadyExists = 601,
		// The type of resource found is invalid
		InvalidResourceType = 602,
		// There are not enough instances of the resource in order to perform the request
		NotEnoughResources = 603,
		// The state of the resource is invalid. For example, if the resource is blocked from being accessed
		InvalidResourceState = 604,
		// The specified input (obs_source_t-OBS_SOURCE_TYPE_INPUT) had the wrong kind
		InvalidInputKind = 605,

		// Creating the resource failed
		ResourceCreationFailed = 700,
		// Performing an action on the resource failed
		ResourceActionFailed = 701,
		// Processing the request failed unexpectedly (comment required)
		RequestProcessingFailed = 702,
		// The combination of request parameters cannot be used to perform an action
		CannotAct = 703,
	};
};
