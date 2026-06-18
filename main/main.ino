/*
Laser Pointer SVG Tracer
Made by Luke Powell on June 16th, 2026
Purpose:
Hosts a webserver on an ESP where users can upload SVG files.
Controls 2 servos to trace the SVG on the ground with an attached laser pointer.
*/

// ESP WiFi Library
#include <ESP8266WiFi.h>
// ESP Web Server Library
#include <ESP8266WebServer.h>
// Micro servo library
#include <Servo.h>
// Config for WiFi network SSID and password
#include "config.h"

// Initializes the web server object
auto server = ESP8266WebServer(80);
// Holds the name of the uploaded file
String fileName = "";
// Stores the contents of the uploaded file
String file = "";

// Servo object for the bottom servo (Z axis)
Servo servo_z;
// Servo object for the upper servo (Y axis)
Servo servo_y;
// Stores the height of the centre of the upper servo's horn in milimeters
const double y_servo_height = 70;

// Returns the HTML for the user interface website
String html(){
  String output = ""
  "<html>"
  "<body>"
  "<form action='/upload' method='POST' enctype='multipart/form-data'>"
  "<input type='file' name='file' required>"
  "<input type='submit' value='Upload'>"
  "</form>";
  // Inserts the name of the last uploaded file
  output += fileName == "" ? "No file uploaded" : "File name: " + fileName;
  output += ""
  "<br><br>"
  "<button> <a href='/redo' style='color: black; text-decoration: none;'> Redo Trace </a> </button>"
  "</body>"
  "</html>";
  return output;
}

// Takes a string of SVG code and draws it using the laser pointer
void draw_svg(String svg){
  // Saves the index of where the points of the path start
  const int index = svg.indexOf("points=", svg.indexOf("<polyline")) + 8;
  // Stores the string of the current number being parsed
  String curNum;
  // Stores true if the X coordinate of the current point has already been processed, false otherwise
  bool hasXCoord = false;
  // Stores true if the current point is the first one processed
  bool first = true;
  // Store the coordinates of the last point
  double lastX;
  double lastY;
  // Stores the current X coordinate
  double xCoord = 0;

  // Loops through the characters of the points list section of the string
  for(int i = index; i < svg.indexOf("\"", index); ++i){
    if (svg[i] == ' '){
      // If the current character is space, processes the next number

      if (hasXCoord){
        // If this is the Y coordinate, moves the laser pointer to that point

        Serial.print("Coordinates processed: ");
        Serial.print(xCoord);
        Serial.print(", ");
        Serial.println(curNum.toDouble());
        if (first){
          // If this is the first coordinate, moves the laser pointer directly there

          point_to_coords(-xCoord*0.353, (1100-curNum.toDouble())*0.353);
          // Delays for 1s
          delay(1000);
          // It's no longer the first one
          first = false;
        } else {
          // If this is not the first coordinate, moves the laser pointer there in 20 steps over 0.6s

          // Calculates the X and Y steps by dividing the difference by 20
          const int stepX = (-xCoord*0.353 - lastX)/20;
          const int stepY = ((1100-curNum.toDouble())*0.353 - lastY)/20;

          // Loops from 1 to 20 and moves to the corresponding step each time
          for (int i = 1; i <= 20; i++){
            point_to_coords(lastX + stepX*i, lastY + stepY*i);
            delay(30);
          }

          // Waits half a second at vertices
          delay(500);
        }

        // Updates the last coordinate values
        lastX = -xCoord*0.353;
        lastY = (1100-curNum.toDouble())*0.353;

        // The X coordinate is next
        hasXCoord = false;
      } else {
        // If this is the X coordinate, saves it and moves onto the Y coordinate

        hasXCoord = true;
        xCoord = curNum.toDouble();
      }

      // When a number is parsed, resets the curNum string
      curNum = "";
    } else 
    // Appends the current character to the curNum string
      curNum += svg[i];
  }

  Serial.print("Coordinates processed: ");
  Serial.print(xCoord);
  Serial.print(", ");
  Serial.println(curNum.toDouble());

  // Calculates the X and Y steps by dividing the difference by 20
  const int stepX = (-xCoord*0.353 - lastX)/20;
  const int stepY = ((1100-curNum.toDouble())*0.353 - lastY)/20;
  
  // Loops from 1 to 20 and moves the laser pointer to the corresponding step each time
  for (int i = 1; i <= 20; i++){
    point_to_coords(lastX + stepX*i, lastY + stepY*i);
    delay(30);
  }
}

