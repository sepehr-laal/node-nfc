#pragma once

#include <nan.h>
#include <nfc/nfc-types.h>

namespace nodenfc
{

class NFCCard;
class NFCWrapper;
class NFCReadWorker : public Nan::AsyncProgressWorker {
public:
    NFCReadWorker(NFCWrapper *baton, v8::Local<v8::Object>self);
    ~NFCReadWorker();

    void HandleOKCallback();
    void HandleErrorCallback();
    void Execute(const AsyncProgressWorker::ExecutionProgress& progress);
    void ReadTag(NFCCard *tag);
    void HandleProgressCallback(const char *_tag, size_t size);

private:
    NFCWrapper *baton;
    NFCCard *tag;
};

}
