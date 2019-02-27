#include "download.h"
#include <QDebug>
#include <QThread>

using namespace std;
v8::Persistent<v8::Function> Download::m_pConstructor;
Download *Download::m_pInstance = NULL;
static QMap<int, DownloadTask*> m_mapTask;

void Download::Init(v8::Local<v8::Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Download"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(tpl, "addTask", addTask);
    NODE_SET_PROTOTYPE_METHOD(tpl, "start", start);
    NODE_SET_PROTOTYPE_METHOD(tpl, "pause", pause);
    NODE_SET_PROTOTYPE_METHOD(tpl, "resume", resume);
    NODE_SET_PROTOTYPE_METHOD(tpl, "cancle", cancel);

    m_pConstructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Download"),
        tpl->GetFunction());
}


void Download::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    Download *obj = Instace();
    //obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
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

void Download::addTask(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsString() &&
        !args[1]->IsString())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    QString qstr(*(String::Utf8Value)args[0]->ToString());
    QString qstr1(*(String::Utf8Value)args[1]->ToString());
    DownloadTask *task = new DownloadTask(qstr, qstr1);
    if (m_mapTask.size() == 0)
    {
        m_mapTask.insert(0, task);
        task->setIndex(0);
        args.GetReturnValue().Set(0);
        return;
    }
    for (int i = 0; m_mapTask.size(); i++)
    {
        if (m_mapTask.value(i) == NULL)
        {
            m_mapTask.insert(i, task);
            args.GetReturnValue().Set(i);
            task->setIndex(i);
            return;
        }
    }
    isolate->ThrowException(v8::Exception::TypeError(
        String::NewFromUtf8(isolate, "add task faild")));
    args.GetReturnValue().Set(-1);
}

void Download::start(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    if (task != NULL)
    {
        HandleScope scope(isolate);
        task->m_request.data = task;
        task->m_pIsolate = isolate;
        task->m_funCallback.Reset(task->m_pIsolate, Local<Function>::Cast(args[1]));
        QThread *t = new QThread();
        task->moveToThread(t);
        connect(t, &QThread::started, task, &DownloadTask::start);
        connect(task, &DownloadTask::sendProgress, m_pInstance, &Download::progressCallback, Qt::QueuedConnection);
        connect(task, &DownloadTask::downloadEnd, m_pInstance, &Download::onDownloadEnd, Qt::QueuedConnection);
        connect(task, &DownloadTask::downloadEnd, t, &QThread::deleteLater);
        t->start();
        args.GetReturnValue().Set(TRUE);
    }
    else
    {
        args.GetReturnValue().Set(FALSE);
    }
}

void Download::pause(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    bool b = task->pause();
    args.GetReturnValue().Set(b);
}

void Download::resume(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    bool b = task->resume();
    args.GetReturnValue().Set(b);
}

void Download::cancel(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    bool b = task->cancel();

    args.GetReturnValue().Set(b);
}

void Download::remove(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    task->remove();
    m_mapTask.remove(args[0]->IntegerValue());
    task->deleteLater();
    task = NULL;
    args.GetReturnValue().Set(TRUE);
}

void Download::errorCode(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber())
    {
        isolate->ThrowException(v8::Exception::TypeError(
            String::NewFromUtf8(isolate, "Parameter error")));
        return;
    }
    DownloadTask *task = m_mapTask.value(args[0]->IntegerValue());
    args.GetReturnValue().Set(task->error());
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
    DownloadTask *task = m_mapTask.value(nIndex);
    HandleScope handleScope(task->m_pIsolate);
    Local<Value> value[4];
    value[0] = String::NewFromUtf8(task->m_pIsolate, "Progress");
    value[1] = Integer::NewFromUnsigned(task->m_pIsolate, nIndex);
    value[2] = String::NewFromUtf8(task->m_pIsolate, getBytesString(nTotal).toLatin1().constData());
    value[3] = String::NewFromUtf8(task->m_pIsolate, QString::number(nNow).toLatin1().constData());

    Local<Function> js_callback = Local<Function>::New(task->m_pIsolate, task->m_funCallback);
    js_callback->Call(task->m_pIsolate->GetCurrentContext()->Global(), 4, value);
}

void Download::onDownloadStart(int nIndex, int nSize)
{
    DownloadTask *task = m_mapTask.value(nIndex);
    HandleScope scope(task->m_pIsolate);
    Local<Function> js_callback = Local<Function>::New(task->m_pIsolate, task->m_funCallback);

    Local<Value> value[3] = {};

    value[0] = String::NewFromUtf8(task->m_pIsolate, "Start");
    value[1] = Integer::NewFromUnsigned(task->m_pIsolate, nIndex);
    value[2] = String::NewFromUtf8(task->m_pIsolate, getBytesString(nSize).toLatin1().constData());

    js_callback->Call(task->m_pIsolate->GetCurrentContext()->Global(), 3, value);
}

void Download::onDownloadEnd(int nIndex, int errorCode)
{
    DownloadTask *task = m_mapTask.value(nIndex);
    HandleScope scope(task->m_pIsolate);
    Local<Function> js_callback = Local<Function>::New(task->m_pIsolate, task->m_funCallback);

    Local<Value> value[3] = {};

    value[0] = String::NewFromUtf8(task->m_pIsolate, "End");
    value[1] = Integer::NewFromUnsigned(task->m_pIsolate, nIndex);
    value[2] = Integer::NewFromUnsigned(task->m_pIsolate, errorCode);

    js_callback->Call(task->m_pIsolate->GetCurrentContext()->Global(), 3, value);
}
