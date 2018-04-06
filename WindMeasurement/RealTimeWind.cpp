#include "StdAfx.h"
#include "realtimewind.h"

extern CConfigurationSetting g_settings;	// <-- The settings
extern CWinThread *g_comm;								// <-- the communication controller

int wind_measurement_calculation (double PlumeHeight, double wd, double &alpha_center_of_mass, double &phi_center_of_mass, double angle_between_meas, double &alpha_2, double &phi_2, int AzimuthMotorstepsComp, int AzimuthStepsPerRound, const CConfigurationSetting::WindSpeedMeasurementSetting &windSettings);

using namespace WindSpeedMeasurement;
CRealTimeWind::CRealTimeWind(void)
{
}

CRealTimeWind::~CRealTimeWind(void)
{
}

/** Runs through the history of the CSpectrometer and checks the settings
		for automatic wind-speed measurements and judges if we should perform
		an automatic wind-speed measurement now. 
		@return true if a wind-speed measurement should be started else return false */
bool	CRealTimeWind::IsTimeForWindMeasurement(const Evaluation::CSpectrometer *spectrometer){
	unsigned int k;
	CString debugFile;
	CString dateStr;
	CString timeStr;
	Common common;
	common.GetDateText(dateStr);
	common.GetTimeText(timeStr);
	debugFile.Format("%sOutput\\%s\\Debug_WindSpeedMeas.txt", (LPCTSTR)g_settings.outputDirectory, (LPCTSTR)dateStr);

	//// -1. This checking should only be performed on master-channel spectrometers...
	//if(spectrometer->m_channel != 0)
	//	return false;

	// 0. Local handles, to get less dereferencing...
	const CConfigurationSetting::WindSpeedMeasurementSetting *windSettings = &spectrometer->m_scanner.windSettings;

	// 1. Check the settings for the spectrometer, if there's only one channel or
	//		if the settings says that we should not perform any wind-measurements, then don't
	if(false == windSettings->automaticWindMeasurements)
		return false;
	bool isMultiChannelSpec = false;
	for(k = 0; k < spectrometer->m_scanner.specNum; ++k){
		if(spectrometer->m_scanner.spec[k].channelNum > 1)
			isMultiChannelSpec = true;
	}
	if(!isMultiChannelSpec && spectrometer->m_scanner.instrumentType != INSTR_HEIDELBERG)
		return false;
	if(spectrometer->m_history == NULL)
		return false;

	// 2. Check the history of the spectrometer

	// 2a. Have we recieved any scans today?
	int nScans = spectrometer->m_history->GetNumScans();
	if(nScans < windSettings->stablePeriod){ // <-- there must be enough scans received from the instrument today for us to make any wind-measurement
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){
			fprintf(f, "%s\t No measurement: Too few scans today\n", (LPCTSTR)timeStr);
			fclose(f); 
		}
		return false; // <-- too few scans recieved today
	}

	// 2b. How long time has passed since the last ws-measurement?
	int sPassed = spectrometer->m_history->SecondsSinceLastWindMeas();
	if(sPassed > 0 && sPassed < windSettings->interval){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: Little time since last measurement\n", (LPCTSTR)timeStr);
			fclose(f);
		}
		return false;
	}

	// 2c. What's the average time between the start of two scans today?
	double	timePerScan	= spectrometer->m_history->GetScanInterval();
	if(timePerScan < 0)
		return false; // <-- no scans have arrived today

	// 2d. Get the plume centre variation over the last few scans
	//			If any scan missed the plume, return false
	double centreMin, centreMax;
	if(false == spectrometer->m_history->GetPlumeCentreVariation(windSettings->stablePeriod, 0, centreMin, centreMax)){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: At least one of the last %d scans has missed the plume\n", (LPCTSTR)timeStr, windSettings->stablePeriod);
			fclose(f);
		}
		return false;
	}

	// 2e. If there's a substantial variation in plume centre over the last
	//			few scans then don't make any measurement
	if(fabs(centreMax - centreMin) > 30){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: Too large variation in plume centre\n", (LPCTSTR)timeStr);
			fclose(f);
		}
		return false;
	}

	// 2g. If the plumeCentre is at lower angles than the options for the
	//			windmeasurements allow, then don't measure
	double plumeCentre = spectrometer->m_history->GetPlumeCentre(windSettings->stablePeriod);
	if(fabs(plumeCentre) > fabs(windSettings->maxAngle)){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: Plume centre too low\n", (LPCTSTR)timeStr);
			fclose(f);
		}
		return false;
	}

	// 2h. Check so that the maximum column averaged over the last few scans is at least
	//			50 ppmm
	double maxColumn = spectrometer->m_history->GetColumnMax(windSettings->stablePeriod);
	if(maxColumn < windSettings->minPeakColumn){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: Plume too weak\n", (LPCTSTR)timeStr);
			fclose(f);
		}
		return false;
	}

	// 2i. Check so that the average exposure-time over the last few scans is not
	//			too large. This to ensure that we can measure fast enough
	double expTime = spectrometer->m_history->GetExposureTime(windSettings->stablePeriod);
	if(expTime < 0 || (expTime > 600 && spectrometer->m_scanner.instrumentType == INSTR_GOTHENBURG)){
		FILE *f = fopen(debugFile, "a+");
		if(f != NULL){ 
			fprintf(f, "%s\t No measurement: Too long exposure times\n", (LPCTSTR)timeStr);
			fclose(f); 
		}
		return false;
	}

	FILE *f = fopen(debugFile, "a+");
	if(f != NULL){ 
		fprintf(f, "%s\t Ok to do wind-speed measurement!!\n", (LPCTSTR)timeStr);
		fclose(f);
	}

	// We've passed all tests, it's now ok to make a wind-speed measurement
	return true;
}

