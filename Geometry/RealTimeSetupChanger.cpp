#include "StdAfx.h"
#include "realtimesetupchanger.h"

// we need the list of volcanoes...
#include "../VolcanoInfo.h"

// we also need the meterological data
#include "../MeteorologicalData.h"

extern CMeteorologicalData g_metData;			// <-- The meteorological data
extern CWinThread					*g_comm;				// <-- the communication controller
extern CVolcanoInfo				 g_volcanoes;		// <-- A list of all known volcanoes
extern CConfigurationSetting g_settings;	// <-- The settings

using namespace Geometry;

CRealTimeSetupChanger::CRealTimeSetupChanger(void)
{
}

CRealTimeSetupChanger::~CRealTimeSetupChanger(void)
{
}


/** Changes the cfg.txt for the supplied spectrometer */
void	CRealTimeSetupChanger::ChangeCfg(const Evaluation::CSpectrometer *spec, double alpha_min, double alpha_max, double phi_source, double beta, bool isflat){
	CString dateTime, message;
	Common common;

	// allocate the strings	
	CString *serialNumber = new CString();
	CString *fileName = new CString();

	// Get the serial-number
	serialNumber->Format(spec->m_settings.serialNumber);

	double percent		= 0.8;
	double maxExpTime	= 2000;

	// ADDED THESE TWO LINES 2008-07-14 /MJ
	int		averagePeriod = 3;
	double plumeCentre_phi	 = spec->m_history->GetPlumeCentre(averagePeriod, 1);

	int	sum1 = 15;

	int cone_steps			=50;								//number of steps for one cone scan (might be given by the user?)
	double alpha_step	=180.0 / cone_steps;	//gives the same step width for the flat scan of the partial sky
	int motorPosition1;
	int motorPosition2;
	int exp_time=-1;

	int motorStepsComp1 = spec->m_scanner.motor[0].motorStepsComp;
	int motorStepsComp2 = spec->m_scanner.motor[1].motorStepsComp;
	int stepsPerRound1  = abs (int(spec->m_scanner.motor[0].stepsPerRound));
	int stepsPerRound2  = abs (int(spec->m_scanner.motor[1].stepsPerRound));

	double stepsPerDegree1	= stepsPerRound1 / 360.0;		//for zenith motor; different settings for the two motors
	double stepsPerDegree2	= stepsPerRound2 / 360.0;		//for azimuth motor
	double alpha;				//variable for zenith angle which is varied for the flat scan of the partial sky

	// Retrieve the model of the spectrometer
    std::string spectrometerType;
	if(!CSpectrometerModel::ToString(spec->m_scanner.spec[0].model, spectrometerType))
    {
		spectrometerType = "HR2000";
	}

	// 2. Get the directory where to temporarily store the cfg.txt
	if(strlen(g_settings.outputDirectory) > 0){
		fileName->Format("%s\\Temp\\cfg.txt", (LPCSTR)g_settings.outputDirectory);
	}else{
		common.GetExePath();
		fileName->Format("%s\\cfg.txt", (LPCSTR)common.m_exePath);
	}
	FILE *f = fopen(*fileName, "w");
	if(f == NULL){
		return;
	}	

	// 4. Write the configuration-file

	// 4a. A small header 
	common.GetDateTimeText(dateTime);
	fprintf(f, "%%-------------Modified at %s------------\n\n", (LPCSTR)dateTime);

	// 4b. The instrument-type
    fprintf (f, "SPECTROMETERTYPE=%s\n\n", spectrometerType.c_str());
  
	// 4c. Write the Spectrum transfer information
	fprintf(f, "%%  STARTCHN and STOPCHN define which channels in the spectra will be transferred\n");
	fprintf(f, "STARTCHN=0\n");
	fprintf(f, "STOPCHN=2047\n\n");

	// 4d. Don't use real-time collection
	fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
	fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
	fprintf(f, "REALTIME=0\n\n");

	// 4e. Write the motor information
	fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
	fprintf(f, "STEPSPERROUND=%d %d\n",	-stepsPerRound1,		-stepsPerRound2);		//for azimuth and zenith motor
	fprintf(f, "MOTORSTEPSCOMP=%d %d\n",	motorStepsComp1,	motorStepsComp2);		//reference position for azimuth and zenith motor
	fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
	fprintf(f, "SKIPMOTOR=2\n");
	fprintf(f, "DELAY=%d\n\n",				2);

	// 4f. Write the geometry (compass, tilt...)
//	fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
//	fprintf(f, "COMPASS=%.1lf %.1lf %.1lf \n\n", 0.0, 0.0, 0.0);

	// 4g. Write other things
	fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
	fprintf(f,  "PERCENT=%.2lf\n\n",			percent);
	fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
	fprintf(f,	"MAXINTTIME=%.0lf\n\n",		maxExpTime);
	fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
	fprintf(f,  "DEBUG=%d\n\n",         1);

	// 4h. Write the measurement information
	fprintf(f, "%% sum1 is inside the spectrometer [1 to 15]\n%%--zenith----azimuth----time-sum1-sum2--chn--basename-----repetitions\n");

double original_alpha_min	=	alpha_min;
double original_alpha_max	=	alpha_max;


	beta	= fmod(beta, 360);				//make angle beta be within -360..360 degree range

/* if the new azimuth (=phi) direction is not in the same half circle as the old one
   we have to swap the alpha range (that was calculated based on the old azimuth direction): */
	double diff_phi = fmod (fabs(plumeCentre_phi) - (phi_source + beta), 360); // CHANGED HERE 2008-07-14 /MJ
	if (diff_phi > 180) diff_phi = 360 - diff_phi;	
	if (diff_phi > 90) {alpha_min = -original_alpha_max;
                      alpha_max = -original_alpha_min;} 

/* The following redefinition of beta and alpha deliveres the same looking directions but
   limits the range of the finally used azimuth angle (phi_source+beta) to 0...360 degree)*/		
  if (phi_source <= 180){         //confine beta to 0...+180 degree range
     if (beta < -180) beta+=360;
     else if (beta < 0)   {beta += 180;
                           alpha_min = -original_alpha_max;
                           alpha_max = -original_alpha_min;}
     else if (beta >= 180){beta -= 180;
                           alpha_min = -original_alpha_max;
                           alpha_max = -original_alpha_min;}
  }
   else {               //then (phi_source > 180) --> confine beta within -180...0 degree
     if (beta > 180) beta -= 360;
     else if (beta > 0)   {beta -= 180;
                           alpha_min = -original_alpha_max;
                           alpha_max = -original_alpha_min;}
     else if (beta <= -180){beta += 180;
                           alpha_min = -original_alpha_max;
                           alpha_max = -original_alpha_min;}
   }

  //spec_counter is used for numbering the names of the spectra in the scan:
  int spec_counter = 0;

	// in the following the different scanning modes are actually written into the cfg.txt file (using the parameters alpha_min, alpha_max, phi_source, beta):
	
  if(fabs(alpha_max - alpha_min) <= 150) 	{	//this means we're doing a flat scan in a single plane between alpha_min and alpha_max    			
		
		//The sky-measurement (written within the if condition so that the azimuth motor doesn't have to be moved afterwards)
		motorPosition2=int (round((phi_source+beta)*stepsPerDegree2));					

	  fprintf(f, "MEAS=0 %d %d 15 1 0 sky 1 0\n", motorPosition2, exp_time);

  //		exp_time=0;
  // The dark-measurement
		fprintf(f, "MEAS=%d %d 25000 1 1 0 dark_cur 1 0\n", int (round(180*stepsPerDegree1)), motorPosition2);
		fprintf(f, "MEAS=%d %d 3 15 75 0 offset 1 0\n", int (round(180*stepsPerDegree1)), motorPosition2);
		
     if (alpha_max < alpha_min) {alpha = alpha_min;    //shouldn't be necessary...
                                 alpha_min = alpha_max;
                                 alpha_max = alpha;}
     alpha = alpha_max; // added this line
		 while (alpha >= alpha_min){
			 motorPosition1 = int (round(alpha * stepsPerDegree1));
       spec_counter++;
			 fprintf(f, "MEAS=%d %d %d %d 1 0 half%d 1 0\n", motorPosition1, motorPosition2, exp_time, sum1, spec_counter);

			 alpha	-=	alpha_step;
		 }
	}else{
		//this means we're doing two flat partial scans
  
    //The sky-measurement (written within the if condition so that the azimuth motor doesn't have to be moved afterwards)
		motorPosition2=int (round((phi_source+beta)*stepsPerDegree2));
	  fprintf(f, "MEAS=0 %d %d 15 1 0 sky 1 0\n", motorPosition2, exp_time);
  //		exp_time=0;
  // The dark-measurement
		fprintf(f, "MEAS=%d %d 25000 1 1 0 dark_cur 1 0\n", int (round(180*stepsPerDegree1)), motorPosition2);
		fprintf(f, "MEAS=%d %d 3 15 75 0 offset 1 0\n", int (round(180*stepsPerDegree1)), motorPosition2);	
  
  
    //the loop for two half flat scans
    double beta2;
    if (beta < 0) beta2 = -180-beta;
    else beta2 = 180-beta;
   if (alpha_max > alpha_min){  
       alpha=alpha_max;
		   while (alpha >= alpha_min){
		   	motorPosition1=int (round(alpha*stepsPerDegree1));  //zenith angle
		   	spec_counter++;
		   	if (alpha>=0)
		      motorPosition2=int (round((phi_source+beta)*stepsPerDegree2));  	//azimuth angle
		   	else
		      motorPosition2=int (round((phi_source+beta2)*stepsPerDegree2));   //through this turn of the azimuth angle the zenith angle can run from +90 to -90
		   	fprintf(f, "MEAS=%d %d %d %d 1 0 full%d 1 0\n", motorPosition1, motorPosition2, exp_time, sum1, spec_counter);
			alpha -= alpha_step;}
	  }
   else {       //shouldn't be necessary...
       alpha=alpha_max;
		   while (alpha <= alpha_min){
		   	motorPosition1=int (round(alpha*stepsPerDegree1));  //zenith angle
		   	spec_counter++;
		   	if (alpha <= 0) motorPosition2=int (round((phi_source+beta)*stepsPerDegree2));  	//azimuth angle
			  else motorPosition2=int (round((phi_source+beta2)*stepsPerDegree2));   //through this turn of the azimuth angle the zenith angle can run from +90 to -90
		   	fprintf(f, "MEAS=%d %d %d %d 1 0 full%d 1 0\n", motorPosition1, motorPosition2, exp_time, sum1, spec_counter);
			  alpha += alpha_step;}
		}
	}

	// Close the file
	fclose(f);

	// 4. Tell the communication controller that we want to upload a file
	if(g_comm != NULL){
		g_comm->PostThreadMessage(WM_STATUS_UPLOAD, (WPARAM)serialNumber, (LPARAM)fileName);

		// 4b. Tell the user what we've done...
		message.Format("Uploading new cfg.txt to spectrometer %s", (LPCSTR)serialNumber);
		ShowMessage(message);
	}
}

