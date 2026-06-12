#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"

auto server = ESP8266WebServer(80);
String fileName = "";
String file = "";

String html(){
  String output = ""
  "<html>"
  "<body>"
  "<form action='/upload' method='POST' enctype='multipart/form-data'>"
  "<input type='file' name='file' required>"
  "<input type='submit' value='Upload'>"
  "</form>";
  output += fileName == "" ? "No file uploaded" : "File name: " + fileName;
  output += ""
  "</body>"
  "</html>";
  return output;
}

void setup(){
  pinMode(D0, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(D0, LOW);
    delay(800);
    digitalWrite(D0, HIGH);
    delay(100);
  }
  digitalWrite(D0, LOW);
  server.on("/", [](){server.send(200, "text/html", html());});
  server.on("/upload", HTTP_POST, [](){server.send(200);Serial.println("test");}, [](){
    if (server.upload().status == UPLOAD_FILE_START){
      Serial.println(server.upload().filename);
      fileName = server.upload().filename;
      file = "";
    } else if (server.upload().status == UPLOAD_FILE_WRITE){
      for (auto e : server.upload().buf){
        if ((char)e != '\n'){
          file += (char)e;
        }
      }
    } else if (server.upload().status == UPLOAD_FILE_END){
      Serial.print("File: '");
      Serial.print(file);
      Serial.println("'");
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });
  server.begin();
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop(){
  server.handleClient();
  // Serial.print("file name: '");
  // Serial.print(fileName);
  // Serial.println("'");
  // Serial.print("html: '");
  // //Serial.print(html());
  // Serial.println("'");
}