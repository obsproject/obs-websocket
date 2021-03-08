#include "RpcEvent.h"

RpcEvent::RpcEvent (
	const QString& updateType,
	obs_data_t* additionalFields
) :
	_updateType(updateType),
	_additionalFields(nullptr)
{
	if (additionalFields) {
		_additionalFields = obs_data_create();
		obs_data_apply(_additionalFields, additionalFields);
	}
}