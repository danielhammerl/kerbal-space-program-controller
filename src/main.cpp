#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <krpc.hpp>
#include <krpc/services/space_center.hpp>
#include <krpc/services/krpc.hpp>
#include <thread>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>

//
/*
 * max7219.cpp
 * Author: Thomas Müller
 * Copyright 2013 Thomas Müller <tmuel at gmx.net>
 * $Id$
 */


/******************************************************************************
***   Include                                                               ***
******************************************************************************/

#include <stdio.h>
#include <iostream>
#include <cstring>
//#include "max7219.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>

using namespace std;

int DEBUG_ACTIVE = 0; // Global debug variable

/******************************************************************************
***   Defines and Constants                                                 ***
******************************************************************************/

//MAX7219/MAX7221's memory register addresses:
// See Table 2 on page 7 in the Datasheet
const char NoOp        = 0x00;
const char Digit0      = 0x01;
const char Digit1      = 0x02;
const char Digit2      = 0x03;
const char Digit3      = 0x04;
const char Digit4      = 0x05;
const char Digit5      = 0x06;
const char Digit6      = 0x07;
const char Digit7      = 0x08;
const char DecodeMode  = 0x09;
const char Intensity   = 0x0A;
const char ScanLimit   = 0x0B;
const char ShutDown    = 0x0C;
const char DisplayTest = 0x0F;


const char numOfDevices = 2;

/******************************************************************************
***   Function Definitions                                                  ***
******************************************************************************/

void setup();
void loop();

/******************************************************************************
***   Global Variables                                                      ***
******************************************************************************/



/******************************************************************************
***   Class: RasPiSPI                                                       ***
******************************************************************************/

class RasPiSPI
{
private:
    int SpiFd; // File descriptor of spi port

    char *TxBuffer;
    char *RxBuffer;

    int TxBufferIndex;
    int RxBufferIndex;

public:
    RasPiSPI(); // Konstruktor
    ~RasPiSPI(); // Destruktor

    void begin() { begin(0, 1000000); } // default use channel 0 and 1MHz clock speed
    void begin(int, int);

    void transfer(char);
    void endTransfer();
};

RasPiSPI::RasPiSPI() // CONSTRUCTOR
{
    if(DEBUG_ACTIVE > 0) {cout << "RasPiSPI Konstruktor" << endl;}

    TxBuffer = new char[1024]; // Buffer for TxData
    RxBuffer = new char[1024]; // Buffer for RxData

    TxBufferIndex = 0;
    RxBufferIndex = 0;
}
RasPiSPI::~RasPiSPI() // DESTRUCTOR
{
    if(DEBUG_ACTIVE > 0) {cout << "RasPiSPI Destruktor" << endl;}

    delete[] TxBuffer;
    delete[] RxBuffer;

    close(SpiFd); // Close SPI port
}

RasPiSPI SPI; // Create class SPI

void RasPiSPI::begin(int channel, int speed)
{
    if ((SpiFd = wiringPiSPISetup (channel, speed)) < 0)
    {	// Open port for reading and writing
        cout << "Failed to open SPI port " << channel << "! Please try with sudo" << endl;
    }
    if(DEBUG_ACTIVE > 0) {cout << "Filehandle opened" << endl;}
}

void RasPiSPI::transfer(char c)
{
    TxBuffer[TxBufferIndex] = c;
    TxBufferIndex++;
}
void RasPiSPI::endTransfer()
{
    int temp = write(SpiFd, TxBuffer, TxBufferIndex); // Write the data from TxBuffer to the SPI bus...
    if(DEBUG_ACTIVE > 1)
    { // Debug level 2
        cout << "Written: " << temp << " Index: " << TxBufferIndex << " Buffer: ";
        for(int i = 0; i < TxBufferIndex; i++)
        {
            cout << int(TxBuffer[i]) << " ";
        }
        cout << endl;
    }
    TxBufferIndex = 0; // ...and reset the index
}

/*
 * End of class RasPiSPI
 */



// Writes data to the selected device or does broadcast if device number is 255
void SetData(char adr, char data, char device)
{
    // Count from top to bottom because first data which is sent is for the last device in the chain
    for (int i = numOfDevices; i > 0; i--)
    {
        if ((i == device) || (device == 255))
        {
            SPI.transfer(adr);
            SPI.transfer(data);
        }
        else // if its not the selected device send the noop command
        {
            SPI.transfer(NoOp);
            SPI.transfer(0);
        }
    }
    SPI.endTransfer();

    delay(1);
}

