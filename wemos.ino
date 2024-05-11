#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
ESP8266WebServer server(80);
#else
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
WebServer server(80);
#endif

#define DBG_OUTPUT_PORT Serial

const char* ssid = "SkyNet1495";      // Название вашей WiFi-сети
const char* password = "9817428888"; // Пароль вашей WiFi-сети
File fsUploadFile;

void setup(void) {
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  
  // Инициализация SPIFFS
  SPIFFS.begin();
  
  // Инициализация Wi-Fi
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());
  
  // Начало работы с веб-сервером
  server.on("/", HTTP_GET, handleRoot);
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/upload", HTTP_POST, [](){
    server.send(200, "text/plain", "File uploaded successfully");
  }, handleFileUpload);
  server.on("/delete", HTTP_POST, [](){
    server.send(200, "text/plain", "File deleted successfully");
  }, handleFileDelete);
  server.on("/uploadpage", HTTP_GET, handleUploadPage);
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

void handleRoot() {
  String content = "<html><body>";
  content += "<h1>Welcome to SPIFFS File Manager</h1>";
  content += "<ul>";
  content += "<li><a href=\"/list\">List Files</a></li>";
  content += "<li><a href=\"/uploadpage\">Upload File</a></li>";
  content += "<li><form method='POST' action='/delete'><input type='text' name='filename' placeholder='Filename to delete'><input type='submit' value='Delete File'></form></li>";
  content += "</ul>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

void handleUploadPage() {
  String content = "<html><body>";
  content += "<h1>Upload File to SPIFFS</h1>";
  content += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
  content += "<input type='file' name='file'>";
  content += "<input type='submit' value='Upload'>";
  content += "</form>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

void handleFileList() {
  String output = "[";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    output += "{\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  output += "]";
  server.send(200, "application/json", output);
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: ");
    DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      DBG_OUTPUT_PORT.print("handleFileUpload Size: ");
      DBG_OUTPUT_PORT.println(upload.totalSize);
    }
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String filename = server.arg("filename");
  DBG_OUTPUT_PORT.println("handleFileDelete: " + filename);
  if (!SPIFFS.exists(filename)) {
    server.send(404, "text/plain", "FileNotFound");
    return;
  }
  SPIFFS.remove(filename);
  server.send(200, "text/plain", "File deleted successfully");
}
