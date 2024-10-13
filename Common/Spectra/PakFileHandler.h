#pragma once

#include <string>
#include <vector>
#include "../../Common/Common.h"
#include <SpectralEvaluation/Log.h>

namespace novac
{
class CScanFileHandler;
class CSpectrum;
class CSpectrumIO;
}

namespace FileHandler
{
/** The <b>CPakFileHandler</b> takes care of the downloaded pak-files from the scanning instrument
    and splits them up into several pak-files, each containing the data from one single scan. */
class CPakFileHandler
{
public:
    CPakFileHandler(novac::ILogger& log);

    // ----------------------------------------------------------------------
    // ---------------------- PUBLIC DATA -----------------------------------
    // ----------------------------------------------------------------------


    // ----------------------------------------------------------------------
    // --------------------- PUBLIC METHODS ---------------------------------
    // ----------------------------------------------------------------------

    /** This function reads a downloaded pak-file and checks it for errors.
        If all is ok, the scans in the file are evaluated.
        @param fileName - the name of the .pak-file to split
        @param deletePakFile - if true, the .pak-file will be deleted when this function returns.
        @param evaluate - if true the scan-files will be evaluated. This is default
        @param outputDir - if not null, the spectra will be put in this directory
            instead of default which is the directory in g_settings.m_outputDirectory.
        @return 1 if anything in the file is wrong and it should be downloaded again.
        @return 0 if all is ok */
    int ReadDownloadedFile(const CString& fileName, bool deletePakFile = true, bool evaluate = true, const CString* outputDir = NULL);

    /** This function checks the contents of the .pak-file 'fileName'
            and returns the type of measurement which is inside the file */
    static MEASUREMENT_MODE GetMeasurementMode(const CString& fileName);

    /** Adjusts the channel number to be in the range 0 - MAX_CHANNEL_NUM
            @return true if the spectrum is a multichannel spectrum and should be split. */
    static bool CorrectChannelNumber(unsigned char& channel);

private:
    // ----------------------------------------------------------------------
    // ---------------------- PRIVATE DATA ----------------------------------
    // ----------------------------------------------------------------------

    novac::ILogger& m_log;

    /** The size of the buffer for recieving the header data */
    static const int HEADER_BUF_SIZE = 16384;

    /** The filename of the last temporary scan-file created. */
    long    m_tempIndex;

    /** The temporary directory */
    CString m_tempDir;

    /** The directory for 'lost' spectra */
    CString m_lostDir;

    /** The directory for incomplete spectra */
    CString m_incompleteDir;

    /** Where to temporarily copy the spectra while the pak-file is being splitted. */
    CString m_scanFile[MAX_CHANNEL_NUM];

    /** The number of spectra that we've read so far in the checked file.
            When 'ReadDownloadedFile' returns, this is the number of spectra
            there were in that file. */
    long m_spectrumNumber;

    /** True if the output directories have been initialized */
    bool  m_initializedOutput;

    /** An array with all the serial numbers that have passed through
            this instance of CPakFileHandler*/
    std::vector<std::string> m_serialNumbers;

    /** The index for the last 'lost' file. */
    std::vector<int> m_lastLostIndex;

    // ----------------------------------------------------------------------
    // --------------------- PRIVATE METHODS --------------------------------
    // ----------------------------------------------------------------------

    /** Sets up the directories according to the settings in 'g_settings' */
    void InitializeDirectories(const CString& serialNumber, const CString* outputDir = NULL);

    /** Finds the first spectrum in the checked file, which is the first
            spectrum of a scan.
            @param reader - The CSpectrumIO object which takes care of this .pak-file
            @param fileForLost - The spectra before the first scan-start will be outputed
                to this filem if fileForLost is NULL then the spectra will not be saved. */
    int FindFirstScanStart(const CString& fileName, const CString& fileForLost);

    /** Finds the next spectrum in the already opened file, which is the first
            spectrum of a scan. The ignored spectra are saved in the 'incomplete' folder.
            On succesful return the function will return SUCCESS and the provided
            spectrum 'curSpec' will be the next first spectrum of a scan.
            If there is no more scan-start spectrum in the file, this function will
            return FAIL.
            This function alters the member variable 'm_spectrumNumber' */
    RETURN_CODE FindNextScanStart(FILE* file, novac::CSpectrum& curSpec);

    /** Sends a message to the evaluation thread that this scan-file should
            be evaluated. The file will first be moved to a temporary file
            so that nothing */
    RETURN_CODE EvaluateScan(const CString& fileName, const CString& serial);

    /** Looks up the index for the supplied serialNumber into the array of
            serialNumbers. If the serialNumber does not exist in the array, it will be
            inserted and it's index will be returned */
    int	GetSpectrometerIndex(const CString& serialNumber);

    /** Takes a scan file and renames it to an approprate name */
    RETURN_CODE	ArchiveScan(const CString& scanFileName);

    /** Takes appropriate action when a corrupt spectrum has been found */
    RETURN_CODE HandleCorruptSpectrum(novac::CSpectrumIO& reader, FILE* pakFile);

    /** Saves a newly found corrupted spectrum into the appropriate folder */
    RETURN_CODE SaveCorruptSpectrum(const novac::CSpectrum& curSpec, int specHeaderSie, const char* spectrumHeader);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a fixed angle measurement mode.
            @return false - if the file does not contain spectra,
                    or contains spectra which are not collected in a fixed angle measurement mode. */
    static bool IsFixedAngleMeasurement(novac::CScanFileHandler& file);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a wind speed measurement mode.
            @return false - if the file does not contain spectra,
                    or contains spectra which are not collected in a wind speed measurement mode. */
    static bool IsWindSpeedMeasurement(novac::CScanFileHandler& file);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a stratospheric measurement mode.
            @return false - if the file does not contain spectra,
                    or contains spectra which are not collected in a stratospheric measurement mode. */
    static bool IsStratosphericMeasurement(novac::CScanFileHandler& file);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a direct-sun mode.
            @return false - if the file does not contain spectra,
                    or contains spectra which are not collected in a direct-sun measurement mode. */
    static bool IsDirectSunMeasurement(novac::CScanFileHandler& file);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a lunar mode.
            @return false - if the file does not contain spectra,
                    or contains spectra which are not collected in a lunar measurement mode. */
    static bool IsLunarMeasurement(novac::CScanFileHandler& file);

    /** This function checks the contents of the opened scan file.
            @return true - if the spectra are collected in a composition measurment - mode.
            @return false - if the file does not contain readable spectra,
                    or contains spectra which are not collected in a composition measurment - mode.*/
    static bool IsCompositionMeasurement(novac::CScanFileHandler& file);

};

}