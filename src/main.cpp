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

#ifdef __JETBRAINS_IDE__
#define KSP_CONTROLLER_DEV_MODE
#endif

#ifndef KSP_CONTROLLER_DEV_MODE
#include <wiringPi.h>
#else

//#define DEBUG_MODE; // comment this line in for extended debug outputs

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
    std::cout << "Conntected to KSP!" << std::endl;
    digitalWrite(26, HIGH);
}

void onDisconnection() {
    std::cout << "Disconntected from KSP!" << std::endl;
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


    // if false its in ROT mode, if true in LIN mode
    volatile bool rcsInLinMode = false; //volatile for now only to supress CLion warning, Remove in future

    // endless loop
    while (true) {
        if (krpc.current_game_scene() == krpc::services::KRPC::GameScene::flight) {
            if (!isConnected) {
                onConnection();
                isConnected = true;
            }
            auto vessel = sc.active_vessel();
            auto flight = vessel.flight();
            js_event data; // NOLINT(*-pro-type-member-init)
            auto hasEvent = read(joystickDevice, &data, sizeof(data));

            auto control = vessel.control();

            if (hasEvent != -1) {
                if (data.type == JS_EVENT_AXIS) {
                    // maybe I want to have different sensitiviy levels for ROT and LIN RCS mode in future
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
