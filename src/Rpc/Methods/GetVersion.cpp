#include "RpcMethods.h"

const RpcResponse GetVersion::handle(const RpcRequest &request)
{
    QString obsVersion = Utils::OBSVersionString();

    QVariantHash params;
    params.insert("pluginVersion", OBS_WEBSOCKET_VERSION);
    params.insert("programVersion", obsVersion);

    return RpcResponse::ok(request, params);
}
