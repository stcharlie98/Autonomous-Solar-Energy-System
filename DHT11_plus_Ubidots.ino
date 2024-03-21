// Biblioteca para comunicação com a plataforma Ubidots
#include <UbidotsEsp32Mqtt.h>
// Biblioteca para interação com os sensores
#include "DHT.h"
// Biblioteca para entrar em modo hibernação. É o modo 
// 'deep sleep', mas com consumo 30x menor (5 uA) e com
// consumo 9200x menor comparando ao modo normal (46 mA).
#include <esp_sleep.h>

// Parâmetros do sensor DHT 
#define DHTTYPE DHT11  
uint8_t DHTPIN = 4;
DHT dht(DHTPIN, DHTTYPE);

// Parâmetro do sensor de luminosidade
uint8_t analogPin = 35;           

// Configurações do Ubidots e da conexão Wi-Fi
const char *UBIDOTS_TOKEN = "BBFF-lKcfF3UNLvvQCGrZyzJ1XlXnhskj3S";
const char *WIFI_SSID = "Patrick";      
const char *WIFI_PASS = "kikao1234";     
const char *DEVICE_LABEL = "DHT11-ESP32-C";
const char *VARIABLE_LABEL_1 = "temperatura"; 
const char *VARIABLE_LABEL_2 = "umidade";
const char *VARIABLE_LABEL_3 = "luz"; 
// Publica dados 1 segundo depois de sair do modo hibernação
// ou a cada segundo caso não houvesse este modo habilitado.
const int PUBLISH_FREQUENCY = 1000; 
// Este timer conta o tempo em ms para realizar ações no
// tempo requerido.
unsigned long timer; 
Ubidots ubidots(UBIDOTS_TOKEN);

// Função importante de retorno em tempo real para mensagens MQTT. 
void callback(char *topic, byte *payload, unsigned int length)
{
  // Quando chega uma nova mensagem MQTT, esta função é chamada
  // e imprime uma mensagem.
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // Itera cada byte do conteúdo da mensagem MQTT e 
  // exibe no Monitor Serial.
  for (int i = 0; i < length; i++)
  {
    // Transcreve cada byte do conteúdo em caractere
    // para que possamos entendê-lo no Monitor Serial.
    Serial.print((char)payload[i]);
  }
  // Nova linha no M. Serial para melhor compreensão.
  Serial.println();
}

// Aqui, o ESP32 inicializa a comunicação serial, 
// o sensor DHT e a conexão Ubidots, configurando 
// uma função de retorno de chamada, realizando 
// os atrasos essenciais e inicializando o  
// timer com a contagem atual em ms.
void setup()
{
  Serial.begin(115200);     
  dht.begin();
  delay(1000);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  timer = millis();
}

void loop()
{
  // Reconecta automaticamente com a plataforma
  // se não conectada
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  // Como o tempo está passando, ele conta para "puxar"
  // os dados após 1s, mas, claro, se não estiver em
  // modo de hibernação, daí demora 11s, por exemplo.
  if (millis() - timer >= PUBLISH_FREQUENCY) 
  {
    // Leitura dos sensores
    float u = dht.readHumidity();
    float t = dht.readTemperature();
    float l = analogRead(analogPin);
    // Conexão entre sensores e a plataforma
    // Ubidots.
    ubidots.add(VARIABLE_LABEL_1, t);
    ubidots.add(VARIABLE_LABEL_2, u);
    ubidots.add(VARIABLE_LABEL_3, l);

    // Publicação dos dados
    ubidots.publish(DEVICE_LABEL);

    // Para saber pelo Monitor Serial se está
    // passando dados para a nuvem.
    Serial.println("Temperatura: " + String(t));
    Serial.println("Umidade: " + String(u));
    Serial.println("Luminosidade: " + String(l));

    // Melhor leitura no M. Serial
    Serial.println("-----------------------------------------");
    // Timer resetado
    timer = millis();

    // Delay adicionado ao modo hibernação, porque estava dando erro
    delay(100); 

    // Checa conexão do sinal do Wi-Fi antes de entrar no modo hibernação
    if (ubidots.connected() && WiFi.status() == WL_CONNECTED) {
      // Configura e entra no modo hibernação
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
      esp_sleep_enable_timer_wakeup(10000000); // Dorme por 10 segundos
      esp_deep_sleep_start();
    } else {
      // Wi-Fi instável, porém em loop até funcionar e ficar estável.
      Serial.println("Conexão instável, trabalhando nisso...");
    }
  }
  // Pausa estratégica de 100 ms
  // para que algumas tarefas fun-
  // cionem de modo assíncrono.
  delay(100); 
  ubidots.loop();
}