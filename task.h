#pragma once
#include "curl/curl.h"
#include <QString>
#include <node_api.h>
#include <QObject>

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
    napi_value m_NapiCallback;
    napi_value m_NapiGlobal;
    napi_env m_NapiEnv;
    napi_ref m_NapiRef;
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
