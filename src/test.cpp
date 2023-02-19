#define DEBUG
#include "debug.h"

#include <SD.h>
#include "Adafruit_NeoTrellis.h"

#include <PN532_SPI.h>
#include <PN532.h>

#include "box.h"
#include "test.h"

extern Box box;
extern Adafruit_NeoTrellis trellis;
extern PN532 nfc;

void printDirectory(File dir, int numTabs) {
    
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
            printDirectory(entry, numTabs+1);
        } else {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }

        entry.close();
    }
}

TrellisCallback setColor(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
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

void Test::begin() {

    DEBUG_PRINT("StartFunction");

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

    File root = SD.open("/MUSIC1");
    printDirectory(root,0);

    DEBUG_PRINT("ExitFunction");
}

void Test::loop() {

    DEBUG_PRINT("StartFunction");
    DEBUG_PRINT("Checking keyboard and encoder, press keys to test or turn encoder to test...");

    // Looping forever... until reset !
    while(1) {
        delay(100);
        trellis.read();
        box.readRFID();
    }
    
}