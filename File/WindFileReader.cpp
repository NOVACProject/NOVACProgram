#include "StdAfx.h"
#include "WindFileReader.h"

using namespace FileHandler;

RETURN_CODE CWindFileReader::ReadWindFile(CWindFieldDatabase& result)
{
    CWindField windfield; // <-- the next wind-field to insert
    char dateStr[] = _T("date"); // this string only exists in the header line.
    char sourceStr[] = _T("source");
    char szLine[8192];
    MET_SOURCE windFieldSource = MET_USER; // the source for the wind-information

    result.Clear();

    // Lock this object to make sure that now one else tries to read
    //  data from this object while we are reading
    CSingleLock singleLock(&m_critSect);
    singleLock.Lock();

    if (singleLock.IsLocked())
    {
        // If no evaluation log selected, quit
        if (strlen(m_windFile) <= 1)
        {
            singleLock.Unlock(); // open up this object again
            return FAIL;
        }

        // Open the wind log
        FILE *f = fopen(m_windFile, "r");
        if (nullptr == f)
        {
            singleLock.Unlock(); // open up this object again
            return FAIL;
        }

        // Reset prior knowledge of the contents of each column
        ResetColumns();

        // Read the file, one line at a time
        while (fgets(szLine, 8192, f))
        {
            // ignore empty lines
            if (strlen(szLine) < 2)
            {
                continue;
            }

            // ignore comment lines
            if (szLine[0] == '#' || szLine[0] == '%')
                continue;

            // convert the string to all lower-case letters
            for (unsigned int it = 0; it < strlen(szLine); ++it)
            {
                szLine[it] = tolower(szLine[it]);
            }

            // If this is the line saying the source of the information...
            if (strstr(szLine, sourceStr))
            {
                ParseSourceString(szLine, windFieldSource);
            }

            // if this is a header-line then parse it
            if (nullptr != strstr(szLine, dateStr))
            {
                ParseFileHeader(szLine, result);
                continue;
            }

            // Split the scan information up into tokens and parse them. 
            char* szToken = (char*)(LPCSTR)szLine;
            int curCol = -1;
            while (szToken = strtok(szToken, " \t"))
            {
                ++curCol;

                // First check the time
                if (curCol == m_col.time)
                {
                    int fValue1, fValue2, fValue3;
                    int nValues;

                    if (strstr(szToken, ":"))
                        nValues = sscanf(szToken, "%d:%d:%d", &fValue1, &fValue2, &fValue3);
                    else
                        nValues = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);

                    if (nValues == 2)
                    {
                        windfield.SetTime(fValue1, fValue2, 0);
                    }
                    else
                    {
                        windfield.SetTime(fValue1, fValue2, fValue3);
                    }

                    szToken = nullptr;
                    continue;
                }

                // Then check the date
                if (curCol == m_col.date)
                {
                    int fValue1, fValue2, fValue3;
                    int nValues;

                    if (strstr(szToken, "_"))
                        nValues = sscanf(szToken, "%d_%d_%d", &fValue1, &fValue2, &fValue3);
                    else
                        nValues = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);

                    if (nValues == 2)
                    {
                        windfield.SetDate(fValue1, fValue2, 0);
                    }
                    else
                    {
                        windfield.SetDate(fValue1, fValue2, fValue3);
                    }
                    szToken = nullptr;
                    continue;
                }

                // The wind-direction
                if (curCol == m_col.windDirection)
                {
                    double fValue;
                    int nValues = sscanf(szToken, "%lf", &fValue);
                    windfield.SetWindDirection(fValue, windFieldSource);
                    szToken = nullptr;
                    continue;
                }

                // The wind-speed
                if (curCol == m_col.windSpeed)
                {
                    double fValue;
                    int nValues = sscanf(szToken, "%lf", &fValue);
                    windfield.SetWindSpeed(fValue, windFieldSource);
                    szToken = nullptr;
                    continue;
                }

                // The plume height
                if (curCol == m_col.plumeHeight)
                {
                    double fValue;
                    int nValues = sscanf(szToken, "%lf", &fValue);
                    windfield.SetPlumeHeight(fValue, windFieldSource);
                    szToken = nullptr;
                    continue;
                }

                // parse the next token...
                szToken = nullptr;
            }//end while(szToken = strtok(szToken, " \t"))

            // insert the recently read wind-field into the list
            result.InsertWindField(windfield);
        }

        // close the file
        fclose(f);

        // Remember to open up this object again
        singleLock.Unlock();
    }

    // all is ok..
    return SUCCESS;
}

