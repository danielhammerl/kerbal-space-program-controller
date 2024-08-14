#ifndef BASE_HEADER_H
#define BASE_HEADER_H

#ifdef __JETBRAINS_IDE__
#define KSP_CONTROLLER_DEV_MODE
#endif

//#define DEBUG_MODE; // comment this line in for extended debug outputs

#ifndef KSP_CONTROLLER_DEV_MODE
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <mcp23017.h>
#else
// mock wiringpi in dev env

static int wiringPiSPISetup(int channel, int speed, int mod) { return 0; };

static int mcp23017Setup(int pin_base, int addr) { return 0; };

static int wiringPiSPISetup(int channel, int speed) { return 0; };

static int wiringPiSetupGpio() { return 0; };

static void delay(unsigned int howLong) {};

#define INPUT 0
#define OUTPUT 1
#define WPI_ALT0 4

#define HIGH 1
#define LOW 0

#define    PUD_OFF 0
#define    PUD_DOWN 1
#define    PUD_UP 2

static void pinMode(int pin, int mode) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked pinMode with pin " << pin << " and mode " << mode << std::endl;
#endif
};

static void pinModeAlt(int pin, int mode) {};

static int digitalRead(int pin) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked digitalRead with pin " << pin << ". Note that this mock always returns LOW" << std::endl;
#endif
    return LOW;
};

static void digitalWrite(int pin, int value) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked digitalWrite with pin " << pin << " and value" << value << std::endl;
#endif
};

static void pullUpDnControl(int pin, int mode) {};

static void* wiringPiNodes;

#endif

#ifdef KSP_CONTROLLER_DEV_MODE
#define CLIENT_NAME "daniels ksp controller dev"
#else
#define CLIENT_NAME "daniels ksp controller"
#endif

#include <cstdlib>
#include <limits>
#include <string>

static std::string getHostName() {
    const auto env = std::getenv("KSP_SERVER_HOSTNAME");
    return env ? env : "daniel-MS-7E26";
}

static constexpr float normalizeShort(short value) {
    return value < 0
           ? -static_cast<float>(value) / std::numeric_limits<short>::min()
           : static_cast<float>(value) / std::numeric_limits<short>::max();
}

#endif
