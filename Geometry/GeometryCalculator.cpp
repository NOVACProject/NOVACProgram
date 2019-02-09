#include "StdAfx.h"
#include "geometrycalculator.h"

#include "../Common/Common.h"

#include "../VolcanoInfo.h"

#include "../Common/EvaluationLogFileHandler.h"

using namespace Geometry;

// The global list of volcanoes in the NOVAC network
extern CVolcanoInfo g_volcanoes;

CGeometryCalculator::CGeometryCalculator(void)
{
}

CGeometryCalculator::~CGeometryCalculator(void)
{
}



CGeometryCalculator::CGeometryCalculationInfo::CGeometryCalculationInfo(){
	Clear();
}
CGeometryCalculator::CGeometryCalculationInfo::~CGeometryCalculationInfo(){
}
void CGeometryCalculator::CGeometryCalculationInfo::Clear(){
	for(int k = 0; k < 2; ++k){
		scanner[k].m_altitude		=	0;
		scanner[k].m_latitude		= 0.0;
		scanner[k].m_longitude	= 0.0;
		plumeCentre[k]					= 0.0;
	}
}
Geometry::CGeometryCalculator::CGeometryCalculationInfo &CGeometryCalculator::CGeometryCalculationInfo::operator =(const Geometry::CGeometryCalculator::CGeometryCalculationInfo &info2){
	for(int k = 0; k < 2; ++k){
		scanner[k]			= info2.scanner[k];
		plumeCentre[k]	= info2.plumeCentre[k];
	}
	return *this;
}

int CGeometryCalculator::GetNearestVolcano(double lat, double lon){
	double distance = 1e56; // the distance to the nearest volcano, in meters
	int index = -1;
	Common common;

	for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
		double tmpDist = common.GPSDistance(g_volcanoes.m_peakLatitude[k], g_volcanoes.m_peakLongitude[k], lat, lon);

		if(tmpDist < distance){
			distance = tmpDist;
			index = k;
		}
	}

	return index;
}

/** Rotates the given vector the given angle [degrees] around the given axis
		@param vec - the coordiates of the vector
		@param angle - the angle to rotate, in degrees
		@param axis - the axis to rotate around (1,2 or 3) */
void CGeometryCalculator::Rotate(double vec[3], double angle, int axis){
	double COS = cos(angle * DEGREETORAD);
	double SIN = sin(angle * DEGREETORAD);
	double a = vec[0], b = vec[1], c = vec[2];

	if(axis == 1){
		/** Rotation around X - axis*/
		a = vec[0];
		b = COS * vec[1] + SIN * vec[2];
		c = -SIN * vec[1] + COS * vec[2];
	}else if(axis == 2){
		/** Rotation around Y - axis*/
		a = COS * vec[0] - SIN * vec[2];
		b = vec[1];
		c = SIN * vec[0] + COS * vec[2];
	}else if(axis == 3){
		/** Rotation around Z - axis*/
		a = COS * vec[0] + SIN * vec[1];
		b = -SIN * vec[0] + COS * vec[1];
		c = vec[2];
	}

	vec[0] = a;
	vec[1] = b;
	vec[2] = c;
}

/** Calculates the parameters t1 and t2 so that the lines 'origin1 + t1*direction1'
		intersects the line 'origin2 + t2*direction2'. If the lines cannot intersect
		t1 and t2 define the points of closest approach.
		If the lines are parallel, t1 and t2 will be set to 0 and the function will return false.
		@origin1 - the origin of the first ray
		@direction1 - the direction of the first ray, should be normalized
		@origin2 - the origin of the second ray
		@direction2 - the direction of the second ray, should be normalized
		@t1 - will on return be the parameter t1, as defined above
		@t2 - will on return be the parameter t2, as defined above
		@return true if the rays do intersect
		@return false if the rays don't intersect */
