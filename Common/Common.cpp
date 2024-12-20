#include "StdAfx.h"
#include "Common.h"

// include the global settings
#include "../Configuration/Configuration.h"
#include "../VolcanoInfo.h"
#include "../Communication/FTPServerContacter.h"
#include "../Meteorology/WindField.h"

#include <SpectralEvaluation/Flux/Flux.h>

#include "PSAPI.H"
#include <tlhelp32.h>
#include <VersionHelpers.h>
#pragma comment( lib, "PSAPI.LIB" )

extern CFormView* pView;
extern CConfigurationSetting g_settings;    // <-- the settings for the scanners
extern CVolcanoInfo g_volcanoes;            // <-- the list of volcanoes
extern CWinThread* g_ftp;                   // <-- The Ftp-uploading thread.

long GetSleepTime(struct timeStruct& startTime, struct timeStruct& stopTime)
{
    CTime currentTime;
    int hr, min, sec;
    long waitTime, currentT, startT, stopT;
    waitTime = -1;
    currentTime = CTime::GetCurrentTime();
    hr = currentTime.GetHour();
    min = currentTime.GetMinute();
    sec = currentTime.GetSecond();
    currentT = hr * 3600 + min * 60 + sec;
    startT = startTime.hour * 3600 + startTime.minute * 60 + startTime.second;
    stopT = stopTime.hour * 3600 + stopTime.minute * 60 + stopTime.second;
    if (startT > stopT)
    {
        if ((currentT >= startT) && (currentT <= 86399))
            waitTime = (86400 - currentT + stopT) * 1000;
        else if ((currentT <= stopT) && (currentT >= 0))
            waitTime = (stopT - currentT) * 1000;
    }
    else if (startT < stopT)
    {
        if ((currentT >= startT) && (currentT <= stopT))
            waitTime = (stopT - currentT) * 1000;
    }
    else if (startT == stopT)
    {
        if (currentT >= startT)
            waitTime = (86400 - currentT + startT) * 1000;
    }
    return waitTime;
}
void GetSysTempFolder(CString& folderPath)
{
    TCHAR buffer[MAX_PATH];
    GetTempPath(MAX_PATH, buffer);
    folderPath.Format("%s", buffer);
}

int IsExistingFile(const CString& fileName)
{
    WIN32_FIND_DATA FindFileData;
    char fileToFind[MAX_PATH];

    sprintf(fileToFind, "%s", (LPCSTR)fileName);

    // Search for the file
    HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
        return 0; // file not found

    FindClose(hFile);

    return 1; // file found
}

int CreateDirectoryStructure(const CString& path)
{
    char buffer[1024]; // buffer is a local copy of 'path'
    memset(buffer, 0, 1024 * sizeof(char));
    if (strlen(path) > 1023)
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
    if (buffer[strlen(buffer) - 1] != pathSeparator)
        buffer[strlen(buffer)] = pathSeparator;

    char* pt = strchr(buffer, pathSeparator);

    while (pt != NULL)
    {
        pt[0] = 0;
        if ((strlen(buffer) == 2) && (':' == buffer[1]))
        {
            // do nothing, don't create C: !!
        }
        else
        {
            ret = CreateDirectory(buffer, NULL);
            if (!ret)
            {
                int error = GetLastError();
                if (error != ERROR_FILE_EXISTS && error != ERROR_ALREADY_EXISTS)
                {
                    CString message;
                    message.Format("Debug: Could not create output directory: %s", buffer);
                    ShowMessage(message); // tell the user about our problems...
                    return 1;
                }
            }
        }
        pt[0] = pathSeparator;
        if (*(pt + 1) == pathSeparator)
            pt = strchr(pt + 2, pathSeparator);
        else
            pt = strchr(pt + 1, pathSeparator);
    }

    return 0;
}

int Equals(const CString& str1, const CString& str2)
{
    return (0 == _tcsnicmp(str1, str2, max(strlen(str1), strlen(str2))));
}

int Equals(const CString& str1, const CString& str2, unsigned int nCharacters)
{
    return (0 == _tcsnicmp(str1, str2, min(nCharacters, max(strlen(str1), strlen(str2)))));
}

/** Sends a message that the given file should be uploaded to the NOVAC-Server
        as soon as possible */
void UploadToNOVACServer(const CString& fileName, int volcanoIndex, bool deleteFile)
{
    CString* fileNameBuffer = NULL;
    Communication::FTPUploadOptions* options;
    CString message;

    // The uploading thread is not running, quit it...
    if (g_ftp == NULL)
    {
        return;
    }

    // The file-name
    fileNameBuffer = new CString();
    fileNameBuffer->Format("%s", (LPCSTR)fileName);

    // The options for uploading the file
    options = new Communication::FTPUploadOptions;
    options->volcanoIndex = volcanoIndex;
    options->deleteFile = deleteFile;

    // Tell the world about our mission
    //message.Format("Will try to upload %s to NOVAC FTP-Server", (LPCSTR)fileName);
    //ShowMessage(message);

    // Tell the uploading thread to upload this file
    g_ftp->PostThreadMessage(WM_UPLOAD_NEW_FILE, (WPARAM)fileNameBuffer, (LPARAM)options);

    return;
}

