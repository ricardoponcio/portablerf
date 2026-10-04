#include "meshtastic.h"
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>

// Chave que o firmware Meshtastic usa quando o PSK é de 1 byte ("AQ==" = índice 1)
static const uint8_t DEFAULT_PSK[16] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
                                        0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};

static uint8_t xorHash(const uint8_t* p, size_t n) {
    uint8_t h = 0;
    while (n--) h ^= *p++;
    return h;
}

MeshDecoder::MeshDecoder() {
    String name = "LongFast", psk = "AQ==";
    setChannels(&name, &psk, 1);
}

// Mesmas regras do firmware Meshtastic (Channels::getKey)
bool MeshDecoder::expandPsk(const String& psk, MeshChannel& ch) {
    uint8_t raw[48];
    size_t n = 0;
    if (mbedtls_base64_decode(raw, sizeof(raw), &n, (const unsigned char*)psk.c_str(), psk.length()) != 0 || n > 32)
        return false;
    memset(ch.key, 0, sizeof(ch.key));
    if (n == 0 || (n == 1 && raw[0] == 0)) {
        ch.keyLen = 0;  // canal sem criptografia
    } else if (n == 1) {
        memcpy(ch.key, DEFAULT_PSK, 16);
        ch.key[15] += raw[0] - 1;  // "chaves simples" 1..10 variam o último byte
        ch.keyLen = 16;
    } else {
        memcpy(ch.key, raw, n);  // curta demais completa com zeros
        ch.keyLen = n <= 16 ? 16 : 32;
    }
    return true;
}

bool MeshDecoder::setChannels(const String* names, const String* psks, size_t n) {
    if (n > MAX_CHANNELS) return false;
    MeshChannel tmp[MAX_CHANNELS];
    for (size_t i = 0; i < n; i++) {
        tmp[i].name = names[i];
        tmp[i].psk = psks[i];
        if (!expandPsk(psks[i], tmp[i])) return false;
        tmp[i].hash = xorHash((const uint8_t*)names[i].c_str(), names[i].length()) ^ xorHash(tmp[i].key, tmp[i].keyLen);
    }
    for (size_t i = 0; i < n; i++) channels_[i] = tmp[i];
    count_ = n;
    return true;
}

int MeshDecoder::decrypt(const uint8_t* pkt, size_t len, uint8_t* out) const {
    if (len <= HEADER_LEN) return -1;
    const uint8_t* payload = pkt + HEADER_LEN;
    size_t n = len - HEADER_LEN;

    for (size_t i = 0; i < count_; i++) {
        const MeshChannel& ch = channels_[i];
        if (ch.hash != pkt[13]) continue;
        if (ch.keyLen == 0) {
            memcpy(out, payload, n);
        } else {
            // Nonce: id do pacote (64 bits LE, o id ocupa só os 4 primeiros) + nó de origem + 4 zeros
            uint8_t nonce[16] = {0};
            memcpy(nonce, pkt + 8, 4);
            memcpy(nonce + 8, pkt + 4, 4);
            uint8_t stream[16];
            size_t off = 0;
            mbedtls_aes_context aes;
            mbedtls_aes_init(&aes);
            mbedtls_aes_setkey_enc(&aes, ch.key, ch.keyLen * 8);
            mbedtls_aes_crypt_ctr(&aes, n, &off, nonce, stream, payload, out);
            mbedtls_aes_free(&aes);
        }
        // Dois canais podem ter o mesmo hash: o protobuf Data sempre começa pelo campo 1 (portnum)
        if (out[0] == 0x08) return i;
    }
    return -1;
}
