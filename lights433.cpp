/*
lights433.cpp 

Sunset calculations are performed with the AstroCalc4R library
  http://www.nefsc.noaa.gov/AstroCalc4R/ 
  Required files: AstroCalc4R.c and myfuncs1.c

Required library: WiringPi 
  https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/
  git clone git://git.drogon.net/wiringPi
  Link with flag: -lwiringPi

Used code from 'codesend' and 'RCSwitch' (part of the 433Utils library)
  https://github.com/ninjablocks/433Utils

Send 433 MHz codes using a Raspberry Pi
  Pins are: https://projects.drogon.net/raspberry-pi/wiringpi/pins/
  We will use GPIO0, wiringPi Pin 0, BCM GPIO 17, physical pin 11 (confusing!!)

Install with:
  make clean
  make all
  sudo make install 

Auto-run with "sudo crontab -e"
  @reboot /usr/local/bin/lights433 &

Usage: sudo /usr/local/bin/lights433
*/

#include "lights433.h" 
using namespace std;
using std::chrono::system_clock;

// **********************************************************************
//     Global variables
// **********************************************************************

// Arrays to store switch on/off codes and whether they should be contolled
int code_on  [7];
int code_off [7];
bool contolled_switches [7];

// Variables specific to current location (used in AstroCalc4R)
double xlat;   // Latitude
double xlon;   // Longitude
int    tzone;  // Hours from GST (e.g. EST = -5)

// Pin to use as output. GPIO0 is physical pin 11 on rev B 
// Read about pins here: http://wiringpi.com/pins/
int PIN;

// Variables to control lights on/off cycle
int on_hour;    // Time to switch off; hour (24 hour format)
int on_min;     //                     min  (24 hour format)
int on_offset;  // offset ( in minutes) before/after on time
int off_hour;   // Hour to switch off; hour (24 hour format)
int off_min;    //                     min  (24 hour format)
int off_ofset;  // Randomized offset (in minutes)


// **********************************************************************
//    main
// **********************************************************************
int main(int argc, char *argv[]) {
 
  bool lights_are_on;       // flag to keep track of the status of the lights
  int status;               // flag to determine whether the time is within a range
  char buffer [CHARSIZE];   // character buffer for output

  // write to the log file that the program is starting 
  logthis("*******************************************");
  logthis("Starting program lights433 ....");

  // Read the initialization file, named 
  read_ini_file("/etc/lights433.conf");
  logthis("- Reading the configuration file");

  // Initialize wiringPi
  wiringPiSetup ();
  logthis("- Initializing the wiringPi library");

  // Switch off the lights 
  logthis("- Make sure that lights are off");
  switch_lights(LIGHTS_OFF);
  lights_are_on = false;

  int daynum       = daynumber( time(NULL) );   // The current day number of the year
  time_t t_sunset  = calc_sunriseset(SUNSET);   // Calculate time of sunset
  time_t t_ontime  = calc_ontime(t_sunset);     // Calculate time to switch on the lights
  time_t t_offtime = calc_offtime(t_sunset);    // Calculate time to switch off the lights 
  
  // Enter an infinate loop
  while( 1 )
  { 
    // if it is past midnight AND the lights are off, then recalculate 
    if ((daynumber( time(NULL) ) != daynum) && lights_are_on == false) 
    {
      daynum    = daynumber( time(NULL) );
      t_sunset  = calc_sunriseset(SUNSET);
      t_ontime  = calc_ontime(t_sunset);  
      t_offtime = calc_offtime(t_sunset); 
    }

    status = time_in_range(t_ontime, t_offtime);
      #ifdef VERBOSE
      cout << "On time: "  << std::asctime(std::localtime(&t_ontime)) << \
              "Off time: " << std::asctime(std::localtime(&t_offtime)) << \
              currentDateTime() << ": Status: " << status << \
              "  On: " << lights_are_on << endl; 
      #endif
    switch (status) {
      // ************ time is within the range *****************
      case 1:   
        // if the lights are off, then switch them on 
        if (lights_are_on == false) {
          logthis("Switching on the lights ");
          switch_lights(LIGHTS_ON);
          lights_are_on = true;
        }
        // however, if the lights are on, then proceed
        break;
      
      // ************ time is outside of the range **************
      case 0:
        // if the lights are on, then switch them off
        if (lights_are_on == true) {
          logthis("Switching off the lights ");
          switch_lights(LIGHTS_OFF);
          lights_are_on = false;
        }
        // however, if lights are off, then proceed
        break;
    }  // end of switch (status)
    // wait a bit before repeaing the infinate loop
    std::this_thread::sleep_for(std::chrono::milliseconds(CYCLE));
  } // end of infinate loop 
  return 0;
} /* ***** end of main() ****** */


