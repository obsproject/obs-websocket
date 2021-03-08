#pragma once

#include <obs-data.h>
#include <QtCore/QString>

#include "../obs-websocket.h"

class RpcEvent {
	public:
		explicit RpcEvent (
			const QString& updateType,
			obs_data_t* additionalFields = nullptr
		);

		const QString& updateType() const
		{
			return _updateType;
		}

		const OBSData additionalFields() const
		{
			return OBSData(_additionalFields);
		}

	private:
		QString _updateType;
		OBSDataAutoRelease _additionalFields;
}; 