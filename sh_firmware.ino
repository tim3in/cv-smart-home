#include <WiFi.h>
#include "PubSubClient.h"

// Button and relay pins
const int BTN1 = 13;
const int BTN2 = 14;
const int relay1 = 17;
const int relay2 = 16;

// Button states
bool lastBTN1_State = LOW;
bool lastBTN2_State = LOW;

// Relay states
bool relay1State = LOW;
bool relay2State = LOW;

// WiFi and MQTT settings
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqttServer = "broker.hivemq.com";
const int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  wifiConnect();

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());

  client.setServer(mqttServer, port);
  client.setCallback(callback);
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" connected");
      client.subscribe("home/light");
      client.subscribe("home/fan");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "home/light") {
    if (stMessage == "light1_on") {
      relay1State = HIGH;
    } else if (stMessage == "light1_off") {
      relay1State = LOW;
    }
    digitalWrite(relay1, relay1State);
  }

  if (String(topic) == "home/fan") {
    if (stMessage == "fan_on") {
      relay2State = HIGH;
    } else if (stMessage == "fan_off") {
      relay2State = LOW;
    }
    digitalWrite(relay2, relay2State);
  }
}

void loop() {
  // Read the current state of the buttons
  bool currentBTN1_State = digitalRead(BTN1);
  bool currentBTN2_State = digitalRead(BTN2);

  // Check if button 1 state has changed
  if (currentBTN1_State != lastBTN1_State) {
    lastBTN1_State = currentBTN1_State;
    if (currentBTN1_State == HIGH) {
      relay1State = !relay1State;
      digitalWrite(relay1, relay1State);
      Serial.println("Button 1 Clicked - Light toggled");
    }
  }

  // Check if button 2 state has changed
  if (currentBTN2_State != lastBTN2_State) {
    lastBTN2_State = currentBTN2_State;
    if (currentBTN2_State == HIGH) {
      relay2State = !relay2State;
      digitalWrite(relay2, relay2State);
      Serial.println("Button 2 Clicked - Fan toggled");
    }
  }

  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();
}
