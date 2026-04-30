// Basic demo for accelerometer readings from Adafruit MPU6050
#include "aws.h"
#include <WiFi.h>
#include <TinyGPS++.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32 
#define OLED_RESET    -1 

WiFiClientSecure net;
PubSubClient client(net);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

 const int buzzerPin = 23;


// GPS Setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // Use ESP32 Hardware UART 1
const int RXPin = 34;
const int TXPin = 35;
const uint32_t GPSBaud = 9600;

 //detect fall function

unsigned long freeFallStartTime = 0;
bool potentialFall = false;


  void detectFall(float totalAcceleration) {

    


  if (totalAcceleration < 4.5 ){
    potentialFall = true;
    freeFallStartTime = millis();
    
  }
  

  // Step 2: Confirm Impact within a 500ms window
  if (potentialFall) {
    if (millis() - freeFallStartTime < 800) {
      if (totalAcceleration > 20.0) {
     
        publishMessage("FALL DETECTED");
         
        potentialFall = false; // Reset to avoid double-triggering
      }
    } else {
      potentialFall = false; // Timeout: Free-fall didn't lead to impact
      
    }
  
  }


  publishMessage("NO FALL DETECTED");
 
  
}
 
void setup(void) {


Serial.begin(115200);
  connectWiFi();
  connectAWS();

  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("GPS Serial Started");

  pinMode(buzzerPin, OUTPUT); // Sets the pin to "push" electricity
  digitalWrite(buzzerPin, LOW);
  
  
  while (!Serial)
    delay(10); 

if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  display.display(); // This will turn the screen ON and show the logo

  display.clearDisplay();      // Clear any junk data
  display.setTextSize(1);      // Set text size
  display.setTextColor(WHITE); // Set color to white
  display.setCursor(0, 0);     // Start at top-left
  display.print("Screen On"); 
  display.display();           // THIS LINE IS CRITICAL - it pushes data to screen
  display.clearDisplay();      // Clear any junk data

 


  
 // Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  /*Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);

*/
 
}


void loop() {
//AWS 
  client.loop();

  
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);


float totalAcceleration = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

 
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
  
  
  display.clearDisplay();
  display.setCursor(0,0);
/*
// print out serially 
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
  
*/
  /* Print out the values */
  display.clearDisplay();
  display.print("Accel X: "); display.println(a.acceleration.x);
  display.print("Accel Y: "); display.println(a.acceleration.y);
  display.print("Accel Z: "); display.println(a.acceleration.z);

  display.display();           // 4. CRITICAL - Push the buffer to hardware
  delay(100);                  // 5. Small delay so the text doesn't flicker

//fall threshold algorithm

detectFall(totalAcceleration);
}
