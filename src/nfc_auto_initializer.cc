#include "nfc_auto_initializer.hh"
#include "nfc/nfc.h"

namespace nodenfc
{

NFCAutoInitializer::NFCAutoInitializer()
    : mInitialized(false)
    , mContext(nullptr)
{
    nfc_context *ctx = nullptr;
    nfc_init(ctx);

    mInitialized = (ctx != nullptr);
    mContext = ctx;
}

NFCAutoInitializer::~NFCAutoInitializer()
{
    if (!mInitialized)
        return;

    nfc_exit(mContext);
    mContext = nullptr;
    mInitialized = false;
}

bool NFCAutoInitializer::isInitialized()
{
    return mInitialized;
}

nfc_context* NFCAutoInitializer::getContext()
{
    return mContext;
}

NFCAutoInitializer::Handle NFCAutoInitializer::GetHandle()
{
    static Handle handle{ new NFCAutoInitializer };
    return handle;
}

}
