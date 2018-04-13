#include "StdAfx.h"
#include "Common.h"

// include the global settings
#include "../Configuration/Configuration.h"
#include "../VolcanoInfo.h"
#include "../Communication/FTPServerContacter.h"

#include "PSAPI.H"
#include <tlhelp32.h>
#include <VersionHelpers.h>
#pragma comment( lib, "PSAPI.LIB" )

extern CFormView *pView;
extern CConfigurationSetting g_settings;	// <-- the settings for the scanners
extern CVolcanoInfo g_volcanoes;					// <-- the list of volcanoes
extern CWinThread *g_ftp;									// <-- The Ftp-uploading thread.

long GetSleepTime(struct timeStruct& startTime,struct timeStruct& stopTime)
{
	CTime currentTime;
	int hr,min,sec;
	long waitTime, currentT,startT,stopT;
	waitTime = -1;
	currentTime = CTime::GetCurrentTime();
	hr = currentTime.GetHour();
	min = currentTime.GetMinute();
	sec = currentTime.GetSecond();	
	currentT = hr*3600 + min*60 + sec;	
	startT	= startTime.hour*3600 + startTime.minute*60 + startTime.second;
	stopT		= stopTime.hour*3600 + stopTime.minute*60 + stopTime.second;
	if(startT > stopT)
	{
		if((currentT>=startT)&&(currentT <= 86399))		
			waitTime = (86400 - currentT + stopT)*1000;	
		else if((currentT<=stopT)&&(currentT>=0))
			waitTime = (stopT - currentT)*1000;				
	}
	else if(startT < stopT)
	{
		if((currentT >= startT)&&(currentT <= stopT))
			waitTime = (stopT - currentT)*1000;			
	}
	else if(startT == stopT)
	{
		if(currentT >= startT)
			waitTime = (86400 - currentT + startT)*1000;
	}
	return waitTime;
}
void GetSysTempFolder(CString& folderPath)
{
	TCHAR buffer[MAX_PATH];
	GetTempPath(MAX_PATH, buffer);
	folderPath.Format("%s", buffer);
}

int IsExistingFile(const CString &fileName){
	WIN32_FIND_DATA FindFileData;
	char fileToFind[MAX_PATH];

	sprintf(fileToFind, "%s", (LPCSTR)fileName);

	// Search for the file
	HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return 0; // file not found

	FindClose(hFile);

	return 1; // file found
}

int CreateDirectoryStructure(const CString &path)
{
	char buffer [1024]; // buffer is a local copy of 'path'
	memset(buffer, 0, 1024*sizeof(char));
	if(strlen(path) > 1023)
		return 1;

  char pathSeparator;
#ifdef WINDOWS
	pathSeparator = '\\';
#else
	pathSeparator = '//';
#endif

	int ret;

	memcpy(buffer, path, _tcslen(path));

	// add a finishing backslash if it does not exist already
	if(buffer[strlen(buffer)-1] != pathSeparator)
		buffer[strlen(buffer)] = pathSeparator;

	char *pt = strchr(buffer, pathSeparator);

	while(pt != NULL){
		pt[0] = 0;
		if((strlen(buffer) == 2) && (':' == buffer[1])){
			// do nothing, don't create C: !!
		}else{
			ret = CreateDirectory(buffer, NULL);
			if(!ret){
				int error = GetLastError();
				if(error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS){
					CString message;
					message.Format("Debug: Could not create output directory: %s", buffer);
					ShowMessage(message); // tell the user about our problems...
					return 1;
				}
			}
		}
		pt[0] = pathSeparator;
		if(*(pt+1) == pathSeparator)
			pt = strchr(pt+2, pathSeparator);
		else
			pt = strchr(pt+1, pathSeparator);
	}

	return 0;
}

int Equals(const CString &str1, const CString &str2){
	return (0 ==_tcsnicmp(str1, str2, max(strlen(str1), strlen(str2))));
}

int Equals(const CString &str1, const CString &str2, unsigned int nCharacters){
	return (0 ==_tcsnicmp(str1, str2, min(nCharacters, max(strlen(str1), strlen(str2)))));
}

/** Sends a message that the given file should be uploaded to the NOVAC-Server
		as soon as possible */
void UploadToNOVACServer(const CString &fileName, int volcanoIndex, bool deleteFile){
	CString *fileNameBuffer = NULL;
	Communication::FTPUploadOptions *options;
	CString message;

	// The uploading thread is not running, quit it...
	if(g_ftp == NULL) {
		return;
	}

	// The file-name
	fileNameBuffer = new CString();
	fileNameBuffer->Format("%s", (LPCSTR)fileName);

	// The options for uploading the file
	options = new Communication::FTPUploadOptions;
	options->volcanoIndex = volcanoIndex;
	options->deleteFile		= deleteFile;

	// Tell the world about our mission
	message.Format("Will try to upload %s to NOVAC FTP-Server", (LPCSTR)fileName);
	ShowMessage(message);

	// Tell the uploading thread to upload this file
	g_ftp->PostThreadMessage(WM_UPLOAD_NEW_FILE, (WPARAM)fileNameBuffer, (LPARAM)options);

	return;
}

void UpdateMessage(const CString &message){
	CString *msg = new CString();

	msg->Format("%s", (LPCSTR)message);
	if(pView != NULL)
		pView->PostMessage(WM_UPDATE_MESSAGE, (WPARAM)msg, NULL);
}

void ShowMessage(const CString &message){
	CString *msg = new CString();
	CString timeTxt;
	Common commonObj;
	commonObj.GetDateTimeText(timeTxt);
	msg->Format("%s -- %s", (LPCSTR)message , (LPCSTR)timeTxt);
	if(pView != NULL)
		pView->PostMessage(WM_SHOW_MESSAGE, (WPARAM)msg, NULL);
}
void ShowMessage(const CString &message,CString connectionID){
	CString *msg = new CString();
	CString timeTxt;
	Common commonObj;
	commonObj.GetDateTimeText(timeTxt);
	msg->Format("<%s> : %s   -- %s", (LPCSTR)connectionID, (LPCSTR)message, (LPCSTR)timeTxt);
	if(pView != NULL)
		pView->PostMessage(WM_SHOW_MESSAGE, (WPARAM)msg, NULL);
}

void ShowMessage(const TCHAR message[]){
	CString msg;
	msg.Format("%s", message);
	ShowMessage(msg);
}

void Common::GetExePath(){
	TCHAR exeFullPath[MAX_PATH]; 
	GetModuleFileName(NULL, exeFullPath, MAX_PATH); 
	m_exePath     = (CString)exeFullPath;
	m_exeFileName = (CString)exeFullPath; 
	int position  = m_exePath.ReverseFind('\\'); 
	int length    = CString::StringLength(m_exePath);
	m_exePath     = m_exePath.Left(position+1);
	m_exeFileName = m_exeFileName.Right(length - position - 1);
}

/** Calculate the distance (in meters) between the two points (lat1, lon1) and
    (lat2, lon2). All latitudes and longitudes should be in degrees. */
double Common::GPSDistance(double lat1, double lon1, double lat2, double lon2){
	const double R_Earth	= 6367000; // radius of the earth
	double distance, a, c;
	lat1 = lat1*DEGREETORAD;
	lat2 = lat2*DEGREETORAD;
	lon1 = lon1*DEGREETORAD;
	lon2 = lon2*DEGREETORAD;

	double dLon = lon2 - lon1; 
	double dLat = lat2 - lat1; 

	if((dLon == 0) && (dLat == 0))
		return 0;

	a = pow((sin(dLat/2)),2) + cos(lat1) * cos(lat2) * pow((sin(dLon/2)),2) ;
	c = 2 * asin(min(1,sqrt(a))); 
	distance = R_Earth * c;

	return distance;
} 

