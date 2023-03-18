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

#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

//#include <PN532_SPI.h>
//#include <PN532_I2C.h>
#include <PN532_HSU.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include "debug.h"
#include "box.h"
#include "test.h"
#include "player.h"

// VS1053 shield pins
#define VS_SHIELD_CS      6      // VS1053 chip select pin (output) MP3 CS 
#define VS_SHIELD_XDCS    10     // VS1053 Data/command select pin (output) 
#define VS_SHIELD_SDCS    5      // VS1053 shield SD card chip select pin (SDCS ?)
#define VS_SHIELD_DREQ    9      // VS1053 Data request (int pin)
#define VS_SHIELD_RESET   11

#define PN532_CS          12
//#define PN532_IRQ         13

Adafruit_NeoTrellis trellis;
Adafruit_VS1053_FilePlayer vs1053FilePlayer = Adafruit_VS1053_FilePlayer(VS_SHIELD_RESET,VS_SHIELD_CS,VS_SHIELD_XDCS,VS_SHIELD_DREQ,VS_SHIELD_SDCS);
//PN532_SPI pn532spi(SPI, PN532_CS);
//PN532 nfc(pn532spi);

PN532_HSU pn532(Serial1);
PN532 nfc(pn532);

Box box;
Test test;
MusicPlayer musicPlayer;

//=================================================================================
// Callbacks / IRQ vectors
//=================================================================================

//---------------------------------------------------------------------------------
// If any key is pressed at startup, go into test mode
//---------------------------------------------------------------------------------
TrellisCallback setTestMode(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_HIGH:
            for(int i=0;i<5;i++) {
                trellis.pixels.setPixelColor(event.bit.NUM,255,0,0);
                trellis.pixels.show();
                delay(500);
                trellis.pixels.setPixelColor(event.bit.NUM,0,0,0);
                trellis.pixels.show();
                delay(500);
            }

            box.boxMode = BOX_MODE_TEST;
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            DEBUG_PRINT("Entering test mode")
            break;
        
        default:
            break;
    }
    return 0;
}

//=================================================================================
// SETUP
//=================================================================================
void setup() {

    Serial.begin(115200);
    while(!Serial);
    DEBUG_PRINT("StartFunction");

    Serial1.begin(115200);

    // Starting SD Reader
    if (!SD.begin(VS_SHIELD_SDCS)) {
        DEBUG_PRINT("SD failed or not present");
    }
    else {
        DEBUG_PRINT("SD Card started");
        box.sdreader_started = true;
    }

    // Starting VS1053 player
    if (!vs1053FilePlayer.begin()) { 
        DEBUG_PRINT("Could not start VS1053");
    }
    else {
        DEBUG_PRINT("VS1053 started");
        box.vs1053_started = true;
    }

    // Starting RFID Reader
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
        DEBUG_PRINT("Could not start PN532 board");
        //while (1) delay(1); // halt
    }
    else {
        DEBUG_PRINTF("PN5-%x version %x", (versiondata>>24) & 0xFF,(versiondata>>16) & 0xFF); 
        //attachInterrupt(digitalPinToInterrupt(PN532IRQPIN), cardreading, FALLING);
        //nfc.setPassiveActivationRetries(0x02);
        nfc.SAMConfig(); // configure board to read RFID tags
        box.rfid_started = true;
    }

    // Starting Trellis
    if (!trellis.begin()) {
        DEBUG_PRINT("Could not start NeoPixel Trellis");
    } else {
        DEBUG_PRINT("NeoPixel Trellis started");
        box.neotrellis_started = true;
        // First, check if test mode was requested (ie a key is pressed at startup)
        // If no key was pressed before reaching here
        // We'll go on in "normal" mode
        for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
            trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH);
            trellis.registerCallback(i, setTestMode);
        }
        trellis.read();
    }

    box.boxMode = BOX_MODE_TEST;

    // Init box object
    box.begin();
}

//=================================================================================
// MAIN LOOP
//=================================================================================
void loop() {
    DEBUG_PRINT("StartFunction");

    // if no mode selected
    if(box.boxMode == BOX_MODE_UNDEF) {
        box.selectMode();
    }

    // Once mode is selected, starting the right loop, depending on mode
    switch (box.boxMode) {

        case BOX_MODE_TEST:
            DEBUG_PRINT("Test mode");
            test.begin();
            test.loop();
            break;    

        case BOX_MODE_PLAYER:
            DEBUG_PRINT("Player mode");
            musicPlayer.begin();
            musicPlayer.loop();
            break;    

        case BOX_MODE_PIANO:
            DEBUG_PRINT("Piano mode");
            DEBUG_PRINT("Not implemenented yet...");
            box.boxMode = BOX_MODE_UNDEF;
            break;    

        case BOX_MODE_GAME:
            DEBUG_PRINT("Player mode");
            DEBUG_PRINT("Not implemenented yet...");
            box.boxMode = BOX_MODE_UNDEF;
            break;    

        default:
            DEBUG_PRINT("Ooops... unknown mode ! Restarting.");
            box.boxMode = BOX_MODE_UNDEF;
            break;
    }
}
