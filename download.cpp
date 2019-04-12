#include "download.h"
#include <QDebug>
#include <QThread>
#include <QApplication>

using namespace std;

const char *errorMessage[] = {
    "No such file",
    "Parameter error",
    "env check error",
    "Add Task Failed"
};

#define ERRORSTRING(env, code) \
{\
    napi_throw_error(env, QString::number(code).toLatin1().constData(), errorMessage[code]);\
}


static bool checkNapiEnv(napi_env env, napi_callback_info info, size_t argc, napi_value *args, napi_value *jsthis)
{
    napi_value target;
    napi_get_new_target(env, info, &target);
    if (napi_get_new_target(env, info, &target) != napi_ok)
    {
        return false;
    }
    if (napi_get_cb_info(env, info, &argc, args, jsthis, nullptr))
    {
        return false;
    }
    return true;
}

static bool checkValueType(napi_env env, napi_value value, napi_valuetype type)
{
    napi_valuetype valueType;
    if (napi_typeof(env, value, &valueType) != napi_ok)
    {
        return false;
    }
    if (type != valueType)
    {
        return false;
    }
    return true;
}

static napi_value intValue(napi_env env, int num)
{
    napi_value rb;
    napi_create_int32(env, num, &rb);
    return rb;
}

static napi_value stringValue(napi_env env, QString str)
{
    napi_value rb;
    napi_create_string_latin1(env, str.toLatin1(), str.length(), &rb);
    return rb;
}

static napi_value boolenValue(napi_env env, bool b)
{
    napi_value rb;
    napi_get_boolean(env, b, &rb);
    return rb;
}

static bool napiValueToString(napi_env env, napi_value value, QString &string)
{
    if (!checkValueType(env, value, napi_string))
    {
        return false;
    }
    size_t size;
    char *ch;
    if (napi_get_value_string_latin1(env, value, NULL, NAPI_AUTO_LENGTH, &size) != napi_ok)
    {
        return false;
    }
    ch = new char[size + 1];
    if (napi_get_value_string_latin1(env, value, ch, size + 1, &size) != napi_ok)
    {
        delete ch;
        return false;
    }
    string = QString(ch);
    delete ch;
    return true;
}
Download *Download::m_pInstance = NULL;
static QMap<int, DownloadTask*> m_mapTask;
napi_ref Download::constructor;

napi_value Download::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        //{ "value", 0, 0, GetValue, SetValue, 0, napi_default, 0 },
        DECLARE_NAPI_METHOD("addTask", addTask),
        DECLARE_NAPI_METHOD("start", start),
        DECLARE_NAPI_METHOD("pause", pause),
        DECLARE_NAPI_METHOD("resume", resume),
        DECLARE_NAPI_METHOD("cancel", cancel),
        DECLARE_NAPI_METHOD("remove", remove),
        DECLARE_NAPI_METHOD("errorCode", errorCode),
    };

    napi_value cons;
    status = napi_define_class(env, "Download", NAPI_AUTO_LENGTH, New, nullptr, 7, properties, &cons);
    assert(status == napi_ok);

    status = napi_create_reference(env, cons, 1, &constructor);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "Download", cons);
    assert(status == napi_ok);
    return exports;
}

void Download::Destructor(napi_env env, void* nativeObject, void* /*finalize_hint*/)
{
    reinterpret_cast<Download*>(nativeObject)->~Download();
}

napi_value Download::New(napi_env env, napi_callback_info info)
{
    napi_status status;

    size_t argc = 1;
    napi_value args[1];
    napi_value jsthis;

    napi_value target;
    if (napi_get_new_target(env, info, &target) != napi_ok)
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }

    if (napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr) != napi_ok)
    {
        ERRORSTRING(env, 3);
        return false;
    }
    Download *obj;
    if (!m_pInstance)
    {
        obj = Instace();
        obj->env_ = env;
        status = napi_wrap(env,
            jsthis,
            reinterpret_cast<void*>(obj),
            Download::Destructor,
            nullptr,  // finalize_hint
            &obj->wrapper_);
    }
    return jsthis;
}

Download * Download::Instace()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = new Download;
    }
    return m_pInstance;
}
Download::Download()
{
}

Download::~Download()
{
}

napi_value Download::addTask(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 2;
    napi_value args[2];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_string) && !checkValueType(env, args[1], napi_string))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }

    QString url;
    QString path;
    napiValueToString(env, args[0], url);
    napiValueToString(env, args[1], path);

    DownloadTask *task = new DownloadTask(url, path);
    if (m_mapTask.size() == 0)
    {
        m_mapTask.insert(0, task);
        task->setIndex(0);
        return intValue(env, 0);
    }
    for (int i = 0; m_mapTask.size(); i++)
    {
        if (m_mapTask.value(i) == NULL)
        {
            m_mapTask.insert(i, task);
            task->setIndex(i);
            return intValue(env, i);
        }
    }
    ERRORSTRING(env, 4);
    return intValue(env, -1);
}

