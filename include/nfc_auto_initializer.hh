#pragma once

#include <memory>
#include <nfc/nfc-types.h>

namespace nodenfc
{

class NFCAutoInitializer
{
public:
    using Handle = std::shared_ptr<NFCAutoInitializer>;
    ~NFCAutoInitializer();

    bool            isInitialized();
    nfc_context*    getContext();
    static Handle   GetHandle();

private:
    NFCAutoInitializer();
    bool            mInitialized;
    nfc_context*    mContext;
};

}
