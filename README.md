# weather_station_home_made

Suite of two home made weather stations using raspberry pi and arduino.  Also third weather station with home made indoors part and Acurite Atlas outdoors part.  Designed for use with weewx software.  

You would need a great many parts to build any of these three weather stations.  Consequently I do not expect many people to use this github project directly.  If I had to recommend one of the three stations it would be "green".  This github project is useful for:
(A) people who have an interest in using weewx with more than one weather station
(B) people who are interested in modbus especially with weather sensors
(C) people who want examples of how to customize weewx

here is the general layout ...

(0) hostname weathers raspberry pi 4b is the weewx server.
The server accepts input from three weather stations (weewx_atlas, weewx_green, weewx_wmod).  The server stores weather data in a separate sqlite3 database for each weather station and produces a set of graphs every few minutes.  The graphs are available as web pages and weathers also runs an nginx webserver.  The weathers server should be manipulated remotely from a desktop.  All development should take place on the desktop (preferably in ~/git/weather_station_home_made/) and be pushed to weathers and the other raspis using the scripts in $GIT_HOME/bin/wg-arduino-* and $GIT_HOME/bin/wg-WEATHER-*.

(1) weewx_atlas (a weewxd systemd service)
Outdoors hardware is Acurite Atlas (TM).  The radio signal from the atlas is received by and RTL-SDR radio dongle hooked up by usb to weathers.  Weewx uses the weewx-sdr driver (not by me) which in turn uses rtl-433 (not by me) to control the dongle and read the radio data printed on the usb connection.  Indoors hardware is Arduino Adafruit ItsyBitsy M4 nicknamed wpa.  This wpa reads the indoors temperature, pressure and humidity using an Adafruit BOSCH bme688 breakout board.  The wpa Arduino is hooked up to weathers over another usb serial port ttyACMwa.  This is connected to weewx by a custom weewx service found in WEEWX/weewx-atlas.  This weather station has excellent uptime but it had a glitch:  gaps in the weewx database data due to nulls (python Nones).  The gaps were fixed by handling the indoors data with a weewx service I wrote.  The weewx-sdr driver and my service together keep pace with the input from the two usb ports as it arrives.

(2) weewx_green (a weewxd systemd service)
This was an attempt to make the most robust and simple weather station possible and it succeeded.  It has almost perfect uptime and no errors.
The outdoors part is a raspi02w with hostname weathere (for external) with I2C sensors for light and UV and an interrupt driven handler for the rain bucket switch.  Anemometer is made by the company Renke and has modbus output.  Also temperature is Taidacent SHT30 and has modbus output.  I wrote a tiny modbus library.  The outdoors raspi runs a systemd service also called "weathere".  This service controls all the sensors and simply prints the result to /tmp/weathere_final.dat.  The indoors part of the weather station is a raspi02w with hostname weatherg (for green) which is hooked up to an Adafruit Itsy Bitsy M4 Arduino which controls another BME688 temperature pressure and humidity sensor.  The weatherg runs a systemd service also called "weatherg".  This also simply prints the output from the bme688 on /tmp/weatherg_final.dat.  The driver for weewx simply scps (secure copies) the two files to weathers!  Stupid but flawless.

(3) weewx_wmod (a weewxd systemd service)
This is much more complicated.  Outdoors there are two modbus sensors:  the anemometer (same Renke model but currently absent) and the temperature and humidity sensor (another SHT30).  The original design was to place the light sensor separate to keep it out of the shade and so on for each sensor.  Connecting the modbus wires to the central outdoors modbus client (wpc) I had too many bad connections.  I would have to have better hardware to do this.  The light and the rain each had their own arduino (wpl and wpr) and acted as a modbus sensor/server; I just crammed them into the same enclosure as the client wpc and used shorter wires to connect them all.  The client wpc is also a LoRa radio transmitter and sends the signal to the indoors.  The wpw arduino indoors is the radio receiver.  It also has the indoors sensors (you guessed it another bme688).  The arduinos  have a much more complicated modbus library I wrote in c++.![image](https://github.com/garberw/weather_station_home_made/assets/45077264/bced678e-2037-4b42-9722-8b3aa69a8fa2)