bool	CGeometryCalculator::Intersection(const double o1[3], const double d1[3], const double o2[3], const double d2[3], double &t1, double &t2){
	double eps = 1e-19;
	double d1_cross_d2[3], point1[3], point2[3];	
	double o2_minus_o1[3];

	// calculate the cross-product (d1 x d2)
	Cross(d1, d2, d1_cross_d2);

	// calculate the squared norm: ||d1 x d2||^2
	double N2 = Norm2(d1_cross_d2);

	if(fabs(N2) < eps){
		/** The lines are parallel */
		t1 = 0;		t2 = 0;
		return false;
	}

	// calculate the distance between the origins
	o2_minus_o1[0] = o2[0] - o1[0];
	o2_minus_o1[1] = o2[1] - o1[1];
	o2_minus_o1[2] = o2[2] - o1[2];

	// Calculate the first determinant
	double det1 = Det(o2_minus_o1, d2, d1_cross_d2);

	// Calculate the second determinant
	double det2 = Det(o2_minus_o1, d1, d1_cross_d2);

	// The result...
	t1 = det1 / N2;
	t2 = det2 / N2;

	// See if the lines do intersect or not
	PointOnRay(o1, d1, t1, point1);
	PointOnRay(o2, d2, t2, point2);

	if(fabs(point1[0] - point2[0]) > eps && fabs(point1[1] - point2[1]) > eps && fabs(point1[2] - point2[2]) > eps)
		return false;

	return true;
}

/** Calculates the coordinates of the point (origin + t*direction) */
void CGeometryCalculator::PointOnRay(const double origin[3], const double direction[3], double t, double point[3]){
	for(int k = 0; k < 3; ++k)
		point[k] = origin[k] + t * direction[k];
}

/** Calculates the height of the plume given data from two scans
		@param gps - the gps-positions for the two scanning instruments 
				that collected the data
		@param compass - the compass-directions for the two scanning instruments 
				that collected the data. In degrees from north
		@param plumeCentre - the centre of the plume, as seen from each of
				the two scanning instruments. Scan angle, in degrees
		@param plumeHeight - will on return be filled with the calculated
				height of the plume above the lower of the two scanners
		@return true if a plume height could be calculated. */
bool CGeometryCalculator::GetPlumeHeight_Exact(const CGPSData gps[2], const double compass[2], const double plumeCentre[2], const double coneAngle[2], const double tilt[2], double &plumeHeight){
	double distance, bearing;
	double posLower[3] = {0, 0, 0}; // <-- the position of the lower scanner in our changed coordinate system
	double posUpper[3];						// <-- the position of the higher scanner in our changed coordinate system
	Common common;

	// 1. To make the calculations easier, we put a changed coordinate system
	//		on the lowest of the two scanners and calculate the position of the 
	//		other scanner in this coordinate system.
	int lowerScanner = (gps[0].m_altitude < gps[1].m_altitude) ? 0 : 1;
	int upperScanner = 1 - lowerScanner;

	// 2. The distance between the two systems
	distance = common.GPSDistance(gps[lowerScanner].m_latitude, gps[lowerScanner].m_longitude,
																gps[upperScanner].m_latitude, gps[upperScanner].m_longitude);

	// 3. The bearing from the lower to the higher system (degrees from north, counted clock-wise)
	bearing		= common.GPSBearing(gps[lowerScanner].m_latitude, gps[lowerScanner].m_longitude,
																gps[upperScanner].m_latitude, gps[upperScanner].m_longitude);

	// 4. The position of the upper scanner 
	posUpper[0]	= distance * cos(bearing * DEGREETORAD);
	posUpper[1]	= distance * sin(-bearing * DEGREETORAD);
	posUpper[2]	= gps[upperScanner].m_altitude - gps[lowerScanner].m_altitude;

	// 5. The directions of the two plume-center rays (defined in the coordinate systems of each scanner)
	double dirLower[3], dirUpper[3]; // <-- the directions
	GetDirection(dirLower, plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]);
	GetDirection(dirUpper, plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]);

	// 6. Find the direction of the plume-center ray of the upper scanner
	//		in the coordinate system of the lower scanner
	Rotate(dirUpper, compass[upperScanner] - compass[lowerScanner], 3);

	// 7. Find the position of the upper scanner in the coordinate-system
	//		of the lower scanner.
	CGeometryCalculator::Rotate(posUpper,		-compass[lowerScanner], 3);

	// 8. Calculate the intersection point of the two rays
	double t1, t2;
	Normalize(dirLower); 
	Normalize(dirUpper); 
	bool hit = Intersection(posLower, dirLower, posUpper, dirUpper, t1, t2);


	// 9. The plume-height (above the lower scanner) is the z-component of 
	//		the intersection-point
	if(hit){
		double intersectionPoint[3];
		PointOnRay(posLower, dirLower, t1, intersectionPoint); // <-- calculate the intersection point
		plumeHeight = intersectionPoint[2];
	}else{
		// if the rays don't actually hit each other, calculate the distance between
		//	them. If this is small enough let's consider them as a hit.
		double point1[3], point2[3];
		PointOnRay(posLower, dirLower, t1, point1); // <-- calculate the intersection point
		PointOnRay(posUpper, dirUpper, t2, point2); // <-- calculate the intersection point
		double distance2 = pow(point1[0] - point2[0], 2) + pow(point1[1] - point2[1], 2) + pow(point1[2] - point2[2], 2);
		if(distance2 > 1600)
			return false; // the distance between the intersection points is > 400 m!!

		// take the plume-height as the average of the heights of the two intersection-points
		plumeHeight = (point1[2] + point2[2]) * 0.5;
	}

	return true;
}

