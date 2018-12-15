//"http://esp.local";
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include "switch.h";

int Switch::Count = 0;
Switch slaveSwitch;
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50
#define PORT 80
#define DBG_OUTPUT_PORT Serial

const char *mdnsName = "esp"; // Domain name for the mDNS responder
const char *ssid = "Asianet";
const char *password = "95679760";

const char *ap_ssid = "esp_switch"; // ap_ssid of server (Access Point (AP))
const char *ap_password = "";       // ap_password of server (Access Point (AP))

ESP8266WebServer server(PORT);
WebSocketsServer webSocket(81);

File fsUploadFile;
MDNSResponder mdns;
bool startBlink = false;

void startWiFi()
{
    WiFi.mode(WIFI_STA);

    Serial.print("Connecting to WiFi AP..........");
    //Device as AP
    if (WiFi.softAP(ap_ssid, ap_password))
    {
        // Display the server address
        Serial.print("Connected, AP Address: ");
        Serial.print("http://");
        Serial.print(WiFi.softAPIP());
        Serial.println("/");
    }
    else
        Serial.print("Not able to start AP.");

    //Connecting to WIFI
    int retries = 0;

    Serial.print("Connecting to WiFi ..........");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY))
    {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print("#");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Connetted to ");
        Serial.print(ssid);
        Serial.print("--- IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.print("Error connecting to: ");
        Serial.println(ssid);
    }
}
void startSPIFFS()
{                   // Start the SPIFFS and list all contents
    SPIFFS.begin(); // Start the SPI Flash File System (SPIFFS)
    Serial.println("SPIFFS started. Contents:");
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next())
        { // List the file system contents
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
    }
}
void register_switch()
{

    StaticJsonBuffer<500> jsonBuffer;
    String post_body = server.arg("plain");
    Serial.println(post_body);

    JsonObject &jsonBody = jsonBuffer.parseObject(server.arg("plain"));

    _prepareHeader();

    if (!jsonBody.success())
    {
        Serial.println("error in parsin json body");
        server.send(400);
    }
    else
    {
        if (server.method() == HTTP_POST)
        {
            // Switch newSwitch(jsonBody["Ip"], jsonBody["Status"]);
            // slaveSwitch.Ip =  (String)jsonBody["Ip"];;
            slaveSwitch.Ip = jsonBody["Ip"].asString();
            slaveSwitch.Status = jsonBody["Status"];
            server.send(201);
            delay(3000);
            startBlink = true;
        }

        // if (exist == 0)
        // {
        //     server.send(404);
        // }
    }
}
void blink()
{
    Serial.println("Inside Blink");
    if (slaveSwitch.Status)
    {
        //off here
        slaveSwitch.Status = 0;
        onRequest(slaveSwitch);
    }
    else
    {
        //on here
        slaveSwitch.Status = 1;
        onRequest(slaveSwitch);
    }
}

void onRequest(Switch data)
{
    Serial.println("On Request");
    HTTPClient http; //Declare object of class HTTPClient
    Serial.println(data.Ip);
    http.begin("http://" + data.Ip + "/update");        //Specify request destination
    http.addHeader("Content-Type", "application/json"); //Specify content-type header

    char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;

    JsonObject &jsonObj = jsonBuffer.createObject();
    jsonObj["Status"] = data.Status;
    jsonObj["Ip"] = data.Ip;
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

    Serial.println(JSONmessageBuffer);

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload);  //Print request response payload

    http.end(); //Close connection
}

void get_devices()
{
    char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;
    JsonArray &array = jsonBuffer.createArray();

    JsonObject &jsonObj = jsonBuffer.createObject();
    jsonObj["Ip"] = slaveSwitch.Ip;

    array.prettyPrintTo(JSONmessageBuffer, sizeof(jsonObj));

    _prepareHeader();
    server.send(200, "application/json", JSONmessageBuffer);
}
String getContentType(String filename)
{
    if (server.hasArg("download"))
        return "application/octet-stream";
    else if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}
