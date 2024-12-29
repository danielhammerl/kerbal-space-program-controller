#ifndef SHIFT_OUTPUTS_H
#define SHIFT_OUTPUTS_H

#include <unistd.h>
#include "base-header.h"
#include <iostream>
#include <array>
#include <functional>
#include <bitset>

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

        timer.reset();
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
        if (timer.elapsedMilliseconds() > 250) {
            // third bit shifter, 7 to 0
            storeBitInRegister(data.highG);
            storeBitInRegister(data.heatShieldWarning);
            storeBitInRegister(data.connectedToKspServer);
            storeBitInRegister(data.connectedToCommnet);
            storeBitInRegister(data.electricalCharge[9]);
            storeBitInRegister(data.electricalCharge[8]);
            storeBitInRegister(data.electricalCharge[7]);
            storeBitInRegister(data.electricalCharge[6]);

            // second bit shifter, 7 to 0
            storeBitInRegister(data.electricalCharge[5]);
            storeBitInRegister(data.electricalCharge[4]);
            storeBitInRegister(data.electricalCharge[3]);
            storeBitInRegister(data.electricalCharge[2]);
            storeBitInRegister(data.electricalCharge[1]);
            storeBitInRegister(data.electricalCharge[0]);
            storeBitInRegister(data.fuelInCurrentStage[9]);
            storeBitInRegister(data.fuelInCurrentStage[8]);

            // first bit shifter, 7 to 0
            storeBitInRegister(data.fuelInCurrentStage[7]);
            storeBitInRegister(data.fuelInCurrentStage[6]);
            storeBitInRegister(data.fuelInCurrentStage[5]);
            storeBitInRegister(data.fuelInCurrentStage[4]);
            storeBitInRegister(data.fuelInCurrentStage[3]);
            storeBitInRegister(data.fuelInCurrentStage[2]);
            storeBitInRegister(data.fuelInCurrentStage[1]);
            storeBitInRegister(data.fuelInCurrentStage[0]);

            triggerLatch();

            timer.reset();
        }
    }

private:

    void storeBitInRegister(bool value) {
        digitalWrite(PIN_SERIAL_DATA_IN, value ? LOW : HIGH); // flip value here

        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
    }

    void triggerLatch() {
        // define refresh rate here

        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
    }

    Timer timer;
};

#endif