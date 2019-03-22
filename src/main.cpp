#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "mbedtls/md.h"
#include "ArduinoJson.h"
#include "esp_spi_flash.h"

// Replace with your network credentials

const char* ssid = "XXX";
const char* password = "YYY";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void json(JsonDocument doc, AsyncWebServerRequest * request) {
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


void setup() {

  Serial.begin(115200);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

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
    json(doc, request);
  });

  server.on("/led_off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(LED_BUILTIN, LOW); 
    StaticJsonDocument<256> doc;
    doc["ok"] = true;
    json(doc, request);
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


    json(doc, request);
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
}