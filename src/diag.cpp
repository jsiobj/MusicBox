#define PREFER_SDFAT_LIBRARY 1
//#define DEBUG
#include "debug.h"

#include <Adafruit_VS1053.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_PN532.h>

#include "box.h"
#include "diag.h"
#include "player.h"

extern Box box;
extern Adafruit_NeoTrellis trellis;
extern Adafruit_PN532 nfc;
extern SdFat SD;
extern MusicPlayer musicPlayer;

enum DIAG_BTN_ID {
  BTN_ID_I2C = 0,
  BTN_ID_LS,          
  BTN_ID_RESET_PARAM,
  BTN_ID_LONG_PRESS1,
  BTN_ID_LONG_PRESS2,
  BTN_ID_LONG_PRESS3,
  BTN_ID_BLINK,
  BTN_ID_CYCLE,
};

uint32_t colors[] = { COLOR_BLUE, COLOR_RED, COLOR_ORANGE, COLOR_NAVY, COLOR_YELLOW, COLOR_PURPLE };

TrellisCallback listI2C(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_ORANGE);
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

        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLUE);
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback lsSD(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_ORANGE);
            if(box.sdreader_started) {
                File root = SD.open("/");
                SD.ls(LS_R);
                root.close();
            }
            else {
                Serial.println("SD reader disabled");
            }
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLUE);
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback resetParams(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_ORANGE);
            musicPlayer.saveParam("track",NO_KEY);
            musicPlayer.saveParam("album",NO_KEY);
            musicPlayer.saveParam("library",NO_KEY);

            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.setPixelColor(event.bit.NUM,COLOR_BLUE);
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback longPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM, COLOR_RED);
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.setBlink(BTN_ID_LONG_PRESS2, COLOR_RED, 200);
            trellis.pixels.setPixelColor(event.bit.NUM, COLOR_ORANGE);
            if(trellis.wasLongPressed(event.bit.NUM)) trellis.pixels.enableCycling(BTN_ID_LONG_PRESS3, false);
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback duringLongPress(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    trellis.pixels.setCycle(BTN_ID_LONG_PRESS3, colors, 6, 100);
    trellis.pixels.setCycle(event.bit.NUM, colors, 6, 100);
    return 0;
}

TrellisCallback blink(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM, COLOR_RED);
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.enableCycling(event.bit.NUM, !trellis.pixels.isCyclingEnabled(event.bit.NUM));
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback cycle(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM, COLOR_RED);
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            trellis.pixels.enableCycling(event.bit.NUM, !trellis.pixels.isCyclingEnabled(event.bit.NUM));
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
    return 0;
}

TrellisCallback switchOff(keyEvent event) {
    DEBUG_PRINTF("Key event: EDGE[%d] NUM[%d] Reg[%x]",event.bit.EDGE, event.bit.NUM, event.reg);
    switch (event.bit.EDGE) {
        case SEESAW_KEYPAD_EDGE_RISING:
            trellis.pixels.setPixelColor(event.bit.NUM, 0);
            DEBUG_PRINTF("Key %d pressed",event.bit.NUM);
            break;
        
        case SEESAW_KEYPAD_EDGE_FALLING:
            DEBUG_PRINTF("Key %d released",event.bit.NUM);
            break;

        default:
            break;
    }
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
            trellis.registerCallback(i, switchOff);
            trellis.pixels.setPixelColor(i, COLOR_NAVY);
        }

        trellis.registerCallback(BTN_ID_I2C, listI2C);
        trellis.registerCallback(BTN_ID_LS, lsSD);          
        trellis.registerCallback(BTN_ID_RESET_PARAM, resetParams);
        trellis.registerCallback(BTN_ID_BLINK, blink);
        trellis.registerCallback(BTN_ID_CYCLE, cycle);

        trellis.registerCallback(BTN_ID_LONG_PRESS1, longPress);
        trellis.registerCustomCallback(BTN_ID_LONG_PRESS1, duringLongPress);

        trellis.pixels.setBlink(BTN_ID_BLINK, COLOR_GREEN, 200);
        trellis.pixels.setCycle(BTN_ID_CYCLE, colors, 3 , 600);
    }

    DEBUG_PRINT("ExitFunction");
}

// Main diag loop function
void Diag::loop() {
    while(true) {
        trellis.read();
        trellis.pixels.showCycle();
    }
}