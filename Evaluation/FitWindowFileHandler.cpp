#include "StdAfx.h"
#include "FitWindowFileHandler.h"

using namespace FileHandler;

CFitWindowFileHandler::CFitWindowFileHandler()
{
}

CFitWindowFileHandler::~CFitWindowFileHandler()
{
}

std::vector<Evaluation::CFitWindow> CFitWindowFileHandler::ReadFitWindowFile(const CString &fileName)
{
    std::vector<Evaluation::CFitWindow> allWindowsRead;

    CFileException exceFile;
    CStdioFile file;

    // 1. Open the file
    if (!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile))
    {
        return allWindowsRead;
    }
    this->SetFile(&file);

    // parse the file
    while (szToken = NextToken())
    {
        // no use to parse empty lines
        if (strlen(szToken) < 3)
            continue;

        if (Equals(szToken, "fitWindow", 9))
        {
            Evaluation::CFitWindow tmpWindow;
            if (SUCCESS == Parse_FitWindow(tmpWindow))
            {
                allWindowsRead.push_back(tmpWindow);
            }
            else
            {
                // parse_fit window has failed!
                break;
            }
        }
    }

    file.Close();
    return allWindowsRead;
}

RETURN_CODE CFitWindowFileHandler::Parse_FitWindow(Evaluation::CFitWindow &window)
{
    window.Clear(); // <-- Reset the data before we start reading from the file.

    // find the name for this fit window
    if (char *pt = strstr(szToken, "name"))
    {
        if (pt = strstr(pt, "\""))
        {
            if (char *pt2 = strstr(pt + 1, "\""))
                pt2[0] = 0; // remove the second quote
            char tmpStr[512];
            if (sscanf(pt + 1, "%s", &tmpStr))
            {
                window.name = std::string(tmpStr);
            }
        }
    }

    // parse the file
    while (szToken = NextToken()) {
        // no use to parse empty lines
        if (strlen(szToken) < 2)
            continue;

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        // end of fit-window section
        if (Equals(szToken, "/fitWindow")) {
            return SUCCESS;
        }

        if (Equals(szToken, "fitLow")) {
            Parse_IntItem(TEXT("/fitLow"), window.fitLow);
            continue;
        }
        if (Equals(szToken, "fitHigh")) {
            Parse_IntItem(TEXT("/fitHigh"), window.fitHigh);
            continue;
        }
        if (Equals(szToken, "polyOrder")) {
            Parse_IntItem(TEXT("/polyOrder"), window.polyOrder);
            continue;
        }
        if (Equals(szToken, "fitType")) {
            Parse_IntItem(TEXT("/fitType"), (int&)window.fitType); // TODO: Will this be ok????
            continue;
        }
        if (Equals(szToken, "channel")) {
            Parse_IntItem(TEXT("/channel"), window.channel);
            continue;
        }
        if (Equals(szToken, "specLength")) {
            Parse_IntItem(TEXT("/specLength"), window.specLength);
            continue;
        }
        if (Equals(szToken, "fOptShift")) {
            int parsedFlag = 0;
            Parse_IntItem(TEXT("/fOptShift"), parsedFlag);
            window.findOptimalShift = (parsedFlag > 0);
            continue;
        }
        if (Equals(szToken, "UV")) {
            int parsedFlag = 0;
            Parse_IntItem(TEXT("/UV"), parsedFlag);
            window.UV = (parsedFlag > 0);
            continue;
        }
        if (Equals(szToken, "shiftSky")) {
            int parsedFlag = 0;
            Parse_IntItem(TEXT("/shiftSky"), parsedFlag);
            window.shiftSky = (parsedFlag > 0);
            continue;
        }
        if (Equals(szToken, "interlaceStep")) {
            Parse_IntItem(TEXT("/interlaceStep"), window.interlaceStep);
            continue;
        }
        if (Equals(szToken, "interlaced")) {
            Parse_IntItem(TEXT("/interlaced"), window.interlaceStep);
            window.interlaceStep += 1;
            continue;
        }
		if (Equals(szToken, "solarSpectrum")) {
			Parse_StringItem(TEXT("/solarSpectrum"), window.fraunhoferRef.m_path);
			window.fraunhoferRef.m_specieName = "SolarSpec";
			continue;
		}
        //if(Equals(szToken, "nRef")){
        //	Parse_IntItem(TEXT("/nRef"), window.nRef);
    //		continue;
    //	}

        if (Equals(szToken, "ref", 3)) {
            Parse_Reference(window);
            continue;
        }
    }

    return FAIL;
}