/**count the angle from wind to north,also the plume direction compared with north
* the direction is from plume center to the source of the plume
* return degree value
*@lat1 - the latitude of beginning point or plume source, rad
*@lon1 - the longitude of beginning point orplume source,rad
*@lat2   - the latitude of ending point or plume center,rad
*@lon2   - the longitude of ending point or plume center,rad
*/
double Common::GPSBearing(double lat1, double lon1, double lat2, double lon2)
{
	lat1 = lat1*DEGREETORAD;
	lat2 = lat2*DEGREETORAD;
	lon1 = lon1*DEGREETORAD;
	lon2 = lon2*DEGREETORAD;
	double tmpAngle;
	double dLat = lat1 - lat2;
	double dLon = lon1 - lon2;

	if((dLon == 0) && (dLat == 0))
		return 0;

	tmpAngle = atan2(-sin(dLon)*cos(lat2),
                    cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(dLon));

  /*  	tmpAngle = atan2(lon1*cos(lat1)-lon2*cos(lat2), lat1-lat2); */

	if(tmpAngle < 0)
		tmpAngle = TWO_PI + tmpAngle;

	tmpAngle = RADTODEGREE*tmpAngle;
	return tmpAngle;
}

/** This function calculates the latitude and longitude for a point
		which is the distance 'dist' m and bearing 'az' degrees from 
		the point defied by 'lat1' and 'lon1' */
void Common::CalculateDestination(double lat1, double lon1, double dist, double az, double &lat2, double &lon2){
	const double R_Earth	= 6367000; // radius of the earth

	double dR = dist / R_Earth;

	// convert to radians
	lat1 = lat1 * DEGREETORAD;
	lon1 = lon1 * DEGREETORAD;
	az	 = az	  * DEGREETORAD;

	// calculate the second point
	lat2 = asin( sin(lat1)*cos(dR) + cos(lat1)*sin(dR)*cos(az) );

	lon2 = lon1 + atan2(sin(az)*sin(dR)*cos(lat1), cos(dR)-sin(lat1)*sin(lat2));

	// convert back to degrees
	lat2	= lat2 * RADTODEGREE;
	lon2	= lon2 * RADTODEGREE;
}

int IsSerialNumber(const CString &serialNumber){
	return (strlen(serialNumber) > 0);
}

// open a browser window and let the user search for a file
bool Common::BrowseForFile(TCHAR *filter, CString &fileName){
	TCHAR szFile[4096];
	sprintf(szFile, "%s", (LPCSTR)fileName);

	OPENFILENAME ofn;       // common dialog box structure
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = AfxGetInstanceHandle();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER ;

	if (GetOpenFileName(&ofn) == TRUE){
		fileName.Format(szFile);
		return true;
	}
	fileName.Format("");
	return false;
}

// open a browser window and let the user search for a file
bool Common::BrowseForFile_SaveAs(TCHAR *filter, CString &fileName){
	TCHAR szFile[4096];
	sprintf(szFile, "%s", (LPCSTR)fileName);

	OPENFILENAME ofn;       // common dialog box structure
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.hInstance = AfxGetInstanceHandle();
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER ;

	if (GetSaveFileName(&ofn) == TRUE){
		fileName.Format(szFile);
		return true;
	}
	fileName.Format("");
	return false;
}

bool Common::BrowseForDirectory(CString &folderName){
	BROWSEINFO bi;
	char tmp_FolderName[MAX_PATH ];       // temporary buffer for folder name
	char title[] = "Select Directory";

	// Initialize BROWSEINFO
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner      = NULL;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = tmp_FolderName;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_USENEWUI | BIF_VALIDATE | BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if(NULL != pidl){
		// get the name of the folder
		TCHAR path[MAX_PATH];
		if ( SHGetPathFromIDList ( pidl, path ) )
		{
			folderName.Format("%s", path);
		}

		// free memory used
		IMalloc * imalloc = 0;
		if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
		{
			imalloc->Free ( pidl );
			imalloc->Release ( );
		}
		return true;
	}else{
		/* Error */
		return false;
	}
}

/* pretty prints the current date into the string 'txt' */
void Common::GetDateText(CString &txt)
{
	struct tm *tim;
	time_t t;

	time(&t);
	tim=localtime(&t);
	txt.Format("%04d.%02d.%02d",tim->tm_year+1900,tim->tm_mon+1,tim->tm_mday);
}
int Common::GetHour()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim=localtime(&t);
	return tim->tm_hour;
}
int Common::GetMinute()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim=localtime(&t);
	return tim->tm_min;
}
int Common::GetSecond()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim=localtime(&t);
	return tim->tm_sec;
}

/** Converts a time, given in seconds since midnight to hour, minutes and seconds */
void Common::ConvertToHMS(const int time, int &hours, int &minutes, int &seconds){
	hours			= (int)(time / 3600);
	minutes		= (time - hours * 3600) / 60;
	seconds		= time % 60;	
}

/* pretty prints the current time into the string 'txt' */
void Common::GetTimeText(CString &txt)
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim=localtime(&t);
	txt.Format("%02d:%02d:%02d",tim->tm_hour,tim->tm_min,tim->tm_sec);
}
/* pretty prints the current time into the string 'txt' */
void Common::GetTimeText(CString &txt,char* seperator)
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim=localtime(&t);
	txt.Format("%02d%s%02d%s%02d",tim->tm_hour,seperator,tim->tm_min,seperator,tim->tm_sec);
}
/* pretty prints the current date and time into the string 'txt' */
void Common::GetDateTimeText(CString &txt)
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim = localtime(&t);
	txt.Format("%04d.%02d.%02d  %02d:%02d:%02d",tim->tm_year+1900,tim->tm_mon+1,tim->tm_mday,tim->tm_hour,tim->tm_min,tim->tm_sec);
}

/** Returns the current year */
int Common::GetYear()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim = localtime(&t);
	return (tim->tm_year + 1900);
}

/** Returns the current month */
int Common::GetMonth()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim = localtime(&t);
	return (tim->tm_mon + 1);
}

/** Returns the current day of the month */
int Common::GetDay()
{
	struct tm *tim;
	time_t t;
	time(&t);
	tim = localtime(&t);
	return (tim->tm_mday);
}

/** Converts the given time to local time using the information in the CGPSData.
		The date is stored as in the CSpectrumInfo-class with date[0] as 4-digit year,
		date[1] is month (1-12) and date[2] is day of month (1-31).
		@param date - the date in GMT
		@param hr - the hour in GMT
		@param gps - the GPS-information with latitude and longitude for the site
			where the local time is wanted
		@return - SUCCES if all is ok. Upon successful return, the parameters
			date and hr will be filled with the local time and date.
		NB!! daylight-saving time is not taken into account in these computations
		NB!! This calculation is only based on the distance to longitude=0. Thefore
			the resulting hour can have an error of up to +- 3 hours from the real local-time.
		*/
RETURN_CODE Common::ConvertToLocalTime(unsigned short date[3], int &hr, CGPSData &gps){
	// Direction is -1 if the local time is after the GMT-time, otherwise positive.
	int		 direction = (gps.Longitude() > 0) ? 1 : -1;

	// The absolute number of degrees from Greenwitch
	double degreesToGreenwitch = fabs(gps.Longitude());
	
	// The number of hours that differ between the local time and GMT
	int hoursToGreenwitch			= (int)round((12.0 / 180.0) * degreesToGreenwitch);

	// Change the hour
	hr += direction * hoursToGreenwitch;

	// If necessary, change the date.
	if(hr < 0){
		RETURN_CODE ret = DecreaseDate(date, 1 + (-hr/24));
		hr = 24 - (-hr) % 24;
		return ret;
	}else if(hr >= 24){
		RETURN_CODE ret = IncreaseDate(date, hr/24);
		hr %= 24;
		return ret;
	}

	// all done, return!
	return SUCCESS;
}

/** Decreases the given date by the given number of days.
		If nDays is negative, the date will be increased instead. */