void UpdateMessage(const CString& message)
{
    CString* msg = new CString();

    msg->Format("%s", (LPCSTR)message);
    if (pView != NULL)
        pView->PostMessage(WM_UPDATE_MESSAGE, (WPARAM)msg, NULL);
}

void ShowMessage(const CString& message)
{
    CString* msg = new CString();
    CString timeTxt;
    Common commonObj;
    commonObj.GetDateTimeText(timeTxt);
    msg->Format("%s -- %s", (LPCSTR)message, (LPCSTR)timeTxt);
    if (pView != NULL)
        pView->PostMessage(WM_SHOW_MESSAGE, (WPARAM)msg, NULL);
}
void ShowMessage(const std::string& message)
{
    CString* msg = new CString();
    CString timeTxt;
    Common commonObj;
    commonObj.GetDateTimeText(timeTxt);
    msg->Format("%s -- %s", message.c_str(), (LPCSTR)timeTxt);
    if (pView != NULL)
        pView->PostMessage(WM_SHOW_MESSAGE, (WPARAM)msg, NULL);
}
void ShowMessage(const CString& message, CString connectionID)
{
    CString* msg = new CString();
    CString timeTxt;
    Common commonObj;
    commonObj.GetDateTimeText(timeTxt);
    msg->Format("<%s> : %s   -- %s", (LPCSTR)connectionID, (LPCSTR)message, (LPCSTR)timeTxt);
    if (pView != NULL)
        pView->PostMessage(WM_SHOW_MESSAGE, (WPARAM)msg, NULL);
}

void ShowMessage(const TCHAR message[])
{
    CString msg;
    msg.Format("%s", message);
    ShowMessage(msg);
}

void Common::GetExePath()
{
    TCHAR exeFullPath[MAX_PATH];
    GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    m_exePath = (CString)exeFullPath;
    m_exeFileName = (CString)exeFullPath;
    int position = m_exePath.ReverseFind('\\');
    int length = CString::StringLength(m_exePath);
    m_exePath = m_exePath.Left(position + 1);
    m_exeFileName = m_exeFileName.Right(length - position - 1);
}

/** Calculate the distance (in meters) between the two points (lat1, lon1) and
    (lat2, lon2). All latitudes and longitudes should be in degrees. */
double Common::GPSDistance(double lat1, double lon1, double lat2, double lon2)
{
    const double R_Earth = 6367000; // radius of the earth
    double distance, a, c;
    lat1 = lat1 * DEGREETORAD;
    lat2 = lat2 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    lon2 = lon2 * DEGREETORAD;

    double dLon = lon2 - lon1;
    double dLat = lat2 - lat1;

    if ((dLon == 0) && (dLat == 0))
        return 0;

    a = pow((sin(dLat / 2)), 2) + cos(lat1) * cos(lat2) * pow((sin(dLon / 2)), 2);
    c = 2 * asin(min(1, sqrt(a)));
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
    lat1 = lat1 * DEGREETORAD;
    lat2 = lat2 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    lon2 = lon2 * DEGREETORAD;
    double tmpAngle;
    double dLat = lat1 - lat2;
    double dLon = lon1 - lon2;

    if ((dLon == 0) && (dLat == 0))
        return 0;

    tmpAngle = atan2(-sin(dLon) * cos(lat2),
        cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon));

    /*  	tmpAngle = atan2(lon1*cos(lat1)-lon2*cos(lat2), lat1-lat2); */

    if (tmpAngle < 0)
        tmpAngle = TWO_PI + tmpAngle;

    tmpAngle = RADTODEGREE * tmpAngle;
    return tmpAngle;
}

/** This function calculates the latitude and longitude for a point
        which is the distance 'dist' m and bearing 'az' degrees from
        the point defied by 'lat1' and 'lon1' */
void Common::CalculateDestination(double lat1, double lon1, double dist, double az, double& lat2, double& lon2)
{
    const double R_Earth = 6367000; // radius of the earth

    double dR = dist / R_Earth;

    // convert to radians
    lat1 = lat1 * DEGREETORAD;
    lon1 = lon1 * DEGREETORAD;
    az = az * DEGREETORAD;

    // calculate the second point
    lat2 = asin(sin(lat1) * cos(dR) + cos(lat1) * sin(dR) * cos(az));

    lon2 = lon1 + atan2(sin(az) * sin(dR) * cos(lat1), cos(dR) - sin(lat1) * sin(lat2));

    // convert back to degrees
    lat2 = lat2 * RADTODEGREE;
    lon2 = lon2 * RADTODEGREE;
}

