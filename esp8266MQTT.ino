/*
 * Projeto da disciplina Microcontrolador
 * Tales Iago Batista
 * 
 * O seguinte projeto busca controlar a velocidade e direção de um motor
 * via comandos MQTT. 
 * 
 * Para isso, foi implemetado uma lógica para um ESP8266 juntamente com uma
 * ponte H. O uso de comandos enviados  via MQTT, usando servidor publico.
 * para acionar um motor, podendo gerenciar a direção e a velocidade
 * 
 * Velocidade de 0 a 1024
 * 
 * Direção: frente, re, esquerda e direita
 */

#include <StringSplitter.h> //Biblioteca para faticar string
#include <ESP8266WiFi.h> // Biblioteca que gerencia o Wifi 
#include <PubSubClient.h> // Biblioteca do MQTT

#define TOPICO_SUBSCRIBE "motorControl-talesiago-micro"     //tópico MQTT de escuta
#define ID_MQTT  "Motor"     //id mqtt (para identificação de sessão)

// Configuração do WIFI
const char* SSID = "RITA"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "s3m_s3nh@"; // Senha da rede WI-FI que deseja se conectar

// Configuração do MQTT
const char* BROKER_MQTT = "broker.emqx.io"; //Servidor
int BROKER_PORT = 1883; //Porta

WiFiClient espClient; // Cria o objeto referente a ao Wifi

PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient

//Funções
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);

/*
    Implementações das funções
*/
void setup()
{
  //inicializações:
  InitOutput();
  initSerial();
  initWiFi();
  initMQTT();
  //Led da placa, para acompanhar o funcionamento
  pinMode(D4,OUTPUT);
  digitalWrite(D4,HIGH);
}

void initSerial()
{
  Serial.begin(115200);
}

void initWiFi()
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconectWiFi();
}

void initMQTT()
{
  MQTT.setServer(BROKER_MQTT,BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
  //Cria um vetor para receber a string fatiada
  String comando[3];
  //Chama o fatiador da string , usando o '/' como referencia.
  StringSplitter *splitter = new StringSplitter(msg, '/', 3);  
  int itemCount = splitter->getItemCount();

  //Passa do fatiador para o vetor
  for (int i = 0; i < itemCount; i++) {
    comando[i] = splitter->getItemAtIndex(i);
  }

  Serial.println("Comando recebido: " + msg);
  msg = "";

  //Chama a função referente ao motor, onde passa a direção e a velocidade
  if (comando[0] == "Motor") {
    direcao(comando[1],comando[2].toInt());
  }
}

void reconnectMQTT()
{
  while (!MQTT.connected())
  {
    digitalWrite(D4,HIGH);
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      hold(100);
    }
  }
}

void reconectWiFi()
{
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(D4,HIGH);
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
void InitOutput(void)
{  
  //PWM
  pinMode(D8, OUTPUT);
  pinMode(D7, OUTPUT);
  digitalWrite(D8, LOW);
  digitalWrite(D7, LOW);

  //Direita
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  //Esquerda
  pinMode(D6, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D6, LOW);
  digitalWrite(D5, LOW);
}


//programa principal
void loop()
{
  //garante funcionamento das conexões WiFi e ao broker MQTT
  VerificaConexoesWiFIEMQTT();

  //envia o status de todos os outputs para o Broker no protocolo esperado
  //EnviaEstadoOutputMQTT();

  //keep-alive da comunicação com broker MQTT
  MQTT.loop();
  digitalWrite(D4,LOW);
}
void direcao(String dir, int veloc) {
  Serial.println("Direcao: " + dir + " Velocidade: " + veloc);
  if (dir == "frente") {
    analogWrite(D8, veloc);
    analogWrite(D7, veloc);
    digitalWrite(D2, HIGH);
    digitalWrite(D3, LOW);
    digitalWrite(D5, HIGH);
    digitalWrite(D6, LOW);
  } else if (dir == "esquerda") {
    analogWrite(D8, veloc);
    analogWrite(D7, veloc);
    digitalWrite(D2, LOW);
    digitalWrite(D3, HIGH);
    digitalWrite(D5, HIGH);
    digitalWrite(D6, LOW);
  } else if (dir == "direita") {
    analogWrite(D8, veloc);
    analogWrite(D7, veloc);
    digitalWrite(D2, HIGH);
    digitalWrite(D3, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, HIGH);
  } else if (dir == "re") {
    analogWrite(D8, veloc);
    analogWrite(D7, veloc);
    digitalWrite(D2, LOW);
    digitalWrite(D3, HIGH);
    digitalWrite(D5, LOW);
    digitalWrite(D6, HIGH);
  } else if (dir == "off") {
    analogWrite(D8, 0);
    analogWrite(D7, 0);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
  }
}
void hold(const unsigned int &ms) {
  // Non blocking hold
  unsigned long m = millis();
  while (millis() - m < ms) {
    yield();
  }
}
