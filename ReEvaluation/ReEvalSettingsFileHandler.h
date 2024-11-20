#pragma once
#include "../Common/XMLFileReader.h"
#include "ReEvaluator.h"

namespace FileHandler
{

/** A <b>CReEvalSettingsFileHandler</b> object is capable of reading
    and writing the settings for the re-evaluations to and from a .xml file */

class CReEvalSettingsFileHandler : public CXMLFileReader
{
public:
    CReEvalSettingsFileHandler(void);
    ~CReEvalSettingsFileHandler(void);

    /** Reads the desired re-evaluation settings from the file.
            @param reeval - will be filled with the read settings if successfull
            @param fileName - the name and path of the file to read from
            @return SUCCESS on success */
    RETURN_CODE ReadSettings(ReEvaluation::CReEvaluator& reeval, const CString& fileName);

    /** Writes the supplied re-evaluation settings to a file.
            @param reeval - the settings to be written to file
            @param fileName - the name and path of the file to which to write
            @param overWrite - if true the file will be overwritten, if false, the file will be appended */
    RETURN_CODE WriteSettings(const ReEvaluation::CReEvaluator& reeval, const CString& fileName, bool overWrite);

protected:

    /** Parses an intensity section of the file */
    RETURN_CODE Parse_IntensitySection(ReEvaluation::CReEvaluator& reeval, int type);

    /** Parses an sky-spectrum section of the file */
    RETURN_CODE Parse_SkySpecSection(ReEvaluation::CReEvaluator& reeval);

    /** Parses an dark-spectrum section of the file */
    RETURN_CODE Parse_DarkSpecSection(ReEvaluation::CReEvaluator& reeval);

};
}