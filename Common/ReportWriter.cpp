// ReportWriter.cpp : implementation file
//
#include "StdAfx.h"
#include "../NovacMasterProgram.h"
#include "ReportWriter.h"
#include "Common.h"
#include "../Configuration/configuration.h"
#include "../VolcanoInfo.h"
#include "../UserSettings.h"

#include <atlimage.h>

using namespace FileHandler;

extern CConfigurationSetting g_settings;    // <-- the settings
extern CVolcanoInfo g_volcanoes;            // <-- the list of volcanoes
extern CFormView *pView;                    // <-- the main window
extern CUserSettings g_userSettings;        // <-- The users preferences
// CReportWriter

IMPLEMENT_DYNCREATE(CReportWriter, CWinThread)

CReportWriter::CReportWriter()
{
}

CReportWriter::~CReportWriter()
{
}

BOOL CReportWriter::InitInstance(){
	CWinThread::InitInstance();

	m_nTimerID = 0;

	SetTimer();

	return 1;
}

void CReportWriter::SetTimer(){
	long lastSleep = -1, curTime = 0;

	// If the timer is already running, then we need to stop it...
	if(0 != m_nTimerID)
	{
		::KillTimer(NULL, m_nTimerID);
	}

	// Find the last sleeptime
	for(unsigned int k = 0; k < g_settings.scannerNum; ++k){
		long sleepTime = 3600 * g_settings.scanner[k].comm.sleepTime.hour + 60 * g_settings.scanner[k].comm.sleepTime.minute;
		if(sleepTime > lastSleep){
			lastSleep = sleepTime;
		}
	}
//	lastSleep = lastSleep + 7200; // make the time 2 hours after the last sleeping time
	lastSleep = lastSleep + 60; // make the time 1 minute after the last sleeping time

	// Get the current time
	CTime currentTime = CTime::GetCurrentTime();
	curTime						= 3600 * currentTime.GetHour() + 60 * currentTime.GetMinute();

	// if we're starting up during the sleeping time...
	while(lastSleep <= curTime)
		lastSleep += 86400;

	// Set a timer to wake up 2 hours after the last instrument has fallen asleep...
	m_nTimerID = ::SetTimer(NULL, 0, (lastSleep - curTime) * 1000, NULL);
}

int CReportWriter::ExitInstance()
{
	if(0 != m_nTimerID)
	{
		::KillTimer(NULL, m_nTimerID);
	}
	m_nTimerID = 0;


	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CReportWriter, CWinThread)
	ON_THREAD_MESSAGE(WM_TIMER,						OnTimer)
END_MESSAGE_MAP()


// CReportWriter message handlers


void CReportWriter::OnTimer(UINT nIDEvent, LPARAM lp){
	pView->PostMessage(WM_WRITE_REPORT);

	// Set the timer for a new period
	SetTimer();
}