RETURN_CODE Common::DecreaseDate(unsigned short date[3], int nDays){
	// Check for illegal dates
	if(date[1] < 1 || date[1] > 12)
		return FAIL;
	if(date[2] < 1  || date[2] > DaysInMonth(date[0], date[1]))
		return FAIL;

	// If we should not change the date, return without doing anything
	if(nDays == 0)
		return SUCCESS;

	// Check if we instead should increase the date
	if(nDays < 0)
		IncreaseDate(date, -nDays);

	unsigned short *day		= &date[2];
	unsigned short *month = &date[1];
	unsigned short *year	= &date[0];

	// reduce the day of the month
	*day -= nDays;

	// Check the day of the month
	while(*day < 1){ // <-- if we've passed to the month before
		--*month; // go the month before

		while(*month < 1){ // <-- if we've passed to the year before
			*year -= 1;		// go to the year before
			*month += 12;
		}

		*day += DaysInMonth(*year, *month);
	}
	// Check the month 
	while(*month < 1){
		*year -= 1; // go to the year before
		*month += 12;
	}

	return SUCCESS;
}

/** Increases the given date by the given number of days.
		If nDays is negative, the date will be decreased instead. */
RETURN_CODE Common::IncreaseDate(unsigned short date[3], int nDays){
	// Check for illegal dates
	if(date[1] < 1 || date[1] > 12)
		return FAIL;
	if(date[2] < 1  || date[2] > DaysInMonth(date[0], date[1]))
		return FAIL;

	// If we should not change the date, return without doing anything
	if(nDays == 0)
		return SUCCESS;

	// Check if we instead should decrease the date
	if(nDays < 0)
		DecreaseDate(date, -nDays);

	unsigned short *day		= &date[2];
	unsigned short *month = &date[1];
	unsigned short *year	= &date[0];

	// increase the day of the month
	*day += nDays;

	// Check the day of the month
	while(*day > DaysInMonth(*year, *month)){ // <-- if we've passed to the next month
		*day -= DaysInMonth(*year, *month);
		++*month; // go the next month

		while(*month > 12){ // <-- if we've passed to the next year
			*year += 1;		// go to the nex year
			*month -= 12;
		}
	}
	// Check the month 
	while(*month > 12){ // <-- if we've passed to the next year
		*year += 1;		// go to the nex year
		*month -= 12;
	}

	return SUCCESS;
}

/** Takes a given year and month and returns the number of days in that month. */
int	Common::DaysInMonth(int year, int month){
	static int nDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	// detect non-existing months.
	if(month < 1 || month > 12)
		return 0;

	// If the month is not february, then it's easy!!!
	if(month != 2)
		return nDays[month - 1];

	// If february, then check for leap-years
	if(year % 4 != 0)
		return 28; // not a leap-year

	if(year % 400 == 0) // every year dividable by 400 is a leap-year
		return 29;

	if(year % 100 == 0) // years diviable by 4 and by 100 are not leap-years
		return 28;
	else
		return 29;		// years dividable by 4 and not by 100 are leap-years
}

/** Takes a given date and calculates the day of the year. */
int	Common::DayNr(const unsigned short day[3]){
	CDateTime d;
	d.year	= day[2];
	d.month	= (unsigned char)day[1];
	d.day		= (unsigned char)day[0];
	return DayNr(d);
}

/** Takes a given date and calculates the day of the year. */
int	Common::DayNr(const CDateTime &day){
	// Check errors in input
	if(day.month <= 0 || day.month > 12 || day.day < 1 || day.day > DaysInMonth(day.year, day.month))
		return 0;

	int dayNr = day.day; // the daynumber

	int m = day.month;
	while(m > 1){
		dayNr += DaysInMonth(day.year, m - 1);
		--m;
	}

	return dayNr;
}

/** Returns the Julian Day. */
double Common::JulianDay(const CDateTime &utcTime){
	int N, J, b;
	double Hd,H, JD;

	if(utcTime.year < 1901 || utcTime.year > 2100 || utcTime.month < 1 || utcTime.month > 12 || utcTime.day < 1 || utcTime.day > 31)
		return 0.0;
	if(utcTime.hour < 0 || utcTime.hour > 23 || utcTime.minute < 0 || utcTime.minute > 59 || utcTime.second < 0 || utcTime.second > 59)
		return 0.0;

	N = 4713 + utcTime.year - 1;
	J = N * 365 + N/4 - 10 - 3;

	if(N % 4 == 1 || N % 4 == 2 || N % 4 == 3)
		++J;

	switch(utcTime.month)
	{	
	case(1):  b = utcTime.day - 1;		break;			
	case(2):  b = utcTime.day + 30;	break;
	case(3):  b = utcTime.day + 58;	break;
	case(4):  b = utcTime.day + 89;	break;
	case(5):  b = utcTime.day + 119;	break;
	case(6):  b = utcTime.day + 150;	break;
	case(7):  b = utcTime.day + 180;	break;
	case(8):  b = utcTime.day + 211;	break;
	case(9):  b = utcTime.day + 242;	break;
	case(10): b = utcTime.day + 272;	break;
	case(11): b = utcTime.day + 303;	break;
	case(12): b = utcTime.day + 333;	break;	
	default: return 0.0; // should never be the case
	}
	if(utcTime.year % 4 == 0)
		++b;

	H		= (double)(J + b);

	Hd	=	(utcTime.hour-12.0)/24.0 + utcTime.minute/(60.0*24.0) + utcTime.second/(3600.0*24.0);						/*CONVERSION HORAR TO DECIMAL SYSTEM*/

	JD	= J + Hd + b;
	return JD;
}

/** Retrieves the solar zenith angle (SZA) and the solar azimuth angle (SAZ)
		for the site specified by (lat, lon) and for the time given in gmtTime. 
		Note that the returned angles are in degrees and that the specified
		time _must_ be GMT-time. */
RETURN_CODE Common::GetSunPosition(const CDateTime &gmtTime, double lat, double lon, double &SZA, double &SAZ){
	SZA = SAZ = 0; // reset the numbers

	// Get the julian day
	double D = JulianDay(gmtTime) - 2451545.0;

	// Get the Equatorial coordinates...
	double	RA; //	the right ascension (deg)
	double	dec; // the declination	(deg)
	double	EqT;	// the equation of time (hours)
	EquatorialCoordinates(D, RA, dec, EqT);

	// Get the hour angle
	double fractionalHour = (double)gmtTime.hour + gmtTime.minute/60.0 + gmtTime.second/3600.0;
	double H = GetHourAngle(fractionalHour, lon, EqT);

	// Get the horizontal coordinates
	double	elev, sAzim; // The elevation and azimuth (towards south);
	HorizontalCoordinates(lat, H, dec, elev, sAzim);

	// Convert the elevation into sza
	SZA		= 90.0 - elev;

	// Convert the azimuth to a value counted from the north and 
	SAZ		= fmod(180.0 + sAzim, 360.0);

	return SUCCESS;
}

const CString &Common::GetString(const UINT uID){
	static int index = 0;

	index += 1;
	index %= 3;

	int ret = m_string[index].LoadString(uID);
	return m_string[index];
}

CString &Common::SimplifyString(const CString &in){
	static CString str;

	// Clean the string for non-printable characters
	CleanString(in, str);

	// Make a local copy of the string
	unsigned long L = (unsigned long)strlen(str);
	char *buffer		= new char[L + 2];
	sprintf(buffer, "%s", (LPCSTR)str);

	// Check all characters in the string
	for(unsigned long i = 0; i < L; ++i){
		// 1. Replace spaces with underscores
		if(buffer[i] == ' '){
			buffer[i] = '_';
			continue;
		}

		// 2. Convert the character to lower-case
		buffer[i] = tolower(buffer[i]);

		// 3. Remove paranthesis...
		if(buffer[i] == '(' || buffer[i] == '[' || buffer[i] == '{' || buffer[i] == ')' || buffer[i] == ']' || buffer[i] == '}'){
			for(unsigned long j = i; j < L - 1; ++j){
				buffer[j] = buffer[j + 1];
			}
			i = i - 1;
			continue;
		}

		// 4. Check if there's any accent on the character
		if((unsigned char)buffer[i] <= 127)
			continue;

		char c = buffer[i];

		if(c == 'á' || c == 'à' || c == 'â' || c == 'ä' || c == 'å')
			buffer[i] = 'a';
		else if(c == 'ç' || c == 'c')
			buffer[i] = 'c';
		else if(c == 'é' || c == 'è' || c == 'ê' || c == 'ë')
			buffer[i] = 'e';
		else if(c == 'í' || c == 'ì' || c == 'î' || c == 'ï')
			buffer[i] = 'i';
		else if(c == 'ó' || c == 'ò' || c == 'ô' || c == 'ö')
			buffer[i] = 'o';
		else if(c == 'ú' || c == 'ù' || c == 'ü' || c == 'û')
			buffer[i] = 'u';
		else if(c == 'ñ')
			buffer[i] = 'n';
	}

	// copy the buffer to a CString
	str.Format("%s", buffer);

	delete [] buffer;
	return str;
}

