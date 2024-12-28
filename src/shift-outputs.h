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

//#define DATA_LENGTH 24
#define DATA_LENGTH 16

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
        std::bitset<DATA_LENGTH> dataToWrite;

        for (int x = 0; x < 10; x++) {
            dataToWrite.set(x, data.fuelInCurrentStage[x]);
        }

        /*for (int i = 0; i < 10; ++i) {
            dataToWrite.set(i + 10, data.electricalCharge[i]);
        }

        dataToWrite.set(20, data.highG);

        dataToWrite.set(21, data.heatShieldWarning);

        dataToWrite.set(22, data.connectedToCommnet);

        dataToWrite.set(23, data.connectedToKspServer);*/

        processBitsetLittleEndian(dataToWrite, [this](bool value) { this->triggerLatch(); });

        triggerLatch();
    }

private:

    void storeBitInRegister(bool value) {
        digitalWrite(PIN_SERIAL_DATA_IN, value ? LOW : HIGH); // flip value here

        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
    }

    void processBitsetLittleEndian(const std::bitset<DATA_LENGTH> &bitset, const std::function<void(bool)> &callback) {
        const int BITS_PER_BYTE = 8;
        constexpr std::size_t BYTES = (DATA_LENGTH + BITS_PER_BYTE - 1) / BITS_PER_BYTE; // Anzahl der Bytes

        // Verarbeite jedes Byte in Little-Endian-Reihenfolge
        for (std::size_t byteIndex = 0; byteIndex < BYTES; ++byteIndex) {
            std::size_t reversedByteIndex = BYTES - 1 - byteIndex; // Reverse die Byte-Reihenfolge

            // Verarbeite jedes Bit innerhalb des Bytes
            for (std::size_t bitOffset = 0; bitOffset < BITS_PER_BYTE; ++bitOffset) {
                std::size_t bitIndex = reversedByteIndex * BITS_PER_BYTE + bitOffset;

                // Überspringe Bits außerhalb der tatsächlichen Größe
                if (bitIndex < DATA_LENGTH) {
                    callback(bitset[bitIndex]);
                }
            }
        }
    }

    void triggerLatch() {
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
    }
};

#endif