// **********************************************************************
//      Minimalist logging system
// **********************************************************************
int logthis(std::string messg)
{
  char buffer [CHARSIZE];     // character buffer for output

  std::string logfile = "/var/log/lights433.log";
  std::string logmessg = currentDateTime();
  logmessg = logmessg + ": " + messg; 

  std::ofstream myfile;
  myfile.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
  try {
    myfile.open (logfile, std::ofstream::out | std::ofstream::app);
    if (myfile.is_open())  {
        myfile << logmessg << endl;
        myfile.close();
    }
    // log all messages also to screen if VERBOSE is set
    #ifdef VERBOSE
    std::cout << logmessg << endl;
    #endif
  }
  catch (std::ifstream::failure e) {
    strcpy (buffer, "ERROR: Exception opening/reading/closing the log file.");
    logthis(buffer);
    std::cerr << "Exception opening/reading/closing the log file\n";
    return 1;
  }
  return 0; 
}


// **********************************************************************
//      Function to determine if the current time is within a given range
// **********************************************************************
int time_in_range(time_t a, time_t b)
{
  std::time_t tnow = std::time(NULL); 

  if ( (difftime(tnow, a) > 0) && (difftime(b, tnow) > 0) )
    return 1;
  else 
    return 0;
}

// **********************************************************************
//      Function to calculate the day number of the year
// **********************************************************************
int daynumber(time_t timestamp) 
{
  struct tm *tml = localtime(&timestamp);
  int dayOfMonth = tml->tm_mday;
  int month      = tml->tm_mon+1;
  int year       = tml->tm_year+1900;

  int currentDay = dayOfMonth + ((month < 3) ?
  (int)((306 * month - 301) / 10) :
  (int)((306 * month - 913) / 10) + ((year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 60 : 59));
  return currentDay;
}


// **********************************************************************
//      Get current date/time, format is YYYY-MM-DD.HH:mm:ss
// **********************************************************************
std::string currentDateTime() {
  std::time_t tt = system_clock::to_time_t (system_clock::now());
  std::string ret = std::ctime(&tt);
  ret.erase(ret.length()-1);  // remove the \n at the end of string
  return ret;
}

// **********************************************************************
//  Switch lights on/off, depending on LIGHTS_ON/OFF flag (defined in .h file)
// **********************************************************************
int switch_lights(int flag)
{
  int ret, i = 0;
  for ( bool b : contolled_switches ) {
    if (b) {
      switch (flag) {
        case LIGHTS_ON:
          ret = send_code(code_on [i]);
          break;
        default:
          ret = send_code(code_off[i]);
          break;
      }
    }
    i+=1;
  }
  return (ret);
}


// **********************************************************************
//      Function to send the 433MHz signal. Needs wiringPi library.
// **********************************************************************
int send_code(int code)
{  
  int ret = 0;
  char buffer [CHARSIZE];     // character buffer for output
    
    #ifdef SEND
    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableTransmit(PIN);
    mySwitch.send(code, 24);  
    #endif /* SEND */
        
    std::sprintf (buffer, "   Sending code: %d", code);
    logthis(buffer);

    std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    return ret;
  }

 
// **********************************************************************
//      Function to determine the time to switch on the lights
// **********************************************************************
time_t calc_ontime(time_t sunset)
{ 
  char buffer [CHARSIZE];
  int offset = on_offset; // time before/after sunset in minutes
  struct tm *tml = localtime(&sunset);
  tml->tm_min  = tml->tm_min + offset;
  sunset = std::mktime(tml);
  
  // write to the log file
  strftime(buffer,CHARSIZE,"The time to switch on is:  %d-%m-%Y %H:%M:%S%p",tml);
  logthis(buffer);

  return (sunset); 
}

// **********************************************************************
//      Function to determine the time to switch the lights off
// **********************************************************************
time_t calc_offtime(time_t sunset)
{ 
  char buffer [CHARSIZE];
  struct tm *tml = localtime (&sunset);
  // generate random number
  std::srand(time(NULL));
  int offset = rand() % off_ofset + 1; // Random time in minutes after after offtime
  tml->tm_hour = off_hour;   // Hour at which to switch off (24 hour format)
  tml->tm_min  = off_min + offset;
  sunset = std::mktime(tml);

  // write to the log file
  std::strftime(buffer,CHARSIZE,"The time to switch off is: %d-%m-%Y %H:%M:%S%p",tml);
  logthis(buffer);

  return (sunset);
}

// **********************************************************************
//      Function to determine today's sunset time
// **********************************************************************
time_t calc_sunriseset(int value)
{
  char buffer [CHARSIZE];     // character buffer for output
  time_t dt_sunrise;
  time_t dt_sunset;

  // Variables used by AstroCalc4R
  int nrec  =  1;   // some kind of counter
  int day, month, year; 
  double hhour, noon, astro_sunrise, astro_sunset, azimuth, zenith, eqtime, declin, daylength, par;

  // temporary values for sunset/sunrise based on current date/time of system
  dt_sunrise   = time(0);
  dt_sunset    = time(0);
  tm *sunrise  = localtime(&dt_sunrise);
  tm *sunset   = localtime(&dt_sunset);
  int daylight_savings = sunset->tm_isdst;

  // set up the values for a function call to AstroCalc4R
  day   = sunset->tm_mday;        // day of month
  month = 1 + sunset->tm_mon;     // month
  year  = 1900 + sunset->tm_year; // year
  hhour = sunset->tm_hour;        // hour in day

  AstroCalc4R(&nrec, &tzone, &day, &month, &year, &hhour, &xlat, &xlon, \
         &noon, &astro_sunrise, &astro_sunset, &azimuth, &zenith, \
         &eqtime, &declin, &daylength, &par);  
  
  // Calculate the sunrise time
  sunrise->tm_hour = floor(astro_sunrise); 
  if (daylight_savings == 1) {
    sunrise->tm_hour += 1;
  }
  sunrise->tm_min  = 60*(astro_sunrise-floor(astro_sunrise));
  dt_sunrise = std::mktime(sunrise);

  // Calculate the sunset time
  sunset->tm_hour = floor(astro_sunset); 
  if (daylight_savings == 1) {
    sunset->tm_hour  += 1;
  }
  sunset->tm_min  = 60*(astro_sunset-floor(astro_sunset));
  dt_sunset = std::mktime(sunset);

  // adust for daylight savings 


  // log the results
  struct tm *tml = localtime(&dt_sunrise);
  strftime(buffer,CHARSIZE,"Sunrise is at: %d-%m-%Y %H:%M:%S%p",tml);
  logthis(buffer);
  tml = localtime(&dt_sunset);
  strftime(buffer,CHARSIZE,"Sunset is at:  %d-%m-%Y %H:%M:%S%p",tml);
  logthis(buffer);

  if (value == SUNSET)
    return dt_sunset;
  else
    return dt_sunrise;
} 

  int read_ini_file(string filename) {

    INIReader reader(filename);

    if (reader.ParseError() < 0) {
        std::cout << "Can't load " << filename << endl;
        return 1;
    }

    code_on[0] = reader.GetInteger("switch_01",  "on_code", -1);
    code_on[1] = reader.GetInteger("switch_02",  "on_code", -1);
    code_on[2] = reader.GetInteger("switch_03",  "on_code", -1);
    code_on[3] = reader.GetInteger("switch_04",  "on_code", -1);
    code_on[4] = reader.GetInteger("switch_05",  "on_code", -1);
    code_on[5] = reader.GetInteger("switch_06",  "on_code", -1);
    code_on[6] = reader.GetInteger("switch_ALL", "on_code", -1);

    code_off[0] = reader.GetInteger("switch_01",  "off_code", -1);
    code_off[1] = reader.GetInteger("switch_02",  "off_code", -1);
    code_off[2] = reader.GetInteger("switch_03",  "off_code", -1);
    code_off[3] = reader.GetInteger("switch_04",  "off_code", -1);
    code_off[4] = reader.GetInteger("switch_05",  "off_code", -1);
    code_off[5] = reader.GetInteger("switch_06",  "off_code", -1);
    code_off[6] = reader.GetInteger("switch_ALL", "off_code", -1);

    contolled_switches [0] = reader.GetBoolean("switch_01", "controlled", true);
    contolled_switches [1] = reader.GetBoolean("switch_02", "controlled", true);
    contolled_switches [2] = reader.GetBoolean("switch_03", "controlled", true);
    contolled_switches [3] = reader.GetBoolean("switch_04", "controlled", true);
    contolled_switches [4] = reader.GetBoolean("switch_05", "controlled", true);
    contolled_switches [5] = reader.GetBoolean("switch_06", "controlled", true);
    contolled_switches [6] = reader.GetBoolean("switch_ALL", "controlled", true);

    PIN = reader.GetInteger("GPIO0", "pin", -1);

    // Variables specific to current location (used in AstroCalc4R)
    xlat   = reader.GetReal("location", "latitude",    -1);    // Chappaqua, NY is Latitude:   41.157775
    xlon   = reader.GetReal("location", "longitude",   -1) ;   //                  Longitude: -73.788873
    tzone  = reader.GetInteger("location", "timezone", -1);    // Hours from GST (EST = -5)

    // Variables to control lights on/off cycle
    std::string s_ontime = reader.Get("Cycle_01", "on_time", "UNKNOWN");
    std::string s_offtime = reader.Get("Cycle_01", "off_time", "UNKNOWN");
    on_offset = reader.GetInteger("Cycle_01", "on_offset", -1);              // minutes before sunset
    off_ofset  = reader.GetInteger("Cycle_01", "off_offset", -1); 

    std::string delimiter = ":";
    on_hour =  std::stoi(  s_ontime.substr(0, s_ontime.find(delimiter)) );
    std::string s = s_ontime.substr( s_ontime.find(delimiter)+1, s_ontime.length() );
    on_min  =  std::stoi(  s ) ;

    try {
      off_hour =  std::stoi(  s_offtime.substr(0, s_offtime.find(delimiter)) );
      s = s_offtime.substr( s_offtime.find(delimiter)+1, s_offtime.length() );
      off_min  =  std::stoi(  s ) ;
    }
    catch (const std::invalid_argument& ia) {
      std::cerr << "Invalid argument in ini file: " << ia.what() << '\n';
    }
    
    return 0;
  }