#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Willphone";
const char* password = "bruhbruh";

const char* mqtt_server = "a3au8j7vh6yy3c-ats.iot.us-east-2.amazonaws.com";  // e.g. a1b2c3-ats.iot.us-east-1.amazonaws.com
const int mqtt_port = 8883;
const char* mqtt_topic = "esp32/pub";

static const char AMAZON_ROOT_CA1[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char DEVICE_CERT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUeIbV4p/aPU+hMmNM6ts4xUkN/bkwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDQxMDE2NDM0
OFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALpe+DyfzrkYqUCzVndG
3rlHNnsTn9YCvN+VlCdhOUYyHvnz/Z+2KJW+Ez/Mh2L2ST7tim2NN6a3lKqbaGt3
128F8uHSiSJqVdwfIfbGSvkALcUm1+JEQu+QtQZXujRvdBr/qXfJb52b4PzTe6MK
6vMLNDThrwNL4LxwQuYQtsEjq0EudZy/Ysy2wbeLJSjSKLzN6HgNc7FkrpgzZQ4t
JwGIYmYpqFiWCmdQd0Wg1LS7FfLaSS4ToXJjtFAzUM01GfFcNWOH2qS6eOoyOi8h
8+YuJLkEknQZYCiBgsanbOW+wHlg7TYA1d9vYcvfcC1shwlqMNH2tr7AKRgjgr5w
Df0CAwEAAaNgMF4wHwYDVR0jBBgwFoAU9vwPcpn3OCvbb2n4TrRJMQuxiw8wHQYD
VR0OBBYEFMLUgaKUGxfFAr7G4CRazwxgxVY7MAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCXXcUHuZRZJh9bcdikfk/SjMD8
XhwJiS/k1xb5TzlJtSgyfx7vSRdYq9iBPpusfFJgPHOD9a0CF/NK12V2eWJoujuq
xFdH19fE7MyCq9+GbU5sX/M3xfquPsPtZJTfxg65jGzgltuBtXbpC4dNeZyBz9yD
GG3i176hgX6sLkvV5XRqD3THHyuCIQzNxvVOpQ3G0ARCJ342c7dyJOaSLw/VsSAc
X7fYTVZEmuiZyAsP0QW8wXt99oEK2ZrYs1QgPy0yhq1Oy3iCJ6kG6DMK8noDYLfF
BaPSx1wZVV5uVb4i8rrbBXGK6u9jYyuAVaWH/6lsIP/t+fskLN37LnyJyGdT
-----END CERTIFICATE-----
)KEY";

static const char PRIVATE_KEY[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAul74PJ/OuRipQLNWd0beuUc2exOf1gK835WUJ2E5RjIe+fP9
n7Yolb4TP8yHYvZJPu2KbY03preUqptoa3fXbwXy4dKJImpV3B8h9sZK+QAtxSbX
4kRC75C1Ble6NG90Gv+pd8lvnZvg/NN7owrq8ws0NOGvA0vgvHBC5hC2wSOrQS51
nL9izLbBt4slKNIovM3oeA1zsWSumDNlDi0nAYhiZimoWJYKZ1B3RaDUtLsV8tpJ
LhOhcmO0UDNQzTUZ8Vw1Y4fapLp46jI6LyHz5i4kuQSSdBlgKIGCxqds5b7AeWDt
NgDV329hy99wLWyHCWow0fa2vsApGCOCvnAN/QIDAQABAoIBAAbQSgsNXKVvvWBc
LiEWOjZT4C4HvaQ5XWZfNZdq+okSdAd3YCJiqoEu/c0rURfRIURnstPmgc5aj9jc
7ahgLUQl14diBxl3AcSWbsty3HLWCtZWYTYogLn5dGTS+D/Kg1gpWKzEdIwykrsr
kC2ZzJUuJ653QcrBD2pUQF3g5o2RDD9/9rbilA/HsZXir5ZaI208ODT6d7EveZ8S
Lvt8hIVkJ8lX5vISz9CAqwUHscXPVbX8yHLsV1sW2ZRtfY6YEq6bO2yt1Z9nI21J
bOdPEXVJ2jvjKyGl+BYSy6T+3X8w31dcquR8aYemv7j9QHJ/FPQbVTMIbFHROJrb
345BpBECgYEA7Pjsp2JUu8kUY8mPARfM7BAIAwvUE9KVHAyd2xCxGavLPqyL4DgQ
uYtWn1HSUB5zDfVPbWdvEbHsde6Po13Ezc4nxfBngPIivmK7slgUvThe/GLMK20U
J47aLCzjYxURc24cCazP8+QqN8Va1g/fvOsCRMY4uJSd2NPWwi2GJbsCgYEAyVXp
Gf9M2TFj0tzcW4PND3CM5pOvQmsUgghmTtVdR66Ptifp0wceHaWls3GX8PsGRBwM
GBFZhtZ0LjfbBlIz5o/7YIA9l/Z28gsBMdvBfE24/CGAu/0Stj69zkoBUIpVau99
D9wtP5Ia66f2jwfNz1WB2kVD1OWWuqz04ayQw6cCgYBaMLdpUWxseDBxdjIeOV+9
7LAP8L2LudcUcNzXlnfBDHhzZxppgQ0kgRq70llYMu6ZPmc3fWlxc6VKkTxL8hFI
PxE2LbRHCGEe0tcbHpjuF3lXV4bbdE16bM0uiPsWJ9uXci3rpMEU0x8aUNusoygF
b/BSGptV7QJSlvWiu87VpwKBgD+dIGixbLOXUmYWmJ1luI3vTuWvtglzUeiABOtV
qQJ0Gj5AwDWIIzDU3NUkyBBBEErG3d9g4/uhmNXdmhuWL4Xi6lckTdHM8uopJfh9
6dV/rTokz8sV/CqPCmO6qE2tV63Q+5/ePoG+hTj+Of4ZRiMDbmLrzbHf1M8oxLrl
TlwzAoGAX22XUsuryqzUVoFaC6uXHhFU2yQZclHn5ZjWqzc8CkdORSV//cVv8uE8
2j69uuPrt+YpuE9MCbvQgqYru1V7RtJrWPMqUjNPJvMdgYE8zLz8TJ/sSiP0p/gG
vsHMqFQBiOnG+GVkZcG7wa2ugbMcRivFwwJmLUppgvEpb8DN17U=
-----END RSA PRIVATE KEY-----
)KEY";

// If your key is PKCS#8 format, it may look like:
// -----BEGIN PRIVATE KEY-----
// ...
// -----END PRIVATE KEY-----

WiFiClientSecure net;
PubSubClient client(net);

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

void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWiFi();
  connectAWS();
}

void loop() {
  if (!client.connected()) {
    connectAWS();
  }

  client.loop();

  StaticJsonDocument<200> doc;
doc["device"] = "esp32";
doc["message"] = "Hello AWS IoT";
doc["count"] = 1;

String payload;
serializeJson(doc, payload);


  boolean ok = client.publish(mqtt_topic, payload.c_str());

  if (ok) {
    Serial.println("Published:");
    Serial.println(payload);
  } else {
    Serial.println("Publish failed");
  }

  delay(5000);
}
