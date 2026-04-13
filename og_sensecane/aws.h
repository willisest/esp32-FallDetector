#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

 
extern WiFiClientSecure net;
extern PubSubClient client;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void connectAWS() {
  net.setCACert(AMAZON_ROOT_CA1);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(PRIVATE_KEY);

  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected()) {
    Serial.print("Connecting to AWS IoT ... ");

    String clientId = "esp32-client-01";

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void publishMessage(String fallDetected){
  if (!client.connected()) {
    connectAWS();1
  }

  StaticJsonDocument<200> doc;
doc["device"] = "esp32";
doc["message"] = "Hello AWS IoT";
doc["count"] = 1;
doc["fallDetected"] = fallDetected;


String payload;
serializeJson(doc, payload);

static unsigned long lastPublishTime = 0;

bool isEmergency = (fallDetected == "FALL DETECTED");

  if (isEmergency || (millis() - lastPublishTime > 5000)) {
  
  boolean ok = client.publish(mqtt_topic, payload.c_str());
  lastPublishTime = millis();

  if (ok) {
    Serial.println("Published:");
    Serial.println(payload);
  } else {
    Serial.println("Publish failed");
  }

  

}
  
  }
