#include "nfc_auto_initializer.hh"
#include "nfc/nfc.h"

namespace nodenfc
{

NFCAutoInitializer::NFCAutoInitializer()
{
    nfc_context *ctx = nullptr;
    nfc_init(nullptr);
}

NFCAutoInitializer::~NFCAutoInitializer()
{
    Shutdown();
}

NFCAutoInitializer::Handle NFCAutoInitializer::GetHandle()
{
    static Handle handle{ new NFCAutoInitializer };
    return handle;
}

void NFCAutoInitializer::Shutdown()
{
    static bool isShutdown = false;

    if (!isShutdown)
    {
        nfc_exit(nullptr);
        isShutdown = true;
    }
}

}
