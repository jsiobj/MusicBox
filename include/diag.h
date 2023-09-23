#ifndef BOX_DIAG_H
#define BOX_DIAG_H

#include "box.h"

class Diag {
    public:
        long int rfidLastRead = 0;         // last time RFID was read
        long int rfidReadInterval = 1000;  // Delay between reads

        void begin();
        void loop();
        void nfcRead();
};

#endif