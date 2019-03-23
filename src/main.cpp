#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "mbedtls/md.h"
#include "ArduinoJson.h"
#include "esp_spi_flash.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void json200(JsonDocument doc, AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");    
    serializeJsonPretty(doc, *response);
    request->send(response);
}


char * hash( const char * payload ) {
  static char result[31];
  result[0] = '/';
  byte shaResult[32];
  
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t payloadLength = strlen(payload);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);  

  for(int i= 0; i< 15; i++)
  {
    sprintf(&result[1+(i*2)], "%02x", (int)shaResult[i]);    
  }  

  return result;
}

String getMimeType(String url){
  if( url.endsWith(".css") ) return String("text/css");
  if( url.endsWith(".js") ) return String("text/javascript");
  if( url.endsWith(".svg") ) return String("image/svg+xml");
  return "text/html";
}

void setUpAccessPoint() {
  // set ip AP
  // IPAddress Ip(192, 168, 4, 1);
  // IPAddress NMask(255, 255, 255, 0);
  // WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP("esp32");
  delay(4000);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
}

int readJson(String body, JsonDocument &d) {
  DeserializationError error = deserializeJson(d, body.c_str());
  if (error) return 0;
  return 1;
}

void setupLanWifi() {
  // save
  File file = SPIFFS.open("/esp32config.json");
  if( file.size() == 0 ) return;
  StaticJsonDocument<256> config;
  
  //String(file.readString())
  if( readJson( file.readString(), config) == 0 ) return;
  const char * ssid = config["ssid"];
  const char * password = config["password"];

  WiFi.begin(ssid,password);
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
  
  setupLanWifi();
  setUpAccessPoint();
  

  // // Connect to Wi-Fi

  server.on("/led_on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_BUILTIN, HIGH); 
    request->send(200, "application/json", "{\"ok\": true}");
  });

  server.on("/led_off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_BUILTIN, LOW); 
    request->send(200, "application/json", "{\"ok\": true}");
  });

  server.on("/read", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("file")) {
      request->send(400, "application/json", "{\"error\" :\"missing file param\"}");
    }

    String file = request->getParam("file")->value();
    Serial.println(file);
    request->send(SPIFFS, file, getMimeType(file)); 

  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!request->hasParam("file")) {
      request->send(400, "application/json", "{\"error\" :\"missing file param\"}");
    }

    String file = request->getParam("file")->value();
    Serial.println(file);

    SPIFFS.remove(file);
    request->send(200, "application/json", "{\"ok\": true}");

  });

  server.on( "/set_wifi", HTTP_POST, [](AsyncWebServerRequest *request){

    StaticJsonDocument<200> d;
    if( !readJsonBody(request, d) ) return;

    // save
    File file = SPIFFS.open("/esp32config.json", FILE_WRITE);
    serializeJsonPretty(d, file);
    file.close();
    
    const char* ssid = d["ssid"];
    const char* password = d["password"];
    Serial.println(ssid);

    WiFi.begin(ssid, password);

   request->send(200, "application/json", "{\"ok\": true}");

  });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    StaticJsonDocument<256> doc;

    while (WiFi.status() != WL_CONNECTED) {
      doc["status"] = "NOT_CONNECTED";    
      json200(doc, request);
      return;
    }

    doc["status"] = "CONNECTED";    
    doc["ip"] = WiFi.localIP().toString();
    json200(doc, request);
  });


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
    request->send(SPIFFS, hash(url.c_str()), getMimeType(url));            
  });
  server.begin();


  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
}

int count = 0;

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    Serial.println(WiFi.localIP());
  }
  delay(1000);
  Serial.println(WiFi.localIP());

}