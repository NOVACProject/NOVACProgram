#include "StdAfx.h"
#include "postfluxcalculator.h"

#include "../Common/Version.h"

using namespace PostFlux;

CPostFluxCalculator::CPostFluxCalculator(void)
{

	m_scanNum = 0;
	m_gasFactor = GASFACTOR_SO2;

	m_compass		= 0;
	m_coneAngle		= 90.0;
	m_tilt			= 0.0;
	m_flux			= 0;
	m_specieNum		= m_curSpecie = 0;
}

CPostFluxCalculator::~CPostFluxCalculator(void)
{
}


double CPostFluxCalculator::CalculateFlux(int scanNr){
	double plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high;

	// Write the header of the flux-log file, if necessary
	WriteFluxLogHeader(scanNr);

	// Calculate the flux
	m_scan[scanNr].CalculateFlux(m_specie[m_curSpecie], m_wind, m_compass, m_coneAngle, m_tilt);

	// Calculate the plume-centre
	m_scan[scanNr].CalculatePlumeCentre(m_specie[m_curSpecie], plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high);

	// Get the flux
	m_flux = m_scan[scanNr].GetFlux();

	// Append the flux to the flux-log file
	AppendToFluxLog(scanNr);

	// Remember the wind-field that was used
	m_windField[scanNr].SetPlumeHeight(m_wind.GetPlumeHeight(), m_wind.GetPlumeHeightSource());
	m_windField[scanNr].SetWindSpeed(m_wind.GetWindSpeed(), m_wind.GetWindSpeedSource());
	m_windField[scanNr].SetWindDirection(m_wind.GetWindDirection(), m_wind.GetWindDirectionSource());

	return m_flux;
}

/** Calculates the offset for the given scan. If anyone of the parameters 
    'chi2limit', 'intensAbove', or 'intensBelow' is supplied it will be
    used instead of the default parameter. */
double CPostFluxCalculator::CalculateOffset(int scanNr, const CString & specie, float chi2Limit, float intensAbove, float intensBelow){
	if(scanNr < 0 || scanNr > m_scanNum)
		return 0.0;

	Evaluation::CScanResult &scan = m_scan[scanNr];

	for(unsigned long k = 0; k < scan.GetEvaluatedNum(); ++k){
		// Check the goodness of fit using the supplied parameters
		scan.CheckGoodnessOfFit(scan.GetSpectrumInfo(k), k, chi2Limit, intensBelow, intensAbove);
	}

	// Do the final calculation of the offset
	scan.CalculateOffset(specie);

	return scan.GetOffset();
}


/** Manually sets the offset for the given scan */
void  CPostFluxCalculator::SetOffset(double offset, int scanNr){
	if(scanNr < 0 || scanNr > m_scanNum)
		return;

	m_scan[scanNr].SetOffset(offset);
}

/** Writes the header of the post-flux log file */
void  CPostFluxCalculator::WriteFluxLogHeader(int scanNr){
	CString fluxFilePath, str, serial;
	fluxFilePath.Format(m_evaluationLog);
	Common::GetDirectory(fluxFilePath);

	serial = (scanNr < 0) ? m_scan[0].GetSerial() : m_scan[scanNr].GetSerial();

	if(CString::StringLength(serial) == 0)
	  m_logFile.Format("%sPostFluxLog.txt", (LPCTSTR)fluxFilePath);
	else
		m_logFile.Format("%sPostFluxLog_%s.txt", (LPCTSTR)fluxFilePath, (LPCTSTR)serial);

	if(IsExistingFile(m_logFile))
		return; // if the file already exists, then we don't need to add anything...

	FILE *f = fopen(m_logFile, "a+");
	if(NULL != f){
		str.Format("\nFluxLogFile generated by NovacProgram version %d.%02d, build %s", CVersion::majorNumber, CVersion::minorNumber, __DATE__);

		str.AppendFormat("\nscandate\tscanstarttime\tscanstoptime\tflux_[kg/s]\tflux_[ton/day]\t");
		str.AppendFormat("windspeed_[m/s]\twindspeedsource\twinddirection_[deg]\twinddirectionsource\tcompassdirection_[deg]\tconeangle_[deg]\ttilt_[deg]\tplumeheight_[m]\tplumeheightsource\t");
		str.AppendFormat("offset\tplumecentre_[deg]\tplumeedge1_[deg]\tplumeedge2_[deg]\tplumecompleteness_[%%]\t");
		str.AppendFormat("geom_error\tspectr_error\tscattering_error\twind_error\n");

		fprintf(f, str);

		fclose(f);
	}else{
		CString msg;
		msg.Format("Could not create the following post-flux log-file for writing: %s", (LPCTSTR)m_logFile);
		MessageBox(NULL, msg, "Error", MB_OK);
	}
}

