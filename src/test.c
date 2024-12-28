#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pigpio.h>

#define PIN_SH_CP   17  // Pin ist GPIO-#
#define PIN_ST_CP   27  // Pin ist GPIO-#
#define PIN_DS      22  // Pin ist GPIO-#

#define PIN_SHIFT_REGISTER_CLOCK   PIN_SH_CP
#define PIN_STORAGE_REGISTER_CLOCK PIN_ST_CP
#define PIN_SERIAL_DATA_IN         PIN_DS

// this function sends a uint8_t value bitwise to "dataPin", clocking (HIGH/LOW change) "clockPin"
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint16_t val)
{
    unsigned int swappedVal = ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);

    for (int i = 0; i < 16; i++)  {
        int bitValue = !!(swappedVal & (1 << i)); // Extrahiere das i-te Bit
        gpioWrite(dataPin, bitValue);

        gpioWrite(clockPin, PI_HIGH);
        gpioWrite(clockPin, PI_LOW);
    }
}


// this function triggers transfer of stored bits to the output register
// by a HIGH/LOW change of "latchPin"
void triggerLatch( uint8_t latchPin )
{
    gpioWrite(latchPin, PI_HIGH);
    gpioWrite(latchPin, PI_LOW);
}


int main( int argc, char *argv[] )
{
    if (gpioInitialise() < 0)
    {
        // pigpio initialisation failed.
        fprintf(stderr, "failed to init!\n");
        exit(1);
    }

    // set pins as output
    gpioSetMode( PIN_SHIFT_REGISTER_CLOCK,   PI_OUTPUT );
    gpioSetMode( PIN_STORAGE_REGISTER_CLOCK, PI_OUTPUT );
    gpioSetMode( PIN_SERIAL_DATA_IN,         PI_OUTPUT );


    // initialize output pins to LOW
    gpioWrite( PIN_SHIFT_REGISTER_CLOCK,   PI_LOW );
    gpioWrite( PIN_STORAGE_REGISTER_CLOCK, PI_LOW );
    gpioWrite( PIN_SERIAL_DATA_IN,         PI_LOW );


    // do a loop over eight bits ...
    for(int val = 0; val <= 65000; val++ )
    {
        // send bitwise to 74hc595
        shiftOut(PIN_SERIAL_DATA_IN, PIN_SHIFT_REGISTER_CLOCK, ~val);
        // transfer to output registers
        triggerLatch( PIN_STORAGE_REGISTER_CLOCK );

        gpioSleep(PI_TIME_RELATIVE, 0, 50000);
    }

    gpioTerminate();

    return(0);
}