#include "StdAfx.h"
#include "stdfile.h"

using namespace SpectrumIO;

CSTDFile::CSTDFile(void)
{
}

CSTDFile::~CSTDFile(void)
{
}

/** Reads a spectrum from a STD-file */
RETURN_CODE CSTDFile::ReadSpectrum(CSpectrum &spec, const CString &fileName){
	FILE *f = fopen(fileName, "r");
	if(f == NULL)
		return FAIL;
	const int bufSize = 1024;
	char buffer[bufSize];
	int tmpInt, tmpInt2, tmpInt3;
	double tmpDbl;

	// 0. Clear the spectrum's information
	spec.Clear();

	// 1. the "GDBGMNUP" string that identifies a STD-file
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(0 != _strnicmp("GDBGMNUP\n", buffer, strlen(buffer))){
		fclose(f); return FAIL;
	}

	// 2. The version number (always 1)
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}

	// 3. The spectrum length
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(1 > sscanf(buffer, "%d", &tmpInt)){
		fclose(f); return FAIL;
	}
	spec.m_length = min(tmpInt, MAX_SPECTRUM_LENGTH);

	// 4. The spectrum data
	for(int i = 0; i < spec.m_length; ++i){
		if(NULL == fgets(buffer, bufSize, f)){
			fclose(f); return FAIL;
		}
		if(1 > sscanf(buffer, "%lf", &tmpDbl)){
			fclose(f); return FAIL;
		}
		spec.m_data[i] = tmpDbl;
	}

	// 5. The fileName (ignore)
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}

	// 6. The detector (ignore)
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}

	// 7. The spectrometer
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	buffer[strlen(buffer) - 1] = 0; // <-- Remove the trailing newline character
	spec.m_info.m_device = std::string(buffer);

	// 8. The date
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(strstr(buffer, "/")){
		// DOASIS sometimes writes the date as MM/DD/YYYY
		if(3 > sscanf(buffer, "%d/%d/%d", &tmpInt2, &tmpInt3, &tmpInt)){
			fclose(f); return FAIL;
		}
	}else{
		if(3 > sscanf(buffer, "%d.%d.%d", &tmpInt3, &tmpInt2, &tmpInt)){
			fclose(f); return FAIL;
		}
	}
	if(tmpInt > 31){
		spec.m_info.m_startTime.year  = (tmpInt < 1900) ? tmpInt + 2000 : tmpInt;
		spec.m_info.m_startTime.month = tmpInt2;
		spec.m_info.m_startTime.day   = tmpInt3;
	}else{
		spec.m_info.m_startTime.year  = (tmpInt3 < 1900) ? tmpInt3 + 2000 : tmpInt3;
		spec.m_info.m_startTime.month = tmpInt2;
		spec.m_info.m_startTime.day   = tmpInt;
	}

	// 9. The starttime
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(3 > sscanf(buffer, "%d:%d:%d", &tmpInt, &tmpInt2, &tmpInt3)){
	}else{
		spec.m_info.m_startTime.hour   = tmpInt;
		spec.m_info.m_startTime.minute = tmpInt2;
		spec.m_info.m_startTime.second = tmpInt3;
	}

	// 10. The stoptime
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(3 > sscanf(buffer, "%d:%d:%d", &tmpInt, &tmpInt2, &tmpInt3)){
		fclose(f); return FAIL;
	}
	spec.m_info.m_stopTime.hour   = tmpInt;
	spec.m_info.m_stopTime.minute = tmpInt2;
	spec.m_info.m_stopTime.second = tmpInt3;

	// 11. The start wavelength (ignore)
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}

	// 12. The stop wavelength (ignore)
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}

	// 13. The number of scans
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(1 > sscanf(buffer, "SCANS %d", &tmpInt)){
		fclose(f); return FAIL;
	}
	spec.m_info.m_numSpec = tmpInt;

	// 14. The integration time
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(1 > sscanf(buffer, "INT_TIME %lf", &tmpDbl)){
		fclose(f); return FAIL;
	}
	spec.m_info.m_exposureTime = (int)tmpDbl;

	// 15. The site
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	buffer[strlen(buffer) - 1] = 0; // <-- Remove the trailing newline character
	spec.m_info.m_name = std::string(buffer + 5);

	// 15. The longitude
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(1 > sscanf(buffer, "LONGITUDE %lf", &tmpDbl)){
		fclose(f); return FAIL;
	}
	spec.m_info.m_gps.m_longitude = tmpDbl;

	// 15. The latitude
	if(NULL == fgets(buffer, bufSize, f)){
		fclose(f); return FAIL;
	}
	if(1 > sscanf(buffer, "LATITUDE %lf", &tmpDbl)){
		fclose(f); return FAIL;
	}
	spec.m_info.m_gps.m_latitude = tmpDbl;

	// ----------- EXTENDED STD ------------------
	// - if the file is in the extended STD-format then we can continue here... -
  char szLine[8192];
  while(fgets(szLine, 8192, f)){

		// Read in scanAngle
		if(NULL != strstr(szLine, "ElevationAngle = ")){
			char *pt = strstr(szLine, "ElevationAngle = ");
			if (sscanf(pt + 17, "%lf", &tmpDbl) == 1) {
				if (fabs(tmpDbl) < 360.0) {
					spec.m_info.m_scanAngle = (float)tmpDbl;
				}
			}
		}

		// Read in scanAngle2
		if(NULL != strstr(szLine, "AzimuthAngle = ")){
			char *pt = strstr(szLine, "AzimuthAngle = ");
			if (sscanf(pt + 15, "%lf", &tmpDbl) == 1) {
				if (fabs(tmpDbl) < 360.0) {
					spec.m_info.m_scanAngle2 = (float)tmpDbl;
				}
			}
		}

		// Read in the temperature
		if(NULL != strstr(szLine, "Temperature = ")){
			char *pt = strstr(szLine, "Temperature = ");
			if (sscanf(pt + 14, "%lf", &tmpDbl) == 1) {
				if (fabs(tmpDbl) < 100.0) {
					spec.m_info.m_temperature = (float)tmpDbl;
				}
			}
		}
	}

	// Get the intensity
	float numSpec_inv = 1 / (float)spec.NumSpectra();
	spec.m_info.m_peakIntensity = (float)spec.MaxValue() * numSpec_inv;
	spec.m_info.m_offset				= (float)spec.GetOffset() * numSpec_inv;

	// If the intensity is saved in the ddmm.mmmm - format (which is what
	//	the gps-reciever sends out), convert it to the dd.dddddd format
	if(fabs(spec.m_info.m_gps.m_latitude) > 180)
		spec.m_info.m_gps.m_latitude = CGPSData::DoubleToAngle(spec.m_info.m_gps.m_latitude);
	if(fabs(spec.m_info.m_gps.m_longitude) > 180)
		spec.m_info.m_gps.m_longitude = CGPSData::DoubleToAngle(spec.m_info.m_gps.m_longitude);

	fclose(f);
	return SUCCESS;
}
RETURN_CODE CSTDFile::WriteSpectrum(const CSpectrum *spec, const CString &fileName, int extendedFormat){
	return WriteSpectrum(*spec, fileName);
}

