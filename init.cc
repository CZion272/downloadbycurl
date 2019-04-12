#pragma once

#include <node_api.h>
#include <QApplication>
#include <QThread>
#include "download.h"

napi_value runExec(napi_env env, napi_callback_info info)
{
    int argc = 0;
    QApplication *app = new QApplication(argc, NULL);
    //app.exec();
    return nullptr;
}

napi_value InitAll(napi_env env, napi_value exports)
{
    Download::Init(env, exports);
    const int nPorperty = 1;
    napi_status status;
    napi_property_descriptor desc[nPorperty];
    desc[0] = DECLARE_NAPI_METHOD("runExec", runExec);
    status = napi_define_properties(env, exports, nPorperty, desc);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, InitAll)