bool Common::BrowseForReferenceFile(CString& fileName)
{
    return BrowseForFile("Reference files\0*.txt;*.xs\0", fileName);
}

bool Common::BrowseForFile(const TCHAR* filter, CString& fileName)
{
    TCHAR szFile[4096];
    sprintf(szFile, "%s", (LPCSTR)fileName);

    OPENFILENAME ofn;       // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.hInstance = AfxGetInstanceHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        fileName.Format(szFile);
        return true;
    }
    fileName.Format("");
    return false;
}

bool Common::BrowseForFile_SaveAs(const TCHAR* filter, CString& fileName)
{
    return BrowseForFile_SaveAs(filter, fileName, nullptr);
}

bool Common::BrowseForFile_SaveAs(const TCHAR* filter, CString& fileName, int* filterType)
{
    static TCHAR szFile[4096];
    sprintf(szFile, "%s", (LPCTSTR)fileName);

    OPENFILENAME ofn;       // common dialog box structure
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = nullptr;
    ofn.hInstance = AfxGetInstanceHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        fileName.Format(szFile);

        if (filterType != nullptr)
        {
            *filterType = ofn.nFilterIndex;
        }

        return true;
    }
    fileName.Format("");
    return false;
}

bool Common::BrowseForDirectory(CString& folderName)
{
    BROWSEINFO bi;
    char tmp_FolderName[MAX_PATH];       // temporary buffer for folder name
    char title[] = "Select Directory";

    // Initialize BROWSEINFO
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = tmp_FolderName;
    bi.lpszTitle = title;
    bi.ulFlags = BIF_USENEWUI | BIF_VALIDATE | BIF_RETURNONLYFSDIRS;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (NULL != pidl)
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            folderName.Format("%s", path);
        }

        // free memory used
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
        return true;
    }
    else
    {
        /* Error */
        return false;
    }
}

std::vector<std::string> Common::ListFilesInDirectory(const char* directory, const char* fileNameFilter)
{
    std::vector<std::string> result;

    char searchPattern[MAX_PATH];
    if (fileNameFilter == nullptr)
    {
        sprintf(searchPattern, "%s*", directory);
    }
    else
    {
        sprintf(searchPattern, "%s%s", directory, fileNameFilter);
    }

    WIN32_FIND_DATA FindFileData;
    HANDLE hFile = FindFirstFile(searchPattern, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return result; // no files found
    }

    do
    {
        if (Equals(FindFileData.cFileName, ".") || Equals(FindFileData.cFileName, ".."))
        {
            continue;
        }

        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // This is a directory that we've found. Skip
        }
        else
        {
            // Get the full name of the file that we've just found...
            CString fileName;
            fileName.Format("%s%s", directory, FindFileData.cFileName);
            result.push_back(std::string(fileName));
        }
    } while (0 != FindNextFile(hFile, &FindFileData));

    FindClose(hFile);

    return result;
}

/* pretty prints the current date into the string 'txt' */
void Common::GetDateText(CString& txt)
{
    struct tm* tim;
    time_t t;

    time(&t);
    tim = gmtime(&t);
    txt.Format("%04d.%02d.%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday);
}
int Common::GetHour()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return tim->tm_hour;
}
int Common::GetMinute()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return tim->tm_min;
}
int Common::GetSecond()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return tim->tm_sec;
}

/* pretty prints the current time into the string 'txt' */
void Common::GetTimeText(CString& txt)
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    txt.Format("%02d:%02d:%02d", tim->tm_hour, tim->tm_min, tim->tm_sec);
}
/* pretty prints the current time into the string 'txt' */
void Common::GetTimeText(CString& txt, char* seperator)
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    txt.Format("%02d%s%02d%s%02d", tim->tm_hour, seperator, tim->tm_min, seperator, tim->tm_sec);
}
/* pretty prints the current date and time into the string 'txt' */
void Common::GetDateTimeText(CString& txt)
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    txt.Format("%04d.%02d.%02d  %02d:%02d:%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min, tim->tm_sec);
}

/** Returns the current year */
int Common::GetYear()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return (tim->tm_year + 1900);
}

/** Returns the current month */
int Common::GetMonth()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return (tim->tm_mon + 1);
}

/** Returns the current day of the month */
int Common::GetDay()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = gmtime(&t);
    return (tim->tm_mday);
}

/** Takes a given year and month and returns the number of days in that month. */
int	Common::DaysInMonth(int year, int month)
{
    static int nDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    // detect non-existing months.
    if (month < 1 || month > 12)
        return 0;

    // If the month is not february, then it's easy!!!
    if (month != 2)
        return nDays[month - 1];

    // If february, then check for leap-years
    if (year % 4 != 0)
        return 28; // not a leap-year

    if (year % 400 == 0) // every year dividable by 400 is a leap-year
        return 29;

    if (year % 100 == 0) // years diviable by 4 and by 100 are not leap-years
        return 28;
    else
        return 29;		// years dividable by 4 and not by 100 are leap-years
}