/** Called to write the report */
void CReportWriter::WriteReport(CEvaluatedDataStorage *evalDataStorage, CCommunicationDataStorage *commDataStorage){
	CString reportFileName, dateStr, fluxUnitStr, style;
//	CString imageDirectory, imageFileName_temp, imageFileName_linkSpeed, imageFileName_battery, imageFileName_flux;
	long numDataPoints, numGoodDataPoints;

	// Check that we have something to write
	if(evalDataStorage == NULL || commDataStorage == NULL)
		return;

	// Allocate some memory for storing the data
	const int BUF_SIZE  = 1024;
	double *timeBuffer = new double[BUF_SIZE];
	double *dataBuffer = new double[BUF_SIZE];
	int *qualityBuffer	= new int[BUF_SIZE];

	// clear the buffers
	memset(timeBuffer,		0, BUF_SIZE * sizeof(double));
	memset(dataBuffer,		0, BUF_SIZE * sizeof(double));
	memset(qualityBuffer, 0, BUF_SIZE * sizeof(int));

	// The name of the report-file
	Common::GetDateText(dateStr);
	reportFileName.Format("%s\\Output\\%s\\DailyReport_%s.html", g_settings.outputDirectory, dateStr, dateStr);

	//// The names of the images
	//imageDirectory.Format("%sOutput\\%s\\",							g_settings.outputDirectory, dateStr);
	//imageFileName_flux.Format("FluxHist_%s.png",				dateStr);
	//imageFileName_temp.Format("TempHist_%s.png",				dateStr);
	//imageFileName_linkSpeed.Format("LinkSHist_%s.png",	dateStr);
	//imageFileName_battery.Format("BatteryHist_%s.png",	dateStr);

	// Open the file
	FILE *f = fopen(reportFileName, "w");
	if(f == NULL){
		delete [] timeBuffer, qualityBuffer, dataBuffer;
		return;
	}

	// Generate the header
	WriteReportHeader(f);

	// Loop through each of the connected systems...
	for (unsigned int instrument = 0; instrument < g_settings.scannerNum; ++instrument){
		const CString &serial = g_settings.scanner[instrument].spec[0].serialNumber;
	
		// Write a header for this spectrometer
		fprintf(f, "<h1>%s (%s)</h1>\r\n", g_settings.scanner[instrument].site, serial);

		// Get the flux data
		numDataPoints			= evalDataStorage->GetFluxData(serial, timeBuffer, dataBuffer, qualityBuffer, BUF_SIZE);
		numGoodDataPoints		= RemoveBadFluxData(timeBuffer, dataBuffer, qualityBuffer, numDataPoints);
		FLUX_UNIT fluxUnit		= g_userSettings.m_fluxUnit;
		if(fluxUnit == UNIT_TONDAY)
			fluxUnitStr.Format("ton/day");
		else
			fluxUnitStr.Format("kg/s");

		// ------------ Write a table with the flux data -------------
		fprintf(f, "<p><table border=\"2\" frame=\"box\" rules=\"groups\">\r\n");
		fprintf(f, "<caption>Statistics for todays flux data</caption>\r\n");

		fprintf(f, "<colgroup align=\"left\">\r\n");
		fprintf(f, "<colgroup align=\"right\" span=\"2\">\r\n");
		fprintf(f, "<colgroup align=\"right\">\r\n");

		fprintf(f, "<tbody>\r\n");
		if(numGoodDataPoints == 0 || (numDataPoints - numGoodDataPoints >= 2 * numGoodDataPoints))
			style.Format(" id=\"warning\"");
		else
			style.Format("");

		fprintf(f, "<tr><th>Number of good scans</th><td%s>%ld</td><td></td><td></td></tr>\r\n",  style, numGoodDataPoints);
		fprintf(f, "<tr><th>Number of bad scans</th><td%s>%ld</td><td></td><td></td></tr>\r\n",	 style, numDataPoints - numGoodDataPoints);

		fprintf(f, "<tbody>\r\n");
//		fprintf(f, "<tr><th>Average Flux</th><td>%.1lf</td><td>%s</td><td><img src=\"%s\"></td></tr>\r\n", Average(dataBuffer, numGoodDataPoints), fluxUnitStr, imageFileName_flux);
		fprintf(f, "<tr><th>Average Flux</th><td>%.1lf</td><td>%s</td><td></tr>\r\n",  Average(dataBuffer, numGoodDataPoints), fluxUnitStr);
		fprintf(f, "<tr><th>Std Flux</th><td>%.1lf</td><td>%s</td><td></td></tr>\r\n", Std(dataBuffer, numGoodDataPoints), fluxUnitStr);
		fprintf(f, "<tr><th>Max Flux</th><td>%.1lf</td><td>%s</td><td></td></tr>\r\n", Max(dataBuffer, numGoodDataPoints), fluxUnitStr);
		fprintf(f, "<tr><th>Min Flux</th><td>%.1lf</td><td>%s</td><td></td></tr>\r\n", Min(dataBuffer, numGoodDataPoints), fluxUnitStr);

		fprintf(f, "</table>\r\n</p>");

		// Generate the histogram...
		//WriteDataHistogram(dataBuffer, numDataPoints, numDataPoints/10, imageDirectory, imageFileName_flux);

		// Get the temperature data
		numDataPoints = evalDataStorage->GetTemperatureData(serial, timeBuffer, dataBuffer, BUF_SIZE);

		// -------------- Write a table with the temperature data --------------
		fprintf(f, "<p><table border=\"2\" frame=\"box\" rules=\"groups\">\r\n");
		fprintf(f, "<caption>Statistics for todays temperature</caption>\r\n");

		fprintf(f, "<colgroup align=\"left\">\r\n");
		fprintf(f, "<colgroup align=\"right\" span=\"2\">\r\n");

		fprintf(f, "<tbody>\r\n");
		fprintf(f, "<tr><th>Average Temp</th><td>%.1lf</td><td> °C</td></tr>\r\n",  Average(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Max Temp</th><td>%.1lf</td><td> °C</td></tr>\r\n",		  Max(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Min Temp</th><td>%.1lf</td><td> °C</td></tr>\r\n",		  Min(dataBuffer, numDataPoints));

		fprintf(f, "</table>\r\n</p>");

		// Generate the histogram...
///		WriteDataHistogram(dataBuffer, numDataPoints, numDataPoints/10, imageDirectory, imageFileName_temp);
//		fprintf(f, "<img src=\"%s\">\r\n",  imageFileName_temp);

		// Get the temperature data
		numDataPoints = evalDataStorage->GetBatteryVoltageData(serial, timeBuffer, dataBuffer, BUF_SIZE);

		// -------------- Write a table with the battery voltage data --------------
		fprintf(f, "<p><table border=\"2\" frame=\"box\" rules=\"groups\">\r\n");
		fprintf(f, "<caption>Statistics for todays voltage of the battery</caption>\r\n");

		fprintf(f, "<colgroup align=\"left\">\r\n");
		fprintf(f, "<colgroup align=\"right\" span=\"2\">\r\n");

		fprintf(f, "<tbody>\r\n");
		fprintf(f, "<tr><th>Average Voltage</th><td>%.1lf</td><td> V</td></tr>\r\n",  Average(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Max Voltage</th><td>%.1lf</td><td> V</td></tr>\r\n",		  Max(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Min Voltage</th><td>%.1lf</td><td> V</td></tr>\r\n",		  Min(dataBuffer, numDataPoints));

		fprintf(f, "</table>\r\n</p>");

		// Generate the histogram...
		//WriteDataHistogram(dataBuffer, numDataPoints, numDataPoints/10, imageDirectory, imageFileName_battery);
		//fprintf(f, "<img src=\"%s\">\r\n",  imageFileName_battery);

		// Get the link-speed data
		numDataPoints = commDataStorage->GetLinkSpeedData(serial, timeBuffer, dataBuffer, BUF_SIZE);
		// -------------- Write a table with the link speed data --------------
		fprintf(f, "<p><table border=\"2\" frame=\"box\" rules=\"groups\">\r\n");
		fprintf(f, "<caption>Statistics for todays downloads</caption>\r\n");

		fprintf(f, "<colgroup align=\"left\">\r\n");
		fprintf(f, "<colgroup align=\"right\" span=\"2\">\r\n");

		fprintf(f, "<tbody>\r\n");
		fprintf(f, "<tr><th>Number of Downloads</th><td>%ld</td><td></td></tr>\r\n",		numDataPoints);
		fprintf(f, "<tr><th>Average Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",  Average(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Max Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",		  Max(dataBuffer, numDataPoints));
		fprintf(f, "<tr><th>Min Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",		  Min(dataBuffer, numDataPoints));

		fprintf(f, "</table>\r\n</p>");

		// Generate the histogram...
		//WriteDataHistogram(dataBuffer, numDataPoints, numDataPoints/10, imageDirectory, imageFileName_linkSpeed);
		//fprintf(f, "<img src=\"%s\">\r\n",  imageFileName_linkSpeed);
	}

	// Finally, write the link speed to the FTP-server
	numDataPoints = commDataStorage->GetLinkSpeedData("FTP", timeBuffer, dataBuffer, BUF_SIZE);

	// -------------- Write a table with the link speed data for the FTP-server --------------
	fprintf(f, "<h1>Uploaded data to FTP - Server</h1>\r\n");
	fprintf(f, "<p><table border=\"2\" frame=\"box\" rules=\"groups\">\r\n");
	fprintf(f, "<caption>Statistics for todays uploads to FTP server</caption>\r\n");

	fprintf(f, "<colgroup align=\"left\">\r\n");
	fprintf(f, "<colgroup align=\"right\" span=\"2\">\r\n");

	fprintf(f, "<tbody>\r\n");
	fprintf(f, "<tr><th>Number of Uploads</th><td>%ld</td><td></td></tr>\r\n",			numDataPoints);
	fprintf(f, "<tr><th>Average Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",  Average(dataBuffer, numDataPoints));
	fprintf(f, "<tr><th>Max Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",		  Max(dataBuffer, numDataPoints));
	fprintf(f, "<tr><th>Min Link Speed</th><td>%.1lf</td><td> kb/s</td></tr>\r\n",		  Min(dataBuffer, numDataPoints));

	fprintf(f, "</table>\r\n</p>");

	// Generate the footer
	WriteReportFooter(f);

	// Remember to close the file
	fclose(f);

	// Clear up...
	delete [] timeBuffer, qualityBuffer, dataBuffer;

}

/** Generate the header of the Report */
void CReportWriter::WriteReportHeader(FILE *f){
	CString dateStr;
	Common::GetDateText(dateStr);

	if(f == NULL)
		return;

	fprintf(f, "<html>\r\n");
	fprintf(f, "<head>\r\n");
	fprintf(f, "<title>End of Day Report %s</title>\r\n", dateStr);
	fprintf(f, "<style type=\"text/css\">\r\n");
	fprintf(f, "#warning {font-style: italic; color: red}\r\n");
	fprintf(f, "</style>\r\n");
	fprintf(f, "</head>\r\n");
	fprintf(f, "<body>\r\n");
}

/** Generate the footer of the Report */
void CReportWriter::WriteReportFooter(FILE *f){
	if(f == NULL)
		return;

	fprintf(f, "</body>\r\n");
	fprintf(f, "</html>\r\n");
}

long CReportWriter::RemoveBadFluxData(double *timeBuffer, double *dataBuffer, int *qualityBuffer, long numDataPoints){
	double *tempDataBuffer = new double[numDataPoints];
	double *tempTimeBuffer = new double[numDataPoints];
	memset(tempDataBuffer, 0, numDataPoints * sizeof(double));
	memset(tempTimeBuffer, 0, numDataPoints * sizeof(double));

	// Copy the good data points to the temporary buffers
	long nGoodFluxes = 0;
	for(int k = 0; k < numDataPoints; ++k){
		if(qualityBuffer[k]){
			tempDataBuffer[nGoodFluxes] = dataBuffer[k];
			tempTimeBuffer[nGoodFluxes] = timeBuffer[k];
		}
	}

	// Copy the temporary buffers to the output-buffers
	if(nGoodFluxes > 0){
		memcpy(dataBuffer, tempDataBuffer, nGoodFluxes * sizeof(double));
		memcpy(timeBuffer, tempTimeBuffer, nGoodFluxes * sizeof(double));
	}

	// Return the number of data points in the buffers now
	return nGoodFluxes;
}

/** Generate a histogram of the data and stores it to a graphics file */
int CReportWriter::WriteDataHistogram(double *data, long nDataPoints, long nBins, const CString &filePath, const CString &fileName){
	DWORD errorCode;
	long *sortedData = new long[nBins];
	memset(sortedData, 0, nBins * sizeof(long));
	long curBin;
	CRect rect;
	CString axisStr;
	CString fileName_WMF, fullFileName;
	fullFileName.Format("%s%s",			filePath, fileName);

	fileName_WMF.Format(fullFileName.Left(CString::StringLength(fullFileName) - 3));
	fileName_WMF.AppendFormat("wmf");

	double imageHeight = 200;
	double axisMargin  = 20;

	// Make the sorting of the data...
	double maxV			= Max(data, nDataPoints);
	double minV			= Min(data, nDataPoints);
	double binSize	= (maxV - minV) / (double)nBins; // the size of each bin
	binSize					= (binSize == 0.0) ? 0.1 : binSize;
	for(int k = 0; k < nDataPoints; ++k){
		curBin = (data[k] - minV) / binSize;
		if(curBin >= 0 && curBin < nBins){
			++sortedData[curBin];
		}else{
			++sortedData[nBins - 1];
		}
	}
	double maxBinHeight = Max(sortedData, nBins);

	CMetaFileDC *dc = new CMetaFileDC();
	if(dc == NULL)
		return 1; // FAIL
	int ret = dc->Create(fileName_WMF); // create a meta-file
	if(ret == 0 || dc->m_hDC){
		delete dc;
		return 1; // FAIL;
	}

	// Draw the histogram...

	// 1. Draw the bins
	double binWidth = imageHeight / nBins;
	double x1, x2, y1, y2;
	CPoint pt[4]; // each rectangle...
	CBrush barBrush(RGB(0, 0, 240));
	CBrush *pBrush = dc->SelectObject(&barBrush);
	for(k = 0; k < nBins; ++k){
		x1 = axisMargin + binWidth * k;
		x2 = axisMargin + binWidth * (k+1);
		y1 = imageHeight - axisMargin;
		y2 = imageHeight - (axisMargin + (imageHeight - axisMargin) * sortedData[k] / maxBinHeight);

		pt[0]	= CPoint(x1, y1);
		pt[1]	= CPoint(x1, y2);
		pt[2]	= CPoint(x2, y2);
		pt[3]	= CPoint(x2, y1);

		dc->Polygon(pt, 4);
	}

	// 2. Draw the axes
	dc->MoveTo(axisMargin,		axisMargin);
	dc->LineTo(axisMargin,		imageHeight);
	dc->LineTo(imageHeight,	imageHeight);
	dc->LineTo(imageHeight,	axisMargin);
	dc->LineTo(axisMargin,		axisMargin);

	// 2a. minimum value
	//rect.left		= axisMargin;
	//rect.bottom = imageHeight - 1;
	//rect.right	= imageHeight;
	//rect.top		= imageHeight - 20;
	//axisStr.Format("%.1lf", minV);
	//dc->DrawText(axisStr, rect, DT_END_ELLIPSIS);
	rect.left		= (LONG)imageHeight;
	rect.bottom = (LONG)(imageHeight - 20);
	rect.right	= (LONG)axisMargin;
	rect.top		= (LONG)(imageHeight - 1);
	axisStr.Format("%.1lf", minV);
	dc->DrawText(axisStr, rect, DT_END_ELLIPSIS);

	// 2a. maximum value
	//rect.left		= imageHeight * 0.9;
	//rect.bottom = imageHeight;
	//rect.right	= imageHeight;
	//rect.top		= imageHeight - 20;
	//axisStr.Format("%.1lf", maxV);
	//dc->DrawText(axisStr, rect, DT_END_ELLIPSIS);


	dc->SelectObject(pBrush);

	// Close the file
	dc->Close();

	// Release the memory
	free(sortedData);
	delete dc;

	// Make the file a file of the given file-format instead of a .wmf - file
	CImage image;
	image.Load(fileName_WMF);
	image.Save(fullFileName);

	// Remove the original .wmf - file
	BOOL bFileDeleted = DeleteFile(fileName_WMF);
	if(0 == bFileDeleted){
		errorCode = GetLastError();
	}

	return 0; // no problems
}