void Common::CleanString(const CString &in, CString &out){
	char *buffer = new char[strlen(in) + 2];
	sprintf(buffer, "%s", (LPCSTR)in); // make a local copy of the input string

	CleanString(buffer, out);
	delete [] buffer; // clean up after us
}

void Common::CleanString(const char *in, CString &out){
	out.Format("");
	for(unsigned int it = 0; it < strlen(in); ++it){
		if((unsigned char)in[it] >= 32)
			out.AppendFormat("%c", in[it]);
	}
}

/** Sorts a list of strings in either ascending or descending order */
void Common::Sort(CList <CString, CString&> &strings, bool files, bool ascending){
	unsigned long nStrings = (unsigned long)strings.GetCount(); // number of elements
	unsigned long it = 0; // <-- iterator

	if(nStrings <= 1){
		return; // <-- We're actually already done
	}else{
		CList <CString, CString&> left;
		CList <CString, CString&> right;

		// Make two copies of the list, one of the first half and one of the second half
		POSITION pos = strings.GetHeadPosition();
		while(it < nStrings / 2){
			left.AddTail(strings.GetNext(pos));
			++it;
		}
		while(pos != NULL){
			right.AddTail(strings.GetNext(pos));
		}

		// Sort each of the two halves
		Sort(left, files, ascending);
		Sort(right, files, ascending);

		// Merge the two...
		MergeLists(left, right, strings, files, ascending);
	}
}

/** Merges the two lists 'list1' and 'list2' in a sorted way and stores
		the result in the output-list 'result' */
void Common::MergeLists(const CList <CString, CString&> &list1, const CList <CString, CString&> &list2, CList <CString, CString&> &result, bool files, bool ascending){
	CString	name1, name2, fullName1, fullName2;
	int comparison;

	POSITION pos_1 = list1.GetHeadPosition();
	POSITION pos_2 = list2.GetHeadPosition();

	// Clear the output-list
	result.RemoveAll();

	// 1. As long as there are elements in both lists, do this
	while(pos_1 != NULL && pos_2 != NULL){
			// Get the file-names of the first and the second 
			fullName1.Format(list1.GetAt(pos_1));	// position k
			fullName2.Format(list2.GetAt(pos_2));	// position k+1

			if(files){
				// Extract the file-names only
				name1.Format(fullName1); 
				name2.Format(fullName2);
				Common::GetFileName(name1);
				Common::GetFileName(name2);

				// Compare the two names
				comparison = name1.Compare(name2);
			}else{
				// Compare the two names
				comparison = fullName1.Compare(fullName2);
			}

			if(comparison == 0){
				// if equal
				result.AddTail(fullName1);
				list1.GetNext(pos_1);
				continue;
			}else if(comparison < 0){
				// fullName1 < fullName2
				if(ascending){
					result.AddTail(fullName1);
					list1.GetNext(pos_1);
					continue;
				}else{
					result.AddTail(fullName2);
					list2.GetNext(pos_2);
					continue;
				}
			}else{
				// fullName1 > fullName2
				if(ascending){
					result.AddTail(fullName2);
					list2.GetNext(pos_2);
					continue;
				}else{
					result.AddTail(fullName1);
					list1.GetNext(pos_1);
					continue;
				}
			}

	}

	// 2. If we're out of elements in list 2 but not in list 1, do this
	while(pos_1 != NULL){
		fullName1.Format(list1.GetNext(pos_1));
		result.AddTail(fullName1);
	}

	// 3. If we're out of elements in list 1 but not in list 2, do this
	while(pos_2 != NULL){
		fullName2.Format(list2.GetNext(pos_2));
		result.AddTail(fullName2);
	}

}

// get the gas factor for the supplied specie
double Common::GetGasFactor(const CString &specie)
{
	if(0 == _tcsncicmp(specie, "SO2", 3)){
		return GASFACTOR_SO2;
	}
	if(0 == _tcsncicmp(specie, "O3", 2)){
		return GASFACTOR_O3;
	}
	if(0 == _tcsncicmp(specie, "NO2", 3)){
		return GASFACTOR_NO2;
	}

	return -1;
}

double Common::CalculateFlux(const double *scanAngle, const double *scanAngle2, const double *column, double offset, int nDataPoints, const CWindField &wind, double compass, double gasFactor, INSTRUMENT_TYPE type, double coneAngle, double tilt){
	if(type == INSTR_HEIDELBERG){
		return CalculateFlux_HeidelbergFormula(scanAngle, scanAngle2, column, offset, nDataPoints, wind, compass, gasFactor);
	}else if(type == INSTR_GOTHENBURG){
		if(fabs(coneAngle - 90.0) < 1.0)
			return CalculateFlux_FlatFormula(scanAngle, column, offset, nDataPoints, wind, compass, gasFactor);
		else
			return CalculateFlux_ConeFormula(scanAngle, column, offset, nDataPoints, wind, compass, gasFactor, coneAngle, tilt);
	}else{
		return 0.0; // unsupported instrument-type
	}
}

/** Calculates the flux using the supplied data using the old algorithm */
double Common::CalculateFlux_FlatFormula(const double *scanAngle, const double *column, double offset, int nDataPoints, const CWindField &wind, double compass, double gasFactor){
	double avgVCD, VCD1, VCD2, TAN1, TAN2, distance;
	double flux = 0;
	double partialFlux;

	// the wind factor
	double windFactor = fabs(cos(DEGREETORAD*(wind.GetWindDirection() - compass)));

	// now calculate the flux
	for(int i = 0; i < nDataPoints - 1; ++i){
		if(fabs(fabs(scanAngle[i]) - 90.0) < 0.5)
			continue; // the distance-calculation has a singularity at +-90 degrees so just skip those points!
		if(fabs(fabs(scanAngle[i+1]) - 90.0) < 0.5)
			continue; // the distance-calculation has a singularity at +-90 degrees so just skip those points!

		// The vertical columns
		VCD1			= (column[i] - offset)	 * cos(DEGREETORAD * scanAngle[i]);
		VCD2			= (column[i+1] - offset) * cos(DEGREETORAD * scanAngle[i+1]);

		// calculating the horisontal distance
		TAN1			= tan(DEGREETORAD * scanAngle[i]);
		TAN2			= tan(DEGREETORAD * scanAngle[i+1]);
		distance		= wind.GetPlumeHeight() * fabs(TAN2 - TAN1);

		// The average vertical column
		avgVCD			= (1E-6) * gasFactor * ( VCD1 + VCD2) * 0.5;

		// The flux...
		partialFlux		= distance * avgVCD * wind.GetWindSpeed() * windFactor;
		flux			+= partialFlux;
	}

	return fabs(flux);
}

