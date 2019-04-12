#pragma once
#include "curl/curl.h"
#include "task.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <uv.h>
#include <stdio.h>
#include <node_api.h>

#define DECLARE_NAPI_METHOD(name, func)                          \
  { name, 0, func, 0, 0, 0, napi_default, 0 }


static napi_value intValue(napi_env env, int num);
static napi_value stringValue(napi_env env, QString str);
static napi_value boolenValue(napi_env env, bool b);


class Download : public QObject
{
    Q_OBJECT
public:
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);

    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);
    static Download *Instace();
private:
    Download();
    ~Download();
private:
    static napi_value addTask(napi_env env, napi_callback_info info);
    static napi_value start(napi_env env, napi_callback_info info);
    static napi_value pause(napi_env env, napi_callback_info info);
    static napi_value resume(napi_env env, napi_callback_info info);
    static napi_value cancel(napi_env env, napi_callback_info info);
    static napi_value remove(napi_env env, napi_callback_info info);
    static napi_value errorCode(napi_env env, napi_callback_info info);

    void progressCallback(int nIndex, double nTotal, double nNow);

    void onDownloadStart(int nIndex, int nSize);
    void onDownloadEnd(int nIndex, int errorCode);
private:
    static Download *m_pInstance;
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};