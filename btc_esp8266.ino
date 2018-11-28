/**
 * Created by esp8266_IDE.
 * User: max33303
 * Date: 7.10.18
 * Time: 15:12
 * Получение данных с сайта в виде JSON массива, парсинг данных и вывод их на дисплей от Nokia5110
 */
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include<string.h>;
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 5, 4);
char ssid[] = "WiFi";       //ID WiFI сети
char password[] = "12340000";  // пароль от Wifi
WiFiClientSecure client; //+ SSL колиент
long checkBlockchainDueTime;
int checkBlockchainDelay = 60000; // 60 x 1000 1 минута

String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}

void getDataOfBlockchain() {
  String headers = "";
  String body = "";
  bool finishedHeaders = false;
  bool currentLineIsBlank = true;
  bool gotResponse = false;
  long now;

  char host[] = "blockchain.info";

  if (client.connect(host, 443)) {
    Serial.println("connected");
    
    String URL = "/ru/ticker";

    Serial.println(URL);    
    client.println("GET " + URL + " HTTP/1.1");
    client.print("Host: "); client.println(host);
    client.println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36");
    client.println("");
    now = millis();
    // выдерживаем паузу
    while (millis() - now < 1500) {
      while (client.available()) {
        char c = client.read();
      //  Serial.print(c);
   if (finishedHeaders) {
          body=body+c;
        } else {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          }
          else {
            headers = headers + c;
          }
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        }else if (c != '\r') {
          currentLineIsBlank = false;
        }
        //ответ пришел без ошибок
        gotResponse = true;
      }
      if (gotResponse) {
		   // чистит любые символы до { - признак начала json
       while(body.startsWith("{")==false){
      body=body.substring(1,body.length()+1);
       }
  const size_t bufferSize = 22*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(22) + 1660;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(body);    
  JsonObject& USD = root["USD"];
  float USD_15m = USD["15m"]; // 6533.96
  float USD_last = USD["last"]; // 6533.96
  float USD_buy = USD["buy"]; // 6533.96
  float USD_sell = USD["sell"]; // 6533.96
  const char* USD_symbol = USD["symbol"]; // "$"
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(utf8rus("Курс битка:"));
  display.setCursor(0,9);
  display.print(USD_15m);
  display.print("$ 15m");
  display.setCursor(0,18);
  display.print(USD_last);
  display.print("$ last");
  display.setCursor(0,27);
  display.print(USD_buy);
  display.print("$ buy");
  display.setCursor(0,36);
  display.print(USD_sell);
  display.print("$ sell");
  display.display();        
        }   
      }
    }
    Serial.println("-************-");
    Serial.println(body);
    Serial.println("-************-");
  }

void setup() {
  Serial.begin(115200);
  display.begin();
  display.cp437(true);
  display.setContrast(60);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);  
  display.display();
  delay(500); 
}

void loop() {
  long now = millis();
  if(now >= checkBlockchainDueTime) {
  Serial.println("---------");
  getDataOfBlockchain();
  checkBlockchainDueTime = now + checkBlockchainDelay;
  }
}
