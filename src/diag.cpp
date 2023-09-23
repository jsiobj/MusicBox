#define DEBUG
#include "debug.h"

#include <SD.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_PN532.h>

#include "box.h"
#include "diag.h"

extern Box box;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_PN532 nfc;

void printDirectory(File dir, int numTabs, int max_depth, int current_depth = 0) {
    
    DEBUG_PRINTF("Base directory: %s", dir.name());

    while(true) {
        
        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files
            DEBUG_PRINT("No more files");
            break;
        }
        for (uint8_t i=0; i<numTabs; i++) {
            Serial.print('\t');
        }
        
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            if (current_depth <= max_depth) {
                printDirectory(entry, numTabs+1, max_depth, current_depth+1);
            }
        } else {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }

        entry.close();
    }
    dir.rewindDirectory();
}

TrellisCallback setColor(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            if(event.bit.NUM == 0)  {
                trellis.pixels.setPixelColor(event.bit.NUM,255,0,0);
                // Scanning for I2C devices
                // I2C addresses :
                // PN532      : 0x24
                // NeoTrellis : 0x2E
                // MAX9744    : 0x4B
                DEBUG_PRINT("Scanning I2C...");
                for (uint8_t addr = 0; addr<=127; addr++) {
                    //Serial.print("Trying I2C 0x");  Serial.println(addr,HEX);
                    Wire.beginTransmission(addr);
                    if (!Wire.endTransmission()) {
                        DEBUG_PRINTF("Found I2C %x",addr);
                    }
                }
                DEBUG_PRINT("Scanning done");
                break;
            }

            if(event.bit.NUM == 1) {
                if(box.sdreader_started) {
                    File root = SD.open("/");
                    printDirectory(root,0,0);
                }
                else {
                    Serial.println("SD reader disabled");
                }
            }
            trellis.pixels.setPixelColor(event.bit.NUM,255,255,0);
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.setPixelColor(event.bit.NUM,0);
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    trellis.pixels.show();
    return 0;
}

void Diag::begin() {

    DEBUG_PRINT("StartFunction");

    DEBUG_PRINTF("VS4053     : %s",box.vs1053_started ? "Started" : "No started");
    DEBUG_PRINTF("SD         : %s",box.sdreader_started ? "Started" : "No started");
    DEBUG_PRINTF("NeoTrellis : %s",box.neotrellis_started ? "Started" : "No started");
    DEBUG_PRINTF("NFC        : %s",box.rfid_started ? "Started" : "No started");
    DEBUG_PRINTF("MAX9744    : %s",box.max9744_started ? "Started" : "No started");

    if(box.neotrellis_started) {
        for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH,false);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_LOW,false);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
            trellis.registerCallback(i, setColor);
        }
        
        for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
            trellis.pixels.setPixelColor(i, 240, 20, 20);
            trellis.pixels.show();
            delay(50);
        }
    
        for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
            trellis.pixels.setPixelColor(i, 0x000000);
            trellis.pixels.show();
            delay(50);
        }

        trellis.pixels.show();
    }

    DEBUG_PRINT("ExitFunction");
}

// Main diag loop function
void Diag::loop() {
   
}