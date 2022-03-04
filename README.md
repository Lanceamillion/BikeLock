# BikeLock
Tutorial to push new firmware to the bike lock.

### Download the software
To begin you will need to install the Arduino IDE (Integrated Development Environment) software on your computer. The download link can be found at [arduino.cc/en/software](https://www.arduino.cc/en/software).

### Downloading the repository
1. To download this repository navigate to 
[https://github.com/Lanceamillion/BikeLock](https://github.com/Lanceamillion/BikeLock) 
(you may already be here)
2. In the top right Code>Download ZIP
3. Extract the .zip to a convenient location (Maybe your desktop or documents folder)
4. Done!

### Upload
1. Navigate to **BikeLock>Lock>Lock.ino**
2. In the arduino IDE **File>Preferences>Additional Boards Manager URLs:** then paste "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json" into the box
3. Press OK
4. Now **Tools>Board: "...">Boards Manager...**
5. Once it loads search for "m0" that is "em *zero*" in the search box
6. Hover over **Adafruit SAMD Boards** by **Adafruit** and click install
7. Once the install is done click close and go **Tools>Board: "...">Adafruit Feather M0 (SAMD21)** and click on it
8. Plug in the lock
9. Now **Tools>Port:>COM?** Select the com port of the lock if you can't immediately tell what one it is try unplugging the device and figure out what port disappears
10. Look near the top of the code for the line **#define threshold ...** and change the value to desired (if this is what you want to do)
11. Finally click the arrow at the top left "upload"
12. Check the progress in the console at the bottom of the screen
13. Done!