/** Calculates the height of the plume given data from two scans
		@param gps - the gps-positions for the two scanning instruments 
				that collected the data
		@param compass - the compass-directions for the two scanning instruments 
				that collected the data. In degrees from north
		@param plumeCentre - the centre of the plume, as seen from each of
				the two scanning instruments. Scan angle, in degrees
		@param plumeHeight - will on return be filled with the calculated
				height of the plume above the lower of the two scanners
		@return true if a plume height could be calculated. */
bool CGeometryCalculator::GetPlumeHeight_Fuzzy(const CGPSData source, const CGPSData gps[2], const double compass[2], const double plumeCentre[2], const double coneAngle[2], const double tilt[2], double &plumeHeight){
	Common common;

	// 1. To make the calculations easier, we put a changed coordinate system
	//		on the lowest of the two scanners and calculate the position of the 
	//		other scanner in this coordinate system.
	int lowerScanner = (gps[0].m_altitude < gps[1].m_altitude) ? 0 : 1;
	int upperScanner = 1 - lowerScanner;

	// 2. Find the plume height that gives the same wind-direction for the two instruments
	double guess		= 1000;	// the current guess for the plume height
	double h				= 10;		// the step we use when searching for the plume height
	double maxDiff	= 2;		// the maximum allowed difference in wind-direction, the convergence criterion

	// 2a. Make an initial guess of the plume height...
	if(gps[lowerScanner].m_altitude > 0 && source.m_altitude > 0){
		guess = min(5000, max(0, source.m_altitude - gps[lowerScanner].m_altitude));
	}

	// ------------------------ HERE FOLLOW THE NEW ITERATION ALGORITHM -------------------
	double f = 1e9, f_plus = 1e9;
	double f1, f2;
	int nIterations = 0;
	while(1){
		// Calculate the wind-direction for the current guess of the plume height
		f1 = GetWindDirection(source, guess,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]);
		f2 = GetWindDirection(source, guess,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]);
		f  = max(f1, f2) - min(f1, f2);
		if(f > 180.0)
			f = 360.0 - f;

		// Calculate the wind-direction for a plume height a little bit higher than the current guess of the plume height
		f1			= GetWindDirection(source, guess + h,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]);
		f2			= GetWindDirection(source, guess + h,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]);
		f_plus  = max(f1, f2) - min(f1, f2);
		if(f_plus > 180.0)
			f_plus = 360.0 - f_plus;

		// Check if we have a good enough result already
		if(f < maxDiff){
			plumeHeight = guess;
			if(plumeHeight < 0 || plumeHeight > 10000)
				return false;
			else
				return true;
		}else if(f_plus < maxDiff){
			plumeHeight = guess + h;
			if(plumeHeight < 0 || plumeHeight > 10000)
				return false;
			else
				return true;
		}

		// the local derivative
		double dfdx = (f_plus - f) / h; 

		// one step using the Newton method, make a line-search
		//	of the step-size to guarantee that we do decrease
		//	the difference at each step
		double alpha = 0.5;
		double newGuess = guess - alpha * f / dfdx;
		double f_new = fabs(GetWindDirection(source, newGuess,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]) - 
												GetWindDirection(source, newGuess,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]));
		int nIterations2 = 0;
		while(f_new > f){
			alpha			= alpha / 2;
			newGuess	= guess - alpha * f / dfdx;
			f_new			= fabs(GetWindDirection(source,		newGuess,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]) - 
												GetWindDirection(source,  newGuess,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]));
			if(nIterations2++ > 1000){
				return false;
			}
		}
		if(f_new < maxDiff){
			plumeHeight = newGuess;
			return true;
		}
		guess = newGuess;


		// Increase and check the number of iterations
		if(nIterations++ > 100){
			return false;
		}
	}

	// Return our guess
	plumeHeight = guess;

	return true;

	//// ------------------------ HERE FOLLOW THE OLD ITERATION ALGORITHM -------------------
	//// Calculate the local derivaitve
	//double diff			= fabs(GetWindDirection(source, guess,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]) -
	//								 GetWindDirection(source, guess,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]));
	//double diff2			= fabs(GetWindDirection(source, guess + h, gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]) -
	//								 GetWindDirection(source, guess + h, gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]));

	//if(diff == diff2)
	//	return false; // error

	//// The sign of the local derivative
	//int sign = (diff2 > diff) ? -1 : 1;

	//// Now make a step in the opposite direction of the derivative.
	////	Make a line search to find the optimal step to take
	//int nIterations = 0;
	//while(diff > maxDiff || nIterations == 0){// make at least 1 iteration
	//	guess			= guess + sign * diff;

	//	diff			= fabs(GetWindDirection(source, guess,		 gps[lowerScanner], compass[lowerScanner], plumeCentre[lowerScanner], coneAngle[lowerScanner], tilt[lowerScanner]) -
	//								   GetWindDirection(source, guess,		 gps[upperScanner], compass[upperScanner], plumeCentre[upperScanner], coneAngle[upperScanner], tilt[upperScanner]));
	//	
	//	// Increase and check the number of iterations
	//	if(nIterations++ > 100){
	//		return false;
	//	}
	//}

	//// Return our guess
	//plumeHeight = guess;

	//return true;
}