/** Starts an automatic wind-speed measurement for the supplied spectrometer */
void CRealTimeWind::StartWindSpeedMeasurement(const Evaluation::CSpectrometer *spec, CWindField &windField){
	static CString message, spectrometerType;
	CString dateTime, directory;
	Common common;
	int stepsPerRound	= 200;
	double maxExpTime	= 300.0; // the maximum allowed exposure-time
	int readOutTime		= 100; // time to read out one spectrum from the detector
	int stablePeriod	= spec->m_scanner.windSettings.stablePeriod;
	double plumeCentre	= spec->m_history->GetPlumeCentre(stablePeriod);
	int motorPosition	= (int)((plumeCentre / 2) * (stepsPerRound / 360.0));
	double avgExpTime	= min(fabs(spec->m_history->GetExposureTime(stablePeriod)), maxExpTime);
	int sum1			= (int)min(max(1000.0 / (avgExpTime + readOutTime), 1), 15); // <-- calculate the number of co-adds assuming 100ms to read out each spectrum

	int	repetitions		= (int)(spec->m_scanner.windSettings.duration / (sum1 * (avgExpTime + readOutTime) / 1000 ));
	repetitions			= max(min(repetitions, 800), 400);

	// 0. Simple error checking...
	if(fabs(motorPosition) > 50 || fabs(plumeCentre) > 90)
		return; // illegal angle...
	if(fabs(plumeCentre) > fabs(spec->m_scanner.windSettings.maxAngle))
		return; // attempt to make a measurement outside of allowed range...

	// allocate the strings	
	CString *serialNumber = new CString();
	CString *fileName = new CString();

	// 1. Get the serial-number of the selected spectrometer
	serialNumber->Format(spec->m_scanner.spec[0].serialNumber);

	// 2. Get the directory where to temporarily store the cfgonce.txt
	if(strlen(g_settings.outputDirectory) > 0){
		directory.Format("%sTemp\\%s\\", (LPCTSTR)g_settings.outputDirectory, (LPCTSTR)serialNumber);
		if(CreateDirectoryStructure(directory)){
			common.GetExePath();
			fileName->Format("%s\\cfgonce.txt", (LPCTSTR)common.m_exePath);
		}else{
			fileName->Format("%scfgonce.txt", (LPCTSTR)directory);
		}
	}else{
		common.GetExePath();
		fileName->Format("%s\\cfgonce.txt", (LPCTSTR)common.m_exePath);
	}

	FILE *f = fopen(*fileName, "w");
	if(f == NULL){
		ShowMessage("Could not open cfgonce.txt for writing. Wind speed measurement failed!!");
		return;
	}

	// 3. Write the configuration-file

	// 3a. A small header 
	common.GetDateTimeText(dateTime);
	fprintf(f, "%%-------------Modified at %s------------\n\n", (LPCTSTR)dateTime);
	fprintf(f, "%% Questions? email\n%% mattias.johansson@chalmers.se\n\n");

	if(spec->m_scanner.instrumentType == INSTR_GOTHENBURG){

		// 3c. Write the Spectrum transfer information
		fprintf(f, "%% The following channels defines which channels in the spectra that will be transferred\n");
		fprintf(f, "STARTCHN=0\n");
		fprintf(f, "STOPCHN=684\n\n");

		// 3d. Don't use real-time collection
		fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
		fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
		fprintf(f, "REALTIME=0\n\n");

		// 3e. Write the motor information
		fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
		fprintf(f, "STEPSPERROUND=%d\n",	stepsPerRound);
		fprintf(f, "MOTORSTEPCOMP=%d\n",	spec->m_scanner.motor[0].motorStepsComp);
		fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
		fprintf(f, "SKIPMOTOR=0\n");
		if(spec->m_scanner.coneAngle == 90.0)
			fprintf(f, "DELAY=%d\n\n",				 200);
		else
			fprintf(f, "DELAY=%d\n\n",				 400);

		// 3f. Write the geometry (compass, tilt...)
		fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
		fprintf(f, "COMPASS=%.1lf 0.0 0.0\n\n", spec->m_scanner.compass);

		// 3g. Write other things
		fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
		fprintf(f,  "PERCENT=%.2lf\n\n",			0.8);
		fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
		fprintf(f,	"MAXINTTIME=%.0lf\n\n",		maxExpTime);
		fprintf(f, "%% The pixel where we want to measure the intensity of the spectra \n");
		fprintf(f,	"CHANNEL=670\n\n");
		fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
		fprintf(f,  "DEBUG=1\n\n");

		// 3h. Write the measurement information
		fprintf(f, "%% sum1 is inside the specrometer [1 to 15]\n%%-----pos----time-sum1-sum2--chn--basename----- repetitions\n");

		// 3i. The sky-measurement
		fprintf(f, "MEAS=%d -1 15 1 257 sky 1 0\n", motorPosition);

		// 3j. The dark-measurement
		fprintf(f, "MEAS=100 0 15 1 257 dark 1 0\n");
		
		// 3k. The actual measurement
		fprintf(f, "MEAS=%d 0 %d 1 257 wind %d 0\n", motorPosition, sum1, repetitions);

		// 3l. Another dark-measurement
		fprintf(f, "MEAS=100 0 15 1 257 dark 1 0\n");

	}else{
		// Heidelberg instrument!

		double alpha_center_of_mass = spec->m_history->GetPlumeCentre(stablePeriod, 0);
		double phi_center_of_mass		= spec->m_history->GetPlumeCentre(stablePeriod, 1);

		//alpha_2 and phi_2 define the second direction for the wind speed measurement
		double alpha_2=0;		
		double phi_2=0;
		//they are calculated by the following function:
		wind_measurement_calculation (windField.GetPlumeHeight(), windField.GetWindDirection(), alpha_center_of_mass, phi_center_of_mass, spec->m_scanner.windSettings.desiredAngle, alpha_2, phi_2, int (round(spec->m_scanner.motor[1].motorStepsComp)), abs (int(spec->m_scanner.motor[1].stepsPerRound)), spec->m_scanner.windSettings);
		
		int stepsPerRound1			=	abs (int(spec->m_scanner.motor[0].stepsPerRound));
		int stepsPerRound2			=	abs (int(spec->m_scanner.motor[1].stepsPerRound));
		double stepsPerDegree1		=	stepsPerRound1/360.0;
		double stepsPerDegree2		=	stepsPerRound2/360.0;

		int first_motorPosition1	=	int (round(stepsPerDegree1*round(alpha_center_of_mass)));
		int first_motorPosition2	=	int (round(stepsPerDegree2*round(phi_center_of_mass)));
		int second_motorPosition1	=	int (round(stepsPerDegree1*round(alpha_2)));
		int second_motorPosition2	=	int (round(stepsPerDegree2*round(phi_2)));

		double avgExpTime	= fabs(spec->m_history->GetExposureTime(stablePeriod));
		int sum1			= (int)min(max(1000.0 / (avgExpTime + readOutTime), 1), 15); // <-- calculate the number of co-adds assuming 100ms to read out each spectrum

		int	repetitions		= (int)(spec->m_scanner.windSettings.duration / (sum1 * (avgExpTime + readOutTime) / 1000 ));
		repetitions			= max(min(repetitions, 800), 400);

		// 4b. The instrument-type
		if(SUCCESS != CSpectrometerModel::ToString(spec->m_scanner.spec[0].model, spectrometerType)){
			spectrometerType.Format("HR2000");
		}
		fprintf (f, "SPECTROMETERTYPE=%s\n\n", (LPCTSTR)spectrometerType);  //can spectrometertype be retrieved automatically from configuration?

		// 3c. Write the Spectrum transfer information
		fprintf(f, "%% STARTCHN and STOPCHN define which channels in the spectra will be transferred\n");
		fprintf(f, "STARTCHN=0\n");
		fprintf(f, "STOPCHN=2047\n\n");

		// 3d. Don't use real-time collection
		fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
		fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
		fprintf(f, "REALTIME=0\n\n");

		// 3e. Write the motor information
		fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
		fprintf(f, "STEPSPERROUND=%d %d\n",	-stepsPerRound1, -stepsPerRound2); //2 motors HD
		fprintf(f, "MOTORSTEPSCOMP=%d %d\n",	int (round(spec->m_scanner.motor[0].motorStepsComp)), int (round(spec->m_scanner.motor[1].motorStepsComp))); //2 motors HD
		fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
		fprintf(f, "SKIPMOTOR=2\n");
		fprintf(f, "DELAY=%d\n\n",	 2); // what is an appropriate value

		// 3f. Write the geometry (compass, tilt...)
	//	fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
	//  fprintf(f, "COMPASS=%.1lf %.1lf %.1lf %.1lf \n\n", 0.0, 0.0, 0.0);

		// 3g. Write other things
		fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
		fprintf(f,  "PERCENT=%.2lf\n\n",			0.8);
		fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
		fprintf(f,	"MAXINTTIME=%.0lf\n\n",		1000.0);
		fprintf(f, "%% The pixel where we want to measure the intensity of the spectra \n");
		fprintf(f,	"CHANNEL=-1\n\n");
		fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
		fprintf(f,  "DEBUG=1\n\n");

		// 3h. Write the measurement information
		fprintf(f, "%% sum1 is inside the specrometer [1 to 15]\n%%--zenith----azimuth----time-sum1-sum2--chn--basename-----repetitions\n");

		// 3i. The sky-measurement
		fprintf(f, "MEAS=0 %d -1 %d 1 0 sky1 1 0\n", first_motorPosition2, sum1);

		// 3j. The dark-measurement
		fprintf(f, "MEAS=%d %d 0 %d 1 0 dark 1 0\n", int (round(180*stepsPerDegree1)), first_motorPosition2, sum1);
		
		// 3k. The actual measurement
		/* These two lines are to be executed alternately. 'repetitions' gives the
		total amount of collected spectra. Therefore each line is executed 
		one half 'repetitions' times. (this is done using flag 8) */
		{
			fprintf(f, "MEAS=%d %d 0 %d 1 0 wind 1 0\n", first_motorPosition1, first_motorPosition2, sum1);
			fprintf(f, "MEAS=%d %d 0 %d 1 0 wind %d 8\n", second_motorPosition1, second_motorPosition2, sum1, repetitions);	
		}

		// 3l. Another dark-measurement
		fprintf(f, "MEAS=%d %d 0 %d 1 0 dark 1 0\n", int (round(180*stepsPerDegree1)), second_motorPosition2, sum1);

	}

	// Close the file
	fclose(f);

	// 4. Tell the communication controller that we want to upload a file
	if(g_comm != NULL){
		g_comm->PostThreadMessage(WM_STATUS_UPLOAD, (WPARAM)serialNumber, (LPARAM)fileName);

		// 4b. Tell the user what we've done...
		message.Format("Telling spectrometer %s to perform wind-measurement", (LPCTSTR)serialNumber);
		ShowMessage(message);
	}
}



