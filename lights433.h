/* 
	lights433.h
*/
#define RPI 1
#include "../433Utils/rc-switch/RCSwitch.h"
#include "INIReader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip> 
#include <thread>	
#include <sstream>
//#include "easylogging++.h"    // logging: https://github.com/easylogging/easyloggingpp
//INITIALIZE_EASYLOGGINGPP

#define SEND 1
//#define VERBOSE 1
#define LIGHTS_ON 1
#define LIGHTS_OFF 0
#define SUNRISE 1
#define SUNSET 2
#define CHARSIZE 80
#define CYCLE 60000		// Cycle time in ms 
#define DELAY 5000		// Delay between 433MHz signals (in ms)

time_t calc_sunriseset ( int );
time_t calc_ontime ( time_t );
time_t calc_offtime( time_t );
int time_in_range(time_t, time_t);
int switch_lights( int );
int send_code( int );
int daynumber( time_t ); 
std::string currentDateTime(void);
int logthis(std::string);
int read_ini_file(std::string);
void AstroCalc4R(int *, int *, int *,int *,int *, double *,double *,double *, \
				double *,double *,double *,double *,double *, double *,double *, double *, double *);
