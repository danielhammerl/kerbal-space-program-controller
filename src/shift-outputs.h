#ifndef SHIFT_OUTPUTS_H
#define SHIFT_OUTPUTS_H

#include <unistd.h>
#include "base-header.h"
#include <iostream>
#include <array>

#define PIN_SHIFT_REGISTER_CLOCK   17
#define PIN_STORAGE_REGISTER_CLOCK 27
#define PIN_SERIAL_DATA_IN         22

class ShiftOutputs {
public:
    ShiftOutputs() {
        // set pins as output
        pinMode(PIN_SHIFT_REGISTER_CLOCK, OUTPUT);
        pinMode(PIN_STORAGE_REGISTER_CLOCK, OUTPUT);
        pinMode(PIN_SERIAL_DATA_IN, OUTPUT);


        // initialize output pins to LOW
        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
        digitalWrite(PIN_SERIAL_DATA_IN, LOW);
    }

    struct ShiftOutputData {
        bool fuelInCurrentStage[10];
        bool electricalCharge[10];
        bool highG;
        bool heatShieldWarning;
        bool connectedToCommnet;
        bool connectedToKspServer;
    };

    void outputData(const ShiftOutputData &data) {
        for (bool i : data.fuelInCurrentStage) {
            storeBitInRegister(i);
        }
        storeBitInRegister(false);
        storeBitInRegister(false);
        storeBitInRegister(false);
        storeBitInRegister(false);
        storeBitInRegister(false);
        storeBitInRegister(false);

       /* for (int i = 0; i < 10; ++i) {
            storeBitInRegister(data.electricalCharge[i]);
        }

        storeBitInRegister(data.highG);

        storeBitInRegister(data.heatShieldWarning);

        storeBitInRegister(data.connectedToCommnet);

        storeBitInRegister(data.connectedToKspServer);
*/
        triggerLatch();
    }
private:
    
    void storeBitInRegister(bool value) {
        digitalWrite(PIN_SERIAL_DATA_IN, value ? LOW : HIGH); // flip value here

        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
    }

    void triggerLatch() {
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
    }
};

#endif