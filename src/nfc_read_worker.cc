#include "nfc_read_worker.hh"
#include "nfc_wrapper.hh"
#include "nfc_card.hh"

#include <nfc/nfc.h>
#include <mifare.h>

#include <thread>
#include <chrono>

namespace
{
static const nfc_modulation nmMifare = {
    NMT_ISO14443A,
    NBR_106,
};
static uint8_t keys[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5,
    0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd,
    0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xab, 0xcd, 0xef, 0x12, 0x34, 0x56
};
static size_t num_keys = sizeof(keys) / 6;

static void zero_mem(void* memory, size_t len) {
    ::memset(memory, '\0', len);
}

}

namespace nodenfc
{

using namespace v8;

NFCReadWorker::NFCReadWorker(NFCWrapper *baton, Local<Object>self)
    : Nan::AsyncProgressWorker(new Nan::Callback(self.As<Function>()))
    , baton(baton)
{
    SaveToPersistent("self", self);
    baton->run = true;
}

NFCReadWorker::~NFCReadWorker()
{
    delete callback; //For some reason HandleProgressCallback only fires while callback exists.
}

void NFCReadWorker::HandleOKCallback()
{
    Local<Value> argv = Nan::New("stopped").ToLocalChecked();

    Local<Object> self = GetFromPersistent("self").As<Object>();
    Nan::MakeCallback(self, "emit", 1, &argv);
}

void NFCReadWorker::HandleErrorCallback()
{
    Local<Value> argv[1];
    argv[0] = Nan::New("error").ToLocalChecked();
    argv[1] = Nan::Error(AsyncProgressWorker::ErrorMessage());

    Local<Object> self = GetFromPersistent("self").As<Object>();
    Nan::MakeCallback(self, "emit", 2, argv);
    HandleOKCallback();
}

void NFCReadWorker::Execute(const AsyncProgressWorker::ExecutionProgress& progress)
{
    while (baton->run && nfc_initiator_select_passive_target(baton->pnd, nmMifare, NULL, 0, &baton->nt) > 0) {
        baton->claimed = true;
        tag = new NFCCard();
        if (baton->run) ReadTag(tag);
        baton->claimed = false;

        progress.Send(NULL, 0);

        int timeout = 5 * 1000; //5 second timeout
        while (baton->run && tag && --timeout > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
        if (timeout <= 0) {
            //unresponsive VM, node was likely killed while this devide was not stopped.
            baton->stop();
            baton->run = false;
            fprintf(stderr, "Node was stopped while some NFC devices where still started.\n");
        }
    }
}

void NFCReadWorker::ReadTag(NFCCard *tag)
{
    unsigned long cc, n;
    char *bp, result[BUFSIZ];
    const char *sp;

    tag->SetDeviceID(nfc_device_get_connstring(baton->pnd));
    tag->SetName(nfc_device_get_name(baton->pnd));

    cc = baton->nt.nti.nai.szUidLen;
    if (cc > sizeof baton->nt.nti.nai.abtUid) cc = sizeof baton->nt.nti.nai.abtUid;
    char uid[3 * sizeof baton->nt.nti.nai.abtUid]{ 0 };
    zero_mem(uid, sizeof uid);

    for (n = 0, bp = uid, sp = ""; n < cc; n++, bp += strlen(bp), sp = ":") {
        snprintf(bp, sizeof uid - (bp - uid), "%s%02x", sp, baton->nt.nti.nai.abtUid[n]);
    }
    tag->SetUID(uid);
    tag->SetType(baton->nt.nti.nai.abtAtqa[1]);

    switch (baton->nt.nti.nai.abtAtqa[1]) {
    case 0x04:
    {
        tag->SetTag("mifare-classic");

        // size guessing logic from nfc-mfclassic.c
        uint8_t uiBlocks = ((baton->nt.nti.nai.abtAtqa[1] & 0x02) == 0x02) ? 0xff    //  4Kb
            : ((baton->nt.nti.nai.btSak & 0x01) == 0x01) ? 0x13    // 320b
            : 0x3f;   //  1Kb/2Kb
        if (nfc_device_set_property_bool(baton->pnd, NP_EASY_FRAMING, false) < 0) {
            snprintf(result, sizeof result, "nfc_device_set_property_bool easyFraming=false: %s",
                nfc_strerror(baton->pnd));
            tag->SetError(result);
            break;
        }

        const auto MAX_FRAME_LENGTH = 264;
        uint8_t abtRats[2] = { 0xe0, 0x50 };
        uint8_t abtRx[MAX_FRAME_LENGTH];
        auto rapdulen = sizeof abtRx;
        int res = nfc_initiator_transceive_bytes(baton->pnd, abtRats, sizeof abtRats, abtRx, &rapdulen, 0);
        if (res > 0) {
            int flip;

            for (flip = 0; flip < 2; flip++) {
                if (nfc_device_set_property_bool(baton->pnd, NP_ACTIVATE_FIELD, flip > 0) < 0) {
                    snprintf(result, sizeof result, "nfc_device_set_property_bool activateField=%s: %s",
                        flip > 0 ? "true" : "false", nfc_strerror(baton->pnd));
                    tag->SetError(result);
                    break;
                }
            }
            if (flip != 2) break;

            if ((res >= 10)
                && (abtRx[5] == 0xc1)
                && (abtRx[6] == 0x05)
                && (abtRx[7] == 0x2f)
                && (abtRx[8] == 0x2f)
                && ((baton->nt.nti.nai.abtAtqa[1] & 0x02) == 0x00)) uiBlocks = 0x7f;
        }
        if (nfc_initiator_select_passive_target(baton->pnd, nmMifare, NULL, 0, &baton->nt) <= 0) {
            tag->SetError("unable to reselect tag");
            break;
        }

        if (nfc_device_set_property_bool(baton->pnd, NP_EASY_FRAMING, true) < 0) {
            snprintf(result, sizeof result, "nfc_device_set_property_bool easyFraming=false: %s",
                nfc_strerror(baton->pnd));
            tag->SetError(result);
            break;
        }

        int cnt, len;
        uint8_t command[MAX_FRAME_LENGTH], data[4 * 1024], *dp;
        len = (uiBlocks + 1) * 16;
        if (((unsigned long)len) > sizeof data) len = sizeof data;
        for (cnt = uiBlocks, dp = data + len - 16;
        cnt >= 0;
            cnt--, dp -= 16) {
            if (((cnt + 1) % (cnt < 128 ? 4 : 16)) == 0) {
                size_t key_index;
                struct mifare_param_auth auth_params;
                for (key_index = 0; key_index < num_keys; key_index++) {
                    zero_mem(command, sizeof command);
                    command[0] = MC_AUTH_B;
                    command[1] = cnt;

                    zero_mem(&auth_params, sizeof auth_params);
                    memcpy(auth_params.abtKey, (uint8_t *)keys + (key_index * 6),
                        sizeof auth_params.abtKey);
                    memcpy(auth_params.abtAuthUid, baton->nt.nti.nai.abtUid + baton->nt.nti.nai.szUidLen - 4,
                        sizeof auth_params.abtAuthUid);
                    memcpy(command + 2, &auth_params, sizeof auth_params);

                    auto rapdulen = sizeof abtRx;
                    res = nfc_initiator_transceive_bytes(baton->pnd, command, 2 + sizeof auth_params, abtRx,
                        &rapdulen, -1);
                    if (res >= 0) break;
                }
                if (key_index >= num_keys) {
                    snprintf(result, sizeof result, "nfc_initiator_transceive_bytes: %s", nfc_strerror(baton->pnd));
                    tag->SetError(result);
                    break;
                }
            }

            command[0] = MC_READ;
            command[1] = cnt;

            size_t rapdulen = 16;
            res = nfc_initiator_transceive_bytes(baton->pnd, command, 2, dp, &rapdulen, -1);
            if (res >= 0) continue;

            if (res != NFC_ERFTRANS) {
                snprintf(result, sizeof result, "nfc_initiator_transceive_bytes: %s", nfc_strerror(baton->pnd));
                tag->SetError(result);
            }
            break;
        }
        if (cnt >= 0) break;

        tag->SetData(data, len);

        tag->SetOffset(16 * 4);
        break;
    }

    case 0x44:
    {
        tag->SetTag("mifare-ultralight");

        if (nfc_device_set_property_bool(baton->pnd, NP_EASY_FRAMING, true) < 0) {
            snprintf(result, sizeof result, "nfc_device_set_property_bool easyFraming=false: %s",
                nfc_strerror(baton->pnd));
            tag->SetError(result);
            break;
        }

        size_t cnt, len, res;
        uint8_t command[2], data[16 * 12], *dp;
        for (n = 0, cc = 0x0f, dp = data, cnt = sizeof data, len = 0;
        n < cc;
            n += 4, dp += res, cnt -= res, len += res) {
            command[0] = MC_READ;
            command[1] = static_cast<uint8_t>(n);
            res = nfc_initiator_transceive_bytes(baton->pnd, command, 2, dp, &cnt, -1);
            if (res >= 0) continue;

            if (res != NFC_ERFTRANS) {
                snprintf(result, sizeof result, "nfc_initiator_transceive_bytes: %s", nfc_strerror(baton->pnd));
                tag->SetError(result);
            }
            break;
        }
        if (n < cc) break;

        tag->SetData(data, len);

        tag->SetOffset(16);
        break;
    }

    default:
        break;
    }
}

void NFCReadWorker::HandleProgressCallback(const char *_tag, size_t size)
{
    Nan::HandleScope scope;

    Local<Object> object = Nan::New<Object>();
    tag->AddToNodeObject(object);
    delete tag;
    tag = NULL;

    Local<Value> argv[2];
    argv[0] = Nan::New("read").ToLocalChecked();
    argv[1] = object;

    Local<Object> self = GetFromPersistent("self").As<Object>();
    Nan::MakeCallback(self, "emit", 2, argv);
}

}