bool handleFileRead(String path)
{
    DBG_OUTPUT_PORT.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {
        if (SPIFFS.exists(pathWithGz))
            path += ".gz";
        File file = SPIFFS.open(path, "r");

        //if ((contentType != "text/html") || (contentType != "text/htm")) {
        //  bool isPublic = true;
        //  String cache = String(isPublic ? "public" : "private") +", max-age=" + String(86400) + ", must-revalidate";
        //  server.sendHeader("Cache-Control", cache);
        //}

        size_t sent = server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}
String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}
void _prepareHeader()
{
    // ... some code to prepare the response data...
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
}
void routingSetup()
{
    server.on("/", HTTP_GET, []() {
        if (!handleFileRead("/index.html"))
            server.send(404, "text/plain", "FileNotFound");
    });
    // server.on("/", HTTP_GET, []() {
    //     server.send(200, "text/html",
    //                 "Welcome to the Master");
    // });
    server.on("/register", HTTP_POST, register_switch);
    server.on("/test", HTTP_POST, blink);
    server.onNotFound(handleNotFound);
    server.serveStatic("/", SPIFFS, "/", "max-age=86400");
}
void handleNotFound()
{
    if (server.method() == HTTP_OPTIONS)
    {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Max-Age", "10000");
        server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.send(204);
    }
    else
    {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";

        for (uint8_t i = 0; i < server.args(); i++)
        {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }

        server.send(404, "text/plain", message);
    }
}

void startWebSocket()
{                                      // Start a WebSocket server
    webSocket.begin();                 // start the websocket server
    webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
    Serial.println("WebSocket server started.");
}

void startMDNS()
{                         // Start the mDNS responder
    mdns.begin(mdnsName); // start the multicast domain name server
    Serial.print("mDNS responder started: http://");
    Serial.print(mdnsName);
    Serial.println(".local");
    // mdns.addService("http", "tcp", 80);
}

void startServer()
{

    // Start a HTTP server with a file read handler and an upload handler
    routingSetup();

    server.begin(); // start the HTTP server
    Serial.println("HTTP server started.");
}

void handleFileUpload()
{ // upload a new file to the SPIFFS
    HTTPUpload &upload = server.upload();
    String path;
    if (upload.status == UPLOAD_FILE_START)
    {
        path = upload.filename;
        if (!path.startsWith("/"))
            path = "/" + path;
        if (!path.endsWith(".gz"))
        {                                     // The file server always prefers a compressed version of a file
            String pathWithGz = path + ".gz"; // So if an uploaded file is not compressed, the existing compressed
            if (SPIFFS.exists(pathWithGz))    // version of that file must be deleted (if it exists)
                SPIFFS.remove(pathWithGz);
        }
        Serial.print("handleFileUpload Name: ");
        Serial.println(path);
        fsUploadFile = SPIFFS.open(path, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
        path = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {                         // If the file was successfully created
            fsUploadFile.close(); // Close the file again
            Serial.print("handleFileUpload Size: ");
            Serial.println(upload.totalSize);
            server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
            server.send(303);
        }
        else
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{ // When a WebSocket message is received
    switch (type)
    {
    case WStype_DISCONNECTED: // if the websocket is disconnected
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    { // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    }
    break;
    case WStype_TEXT: // if new text data is received
        Serial.printf("[%u] get Text: %s\n", num, payload);
        break;
    }
}
void setup()
{
    Serial.begin(115200); // Start the Serial communication to send messages to the computer
    delay(10);
    Serial.println("\r\n");

    startWiFi();

    startSPIFFS();

    startWebSocket(); // Start a WebSocket server

    startMDNS();

    startServer();
}

int stationsCount = 0;
void loop()
{
    webSocket.loop();      // constantly check for websocket events
    server.handleClient(); // run the server

    if (startBlink)
    {
        blink();
        delay(2000);
    }
    //debug only
    if (stationsCount != WiFi.softAPgetStationNum())
    {
        stationsCount = WiFi.softAPgetStationNum();
        Serial.printf("Stations connected to soft-AP = %d\n", stationsCount);
    }
}
 