// Writes the same data to all devices
void SetData(char adr, char data) { SetData(adr, data, 255); } // write to all devices (255 = Broadcast)

void SetShutDown(char Mode) { SetData(ShutDown, !Mode); }
void SetScanLimit(char Digits) { SetData(ScanLimit, Digits); }
void SetIntensity(char intense) { SetData(Intensity, intense); }
void SetDecodeMode(char Mode) { SetData(DecodeMode, Mode); }

/******************************************************************************
***   Setup                                                                 ***
******************************************************************************/

void setup()
{
    // The MAX7219 has officially no SPI / Microwire support like the MAX7221 but the
    // serial interface is more or less the same like a SPI connection

    SPI.begin();

    // Disable the decode mode because at the moment i dont use 7-Segment displays
    if(DEBUG_ACTIVE > 0) {cout << "SetDecodeMode(false);" << endl;}
    SetDecodeMode(false);
    // Set the number of digits; start to count at 0
    if(DEBUG_ACTIVE > 0) {cout << "SetScanLimit(7);" << endl;}
    SetScanLimit(7);
    // Set the intensity between 0 and 15. Attention 0 is not off!
    if(DEBUG_ACTIVE > 0) {cout << "SetIntensity(5);" << endl;}
    SetIntensity(5);
    // Disable shutdown mode
    if(DEBUG_ACTIVE > 0) {cout << "SetShutDown(false);" << endl;}
    SetShutDown(false);

    if(DEBUG_ACTIVE > 0) {cout << "Write Patterns" << endl;}

    // Write some patterns
    SetData(Digit0, 0b10000000, 1);
    SetData(Digit1, 0b01000000, 1);
    SetData(Digit2, 0b00100000, 1);
    SetData(Digit3, 0b00010000, 1);
    SetData(Digit4, 0b00001000, 1);
    SetData(Digit5, 0b00000100, 1);
    SetData(Digit6, 0b00000010, 1);
    SetData(Digit7, 0b00000001, 1);

    SetData(Digit0, 0b00000001, 2);
    SetData(Digit1, 0b00000010, 2);
    SetData(Digit2, 0b00000100, 2);
    SetData(Digit3, 0b00001000, 2);
    SetData(Digit4, 0b00010000, 2);
    SetData(Digit5, 0b00100000, 2);
    SetData(Digit6, 0b01000000, 2);
    SetData(Digit7, 0b10000000, 2);

    if(DEBUG_ACTIVE > 0) {cout << "Delay 1000" << endl;}
    delay(1000);

}

/******************************************************************************
***   Loop                                                                  ***
******************************************************************************/

void loop()
{

    //you may know this from space invaders
    unsigned int rowBuffer[]=
            {
                    0b0010000010000000,
                    0b0001000100000000,
                    0b0011111110000000,
                    0b0110111011000000,
                    0b1111111111100000,
                    0b1011111110100000,
                    0b1010000010100000,
                    0b0001101100000000
            };

    if(DEBUG_ACTIVE > 0) {cout << "Start with space invader animation" << endl;}

    while(1)
    {
        for (int shiftCounter = 0; 15 >= shiftCounter; shiftCounter++)
        {
            for (int rowCounter = 0; 7 >= rowCounter; rowCounter++)
            {
                // roll the 16bits...
                // The information how to roll is from http://arduino.cc/forum/index.php?topic=124188.0
                rowBuffer[rowCounter] = ((rowBuffer[rowCounter] & 0x8000)?0x01:0x00) | (rowBuffer[rowCounter] << 1);

                // ...and then write them to the two devices
                SetData(rowCounter+1, char(rowBuffer[rowCounter]), 1);
                SetData(rowCounter+1, char(rowBuffer[rowCounter]>>8), 2);
            }
            delay(100);
        }
    }
}
//

#ifdef __JETBRAINS_IDE__
#define KSP_CONTROLLER_DEV_MODE
#endif

//#define DEBUG_MODE; // comment this line in for extended debug outputs

#ifndef KSP_CONTROLLER_DEV_MODE
#include <wiringPi.h>
#else
// mock wiringpi in dev env

void wiringPiSetupGpio() {
};
#define INPUT 0
#define OUTPUT 1
#define HIGH 1
#define LOW 0

