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

#define BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN
#define BMP_SCK  (22)
#define BMP_SDI  (21)
#define BLYNK_PRINT Serial
#define WIFI_RESET_BUTTON 15
LiquidCrystal_I2C lcd(0x27,20,4);

char auth[] = "lFIuB0T7RorE7rthyQceXEi1dJ_oWm-0"; 

DHT dht(18, DHT22); //(sensor pin,sensor type)
Adafruit_SI1145 GY1145 = Adafruit_SI1145();
Adafruit_BMP280 bmp280;
BlynkTimer timer;
WiFiManager wifiManager;

void weather() {
  float humidity = dht.readHumidity(); // in Percentage Relative Humidity (RH)
  float temperature = dht.readTemperature(); // in Celcius

  float pressure = bmp280.readPressure() / 100.0F; // Atmospheric Pressure in Pascals to hPa by dividing it by 100
  float altitude = bmp280.readAltitude(1013.25) * 0.3048; // Altitude measured in feet converted to metes by multiplying by 0.3048. 1013.25 is the standard sea level pressure in hPa.
  int visibleLight = GY1145.readVisible(); // Visible Radiation (SI1145 does not have a unit for this)
  int uv = GY1145.readUV(); // UV Index is a unitless quantity

  Blynk.virtualWrite(V0, temperature);  //V0 is for Temperature
  Blynk.virtualWrite(V1, humidity);  //V1 is for Humidity
  Blynk.virtualWrite(V2, pressure);  //V2 is for Pressure
  Blynk.virtualWrite(V3, altitude);  //V3 is for Altitude
  Blynk.virtualWrite(V4, visibleLight);  //V4 is Visible Light
  Blynk.virtualWrite(V5, uv);  //V5 is for UV Radiation

  char temperatureString[10];
  char humidityString[10];
  char pressureString[10];
  char altitudeString[10];
  sprintf(temperatureString, "T:%.1f", temperature);
  sprintf(humidityString, "H:%.1f", humidity);
  sprintf(pressureString, "P:%.1f", pressure);
  sprintf(altitudeString, "Alt:%.1f", altitude);


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
  wifiManager.autoConnect("ESP32WeatherStation");

  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("Waiting for WiFi");
  }

  lcd.setCursor(0, 0);
  lcd.print("Connected to WiFi");

  Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());
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
  timer.setInterval(1000L, weather); //Calls the weather function every 1000 milliseconds or 1 seconds
}

void loop() {
  Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer
  if(digitalRead(WIFI_RESET_BUTTON) == LOW) {
    wifiManager.resetSettings();
    ESP.restart();
  }
}