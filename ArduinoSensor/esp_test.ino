#include <ESP8266.h>
#include <SoftwareSerial.h>
#include "../../Jaram/IoTDevices/MQTT/3.1.1/mqtt.h"
#include <DHT.h>
#include <IRremote.hpp>

#define SSID "jaram"
#define PASSWORD "Qoswlfdlsnrn"
#define HOST "192.168.0.5"
#define PORT 1883
#define TIMEOUT 10

#define DHTPIN 9
#define DHTTYPE DHT22

#define IR_RECEIVE_PIN 10

int res = 0;

SoftwareSerial esp(7, 8);
ESP8266 wifi(esp, 9600);
struct mqtt_client_t client = {
  .client_name = "C Client",
  .username = "test",
  .password = "test",
  .keepalive = 30,
  .is_ssl = 0
};

DHT dht(DHTPIN, DHTTYPE);

void connectToBroker() {
  wifi.createTCP(HOST, PORT);
  res = mqtt_connect(&client, NULL, NULL);
  if (res) {
    Serial.println("connect failed");
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {  }
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  IrReceiver.start();
  dht.begin();
  Serial.println(wifi.getVersion().c_str());
  
  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.println("Join AP success");
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.println("Join AP failure");
  }

  if (wifi.disableMUX())
    Serial.println("single OK");
  else
    Serial.println("single ERR");

  connectToBroker(); 
  Serial.println("setup end");
}


void loop() {
  delay(1000);
  
  if (IrReceiver.available()) {
    Serial.println(IrReceiver.decode());
    IrReceiver.resume();
  }
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  Serial.print("humidity: ");
  Serial.println(h);
  Serial.print("temperature: ");
  Serial.println(t);char message[10] = {0};
  sprintf(message, "%d", 0);

  res = mqtt_publish(&client, "test/jaram/humidity", message, 1);
  if (res) {
    Serial.println("publish falied");
    
    res = mqtt_pingreq(&client);
    if (res) connectToBroker();
  }
}