/** Appends a result to the end of the post-flux log file */
void  CPostFluxCalculator::AppendToFluxLog(int scanNr){
	CString str;
	CString wsSrc, wdSrc, phSrc;
	double edge1, edge2;
	
	// Get the sources of wind-information
	m_wind.GetWindSpeedSource(wsSrc);
	m_wind.GetWindDirectionSource(wdSrc);
	m_wind.GetPlumeHeightSource(phSrc);
	
	// Write the header of the flux-log file, if necessary
	WriteFluxLogHeader(scanNr);

	FILE *f = fopen(m_logFile, "a+");
	if(f != NULL){
		CDateTime dateNTime, stopTime;
		m_scan[scanNr].GetStartTime(0, dateNTime);
		m_scan[scanNr].GetStopTime(m_scan[scanNr].GetEvaluatedNum() - 1, stopTime);

		// 1. Date
		str.Format("%04d-%02d-%02d\t", dateNTime.year, dateNTime.month, dateNTime.day);

		// 2. StartTime
		str.AppendFormat("%02d:%02d:%02d\t", dateNTime.hour, dateNTime.minute, dateNTime.second);

		// 3. StopTime
		str.AppendFormat("%02d:%02d:%02d\t", stopTime.hour, stopTime.minute, stopTime.second);

		// 4. Flux
		str.AppendFormat("%.2lf\t", m_flux);	// <-- the flux in kg/s
		str.AppendFormat("%.2lf\t", m_flux*3.6*24.0);	// <-- the flux in ton/day

		// 5. Wind speed and direction
		str.AppendFormat("%.2lf\t%s\t", m_wind.GetWindSpeed(), (LPCTSTR)wsSrc);
		str.AppendFormat("%.2lf\t%s\t", m_wind.GetWindDirection(), (LPCTSTR)wdSrc);

		// 6. Compass direction of the scanning instrument
		str.AppendFormat("%.2lf\t", m_compass);

		// 7. The cone angle of the scanner
		str.AppendFormat("%.2lf\t", m_coneAngle);

		// 8. The tilt of the scanner
		str.AppendFormat("%.2lf\t", m_tilt);

		// 9. Plume height
		str.AppendFormat("%.2lf\t%s\t", m_wind.GetPlumeHeight(), (LPCTSTR)phSrc);

		// 10. Offset
		str.AppendFormat("%.2lf\t", m_scan[scanNr].GetOffset());

		// 11. The calculated centre position of the plume
		str.AppendFormat("%.1lf\t", m_scan[scanNr].GetCalculatedPlumeCentre());
		
		// 12. The calculated edges of the plume
		m_scan[scanNr].GetCalculatedPlumeEdges(edge1, edge2);
		str.AppendFormat("%.1lf\t", edge1);
		str.AppendFormat("%.1lf\t", edge2);

		// 13. The calculated completeness of the plume
		str.AppendFormat("%.2lf\t", m_scan[scanNr].GetCalculatedPlumeCompleteness());

		// 14. The Geometrical error
		str.AppendFormat("%.1lf\t", m_scan[scanNr].GetGeometricalError());

		// 15. The spectroscopical error
		str.AppendFormat("%.1lf\t", m_scan[scanNr].GetSpectroscopicalError());

		// 16. The scattering error
		str.AppendFormat("%.1lf\t", m_scan[scanNr].GetScatteringError());

		// 17. The wind error
		str.AppendFormat("%.1lf\n", m_wind.GetWindError());

		// Write it all
		fprintf(f, str);
		fclose(f);
	}
}