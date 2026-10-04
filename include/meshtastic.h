#pragma once
#include <Arduino.h>

// Leitura de pacotes Meshtastic (só escuta: não retransmite nem se anuncia como nó).
// Pacote no ar = cabeçalho de 16 bytes em claro + protobuf "Data" cifrado em AES-CTR
// com a chave do canal. Aqui só decifra; o protobuf é decodificado na página.
struct MeshChannel {
    String name;
    String psk;        // base64, como aparece no app ("AQ==" = chave padrão)
    uint8_t key[32];
    uint8_t keyLen;    // 0 = sem criptografia; 16 = AES-128; 32 = AES-256
    uint8_t hash;      // vai no byte 13 do cabeçalho: XOR do nome ^ XOR da chave
};

class MeshDecoder {
public:
    static constexpr size_t MAX_CHANNELS = 4;
    static constexpr size_t HEADER_LEN = 16;

    MeshDecoder();  // começa com o canal padrão: LongFast / AQ==

    // Troca todos os canais. Não mexe em nada se algum PSK for inválido.
    bool setChannels(const String* names, const String* psks, size_t n);
    size_t count() const { return count_; }
    const MeshChannel& channel(size_t i) const { return channels_[i]; }

    // Tenta os canais cujo hash bate com o do pacote; out recebe len - HEADER_LEN bytes.
    // Devolve o índice do canal que decifrou, ou -1.
    int decrypt(const uint8_t* pkt, size_t len, uint8_t* out) const;

private:
    static bool expandPsk(const String& psk, MeshChannel& ch);

    MeshChannel channels_[MAX_CHANNELS];
    size_t count_ = 0;
};
