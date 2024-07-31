This program uses kRPC to remote control KSP with connected special hardware
Installing krpc is described here: https://krpc.github.io/krpc/cpp/client.html, prefer the cmake way.

protobuf can be installed via apt as 'protobuf-compiler'

Runs only on raspberry pi with wiringPi installed.
Uses Raspi GPIO interface and I2C AND SPI bus
Ensure I2C and SPI are enabled in raspi-config

Controls:
  - RCS on/off - Switch (1x Kippschalter)
  - SAS on/off - Switch (1x Kippschalter)
  - Staging - Push Button (1x Kippschalter fürs aktivieren, 1x Push Button, Rot/Grün LED)
  - Abort - Push Button groß
  - Light - Switch (1x Kippschalter)
  - Gear - Switch (1x Kippschalter)
  - Flight Control - Thrustmaster Joystick
  - Flight Control On/Off? (1x Kippschalter)
  - Thrust (1x Schiebepoti 1kOhm linear)
  - Thrust on/off (1x Kippschalter + Gelb LED) mit Blende
  - Action Groups (10 Push Buttons?)
  - SAS Mode (dreh poti mit mind. 8 fixe positionen)
  - RCS Mode - Lin/Rot (1x Kippschalter)
  - RCS Precise Mode? (1x Kippschalter, wenn aktiv, erhöhrt joystick sensitivität um faktor x)

Indicators:
  - Altitude (over ground) (7 segment anzeige)
  - Electrical Charge (gauge)
  - Fuel in current stage (gauge)
  - Dicke Atmosphäre (gauge)
  - HIGH G (Rote LED) (how high is high)
  - Hitzeschild warnung, weniger als 20% (Gelbe LED) 
  - Connected to Commnet (LED Blau)
  - Connected to KSP Server (LED Grün)