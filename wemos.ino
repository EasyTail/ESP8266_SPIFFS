#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2SNoDAC.h>
я хочу чтоб я мог с главной страницы управлять всеми этими функциями
const char* ssid = "/////////////";      // Название вашей WiFi-сети
const char* password = "///////////";    // Пароль

ESP8266WebServer server(80);

AudioFileSourceSPIFFS audioFile("/test.mp3");
AudioGeneratorMP3 audioGenerator;
AudioOutputI2SNoDAC audioOutput;

void handleRoot() {
  server.send(200, "text/plain", "Hello from ESP8266!");
}

void handlePlay() {
  audioGenerator.begin(&audioFile, &audioOutput);
  while (audioGenerator.isRunning()) {
    audioGenerator.loop();
  }
  server.send(200, "text/plain", "Playing audio...");
}

void setup() {
  Serial.begin(115200);

  // Подключение к Wi-Fi сети
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Инициализация файловой системы
  SPIFFS.begin();

  // Настройка маршрутов для веб-интерфейса
  server.on("/", handleRoot);
  server.on("/play", handlePlay);
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
