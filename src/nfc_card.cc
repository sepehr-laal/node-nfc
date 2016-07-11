#include "nfc_card.hh"

namespace nodenfc
{

NFCCard::NFCCard()
{
    deviceID = name = uid = tag = error = NULL;
    type = data_size = offset = 0;
    data = NULL;
}

NFCCard::~NFCCard()
{
    delete deviceID;
    delete name;
    delete uid;
    delete tag;
    delete error;
    if (data) delete data;
}

void NFCCard::AddToNodeObject(v8::Local<v8::Object> object)
{
    if (deviceID) object->Set(Nan::New("deviceID").ToLocalChecked(), Nan::New(deviceID).ToLocalChecked());
    if (name) object->Set(Nan::New("name").ToLocalChecked(), Nan::New(name).ToLocalChecked());
    if (uid) object->Set(Nan::New("uid").ToLocalChecked(), Nan::New(uid).ToLocalChecked());
    if (type) object->Set(Nan::New("type").ToLocalChecked(), Nan::New<v8::Int32>(type));
    if (tag) object->Set(Nan::New("tag").ToLocalChecked(), Nan::New(tag).ToLocalChecked());
    if (error) object->Set(Nan::New("error").ToLocalChecked(), Nan::Error(error));
    if (data) object->Set(Nan::New("data").ToLocalChecked(), Nan::NewBuffer(data, data_size).ToLocalChecked());
    if (offset) object->Set(Nan::New("offset").ToLocalChecked(), Nan::New<v8::Int32>((int32_t)offset));
    data = NULL; //ownership transferred to nodejs
}

void NFCCard::SetDeviceID(const char *deviceID)
{
    if (this->deviceID) delete this->deviceID;
    this->deviceID = strdup(deviceID);
}

void NFCCard::SetName(const char *name)
{
    if (this->name) delete this->name;
    this->name = strdup(name);
}

void NFCCard::SetUID(const char *uid)
{
    if (this->uid) delete this->uid;
    this->uid = strdup(uid);
}

void NFCCard::SetType(int32_t type)
{
    this->type = type;
}

void NFCCard::SetTag(const char *tag)
{
    if (this->tag) delete this->tag;
    this->tag = strdup(tag);
}

void NFCCard::SetError(const char *error)
{
    if (this->error) delete this->error;
    this->error = strdup(error);
}

void NFCCard::SetOffset(size_t offset)
{
    this->offset = offset;
}

void NFCCard::SetData(const uint8_t *data, size_t data_size)
{
    if (this->data) free(this->data);
    this->data_size = data_size;
    this->data = (char*)malloc(data_size);
    memcpy(this->data, data, data_size);
}

}
