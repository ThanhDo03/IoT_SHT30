#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL6mVSGUNO-"
#define BLYNK_TEMPLATE_NAME "SHT30"

#define BUZZER_PIN D5 // Buzzer (Piezo) connected to GPIO 5 (D5) on ESP8266
#define LIGHT_SENSOR_PIN D4 // Analog pin connected to the light sensor
#define RELAY_PIN D3 // Relay connected to GPIO 4 (D4) on ESP8266

char auth[] = "ObtjykHbxmh1krLUK6HP6KKcUMXaum6m";
char ssid[] = "ThanhDoCompany"; 
char pass[] = "Hongwifiroi"; 

#include <Wire.h>
#include "Adafruit_SHT31.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);
byte degree[8] = {0B01110,0B01010,0B01110,0B00000,
                  0B00000,0B00000,0B00000,0B00000};
float temp;
float hum;
int led = 2;
int lightValue;

WidgetLED ledconnect(V0);
WidgetLED ledbuzzer(V3);
WidgetLED ledlight(V4);
BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(led,OUTPUT);
  digitalWrite(led,HIGH);

  pinMode(LIGHT_SENSOR_PIN, INPUT_PULLUP);// define pin as Input  sensor
  pinMode(RELAY_PIN, OUTPUT);// define pin as OUTPUT for relay

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.setCursor(3,0);
  lcd.print("Welcome to");
  lcd.setCursor(2,1);
  lcd.print("Tran Thanh Do");
  delay(2000);
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  int n=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(n>20){
      break;
    }
    n++;
  }
  if(WiFi.isConnected()){
    Serial.println("Wifi is connected!");
    Blynk.config(auth,"blynk.cloud", 80);
    Blynk.connect();
  }  
  lcd.clear();
  lcd.setCursor(0,0);        
  lcd.print("Temp :");    
  lcd.setCursor(0,1);        
  lcd.print("Hum  :");
  timer.setInterval(1000L, blinkLedWidget);
}

void loop() {
  if(WiFi.isConnected()){
    Blynk.run();
  }
  timer.run();

  // Read light sensor value
  int L = digitalRead(LIGHT_SENSOR_PIN);// read the sensor 

  // If light sensor value is below a threshold, turn on the relay (turn on the light)
  if (L == 1) {
    Serial.println("Light is ON");
    digitalWrite(RELAY_PIN,LOW);
    ledlight.on();
    digitalWrite(led,HIGH);
  } else {
    // Otherwise, turn off the relay (turn off the light)
    Serial.println("Light is OFF");
    digitalWrite(RELAY_PIN,HIGH);
    ledlight.off();
    digitalWrite(led,LOW);
  }
  delay(500);
}

void blinkLedWidget() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if(!isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
    temp=t;
  } else { 
    Serial.println("Failed to read temperature");
  }

  if(!isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    hum=h;
  } else { 
    Serial.println("Failed to read humidity");
  }

  if (Blynk.connected()) {
    if (ledconnect.getValue()) {
      ledconnect.off();
      digitalWrite(led,HIGH);
    } else {
      ledconnect.on();
      digitalWrite(led,LOW);
    }
    Blynk.virtualWrite(V1,temp);
    Blynk.virtualWrite(V2,hum);
  }

  // Check if the temperature is higher than 30 degrees C
  if (t >= 35.00) {
    // Turn on the buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    ledbuzzer.on();
    digitalWrite(led,HIGH);
    // Send a notification to the Blynk app
    // Serial.println(warning);
  } else {
    // Turn off the buzzer
    digitalWrite(BUZZER_PIN, LOW);
    ledbuzzer.off();
    digitalWrite(led,LOW);
  }

  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
  loopCnt++;
  lcd.setCursor(7,0);        
  lcd.print(String(temp, 1)); 
  lcd.write(0);
  lcd.print("C  ");   
  lcd.setCursor(7,1);
  lcd.print(String(hum, 1));        
  lcd.print("%  ");
}