/** Calculates the direction of a ray from a cone-scanner with the given angles.
		Direction defined as direction from scanner, in a coordinate system with
			the x-axis in the direction of the scanner, the z-axis in the vertical direction
			and the y-axis defined as to get a right-handed coordinate system */
void CGeometryCalculator::GetDirection(double direction[3], double scanAngle, double coneAngle, double tilt){
	double tan_coneAngle = tan(coneAngle * DEGREETORAD);
	double cos_tilt      = cos(tilt * DEGREETORAD);
	double sin_tilt      = sin(tilt * DEGREETORAD);
	double cos_alpha     = cos(scanAngle * DEGREETORAD);
	double sin_alpha     = sin(scanAngle * DEGREETORAD);
	double divisor       = (cos_alpha*cos_tilt + sin_tilt/tan_coneAngle);

	direction[0] = (cos_tilt/tan_coneAngle - cos_alpha*sin_tilt)	/ divisor;
	direction[1] = sin_alpha										/ divisor;
	direction[2] = 1;
}

/** Calculate the plume-height using the two scans found in the 
		given evaluation-files. */
bool CGeometryCalculator::CalculateGeometry(const CString &evalLog1, int scanIndex1, const CString &evalLog2, int scanIndex2, double &plumeHeight, double &plumeHeightError, double &windDirection, double &windDirectionError, CGeometryCalculationInfo *info){
	FileHandler::CEvaluationLogFileHandler reader[2];
	CGPSData gps[2], source;
	double plumeCentre[2], plumeCompleteness, tmp, plumeEdge_low, plumeEdge_high;
	Common common;
	int k;

	// 1. Read the evaluation-logs
	reader[0].m_evaluationLog.Format("%s", (LPCSTR)evalLog1);
	reader[1].m_evaluationLog.Format("%s", (LPCSTR)evalLog2);
	if(SUCCESS != reader[0].ReadEvaluationLog())
		return false;
	if(SUCCESS != reader[1].ReadEvaluationLog())
		return false;

	// 2. Get the gps-data from the eval-logs, if they don't contain any
	//      GPS-information or if the instruments are too close then return.
	for(k = 0; k < 2; ++k){
		gps[k].m_latitude  = reader[k].m_specInfo.m_gps.m_latitude;
		gps[k].m_longitude = reader[k].m_specInfo.m_gps.m_longitude;
		gps[k].m_altitude  = reader[k].m_specInfo.m_gps.m_altitude;
	}
	if(fabs(gps[0].m_latitude) < 1e-2 && fabs(gps[0].m_longitude) < 1e-2)
		return false;
	if(fabs(gps[1].m_latitude) < 1e-2 && fabs(gps[1].m_longitude) < 1e-2)
		return false;
	double instrumentDistance = common.GPSDistance(gps[0].m_latitude, gps[0].m_longitude, gps[1].m_latitude, gps[1].m_longitude);
#ifndef _DEBUG
	if(instrumentDistance < 200)
		return false;
#endif

	// 3. Get the nearest volcanoes, if these are different then quit the calculations
	int volcanoIndex1 = CGeometryCalculator::GetNearestVolcano(gps[0].m_latitude, gps[0].m_longitude);
	int volcanoIndex2 = CGeometryCalculator::GetNearestVolcano(gps[1].m_latitude, gps[1].m_longitude);
	if(volcanoIndex1 == -1)
		return false; // if we couldn't find any volcano...
	if(volcanoIndex1 != volcanoIndex2)
		return false; // if the two systems does not monitor the same volcano...
	source.m_latitude  = g_volcanoes.m_peakLatitude[volcanoIndex1];
	source.m_longitude = g_volcanoes.m_peakLongitude[volcanoIndex1];
	source.m_altitude  = (long)g_volcanoes.m_peakHeight[volcanoIndex1];

	// 4. Get the scan-angles around which the plumes are centred
	int index[2] = {scanIndex1, scanIndex2};
	for(k = 0; k < 2; ++k){
		if(false == reader[k].m_scan[index[k]].CalculatePlumeCentre("SO2", plumeCentre[k], tmp, plumeCompleteness, plumeEdge_low, plumeEdge_high))
			return false; // <-- cannot see the plume
	}

	// 5. Get the compass-directions, the tilt of the two systems and the coneAngles
	double compass[2], coneAngle[2], tilt[2];
	for(k = 0; k < 2; ++k){
		compass[k]    = reader[k].m_specInfo.m_compass;
		coneAngle[k]  = reader[k].m_specInfo.m_coneAngle;
		tilt[k]       = reader[k].m_specInfo.m_pitch;
	}

	// 6. Calculate the plume-height
	if(false == CGeometryCalculator::GetPlumeHeight_Exact(gps, compass, plumeCentre, coneAngle, tilt, plumeHeight)){
		if(false == CGeometryCalculator::GetPlumeHeight_Fuzzy(source, gps, compass, plumeCentre, coneAngle, tilt, plumeHeight)){
			return false; // <-- could not calculate plume-height
		}
	}
	if(plumeHeight < 0){
		return false; // we failed to calculate anything reasonable
	}

	// 7. As a bonus, also calculate the wind-direction
	CGPSData volcanoGPS = CGPSData(g_volcanoes.m_peakLatitude[volcanoIndex1], g_volcanoes.m_peakLongitude[volcanoIndex1], g_volcanoes.m_peakHeight[volcanoIndex1]);
	windDirection = GetWindDirection(volcanoGPS, plumeHeight, gps[0], compass[0], plumeCentre[0], coneAngle[0], tilt[0]);

	// 8. Also calculate the errors in wind-direction and plume height

	// 8a. The error in wind-direction
	double plumeCentreError = 7.2; // an estimate for the error in finding the plumeCentre [deg]
	double wdPlus           = GetWindDirection(volcanoGPS, plumeHeight, gps[0], compass[0], plumeCentre[0] + plumeCentreError, coneAngle[0], tilt[0]);
	double wdMinus          = GetWindDirection(volcanoGPS, plumeHeight, gps[0], compass[0], plumeCentre[0] - plumeCentreError, coneAngle[0], tilt[0]);
	windDirectionError      = fabs(wdPlus - wdMinus) / 2;

	// 8b. The error in plume height
	double plumeCentre_perturbated[2], phPlus, phMinus;

	plumeCentre_perturbated[0] = plumeCentre[0] - plumeCentreError;	plumeCentre_perturbated[1] = plumeCentre[1] + plumeCentreError;
	if(false == CGeometryCalculator::GetPlumeHeight_Exact(gps, compass, plumeCentre_perturbated, coneAngle, tilt, phPlus)){
		if(false == CGeometryCalculator::GetPlumeHeight_Fuzzy(source, gps, compass, plumeCentre_perturbated, coneAngle, tilt, phPlus)){
			phPlus = 1e99; // <-- could not calculate plume-height
		}
	}	
	plumeCentre_perturbated[0]	= plumeCentre[0] + plumeCentreError;	plumeCentre_perturbated[1]	= plumeCentre[1] - plumeCentreError;
	if(false == CGeometryCalculator::GetPlumeHeight_Exact(gps, compass, plumeCentre_perturbated, coneAngle, tilt, phMinus)){
		if(false == CGeometryCalculator::GetPlumeHeight_Fuzzy(source, gps, compass, plumeCentre_perturbated, coneAngle, tilt, phMinus)){
			phMinus = -1e99; // <-- could not calculate plume-height
		}
	}
	plumeHeightError  = fabs(phPlus - phMinus) / 2;

	// 9. If the user wants to have some of the parameters used then give them
	if(info != NULL){
		info->scanner[0]     = gps[0];
		info->scanner[1]     = gps[1];
		info->plumeCentre[0] = plumeCentre[0];
		info->plumeCentre[1] = plumeCentre[1];
	}

	return true;
}

