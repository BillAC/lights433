/*
** Functions written by Al Seaver for calculations in the 
** AstroCalc program.  These functions are inserted into
** the main body of the AstroCalc function so that their
** scope is limited and R can't see them.
*/

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
