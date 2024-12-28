#include <cmath>
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
#include "SevenSegment.h"
#include "base-header.h"
#include <sstream>

bool isConnected = false;

/*
gcc -Wall -o 74hc595 74hc595.c -lpthread -lpigpio
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
GPIO17 Pin #11  SH_CP
GPIO27 Pin #13  ST_CP
GPIO22 Pin #15  DS
*/
#define PIN_SH_CP   17  // Pin ist GPIO-#
#define PIN_ST_CP   27  // Pin ist GPIO-#
#define PIN_DS      22  // Pin ist GPIO-#

#define PIN_SHIFT_REGISTER_CLOCK   PIN_SH_CP
#define PIN_STORAGE_REGISTER_CLOCK PIN_ST_CP
#define PIN_SERIAL_DATA_IN         PIN_DS

#define LSBFIRST     0
#define MSBFIRST     1


// this function triggers transfer of stored bits to the output register
// by a HIGH/LOW change of "latchPin"
void triggerLatch(uint8_t latchPin) {
    digitalWrite(latchPin, HIGH);
    digitalWrite(latchPin, LOW);
}


void get_altitude(SevenSegment &sevenSegment, unsigned long long altitude) {
    std::stringstream ss;
    int exponent = 1; // 1 for meters, 4 for kilometers, 7 for megameters, ...
    while (altitude > 1000000) {
        if (exponent < 7) {
            exponent += 3;
            altitude /= 1000;
        } else {
            exponent++;
            altitude /= 10;
        }
    }

    if (exponent <= 9) {
        ss << altitude << "E" << exponent;
    } else {
        ss << "EEEEEEEE";
    }
    sevenSegment.writeString(ss.str(), 0b00100000);
}

void onConnection() {
    std::cout << "Connected to KSP!" << std::endl;
    isConnected = true;
    digitalWrite(26, HIGH);
}

void onDisconnection() {
    std::cout << "Disconnected from KSP!" << std::endl;
    isConnected = false;
    digitalWrite(26, LOW);
}

float flightControlSensitivity = .4f;
bool actionGroupsPressed[10] = {true, true, true, true, true, true, true, true, true, true};

[[noreturn]] int main() {
    std::cout << "Starting ksp controller ..." << std::endl;

#ifdef KSP_CONTROLLER_DEV_MODE
    std::cout << "We are in Dev mode!" << std::endl;
#endif

    wiringPiSetupGpio();

    /////

    // set pins as output
    pinMode(PIN_SHIFT_REGISTER_CLOCK, OUTPUT);
    pinMode(PIN_STORAGE_REGISTER_CLOCK, OUTPUT);
    pinMode(PIN_SERIAL_DATA_IN, OUTPUT);


    // initialize output pins to LOW
    digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
    digitalWrite(PIN_STORAGE_REGISTER_CLOCK, LOW);
    digitalWrite(PIN_SERIAL_DATA_IN, LOW);
/*
    for (int i = 0; i < 16; i++)  {
        digitalWrite(PIN_SERIAL_DATA_IN, LOW);


        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);

        triggerLatch( PIN_STORAGE_REGISTER_CLOCK );
    }

    for(int val = 0; val <= 65000; val++ )
    {
        unsigned int swappedVal = ((~val & 0x00FF) << 8) | ((~val & 0xFF00) >> 8);

        for (int i = 0; i < 16; i++)  {
            int bitValue = !!(swappedVal & (1 << i)); // Extrahiere das i-te Bit
            digitalWrite(PIN_SERIAL_DATA_IN, bitValue);

            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
        }

        // transfer to output registers
        triggerLatch( PIN_STORAGE_REGISTER_CLOCK );

        std::this_thread::sleep_for(std::chrono::milliseconds (50));
    }*/

    uint16_t val = 1;
    while (true) {

        unsigned int swappedVal = ((~val & 0x00FF) << 8) | ((~val & 0xFF00) >> 8);

        for (int i = 0; i < 16; i++) {
            int bitValue = !!(swappedVal & (1 << i)); // Extrahiere das i-te Bit
            digitalWrite(PIN_SERIAL_DATA_IN, bitValue);

            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
        }

        // transfer to output registers
        triggerLatch(PIN_STORAGE_REGISTER_CLOCK);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        if (val == 1) {
            val = 2;
        } else {
            val = pow(val, 2);
        }
    }

    /*while(true) {
        for (int i = 0; i < 16; i++)  {
            digitalWrite(PIN_SERIAL_DATA_IN, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds (50));


            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, HIGH);
            digitalWrite(PIN_SHIFT_REGISTER_CLOCK, LOW);
        }

        triggerLatch( PIN_STORAGE_REGISTER_CLOCK );
    }*/

    ////

    SevenSegment sevenSegment;

    // joystick has to be js0, means if multiple joysticks are connected only one works
    auto joystickDevice = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

    std::cout << "Trying to connect to " << getHostName() << std::endl;
    auto conn = krpc::connect(CLIENT_NAME, getHostName());

    krpc::services::KRPC krpc(&conn);

    krpc::services::SpaceCenter sc(&conn);
    onConnection();

//
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
                        default:;
                    }
                }
            }

            auto lights = vessel.control().lights();

            digitalWrite(100, lights ? HIGH : LOW);

            /*if (digitalRead(106) == LOW) {
                std::cout << "digital read 106 is LOW" << std::endl;
                if(!actionGroupsPressed[0]) {
                    actionGroupsPressed[0] = true;
                    vessel.control().toggle_action_group(0);
                   // std::cout << "trigger action group 0" << std::endl;
                }
            } else {
                std::cout << "digital read 106 is HIGH" << std::endl;
                actionGroupsPressed[0] = false;
            }*/

            get_altitude(sevenSegment, static_cast<unsigned long long>(flight.surface_altitude()));
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
