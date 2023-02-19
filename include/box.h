#ifndef BOX_H
#define BOX_H

#include <Arduino.h>

#define BOOT_MODE_PIN    47      // Boot mode / GPIO1 : if high => MIDI
#define NO_KEY   255

// Main Buttons
#define BTN_COUNT       4

#define BTN_PIN_ENCODER 40
#define BTN_PIN_BLUE    48
#define BTN_PIN_GREEN   34
#define BTN_PIN_YELLOW  17

#define BTN_ID_ENCODER  0
#define BTN_ID_BACK     0   // Encoder
#define BTN_ID_PIANO    1
#define BTN_ID_PLAYER   2
#define BTN_ID_TILT     3

#define BTN_ID_BLUE     1
#define BTN_ID_GREEN    2
#define BTN_ID_YELLOW   3

// Main buttons LEDs
#define LED_PLAYER     35
#define LED_PIANO      49
#define LED_TILT       16

// Encoder
#define ENCODER_A      43
#define ENCODER_B      41
#define ENCODER_RED    4
#define ENCODER_GREEN  13 
#define ENCODER_BLUE   12

// Led states
#define ON 0x0
#define OFF 0x1

// Box modes
#define BOX_MODE_COUNT 3

#define BOX_MODE_DEFAULT -1
#define BOX_MODE_UNDEF   -1
#define BOX_MODE_TEST    -2
#define BOX_MODE_PLAYER   0
#define BOX_MODE_PIANO    1
#define BOX_MODE_GAME     2

// Led bliking period
#define MODE_LED_BLINK       100 // Blinking period in ms for mode selection
#define OPTION_LED_BLINK     300 // Matrix leds blinking period for option selection

// Player states
#define PLAYER_OK   0
#define PLAYER_NOSD 1
#define PLAYER_ERR  2

#define RFID_READ_INTERVAL 100

// See https://www.w3schools.com/colors/colors_picker.asp
#define COLOR_RED    0xFF0000
#define COLOR_ORANGE 0xB34700
#define COLOR_YELLOW 0xFFFF00
#define COLOR_OLIVE  0x66DD00
#define COLOR_GREEN  0x008000
#define COLOR_AQUA   0x00FF66
#define COLOR_TEAL   0x00BFFF
#define COLOR_BLUE   0x0080FF
#define COLOR_NAVY   0x000080
#define COLOR_MAROON 0x800000
#define COLOR_PURPLE 0x800080
#define COLOR_PINK   0xFF66B3
#define COLOR_WHITE  0xFFFFFF
#define COLOR_BLACK  0x000000

class Box {
    public:

        int boxMode=BOX_MODE_DEFAULT;
        const int Max9744i2cAddr = 0x4B;
        uint8_t volume;
        uint8_t rfidUid[7];
        long rfidLastRead = 0;

        void begin();          // No constructor but a begin()
        void selectMode();     // 

        bool setVolume(int8_t v);
        bool increaseVolume();
        bool decreaseVolume();

        boolean readRFID();
};

#endif