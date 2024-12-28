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
    digitalWrite(26, HIGH);
}

void onDisconnection() {
    std::cout << "Disconnected from KSP!" << std::endl;
    digitalWrite(26, LOW);
}

float flightControlSensitivity = .4f;
bool actionGroupsPressed[10] = {true, true, true, true, true, true, true, true, true, true};

[[noreturn]] int main() {
    std::cout << "Starting ksp controller ..." << std::endl;
    std::cout << "Trying to connect to " << getHostName() << std::endl;

#ifdef KSP_CONTROLLER_DEV_MODE
    std::cout << "We are in Dev mode!" << std::endl;
#endif

    SevenSegment sevenSegment;

    wiringPiSetupGpio();

    pinMode(100, OUTPUT); // light on / off (temp)

    // joystick has to be js0, means if multiple joysticks are connected only one works
    auto joystickDevice = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);

    auto conn = krpc::connect(CLIENT_NAME, getHostName());

    krpc::services::KRPC krpc(&conn);

    krpc::services::SpaceCenter sc(&conn);
    onConnection();
    bool isConnected = true;

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
