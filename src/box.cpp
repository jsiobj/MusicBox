/*
    Interacting Objects - Noe's Juke Box Project
    http://www.interactingobjects.com

    Project page : http://interactingobjects.com/category/projects/juke-box/

    ---------------------------------------------------------------------------
    The MIT License (MIT)

    Copyright (c) 2014 Interacting Objects (http://interactingobjects.com)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    ---------------------------------------------------------------------------

*/

#define DEBUG
#include "debug.h"

// #include <TimerOne.h>
// #include <MMA8453_n0m1.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_NeoTrellis.h>
#include <PN532.h>

#include "box.h"
extern Box box;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_VS1053_FilePlayer vs1053FilePlayer;
extern PN532 nfc;


TrellisCallback setMode(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            if(event.bit.NUM < BOX_MODE_COUNT) {
                trellis.pixels.setPixelColor(event.bit.NUM,COLOR_GREEN);
                trellis.pixels.show();
                box.boxMode = event.bit.NUM;
                DEBUG_PRINTF("Mode set to %d",event.bit.NUM);
            }
            else {
                trellis.pixels.setPixelColor(event.bit.NUM,COLOR_RED);
                trellis.pixels.show();
                DEBUG_PRINTF("No mode affected to key %d",event.bit.NUM);
            }
            delay(500);
            break;

        case SEESAW_KEYPAD_EDGE_FALLING:
            if(event.bit.NUM < BOX_MODE_COUNT) {
                trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLUE);
                trellis.pixels.show();
                box.boxMode = event.bit.NUM;
            }
            else {
                trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLACK);
                trellis.pixels.show();
            }

            break;

        default:
            break;
    }


    return 0;
}

//=================================================================================
// Doing all configuration
//=================================================================================
void Box::begin() {
    DEBUG_PRINT("StartFunction");

    if(box.neotrellis_started) {
        DEBUG_PRINT("Initializing NeoTrellis");
        for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH, false);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_LOW, false);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
            trellis.unregisterCallback(i);
            trellis.registerCallback(i, setMode);
        }

        for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++) {
            if(i<BOX_MODE_COUNT) trellis.pixels.setPixelColor(i,COLOR_BLUE);
            else                 trellis.pixels.setPixelColor(i,COLOR_BLACK);
        }
        trellis.pixels.show();
    }

    if(box.vs1053_started) setVolume(0);

}

void Box::selectMode() {

    // Wait until key box mode is set (by pressing an allowed key)
    while(boxMode == BOX_MODE_UNDEF) {
        trellis.read();
    }

    return;
}

bool Box::setVolume(int8_t v) {
    DEBUG_PRINTF("Setting volume to %d",v);
    
    Wire.beginTransmission(Max9744i2cAddr);
    Wire.write(v);
    
    if (Wire.endTransmission() == 0) {
        volume = v;
        return true;
    }
    else {
        return false;
    }
}

bool Box::increaseVolume() {
    DEBUG_PRINTF("Increasing volume to %d",volume+1);
    if(volume >= 63) return setVolume(63);
    else             return setVolume(volume+1);
}

bool Box::decreaseVolume() {
    DEBUG_PRINTF("Setting volume to %d",volume-1);
    if(volume <=0) return setVolume(0);
    else           return setVolume(volume-1);
}

void intArrayToHexString(uint8_t * array, uint8_t length, char* hexString) {
    int i;
    for (i = 0; i < length; i++) {
        sprintf(hexString + 2*i, "%02X", array[i]); // Append 2 hexadecimal characters for each integer
    }
    hexString[2*length] = '\0'; // Terminate the string with null character
}

boolean Box::readRFID() {
    
    boolean success;
    uint8_t uidLength;
    
    if(millis() - rfidLastRead < RFID_READ_INTERVAL)  {
        DEBUG_PRINT("Last RFDI read was to close, ignoring")    
        return false;
    }

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &rfidUid[0], &uidLength);
    if (success) {
        char uidStr[15];
        intArrayToHexString(rfidUid,uidLength,uidStr);
        DEBUG_PRINTF("Card detected, UID Length: %d bytes,  value: %s", uidLength, uidStr);
        rfidLastRead = millis();
    }

    return success;
}
