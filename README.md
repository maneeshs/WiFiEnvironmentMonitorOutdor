# WiFiEnvironmentMonitorOutdor
Read sensors and upload to emoncms.

Get temp, humidity, barometric pressure and light level and send them via WIFI to emoncms.

Powerd by a 18650 battery, charged by a solar pannel.
Total Power consumption while trasmiting ~75mA

Power consuption in deepSleep 0.20mA (on 3 AAA batteries will run for ~3 months)

## Components used:
 - Wemos D1 Mini
 - DHT22 Temperature and humidity sensor
 - BH1750 Light level sensor
 - GY-68 BMP180 barometric sensor
 - 1 x 18650 Battery socket
 - 1 x 18650 Battery 3.7V
 - 1 x 5V Micro USB 1A 18650 Lithium Battery Charging Board
 - 6V solar pannel

##Pins used on Wemos D1 Mini:
- D6 - power for the sensors
- D1 - DHT22 output
- D2 - SCL BH1750
- D4 - SDA BH1750
- A0 - Battery Level Voltage thru a 100k and 470k votage divider (not ideal but good enough)
- RST - D0 - used to wake the ESP from deepSleep
- 5V - To the Power supply

###Pictures with the finished product
http://imgur.com/a/Zx4D4