//This function calculates the second scanning direction for the wind measurement (alpha_2, phi_2):
int wind_measurement_calculation (double PlumeHeight, double wd, double &alpha_center_of_mass, double &phi_center_of_mass, double angle_between_meas, double &alpha_2, double &phi_2, int AzimuthMotorstepsComp, int AzimuthStepsPerRound, const CConfigurationSetting::WindSpeedMeasurementSetting &windSettings)
{
	double distance = 0;
	double SwitchPosition = AzimuthMotorstepsComp / (AzimuthStepsPerRound/360.0);
	double SwitchLow = SwitchPosition - windSettings.SwitchRange;

	//If distance along plume is given use that; else calculate it from the angle between the two wind measurements:
	if (angle_between_meas < 0) distance=-angle_between_meas;	//distance is given directly (negative values are used for distance; positive for angle)
	if (angle_between_meas > 0) distance=angle_between_meas*DEGREETORAD*PlumeHeight/ cos(alpha_center_of_mass);  //good approximation for smaller angles

	//position of the measured center of mass:
	double x1 = PlumeHeight*tan(alpha_center_of_mass*DEGREETORAD)*sin(phi_center_of_mass*DEGREETORAD);
	double y1 = PlumeHeight*tan(alpha_center_of_mass*DEGREETORAD)*cos(phi_center_of_mass*DEGREETORAD);

	double wd_rad=wd*DEGREETORAD;
	double x_wind=0;
	double y_wind=0;

  //calculate the wind vector with the length 'distance'
  //different angle ranges are treated seperately to get the right signs
	if (wd >= 0 && wd < 90){ 	
	x_wind = distance*tan(wd_rad)/sqrt(tan(wd_rad)*tan(wd_rad)+1);
	y_wind = distance/sqrt(tan(wd_rad)*tan(wd_rad)+1);}
	else if (wd == 90) {x_wind = distance;
						y_wind = 0;}
  else if (wd > 90 && wd <= 180){
	x_wind = distance*fabs(tan(wd_rad))/sqrt(tan(wd_rad)*tan(wd_rad)+1);
	y_wind = -distance/sqrt(tan(wd_rad)*tan(wd_rad)+1);}
  else if (wd > 180 && wd < 270){
	x_wind = -distance*fabs(tan(wd_rad))/sqrt(tan(wd_rad)*tan(wd_rad)+1);
	y_wind = -distance/sqrt(tan(wd_rad)*tan(wd_rad)+1);}	
	else if (wd == 270){x_wind = -distance;
						y_wind = 0;}
  else if (wd > 270 && wd <= 360){
	x_wind = -distance*fabs(tan(wd_rad))/sqrt(tan(wd_rad)*tan(wd_rad)+1);
	y_wind = distance/sqrt(tan(wd_rad)*tan(wd_rad)+1);}

  /*second measurement position is the original position plus the wind vector.
  Thus the second position is always upwind the plume.*/
	double x2 = x1+x_wind;			
	double y2 = y1+y_wind;

  //from the position calculate the measurement angles:
	alpha_2 = atan(sqrt(x2*x2+y2*y2)/PlumeHeight)/DEGREETORAD;		
	phi_2   = atan2(x2,y2)/DEGREETORAD;
  if (phi_2 < 0) phi_2 += 360;         //because atan2 returns the range -180..+180; we want 0..360
  
  phi_center_of_mass = fmod (phi_center_of_mass, 360);  //make sure phi_center_of_mass is between 0° and 360°
  if (phi_center_of_mass < 0) phi_center_of_mass += 360;

  double phi[2] = {phi_center_of_mass, phi_2};
  double alpha[2] = {alpha_center_of_mass, alpha_2}; 
  
  for (int i=0; i<2; i++){
  if (phi[i] < SwitchPosition) phi[i] += 360;
  if (phi[i] >= SwitchPosition+360) phi[i] -= 360;}
  
  for (int i=0; i<2; i++){
  int j = (i==0)?1:0;
  
    if (phi[j] < SwitchLow+180){
         if ( max (  max(alpha[i], alpha[j]) - min(alpha[i], alpha[j]),
                     max(phi[i], phi[j])     - min(phi[i], phi[j])      )
            > max (  max(alpha[i], -alpha[j])- min(alpha[i], -alpha[j]),
                     max(phi[i], phi[j]+180) - min(phi[i], phi[j]+180)  ) )
             {alpha[j] = -alpha[j];
              phi[j] = phi[j]+180;}}
    else if (phi[j] >= SwitchPosition+180 && phi[j] < SwitchLow+360){
         if ( max (  max(alpha[i], alpha[j]) - min(alpha[i], alpha[j]),
                     max(phi[i], phi[j])     - min(phi[i], phi[j])      )
            > max (  max(alpha[i], -alpha[j])- min(alpha[i], -alpha[j]),
                     max(phi[i], phi[j]-180) - min(phi[i], phi[j]-180)  ) )
             {alpha[j] = -alpha[j];
              phi[j] = phi[j]-180;}}
    else if (phi[j] >= SwitchLow+360)
             {alpha[j] = -alpha[j];
              phi[j] = phi[j]-180;}
  }
  phi_center_of_mass = phi[0];
  phi_2 = phi[1];
  alpha_center_of_mass = alpha[0];
  alpha_2 = alpha[1];

	return 0;
}