/** Takes a given date and calculates the day of the year. */
int	Common::DayNr(const unsigned short day[3])
{
    novac::CDateTime d;
    d.year = day[2];
    d.month = (unsigned char)day[1];
    d.day = (unsigned char)day[0];
    return DayNr(d);
}

/** Takes a given date and calculates the day of the year. */
int	Common::DayNr(const novac::CDateTime& day)
{
    // Check errors in input
    if (day.month <= 0 || day.month > 12 || day.day < 1 || day.day > DaysInMonth(day.year, day.month))
        return 0;

    int dayNr = day.day; // the daynumber

    int m = day.month;
    while (m > 1)
    {
        dayNr += DaysInMonth(day.year, m - 1);
        --m;
    }

    return dayNr;
}

/** Returns the Julian Day. */
double Common::JulianDay(const novac::CDateTime& utcTime)
{
    int N, J, b;
    double Hd, H, JD;

    if (utcTime.year < 1901 || utcTime.year > 2100 || utcTime.month < 1 || utcTime.month > 12 || utcTime.day < 1 || utcTime.day > 31)
        return 0.0;
    if (utcTime.hour < 0 || utcTime.hour > 23 || utcTime.minute < 0 || utcTime.minute > 59 || utcTime.second < 0 || utcTime.second > 59)
        return 0.0;

    N = 4713 + utcTime.year - 1;
    J = N * 365 + N / 4 - 10 - 3;

    if (N % 4 == 1 || N % 4 == 2 || N % 4 == 3)
        ++J;

    switch (utcTime.month)
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
    if (utcTime.year % 4 == 0)
        ++b;

    H = (double)(J + b);

    Hd = (utcTime.hour - 12.0) / 24.0 + utcTime.minute / (60.0 * 24.0) + utcTime.second / (3600.0 * 24.0);						/*CONVERSION HORAR TO DECIMAL SYSTEM*/

    JD = J + Hd + b;
    return JD;
}

__int64 Common::Epoch()
{
    time_t rawtime;
    struct tm* utc;
    time(&rawtime);
    utc = gmtime(&rawtime);
    return rawtime;
}

__int64 Common::Epoch(const novac::CDateTime& utcTime)
{
    time_t rawtime;
    struct tm* utc;
    time(&rawtime);
    utc = gmtime(&rawtime);
    time_t offset = mktime(utc) - rawtime;
    if (utc->tm_isdst)
    {
        offset -= 3600;
    }
    utc->tm_year = utcTime.year - 1900;
    utc->tm_mon = utcTime.month - 1;
    utc->tm_mday = utcTime.day;
    utc->tm_hour = utcTime.hour;
    utc->tm_min = utcTime.minute;
    utc->tm_sec = utcTime.second;
    __int64 epoch = mktime(utc) - offset;
    return epoch;
}

/** Retrieves the solar zenith angle (SZA) and the solar azimuth angle (SAZ)
        for the site specified by (lat, lon) and for the time given in gmtTime.
        Note that the returned angles are in degrees and that the specified
        time _must_ be GMT-time. */
RETURN_CODE Common::GetSunPosition(const novac::CDateTime& gmtTime, double lat, double lon, double& SZA, double& SAZ)
{
    SZA = SAZ = 0; // reset the numbers

    // Get the julian day
    double D = JulianDay(gmtTime) - 2451545.0;

    // Get the Equatorial coordinates...
    double	RA; //	the right ascension (deg)
    double	dec; // the declination	(deg)
    double	EqT;	// the equation of time (hours)
    EquatorialCoordinates(D, RA, dec, EqT);

    // Get the hour angle
    double fractionalHour = (double)gmtTime.hour + gmtTime.minute / 60.0 + gmtTime.second / 3600.0;
    double H = GetHourAngle(fractionalHour, lon, EqT);

    // Get the horizontal coordinates
    double	elev, sAzim; // The elevation and azimuth (towards south);
    HorizontalCoordinates(lat, H, dec, elev, sAzim);

    // Convert the elevation into sza
    SZA = 90.0 - elev;

    // Convert the azimuth to a value counted from the north and 
    SAZ = fmod(180.0 + sAzim, 360.0);

    return SUCCESS;
}

const CString& Common::GetString(const UINT uID)
{
    static int index = 0;

    index += 1;
    index %= 3;

    int ret = m_string[index].LoadString(uID);
    return m_string[index];
}

