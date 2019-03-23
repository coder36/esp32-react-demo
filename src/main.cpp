#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "mbedtls/md.h"
#include "ArduinoJson.h"
#include "esp_spi_flash.h"

// Replace with your network credentials

const char* ssid = "BTHub5-TJWK";
const char* password = "b85b72c9a5";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);



void json200(JsonDocument doc, AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");    
    serializeJsonPretty(doc, *response);
    request->send(response);
}


String hash( String payload ) {
  char result[31];
  result[0] = '/';
  byte shaResult[32];
  
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = payload.length();

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) payload.c_str(), payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);  

  for(int i= 0; i< 15; i++)
  {
    sprintf(&result[1+(i*2)], "%02x", (int)shaResult[i]);    
  }  

  return String(result);
}

String getMimeType(String url){
  if( url.endsWith(".css") ) return String("text/css");
  if( url.endsWith(".js") ) return String("text/javascript");
  if( url.endsWith(".svg") ) return String("image/svg+xml");
  return "text/html";
}

void setUpAccessPoint() {
  // set ip AP
  IPAddress Ip(192, 168, 250, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP("esp32");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

int readJsonBody(AsyncWebServerRequest *request, JsonDocument &d) {
  AsyncWebParameter* p = request->getParam("body", true);    
  String j = p->value();
  DeserializationError error = deserializeJson(d, j.c_str());
  if (error) {
    request->send(400, "application/json", "{\"error\" :\"invalid json\"}");
    return 0;
  }
  return 1;
}


void setup() {

  Serial.begin(115200);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  setUpAccessPoint();

  // // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  server.on("/led_on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_BUILTIN, HIGH); 
    StaticJsonDocument<256> doc;
    doc["ok"] = true;
    json200(doc, request);
  });

  server.on("/led_off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_BUILTIN, LOW); 
    StaticJsonDocument<256> doc;
    doc["ok"] = true;
    json200(doc, request);
  });


  server.on( "/set_wifi", HTTP_POST, [](AsyncWebServerRequest *request){

    StaticJsonDocument<200> d;
    if( !readJsonBody(request, d) ) return;
    
    const char* ssid = d["ssid"];
    const char* password = d["password"];
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println(WiFi.localIP());


    StaticJsonDocument<256> doc;
    doc["ok"] = true;
    doc["ip"] = WiFi.localIP();
    
    json200(doc, request);

  });
  //     [](AsyncWebServerRequest * request){},NULL,
  //     [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
  //   Serial.println("jhere");
  //   StaticJsonDocument<200> doc;
  //   DeserializationError error = deserializeJson(doc, (const char*)data);
  //   if (error) {
  //     Serial.print(F("deserializeJson() failed: "));
  //     Serial.println(error.c_str());
  //     request->send(400, "application/json", "{}");
  //     return;
  //   }

  //   StaticJsonDocument<256> res;
  //   res["ok"] = true;
  //   json(res, request);
  // });


  server.on("/wifi_scan", HTTP_GET, [](AsyncWebServerRequest *request){

    digitalWrite(LED_BUILTIN, LOW); 
    StaticJsonDocument<256> doc;
    doc["ok"] = true;
    JsonArray networks = doc.createNestedArray("networks");


    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
      JsonObject e = networks.createNestedObject();
      e["ssid"] = WiFi.SSID(i);
      e["rssi"] = WiFi.RSSI(i);

      String encType = "";
      if( WiFi.encryptionType(i) == WIFI_AUTH_OPEN ) encType = "OPEN";
      if( WiFi.encryptionType(i) == WIFI_AUTH_WEP ) encType = "WEP";
      if( WiFi.encryptionType(i) == WIFI_AUTH_WPA_PSK ) encType = "WPA_PSK";
      if( WiFi.encryptionType(i) == WIFI_AUTH_WPA2_PSK ) encType = "WPA2_PSK";
      if( WiFi.encryptionType(i) == WIFI_AUTH_WPA_WPA2_PSK ) encType = "WPA_WPA2_PSK";
      if( WiFi.encryptionType(i) == WIFI_AUTH_WPA2_ENTERPRISE ) encType = "WPA2_ENTERPRISE";
      if( WiFi.encryptionType(i) == WIFI_AUTH_MAX ) encType = "MAX";

      e["security"] = encType;
    }

    json200(doc, request);


    

// int n = WiFi.scanNetworks();
//     Serial.println("scan done");
//     if (n == 0) {
//         Serial.println("no networks found");
//     } else {
//         Serial.print(n);
//         Serial.println(" networks found");
//         for (int i = 0; i < n; ++i) {
//             // Print SSID and RSSI for each network found
//             Serial.print(i + 1);
//             Serial.print(": ");
//             Serial.print(WiFi.SSID(i));
//             Serial.print(" (");
//             Serial.print(WiFi.RSSI(i));
//             Serial.print(")");
//             Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
//             delay(10);
//         }
//     }
//     Serial.println("");

  });


  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    StaticJsonDocument<256> doc;
    doc["ssid"] = WiFi.SSID();
    doc["cores"] = chip_info.cores;
    doc["flash"] = spi_flash_get_chip_size() / (1024 * 1024);
    doc["BT"] = (chip_info.features & CHIP_FEATURE_BT) > 0;
    doc["BLE"] = (chip_info.features & CHIP_FEATURE_BLE) > 0;
    doc["freeHeap"] = ESP.getFreeHeap();


    json200(doc, request);
  });

  server.on("", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println(request->url());
    String url = request->url();
    if( url == "/" ) url = "/index.html";
    request->send(SPIFFS, hash(url), getMimeType(url));            
  });
  server.begin();


  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
// Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    // int n = WiFi.scanNetworks();
    // Serial.println("scan done");
    // if (n == 0) {
    //     Serial.println("no networks found");
    // } else {
    //     Serial.print(n);
    //     Serial.println(" networks found");
    //     for (int i = 0; i < n; ++i) {
    //         // Print SSID and RSSI for each network found
    //         Serial.print(i + 1);
    //         Serial.print(": ");
    //         Serial.print(WiFi.SSID(i));
    //         Serial.print(" (");
    //         Serial.print(WiFi.RSSI(i));
    //         Serial.print(")");
    //         Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
    //         delay(10);
    //     }
    // }
    // Serial.println("");

    // Wait a bit before scanning again

}