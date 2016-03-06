/* 
	AstroCalc4R.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int isleap(int);
int daymonth(int, int);
double JulianDay(double,double,double);
double EquationTime(double, double, double, double);
double parcalc(double);
void AstroCalc4R(int *, int *, int *,int *,int *, double *,double *,double *, \
				 double *,double *,double *,double *,double *, \
				 double *,double *, double *, double *);