CString& Common::SimplifyString(const CString& in)
{
    static CString str;

    // Clean the string for non-printable characters
    CleanString(in, str);

    // Make a local copy of the string
    unsigned long L = (unsigned long)strlen(str);
    char* buffer = new char[L + 2];
    sprintf(buffer, "%s", (LPCSTR)str);

    // Check all characters in the string
    for (unsigned long i = 0; i < L; ++i)
    {
        // 1. Replace spaces with underscores
        if (buffer[i] == ' ')
        {
            buffer[i] = '_';
            continue;
        }

        // 2. Convert the character to lower-case
        buffer[i] = tolower(buffer[i]);

        // 3. Remove paranthesis...
        if (buffer[i] == '(' || buffer[i] == '[' || buffer[i] == '{' || buffer[i] == ')' || buffer[i] == ']' || buffer[i] == '}')
        {
            for (unsigned long j = i; j < L - 1; ++j)
            {
                buffer[j] = buffer[j + 1];
            }
            i = i - 1;
            continue;
        }

        // 4. Check if there's any accent on the character
        if ((unsigned char)buffer[i] <= 127)
            continue;

        char c = buffer[i];

        if (c == '�' || c == '�' || c == '�' || c == '�' || c == '�')
            buffer[i] = 'a';
        else if (c == '�' || c == 'c')
            buffer[i] = 'c';
        else if (c == '�' || c == '�' || c == '�' || c == '�')
            buffer[i] = 'e';
        else if (c == '�' || c == '�' || c == '�' || c == '�')
            buffer[i] = 'i';
        else if (c == '�' || c == '�' || c == '�' || c == '�')
            buffer[i] = 'o';
        else if (c == '�' || c == '�' || c == '�' || c == '�')
            buffer[i] = 'u';
        else if (c == '�')
            buffer[i] = 'n';
    }

    // copy the buffer to a CString
    str.Format("%s", buffer);

    delete[] buffer;
    return str;
}

void Common::CleanString(const CString& in, CString& out)
{
    char* buffer = new char[strlen(in) + 2];
    sprintf(buffer, "%s", (LPCSTR)in); // make a local copy of the input string

    CleanString(buffer, out);
    delete[] buffer; // clean up after us
}

void Common::CleanString(const char* in, CString& out)
{
    out.Format("");
    for (unsigned int it = 0; it < strlen(in); ++it)
    {
        if ((unsigned char)in[it] >= 32)
            out.AppendFormat("%c", in[it]);
    }
}

