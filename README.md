# Laser Tag project

## Introduction

This project aims to create a laser tag system based around the ATMEGA328P
microcontroller. This is the one used in the Arduino UNO, however no Arduino
libraries are being used here.

## Building

A makefile is included in the `firmware` folder. The default `make` target
will generate `main.hex` and `main.eep` files to be uploaded to the device.
A `make program` target is provided to program the device using AVRDUDE. Make
sure to set the `AVRDUDE_PROGRAMMER` and `AVRDUDE_PORT` settings to match the
programmer you are using.