/** Calculates the cross product of the supplied vectors */
void CGeometryCalculator::Cross(const double u[3], const double v[3], double result[3]){
	result[0] = u[1] * v[2] - u[2] * v[1];
	result[1] = u[2] * v[0] - u[0] * v[2];
	result[2] = u[0] * v[1] - u[1] * v[0];
}

/** Calculates the squared norm of the supplied vector */
double CGeometryCalculator::Norm2(const double v[3]){
	return (v[0] * v[0] + v[1] * v[1] + v[2]*v[2]);
}

/** Calculates the determinant of a matrix whose columns are defined
		by the three supplied vectors */
double CGeometryCalculator::Det(const double c1[3], const double c2[3], const double c3[3]){
	double ret = c1[0]*c2[1]*c3[2] + c2[0]*c3[1]*c1[2] + c3[0]*c1[1]*c2[2];
	ret = ret - c1[0]*c3[1]*c2[2] - c2[0]*c1[1]*c3[2] - c3[0]*c2[1]*c1[2];

	return ret;
}

/** Normalizes the supplied vector */
void CGeometryCalculator::Normalize(double v[3]){
	double norm_inv = 1 / sqrt(Norm2(v));
	v[0] *= norm_inv;
	v[1] *= norm_inv;
	v[2] *= norm_inv;
}