void pinMode(int pin, int mode) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked pinMode with pin " << pin << " and mode " << mode << std::endl;
#endif
};

int digitalRead(int pin) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked digitalRead with pin " << pin << ". Note that this mock always returns LOW" << std::endl;
    return LOW;
#endif
};

void digitalWrite(int pin, int value) {
#ifdef DEBUG_MODE
    std::cout << "Call mocked digitalWrite with pin " << pin << " and value" << value << std::endl;
#endif
};

#endif

#ifdef KSP_CONTROLLER_DEV_MODE
#define CLIENT_NAME "daniels ksp controller dev"
#else
#define CLIENT_NAME "daniels ksp controller"
#endif


void get_altitude(const unsigned long long altitude) {
    std::cout << std::fixed << std::setprecision(1);
    std::cout << altitude << std::endl;
}
void onConnection() {
    std::cout << "Connected to KSP!" << std::endl;
    digitalWrite(26, HIGH);
}

void onDisconnection() {
    std::cout << "Disconnected from KSP!" << std::endl;
    digitalWrite(26, LOW);
}


std::string getHostName() {
    const auto env = std::getenv("KSP_SERVER_HOSTNAME");
    return env ? env : "daniel-MS-7E26";
}

constexpr float normalizeShort(short value) {
    return value < 0
               ? -static_cast<float>(value) / std::numeric_limits<short>::min()
               : static_cast<float>(value) / std::numeric_limits<short>::max();
}

float flightControlSensitivity = .4f;

[[noreturn]] int main() {
    std::cout << "Starting ksp controller ..." << std::endl;

#ifdef KSP_CONTROLLER_DEV_MODE
    std::cout << "We are in Dev mode!" << std::endl;
#endif
    setup();

    while(1)
    {
        loop();
    }

    wiringPiSetupGpio();

    pinMode(4, OUTPUT);
    //pinMode(26, OUTPUT);

    // joystick has to be js0, means if multiple joysticks are connected only one works
    auto joystickDevice = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

    auto conn = krpc::connect(CLIENT_NAME, getHostName());

    krpc::services::KRPC krpc(&conn);

    krpc::services::SpaceCenter sc(&conn);
    onConnection();
    bool isConnected = true;


    // if false it's in ROT mode, if true in LIN mode
    volatile bool rcsInLinMode = false; //volatile for now only to suppress CLion warning, Remove in future

    // endless loop
    while (true) {
        if (krpc.current_game_scene() == krpc::services::KRPC::GameScene::flight) {
            if (!isConnected) {
                onConnection();
                isConnected = true;
            }
            auto vessel = sc.active_vessel();
            auto flight = vessel.flight();
            // todo can i declare this outside of the while loop??
            js_event data; // NOLINT(*-pro-type-member-init)
            auto hasEvent = read(joystickDevice, &data, sizeof(data));

            auto control = vessel.control();

            if (hasEvent != -1) {
                if (data.type == JS_EVENT_AXIS) {
                    // maybe I want to have different sensitivity levels for ROT and LIN RCS mode in future
                    auto valueForMovement = normalizeShort(data.value) * flightControlSensitivity;
                    switch (data.number) {
                        case 0:
                            if (rcsInLinMode) {
                                control.set_right(valueForMovement);
                            } else {
                                control.set_yaw(valueForMovement);
                            }
                            break;
                        case 1:
                            if (rcsInLinMode) {
                                control.set_up(valueForMovement);
                            } else {
                                control.set_pitch(valueForMovement);
                            }
                            break;
                        case 2:
                            if (rcsInLinMode) {
                                control.set_forward(valueForMovement);
                            } else {
                                control.set_roll(valueForMovement);
                            }
                            break;
                        default: ;
                    }
                }
            }

            //bool light = true;// change
            //vessel.control().set_lights(light);

            auto lights = vessel.control().lights();
            //std::cout << "lights: " << (lights ? "on" : "off") << std::endl;

            digitalWrite(4, lights ? HIGH : LOW);

            //get_altitude(static_cast<unsigned long long>(flight.surface_altitude()));
        } else {
            if (isConnected) {
                onDisconnection();
                isConnected = false;
            }
        }

#ifdef KSP_CONTROLLER_DEV_MODE
        std::this_thread::sleep_for(std::chrono::microseconds(50));
#else
        std::this_thread::sleep_for(std::chrono::microseconds(1));
#endif
    }
}
