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

#include <Adafruit_VS1053.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_PN532.h>

#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
//#include <flash_config.h>

#include "debug.h"
#include "box.h"
#include "diag.h"
#include "player.h"

// VS1053 shield pins
#define VS_SHIELD_CS      6      // VS1053 chip select pin (output) MP3 CS 
#define VS_SHIELD_XDCS    10     // VS1053 Data/command select pin (output) 
#define VS_SHIELD_SDCS    5      // VS1053 shield SD card chip select pin (SDCS ?)
#define VS_SHIELD_DREQ    9      // VS1053 Data request (int pin)
#define VS_SHIELD_RESET   11

Adafruit_NeoTrellis trellis;
Adafruit_VS1053_FilePlayer vs1053FilePlayer = Adafruit_VS1053_FilePlayer(VS_SHIELD_RESET,VS_SHIELD_CS,VS_SHIELD_XDCS,VS_SHIELD_DREQ,VS_SHIELD_SDCS);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

Box box;
Diag diag;
MusicPlayer musicPlayer;
SdFat SD;
Adafruit_SPIFlash onboardStorage(&flashTransport);
FatVolume onboardFS;

//=================================================================================
// Callbacks / IRQ vectors
//=================================================================================

//---------------------------------------------------------------------------------
// If any key is pressed at startup, go into test mode
//---------------------------------------------------------------------------------
// TrellisCallback setTestMode(keyEvent event) {
//     DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
//     switch (event.bit.EDGE) {
//         case SEESAW_KEYPAD_EDGE_HIGH:
//             for(int i=0;i<5;i++) {
//                 trellis.pixels.setPixelColor(event.bit.NUM,0xFF0000);
//                 trellis.pixels.show();
//                 delay(500);
//                 trellis.pixels.setPixelColor(event.bit.NUM,0x0);
//                 trellis.pixels.show();
//                 delay(500);
//             }

//             box.boxMode = BOX_MODE_DIAG;
//             DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
//             DEBUG_PRINT("Entering test mode")
//             break;
        
//         default:
//             break;
//     }
//     return 0;
// }

//------------------------------------------------------------------------------
void printCardType() {

  switch (SD.card()->type()) {
      case SD_CARD_TYPE_SD1:
          DEBUG_PRINT("SD started (SD1)");
            break;

    case SD_CARD_TYPE_SD2:
        DEBUG_PRINT("SD started (SD2)");
        break;

    case SD_CARD_TYPE_SDHC:
            DEBUG_PRINT("SD started (SDHC/SDXC)");
        break;

    default:
        DEBUG_PRINT("SD started (Unknown)");
  }
}

void i2cScan() {
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
}

void startSD() {
    // Starting SD Reader
    #define SD_CONFIG SdSpiConfig(VS_SHIELD_SDCS, SHARED_SPI, SD_SCK_MHZ(4))
    if (!SD.cardBegin(SD_CONFIG)) {
        SD.initErrorHalt(&Serial);
        DEBUG_PRINT("SD failed or not present");
        return;
    }

    if(!SD.volumeBegin()) {
        DEBUG_PRINTF("Could not start SD Volume (error code: %d)", SD.sdErrorCode());
        return;
    }

    printCardType();
    box.sdreader_started = true;
}

void startVS1053() {
    // Starting VS1053 player
    if (!vs1053FilePlayer.begin()) { 
        DEBUG_PRINT("Could not start VS1053");
        return;
    }

    DEBUG_PRINT("VS1053 started");
    box.vs1053SetVolume(VS1053_DEFAULT_VOLUME); // Setting to max, will use MAX9477 volume
    box.vs1053_started = true;
}

void startTrellis() {
    DEBUG_PRINT("Starting trellis");

    if (!trellis.begin()) {
        DEBUG_PRINT("Could not start NeoPixel Trellis");
        return;
    }

    DEBUG_PRINT("NeoPixel Trellis started");
    box.neotrellis_started = true;
    for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_HIGH);
    }
    trellis.read();
}

void startMAX9744() {
    if (! box.max9744SetVolume(MAX9744_DEFAULT_VOLUME)) {
        Serial.println("Failed to set volume, MAX9744 not found!");
        return;
    }

    DEBUG_PRINT("MAX9744 Amplifier started");
    box.max9744_started = true;
}

void startOnboardStorage() {
    onboardStorage.begin();
    if (!onboardFS.begin(&onboardStorage,true)) {
        DEBUG_PRINT("Failed to start on board storage");
        box.onboardStorage_started = false;
    }
    else {
        DEBUG_PRINT("On board storage started");
        box.onboardStorage_started = true;
    }
}

// void startNFC() {
//     nfc.begin();
//     uint32_t versiondata = nfc.getFirmwareVersion();
//     if (! versiondata) {
//         DEBUG_PRINT("Could not start PN532 board");
//         return;
//     }

//     //attachInterrupt(digitalPinToInterrupt(PN532_IRQ), cardreading, FALLING);
//     DEBUG_PRINTF("PN5-%lx version %lx", (versiondata>>24) & 0xFF,(versiondata>>16) & 0xFF); 
//     //nfc.SAMConfig();
//     nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);            
//     box.rfid_started = true;
// }

//=================================================================================
// SETUP
//=================================================================================
void setup() {

    #ifdef DEBUG
    Serial.begin(115200);
    while(!Serial);
    #endif
    
    DEBUG_PRINT("StartFunction");

    i2cScan();

    startVS1053();
    startSD();
    startOnboardStorage();
    startTrellis();
    startMAX9744();
    //startNFC();

    box.begin();
    box.selectMode();
}

//=================================================================================
// MAIN LOOP
//=================================================================================
void loop() {

    // Once mode is selected, Starting the right loop
    switch (box.boxMode) {
        case BOX_MODE_DIAG:
            diag.begin();
            diag.loop();
            break;    

        case BOX_MODE_PLAYER:
            musicPlayer.begin();
            musicPlayer.loop();
            break;    

        case BOX_MODE_PIANO:
            break;    

        case BOX_MODE_GAME:
            break;    

        default:
            DEBUG_PRINT("Ooops... unknown mode ! Restarting.");
            box.boxMode = BOX_MODE_UNDEF;
            box.selectMode();
            break;
    }
}