RETURN_CODE CSTDFile::WriteSpectrum(const CSpectrum &spec, const CString &fileName, int extendedFormat){
	const CSpectrumInfo &info = spec.m_info;
	FILE *f = fopen(fileName, "w");
	if(f == NULL)
		return FAIL;

	int i;

	fprintf(f, "GDBGMNUP\n1\n");
	fprintf(f, "%ld\n", spec.m_length);
	for(i = 0; i < spec.m_length; ++i){
		if(fabs(spec.m_data[i] - floor(spec.m_data[i])) > 1e-9)
			fprintf(f, "%.9lf\n", spec.m_data[i]);
		else
			fprintf(f, "%.0lf\n", spec.m_data[i]);
	}

	int index = fileName.ReverseFind('\\');
	if(index != 0)
		fprintf(f, "%s\n", (LPCSTR)fileName.Left(index));
	else
		fprintf(f, "%s\n", (LPCSTR)fileName);
	fprintf(f, ".......\n"); // the detector
	fprintf(f, "%s\n", info.m_device.c_str()); // the spectrometer

	if(info.m_startTime.year == info.m_startTime.month && info.m_startTime.month == info.m_startTime.day && info.m_startTime.day == 0){
		fprintf(f, "01.01.01\n"); /* Default date */
	}else{
		fprintf(f, "%02d.%02d.%02d\n", info.m_startTime.year, info.m_startTime.month, info.m_startTime.day); // the date
	}
	fprintf(f, "%02d:%02d:%02d\n", info.m_startTime.hour, info.m_startTime.minute, info.m_startTime.second);
	fprintf(f, "%02d:%02d:%02d\n", info.m_stopTime.hour, info.m_stopTime.minute, info.m_stopTime.second);

	fprintf(f, "0.0\n0.0\n"); // the start and stop wavelengths

	fprintf(f, "SCANS %ld\n", info.m_numSpec);
	fprintf(f, "INT_TIME %ld\n", info.m_exposureTime);

	fprintf(f, "SITE %s\n", info.m_name.c_str());
	fprintf(f, "LONGITUDE %.6lf\n", info.m_gps.m_longitude);
	fprintf(f, "LATITUDE %.6lf\n", info.m_gps.m_latitude);

	if(extendedFormat){
		fprintf(f, "Author = \"\"\n");
		fprintf(f, "Average = %.2lf\n", spec.AverageValue());
		fprintf(f, "AzimuthAngle = 0\n");
		fprintf(f, "Delta = 0\n");
		fprintf(f, "DeltaRel = 0\n");
		fprintf(f, "Deviation = 0\n");
		fprintf(f, "Device = \"\"\n");
		fprintf(f, "ElevationAngle = %.2lf\n",	info.m_scanAngle);
		fprintf(f, "ExposureTime = %d\n",				info.m_exposureTime);
		fprintf(f, "FileName = %s\n", (LPCSTR)fileName);
		fprintf(f, "FitHigh = 0\n");
		fprintf(f, "FitLow = 0\n");
		fprintf(f, "Gain = 0\n");
		fprintf(f, "Latitude = %.6lf\n",				info.m_gps.m_latitude);
		fprintf(f, "LightPath = 0\n");
		fprintf(f, "LightSource = \"\"\n");
		fprintf(f, "Longitude = %.6lf\n",				info.m_gps.m_longitude);
		fprintf(f, "Marker = %d\n",							spec.m_length / 2);
		fprintf(f, "MathHigh = %d\n",						spec.m_length - 1);
		fprintf(f, "MathLow = 0\n");
		fprintf(f, "Max = %.3lf\n",							spec.MaxValue());
		fprintf(f, "MaxChannel = %d\n",					spec.m_length);
		fprintf(f, "Min = %.3lf\n",							spec.MinValue());
		fprintf(f, "MinChannel = 0\n");
		fprintf(f, "MultiChannelCounter = 0\n");
		fprintf(f, "Name = \"%s\"\n", info.m_name.c_str());
		fprintf(f, "NumScans = %d\n",						info.m_numSpec);
		fprintf(f, "OpticalDensity = 0\n");
		fprintf(f, "OpticalDensityCenter = %d\n",	spec.m_length / 2);
		fprintf(f, "OpticalDensityLeft = 0\n");
		fprintf(f, "OpticalDensityRight = %d\n",	spec.m_length - 1);
		fprintf(f, "Pressure = 0\n");
		fprintf(f, "Remark = \"\"\n");
		fprintf(f, "ScanGeometry = 0\n"); //(DoasCore.Math.ScanGeometry)SAZ: 137.41237083135 SZA: 31.5085943481828 LAZ: 298.523110145623 LAZ: 129.285101310559 Date: 1/5/2007 10:35:07 Lat.: 0 Lon.: 0\n");
		fprintf(f, "ScanMax = 0\n");
		fprintf(f, "Temperature = %.2f\n",				info.m_temperature);
		fprintf(f, "Variance = 0\n");
	}

	fclose(f);

	return SUCCESS;
}

