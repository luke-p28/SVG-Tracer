# Laser Pointer SVG Tracer

## Purpose
Hosts a web server on an ESP where users can upload SVG files.
Controls 2 servos to trace the SVG on the ground with an attached laser pointer.

## Assembly
This project consists of 3D-printed and electrical parts. The servo mount is 3D-printed and assembled as shown here.

<img width="400" height="300" alt="image" src="https://github.com/user-attachments/assets/8ee71e08-da8f-4f60-8b2f-274847390cb2" />

The 2 9G micro servos are lightly hot-glued into their respective pockets, and the two gears are hot-glued to the servo horns.

Note: if anyone actually wants to build this, they would likely have to design their own laser pointer mount, since I just used a random laser pointer I had lying around and have no clue what brand it is.

## Wiring
The positive pins on the 9G servos are connected to pin wires, which are soldered to the positive terminal of a 3x AA battery mount. The negative pins are likewise connected to the negative terminal of the battery mount, but are also connected to the ground pin of a NodeMCU 1.0 ESP-12E module. The Z-axis servo (the lower one) has its PWM pin connected to D5 on the ESP, while the Y-axis servo (the upper one) is connected to the D6 pin.

## Config
The code uses a config.h file for WiFi credentials. It should declare two constant String variables, one named 'ssid' and one named 'password', and be placed next to main.ino in the main directory.
