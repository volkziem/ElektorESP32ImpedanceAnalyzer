# ElektorESP32ImpedanceAnalyzer
ESP32 Impedance Analyzer for [Elektor magazine Jul/Aug 2023](https://www.elektormagazine.com/magazine/elektor-304/61865). Translations: [NL](https://www.elektormagazine.nl/magazine/elektor-307/61834), [DE](https://www.elektormagazine.de/magazine/elektor-306/61803), [FR](https://www.elektormagazine.fr/magazine/elektor-308/61767).

This repository contains the sources for the project.
- **nwa2.m** controls the ESP32 via the serial line from octave.
- **serialReadline.m** reads characters from the serial line until the end-of-line character '\n'.
- **nwa.py** controls the ESP32 via the serial line from Python3.
- **esp32_impedance_analyzer_serial** contains (Arduino-IDE) sources for the program that runs on the ESP32, which is controlled via the serial line.
- **esp32_impedance_analyzer_websockets** contains (Arduino-IDE) sources for the program that runs on the ESP32, which publishes the controlling webpage itself. The webpage is stored in the /data subdirectory inside. Don't forget to adapt the WLAN credentials.

Have fun!

## My github repositories for related projects
- for my book [A Hands-on Course in Sensors Using the Arduino and Raspberry Pi, 2nd ed.](https://github.com/volkziem/HandsOnSensors2ed);
- for the [Data Acquisition Pods](https://github.com/volkziem/EpicsDaqPods) using the ESP-01;
- for the [Geoscope](https://github.com/volkziem/GeoScope) to fingerprint vibrations;

## my webpage 
with more [gadgets](https://ziemann.web.cern.ch/ziemann/gadget/);

## on Elektor-labs
[ESP32-DAQ](https://www.elektormagazine.de/labs/esp32-daq-controlling-the-esp32-via-websockets-from-a-browser), an ESP32-based Oscilloscope.

