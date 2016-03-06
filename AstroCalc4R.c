/*
**  AstroCalc4R.c
**	Version 1.0 (March 7, 2011)
**
**  27 July 2010 - A.W. Seaver
**  Astronomical Calculations for use with Survey databases
**  Based on "Astronomical Algorithms" by Jean Meeus, 2nd Edtion, August 2009
**  Recommended by NOAA ESRL  - GMD "NOAA Solar Calculator"
*/

/*
**  17 February 2011 - L. Tang and L. Jacobson
**  Modified to be easier to call from R using the .C function.
**  In particular, all functions visible to R are of type VOID.
**  Also modified to accept vector arguments as pointers and
**  to return vector results.  The easiest way to use this code
**  from R is as a AstroCalcV.dll in Windows for AstroCalcV.so in
**  linus. 
**  
*/

#include "AstroCalc4R.h"
using namespace std;

int isleap(int year)
{
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		return 1;
	else
		return 0;
}

int daymonth(int month, int year)
{
	int leap;

	static int daytab[2][13] =
	{
		{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
	};


	leap = isleap(year);

	return daytab[leap][month];
}

double JulianDay(double xday,double xmonth,double xyear)
{
	int mm;
	double xa, xb;
	double jd;

	/* Calculate Julian Day Starting at 4712 BC
	** Method from "Astronomical Algorithms" P. 61
	*/

	mm = (int) xmonth;

	if (mm <= 2)
	{
		xmonth = xmonth + 12;
		xyear = xyear - 1.0;
	}

	xa = floor(xyear / 100.0);
	xb = 2.0 - xa + floor(xa / 4.0);

	jd = floor(365.25 * (xyear + 4716.)) + floor(30.6001 * (xmonth + 1.0)) + xday + xb - 1524.5;

	return jd;
}
double EquationTime(double epsilon, double sl, double eeo, double sa)
{
	/* Calculate Equation of Time
	** "Astronomical Algoritms" Eq. 28.3
	**
	**
	**  epsilon - Obliquity of the Ecliptic (radians)
	**  sl      - Sun's Mean Longitude      (radians)
	**  eeo     - Eccentricity of Earth's Orbit
	**  sa      - Sun's Mean Anomaly        (radians)
	*/

    double XDEGRAD=3.141592654 / 180.;
	
	double xx, yy, xx2, yy2;
	double e;

	xx = epsilon / 2.0;

	yy = tan(xx) * tan(xx);

	yy2 = yy * yy / 2.0;

	xx2 = 1.25 * eeo * eeo;

	e = yy * sin(2.* sl) - 2.0 * eeo * sin(sa) + 4.0 * eeo * yy * sin(sa) * cos(2. * sl) - yy2 * sin(4. * sl) - xx2 * sin(2. * sa);

	/* convert to degrees */

	e = e / XDEGRAD;

	/* Convert to minutes */

	e = e * 4.0;

	return e;

}

/* 
**  Calculate Photosynthetically Available Radiation (PAR - Watts / M2)
**
**  Basis: Fouin, et al "Journal of Geophysical Research, Vol 94, No. C7 pp. 9731-9742 
**  July 15, 1989  
**
**  Equation 6
**
**  equation 1 in frouin et al. 1989 paper should be (d0/d)2 not as it is in the paper (d/d0)2  
**  in their equations 1 and 6
**  email from r.frouin to j.o'reilly, oct 28,2003:
**    "there is a typo in the equation, it's actually (do/d)2. obviously when the actual distance (d) is larger than the average distance
**    "(do), the correction factor should be smaller (less irradiance since the sun is farther away)."
**
*/

double parcalc(double zenith)
{
/*
** 
**  Maritime 400-700nm
**
**  I0 = 531.2
**
**  V= 23 km
**
**  a = 0.068
**  b = 0.379
**  a' = 0.117
**  b' = 0.493
**  av = 0.002
**  bv = 0.87
**  a0 = 0.052
**  b0 = 0.99
**
**  uv = 1.4
**  u0 = 0.34
**  r = 0.05
**  d = d0 = 1.0
**
**  (do/d)^2 = 1.0

*/
    const double XDEGRAD=3.141592654 / 180.;
	double par;
	double zrad;
	double x1, x2;
	double xx, xxx;
	double xa, xb;

	if (zenith > 89.9999)
		return 0.0;

	zrad = zenith * XDEGRAD;
	x1 = 1.4 / cos(zrad);
	xx = exp(-0.002 * pow(x1,0.87));
	x2 = 0.34 / cos(zrad);
	xxx = exp(-0.052 * pow(x2,0.99));
	xa = 0.068 + 0.379 / 23.0;
	xb = 1.0 - 0.05 * (0.117 + 0.493 / 23.0);
	par = 531.2 * cos(zrad) * exp(-(xa) / cos(zrad)) / xb  * xx * xxx;

	return par;		 
}


void AstroCalc4R(int *nrec, int *tzone, int *day,int *month,int *year, double *hhour,double *xlat,double *xlon, \
				 double *noon,double *sunrise,double *sunset,double *azimuth,double *zenith, \
				 double *eqtime,double *declin, double *daylength, double *par)
{ 

/* 
** XDEGRAD is used by AstroCalc and by functions that it calls. 
*/
    const double XDEGRAD=3.141592654 / 180.;

	int dm;
	double xd, xm, xy;
	double jd;
	double jc;
	double xx;
	double yy;
	double gmls;
	double gmas;
	double eeo;
	double scx;
	double stl;
	double omega;
	double lambda;
	double epsilon;
	double oblx;
	double gamma;
	double etime;
	double hzero;
	double hangle;
	double phi;
	double tst;
	double tsa;
	double elev;

/* Scalar values extracted from vectors passed to this 
   function */
	double daytemp;
	double monthtemp;
	double yeartemp;
	double hhourtemp;
	double xlattemp;
	double xlontemp;

	for (int i=0; i<*nrec; i++)
	{
		daytemp = day[i];
		monthtemp = month[i];
		yeartemp = year[i];
		hhourtemp = hhour[i];
		xlattemp = xlat[i];
		xlontemp = xlon[i];

		/* Corrrect Time for GMT */

		hhourtemp = hhourtemp - (double) *tzone;

		if (hhourtemp > 24.0)
		{
			hhourtemp = hhourtemp - 24.0;
			dm = daymonth(monthtemp,yeartemp);
			if (daytemp < dm)
				daytemp++;
			else
			{
				daytemp = 1;
				if (monthtemp < 12)
					monthtemp++;
				else
				{
					monthtemp = 1;
					yeartemp++;
				}
			}
		}

		/* Calculate Julian Day Starting at 4712 BCE
		** Method from "Astronomical Algorithms" P. 61
		*/

		xy = (double) yeartemp;
		xm = (double) monthtemp;
		xd = (double) daytemp + hhourtemp / 24.0; 


		jd = JulianDay(xd,xm,xy);

		/*  Calculate Julian Century
		**  "Astronomical Algoritms" Eq. 25.1
		*/

		jc = (jd - 2451545.0) / 36525.0;

		/* Calculate Geometric Mean Longitude of the Sun
		** "Astronomical Algoritms" Eq. 25.2
		*/

		xx = 280.46646 + jc * (36000.76983 + 0.0003032 * jc);
		gmls = fmod(xx,360.0);

		/* Calculate Mean Anomaly of the Sun
		** "Astronomical Algoritms" Eq. 25.3
		*/

		xx = 357.52911 + jc * (35999.05029 - 0.0001537 * jc);
		gmas = fmod(xx,360.0);

		/* Calculate Eccentricity of the Earth's orbit
		** "Astronomical Algoritms" Eq. 25.4
		*/

		eeo = 0.016708634 - jc * (0.000042037 + 1.267E-07 * jc); 
		/* Calculate Sun's Equation of the Center
		** "Astronomical Algoritms" p. 164
		*/

		xx = gmas * XDEGRAD;
		scx = (1.914602 - jc * (0.004817 + 1.4E-05 * jc)) * sin(xx);
		scx += (0.019993 - 0.000101 * jc) * sin(2.0*xx);
		scx += 0.000289 * sin(3.0*xx);

		
		/* Calculate Sun's True Longitude &
		** Sun's True Anomaly
		** "Astronomical Algoritms" p. 164
		*/

		stl = gmls + scx;

		/* Calculate the Sun's Radius Vector
		** "Astronomical Algoritms" Eq. 25.5
		*/


		/* Calculate Sun's Apparent Longitude
		** "Astronomical Algoritms" p. 164
		*/

		omega = 125.04 - 1934.136 * jc;
		omega = omega * XDEGRAD;
		lambda = stl - 0.00569 - 0.00478 * sin(omega);

			/* Calculate Sun's Equation of the Center
		** "Astronomical Algoritms" p. 164
		*/

		xx = gmas * XDEGRAD;
		scx = (1.914602 - jc * (0.004817 + 1.4E-05 * jc)) * sin(xx);
		scx += (0.019993 - 0.000101 * jc) * sin(2.0*xx);
		scx += 0.000289 * sin(3.0*xx);

		/* Calculate Sun's True Longitude &
		** Sun's True Anomaly
		** "Astronomical Algoritms" p. 164
		*/

		stl = gmls + scx;

		/* Calculate the Sun's Radius Vector
		** "Astronomical Algoritms" Eq. 25.5
		*/


		/* Calculate Sun's Apparent Longitude
		** "Astronomical Algoritms" p. 164
		*/

		omega = 125.04 - 1934.136 * jc;
		omega = omega * XDEGRAD;
		lambda = stl - 0.00569 - 0.00478 * sin(omega);

		/* Calculate Mean Obliquity of the Ecliptic
		** "Astronomical Algoritms" Eq. 22.2
		*/

		epsilon = 23.0 + (26 + ((21.448 - jc * (46.8150 + jc * (0.00059 - 0.001813 * jc)))) / 60.) / 60.;


		/* Calculate Obliquity Correction 
		** "Astronomical Algoritms" Eq. 25.8
		*/

		oblx = 0.00256 * cos(omega);

		epsilon = epsilon + oblx;

		/* Calculate Sun's Right Ascension
		** "Astronomical Algoritms" Eq. 25.6
		*/

		lambda = lambda * XDEGRAD;
		epsilon = epsilon * XDEGRAD;


		/* Calculate Sun's Declination
		** "Astronomical Algoritms" Eq. 25.7
		*/

		xx = sin(epsilon) * sin(lambda);

		gamma = asin(xx);
		declin[i] = gamma / XDEGRAD;

			/* Calculate Equation of Time
		** "Astronomical Algoritms" Eq. 28.3
		*/

		xx = gmls * XDEGRAD;
		yy = gmas * XDEGRAD;

		etime = EquationTime(epsilon,xx,eeo,yy);

		eqtime[i] = etime;

		/* Calculate Hour Angle
		** "Astronomical Algoritms" Eq. 15.1
		*/

		/* Standard value for hzero = -0.83333 degrees */

		hzero = -0.83333 * XDEGRAD;

		phi = xlattemp * XDEGRAD;

		xx = (sin(hzero) - sin(phi) * sin(gamma)) / cos(phi) / cos(gamma);
		
		hangle = acos(xx);

		hangle = hangle / XDEGRAD;

		/* Calculate the Solar Noon (LST)
		** Each 15 Degrees of Longitude = 1 Hour
		** Each Time zone = 1 hour
		** 1440 Minutes in Day
		*/

		xx = (double) *tzone * 60.0;

		noon[i] = (720. - 4.0 * xlontemp + xx - etime) / 1440.0;


		/* Calculate Sunrise & Sunset */
		

		sunrise[i] = ((noon[i] * 1440. - hangle * 4.0) / 1440.0) * 24. ;
		sunset[i]  = ((noon[i] * 1440. + hangle * 4.0) / 1440.0) * 24. ;
	    noon[i] = noon[i] * 24. ;

		/* Calculate Length of Day */

		daytemp = hangle * 8.0;

		/* Calculate True Solar Time (minutes) */

		xx = hhourtemp * 60.0 + etime + 4.0 * xlontemp;
		tst = fmod(xx,1440.0);

		/* Calculate the True Solar Angle (degrees) */

		if (tst < 0.0)
			tsa = tst / 4.0 + 180.0;
		else
			tsa = tst / 4.0 - 180.0;


		/* Calculate Zenith
		** "Astronomical Algoritms" Eq. 13.6
		*/

		xx = tsa * XDEGRAD;

		yy = sin(phi) * sin(gamma) + cos(phi) * cos(gamma) * cos(xx);

		xx = asin(yy);

		elev = xx / XDEGRAD;

		zenith[i] = 90.0 - elev;

		/* Calculate Azimuth (degress clockwise from N 
		** "Astronomical Algoritms" P. 94
		*/

		yy = (sin(phi) * sin(xx) - sin(gamma)) / cos(phi) / cos(xx);
		
		xx = acos(yy) / XDEGRAD;

		xx = xx + 180.0;

		if (tsa > 0.0)
			azimuth[i] = fmod(xx,360.0);
		else
			azimuth[i] = 360.0 - fmod(xx,360.0);

		daylength[i] = daytemp / 60.0;	
		
		par[i] = parcalc((double) zenith[i]);
    }	
	
}