/** Calculates the flux using the supplied data using the cone-scanner algorithm */
double Common::CalculateFlux_ConeFormula(const double *scanAngle, const double *column, double offset, int nDataPoints, const CWindField &wind, double compass, double gasFactor, double coneAngle, double tilt){
	double flux = 0;
	double partialFlux, columnAmplification;

#ifdef _DEBUG
	FILE *f = fopen("Debug_fluxLog.txt", "w");
	if(f != NULL){
		fprintf(f, "Calculating flux for cone-scan with %d spectra\n", nDataPoints);
		fprintf(f, "windDirection=%lf\ncompass=%lf\noffset=%lf\n\n", 
			wind.GetWindDirection(), compass, offset);
		fprintf(f, "ScanAngle[deg]\tColumn[ppmm]\tAvgVCD[ppmm]\tHorizontalDistance\tConeCompass\tWindFactor\tPartialFlux\n");
		fclose(f);
	}
#endif

	// convert the angles to radians
	tilt					*= DEGREETORAD;
	coneAngle			*= DEGREETORAD;

	// local-data buffer to store the intermediate calculations
	std::vector<double> alpha(nDataPoints);
	std::vector<double> scd(nDataPoints);
	std::vector<double> columnCorrection(nDataPoints);
	std::vector<double> x(nDataPoints);
	std::vector<double> y(nDataPoints);

	// Temporary variables, to do less computations
	double	tan_coneAngle	= tan(coneAngle);
	double	sin_tilt			= sin(tilt);
	double	cos_tilt			= cos(tilt);
		
	// First prepare the buffers before we calculate anything
	for(int i = 0; i < nDataPoints - 1; ++i){
		// The slant columns
		scd[i]	= column[i]-offset;

		// The scan-angles, in radians
		alpha[i]	= scanAngle[i]	* DEGREETORAD;

		// cosine and sine of the scan-angle
		double cos_alpha	= cos(alpha[i]);
		double sin_alpha	= sin(alpha[i]);

		// Calculate the AMF in order to get vertical columns
		double x_term				= pow(cos_tilt/tan_coneAngle - cos_alpha*sin_tilt, 2);
		double y_term				= pow(sin_alpha, 2);
		double divisor			= pow(cos_alpha*cos_tilt + sin_tilt/tan_coneAngle, 2);
		columnAmplification	= sqrt( (x_term + y_term)/divisor + 1 );
		columnCorrection[i]	= 1 / columnAmplification;

		// Calculate the projections of the intersection points in the ground-plane
		double commonDenominator = cos_alpha*cos_tilt + sin_tilt/tan_coneAngle;
		x[i]		= (cos_tilt/tan_coneAngle - cos_alpha*sin_tilt)	/ commonDenominator;
		y[i]		= (sin_alpha)																		/ commonDenominator;
	}

	// Now make the actual flux-calculation
	for(int i = 0; i < nDataPoints - 2; ++i){
		if(fabs(fabs(alpha[i]) - HALF_PI) < 1e-2 || fabs(fabs(alpha[i+1]) - HALF_PI) < 1e-2)
			continue;// This algorithm does not work very well for scanangles around +-90 degrees

		// The average vertical column
		double avgVCD	= (1e-6) * gasFactor * (scd[i]*columnCorrection[i] + scd[i+1]*columnCorrection[i+1]) * 0.5;

		// The horizontal distance
		double S = wind.GetPlumeHeight() * sqrt( pow(x[i+1] - x[i], 2) + pow(y[i+1] - y[i], 2) );

		// The local compass-direction [radians] due to the curvature of the cone
		double coneCompass	= atan2(y[i+1] - y[i], x[i+1] - x[i]);

		// The wind-factor 
		double windFactor	= fabs(sin(DEGREETORAD * (wind.GetWindDirection() - compass) - coneCompass));
		
		// The partial flux
		partialFlux	= avgVCD * S * wind.GetWindSpeed() * windFactor;

		// The total flux
		flux	+= partialFlux;

#ifdef _DEBUG
	FILE *f = fopen("Debug_fluxLog.txt", "a+");
	if(f != NULL){
		fprintf(f, "%lf\t%lf\t%lf\t", scanAngle[i], scd[i], avgVCD);
		fprintf(f, "%lf\t%lf\t%lf\t", S, coneCompass * RADTODEGREE, windFactor);
		fprintf(f, "%lf\n",		partialFlux);
		fclose(f);
	}
#endif

	}

	return fabs(flux);
}

/** Calculates the flux using an instrument of type Heidelberg = general algorithm for calculating the flux using cartesian coordinates and angles defined as 
		elev	= scanAngle 1 (elevation, for elevation=0° azimuth=90° pointing to North)
		azim	= scanAngle 2 (azimuth from North, clockwise)*/
double Common::CalculateFlux_HeidelbergFormula(const double *scanAngle1, const double *scanAngle2, const double *column, double offset, int nDataPoints, const CWindField &wind, double compass, double gasFactor){
	double flux = 0;
	double partialFlux;

/*TODO: writing the Log, what needs to be included? */
#ifdef _DEBUG
	FILE *f = fopen("Debug_fluxLog_HD.txt", "w");
	if(f != NULL){
		fprintf(f, "Calculating flux for Heidelberg instrument with %d spectra\n", nDataPoints);
		fprintf(f, "windDirection=%lf\ncompass=%lf\noffset=%lf\n\n", 
			wind.GetWindDirection(), compass, offset);
		fprintf(f, "ScanAngle1_Elev[deg]\tScanAngle2_Azim[deg]\tColumn[ppmm]\tAvgVCD[ppmm]\tHorizontalDistance\tConeCompass\tWindFactor\tPartialFlux\n");
		fclose(f);
	}
#endif

	// local-data buffer to store the intermediate calculations
	// double	*alpha							= new double[nDataPoints];
	
	std::vector<double> elev(nDataPoints);
	std::vector<double> azim(nDataPoints);
	std::vector<double> scd(nDataPoints);
	std::vector<double> columnCorrection(nDataPoints);
	std::vector<double> x(nDataPoints);
	std::vector<double> y(nDataPoints);

	// First prepare the buffers before we calculate anything
	for(int i = 0; i < nDataPoints - 1; ++i){
		// The slant columns
		scd[i]	= column[i+1]-offset;

		// The scan-angles, in radians
		elev[i]		= scanAngle1[i] * DEGREETORAD;
		azim[i]		= scanAngle2[i] * DEGREETORAD;

		double tan_elev	= tan(elev[i]);
		double cos_elev	= cos(elev[i]);
		double sin_azim		= sin(azim[i]);
		double cos_azim		= cos(azim[i]);
		
		// Calculate the AMF in order to get vertical columns
		columnCorrection[i]	= cos_elev;
		

		// Calculate the projections of the intersection points in the ground-plane
		double x_term	= tan_elev * cos_azim;
		double y_term	= tan_elev * sin_azim;

		x[i]		= x_term;
		y[i]		= y_term;
	}

	// Now make the actual flux-calculation
/*TODO: flux calculations for Heidelberg instrument differ from Gothenborg instrument because local and global coordinate system do not differ!!!! Define another loop for Heidelberg?*/
	for(int i = 0; i < nDataPoints - 2; ++i){
		if(fabs(fabs(elev[i]) - HALF_PI) < 1e-2 || fabs(fabs(elev[i+1]) - HALF_PI) < 1e-2)
			continue;// This algorithm does not work very well for scanangles around +-90 degrees
		
		// The average vertical column
		double avgVCD	= (1e-6) * gasFactor * (scd[i]*columnCorrection[i] + scd[i+1]*columnCorrection[i+1]) * 0.5;

		// The horizontal distance
		double S = wind.GetPlumeHeight() * sqrt( pow(x[i+1] - x[i], 2) + pow(y[i+1] - y[i], 2) );
		
		// The local compass-direction [radians] due to the curvature of the cone
		double DirectionCompass = atan2(y[i+1] - y[i], x[i+1] - x[i]);
		
		// The wind-factor HD: compass=azim[i]
		double windFactor	= fabs(sin(DEGREETORAD * wind.GetWindDirection() - DirectionCompass));

		// The partial flux
		partialFlux	= avgVCD * S * wind.GetWindSpeed() * windFactor;

		// The total flux
		flux	+= partialFlux;

#ifdef _DEBUG
	FILE *f = fopen("Debug_fluxLog_HD.txt", "a+");
	if(f != NULL){
		fprintf(f, "%lf\t%lf\t%lf\t%lf\t", scanAngle1[i], scanAngle2[i], scd[i], avgVCD);
		fprintf(f, "%lf\t%lf\t%lf\t", S, DirectionCompass * RADTODEGREE, windFactor);
		fprintf(f, "%lf\n",		partialFlux);
		fclose(f);
	}
#endif

	}

	return fabs(flux);
}

