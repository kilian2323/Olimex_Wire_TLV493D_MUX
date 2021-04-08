# Olimex_Wire_TLV493D_MUX
TwoWire and TLV493D library ported from Arduino to Olimex (Linux), with example.
Created by Kilian Ochs, Centre for Biorobotics, April 2021

## Purpose
This repository contains a very simple version of the Arduino TwoWire library (known as "Wire") which works on Linux-based platforms.
This code was tested on the Olimex A64 OLinuXino single-board computer which provides an I²C interface on the UEXT1 port.

## General note
All Arduino libraries which use the I²C bus via the TwoWire interface library can be ported to Linux using the provided replacements "Wire.h" and "Wire.cpp", but in some cases, amendments might be necessary.
Some Arduino-specific functions (such as delay()) are replaced by Linux-compatible functions in "Olimexino.h" and "Olimexino.cpp". Also added is support to print a char array with known size to the console. All these libraries facilitate the transfer of Arduino-based I²C libraries to Linux, and they are placed in the "olimex" folder.
Using these librraies, Only minimal changes to the specific driver libraries for Arduino should be necessary in order to port them to Linux.
The ported library for TLV493D and its utilities are in the "src" folder and its subfolder "util".
The multiplexer TCA9548A which is used for being able to interface more than one (two, actually) sensors TLV493D does not require a driver library and relies only on the "Wire" libraries.

## Usage
Connect one sensor TLV493D to channel 0 of the multiplexer TCA9548A. Then connect the multiplexer's main bus to the Olimex I²C interface. Run the output file "./build/Sensor_MUX".
The program will initialize the sensor, set the channel of the multiplexer and then print the magnetic flux values measured along X, Y and Z. These values can be understood as Cartesian coordinates of the magnet's location relative to the sensor.
To switch debug messages on, locate the file "config.h" in the "olimex" folder and uncomment the #define DEBUG statement. 

## Build
In case a re-compilation becomes necessary, a Makefile is included, so it's enough to just run "make" in the root directory.

## TODO
- Implement Serial output via Olimex
- Add loop frequency counter
- Add hardware-interrupt-driven loop timer
- Find a cleaner solution for the problem of sensors sporadically not being correctly initialized and therefore giving only 0 values; this would eliminate the function testAndReinitialize()
- Turn it into a ROS node
