#include "nfc_wrapper.hh"
#include "nfc_read_worker.hh"
#include "nfc_auto_initializer.hh"

#include <nfc/nfc.h>

namespace nodenfc
{

using namespace v8;

NFCWrapper::NFCWrapper()
    : Nan::ObjectWrap()
    , handle(NFCAutoInitializer::GetHandle())
{
    /* no-op */
}

void NFCWrapper::stop()
{
    run = false;
    while (claimed);
    if (pnd) {
        nfc_abort_command(pnd);
        nfc_close(pnd);
        pnd = NULL;
    }
}

NAN_METHOD(NFCWrapper::New) {
    Nan::HandleScope scope;
    assert(info.IsConstructCall());
    NFCWrapper* self = new NFCWrapper();
    self->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NFCWrapper::Stop) {
    Nan::HandleScope scope;
    NFCWrapper* nfc = ObjectWrap::Unwrap<NFCWrapper>(info.This());
    nfc->stop();
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NFCWrapper::Start) {
    Nan::HandleScope scope;

    NFCWrapper* nfc = ObjectWrap::Unwrap<NFCWrapper>(info.This());
    if (!nfc->handle->isInitialized())
        return Nan::ThrowError("unable to init libfnc (malloc).");

    auto context = nfc->handle->getContext();

    nfc_device *pnd;
    if (info.Length() > 0) {
        if (!info[0]->IsString()) {
            return Nan::ThrowError("deviceID parameter is not a string");
        }
        nfc_connstring connstring;
        String::Utf8Value device(info[0]->ToString());
        snprintf(connstring, sizeof connstring, "%s", *device);

        pnd = nfc_open(context, connstring);
    }
    else {
        pnd = nfc_open(context, NULL);
    }
    if (pnd == NULL) {
        return Nan::ThrowError("unable open NFC device");
    }

    char result[BUFSIZ];
    if (nfc_initiator_init(pnd) < 0) {
        snprintf(result, sizeof result, "nfc_initiator_init: %s", nfc_strerror(pnd));
        nfc_close(pnd);
        return Nan::ThrowError(result);
    }

    NFCWrapper *baton = ObjectWrap::Unwrap<NFCWrapper>(info.This());
    //baton->context = context; //???
    baton->pnd = pnd;

    NFCReadWorker* readWorker = new NFCReadWorker(baton, info.This());
    Nan::AsyncQueueWorker(readWorker);

    Local<Object> object = Nan::New<Object>();
    object->Set(Nan::New("deviceID").ToLocalChecked(), Nan::New(nfc_device_get_connstring(baton->pnd)).ToLocalChecked());
    object->Set(Nan::New("name").ToLocalChecked(), Nan::New(nfc_device_get_name(baton->pnd)).ToLocalChecked());

    info.GetReturnValue().Set(object);
}

}
