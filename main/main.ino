#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include "config.h"

auto server = ESP8266WebServer(80);
String fileName = "";
String file = "";

Servo servo_z;
Servo servo_y;
const double y_servo_height = 90; //mm

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

void draw_svg(String svg){
  const int index = svg.indexOf("points=", svg.indexOf("<polyline")) + 8;
  String curNum;
  bool hasXCoord = false;
  double xCoord = 0;
  for(int i = index; i < svg.indexOf("\"", index); ++i){
    if (svg[i] == ' '){
      if (hasXCoord){
        Serial.print("Coordinates processed: ");
        Serial.print(xCoord);
        Serial.print(", ");
        Serial.println(curNum.toDouble());
        point_to_coords(xCoord, curNum.toDouble());
        delay(1000);
        hasXCoord = false;
      } else {
        hasXCoord = true;
        xCoord = curNum.toDouble();
      }
      curNum = "";
    } else curNum += svg[i];
  }
  Serial.print("Coordinates processed: ");
  Serial.print(xCoord);
  Serial.print(", ");
  Serial.println(curNum.toDouble());
  point_to_coords(xCoord, curNum.toDouble());
}

double process_angle(double rad){
  double output = rad*180/PI;
  while (output < 0){
    output += 360;
  }
  return output;
}

double z_angle(double x, double y){
  return process_angle(atan(y / x))/2;
}

double y_angle(double x, double y) {
  return process_angle(atan(sqrt(x*x + y*y) / y_servo_height))+3;
}

void point_to_coords(double x, double y){
  servo_z.write(z_angle(x, y));
  servo_y.write(y_angle(x, y));
}

void setup(){
  servo_z.attach(D5, 500, 2500);
  servo_y.attach(D6, 500, 2500);
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
      draw_svg(file);
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });
  server.begin();
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  // servo_z.write(z_angle(-100, 100));
  // servo_y.write(y_angle(-100, 100));
  point_to_coords(-100, 100);
}


void loop(){
  server.handleClient();
}