// Converts an angle from radians to an angle in degrees from 0 to 360
double process_angle(double rad){
  // Converts to degrees
  double output = rad*180/PI;
  // Uses the co-terminal angle between 0 and 360
  while (output < 0){
    output += 360;
  }
  // Returs the processed angle
  return output;
}

// Calculates the angle for the Z axis servo to point to the given coordinates
double z_angle(double x, double y){
  // Uses inverse tangent ratio between the Y and X coordinates
  return process_angle(atan2(y, x))/2;
}

// Calculates the angle for the Y axis servo to point to the given coordinates
double y_angle(double x, double y) {
  // Uses inverse tangent ratio between the XY plane distance and the Y servo's height
  return process_angle(atan2(sqrt(x*x + y*y), y_servo_height));
}

// Points both servos to the given coordinates
void point_to_coords(double x, double y){
  // Prints the current plane coordinates
  Serial.print("Pointing to: ");
  Serial.print(x);
  Serial.print(", ");
  Serial.println(y);
  // Prints the Z and Y angles
  Serial.print("Z angle: ");
  Serial.print(z_angle(x, y));
  Serial.print(", Y angle: ");
  Serial.println(y_angle(x,y));

  // Sets the Z servo to the Z angle and the Y servo to the Y angle
  servo_z.write(z_angle(x, y));
  servo_y.write(y_angle(x, y));
}

// Runs once when the code starts
void setup(){
  // Sets up the Z axis servo on the D5 pin and the Y axis servo on the D6 pin
  servo_z.attach(D5, 600, 2400);
  servo_y.attach(D6, 550, 2450);

  // Sets up the pin connected to the onboard LED for output mode
  pinMode(D0, OUTPUT);

  // Initializes the serial connection
  Serial.begin(115200);

  // Begins connecting to WiFi as configured in config.h
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Blinks the onboard LED while WiFi is connecting
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(D0, LOW);
    delay(800);
    digitalWrite(D0, HIGH);
    delay(100);
  }

  // Turns on onboard LED after WiFi has connected
  digitalWrite(D0, LOW);
  
  // Sends the website HTML at the root address of the website
  server.on("/", [](){server.send(200, "text/html", html());});

  // Handles upload of SVG files
  server.on("/upload", HTTP_POST, [](){Serial.println("Uploaded");}, [](){
    if (server.upload().status == UPLOAD_FILE_START){
      // If this event is the start of the upload, saves the file name and resets the file variable

      Serial.println(server.upload().filename);
      fileName = server.upload().filename;
      file = "";

    } else if (server.upload().status == UPLOAD_FILE_WRITE){
      // If this event is file text, adds it to the file object

      // Loops through all bytes in the buffer
      for (auto e : server.upload().buf){
        // If the character from the current byte is not a new line, appends it to the file variable
        if ((char)e != '\n'){
          file += (char)e;
        }
      }
    } else if (server.upload().status == UPLOAD_FILE_END){
      // If this event is the end of the file upload, commences SVG tracing

      // Prints the contents of the file to the serial monitor
      Serial.print("File: '");
      Serial.print(file);
      Serial.println("'");

      // Traces the file
      draw_svg(file);

      // Redirects the user to the home page
      server.sendHeader("Location", "/");
      server.send(303);
    }
  });

  // When the user presses the redo button, runs the trace again and redirects to the home page
  server.on("/redo", [](){
    draw_svg(file);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  // Starts up the server
  server.begin();

  // Prints the local IP address to the serial monitor
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Points to 100, 100 mm for calibration
  point_to_coords(100, 100);
}

// Runs indefinitely
void loop(){
  // Handles incoming web requests to the server
  server.handleClient();
}