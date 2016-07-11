#pragma once

#include <nan.h>

namespace nodenfc
{

class NFCCard {
public:
    NFCCard();
    ~NFCCard();

    void AddToNodeObject(v8::Local<v8::Object> object);
    void SetDeviceID(const char *deviceID);
    void SetName(const char *name);
    void SetUID(const char *uid);
    void SetType(int32_t type);
    void SetTag(const char *tag);
    void SetError(const char *error);
    void SetOffset(size_t offset);
    void SetData(const uint8_t *data, size_t data_size);

private:
    char        *deviceID;
    char        *name;
    char        *uid;
    int32_t     type;
    char        *tag;
    char        *error;
    size_t      offset;
    size_t      data_size;
    char        *data;
};

}
