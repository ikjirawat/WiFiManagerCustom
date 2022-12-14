#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

// Brownout detector was triggered
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";

// Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

IPAddress localIP;
// IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
// IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000; // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 15;
// Stores LED state

String ledState;

// Initialize SPIFFS
void initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An error has occurred while mounting SPIFFS");
    }
    Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return String();
    }

    String fileContent;
    while (file.available())
    {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- frite failed");
    }
}

// Initialize WiFi
bool initWiFi()
{
    if (ssid == "" || ip == "")
    {
        Serial.println("Undefined SSID or IP address.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    localIP.fromString(ip.c_str());
    localGateway.fromString(gateway.c_str());

    if (!WiFi.config(localIP, localGateway, subnet))
    {
        Serial.println("STA Failed to configure");
        return false;
    }
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;

    while (WiFi.status() != WL_CONNECTED)
    {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            Serial.println("Failed to connect.");
            return false;
        }
    }

    Serial.println(WiFi.localIP());
    return true;
}

// Replaces placeholder with LED state value
String processor(const String &var)
{
    if (var == "STATE")
    {
        if (digitalRead(ledPin))
        {
            ledState = "ON";
        }
        else
        {
            ledState = "OFF";
        }
        return ledState;
    }
    return String();
}

void setup()
{
    // Serial port for debugging purposes
    Serial.begin(115200);

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable detector

    initSPIFFS();

    // Set GPIO 2 as an OUTPUT
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Cut the space in the end of the word.

    // Load values saved in SPIFFS
    ssid = readFile(SPIFFS, ssidPath);
    pass = readFile(SPIFFS, passPath);
    ip = readFile(SPIFFS, ipPath);
    gateway = readFile(SPIFFS, gatewayPath);
    Serial.println(ssid);
    Serial.println(pass);
    Serial.println(ip);
    Serial.println(gateway);

    ip.trim();
    gateway.trim();
    ssid.trim();
    pass.trim();

    if (initWiFi())
    {
        // Route for root / web page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
        server.serveStatic("/", SPIFFS, "/");

        // Route to set GPIO state to HIGH
        server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
      digitalWrite(ledPin, HIGH);
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });

        // Route to set GPIO state to LOW
        server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
      digitalWrite(ledPin, LOW);
      request->send(SPIFFS, "/index.html", "text/html", false, processor); });
        server.begin();
    }
    else
    {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
        // NULL sets an open Access Point
        WiFi.softAP("ESP-WIFI-MANAGER", NULL);

        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);

        // Web Server Root URL
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, "/wifimanager.html", "text/html"); });

        server.serveStatic("/", SPIFFS, "/");

        server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
                  {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });
        server.begin();
    }
}

void loop()
{
}

// //----------------parameter form Web server----------------------
// void Setting_parameter_on_web_server()
// {
//     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/index.html", "text/html", false); });

//     server.on("/espReset", HTTP_GET, [](AsyncWebServerRequest *request)
//               {
//     ESP.restart();
//     request->send(200, "text/plain", "ESP32 has been reset!"); });

//     server.on("/showpara", HTTP_GET, [](AsyncWebServerRequest *request)
//               {
//     String str = "";
//     int paramsNr = request->params();
//     str = "";
//     int wf = 0;
//     String strIP = "";

//     DynamicJsonDocument  doc(1024);
//     JsonObject pconvert =  doc.to<JsonObject>();;

//     AsyncWebParameter* p = request->getParam(0);
//     AsyncWebParameter* p1 = request->getParam(1);
//     AsyncWebParameter* p2 = request->getParam(2);
//     AsyncWebParameter* p3 = request->getParam(3);
//     AsyncWebParameter* p4 = request->getParam(4);
//     AsyncWebParameter* p5 = request->getParam(5);
//     AsyncWebParameter* p6 = request->getParam(6);
//     AsyncWebParameter* p7 = request->getParam(7);
//     AsyncWebParameter* p8 = request->getParam(8);
//     AsyncWebParameter* p9 = request->getParam(9);
//     AsyncWebParameter* p10 = request->getParam(10);

//     pconvert[p->name()] = p->value();
//     pconvert[p1->name()] = p1->value();
//     pconvert[p2->name()] = p2->value();
//     pconvert[p3->name()] = p3->value();
//     pconvert[p4->name()] = p4->value();
//     pconvert[p5->name()] = p5->value();
//     pconvert[p6->name()] = p6->value();
//     pconvert[p7->name()] = p7->value();
//     pconvert[p8->name()] = p8->value();
//     pconvert[p9->name()] = p9->value();
//     pconvert[p10->name()] = p10->value();

//     char bufSend[1024];
//     serializeJson(doc, bufSend);

//     Serial.print("From web:");
//     Serial.println(bufSend);

//     wf = writeToFile("setting.json", bufSend); //////Write to SPIFFS
//     set_parameter_from_SPIFFS();

//     //-----------------RE-Serial Number-------------------
//     snprintf(serialNo, sizeof(serialNo), "SN:%s-%04X%08X", mcName, chip, (uint32_t)chipid);
//     mcSerial = String(serialNo);
//     //----------------------------------------------------

//     request->send(200, "text/plain", "message received\n\n" + String(str)); });
// }
// //--------------------------------------------------------