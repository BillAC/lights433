Controlling 433MHz RF outlets with Raspberry Pi
-----------------------------------------------

This project is designed to control up to six wireless outlets connected to
lights. They will switch on automatically around sunset each day and switch 
off at a specified time. The off time can be randomized for security 
purposes (e.g. when you are not at home).  

I am using [Bye Bye Standby Wireless Remote Control Energy 
Saving Kit 6 outlets](http://www.amazon.com/Bye-Standby-Wireless-Control-outlets/dp/B00F4AQKRI) by Bye Bye Standby ($27.95 on Amazon)

You need a 433MHz receiver/transmitter as described here (and many other places):
- [Decode 433 MHz signals](http://www.princetronics.com/how-to-read-433-mhz-codes-w-raspberry-pi-433-mhz-receiver/)
- [Adding 433 to your Raspberry Pi](http://shop.ninjablocks.com/blogs/how-to/7506204-adding-433-to-your-raspberry-pi)
- [Control anything electrical with the Raspberry Pi using 433 MHz RF](http://stevenhickson.blogspot.com/2015/02/control-anything-electrical-with.html)
- [Turning mains powered appliances on and off](http://www.hoagieshouse.com/RaspberryPi/RCSockets/RCPlug.html) 

Requirements and dependencies 
-----------------------------

Required library: [WiringPi](https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/). Read about this [GPIO Interface library for the Raspberry Pi](http://wiringpi.com/download-and-install/) and get the code [here](git://git.drogon.net/wiringPi). Remember to link with flag: -lwiringPi. 

We are using the @ninjablocks [433Utils library](https://github.com/ninjablocks/433Utils). The directory structure is as follows (change the file locations in the Makefile and in lights433.h is yours is different):

	base directory
 		- 433Utils (subdirectory)
 		- lights433 (subdirectory for our code)

As the .INI file parser, we are using [inih](https://github.com/benhoyt/inih) (INI Not Invented Here) from @benhoyt. Files included here are: INIReader.cpp, INIReader.h, ini.c and ini.h). 

Sunset calculations are performed with the [AstroCalc4R library](http://www.nefsc.noaa.gov/AstroCalc4R/). Files included here are: AstroCalc4R.c and myfuncs1.c

Installation steps:
-------------------

1. Edit lights433.conf
(take care with entries as little error checking is performed)

2. Install with:
```
make clean
make
sudo make install 
sudo cp lights433.conf /etc/
```
3. Manually start with:
	`sudo /usr/local/bin/lights433 &`

4. Auto-run upon reboot with `sudo crontab -e` and add:
  `@reboot /usr/local/bin/lights433 &`

5. Inspect log file with:
	`tail -30  /var/log/lights433.log`

	