double Common::CalculateOffset(const std::vector<double>& columns, const std::vector<bool>& badEvaluation, long numPoints){
//  SpecData m[3] = {1e6, 1e6, 1e6}; 
	SpecData avg;
	Common common;
	long i;

	// calculate the offset as the average of the three lowest column values 
	//    that are not considered as 'bad' values

	std::vector<double> testColumns(numPoints);

	int numColumns = 0;
	for(i = 0; i < numPoints; ++i){
		if(badEvaluation[i])
		continue;

		testColumns[numColumns++] = columns[i];
	}

	if(numColumns <= 5){
		return 0.0;
	}

	// Find the N lowest column values
	int N = (int)(0.2 * numColumns);
	std::vector<SpecData> m(N, 1e6);
	if(FindNLowest(testColumns.data(), numColumns, m.data(), N)){
		avg = Average(m.data(), N);
//		avg = (m[0] + m[1] + m[2]) / 3;
		return avg;
	}

	// could not calculate a good offset.
	return 0;
}

/** Finds the plume in the supplied scan. Return value is true if there is a plume, otherwise false 
		@param scanAngles - the scanAngles for the measurements.
		@param columns - the slant columns for the measurements. Must be from a normal scan
		@param columnErrors - the corresponding slant column errors for the measurement.
		@param badEvaluation - the result of the quality judgement of the measured columns, 
			badEvaluation[i] = true means a bad value
		@param numPoints - the number of points in the scan. Must also be the length 
			of the vectors 'columns', 'columnErrors', and 'badEvaluation'
		@param plumeCentre - Will on successful return be filled with the scan angle 
			which hits the centre of the plume
		@param plumeWidth - will on successful return be filled with the 
			width of the plume (same unit as the scanAngles)	*/
bool Common::FindPlume(const std::vector<double>& scanAngles, const std::vector<double>& phi, const std::vector<double>& columns, const std::vector<double>& columnErrors, const std::vector<bool>& badEvaluation, long numPoints, double &plumeCentre_alpha, double &plumeCentre_phi, double &plumeEdge_low, double &plumeEdge_high){
	Common common;

	// There is a plume iff there is a region, where the column-values are considerably
	//	much higher than in the rest of the scan
	
	// Make a local copy of the values, picking out only the good ones
	std::vector<double> col(numPoints);
	std::vector<double> colE(numPoints);
	std::vector<double> angle(numPoints);
	std::vector<double> p(numPoints);

	int		nCol	= 0; // <-- the number of ok column values
	for(int k = 0; k < numPoints; ++k){
		if(!badEvaluation[k]){
			col[nCol]	= columns[k];
			colE[nCol]	= columnErrors[k];
			angle[nCol]	= scanAngles[k];
			p[nCol]		= phi[k];
			++nCol;
		}
	}
	if(nCol <= 5){ // <-- if too few ok points, then there's no plume
		return false;
	}

	// Try different divisions of the scan to see if there is a region of at least
	//	'minWidth' values where the column-value is considerably higher than the rest
	double highestDifference = -1e16;
	long minWidth = 5;
	int regionLow = 0, regionHigh = 0;
	for(int low = 0; low < nCol; ++low){
		for(int high = low+minWidth; high < nCol; ++high){
			// The width of the region has to be at least 'minWidth' values, otherwise there's no idea to search
			if(high - low < minWidth)
				continue; 

			// the average column value in the region we're testing
			double avgInRegion = Average(col.data() + low, high - low); 

			// the average column value outside of the region we're testing
			double avgOutRegion= (Average(col.data(), low) + Average(col.data() + high, nCol - high)) * 0.5;

			if(avgInRegion - avgOutRegion > highestDifference){
				highestDifference = avgInRegion - avgOutRegion;
				regionLow	= low;
				regionHigh	= high;
			}
		}
	}

	// Calculate the average column error, for the good measurement points
	double avgColError = Average(colE.data(), nCol);

	if(highestDifference > 5 * avgColError){
		// the plume centre is the average of the scan-angles in the 'plume-region'
		//	weighted with the column values
		double sumAngle_alpha = 0, sumAngle_phi = 0, sumWeight = 0;
		for(int k = regionLow; k < regionHigh; ++k){
			sumAngle_alpha	+= angle[k] * col[k];
			sumAngle_phi	+= p[k]		* col[k];
			sumWeight		+= col[k];
		}
		plumeCentre_alpha	= sumAngle_alpha / sumWeight;
		plumeCentre_phi		= sumAngle_phi   / sumWeight; // if phi == NULL then this will be non-sense

		// The edges of the plume
		plumeEdge_low  = angle[0];
		plumeEdge_high = angle[nCol-1];
		double minCol = Min(col.data(), nCol);
		double maxCol_div_e = (Max(col.data(), nCol) - minCol) * 0.3679;
		for(int k = 0; k < nCol; ++k){
			if(angle[k] > plumeCentre_alpha){
				break;
			}else if((col[k] - minCol) < maxCol_div_e){
				plumeEdge_low = angle[k];
			}
		}
		for(int k = nCol - 1; k >= 0; --k){
			if(angle[k] <= plumeCentre_alpha){
				break;
			}else if((col[k] - minCol) < maxCol_div_e){
				plumeEdge_high = angle[k];
			}
		}

		return true;
	}else {
		return false;
	}
}

/** Tries to calculate the completeness of the given scan.
		The completeness is 1.0 if the entire plume can be seen and 0.0 if the plume
			cannot be seen at all.
		Return value is true if there is a plume, otherwise false 
		@param scanAngles - the scanAngles for the measurements.
		@param columns - the slant columns for the measurements. Must be from a normal scan
		@param columnErrors - the corresponding slant column errors for the measurement.
		@param badEvaluation - the result of the quality judgement of the measured columns, 
			badEvaluation[i] = true means a bad value
		@param numPoints - the number of points in the scan. Must also be the length 
			of the vectors 'columns', 'columnErrors', and 'badEvaluation'
		@param completeness - Will on successful return be filled with the completeness of the plume */
bool Common::CalculatePlumeCompleteness(const std::vector<double>& scanAngles, const std::vector<double>& phi, const std::vector<double>& columns, const std::vector<double>& columnErrors, const std::vector<bool>& badEvaluation, double offset, long numPoints, double &completeness) {
	double plumeCentre_alpha, plumeCentre_phi;
	double plumeEdge_low, plumeEdge_high;

	int nDataPointsToAverage = 5;

	// Check if there is a plume at all...
	bool inPlume = FindPlume(scanAngles, phi, columns, columnErrors, badEvaluation, numPoints, plumeCentre_alpha, plumeCentre_phi, plumeEdge_low, plumeEdge_high);
	if(!inPlume){
		completeness = 0.0; // <-- no plume at all
		return false;
	}
	
	// Calculate the average of the 'nDataPointsToAverage' left-most values
	double avgLeft = 0.0;
	int nAverage = 0;
	for(int k = 0; k < numPoints; ++k){
		if(!badEvaluation[k]){
			avgLeft += columns[k] - offset;
			++nAverage;
			if(nAverage == nDataPointsToAverage)
				break;
		}
	}
	if(nAverage < nDataPointsToAverage){
		// not enough data-points to make an ok average, return fail
		completeness = 0.0; // <-- no plume at all
		return false;
	}
	avgLeft /= nDataPointsToAverage;

	// Calculate the average of the 'nDataPointsToAverage' right-most values
	double avgRight = 0.0;
	nAverage = 0;
	for(int k = numPoints-1; k > 0; --k){
		if(!badEvaluation[k]){
			avgRight += columns[k] - offset;
			++nAverage;
			if(nAverage == nDataPointsToAverage)
				break;
		}
	}
	if(nAverage < nDataPointsToAverage){
		// not enough data-points to make an ok average, return fail
		completeness = 0.0; // <-- no plume at all
		return false;
	}
	avgRight /= nDataPointsToAverage;

	// Find the maximum column value
	double maxColumn = 0.0;
	for(int k = 0; k < numPoints; ++k){
		if(!badEvaluation[k]){
			maxColumn = max(maxColumn, columns[k] - offset);
		}
	}

	// The completeness
	completeness = 1.0 - 0.5 * max(avgLeft, avgRight) / maxColumn;
	if(completeness > 1.0)
		completeness = 1.0;

	return true;
}