/** Calculates the wind-direction for a scan, assuming that the plume originates
			at the postition given in 'source' and that the centre of the plume is 
			at the scan angle 'plumeCentre' (in degrees). The height of the plume above
			the scanning instrument is given by 'plumeHeight' (in meters).
			The properties of the scanner are given by the 'compass' - direction (degrees from north) 
			and the 'coneAngle' (degrees) 
			@return the wind-direction if the calculations are successful
			@return -999 if something is wrong.				*/
double CGeometryCalculator::GetWindDirection(const CGPSData source, double plumeHeight, const CGPSData scannerPos, double compass, double plumeCentre, double coneAngle, double tilt){
	if(plumeCentre < -900)
		return -999.0;

	// 1. Calculate the intersection-point
	double intersectionDistance, angle;
	if(fabs(coneAngle - 90.0) > 1){
		// ------------ CONE SCANNERS -----------
		// 1a. the distance from the system to the intersection-point
		double x, y;
		double cos_tilt				= cos(DEGREETORAD * tilt);
		double sin_tilt				= sin(DEGREETORAD * tilt);
		double tan_coneAngle	= tan(DEGREETORAD * coneAngle);
		double cos_alpha			= cos(DEGREETORAD * plumeCentre);
		double sin_alpha			= sin(DEGREETORAD * plumeCentre);

		// Calculate the projections of the intersection points in the ground-plane
		double commonDenominator = cos_alpha*cos_tilt + sin_tilt/tan_coneAngle;
		x		= (cos_tilt/tan_coneAngle - cos_alpha*sin_tilt)	/ commonDenominator;
		y		= (sin_alpha)	/ commonDenominator;

		intersectionDistance = plumeHeight * sqrt( pow(x, 2) + pow(y, 2) );

		// 1b. the direction from the system to the intersection-point
//		angle	= -atan2(y, x) / DEGREETORAD + compass;
			angle	= atan2(y, x) / DEGREETORAD + compass;
	}else{
		// ------------- FLAT SCANNERS ---------------
		// 1a. the distance from the system to the intersection-point
		intersectionDistance = plumeHeight * tan(DEGREETORAD * plumeCentre);

		// 1b. the direction from the system to the intersection-point
		if(plumeCentre == 0)
			angle = 0;
		else if(plumeCentre < 0)
			angle = (compass + 90);
		else
			angle = (compass - 90);
	}

	// 1c. the intersection-point
	double lat2, lon2;
	Common common;
	common.CalculateDestination(scannerPos.m_latitude, scannerPos.m_longitude, intersectionDistance, angle, lat2, lon2);

	// 2. the wind-direction
	double windDirection = common.GPSBearing(source.m_latitude, source.m_longitude, lat2, lon2);

	return windDirection;
}

