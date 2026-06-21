#pragma once
#include <Arduino.h>

// Estados possíveis da conexão de rede
enum NetworkStatus {
    NET_DISCONNECTED,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_API_ERROR
};

// Configuração da API (altere aqui ou via tela de config)
#define NET_API_URL     "https://seusite.com"
#define NET_API_TIMEOUT 8000  // ms para timeout de requests

// Funções de ciclo de vida da rede (independentes do rádio)
void net_init();                            // Carrega credenciais salvas e tenta conectar
void net_disconnect();                      // Desconecta e libera recursos WiFi

// Funções de status (sem bloquear o loop principal)
NetworkStatus net_status();
String net_ip();

// Funções da API (bloqueantes enquanto rodam, mas não interferem no rádio)
bool net_push_signal(int signalIndex);      // Envia signalHistory[i] para o servidor
bool net_pull_history();                    // Baixa histórico do servidor e preenche signalHistory

// Funções de configuração (para a tela de WiFi)
void net_save_credentials(const char* ssid, const char* password, const char* apiUrl, const char* apiToken);
bool net_load_credentials();               // Retorna true se encontrou credenciais salvas
String net_get_ssid();
String net_get_api_url();
bool net_get_ap_mode_flag();
void net_set_ap_mode_flag(bool active);
