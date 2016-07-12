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

    static Handle   GetHandle();
    static void     Shutdown();

private:
    NFCAutoInitializer();
};

}
