# ElektorESP32ImpedanceAnalyzer
ESP32 Impedance Analyzer for Elektor magazine. 

This repository contains the sources for the project.
1. **nwa2.m** controls the ESP32 via the serial line from octave.
2. **serialReadline.m** reads characters from the serial line until the end-of-line character '\n'.
3. **nwa.py** controls the ESP32 via the serial line from Python3.
4. **esp32_impedance_analyzer_serial** contains (Arduino-IDE) sources for the program that runs on the ESP32, which is controlled via the serial line.
5. **esp32_impedance_analyzer_websockets** contains (Arduino-IDE) sources for the program that runs on the ESP32, which publishes the controlling webpage itself. The webpage is stored in the /data subdirectory inside. Don't forget to adapt the WLAN credentials.

Have fun!
