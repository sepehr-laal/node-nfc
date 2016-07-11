#pragma once

#include "nfc_auto_initializer.hh"

#include <nfc/nfc-types.h>
#include <nan.h>

namespace nodenfc
{

class NFCReadWorker;
class NFCWrapper : public Nan::ObjectWrap {
public:
    NFCWrapper();

    static NAN_METHOD(New);
    static NAN_METHOD(Start);
    static NAN_METHOD(Stop);

    void stop();

    NFCAutoInitializer::Handle  handle;
    nfc_device*                 pnd;
    nfc_target                  nt;
    bool                        run;
    bool                        claimed;
};

}
