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

#define PREFER_SDFAT_LIBRARY 1
#include "debug.h"

#include <Adafruit_VS1053.h>
#include <Adafruit_PN532.h>

#include "Adafruit_NeoTrellis.h"
#include "box.h"

extern Box box;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_VS1053_FilePlayer vs1053FilePlayer;
extern Adafruit_PN532 nfc;

// Button used as mode selector
// Set to 255 if not used
uint8_t mapButton2mode[] = { 
    BOX_MODE_PLAYER, BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF,
     BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF,
     BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF,
     BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF, BOX_MODE_UNDEF
}; 

TrellisCallback setMode(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);

    if(event.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
        if(mapButton2mode[event.bit.NUM] != BOX_MODE_UNDEF) {
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_GREEN);
            trellis.pixels.show();
            box.boxMode = mapButton2mode[event.bit.NUM];
            DEBUG_PRINTF("Mode set to %d",box.boxMode);
        }
        else {
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_RED);
            trellis.pixels.show();
            DEBUG_PRINTF("No mode affected to key %d",event.bit.NUM);        
        }
    }

    if(event.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
        trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLACK);
        trellis.pixels.show();
    }
    return 0;
}

//---------------------------------------------------------------------------------
// Doing Box configuration
//---------------------------------------------------------------------------------
void Box::begin() {
    DEBUG_PRINT("StartFunction");

    #ifdef DEBUG
    mapButton2mode[NEO_TRELLIS_NUM_KEYS-1] = BOX_MODE_DIAG;
    #endif

    if(box.neotrellis_started) {
        trellis.pixels.setBrightness(16);
        DEBUG_PRINT("Initializing NeoTrellis");
        for(uint16_t key=0; key<NEO_TRELLIS_NUM_KEYS; key++){
            switch (mapButton2mode[key])
            {
                case BOX_MODE_UNDEF:
                    break;
                
                case BOX_MODE_DIAG:
                    trellis.pixels.setPixelColor(key,COLOR_RED);
                    boxModeCount++;
                    break;

                default:
                    trellis.pixels.setPixelColor(key,COLOR_GREEN);
                    boxModeCount++;
                    break;
            }

            trellis.activateKey(key, SEESAW_KEYPAD_EDGE_HIGH, false);
            trellis.activateKey(key, SEESAW_KEYPAD_EDGE_LOW, false);
            trellis.activateKey(key, SEESAW_KEYPAD_EDGE_FALLING);
            trellis.activateKey(key, SEESAW_KEYPAD_EDGE_RISING);
            trellis.unregisterCallback(key);
            trellis.registerCallback(key, setMode);
        }
        trellis.pixels.show();
    }
}

//---------------------------------------------------------------------------------
// Box loop
//---------------------------------------------------------------------------------
void Box::loop() {
    if(box.neotrellis_started) trellis.read();
    //if(box.rfid_started)  checkNFC();
}

//---------------------------------------------------------------------------------
// Selecting box mode
//---------------------------------------------------------------------------------
void Box::selectMode() {
    
    // Only one mode... not much of a choice...
    if(boxModeCount == 1) {
        boxMode = 0;
    }
    // If more than one mode, wait until key box 
    // mode is set (by pressing an allowed key)
    else {
        while(boxMode == BOX_MODE_UNDEF) {
            trellis.read();
        }
    }
}

//---------------------------------------------------------------------------------
// Setting MAX9744 amp volume
//---------------------------------------------------------------------------------
bool Box::max9744SetVolume(uint8_t v) {
    DEBUG_PRINTF("Setting volume to %d",v);
    Wire.beginTransmission(Max9744i2cAddr);
    Wire.write(v);
    uint8_t rc = Wire.endTransmission();
    if ( rc == 0) {
        max9744_volume = v;
        return true;
    }
    else {
        DEBUG_PRINTF("Could not set volume, rc was : %d", rc)
        return false;
    }
}