void Common::GuessSpecieName(const CString &fileName, CString &specie){
	specie.Format("");
	CString spc[] = {"SO2", "NO2", "O3", "O4", "HCHO", "RING", "H2O", "CLO", "BRO", "CHOCHO", "Glyoxal", "Formaldehyde", "HONO", "NO3"};
	int nSpecies = 12;

	int index = fileName.ReverseFind('\\');
	if(index == 0)
		return;

	CString fil;
	fil.Format("%s", (LPCSTR)fileName.Right((int)strlen(fileName) - index - 1));
	fil.MakeUpper();

	for(int i = 0; i < nSpecies; ++i){
		if(strstr(fil, spc[i])){
		specie.Format("%s", (LPCSTR)spc[i]);
		return;
		}
	}

	// nothing found
	return;
}


int Common::CheckProcessExistance(CString& exeName,int pid)
{
	int ret;
	CString processPath;
	DWORD processid[1024],needed,processcount,i;
	HMODULE hModule;
	char path[MAX_PATH] = "";

	EnumProcesses(processid, sizeof(processid), &needed);
	processcount=needed/sizeof(DWORD);
	for (i=0;i<processcount;i++)
	{
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,processid[i]);
		if (hProcess)
		{
			EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed);
			GetModuleFileNameEx(hProcess, hModule, path, sizeof(path));
			processPath.Format("%s",path);
			GetFileName(processPath);
			ret = processPath.Compare(exeName); //compare exe name and the process name
			if(ret==0) // names are same
			{
				if(pid != -1)
				{
					if(processid[i] == pid)  //compare process id
					{
						CloseHandle(hProcess);
						return processid[i];
					}
					else
						continue;
				}
				else
				{
					//MessageBox(NULL,"find txzm","notice",MB_OK);
					CloseHandle(hProcess);
					return processid[i];
				}
			}
			CloseHandle(hProcess);
		}
	}
	
	//MessageBox(NULL,"CAN NOT find txzm","notice",MB_OK);
	return -1;
}

/** Get all Process-ID's running with a given executable-name
	@param exeName - the name of the executable (e.g. "txzm.exe") 
	@param startPid - the function will search for processes with a 
	    process ID higher than 'pid' 
	@param pIDs[1024] - will on successful return be filled with
	    all pID's found (first empty item will be -1)
	@return - number of processID found , -1 if no process is found */
int Common::GetAllProcessIDs(CString& exeName, int pIDs[1024], int startPid){
	int nPIDsFound = 0;
	DWORD processid[1024];
	DWORD needed;
	HMODULE hModule;
	memset(pIDs, -1, 1024*sizeof(int)); // set all values to -1

	if (!EnumProcesses(processid, sizeof(processid), &needed)) {
		return 0;
	}
	DWORD processcount=needed/sizeof(DWORD);
	for (unsigned int i = 0; i < processcount; i++)
	{
		if (processid[i] == 0) {
			continue;
		}
		CString processPath;
		char path[MAX_PATH] = "";
		HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,false,processid[i]);
		if (hProcess)
		{
			EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed);
			GetModuleFileNameEx(hProcess, hModule, path, sizeof(path));
			processPath.Format("%s",path);
			GetFileName(processPath);
			int ret = processPath.Compare(exeName); //compare exe name and the process name
			if(ret == 0) // names are same
			{
				if((int)processid[i] >= startPid){
					pIDs[nPIDsFound++] = processid[i];
				}
			}
			CloseHandle(hProcess);
		}
	}
	
	return nPIDsFound;
}

BOOL WINAPI Common::KillProcess(IN DWORD dwProcessId)
{
	HANDLE hProcess;
	DWORD dwError;
	 
	// first try to obtain handle to the process without the use of any
	// additional privileges
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if (hProcess == NULL)
	{
		if (GetLastError() != ERROR_ACCESS_DENIED)
			return FALSE;
	 
	//OSVERSIONINFO osvi;
	 
	// determine operating system version
	//osvi.dwOSVersionInfoSize = sizeof(osvi);
	//GetVersionEx(&osvi);
	 
	// we cannot do anything else if this is not Windows NT
	//if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
	if (!IsWindowsXPOrGreater()) {
		MessageBox(NULL, "You need at least Windows XP", "Version Not Supported", MB_OK);
		return SetLastError(ERROR_ACCESS_DENIED), FALSE;
	}
	 
	// enable SE_DEBUG_NAME privilege and try again
	 
	TOKEN_PRIVILEGES Priv, PrivOld;
	DWORD cbPriv = sizeof(PrivOld);
	HANDLE hToken;
	 
	// obtain the token of the current thread
	if (!OpenThreadToken(GetCurrentThread(),
			TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
			FALSE, &hToken))
	{
	if (GetLastError() != ERROR_NO_TOKEN)
		return FALSE;
	 
	// revert to the process token
	if (!OpenProcessToken(GetCurrentProcess(),
			TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
			&hToken))
		return FALSE;
	}
	 
	_ASSERTE(ANYSIZE_ARRAY > 0);
	 
	Priv.PrivilegeCount = 1;
	Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Priv.Privileges[0].Luid);

	// try to enable the privilege
	if (!AdjustTokenPrivileges(hToken, FALSE, &Priv, sizeof(Priv), &PrivOld, &cbPriv))
	{
		dwError = GetLastError();
		CloseHandle(hToken);
		return SetLastError(dwError), FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		// the SE_DEBUG_NAME privilege is not present in the caller's
		// token
		CloseHandle(hToken);
		return SetLastError(ERROR_ACCESS_DENIED), FALSE;
	}

	// try to open process handle again
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	dwError = GetLastError();

	// restore the original state of the privilege
	AdjustTokenPrivileges(hToken, FALSE, &PrivOld, sizeof(PrivOld),
			NULL, NULL);
	CloseHandle(hToken);

	if (hProcess == NULL)
		return SetLastError(FALSE), NULL;
	}

	// terminate the process
	if (!TerminateProcess(hProcess, (UINT)-1))
	{
		dwError = GetLastError();
		CloseHandle(hProcess);
		return SetLastError(dwError), FALSE;
	}

	CloseHandle(hProcess);

	// completed successfully
	return TRUE;
}

/** Take out the exe name from a long path 
	  @param fileName path of the exe file	*/
void Common::GetFileName(CString& fileName)
{
	int position  = fileName.ReverseFind('\\'); 
	int length    = CString::StringLength(fileName);
	fileName = fileName.Right(length - position - 1);	
}

/** Take out the directory from a long path name.
    @param fileName - the complete path of the file */
void Common::GetDirectory(CString &fileName){
	int position  = fileName.ReverseFind('\\');
	if(position >= 0)
		fileName = fileName.Left(position + 1);
}

long Common::RetrieveFileSize(CString& fileName)
{
	TRY
	{
		CFile file(fileName,CFile::modeRead| CFile::shareDenyNone);
		long fileLength = (long)file.GetLength();
		file.Close();
		return fileLength;
	}
	CATCH(CFileException, ex)
	{
		return 0; // could not get file
	}
	END_CATCH;
}

void Common::WriteAFile(const CString& fileName,const CString& msg, char* mode)
{
	FILE *f = fopen(fileName, mode);
	if(f != NULL)
	{
		fprintf(f, "%s\n", (LPCSTR)msg);
		fclose(f);
	}
}

