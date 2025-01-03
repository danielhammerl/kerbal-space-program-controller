#ifndef SEVENSEGMENT_H
#define SEVENSEGMENT_H

#include <unistd.h>
#include "base-header.h"
#include <iostream>
#include <map>

// https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf
enum MAX_REGISTER {
    NOOP = 0x0,
    DIGIT0 = 0x1,
    DIGIT1 = 0x2,
    DIGIT2 = 0x3,
    DIGIT3 = 0x4,
    DIGIT4 = 0x5,
    DIGIT5 = 0x6,
    DIGIT6 = 0x7,
    DIGIT7 = 0x8,
    DECODE_MODE = 0x9,
    INTENSITY = 0xa,
    SCAN_LIMIT = 0xb,
    SHUTDOWN = 0xc,
    DISPLAY_TEST = 0xf,
};

static std::map<char, unsigned char> SEVEN_SEGMENT_FONT = {
    {'0', 0b01111110},
    {'1', 0b00110000},
    {'2', 0b01101101},
    {'3', 0b01111001},
    {'4', 0b00110011},
    {'5', 0b01011011},
    {'6', 0b01011111},
    {'7', 0b01110000},
    {'8', 0b01111111},
    {'9', 0b01111011},
    {'-', 0b00000001},
    {' ', 0b00000000},
    {'E', 0b01001111},
};

class SevenSegment {
public:
    SevenSegment() {
        spiFileDescriptor = wiringPiSPISetup(0, 1000000);

        if (spiFileDescriptor < 0) {
            std::cout << "Cannot connect to spi port!" << std::endl;
        }

        this->shutdown();
        this->setIntensity(4);
        this->setScanLimit(7);
        this->wakeUp();

        timer.reset();
    }

    ~SevenSegment() {
        this->shutdown();
        close(spiFileDescriptor);
    }

    void sendData(unsigned char registerAddress, unsigned char data)  {
        unsigned char dataToWrite[2] = {registerAddress, data};
        write(spiFileDescriptor, dataToWrite, 2);
    }

    void shutdown() { sendData(SHUTDOWN, 0); }
    void wakeUp() { sendData(SHUTDOWN, 1); }

    void setScanLimit(unsigned char limit) { sendData(SCAN_LIMIT, limit); }

    void setIntensity(unsigned char intensity) { sendData(INTENSITY, intensity); }

    // set every bit of decimalPlaceBitmask to 1 where a decimal point should be
    void writeString(std::string toWrite, unsigned char decimalPlaceBitmask = 0b00000000) {
        if(timer.elapsedMilliseconds() > 500) {
            if (toWrite.length() > 8) {
                throw std::invalid_argument("Can only write max 8 digit strings to 7 segment");
            }

            if (toWrite.length() < 8) {
                // pad string left with ' ' to 8 chars
                toWrite.insert(toWrite.begin(), 8 - toWrite.size(), ' ');
            }

            for (unsigned index = 1; index <= toWrite.length(); index++) {
                bool hasDecimalPlace = decimalPlaceBitmask >> (index - 1) & 0x1;

                auto dataValue = SEVEN_SEGMENT_FONT.at(toWrite[toWrite.length() - index]);
                if (hasDecimalPlace) {
                    dataValue = 0b10000000 | dataValue; // set first bit to true to toggle decimal point
                }

                this->sendData(index, dataValue);
            }
            timer.reset();
        }

        // ?
        delay(5);
    }

private:
    int spiFileDescriptor;
    Timer timer;
};


#endif //SEVENSEGMENT_H