/** Retrieves the optimal range for 
			1) the scan-angles (alpha_min -> alpha_max) 
			2) the azimuth-angles (phi)
			3) the opening angle of the two sides of the scan (beta)
		to do half-flat scans
		for a scanner where the wind-direction is given by 'windDirection' and the 
			centre of mass of the last scan is found at the scan angle 'alpha_cm' */
bool CRealTimeSetupChanger::GetParameters(double windDirection, const CGPSData &scannerPos, const CString &source, double minAngle, double alpha_cm, double &alpha_min, double &alpha_max, double &phi_source, double &beta){
	CGPSData sourcePos;
	Common common;

	// Find the volcano we're looking at. If not found then return false
	int volcanoIndex = -1;
	for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
		if(Equals(source, g_volcanoes.m_name[k])){
			volcanoIndex = k;
			break;
		}
	}
	if(volcanoIndex < 0)
		return false;
	sourcePos.m_latitude	= g_volcanoes.m_peakLatitude[volcanoIndex];
	sourcePos.m_longitude = g_volcanoes.m_peakLongitude[volcanoIndex];
	sourcePos.m_altitude	= (long)g_volcanoes.m_peakHeight[volcanoIndex];

	// The scan-angles should cover the centre of mass...
	alpha_min = alpha_cm - 45;
	alpha_max = alpha_cm + 45;

	// However they should not go out of bounds...
	if(alpha_min < -90){
		alpha_min = -90;
		alpha_max = 0;
	}else if(alpha_max > +90){
		alpha_min = 0;
		alpha_max = 90;
	}

	/* The azimuth angle should be perpendicular to the wind-direction
		 however we must make sure that we don't point directly towards the source...
     It is also checked that we don't point away from the source but always either
     perpendicular to the source direction or slightly towards it (up to minAngle)*/

	// The direction from the instrument to the source...
	phi_source = common.GPSBearing(scannerPos.m_latitude, scannerPos.m_longitude, sourcePos.m_latitude, sourcePos.m_longitude);

  double phi_scan = 0;
  if (( (windDirection <= phi_source) && (phi_source <= 90-minAngle+windDirection) )
      || ( (-360+windDirection <= phi_source) && (phi_source <= -270-minAngle+windDirection) ))
      phi_scan = windDirection + 90;
  else if (( (phi_source <= windDirection) && (windDirection <= 90-minAngle+phi_source) )
      || ( (-360+phi_source <= windDirection) && (windDirection <= -270-minAngle+phi_source) ))
      phi_scan = windDirection - 90;
  else if (  ((windDirection-phi_source < 180) && (windDirection-phi_source > 0) )
            ||(windDirection-phi_source < -180))
       phi_scan = phi_source-minAngle;
  else phi_scan = phi_source+minAngle;
  
  beta = -phi_source + phi_scan;

	return true;
}
