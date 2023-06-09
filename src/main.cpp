#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_SI1145.h>
#include <Adafruit_BMP280.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>

#define FIREBASE_HOST "https://ce-binus-iot-course-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "Du6OLGVeZMdhMce3NFlnh4V7JYFTXM6S1el4U5pH"
#define BMP_SCK  (22)
#define BMP_SDI  (21)
#define WIFI_RESET_BUTTON 15

char apName[] = "ESP32-Weather-Station";

LiquidCrystal_I2C lcd(0x27,20,4);
DHT dht(18, DHT22); //(sensor pin,sensor type)
Adafruit_SI1145 GY1145 = Adafruit_SI1145();
Adafruit_BMP280 bmp280;
BlynkTimer timer;
WiFiManager wifiManager;
FirebaseData fbdo;
FirebaseConfig fbConfig;
FirebaseData fbdoStream;

void weather() {
  float humidity = dht.readHumidity(); // in Percentage Relative Humidity (RH)
  float temperature = dht.readTemperature(); // in Celcius

  float pressure = bmp280.readPressure() / 100.0F; // Atmospheric Pressure in Pascals to hPa by dividing it by 100
  float altitude = bmp280.readAltitude(1013.25) * 0.3048; // Altitude measured in feet converted to metes by multiplying by 0.3048. 1013.25 is the standard sea level pressure in hPa.
  int visibleLight = GY1145.readVisible(); // Visible Radiation (SI1145 does not have a unit for this)
  float uv = GY1145.readUV() / 100.0F; // UV Index is a unitless quantity. Divided by 100 to get the value

  Firebase.RTDB.setFloat(&fbdo, "/data/current/humidity", humidity);
  Firebase.RTDB.setFloat(&fbdo, "/data/current/temperature", temperature);
  Firebase.RTDB.setFloat(&fbdo, "/data/current/pressure", pressure);
  Firebase.RTDB.setFloat(&fbdo, "/data/current/altitude", altitude);
  Firebase.RTDB.setInt(&fbdo, "/data/current/visibleLight", visibleLight);
  Firebase.RTDB.setFloat(&fbdo, "/data/current/uv", uv);

  char temperatureString[10];
  char humidityString[10];
  char pressureString[10];
  char altitudeString[10];
  char uvString[10];
  sprintf(temperatureString, "T:%.1f", temperature);
  sprintf(humidityString, "H:%.1f", humidity);
  sprintf(pressureString, "P:%.1f", pressure);
  sprintf(altitudeString, "Alt:%.1f", altitude);
  sprintf(uvString, "UV:%.1f", uv);

  lcd.setCursor(2, 1);
  lcd.print("        ");
  lcd.setCursor(2, 2);
  lcd.print("        ");
  lcd.setCursor(3, 3);
  lcd.print("       ");
  lcd.setCursor(12, 1);
  lcd.print("        ");
  lcd.setCursor(14, 2);
  lcd.print("      ");
  lcd.setCursor(13, 3);
  lcd.print("       ");

  lcd.setCursor(2, 0);
  lcd.print("Weather  Station");

  lcd.setCursor(0, 1);
  lcd.print(temperatureString);

  lcd.setCursor(0, 2);
  lcd.print(humidityString);

  lcd.setCursor(10, 1);
  lcd.print(pressureString);
  
  lcd.setCursor(10, 2);
  lcd.print(altitudeString);

  lcd.setCursor(0, 3);
  lcd.print("UV:");
  lcd.print(uv);

  lcd.setCursor(10, 3);
  lcd.print("VL:");
  lcd.print(visibleLight);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  wifiManager.autoConnect(apName);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi");
  }

  if(digitalRead(WIFI_RESET_BUTTON) == LOW) {
    wifiManager.resetSettings();
    ESP.restart();
  }

  lcd.setCursor(0, 0);
  lcd.print("Connected to WiFi");
  
  Firebase_Init("cmd");
  dht.begin();
  
  if (!bmp280.begin(0x76)) {
    lcd.setCursor(0, 0);
    lcd.print("BMP280 not found");
    while (1);
  }

  if (!GY1145.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("GY1145 not found");
    while (1);
  }

  lcd.clear();
  
  timer.setInterval(900000L, weather); //Calls the weather function every 15 minutes
}

void loop() {
  timer.run(); // Initiates SimpleTimer
  if(digitalRead(WIFI_RESET_BUTTON) == LOW) {
    wifiManager.resetSettings();
    ESP.restart();
  }
}

void onFirebaseStream(FirebaseStream data)
{
  //onFirebaseStream: /cmd /ledGreen int 0
  Serial.printf("onFirebaseStream: %s %s %s %s\n", data.streamPath().c_str(),
                data.dataPath().c_str(), data.dataType().c_str(),
                data.stringData().c_str());
}

void Firebase_Init(const String& streamPath)  {
  FirebaseAuth fbAuth;
  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

  fbdo.setResponseSize(2048);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "medium");
  while (!Firebase.ready())
  {
    Serial.println("Connecting to firebase...");
    delay(1000);
  }
  String path = streamPath;
  if (Firebase.RTDB.beginStream(&fbdoStream, path.c_str()))
  {
    Serial.print("Firebase stream on ");
    Serial.println(path);
    Firebase.RTDB.setStreamCallback(&fbdoStream, onFirebaseStream, 0);
  }
  else
    Serial.print("Firebase stream failed: ");
    Serial.println(fbdoStream.errorReason().c_str());
}