#pragma once

#include <node.h>
#include <QApplication>
#include <QThread>
#include "uv.h"
#include "download.h"

using v8::Local;
using v8::Object;

void runExec(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    int argc = 0;
    QApplication app(argc, NULL);
    args.GetReturnValue().Set(0);
    app.exec();
}

void InitAll(Local<Object> exports)
{
    Download::Init(exports);
    NODE_SET_METHOD(exports, "runExec", runExec);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