napi_value Download::start(napi_env env, napi_callback_info info)
{
    napi_status status;

    napi_value jsthis = NULL;
    size_t argc = 2;
    napi_value args[2];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number) && !checkValueType(env, args[1], napi_function))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);

    if (task != NULL)
    {
        napi_value global;
        status = napi_get_global(env, &global);
        task->m_NapiCallback = args[1];
        task->m_NapiGlobal = global;
        task->m_NapiEnv = env;
        napi_ref ref;
        napi_create_reference(env, args[1], 0, &ref);
        task->m_NapiRef = ref;
        QThread *t = new QThread();
        task->moveToThread(t);
        connect(t, &QThread::started, task, &DownloadTask::start);
        connect(t, &QThread::finished, task, &DownloadTask::deleteLater);
        connect(task, &DownloadTask::started, m_pInstance, &Download::onDownloadStart, Qt::QueuedConnection);
        connect(task, &DownloadTask::sendProgress, m_pInstance, &Download::progressCallback, Qt::QueuedConnection);
        connect(task, &DownloadTask::downloadEnd, m_pInstance, &Download::onDownloadEnd, Qt::QueuedConnection);
        connect(task, &DownloadTask::downloadEnd, t, &QThread::quit);

        t->start();
        return boolenValue(env, true);
    }
    else
    {
        return boolenValue(env, false);
    }
}

napi_value Download::pause(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 1;
    napi_value args[1];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);
    return boolenValue(env, task->pause());
}

napi_value Download::resume(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 1;
    napi_value args[1];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);
    return boolenValue(env, task->resume());
}

napi_value Download::cancel(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 1;
    napi_value args[1];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);
    return boolenValue(env, task->cancel());
}

napi_value Download::remove(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 1;
    napi_value args[1];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);
    task->remove();
    m_mapTask.remove(nNum);
    task->deleteLater();
    task = NULL;
    return boolenValue(env, TRUE);
}

napi_value Download::errorCode(napi_env env, napi_callback_info info)
{
    napi_value jsthis = NULL;
    size_t argc = 1;
    napi_value args[1];
    if (!checkNapiEnv(env, info, argc, args, &jsthis))
    {
        ERRORSTRING(env, 3);
        return nullptr;
    }
    if (!checkValueType(env, args[0], napi_number))
    {
        ERRORSTRING(env, 2);
        return nullptr;
    }
    int nNum;
    napi_get_value_int32(env, args[0], &nNum);
    DownloadTask *task = m_mapTask.value(nNum);
    return intValue(env, task->error());
}

QString getBytesString(DWORD64 dw)
{
    long rest = 0;
    if (dw < 1024)
    {
        return QString::number(rest) + "B";
    }
    else
    {
        dw /= 1024;
    }

    if (dw < 1024)
    {
        return QString::number(rest) + "KB";
    }
    else
    {
        rest = dw % 1024;
        dw /= 1024;
    }

    if (dw < 1024)
    {
        dw = dw * 100;
        return QString::number((dw / 100)) + "." + QString::number((rest * 100 / 1024 % 100)) + "MB";
    }
    else
    {
        dw = dw * 100 / 1024;
        return QString::number((dw / 100)) + "." + QString::number((dw % 100)) + "GB";
    }
}

void Download::progressCallback(int nIndex, double nTotal, double nNow)
{
    napi_value result;
    napi_value cb;

    DownloadTask *task = m_mapTask.value(nIndex);
    napi_value value[4];
    value[0] = stringValue(task->m_NapiEnv, "Progress");
    value[1] = intValue(task->m_NapiEnv, nIndex);
    value[2] = stringValue(task->m_NapiEnv, getBytesString(nTotal).toLatin1().constData());
    value[3] = stringValue(task->m_NapiEnv, QString::number(nNow).toLatin1().constData());

    napi_get_reference_value(task->m_NapiEnv, task->m_NapiRef, &cb);
    napi_call_function(task->m_NapiEnv, task->m_NapiGlobal, cb, 4, value, &result);
}

void Download::onDownloadStart(int nIndex, int nSize)
{
    napi_value result;
    napi_value cb;

    DownloadTask *task = m_mapTask.value(nIndex);

    napi_value value[3] = {};

    value[0] = stringValue(task->m_NapiEnv, "Start");
    value[1] = intValue(task->m_NapiEnv, nIndex);
    value[2] = stringValue(task->m_NapiEnv, getBytesString(nSize).toLatin1().constData());

    napi_get_reference_value(task->m_NapiEnv, task->m_NapiRef, &cb);
    napi_call_function(task->m_NapiEnv, task->m_NapiGlobal, cb, 3, value, &result);
}

void Download::onDownloadEnd(int nIndex, int errorCode)
{
    napi_value result;
    napi_value cb;

    DownloadTask *task = m_mapTask.value(nIndex);

    napi_value value[3] = {};

    value[0] = stringValue(task->m_NapiEnv, "End");
    value[1] = intValue(task->m_NapiEnv, nIndex);
    value[2] = intValue(task->m_NapiEnv, errorCode);

    napi_get_reference_value(task->m_NapiEnv, task->m_NapiRef, &cb);
    napi_call_function(task->m_NapiEnv, task->m_NapiGlobal, cb, 3, value, &result);
}
