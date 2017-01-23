#include <DHT.h>
#include <ESP8266WiFi.h>
#include <config.h>
#include <Wire.h>

// Initialize light level sensor
#include <BH1750.h>
BH1750 lightMeter;

//Initialize temperature and humidity sensor
#define DHTPIN 5      // 5 -> D1   DHT Data Pin
#define DHTPWRPIN 12  // 12 -> D6   DHT/BH1750 Power Pin
#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321

volatile float hum = 0;
volatile float temp = 2147483647;
volatile unsigned long lastRead = 0;
volatile unsigned long lastSleep = millis();
volatile float batVolt;
volatile int dataSent = 0;
const int analogInPin = A0;
volatile int analogValue = 0;
volatile int lightLevel = 0;
volatile int wifiRetryCount = 0;

//Set ip address at startup (no DHCP) makes the connection ~2sec faster; Comment to use DHCP
IPAddress ip(172, 16, 0, 140); // where xx is the desired IP Address
IPAddress gateway(172, 16, 0, 3); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

DHT dht(DHTPIN, DHTTYPE);
//WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Ready!");

  pinMode(analogInPin, INPUT);
  pinMode(DHTPWRPIN, OUTPUT);
  digitalWrite(DHTPWRPIN, HIGH);
  delay(10);
  dht.begin();
  lightMeter.begin();
  Serial.println("BH1750 Test");

  //Set static IP
  Serial.println();
  Serial.println("Setting static ip to : ");
  Serial.println(ip);
  WiFi.config(ip, gateway, subnet);
  //Put WiFi in modem_sleep
  Serial.println("Disabling the WiFi chip....");
  //WiFi.disconnect();
  WiFi.forceSleepBegin();
  delay(100);
}

void loop() {
  delay (500);
  //Read battery level
  readBatt();

  //Read Light level
  readLightLevel();
  Serial.print("Light: ");
  Serial.print(lightLevel);
  Serial.println(" lx");
  //delay(2000);

  //Read Temperature
  readDht22();

  //Check if we have the temp and humid values and send
  if ((temp != 2147483647) && (hum != 2147483647) && (hum != 0) && (temp != 0)) {
    wifiConnect();
    httpClient();
  } else {
    Serial.println("No values to send");
    delay(500);
  }

  // Enter Sleep if we sent data 2 times or stayed awake more than awakesec
  if ((dataSent > 0) || (millis() - lastSleep >= awakeSec * 1000)) {
    Serial.print("Entering sleep for ");
    Serial.print(sleepSec);
    Serial.println(" sec");
    Serial.print("Was awake for:");
    Serial.print((millis() - lastSleep) / 1000);
    Serial.println("sec.");
    ESP.deepSleep(sleepSec * 1000000);
  }
}

void readDht22() {
  if (millis() - lastRead >= 500) {
    Serial.println("Read DHT sensor");
    // Read temperature as Celsius (the default) and humidity
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C ");

    // Check if any reads failed and exit early (to try again).
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    lastRead = millis();
  }
}

void httpClient() {
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/emoncms/input/post.json?";
  url += "node=";
  url += node;
  url += "&json=";
  url += "temp:";
  url += temp;
  url += ",hum:";
  url += hum;
  url += ",batt:";
  url += batVolt;
  url += ",light:";
  url += lightLevel;
  url += "&apikey=";
  url += apikey;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host +
               "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 3000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    // Look for ok at the start of the line;
    int char1 = line.indexOf('o');
    int char2 = line.indexOf('k');
    if (char1 == 1 && char2 == 2) {
      dataSent++;
    }
  }

  Serial.println();
  Serial.println("closing connection");
}

void readBatt() {


  // Read battery voltage from A0 measure ~0.0264V per unit, needs calibration
  analogValue = analogRead(analogInPin);
  batVolt = analogValue * 0.02297;
  //Serial.print("sensor = ");
  //Serial.println(analogValue);
}

void readLightLevel() {
  //The I2C pins are defined in the BH1750.h
  lightLevel = lightMeter.readLightLevel();
}

void wifiConnect() {
  if ( WiFi.status() != WL_CONNECTED ) {
    //Enable WiFi chip
    WiFi.forceSleepWake();
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);
    Serial.print("Attempting to connect to AP, SSID: ");
    Serial.println(ssid);
    // Connect to WiFi network
    WiFi.begin(ssid, password);
  }
  while ( WiFi.status() != WL_CONNECTED ) {
    delay (500);
    Serial.print(".");
    wifiRetryCount++;
    if ( wifiRetryCount > 10 ) {
      wifiRetryCount = 0;
      break;
    }
  }
  if ( WiFi.status() == WL_CONNECTED ) {
    Serial.println("WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}
