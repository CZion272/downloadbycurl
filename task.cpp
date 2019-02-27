#include "task.h"
#include <QFile>
#include <QDir>

DownloadTask::DownloadTask(QString url, QString file) :
    m_strUrl(url),
    m_strFile(file),
    m_bContinue(false),
    m_pCurl(NULL),
    m_nErrorCode(0),
    m_dDownloadFileSize(0),
    m_dTotleFileSzie(0),
    m_bCancel(false)
{
    m_strTemp = file + ".tmp";
}

DownloadTask::~DownloadTask()
{
    if (m_pCurl)
    {
        curl_free(m_pCurl);
        m_pCurl = NULL;
    }
}


size_t getHeader(void* ptr, size_t size, size_t nmemb, void* data)
{
    return (size_t)(size * nmemb);
}

size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam)
{
    DownloadTask* dw = (DownloadTask*)pParam;
    int n = dw->writeFile(pBuffer, nSize, nMemByte);
    if (dw->isCancel())
    {
        return 0;
    }
    return n;
}

int DownloadTask::ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if (dltotal > -0.1 && dltotal < 0.1)
        return 0;
    m_dTotleFileSzie;
    emit sendProgress(m_nIndex, dltotal, dlnow);
    return dlnow;
}

long DownloadTask::getDownloadFileLenth(const char *url)
{
    CURL *handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只需要header头
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需要body
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 20);
    if (curl_easy_perform(handle) == CURLE_OK)
    {
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &m_dDownloadFileSize);
    }
    return m_dDownloadFileSize;
}

void DownloadTask::start()
{
    if (!checkHasFile())
    {
        QFile file(m_strFile);
        if (m_dTotleFileSzie != file.size())
        {
            QFile(m_strFile).remove();
        }
        else
        {
            emit downloadEnd(m_nIndex, -1);
            return;
        }
    }
    else
    {
        getDownloadFileLenth(m_strUrl.toLatin1().constData());
        m_dDownloadFileSize = QFile(m_strFile).size();
    }

    emit started(m_nIndex, m_dTotleFileSzie);

    m_pCurl = curl_easy_init();
    curl_easy_setopt(m_pCurl, CURLOPT_URL, m_strUrl.toLatin1().constData());
    //设置接收数据的回调
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, &DownloadCallback);
    // 设置重定向的最大次数
    curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 5);
    // 设置301、302跳转跟随location
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 0);
    //设置进度回调函数
    curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(m_pCurl, CURLOPT_PROGRESSFUNCTION, &DownloadTask::ProgressCallback);
    // 超时
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 20);
    // 获取文件大小
    curl_easy_setopt(m_pCurl, CURLOPT_HEADERFUNCTION, getHeader);
    curl_easy_setopt(m_pCurl, CURLOPT_HEADERDATA, &m_dTotleFileSzie);

    curl_easy_setopt(m_pCurl, CURLOPT_RESUME_FROM_LARGE, m_dDownloadFileSize);

    m_nErrorCode = curl_easy_perform(m_pCurl);
    if (m_nErrorCode == CURLE_OK)
    {
        QFile(m_strTemp).remove();
    }
    //清理curl，和前面的初始化匹配
    curl_easy_cleanup(m_pCurl);
    m_pCurl = NULL;
    emit downloadEnd(m_nIndex, m_nErrorCode);
}

bool DownloadTask::pause()
{
    if (m_pCurl)
    {
        m_nErrorCode = curl_easy_pause(m_pCurl, CURLPAUSE_ALL);
        return m_nErrorCode == CURLE_OK;
    }
    return false;
}

bool DownloadTask::resume()
{
    if (m_pCurl)
    {
        m_nErrorCode = curl_easy_pause(m_pCurl, CURLPAUSE_CONT);
        return  m_nErrorCode == CURLE_OK;
    }
    return false;
}

bool DownloadTask::cancel()
{
    if (m_pCurl)
    {
        m_bCancel = true;
        return true;
    }
    return false;
}

bool DownloadTask::checkHasFile()
{
    return QFile(m_strFile).exists() && QFile(m_strTemp).exists();
}

double DownloadTask::fileSize()
{
    return m_dTotleFileSzie;
}

void DownloadTask::setContinue(bool bContinue)
{
    m_bContinue = bContinue;
}

int DownloadTask::error()
{
    return m_nErrorCode;
}

int DownloadTask::writeFile(void* pBuffer, size_t nSize, size_t nMemByte)
{
    FILE *fp = NULL;
    fp = fopen(m_strFile.toLatin1().constData(), "ab+");
    int n = fwrite(pBuffer, nSize, nMemByte, fp);
    fclose(fp);
    if (!QFile(m_strTemp).exists())
    {
        QFile f(m_strTemp);
        f.open(QIODevice::ReadWrite);
        f.write(QString::number(m_dTotleFileSzie).toLatin1());
        f.write("\nDon't delete this file.\t\t\t");
        f.close();
    }
    return n;
}

void DownloadTask::setIndex(int nIndex)
{
    m_nIndex = nIndex;
}

void DownloadTask::remove()
{
    QFile(m_strFile).remove();
    QFile(m_strTemp).remove();
}

bool DownloadTask::isCancel()
{
    return m_bCancel;
}
