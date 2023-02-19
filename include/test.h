#ifndef BOX_TEST_H
#define BOX_TEST_H

#include "box.h"

class Test {
    public:
        long int rfidLastRead = 0;         // last time RFID was read
        long int rfidReadInterval = 1000;  // Delay between reads

        void begin();
        void loop();
        void rfidRead();
};

#endif