void CWindFileReader::ParseFileHeader(const char szLine[8192], CWindFieldDatabase& database)
{
    char date[] = _T("date");
    char time[] = _T("time");
    char speed[] = _T("speed");
    char speed2[] = _T("ws");
    char speedErr[] = _T("wserr");
    char direction[] = _T("direction");
    char direction2[] = _T("dir");
    char direction3[] = _T("wd");
    char directionErr[] = _T("wderr");
    char height[] = _T("plumeheight");
    char height2[] = _T("height");
    char height3[] = _T("ph");
    char heightErr[] = _T("pherr");
    int curCol = -1;

    // reset some old information
    ResetColumns();

    // Tokenize the string and see which column belongs to which what value
    char str[8192];
    if (szLine[0] == '#')
        strncpy(str, szLine + 1, 8191 * sizeof(char));
    else
        strncpy(str, szLine, 8192 * sizeof(char));
    char* szToken = (char*)(LPCSTR)str;

    while (szToken = strtok(szToken, "\t"))
    {
        ++curCol;

        // The time
        if (0 == _strnicmp(szToken, time, strlen(time)))
        {
            m_col.time = curCol;
            szToken = nullptr;
            continue;
        }

        // The date
        if (0 == _strnicmp(szToken, date, strlen(date)))
        {
            m_col.date = curCol;
            szToken = nullptr;
            continue;
        }

        // The speed
        if (0 == _strnicmp(szToken, speed, strlen(speed)) || 0 == _strnicmp(szToken, speed2, strlen(speed2)))
        {
            m_col.windSpeed = curCol;
            database.m_containsWindSpeed = true;
            szToken = nullptr;
            continue;
        }

        // The uncertainty in the speed
        if (0 == _strnicmp(szToken, speedErr, strlen(speedErr)))
        {
            m_col.windSpeedError = curCol;
            szToken = nullptr;
            continue;
        }

        // The direction
        if (0 == _strnicmp(szToken, direction, strlen(direction)) ||
            0 == _strnicmp(szToken, direction2, strlen(direction2)) ||
            0 == _strnicmp(szToken, direction3, strlen(direction3)))
        {

            m_col.windDirection = curCol;
            database.m_containsWindDirection = true;
            szToken = nullptr;
            continue;
        }

        // The uncertainty in the direction
        if (0 == _strnicmp(szToken, directionErr, strlen(directionErr)))
        {
            m_col.windDirectionError = curCol;
            szToken = nullptr;
            continue;
        }

        // The plume height
        if (0 == _strnicmp(szToken, height, strlen(height)) ||
            0 == _strnicmp(szToken, height2, strlen(height2)) ||
            0 == _strnicmp(szToken, height3, strlen(height3)))
        {

            m_col.plumeHeight = curCol;
            database.m_containsPlumeHeight = true;
            szToken = nullptr;
            continue;
        }

        // The uncertainty in the plume height
        if (0 == _strnicmp(szToken, heightErr, strlen(heightErr)))
        {
            m_col.plumeHeightError = curCol;
            szToken = nullptr;
            continue;
        }
    }

    // Make sure that columns which are not found are not in the 
    //  log-columns structure.
    if (!database.m_containsWindDirection)
        m_col.windDirection = -1;
    if (!database.m_containsWindSpeed)
        m_col.windSpeed = -1;


    // done!!!
    return;
}

void CWindFileReader::ResetColumns()
{
    m_col.date = 0;
    m_col.time = 1;
    m_col.windDirection = 2;
    m_col.windSpeed = 3;
    m_col.plumeHeight = -1; // not in the first generation of wind-field files...
}

void CWindFileReader::ParseSourceString(const char szLine[8192], MET_SOURCE& source)
{
    source = MET_USER; // general assumption...

    if (strstr(szLine, "user"))
    {
        source = MET_USER;
    }
    else if (strstr(szLine, "default"))
    {
        source = MET_DEFAULT;
    }
    else if (strstr(szLine, "ecmwf_forecast"))
    {
        source = MET_ECMWF_FORECAST;
    }
    else if (strstr(szLine, "ecmwf_analysis"))
    {
        source = MET_ECMWF_ANALYSIS;
    }
    else if (strstr(szLine, "triangulation"))
    {
        source = MET_GEOMETRY_CALCULATION;
    }
}
