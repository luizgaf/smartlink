#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Definições do serviço e característica BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const int relayPin = 5;
bool relayState = false;
bool deviceConnected = false;

// Callbacks para eventos BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Dispositivo conectado");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Dispositivo desconectado");
      delay(500);
      pServer->startAdvertising();
      Serial.println("Aguardando conexão...");
    }
};

// Callbacks para características BLE - VERSÃO COMPATÍVEL
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // Método compatível com todas as versões da biblioteca BLE
      String receivedValue = String(pCharacteristic->getValue().c_str());
      
      if (receivedValue.length() > 0) {
        char cmd = receivedValue[0];
        Serial.print("Comando recebido: ");
        Serial.println(cmd);
        
        String response = "";
        
        if (cmd == '1') {
          digitalWrite(relayPin, HIGH);
          relayState = true;
          response = "LIGADO";
          Serial.println("Relé LIGADO");
        } else if (cmd == '0') {
          digitalWrite(relayPin, LOW);
          relayState = false;
          response = "DESLIGADO";
          Serial.println("Relé DESLIGADO");
        } else if (cmd == 's' || cmd == 'S') {
          response = relayState ? "Status: LIGADO" : "Status: DESLIGADO";
          Serial.println(response);
        } else if (cmd == 't' || cmd == 'T') {
          // Toggle - alterna o estado
          relayState = !relayState;
          digitalWrite(relayPin, relayState);
          response = relayState ? "TOGGLE: LIGADO" : "TOGGLE: DESLIGADO";
          Serial.println(response);
        } else {
          response = "Comando invalido. Use: 1=ON, 0=OFF, s=Status, t=Toggle";
          Serial.println("Comando invalido recebido");
        }
        
        // Definir o valor de resposta
        pCharacteristic->setValue(response.c_str());
        pCharacteristic->notify();
      }
    }
};

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

void setupBLE() {
  // Inicializa o BLE
  BLEDevice::init("ESP32-Relay-BLE");
  
  // Cria o servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria o serviço BLE
  pService = pServer->createService(SERVICE_UUID);

  // Cria a característica BLE
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("ESP32 Relay Control - Ready");
  
  // Inicia o serviço
  pService->start();

  // Configura o advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  
  // Configurações para melhor compatibilidade com iOS e Android
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMaxPreferred(0x12);
  
  // Inicia o advertising
  BLEDevice::startAdvertising();
}

void setup() {
  Serial.begin(115200);
  
  // Configuração do pino do relé
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  
  Serial.println("Iniciando ESP32 BLE Relay...");
  
  // Inicializa BLE
  setupBLE();
  
  Serial.println("BLE iniciado com sucesso!");
  Serial.println("Nome do dispositivo: ESP32-Relay-BLE");
  Serial.println("Conecte-se com qualquer app BLE");
  Serial.println("Comandos disponíveis:");
  Serial.println("  1 - Ligar Relé");
  Serial.println("  0 - Desligar Relé");
  Serial.println("  s - Status do Relé");
  Serial.println("  t - Alternar Estado (Toggle)");
  Serial.println("Aguardando conexão BLE...");
}

void loop() {
  // Controle de heartbeat quando conectado
  if (deviceConnected) {
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 2000) {
      lastBlink = millis();
      Serial.println("Dispositivo conectado - Aguardando comandos...");
    }
  }
  
  delay(1000);
}