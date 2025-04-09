#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <ESP_Mail_Client.h>
#include <math.h>

// === WiFi Credentials ===
#define WIFI_SSID "WIFINAME"
#define WIFI_PASSWORD "PASSWORD"

// === SMTP Email Setup ===
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 465
#define SENDER_EMAIL "addemailhere"
#define SENDER_PASSWORD "addpasswordhere"
#define RECIPIENT_EMAIL "addreceipentemailhere"
#define RECIPIENT_NAME "addnamehere"

// === Sensor Pins ===
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int mq2Pin = 34;
const int mq136Pin = 35;
const int ledPin = 33;
const int buzzerPin = 32;

// === LCD RS, E, D4, D5, D6, D7 ===
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);

// === ThingSpeak Info ===
const char* thingSpeakServer = "api.thingspeak.com";
String apiKey = "VQAPCU67UWQKD3KJ";

// === Email Session Setup ===
SMTPSession smtp;
ESP_Mail_Session session;
SMTP_Message message;

// === Email Control ===
bool emailSent = false;
unsigned long lastEmailTime = 0;
const unsigned long emailCooldown = 10 * 60 * 1000; // 10 minutes

// === Thresholds ===
int sensorThres = 2200;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  lcd.clear();
  Serial.println("WiFi Connected");
}

void setupEmail() {
  session.server.host_name = SMTP_SERVER;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  smtp.debug(1);

  message.sender.name = "ESP32 Smart Alert";
  message.sender.email = SENDER_EMAIL;
  message.subject = "Alert from ESP32";
  message.addRecipient(RECIPIENT_NAME, RECIPIENT_EMAIL);
}

void sendAlertEmail(String msgBody) {
  message.html.content = msgBody.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session)) {
    Serial.println("SMTP connect failed!");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.print("Email error: ");
    Serial.println(smtp.errorReason());
  } else {
    Serial.println("Alert email sent!");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  dht.begin();
  lcd.begin(16, 2);
  lcd.clear();
  connectWiFi();
  setupEmail();
}

void loop() {
  int mq2Value = analogRead(mq2Pin);
  int mq136Value = analogRead(mq136Pin);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("MQ2 Analog: "); Serial.println(mq2Value);
  Serial.print("MQ136 Analog: "); Serial.println(mq136Value);

  lcd.clear();
  lcd.setCursor(0, 0);

  bool danger = (mq2Value > sensorThres || mq136Value > sensorThres);

  if (danger) {
    Serial.println("!!! DANGER DETECTED !!!");
    digitalWrite(ledPin, HIGH);
    tone(buzzerPin, 1000);

    if (mq2Value > sensorThres)
      lcd.print("   DANGER(SMOKE)");
    else
      lcd.print("   DANGER(H2S)");

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C H:");
    lcd.print(humidity, 1);
    lcd.print("%");

    if ((!emailSent || (millis() - lastEmailTime > emailCooldown)) && WiFi.status() == WL_CONNECTED) {
      String body = "";
      if (mq2Value > sensorThres) {
        body = "<h2>\xF0\x9F\x9A\xA8 ESP32 SMOKE ALERT \xF0\x9F\x9A\xA8</h2>";
        body += "<p>MQ2: " + String(mq2Value) + "<br>";
      } else if (mq136Value > sensorThres) {
        body = "<h2>\xF0\x9F\x9A\xA8 ESP32 H2S ALERT \xF0\x9F\x9A\xA8</h2>";
        body += "<p>MQ136: " + String(mq136Value) + "<br>";
      }
      body += "Temp: " + String(temperature) + " C<br>";
      body += "Humidity: " + String(humidity) + " %</p>";
      sendAlertEmail(body);
      emailSent = true;
      lastEmailTime = millis();
    }
  } else {
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    lcd.print("Air Quality Safe");
    emailSent = false;
  }

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print("C H:");
  lcd.print(humidity, 1);
  lcd.print("%");

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = String("http://") + thingSpeakServer + "/update?api_key=" + apiKey +
                 "&field1=" + String(mq2Value) +
                 "&field2=" + String(mq136Value) +
                 "&field3=" + String(temperature) +
                 "&field4=" + String(humidity);
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("ThingSpeak upload OK: %d\n", httpCode);
    } else {
      Serial.printf("ThingSpeak upload failed: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  delay(1000);
}