RETURN_CODE CFitWindowFileHandler::Parse_Reference(Evaluation::CFitWindow &window) {
    int nRef = window.nRef;

    // find the name for this reference.
    if (char *pt = strstr(szToken, "name")) {
        if (pt = strstr(pt, "\"")) {
            if (char *pt2 = strstr(pt + 1, "\""))
                pt2[0] = 0; // remove the second quote
            char tmpStr[512];
            if (sscanf(pt + 1, "%s", &tmpStr)) {
                window.ref[nRef].m_specieName = std::string(tmpStr);
            }
        }
    }

    // the actual reading loop
    while (szToken = NextToken()) {

        // no use to parse empty lines
        if (strlen(szToken) < 3)
            continue;

        // ignore comments
        if (Equals(szToken, "!--", 3)) {
            continue;
        }

        if (Equals(szToken, "/ref")) {
            ++window.nRef;
            return SUCCESS;
        }

        if (Equals(szToken, "path")) {
            Parse_StringItem(TEXT("/path"), window.ref[nRef].m_path);
            continue;
        }

        if (Equals(szToken, "shiftOption")) {
            int tmpInt;
            Parse_IntItem(TEXT("/shiftOption"), tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LIMIT; break;
            }
            continue;
        }

        if (Equals(szToken, "shiftValue")) {
            Parse_FloatItem(TEXT("/shiftValue"), window.ref[nRef].m_shiftValue);
            continue;
        }

        if (Equals(szToken, "squeezeOption")) {
            int tmpInt;
            Parse_IntItem(TEXT("/squeezeOption"), tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LIMIT; break;
            }
            continue;
        }

        if (Equals(szToken, "squeezeValue")) {
            Parse_FloatItem(TEXT("/squeezeValue"), window.ref[nRef].m_squeezeValue);
            continue;
        }

        if (Equals(szToken, "columnOption")) {
            int tmpInt;
            Parse_IntItem(TEXT("/columnOption"), tmpInt);
            switch (tmpInt) {
            case 0: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FREE; break;
            case 1: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FIX; break;
            case 2: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LINK; break;
            case 3: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LIMIT; break;
            }
            continue;
        }

        if (Equals(szToken, "columnValue")) {
            Parse_FloatItem(TEXT("/columnValue"), window.ref[nRef].m_columnValue);
            continue;
        }
    }

    return FAIL;
}

RETURN_CODE CFitWindowFileHandler::WriteFitWindow(const Evaluation::CFitWindow &window, const CString &fileName, bool overWrite)
{
    FILE *f = nullptr;
    CString indent;

    // Open the file
    if (overWrite)
        f = fopen(fileName, "w");
    else
        f = fopen(fileName, "a+");

    // Check so that we could read from the file.
    if (nullptr == f)
        return FAIL;

    fprintf(f, "<fitWindow name=\"%s\">\n", window.name.c_str());
    indent.Format("\t");

    fprintf(f, "%s<fitLow>%d</fitLow>\n", (LPCSTR)indent, window.fitLow);
    fprintf(f, "%s<fitHigh>%d</fitHigh>\n", (LPCSTR)indent, window.fitHigh);
    fprintf(f, "%s<polyOrder>%d</polyOrder>\n", (LPCSTR)indent, window.polyOrder);
    fprintf(f, "%s<fitType>%d</fitType>\n", (LPCSTR)indent, window.fitType);

    fprintf(f, "%s<channel>%d</channel>\n", (LPCSTR)indent, window.channel);
    fprintf(f, "%s<specLength>%d</specLength>\n", (LPCSTR)indent, window.specLength);

    fprintf(f, "%s<fOptShift>%d</fOptShift>\n", (LPCSTR)indent, window.findOptimalShift);
    fprintf(f, "%s<UV>%d</UV>\n", (LPCSTR)indent, window.UV);
    fprintf(f, "%s<shiftSky>%d</shiftSky>\n", (LPCSTR)indent, window.shiftSky);
    fprintf(f, "%s<interlaceStep>%d</interlaceStep>\n", (LPCSTR)indent, window.interlaceStep);

    if (window.fraunhoferRef.m_path.size() > 4)
    {
        fprintf(f, "%s<solarSpectrum>%s</solarSpectrum>\n", (LPCSTR)indent, window.fraunhoferRef.m_path.c_str());
    }

    fprintf(f, "%s<nRef>%d</nRef>\n", (LPCSTR)indent, window.nRef);

    for (int i = 0; i < window.nRef; ++i)
    {
        fprintf(f, "%s<ref name=\"%s\">\n", (LPCSTR)indent, window.ref[i].m_specieName.c_str());
        fprintf(f, "%s\t<path>%s</path>\n", (LPCSTR)indent, window.ref[i].m_path.c_str());

        fprintf(f, "%s\t<shiftOption>%d</shiftOption>\n", (LPCSTR)indent, window.ref[i].m_shiftOption);
        if (window.ref[i].m_shiftOption != Evaluation::SHIFT_FREE)
        {
            fprintf(f, "%s\t<shiftValue>%lf</shiftValue>\n", (LPCSTR)indent, window.ref[i].m_shiftValue);
        }

        fprintf(f, "%s\t<squeezeOption>%d</squeezeOption>\n", (LPCSTR)indent, window.ref[i].m_squeezeOption);
        if (window.ref[i].m_squeezeOption != Evaluation::SHIFT_FREE)
        {
            fprintf(f, "%s\t<squeezeValue>%lf</squeezeValue>\n", (LPCSTR)indent, window.ref[i].m_squeezeValue);
        }

        fprintf(f, "%s\t<columnOption>%d</columnOption>\n", (LPCSTR)indent, window.ref[i].m_columnOption);
        if (window.ref[i].m_columnOption != Evaluation::SHIFT_FREE)
        {
            fprintf(f, "%s\t<columnValue>%lf</columnValue>\n", (LPCSTR)indent, window.ref[i].m_columnValue);
        }

        fprintf(f, "%s</ref>\n", (LPCSTR)indent);
    }

    fprintf(f, "</fitWindow>\n");

    // close the file again
    fclose(f);

    return SUCCESS;
}