/** Calculates the wind-direction for a scan, assuming that the plume originates
			at the postition given in 'source' and that the centre of the plume is 
			at the scan angle 'plumeCentre' (in degrees). The height of the plume above
			the scanning instrument is given by 'plumeHeight' (in meters).
			This function is intended for use with V-II Heidelberg instruments
			@return the wind-direction if the calculations are successful
			@return -999 if something is wrong. 					*/
double CGeometryCalculator::GetWindDirection(const CGPSData source, const CGPSData scannerPos, double plumeHeight, double alpha_center_of_mass, double phi_center_of_mass){
	Common common;

	//longitudinal distance between instrument and source:
	double x_source = common.GPSDistance(scannerPos.m_latitude, source.m_longitude, scannerPos.m_latitude, scannerPos.m_longitude);
		if (source.m_longitude < scannerPos.m_longitude) x_source=-fabs(x_source);
	  else x_source=fabs(x_source);

	//latitudinal distance between instrument and source:
	double y_source = common.GPSDistance(source.m_latitude, scannerPos.m_longitude, scannerPos.m_latitude, scannerPos.m_longitude);
		if (source.m_latitude < scannerPos.m_latitude) y_source=-fabs(y_source);
	  else y_source=fabs(y_source);

	//the two angles for the measured center of mass of the plume converted to rad:
	double alpha_cm_rad	=	DEGREETORAD*alpha_center_of_mass;
	double phi_cm_rad		=	DEGREETORAD*phi_center_of_mass;

	double wd = atan2((x_source-plumeHeight*tan(alpha_cm_rad)*sin(phi_cm_rad)),(y_source-plumeHeight*tan(alpha_cm_rad)*cos(phi_cm_rad)))/DEGREETORAD;
	if (wd<0) 
		wd+=360;		//because atan2 returns values between -pi...+pi

	return wd;
}

/** Retrieve the plume height from a measurement using one scanning-instrument
		with an given assumption of the wind-direction 	*/
double CGeometryCalculator::GetPlumeHeight_OneInstrument(const CGPSData source, const CGPSData gps, double WindDirection, double alpha_center_of_mass, double phi_center_of_mass){
	Common common;

	//horizontal distance between instrument and source:
	double distance_to_source	= common.GPSDistance(gps.m_latitude, gps.m_longitude, source.m_latitude, source.m_longitude);

	//angle (in rad) pointing from instrument to source (with respect to north, clockwise):
	double angle_to_source_rad=DEGREETORAD * common.GPSBearing(gps.m_latitude, gps.m_longitude, source.m_latitude, source.m_longitude);

	//the two angles for the measured center of mass of the plume converted to rad:
	double alpha_cm_rad	= DEGREETORAD*alpha_center_of_mass;
	double phi_cm_rad		= DEGREETORAD*phi_center_of_mass;

	double WindDirection_rad=DEGREETORAD*WindDirection;

	return 1/tan(alpha_cm_rad)*sin(angle_to_source_rad-WindDirection_rad)/sin(phi_cm_rad-WindDirection_rad)*distance_to_source;

}
