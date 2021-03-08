#pragma once

#include <obs-data.h>
#include <QtCore/QString>
#include "../obs-websocket.h"

namespace RequestStatus {
	typedef uint16_t ResponseCode;

	static ResponseCode const Unknown = 0;

	static ResponseCode const Success = 100;

	// The request is denied because the client is not authenticated
	static ResponseCode const AuthenticationMissing = 200;
	// Connection has already been authenticated (for modules utilizing a request to provide authentication)
	static ResponseCode const AlreadyAuthenticated = 201;
	// Authentication request was denied (for modules utilizing a request to provide authentication)
	static ResponseCode const AuthenticationDenied = 202;
	// The request type is invalid (does not exist)
	static ResponseCode const InvalidRequestType = 203;
	// Generic error code (comment is expected to be provided)
	static ResponseCode const GenericError = 204;

	// A required request parameter is missing
	static ResponseCode const MissingRequestParameter = 300;

	// Generic invalid request parameter message
	static ResponseCode const InvalidRequestParameter = 400;
	// A request parameter has the wrong data type
	static ResponseCode const InvalidRequestParameterDataType = 401;
	// A request parameter (float or int) is out of valid range
	static ResponseCode const RequestParameterOutOfRange = 402;
	// A request parameter (string or array) is empty and cannot be
	static ResponseCode const RequestParameterEmpty = 403;

	// An output is running and cannot be in order to perform the request (generic)
	static ResponseCode const OutputRunning = 500;
	// An output is not running and should be
	static ResponseCode const OutputNotRunning = 501;
	// Stream is running and cannot be
	static ResponseCode const StreamRunning = 502;
	// Stream is not running and should be
	static ResponseCode const StreamNotRunning = 503;
	// Record is running and cannot be
	static ResponseCode const RecordRunning = 504;
	// Record is not running and should be
	static ResponseCode const RecordNotRunning = 505;
	// Record is paused and cannot be
	static ResponseCode const RecordPaused = 506;
	// Replay buffer is running and cannot be
	static ResponseCode const ReplayBufferRunning = 507;
	// Replay buffer is not running and should be
	static ResponseCode const ReplayBufferNotRunning = 508;
	// Replay buffer is disabled and cannot be
	static ResponseCode const ReplayBufferDisabled = 509;
	// Studio mode is active and cannot be
	static ResponseCode const StudioModeActive = 510;
	// Studio mode is not active and should be
	static ResponseCode const StudioModeNotActive = 511;

	// The specified source was of the invalid type (Eg. input instead of scene)
	static ResponseCode const InvalidSourceType = 600;
	// The specified source was not found (generic for input, filter, transition, scene)
	static ResponseCode const SourceNotFound = 601;
	// The specified source already exists. Applicable to inputs, filters, transitions, scenes
	static ResponseCode const SourceAlreadyExists = 602;
	// The specified input was not found
	static ResponseCode const InputNotFound = 603;
	// The specified input had the wrong kind
	static ResponseCode const InvalidInputKind = 604;
	// The specified filter was not found
	static ResponseCode const FilterNotFound = 605;
	// The specified transition was not found
	static ResponseCode const TransitionNotFound = 606;
	// The specified transition does not support setting its position (transition is of fixed type)
	static ResponseCode const TransitionDurationFixed = 607;
	// The specified scene was not found
	static ResponseCode const SceneNotFound = 608;
	// The specified scene item was not found
	static ResponseCode const SceneItemNotFound = 609;
	// The specified scene collection was not found
	static ResponseCode const SceneCollectionNotFound = 610;
	// The specified profile was not found
	static ResponseCode const ProfileNotFound = 611;
	// The specified output was not found
	static ResponseCode const OutputNotFound = 612;
	// The specified encoder was not found
	static ResponseCode const EncoderNotFound = 613;
	// The specified service was not found
	static ResponseCode const ServiceNotFound = 614; 

	// Processing the request failed unexpectedly
	static ResponseCode const RequestProcessingFailed = 700;
	// Starting the Output failed
	static ResponseCode const OutputStartFailed = 701;
	// Duplicating the scene item failed
	static ResponseCode const SceneItemDuplicationFailed = 702;
	// Rendering the screenshot failed
	static ResponseCode const ScreenshotRenderFailed = 703;
	// Encoding the screenshot failed
	static ResponseCode const ScreenshotEncodeFailed = 704;
	// Saving the screenshot failed
	static ResponseCode const ScreenshotSaveFailed = 705;
};

class RpcRequest;

class RpcResponse {
	public:
		;

	private:
		;
};

class RpcRequest {
	public:
		;

	private:
		;
};