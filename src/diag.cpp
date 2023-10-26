#define PREFER_SDFAT_LIBRARY 1
#define DEBUG
#include "debug.h"

#include <Adafruit_VS1053.h>
#include <Custom_NeoTrellis.h>
#include <Adafruit_PN532.h>

#include "box.h"
#include "diag.h"
#include "player.h"

extern Box box;
extern Custom_NeoTrellis trellis;
extern Adafruit_PN532 nfc;
extern SdFat SD;

// void printDirectory(char *path, int max_depth) {

//     DEBUG_PRINTF("Listing %s content", path)
//     SD.ls(LS_R);

//     // if(!SD.exists(path)) {
//     //     DEBUG_PRINTF("Directory %s does not exists", path);
//     //     return;
//     // }

//     // File dir = SD.open(path, O_RDONLY);

//     // if(!dir) {
//     //     DEBUG_PRINTF("Cannot open %s", path);
//     //     return;
//     // }

//     // if(! dir.isDirectory()) {
//     //     DEBUG_PRINTF("%s is not a directory", path);
//     //     return;
//     // }
//     // dir.close();

//     // printDirectory(dir, max_depth);
// }

// void printDirectory(File dir, int max_depth, int current_depth) {
//     char fileNameBuffer[MAX_PATH_LENGTH];
//     //char dirNameBuffer[MAX_PATH_LENGTH];

//     //dir.getName(dirNameBuffer, MAX_PATH_LENGTH);
//     //DEBUG_PRINTF("Base directory: %s", dirNameBuffer);

//     dir.rewindDirectory();

//     while(true) {
        
//         File entry =  dir.openNextFile();
//         for (uint8_t i=0; i<current_depth; i++) {
//             Serial.print('\t');
//         }
//         if (! entry) {
//             // no more files
//             Serial.println("#OED");
//             break;
//         }
        
//         entry.getName(fileNameBuffer,MAX_PATH_LENGTH);
//         Serial.print(fileNameBuffer);
//         if (entry.isDirectory()) {
//             Serial.println("/");
//             if (current_depth <= max_depth) {
//                 printDirectory(entry, max_depth, current_depth+1);
//             }
//         } else {
//             // files have sizes, directories do not
//             Serial.print("\t\t");
//             Serial.println(entry.size(), DEC);
//         }

//         entry.close();
//     }
//     dir.rewindDirectory();
// }

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
                    SD.ls(LS_R);
                    root.close();
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

    SD.volumeBegin();
    FsFile rootDir;
    rootDir.open("/");
    rootDir.ls(LS_R);

    DEBUG_PRINT("ExitFunction");
}

// Main diag loop function
void Diag::loop() {
    while(true) trellis.read();
}