/** Sorts a list of strings in either ascending or descending order */
void Common::Sort(CList <CString, CString&>& strings, bool files, bool ascending)
{
    unsigned long nStrings = (unsigned long)strings.GetCount(); // number of elements
    unsigned long it = 0; // <-- iterator

    if (nStrings <= 1)
    {
        return; // <-- We're actually already done
    }
    else
    {
        CList <CString, CString&> left;
        CList <CString, CString&> right;

        // Make two copies of the list, one of the first half and one of the second half
        POSITION pos = strings.GetHeadPosition();
        while (it < nStrings / 2)
        {
            left.AddTail(strings.GetNext(pos));
            ++it;
        }
        while (pos != NULL)
        {
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
void Common::MergeLists(const CList <CString, CString&>& list1, const CList <CString, CString&>& list2, CList <CString, CString&>& result, bool files, bool ascending)
{
    CString	name1, name2, fullName1, fullName2;
    int comparison;

    POSITION pos_1 = list1.GetHeadPosition();
    POSITION pos_2 = list2.GetHeadPosition();

    // Clear the output-list
    result.RemoveAll();

    // 1. As long as there are elements in both lists, do this
    while (pos_1 != NULL && pos_2 != NULL)
    {
        // Get the file-names of the first and the second 
        fullName1.Format(list1.GetAt(pos_1));	// position k
        fullName2.Format(list2.GetAt(pos_2));	// position k+1

        if (files)
        {
            // Extract the file-names only
            name1.Format(fullName1);
            name2.Format(fullName2);
            Common::GetFileName(name1);
            Common::GetFileName(name2);

            // Compare the two names
            comparison = name1.Compare(name2);
        }
        else
        {
            // Compare the two names
            comparison = fullName1.Compare(fullName2);
        }

        if (comparison == 0)
        {
            // if equal
            result.AddTail(fullName1);
            list1.GetNext(pos_1);
            continue;
        }
        else if (comparison < 0)
        {
            // fullName1 < fullName2
            if (ascending)
            {
                result.AddTail(fullName1);
                list1.GetNext(pos_1);
                continue;
            }
            else
            {
                result.AddTail(fullName2);
                list2.GetNext(pos_2);
                continue;
            }
        }
        else
        {
            // fullName1 > fullName2
            if (ascending)
            {
                result.AddTail(fullName2);
                list2.GetNext(pos_2);
                continue;
            }
            else
            {
                result.AddTail(fullName1);
                list1.GetNext(pos_1);
                continue;
            }
        }

    }

    // 2. If we're out of elements in list 2 but not in list 1, do this
    while (pos_1 != NULL)
    {
        fullName1.Format(list1.GetNext(pos_1));
        result.AddTail(fullName1);
    }

    // 3. If we're out of elements in list 1 but not in list 2, do this
    while (pos_2 != NULL)
    {
        fullName2.Format(list2.GetNext(pos_2));
        result.AddTail(fullName2);
    }

}

// get the gas factor for the supplied specie
double Common::GetGasFactor(const CString& specie)
{
    if (0 == _tcsncicmp(specie, "SO2", 3))
    {
        return GASFACTOR_SO2;
    }
    if (0 == _tcsncicmp(specie, "O3", 2))
    {
        return GASFACTOR_O3;
    }
    if (0 == _tcsncicmp(specie, "NO2", 3))
    {
        return GASFACTOR_NO2;
    }

    return -1;
}

double Common::CalculateFlux(const double* scanAngle, const double* scanAngle2, const double* column, double offset, int nDataPoints, const CWindField& wind, double compass, double gasFactor, double coneAngle, double tilt)
{
    double windSpeed = wind.GetWindSpeed();
    double windDirection = wind.GetWindDirection();
    double plumeHeight = wind.GetPlumeHeight();

    if (fabs(coneAngle - 90.0) < 1.0)
        return CalculateFluxFlatScanner(scanAngle, column, offset, nDataPoints, windSpeed, windDirection, plumeHeight, compass, gasFactor);
    else
        return CalculateFluxConicalScanner(scanAngle, column, offset, nDataPoints, windSpeed, windDirection, plumeHeight, compass, gasFactor, coneAngle, tilt);
}


void Common::GuessSpecieName(const CString& fileName, CString& specie)
{
    specie.Format("");
    CString spc[] = { "SO2", "NO2", "O3", "O4", "HCHO", "RING", "H2O", "CLO", "BRO", "CHOCHO", "Glyoxal", "Formaldehyde", "HONO", "NO3" };
    int nSpecies = 12;

    int index = fileName.ReverseFind('\\');
    if (index == 0)
        return;

    CString fil;
    fil.Format("%s", (LPCSTR)fileName.Right((int)strlen(fileName) - index - 1));
    fil.MakeUpper();

    for (int i = 0; i < nSpecies; ++i)
    {
        if (strstr(fil, spc[i]))
        {
            specie.Format("%s", (LPCSTR)spc[i]);
            return;
        }
    }

    // nothing found
    return;
}


int Common::CheckProcessExistance(CString& exeName, int pid)
{
    int ret;
    CString processPath;
    DWORD processid[1024], needed, processcount, i;
    HMODULE hModule;
    char path[MAX_PATH] = "";

    EnumProcesses(processid, sizeof(processid), &needed);
    processcount = needed / sizeof(DWORD);
    for (i = 0; i < processcount; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processid[i]);
        if (hProcess)
        {
            EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed);
            GetModuleFileNameEx(hProcess, hModule, path, sizeof(path));
            processPath.Format("%s", path);
            GetFileName(processPath);
            ret = processPath.Compare(exeName); //compare exe name and the process name
            if (ret == 0) // names are same
            {
                if (pid != -1)
                {
                    if (processid[i] == pid)  //compare process id
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
int Common::GetAllProcessIDs(CString& exeName, int pIDs[1024], int startPid)
{
    int nPIDsFound = 0;
    DWORD processid[1024];
    DWORD needed;
    HMODULE hModule;
    memset(pIDs, -1, 1024 * sizeof(int)); // set all values to -1

    if (!EnumProcesses(processid, sizeof(processid), &needed))
    {
        return 0;
    }
    DWORD processcount = needed / sizeof(DWORD);
    for (unsigned int i = 0; i < processcount; i++)
    {
        if (processid[i] == 0)
        {
            continue;
        }
        CString processPath;
        char path[MAX_PATH] = "";
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processid[i]);
        if (hProcess)
        {
            EnumProcessModules(hProcess, &hModule, sizeof(hModule), &needed);
            GetModuleFileNameEx(hProcess, hModule, path, sizeof(path));
            processPath.Format("%s", path);
            GetFileName(processPath);
            int ret = processPath.Compare(exeName); //compare exe name and the process name
            if (ret == 0) // names are same
            {
                if ((int)processid[i] >= startPid)
                {
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
        if (!IsWindowsXPOrGreater())
        {
            MessageBox(NULL, "You need at least Windows XP", "Version Not Supported", MB_OK);
            return SetLastError(ERROR_ACCESS_DENIED), FALSE;
        }

        // enable SE_DEBUG_NAME privilege and try again

        TOKEN_PRIVILEGES Priv, PrivOld;
        DWORD cbPriv = sizeof(PrivOld);
        HANDLE hToken;

        // obtain the token of the current thread
        if (!OpenThreadToken(GetCurrentThread(),
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
            FALSE, &hToken))
        {
            if (GetLastError() != ERROR_NO_TOKEN)
                return FALSE;

            // revert to the process token
            if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
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

void Common::GetFileName(CString& fileName)
{
    int position = fileName.ReverseFind('\\');
    int length = CString::StringLength(fileName);
    fileName = fileName.Right(length - position - 1);
}

void Common::GetFileName(std::string& fullFileNameAndPath)
{
    auto position = fullFileNameAndPath.rfind('\\');
    if (position == std::string::npos)
    {
        return;
    }
    auto length = fullFileNameAndPath.size();

    fullFileNameAndPath = fullFileNameAndPath.substr(position + 1, length - position - 1);
}

/** Take out the directory from a long path name.
    @param fileName - the complete path of the file */
void Common::GetDirectory(CString& fileName)
{
    int position = fileName.ReverseFind('\\');
    if (position >= 0)
        fileName = fileName.Left(position + 1);
}

long Common::RetrieveFileSize(CString& fileName)
{
    TRY
    {
        CFile file(fileName,CFile::modeRead | CFile::shareDenyNone);
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

bool Common::FormatErrorCode(DWORD error, CString& string)
{
    /* from System Error Codes */
    switch (error)
    {
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

int Common::GetInterlaceSteps(int channel, int& interlaceSteps)
{
    // if the spectrum is a mix of several spectra
    if (channel >= 129)
    {
        interlaceSteps = channel - 127;
        return -1;
    }

    // special case, channel = 128 is same as channel = 0
    if (channel == 128)
        channel = 0;

    // If the spectrum is a single spectrum
    interlaceSteps = (channel / 16) + 1; // 16->31 means interlace=2, 32->47 means interlace=3 etc.
    return (channel % 16); // the remainder tells the channel number
}

/** Find the volcano-index that the spectrometer with the supplied
        serial-number monitors. If none is found then -1 is returned */
int	Common::GetMonitoredVolcano(const CString& serialNumber)
{

    // find the name of the volcano that is monitored
    CString volcanoName;
    for (unsigned int k = 0; k < g_settings.scannerNum; ++k)
    {
        for (unsigned int j = 0; j < g_settings.scanner[k].specNum; ++j)
        {
            if (Equals(serialNumber, g_settings.scanner[k].spec[j].serialNumber))
            {
                volcanoName.Format(g_settings.scanner[k].volcano);
                break;
            }
        }
    }

    // now find the index of this volcano
    if (strlen(volcanoName) == 0)
        return -1; // <-- nothing found

    for (unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k)
    {
        if (Equals(volcanoName, g_volcanoes.m_name[k]))
        {
            return k;
        }
    }

    return -1; // could not find the volcano-name
}

/*EQUATORIAL COORDINATES:RIGHT ASCENSION AND DECLINATION*/
void Common::EquatorialCoordinates(double D, double& RA, double& dec, double& EQT)
{
    double g_deg, q_deg, L_deg;				/*ANGLES IN DEGREES*/
    double g_rad, q_rad, L_rad;				/*ANGLES IN	RADIANS*/
    double R;												/*DISTANCE SUN-EARTH IN A.U*/
    double obliq_deg, obliq_rad;			/*OBLIQUITY OF THE ECLIPTIC*/
    double RA_rad, dec_rad;					/*EQUATORIAL COORDINATES IN RADIANS*/

    g_deg = fmod(357.529 + 0.98560028 * D, 360.0);
    g_rad = g_deg * DEGREETORAD;
    q_deg = fmod(280.459 + 0.98564736 * D, 360.0);
    q_rad = q_deg * DEGREETORAD;

    L_deg = q_deg + 1.915 * sin(g_rad) + 0.02 * sin(2 * g_rad);
    L_rad = L_deg * DEGREETORAD;

    // The distance between the sun and the earth (in Astronomical Units)
    R = 1.00014 - 0.01671 * cos(g_rad) - 0.00014 * cos(2 * g_rad);

    // The obliquity of the earth's orbit:
    obliq_deg = 23.439 - 0.00000036 * D;
    obliq_rad = obliq_deg * DEGREETORAD;

    // The right ascension (RA)
    RA_rad = atan(cos(obliq_rad) * sin(L_rad) / cos(L_rad));
    if (RA_rad < 0)
        RA_rad = TWO_PI + RA_rad;

    if (fabs(RA_rad - L_rad) > 1.570796)
        RA_rad = M_PI + RA_rad;

    dec_rad = asin(sin(obliq_rad) * sin(L_rad));
    RA = fmod(RA_rad * RADTODEGREE, 360.0);		// The right ascension

    // The declination
    dec = dec_rad * RADTODEGREE;

    // The Equation of Time
    EQT = q_deg / 15.0 - RA / 15.0;
}

void Common::HorizontalCoordinates(double lat, double H, double dec, double& elev, double& azim)
{
    double H_rad = H * DEGREETORAD;
    double lat_rad = lat * DEGREETORAD;
    double dec_rad = dec * DEGREETORAD;

    // The elevation angle
    double elev_rad = asin(cos(H_rad) * cos(dec_rad) * cos(lat_rad) + sin(dec_rad) * sin(lat_rad));

    // The cosine of the azimuth - angle
    double cazim_rad = (cos(H_rad) * cos(dec_rad) * sin(lat_rad) - sin(dec_rad) * cos(lat_rad)) / cos(elev_rad);

    // The sine of the azimuth - angle
    double sazim_rad = (sin(H_rad) * cos(dec_rad)) / cos(elev_rad);

    double azim_rad = 0.0;
    if (cazim_rad > 0 && sazim_rad > 0)
        azim_rad = asin(sazim_rad);						// azim is in the range 0 - 90 degrees
    else if (cazim_rad < 0 && sazim_rad > 0)
        azim_rad = M_PI - asin(sazim_rad);			// azim is in the range 90 - 180 degrees
    else if (cazim_rad < 0 && sazim_rad < 0)
        azim_rad = M_PI - asin(sazim_rad);		// azim is in the range 180 - 270 degrees
    else if (cazim_rad > 0 && sazim_rad < 0)
        azim_rad = TWO_PI + asin(sazim_rad);		// azim is in the range 270 - 360 degrees

    elev = elev_rad * RADTODEGREE;
    azim = azim_rad * RADTODEGREE;

    //printf("\n\nHORIZONTAL COORDINATES:\nAZIMUTH (from South to West)=%f\nELEVATION=%f\n\n",*pazim,*pelev);
}

/** Returns the hour angle given the longitude and equation of time. */
double Common::GetHourAngle(double hr, double lon, double EqT)
{
    double H = 15.0 * (hr + lon / 15 + EqT - 12);
    //    printf("HOUR ANGLE (from noon,increasing with time): %f\n",H);
    return(H);
}

bool Common::FillBuffer(char* srcBuf, char* destBuf, long destStart, long moveLen)
{
    long i;
    // destination buffer is full
    if (destStart + moveLen >= MAXBUFFER)
        return false;
    for (i = 0; i < moveLen; i++)
    {
        destBuf[destStart + i] = srcBuf[i];
    }
    return true;

}

int Common::CheckForSpectraInDir(const CString& path, CList <CString, CString&>& fileList)
{
    if (GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
    {
        return INVALID_FILE_ATTRIBUTES;
    }

    WIN32_FIND_DATA FindFileData;
    char fileToFind[MAX_PATH];

    // Find all .pak-files in the specified directory
    sprintf(fileToFind, "%s\\?????????.pak", (LPCTSTR)path);

    // Search for the file
    HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0; // no files found
    }

    do
    {
        CString fileName;
        fileName.Format("%s\\%s", (LPCTSTR)path, FindFileData.cFileName);

        if (!Equals(FindFileData.cFileName, "Upload.pak"))
        {
            // Tell the user that we've found one scan which hasn't been evaluated
            CString msg;
            msg.Format("Spectra in %s not yet evaluated.  Will evalute now.", FindFileData.cFileName);
            ShowMessage(msg);

            // Append the found file to the list of files to split and evaluate...
            fileList.AddTail(fileName);
        }

    } while (0 != FindNextFile(hFile, &FindFileData));

    FindClose(hFile);

    return 0;
}

void Common::CheckForSpectraInHexDir(const CString& path, CList <CString, CString&>& fileList)
{
    WIN32_FIND_DATA FindFileData;
    char fileToFind[MAX_PATH];
    CList <CString, CString&> pathList;
    CString pathName;

    // Find all RXYZ - directories...
    // Since version 3.3, this will check for directories that begin with R and up to 9 chars long.
    // This is to support subdirectory searching in the directory polling option.
    sprintf(fileToFind, "%s\\R?????????", (LPCTSTR)path);

    // Search for the directories
    HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return; // no directories found
    }

    do
    {
        pathName.Format("%s\\%s", (LPCTSTR)path, (LPCTSTR)FindFileData.cFileName);

        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // This is a directory, add it to the list of directories to check...

            pathList.AddTail(pathName);
        }
    } while (0 != FindNextFile(hFile, &FindFileData));

    FindClose(hFile);

    // Check each of the directories found...
    POSITION pos = pathList.GetHeadPosition();
    while (pos != NULL)
    {
        pathName.Format(pathList.GetNext(pos));

        // Check the directory...
        CheckForSpectraInDir(pathName, fileList);
    }

    return;
}
