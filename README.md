Controlling 433MHz RF outlets with Raspberry Pi
-----------------------------------------------

This project is designed to control up to six wireless outlets connected to
lights. They will switch on automatically around sunset each day and switch 
off at a specified time. The off time can be randomized for security 
purposes (e.g. when you are not at home).  

I am using "Bye Bye Standby Wireless Remote Control Energy 
Saving Kit 6 outlets" by Bye Bye Standby ($27.95 on Amazon)

You need a 433MHz receiver/transmitter as described here (and many other places):
http://www.princetronics.com/how-to-read-433-mhz-codes-w-raspberry-pi-433-mhz-receiver/
http://shop.ninjablocks.com/blogs/how-to/7506204-adding-433-to-your-raspberry-pi
http://stevenhickson.blogspot.com/2015/02/control-anything-electrical-with.html
https://github.com/ninjablocks/433Utils

Requirements and dependencies 
-----------------------------

Sunset calculations are performed with the AstroCalc4R library
  http://www.nefsc.noaa.gov/AstroCalc4R/ 
  Files included here: AstroCalc4R.c and myfuncs1.c

Required library: WiringPi 
  http://wiringpi.com/download-and-install/
  https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/
  git clone git://git.drogon.net/wiringPi
  Link with flag: -lwiringPi

We are using code from 'codesend' and 'RCSwitch' (part of the 433Utils library)
  https://github.com/ninjablocks/433Utils
  Files included here: RCSwitch.cpp and RCSwitch.h

We are also using inih (INI Not Invented Here) as the .INI file parser
	https://github.com/benhoyt/inih
	(Files included here: INIReader.cpp, INIReader.h, ini.c and ini.h)

Installation steps:
-------------------

1. Edit lights433.conf
(take are with entries as little error checking is performed)

2. Install with:
	make clean
	make
	sudo make install 
	sudo cp lights433.conf /etc/

3. Manually start with:
	sudo /usr/local/bin/lights433 &

4. Auto-run upon reboot with "sudo crontab -e" and add:
  @reboot /usr/local/bin/lights433 &

5. Inspect log file with:
	tail -30  /var/log/lights433.log

	