bool Common::FormatErrorCode(DWORD error, CString &string){
	/* from System Error Codes */
	switch(error){
		case ERROR_FILE_NOT_FOUND:
			string.Format("File not found"); return true;
		case ERROR_PATH_NOT_FOUND:
			string.Format("Path not found"); return true;
		case ERROR_TOO_MANY_OPEN_FILES:
			string.Format("Too many open files"); return true;
		case ERROR_ACCESS_DENIED:
			string.Format("Access denied"); return true;
		case ERROR_NOT_ENOUGH_MEMORY:
			string.Format("Not enough memory"); return true;
		case ERROR_OUTOFMEMORY:
			string.Format("Out of memory"); return true;
		case ERROR_WRITE_PROTECT:
			string.Format("The media is write protected"); return true;
		case ERROR_SEEK:
			string.Format("The drive cannot locate a specific area or track on the disk."); return true;
		case ERROR_WRITE_FAULT:
			string.Format("The system cannot write to the specified device"); return true;
		case ERROR_READ_FAULT:
			string.Format("The system cannot read from the specified device"); return true;
		case ERROR_HANDLE_DISK_FULL:
		case ERROR_DISK_FULL:
			string.Format("The disk is full"); return true;
		case ERROR_CANNOT_MAKE:
			string.Format("The directory or file cannot be created"); return true;
		case ERROR_BUFFER_OVERFLOW:
			string.Format("The file name is too long"); return true;
		case ERROR_INVALID_NAME: 
			string.Format("The filename, directory name, or volume label syntax is incorrect"); return true;
		case ERROR_DIRECTORY:
			string.Format("The directory name is invalid"); return true;
		case ERROR_DISK_TOO_FRAGMENTED:
			string.Format("The volume is too fragmented to complete this operation"); return true;
		case ERROR_ARITHMETIC_OVERFLOW:
			string.Format("Arithmetic result exceeded 32 bits"); return true;
		case ERROR_ALREADY_EXISTS:
			string.Format("The file already exists"); return true;
		case ERROR_SHARING_VIOLATION:
			string.Format("Cannot access the file, it is used by another process"); return true;

	}

	return false;
}

int Common::GetInterlaceSteps(int channel, int &interlaceSteps){
	// if the spectrum is a mix of several spectra
	if(channel >= 129){
		interlaceSteps = channel - 127;
		return -1;
	}

	// special case, channel = 128 is same as channel = 0
	if(channel == 128)
		channel = 0;

	// If the spectrum is a single spectrum
	interlaceSteps = (channel / 16) + 1; // 16->31 means interlace=2, 32->47 means interlace=3 etc.
	return (channel % 16); // the remainder tells the channel number
}

/** Find the volcano-index that the spectrometer with the supplied 
		serial-number monitors. If none is found then -1 is returned */
int	Common::GetMonitoredVolcano(const CString &serialNumber){
	
	// find the name of the volcano that is monitored
	CString volcanoName;
	for(unsigned int k = 0; k < g_settings.scannerNum; ++k){
		for(unsigned int j = 0; j < g_settings.scanner[k].specNum; ++j){
			if(Equals(serialNumber, g_settings.scanner[k].spec[j].serialNumber)){
				volcanoName.Format(g_settings.scanner[k].volcano);
				break;
			}
		}
	}

	// now find the index of this volcano
	if(strlen(volcanoName) == 0)
		return -1; // <-- nothing found

	for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
		if (Equals(volcanoName, g_volcanoes.m_name[k])) {
			return k;
		}
	}

	return -1; // could not find the volcano-name
}

/*EQUATORIAL COORDINATES:RIGHT ASCENSION AND DECLINATION*/
void Common::EquatorialCoordinates(double D, double &RA, double &dec,double &EQT)
{
	double g_deg,q_deg,L_deg;				/*ANGLES IN DEGREES*/
	double g_rad,q_rad,L_rad;				/*ANGLES IN	RADIANS*/
	double R;												/*DISTANCE SUN-EARTH IN A.U*/
	double obliq_deg,obliq_rad;			/*OBLIQUITY OF THE ECLIPTIC*/
	double RA_rad,dec_rad;					/*EQUATORIAL COORDINATES IN RADIANS*/

	g_deg	=	fmod(357.529 + 0.98560028 * D, 360.0);
	g_rad	=	g_deg	* DEGREETORAD;
	q_deg	=	fmod(280.459 + 0.98564736*D, 360.0);
	q_rad = q_deg * DEGREETORAD;

	L_deg = q_deg + 1.915*sin(g_rad) + 0.02*sin(2*g_rad);
	L_rad = L_deg * DEGREETORAD;

	// The distance between the sun and the earth (in Astronomical Units)
	R			= 1.00014 - 0.01671*cos(g_rad) - 0.00014*cos(2*g_rad);

	// The obliquity of the earth's orbit:
	obliq_deg = 23.439 - 0.00000036 * D;
	obliq_rad	= obliq_deg * DEGREETORAD;

	// The right ascension (RA)
	RA_rad	=	atan(cos(obliq_rad) * sin(L_rad) / cos(L_rad));
	if(RA_rad < 0)
		RA_rad	= TWO_PI + RA_rad;

	if(fabs(RA_rad-L_rad) > 1.570796)
		RA_rad = M_PI + RA_rad;

	dec_rad	=	asin(sin(obliq_rad) * sin(L_rad));
	RA			= fmod(RA_rad * RADTODEGREE, 360.0);		// The right ascension

	// The declination
	dec			=	dec_rad * RADTODEGREE;

	// The Equation of Time
	EQT			=	q_deg/15.0 - RA/15.0;
}

void Common::HorizontalCoordinates(double lat, double H, double dec, double &elev, double &azim){
	double H_rad		=H	 * DEGREETORAD;
	double lat_rad	=lat * DEGREETORAD;
	double dec_rad	=dec * DEGREETORAD;

	// The elevation angle
	double elev_rad		=	asin(cos(H_rad)*cos(dec_rad)*cos(lat_rad)+sin(dec_rad)*sin(lat_rad));

	// The cosine of the azimuth - angle
	double cazim_rad		=	(cos(H_rad)*cos(dec_rad)*sin(lat_rad)-sin(dec_rad)*cos(lat_rad))/cos(elev_rad);

	// The sine of the azimuth - angle
	double sazim_rad		=	(sin(H_rad)*cos(dec_rad))/cos(elev_rad);

	double azim_rad = 0.0;
	if(cazim_rad > 0 && sazim_rad > 0)
		azim_rad	=	asin(sazim_rad);						// azim is in the range 0 - 90 degrees
	else if(cazim_rad < 0 && sazim_rad > 0)
		azim_rad	= M_PI - asin(sazim_rad);			// azim is in the range 90 - 180 degrees
	else if(cazim_rad < 0 && sazim_rad < 0)
		azim_rad	= M_PI - asin(sazim_rad);		// azim is in the range 180 - 270 degrees
	else if(cazim_rad > 0 && sazim_rad < 0)
		azim_rad	= TWO_PI + asin(sazim_rad);		// azim is in the range 270 - 360 degrees
	
	elev = elev_rad * RADTODEGREE;
	azim = azim_rad * RADTODEGREE;

	//printf("\n\nHORIZONTAL COORDINATES:\nAZIMUTH (from South to West)=%f\nELEVATION=%f\n\n",*pazim,*pelev);
}

/** Returns the hour angle given the longitude and equation of time. */
double Common::GetHourAngle(double hr, double lon, double EqT){
	double H = 15.0 * (hr + lon/15 + EqT - 12);
	//    printf("HOUR ANGLE (from noon,increasing with time): %f\n",H);
	return(H);
}

bool Common::FillBuffer(char* srcBuf, char* destBuf,long destStart,long moveLen)
{
	long i;
	// destination buffer is full
	if(destStart + moveLen >= MAXBUFFER)
		return false;
	for(i = 0; i<moveLen;i++)
	{
		destBuf[destStart + i] = srcBuf[i];	
	}
	return true;

}