//---------------------------------------------------------------------------------
// Increase MAX9744 amp volume
//---------------------------------------------------------------------------------
bool Box::max9744IncreaseVolume() {
    DEBUG_PRINTF("Increasing volume to %d",max9744_volume+1);
    if(max9744_volume >= 63) return max9744SetVolume(63);
    else             return max9744SetVolume(max9744_volume+1);
}

//---------------------------------------------------------------------------------
// Decrease MAX9744 amp volume
//---------------------------------------------------------------------------------
bool Box::max9744DecreaseVolume() {
    DEBUG_PRINTF("Setting volume to %d",max9744_volume-1);
    if(max9744_volume <=0) return max9744SetVolume(0);
    else           return max9744SetVolume(max9744_volume-1);
}

//---------------------------------------------------------------------------------
// Setting VS1053 amp volume
// WARNING : Higher value means lower volume
//---------------------------------------------------------------------------------
void Box::vs1053SetVolume(uint8_t v) {
    DEBUG_PRINTF("StartFunction, volume:%d", v);
    if(v < VS1053_MAX_VOLUME) v = VS1053_MAX_VOLUME; 
    if(v > VS1053_MIN_VOLUME) v = VS1053_MIN_VOLUME;
    vs1053FilePlayer.setVolume(v, v);     // Left and right channel volume (lower number mean louder)
    vs1053_volume = v;
    DEBUG_PRINTF("Volume set to %d", vs1053_volume);
}

//---------------------------------------------------------------------------------
// Array to hex string helper 
//---------------------------------------------------------------------------------
void intArrayToHexString(uint8_t * array, uint8_t length, char* hexString) {
    int i;
    for (i = 0; i < length; i++) {
        sprintf(hexString + 2*i, "%02X", array[i]); // Append 2 hexadecimal characters for each integer
    }
    hexString[2*length] = '\0'; // Terminate the string with null character
}

// boolean Box::nfcReadId() {
    
//     boolean success;
//     uint8_t uidLength;

//     //DEBUG_PRINT("Reading tag...");
    
//     if(millis() - nfcLastRead < RFID_READ_INTERVAL)  {
//         DEBUG_PRINT("Last RFDI read was to close, ignoring")    
//         return false;
//     }

//     success = nfc.readDetectedPassiveTargetID(rfidUid, &uidLength);
//     //success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A,rfidUid, &uidLength);
//     if (success) {
//         char uidStr[15];
//         intArrayToHexString(rfidUid,uidLength,uidStr);
//         DEBUG_PRINTF("Card detected, UID Length: %d bytes,  value: %s", uidLength, uidStr);
//         nfc.inRelease();
//         nfcLastRead = millis();
//     }
//     else {
//         DEBUG_PRINTF("Failed to read card");
//         nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
//     }

    
//     return success;
// }

// void Box::checkNFC() {

//     if (readerDisabled) {
//         if (millis() - nfcLastRead > DELAY_BETWEEN_CARDS) {
//             readerDisabled = false;
//             irqPrev = irqCurr = HIGH;
//             DEBUG_PRINT("Waiting for an ISO14443A Card ...");
//             nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);            
//         }
//     } 
//     else {
//             irqCurr = digitalRead(PN532_IRQ);

//             // When the IRQ is pulled low - the reader has got something for us.
//             if (irqCurr == LOW && irqPrev == HIGH) {
//             DEBUG_PRINT("Got NFC IRQ");
//             readNFC();
//         }

//         irqPrev = irqCurr;
//     }    
// }

// void Box::readNFC() {
//     uint8_t success = false;
//     uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
//     uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

//     // read the NFC tag's info
//     success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
//     if (success) {
//         char uidStr[15];
//         intArrayToHexString(uid,uidLength,uidStr);
//         DEBUG_PRINTF("Card detected, UID Length: %d bytes,  value: %s", uidLength, uidStr);
//         nfcLastRead = millis();
//     }
//     else {
//         DEBUG_PRINT("Failed to read card");
//         nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
//     }

//     // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
//     readerDisabled = true;
// }