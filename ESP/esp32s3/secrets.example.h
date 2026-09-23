#pragma once

// ============================================================================
// Template de credenciais — Nó 2 (ESP32-S3)
// ============================================================================
//
// Copie este arquivo para `secrets.h` no mesmo diretório e preencha com a sua
// rede antes de compilar:
//
//     cp secrets.example.h secrets.h
//
// `secrets.h` está no .gitignore e nunca deve ser commitado. Este template,
// sim — ele documenta quais símbolos o sketch espera encontrar.
//
// A rede precisa ser 2.4 GHz: o ESP32-S3 não tem rádio 5 GHz.

static const char* WIFI_SSID     = "sua-rede-2.4ghz";
static const char* WIFI_PASSWORD = "sua-senha";

// ============================================================================
// Identidade do dispositivo (secp256k1) — ADR-014/ADR-016 path B
// ============================================================================
//
// Gere um par de chaves NOVO off-device. Nunca reaproveite uma chave que já
// foi commitada em algum ponto do histórico do git — o repositório é público
// e o histórico não esquece, então a chave fica permanentemente comprometida
// mesmo que seja removida de commits futuros.
//
//     openssl ecparam -name secp256k1 -genkey -noout -out priv.pem
//     openssl ec -in priv.pem -text -noout
//
// O segundo comando imprime o escalar privado (32 bytes) e o ponto público
// (04 || X || Y, 65 bytes). Preencha DEVICE_PRIVATE_KEY com o escalar,
// DEVICE_PUBLIC_KEY com X||Y (64 bytes, sem o prefixo 0x04) e DEVICE_SOURCE
// com "spark:device:04" + hex(X) + hex(Y) (130 caracteres hex após o "04").
//
// Depois de gravar o firmware, registre a nova pubkey pelo botão "Rotate Key"
// no detalhe do sensor no dashboard — o backend só confia na chave que estiver
// em `devices.public_key`, então envelopes assinados são rejeitados até essa
// chamada acontecer.
//
// UNSIGNED_DEV_BYPASS (ver esp32s3.ino) está ligado hoje, então esta chave
// ainda não é usada para nada que o backend verifique — mas deixe-a correta
// mesmo assim: o dia em que o bypass for desligado não deve depender de
// ninguém lembrar de gerar uma chave de verdade primeiro.

static const uint8_t DEVICE_PRIVATE_KEY[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const uint8_t DEVICE_PUBLIC_KEY[64] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const char* DEVICE_SOURCE =
  "spark:device:0400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
