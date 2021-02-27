// Medidor de temperatura e umidade da regiao

//--------------- Bibliotecas para o wifi ------------------------
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//----------------------------------------------------------------


//---------------- Sensor de temperatura e umidade ---------------
#include "DHT.h"

float  leitura_sensor;
float t = 0;
float h = 0;
DHT dht(0, DHT11); // pino do sensor D3
//----------------------------------------------------------------


//------------------- Para pegar o tempo atual --------------------
#include <NTPClient.h>
#include <WiFiUdp.h>

const long utcOffsetInSeconds = -10800;

char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

unsigned long currentTime = 0;
unsigned long initialTime = 0;

//----------------------------------------------------------------




//-------- Responsavel pela comunicacao com websocket ------------
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  // valor da temperatura em graus Celsius
  t = dht.readTemperature();

  // valor da umidade
  h = dht.readHumidity();

  String dia_da_semana = daysOfTheWeek[timeClient.getDay()];
  String hora = timeClient.getFormattedTime();
  
  String dados = String(t) + "," + String(h) + "," + dia_da_semana + "-" + hora;

  switch(type) {
    case WStype_DISCONNECTED:
      digitalWrite(LED_BUILTIN, HIGH); // Desliga o LED
      Serial.printf("[WSc] Desconectado!\n");
      break;
    case WStype_CONNECTED:
      digitalWrite(LED_BUILTIN, LOW); // Liga o LED
      Serial.printf("[WSc] Conectado ao servidor\n");
      break;
      
    case WStype_TEXT:
      webSocket.sendTXT(dados);
      Serial.print("Temperatura: ");
      Serial.println(dados);
      break;
  }

  delay(300);
}
//----------------------------------------------------------------


//------------ Verifica se algum dado foi alterado ---------------
void verificaAlteracaoDados(){
  float temp, umid = 0;
  // valor da temperatura em graus Celsius
  temp = dht.readTemperature();

  // valor da umidade
  umid = dht.readHumidity();

  if(umid != h || temp != t){
    webSocket.loop();
  }
}
//----------------------------------------------------------------

// responsavel por conectar ao wifi, servidor e iniciar o sensor
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); // Inicia o LED interno
  digitalWrite(LED_BUILTIN, HIGH); // Desliga o LED

  WiFiManager wifiManager;
  wifiManager.autoConnect("NODEMCU-ESP8266");
  Serial.println("Conectado ao wifi");

  
  delay(500);
  dht.begin();

  delay(1000);
  

  initialTime = millis();

  // endereco do servidor, porta e URL
  webSocket.begin("api-temp-umid.herokuapp.com", 80, "/");
//  webSocket.begin("192.168.15.10", 4000, "/"); // se for rodar localhost

  // event handler
  webSocket.onEvent(webSocketEvent);

  // tenta a cada 2 segundos se a conexao falhar
  webSocket.setReconnectInterval(2000);

  timeClient.begin();
}


// fica atualizando a hora com a internet e verifica se algum dado foi alterado
void loop() {
  timeClient.update();
  verificaAlteracaoDados();
}
