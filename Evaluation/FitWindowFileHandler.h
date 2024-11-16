#pragma once

#include "../Common/Common.h"
#include "../Common/XMLFileReader.h"
#include <SpectralEvaluation/Evaluation/FitWindow.h>

namespace FileHandler
{

/** A <b>CFitWindowFileHandler</b> object is capable of reading
    and writing fit windows from a .nfw (NovacFitWindow) file. */
class CFitWindowFileHandler : public CXMLFileReader
{
public:
    CFitWindowFileHandler() {}
    ~CFitWindowFileHandler() {}

    /** Reads the fit windows which are defined in the provided file.
            @param fileName The name and path of the file to read from
            @return A vector with all the fit windows defined in the file.
            @return An empty vector if the reading failed. */
    std::vector<novac::CFitWindow> ReadFitWindowFile(const CString& fileName);

    /** Writes the supplied fit-window to a file.
            @param window - the fit window to be written to file
            @param fileName - the name and path of the file to which to write
            @param overWrite - if true the file will be overwritten, if false, the file will be appended */
    RETURN_CODE WriteFitWindow(const novac::CFitWindow& window, const CString& fileName, bool overWrite);

private:

    /** Parses a fit-window section of the .nfs file */
    RETURN_CODE Parse_FitWindow(novac::CFitWindow& window);

    /** Parses a reference-file section of the .nfs file */
    RETURN_CODE Parse_Reference(novac::CFitWindow& window);
};
}