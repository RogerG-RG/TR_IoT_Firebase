#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_SI1145.h>
#include <BMP085.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN

#define BLYNK_PRINT Serial
LiquidCrystal_I2C lcd(0x27,20,4);

char auth[] = "XXXXXXXXX"; 
char ssid[] = "XXXXXXXXX";  
char pass[] = "XXXXXXXXX";

DHT dht(18, DHT22); //(sensor pin,sensor type)
Adafruit_SI1145 GY1145 = Adafruit_SI1145();
BMP085 BMP180;
BlynkTimer timer;

void weather() {
  float humidity = dht.readHumidity(); // in Percentage
  float temperature = dht.readTemperature(); // in Celcius
  if (isnan(humidity) || isnan(temperature)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  float pressure = BMP180.bmp085GetPressure(BMP180.bmp085ReadUP()) / 100; // Atmospheric Pressure in Pascals to hPa by dividing it by 100
  float visibleLight = GY1145.readVisible(); // Visible Radiation (SI1145 does not have a unit for this)
  float ir = GY1145.readIR(); // IR Radiation in Lumens (SI1145 does not have a unit for this)
  float uv = GY1145.readUV(); // UV Index


  Blynk.virtualWrite(V0, temperature);  //V0 is for Temperature
  Blynk.virtualWrite(V1, humidity);  //V1 is for Humidity
  Blynk.virtualWrite(V2, pressure);  //V2 is for Pressure
  Blynk.virtualWrite(V3, visibleLight);  //V3 is for Visible Light
  Blynk.virtualWrite(V4, ir);  //V4 is for IR Radiation
  Blynk.virtualWrite(V5, uv);  //V5 is for UV Radiation

  lcd.setCursor(2, 0);
  lcd.print("Weather  Station");

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature);

  lcd.setCursor(0, 2);
  lcd.print("H:");
  lcd.print(humidity);

  lcd.setCursor(0, 3);
  lcd.print("P:");
  lcd.print(pressure);
  
  lcd.setCursor(10, 1);
  lcd.print("VL:");
  lcd.print(visibleLight);

  lcd.setCursor(10, 2);
  lcd.print("IR:");
  lcd.print(ir);

  lcd.setCursor(10, 3);
  lcd.print("UV:");
  lcd.print(uv);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  BMP180.init();
  GY1145.begin();
  timer.setInterval(1000L, weather); //Calls the weather function every 1000 milliseconds or 1 seconds
}

void loop() {
  Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer
}