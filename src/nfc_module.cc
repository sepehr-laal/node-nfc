#include <stdlib.h>
#ifndef _WIN32
#	include <unistd.h>
#	include <err.h>
#else
#	include <io.h>
#endif
#include <string.h>
#include <nan.h>

#include "nfc/nfc.h"
#include "nfc_wrapper.hh"

using namespace v8;

namespace nodenfc {

NAN_METHOD(Scan)
{
    Nan::HandleScope scope;

    auto handle = NFCAutoInitializer::GetHandle();

    if (!handle->isInitialized())
        return Nan::ThrowError("unable to init libfnc (malloc).");

    auto context = handle->getContext();

    Local<Object> object = Nan::New<Object>();

    const auto MAX_DEVICE_COUNT = 16;
    nfc_connstring connstrings[MAX_DEVICE_COUNT];
    size_t i, n = nfc_list_devices(context, connstrings, MAX_DEVICE_COUNT);
    for (i = 0; i < n; i++) {
        Local<Object> entry = Nan::New<Object>();
        nfc_device *pnd = nfc_open(context, connstrings[i]);
        if (pnd == NULL) continue;

        entry->Set(Nan::New("name").ToLocalChecked(), Nan::New(nfc_device_get_name(pnd)).ToLocalChecked());

        const char *info = nfc_device_get_name(pnd);
        if (info >= 0) {
            entry->Set(Nan::New("info").ToLocalChecked(), Nan::New(info).ToLocalChecked());
        }
        else {
            entry->Set(Nan::New("info").ToLocalChecked(), Nan::New("").ToLocalChecked());
        }
        object->Set(Nan::New(nfc_device_get_connstring(pnd)).ToLocalChecked(), entry);

        nfc_close(pnd);
    }

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Version)
{
    Nan::HandleScope scope;

    auto handle = NFCAutoInitializer::GetHandle();

    if (!handle->isInitialized())
        return Nan::ThrowError("unable to init libnfc (malloc).");

    auto context = handle->getContext();

    Local<Object> object = Nan::New<Object>();
    object->Set(Nan::New("name").ToLocalChecked(), Nan::New("libnfc").ToLocalChecked());
    object->Set(Nan::New("version").ToLocalChecked(), Nan::New(nfc_version()).ToLocalChecked());

    info.GetReturnValue().Set(object);
}

NAN_MODULE_INIT(ModuleInit)
{
    Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(NFCWrapper::New);
    tpl->SetClassName(Nan::New("NFC").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "start", NFCWrapper::Start);
    SetPrototypeMethod(tpl, "stop", NFCWrapper::Stop);

    Nan::Export(target, "version", Version);
    Nan::Export(target, "scan", Scan);

    Nan::Set(target, Nan::New("NFC").ToLocalChecked(), tpl->GetFunction());
};

}

NODE_MODULE(nfc, nodenfc::ModuleInit)
