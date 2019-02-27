#pragma once
#include "curl/curl.h"
#include <QString>
#include <uv.h>
#include <node.h>
#include <QObject>
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

class DownloadTask :public QObject
{
    Q_OBJECT
public:
    DownloadTask(QString url, QString file);
    ~DownloadTask();
    int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    long getDownloadFileLenth(const char *url);
    void start();
    bool pause();
    bool resume();
    bool cancel();
    bool checkHasFile();
    double fileSize();
    void setContinue(bool bContinue);

    int error();
    int writeFile(void* pBuffer, size_t nSize, size_t nMemByte);

    void setIndex(int nIndex);
    void remove();

    bool isCancel();
signals:
    void sendProgress(int nIndex, double nTotal, double nNow);
    void downloadEnd(int nIndex, int errorCode);
    void started(int nIndex, double nSize);
public:
    uv_work_t m_request;
    Isolate * m_pIsolate;
    Persistent<Function> m_funCallback;
private:
    QString m_strUrl;
    QString m_strFile;
    QString m_strTemp;

    LONGLONG m_dTotleFileSzie;
    LONGLONG m_dDownloadFileSize;
    int m_nErrorCode;
    bool m_bContinue;//¶ÏµãÐø´«
    bool m_bCancel;
    CURL* m_pCurl;

    int m_nIndex;
};
