#pragma once
#include "curl/curl.h"
#include "task.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <uv.h>
#include <stdio.h>
#include <node.h>
#include <node_object_wrap.h>

using namespace v8;
using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::HandleScope;


class Download : public QObject, public node::ObjectWrap
{
    Q_OBJECT
public:
    static void Init(v8::Local<v8::Object> exports);
    static void New(const FunctionCallbackInfo<Value>& args);
    static Download *Instace();
private:
    Download();
    ~Download();
private:
    static void addTask(const FunctionCallbackInfo<Value>& args);
    static void start(const FunctionCallbackInfo<Value>& args);
    static void pause(const FunctionCallbackInfo<Value>& args);
    static void resume(const FunctionCallbackInfo<Value>& args);
    static void cancel(const FunctionCallbackInfo<Value>& args);
    static void remove(const FunctionCallbackInfo<Value>& args);
    static void errorCode(const FunctionCallbackInfo<Value>& args);

    void progressCallback(int nIndex, double nTotal, double nNow);

    void onDownloadStart(int nIndex, int nSize);
    void onDownloadEnd(int nIndex, int errorCode);
private:
    static v8::Persistent<v8::Function> m_pConstructor;
    static Download *m_pInstance;

    uv_work_t m_pWork;
    Isolate * isolate;
    Persistent<Function> js_callback;
};