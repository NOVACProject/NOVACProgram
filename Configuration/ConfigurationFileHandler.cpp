#include "StdAfx.h"
#include "ConfigurationFileHandler.h"
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>
#include "../VolcanoInfo.h"
#include "../resource.h"

using namespace novac;

extern CVolcanoInfo g_volcanoes;

namespace FileHandler
{

CConfigurationFileHandler::CConfigurationFileHandler()
    : conf(nullptr), curScanner(nullptr)
{}

CConfigurationFileHandler::~CConfigurationFileHandler(void)
{
    curScanner = nullptr;
    conf = nullptr;
}

static CString GetConfigurationFileName(const CString* optionalFilename)
{
    CString filename;

    if (optionalFilename == nullptr)
    {
        Common common;
        common.GetExePath();

        filename.Format("%sconfiguration.xml", (LPCSTR)common.m_exePath);
    }
    else
    {
        filename.Format("%s", (LPCSTR)*optionalFilename);
    }

    return filename;
}

static CString GetFtpLoginFilename(const CString* optionalFilename)
{
    CString filename;

    if (optionalFilename == nullptr)
    {
        Common common;
        common.GetExePath();

        filename.Format("%sftplogin.xml", (LPCSTR)common.m_exePath);
    }
    else
    {
        filename.Format("%s", (LPCSTR)*optionalFilename);
    }

    return filename;

}

void CConfigurationFileHandler::ReadConfigurationFile(CConfigurationSetting& configuration, const CString* fileName)
{
    this->conf = &configuration;

    // 1. Get the filename
    const CString tmp_fileName = GetConfigurationFileName(fileName);

    // 2. Open the file
    CStdioFile file;
    CFileException exceFile;
    if (!file.Open(tmp_fileName, CFile::modeRead | CFile::typeText, &exceFile))
    {
        this->conf = nullptr;
        throw novac::FileIoException("Could not open configuration file '" + std::string((LPCSTR)tmp_fileName) + "' for reading.");
    }

    this->SetFile(&file);

    // 2b. Reset the current configuration
    conf->scannerNum = 0;

    // 3. Parse the file
    if (Parse())
    {
        // error in parsing
        conf = nullptr;
        curScanner = nullptr;
        Close();
    }
    else
    {
        // parsing was ok
        Close();
        conf = nullptr;
        curScanner = nullptr;
        CheckSettings(configuration);
    }

    return;
}

void CConfigurationFileHandler::ReadFtpLoginConfigurationFile(CConfigurationSetting& configuration, const CString* fileName)
{
    CFileException exceFile;
    CStdioFile file;

    // 1. Get the filename
    const CString tmp_fileName = GetFtpLoginFilename(fileName);

    // 2. Open the file
    if (!file.Open(tmp_fileName, CFile::modeRead | CFile::typeText, &exceFile))
    {
        // File not found, this is normal at first startup
        return;
    }
    this->SetFile(&file);

    while (szToken = NextToken())
    {
        if (Equals(szToken, "ftpUserName"))
        {
            Parse_StringItem(TEXT("/ftpUserName"), configuration.ftpSetting.userName);
            continue;
        }
        if (Equals(szToken, "ftpPassword"))
        {
            Parse_StringItem(TEXT("/ftpPassword"), configuration.ftpSetting.password);
            continue;
        }
    }

    Close();
}

void CConfigurationFileHandler::WriteFtpLoginConfigurationFile(const CConfigurationSetting& configuration, const CString* fileName) const
{
    CString indent, str;
    Common common;

    // 1. Get the filename
    const CString tmp_fileName = GetFtpLoginFilename(fileName);

    // 2. Open the file
    FILE* f = fopen(tmp_fileName, "w");
    if (nullptr == f)
    {
        throw ConfigurationFileException(common.GetString(ERROR_COULD_NOT_OPEN_EVALCONFIG) + "." + common.GetString(MSG_NO_CHANGES_WILL_BE_SAVED));
    }
    // , common.GetString(MSG_ERROR), MB_OK
    // 
        // 3. Write the header information 
    fprintf(f, TEXT("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"));
    fprintf(f, TEXT("<!-- This is the configuration file for the FTP login to the NOVAC server.\n Keep this file in a secure location and do not share with others. -->\n\n"));
    fprintf(f, TEXT("<FtpLogin>\n"));

    // 4d. The ftp server setting - username
    str.Format("\t<ftpUserName>%s</ftpUserName>\n", (LPCSTR)configuration.ftpSetting.userName);
    fprintf(f, str);
    // 4e. The ftp server setting - password
    str.Format("\t<ftpPassword>%s</ftpPassword>\n", (LPCSTR)configuration.ftpSetting.password);
    fprintf(f, str);

    fprintf(f, TEXT("</FtpLogin>"));

    fclose(f);
}

void CConfigurationFileHandler::WriteConfigurationFile(const CConfigurationSetting& configuration, const CString* fileName) const
{
    Common common;

    // 1. Get the filename
    const CString tmp_fileName = GetConfigurationFileName(fileName);

    // 2. Open the file
    FILE* f = fopen(tmp_fileName, "w");
    if (nullptr == f)
    {
        throw ConfigurationFileException(common.GetString(ERROR_COULD_NOT_OPEN_EVALCONFIG) + "." + common.GetString(MSG_NO_CHANGES_WILL_BE_SAVED));
    }

    // 3. Write the header information 
    fprintf(f, TEXT("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"));
    fprintf(f, TEXT("<!-- This is the configuration file for the evaluation of spectra in the NOVAC project.\n 	All spectrometers that are used must be defined here or their spectra will NOT be evaluated -->\n\n"));
    fprintf(f, TEXT("<Configuration>\n"));

    // 4. Write the global information

    // 4a. The output directory
    if (strlen(configuration.outputDirectory) > 0)
    {
        fprintf(f, TEXT("\t<outputDir>") + configuration.outputDirectory + TEXT("</outputDir>\n"));
    }

    // 4b. The startup method
    CString str;
    str.Format("\t<startup>%d</startup>\n", configuration.startup);
    fprintf(f, str);
    // 4c. The data-upload server setting - address
    str.Format("\t<ftpAddress>%s</ftpAddress>\n", (LPCSTR)configuration.ftpSetting.ftpAddress);
    fprintf(f, str);
    // 4d. The data-upload setting - protocol
    str.Format("\t<ftpProtocol>%s</ftpProtocol>\n", (LPCSTR)configuration.ftpSetting.protocol);
    fprintf(f, str);
    // 4e. The settings for when to upload to the FTP-server...
    str.Format("\t<ftpStartTime>%d</ftpStartTime>\n", configuration.ftpSetting.ftpStartTime);
    fprintf(f, str);
    str.Format("\t<ftpStopTime>%d</ftpStopTime>\n", configuration.ftpSetting.ftpStopTime);
    fprintf(f, str);


    // 4f. Write if we should publish results
    if (configuration.webSettings.publish)
    {
        if (strlen(configuration.webSettings.localDirectory) > 0)
        {
            str.Format("\t<publish>1</publish>\n");
            str.AppendFormat("\t<localPublishDirectory>%s</localPublishDirectory>\n", (LPCSTR)configuration.webSettings.localDirectory);
            fprintf(f, str);
        }
        str.Format("\t<publishFormat>%s</publishFormat>\n", (LPCSTR)configuration.webSettings.imageFormat);
        fprintf(f, str);
    }

    // 4g. Write if we should call external softwares
    if (strlen(configuration.externalSetting.fullScanScript) > 1)
    {
        str.Format("\t<fullScanExecute>%s</fullScanExecute>\n", (LPCSTR)configuration.externalSetting.fullScanScript);
        fprintf(f, str);
    }
    if (strlen(configuration.externalSetting.imageScript) > 1)
    {
        str.Format("\t<imageExecute>%s</imageExecute>\n", (LPCSTR)configuration.externalSetting.imageScript);
        fprintf(f, str);
    }

    // 4h. If there is a wind-field file which should be re-read every now and then
    if (configuration.windSourceSettings.windFieldFile.GetLength() > 3)
    {
        str.Format("\t<windImport>\n");
        str.AppendFormat("\t\t<path>%s</path>\n", (LPCSTR)configuration.windSourceSettings.windFieldFile);
        str.AppendFormat("\t\t<reloadInterval>%d</reloadInterval>\n", configuration.windSourceSettings.windFileReloadInterval);
        str.AppendFormat("\t\t<enabled>%d</enabled>\n", configuration.windSourceSettings.enabled);
        str.AppendFormat("\t</windImport>\n");
        fprintf(f, str);
    }

    // 5. Begin the device list
    fprintf(f, TEXT("\t<deviceList>\n"));

    // 6. Write the information about the scanning Instruments

    bool hasDoubleSpectrometer = false;
    for (unsigned int scannerIdx = 0; scannerIdx < configuration.scannerNum; ++scannerIdx)
    {
        fprintf(f, "\t\t<scanningInstrument>\n");

        CString indent = "\t\t\t";

        const auto& currentScanner = configuration.scanner[scannerIdx]; // shorter notation, for better readability below.

        // observatory
        fprintf(f, indent + "<observatory>" + currentScanner.observatory + "</observatory>\n");

        // If the user has configured a volcano, then also write its additional information
        bool ownerConfiguredSource = false;
        for (unsigned int k = g_volcanoes.m_preConfiguredVolcanoNum; k < g_volcanoes.m_volcanoNum; ++k)
        {
            if (Equals(g_volcanoes.m_name[k], currentScanner.volcano))
            {
                fprintf(f, TEXT("\t\t\t<sourceInfo>\n"));
                str.Format("\t\t\t\t<name>%s</name>\n", (LPCSTR)g_volcanoes.m_name[k]);
                str.AppendFormat("\t\t\t\t<latitude>%lf</latitude>\n", g_volcanoes.m_peakLatitude[k]);
                str.AppendFormat("\t\t\t\t<longitude>%lf</longitude>\n", g_volcanoes.m_peakLongitude[k]);
                str.AppendFormat("\t\t\t\t<altitude>%lf</altitude>\n", g_volcanoes.m_peakHeight[k]);
                fprintf(f, str);
                fprintf(f, TEXT("\t\t\t</sourceInfo>\n"));
                ownerConfiguredSource = true;
                break;
            }
        }
        if (!ownerConfiguredSource)
        {
            // volcano
            fprintf(f, indent + "<volcano>" + currentScanner.volcano + "</volcano>\n");
        }

        // site
        fprintf(f, indent + "<site>" + currentScanner.site + "</site>\n");

        // position
        str.Format("%s<instr_latitude>%lf</instr_latitude>\n", indent, currentScanner.gps.m_latitude);
        str.AppendFormat("%s<instr_longitude>%lf</instr_longitude>\n", indent, currentScanner.gps.m_longitude);
        str.AppendFormat("%s<instr_altitude>%lf</instr_altitude>\n", indent, currentScanner.gps.m_altitude);
        str.AppendFormat("%s<instr_compass>%lf</instr_compass>\n", indent, currentScanner.compass);
        fprintf(f, str);

        // electronicsBox
        str.Format("%s<electronics>%d</electronics>\n", indent, currentScanner.electronicsBox);
        fprintf(f, str);

        // plot options
        str.Format("%s<plotColumn>%d</plotColumn>\n", indent, currentScanner.plotColumn);
        str.AppendFormat("%s<plotColumnHistory>%d</plotColumnHistory>\n", indent, currentScanner.plotColumnHistory);
        str.AppendFormat("%s<minColumn>%d</minColumn>\n", indent, currentScanner.minColumn);
        str.AppendFormat("%s<maxColumn>%d</maxColumn>\n", indent, currentScanner.maxColumn);
        str.AppendFormat("%s<plotFluxHistory>%d</plotFluxHistory>\n", indent, currentScanner.plotFluxHistory);
        str.AppendFormat("%s<minFlux>%d</minFlux>\n", indent, currentScanner.minFlux);
        str.AppendFormat("%s<maxFlux>%d</maxFlux>\n", indent, currentScanner.maxFlux);
        fprintf(f, str);

        // --- First write the spectrometer information
        for (unsigned int spectrometerIdx = 0; spectrometerIdx < currentScanner.specNum; ++spectrometerIdx)
        {
            const CConfigurationSetting::SpectrometerSetting& spec = currentScanner.spec[spectrometerIdx];

            fprintf(f, indent + "<spectrometer>\n");

            // serial Number
            str.Format("%s<serialNumber>%s</serialNumber>\n", (LPCSTR)indent, (LPCSTR)spec.serialNumber);
            fprintf(f, str);

            // model Number
            {
                const SpectrometerModel spectrometerModel = CSpectrometerDatabase::GetInstance().GetModel(spec.modelName);
                if (spectrometerModel.isCustom)
                {
                    str.Format("%s<customSpectrometer>\n", (LPCSTR)indent);
                    str.AppendFormat("%s\t<name>%s</name>\n", (LPCSTR)indent, spectrometerModel.modelName.c_str());
                    str.AppendFormat("%s\t<maxIntensity>%lf</maxIntensity>\n", (LPCSTR)indent, spectrometerModel.maximumIntensityForSingleReadout);

                    if (spectrometerModel.numberOfChannels != 1)
                    {
                        str.AppendFormat("%s\t<numChannels>%d</numChannels>\n", (LPCSTR)indent, spectrometerModel.numberOfChannels);
                    }

                    str.AppendFormat("%s</customSpectrometer>\n", (LPCSTR)indent);
                }
                else
                {
                    str.Format("%s<model>%s</model>\n", (LPCSTR)indent, spec.modelName.c_str());
                }
                fprintf(f, str);
            }

            if (currentScanner.spec[spectrometerIdx].channelNum > 1)
                hasDoubleSpectrometer = true;

            // The channels
            for (unsigned int channelIdx = 0; channelIdx < currentScanner.spec[spectrometerIdx].channelNum; ++channelIdx)
            {
                str.Format("%s<channel number=\"%d\">\n", (LPCSTR)indent, channelIdx);
                fprintf(f, str);

                // The species
                const novac::CFitWindow& window = spec.channel[channelIdx].fitWindow;
                for (const auto& reference : window.reference)
                {
                    fprintf(f, TEXT(indent + "\t<Reference>\n"));
                    fprintf(f, "%s\t\t<name>%s</name>\n", (LPCSTR)indent, reference.m_specieName.c_str());
                    fprintf(f, "%s\t\t<path>%s</path>\n", (LPCSTR)indent, reference.m_path.c_str());
                    // writing the shift
                    fprintf(f, TEXT(indent + "\t\t<shift>"));
                    if (reference.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FIX)
                        str.Format("fix to %.2lf", reference.m_shiftValue);
                    else if (reference.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FREE)
                        str.Format("free");
                    else if (reference.m_shiftOption == novac::SHIFT_TYPE::SHIFT_LIMIT)
                        str.Format("limit from %.2lf to %.2lf", reference.m_shiftValue, reference.m_shiftMaxValue);
                    else if (reference.m_shiftOption == novac::SHIFT_TYPE::SHIFT_LINK)
                        str.Format("link to %.0lf", reference.m_shiftValue);
                    fprintf(f, str);
                    fprintf(f, "</shift>\n");
                    // writing the squeeze
                    fprintf(f, TEXT(indent + "\t\t<squeeze>"));
                    if (reference.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FIX)
                        str.Format("fix to %.2lf", reference.m_squeezeValue);
                    else if (reference.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FREE)
                        str.Format("free");
                    else if (reference.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LIMIT)
                        str.Format("limit from %.2lf to %.2lf", reference.m_squeezeValue, reference.m_squeezeMaxValue);
                    else if (reference.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LINK)
                        str.Format("link to %.0lf", reference.m_squeezeValue);
                    fprintf(f, str);
                    fprintf(f, "</squeeze>\n");
                    fprintf(f, TEXT(indent + "\t</Reference>\n"));
                }

                // fitLow
                str.Format("\t%s<fitLow>%ld</fitLow>\n", (LPCSTR)indent, window.fitLow);
                fprintf(f, str);

                // fitHigh
                str.Format("\t%s<fitHigh>%ld</fitHigh>\n", (LPCSTR)indent, window.fitHigh);
                fprintf(f, str);

                // The options for the dark-current
                if (spec.channel[channelIdx].darkSettings.m_darkSpecOption != Configuration::DARK_SPEC_OPTION::MEASURED_IN_SCAN)
                {
                    const auto& darkSettings = spec.channel[channelIdx].darkSettings;

                    // How to get the dark-spectrum
                    str.Format("\t%s<dark>%d</dark>\n", (LPCSTR)indent, (int)darkSettings.m_darkSpecOption);
                    fprintf(f, str);

                    // How to use the offset spectrum
                    str.Format("\t%s<offsetOption>%d</offsetOption>\n", (LPCSTR)indent, (int)darkSettings.m_offsetOption);
                    fprintf(f, str);

                    // The path of the offset-spectrum
                    if (darkSettings.m_offsetSpec.size() > 1)
                    {
                        str.Format("\t%s<offsetPath>%s</offsetPath>\n", (LPCSTR)indent, darkSettings.m_offsetSpec.c_str());
                        fprintf(f, str);
                    }

                    // How to use the dark-current spectrum
                    str.Format("\t%s<darkcurrentOption>%d</darkcurrentOption>\n", (LPCSTR)indent, (int)darkSettings.m_darkCurrentOption);
                    fprintf(f, str);

                    // The path of the dark-current spectrum
                    if (darkSettings.m_darkCurrentSpec.size() > 1)
                    {
                        str.Format("\t%s<darkCurrentPath>%s</darkCurrentPath>\n", (LPCSTR)indent, darkSettings.m_darkCurrentSpec.c_str());
                        fprintf(f, str);
                    }
                }

                // The options for automatic (re)calibration.
                if (spec.channel[channelIdx].autoCalibration.solarSpectrumFile.GetLength() > 0)
                {
                    const auto& calibrationSettings = spec.channel[channelIdx].autoCalibration;
                    const auto previousIndent = indent;
                    indent = indent + "\t";

                    str.Format("%s<calibration>\n", (LPCSTR)indent);

                    str.AppendFormat("\t%s<enable>%d</enable>\n", (LPCSTR)indent, calibrationSettings.enable);
                    str.AppendFormat("\t%s<generateReferences>%d</generateReferences>\n", (LPCSTR)indent, calibrationSettings.generateReferences);
                    str.AppendFormat("\t%s<filterReferences>%d</filterReferences>\n", (LPCSTR)indent, calibrationSettings.filterReferences);

                    str.AppendFormat("\t%s<intervalHours>%d</intervalHours>\n", (LPCSTR)indent, calibrationSettings.intervalHours);
                    str.AppendFormat("\t%s<intervalTimeOfDayLow>%d</intervalTimeOfDayLow>\n", (LPCSTR)indent, calibrationSettings.intervalTimeOfDayLow);
                    str.AppendFormat("\t%s<intervalTimeOfDayHigh>%d</intervalTimeOfDayHigh>\n", (LPCSTR)indent, calibrationSettings.intervalTimeOfDayHigh);

                    str.AppendFormat("\t%s<solarSpectrumFile>%s</solarSpectrumFile>\n", (LPCSTR)indent, calibrationSettings.solarSpectrumFile);

                    if (calibrationSettings.initialCalibrationFile.GetLength() > 0)
                    {
                        str.AppendFormat("\t%s<initialCalibrationFile>%s</initialCalibrationFile>\n", (LPCSTR)indent, calibrationSettings.initialCalibrationFile);
                    }
                    if (calibrationSettings.instrumentLineshapeFile.GetLength() > 0)
                    {
                        str.AppendFormat("\t%s<instrumentLineshapeFile>%s</instrumentLineshapeFile>\n", (LPCSTR)indent, calibrationSettings.instrumentLineshapeFile);
                    }
                    str.AppendFormat("\t%s<initialCalibrationType>%d</initialCalibrationType>\n", (LPCSTR)indent, calibrationSettings.initialCalibrationType);
                    str.AppendFormat("\t%s<instrumentLineShapeFitOption>%d</instrumentLineShapeFitOption>\n", (LPCSTR)indent, calibrationSettings.instrumentLineShapeFitOption);

                    str.AppendFormat("\t%s<instrumentLineShapeFitRegionLow>%lf</instrumentLineShapeFitRegionLow>\n", (LPCSTR)indent, calibrationSettings.instrumentLineShapeFitRegion.low);
                    str.AppendFormat("\t%s<instrumentLineShapeFitRegionHigh>%lf</instrumentLineShapeFitRegionHigh>\n", (LPCSTR)indent, calibrationSettings.instrumentLineShapeFitRegion.high);

                    str.AppendFormat("%s</calibration>\n", (LPCSTR)indent);
                    fprintf(f, str);

                    indent = previousIndent;
                }

                // End of channel section
                str.Format("%s</channel>\n", (LPCSTR)indent);
                fprintf(f, str);
            }

            // End of spectrometer section
            fprintf(f, TEXT("\t\t\t</spectrometer>\n"));
        }

        // --- Then write the communication information
        {
            const CConfigurationSetting::CommunicationSetting& comm = currentScanner.comm;
            fprintf(f, TEXT("\t\t\t<communication>\n"));

            // connection type
            str.Format("%s<connection>%d</connection>\n", (LPCSTR)indent, comm.connectionType);
            fprintf(f, str);

            if (comm.connectionType == SERIAL_CONNECTION)
            {
                // port
                str.Format("%s<port>%d</port>\n", (LPCSTR)indent, comm.port);
                fprintf(f, str);

                // baudrate
                str.Format("%s<baudrate>%d</baudrate>\n", (LPCSTR)indent, comm.baudrate);
                fprintf(f, str);

                // flowControl
                str.Format("%s<flowControl>%d</flowControl>\n", (LPCSTR)indent, comm.flowControl);
                fprintf(f, str);

                // The communication medium
                str.Format("%s<medium>", (LPCSTR)indent);
                switch (comm.medium)
                {
                case MEDIUM_CABLE: str.AppendFormat("Cable"); break;
                case MEDIUM_FREEWAVE_SERIAL_MODEM: str.AppendFormat("Freewave"); break;
                case MEDIUM_SATTELINE_SERIAL_MODEM: str.AppendFormat("Satteline"); break;
                }
                str.AppendFormat("</medium>\n");
                fprintf(f, str);

                // radioID / callbook number
                str.Format("%s<radioID>%s</radioID>\n", (LPCSTR)indent, (LPCSTR)comm.radioID);
                fprintf(f, str);

            }

            if (comm.connectionType == FTP_CONNECTION)
            {
                // IP-address of the scanning system
                // str.Format("%s<IP>%d.%d.%d.%d</IP>\n", (LPCSTR)indent, comm.ftpIP[0], comm.ftpIP[1], comm.ftpIP[2], comm.ftpIP[3]);
                str.Format("%s<host>%s</host>\n", (LPCSTR)indent, comm.ftpHostName);
                fprintf(f, str);

                // ftp port, if not default (21)
                if (comm.ftpPort != 21)
                {
                    str.Format("%s<ftpPort>%d</ftpPort>\n", (LPCSTR)indent, comm.ftpPort);
                    fprintf(f, str);
                }

                // user name to log in on the scanning system
                str.Format("%s<username>%s</username>\n", (LPCSTR)indent, (LPCSTR)comm.ftpUserName);
                fprintf(f, str);

                // password to log in on the scanning system
                str.Format("%s<password>%s</password>\n", (LPCSTR)indent, (LPCSTR)comm.ftpPassword);
                fprintf(f, str);
            }

            if (comm.connectionType == DIRECTORY_POLLING)
            {
                str.Format("%s<directory>%s</directory>\n", (LPCSTR)indent, (LPCSTR)comm.directory);
                fprintf(f, str);
            }

            // timeout
            str.Format("%s<timeout>%d</timeout>\n", (LPCSTR)indent, comm.timeout);
            fprintf(f, str);

            // query period
            str.Format("%s<queryPeriod>%d</queryPeriod>\n", (LPCSTR)indent, comm.queryPeriod);
            fprintf(f, str);
            // sleep time
            str.Format("%s<sleepTime>%d</sleepTime>\n", (LPCSTR)indent,
                comm.sleepTime.hour * 3600 + comm.sleepTime.minute * 60 + comm.sleepTime.second);
            fprintf(f, str);
            //wake up time
            str.Format("%s<wakeupTime>%d</wakeupTime>\n", (LPCSTR)indent,
                comm.wakeupTime.hour * 3600 + comm.wakeupTime.minute * 60 + comm.wakeupTime.second);
            fprintf(f, str);

            fprintf(f, TEXT("\t\t\t</communication>\n"));
        }

        // .. Third: write the wind measurement settings, if applicable
        bool doWindMeasurements = (hasDoubleSpectrometer && currentScanner.windSettings.automaticWindMeasurements);
        if (doWindMeasurements)
        {
            indent.Format("\t\t\t\t");
            fprintf(f, TEXT("\t\t\t<windmeasurement>\n"));

            str.Format("%s<interval>%d</interval>\n", (LPCSTR)indent, currentScanner.windSettings.interval);
            fprintf(f, str);

            str.Format("%s<duration>%d</duration>\n", (LPCSTR)indent, currentScanner.windSettings.duration);
            fprintf(f, str);

            str.Format("%s<maxangle>%.2lf</maxangle>\n", (LPCSTR)indent, currentScanner.windSettings.maxAngle);
            fprintf(f, str);

            str.Format("%s<stableperiod>%d</stableperiod>\n", (LPCSTR)indent, currentScanner.windSettings.stablePeriod);
            fprintf(f, str);

            str.Format("%s<minpeakcolumn>%.2lf</minpeakcolumn>\n", (LPCSTR)indent, currentScanner.windSettings.minPeakColumn);
            fprintf(f, str);

            str.Format("%s<desiredAngle>%.1lf</desiredAngle>\n", (LPCSTR)indent, currentScanner.windSettings.desiredAngle);
            fprintf(f, str);

            str.Format("%s<useCalcWind>%d</useCalcWind>\n", (LPCSTR)indent, currentScanner.windSettings.useCalculatedPlumeParameters);
            fprintf(f, str);

            str.Format("%s<switchRange>%.1lf</switchRange>\n", (LPCSTR)indent, currentScanner.windSettings.SwitchRange);
            fprintf(f, str);

            fprintf(f, TEXT("\t\t\t</windmeasurement>\n"));
        }

        fprintf(f, TEXT("\t\t</scanningInstrument>\n"));

    }

    // Write the ending lines
    fprintf(f, TEXT("\t</deviceList>\n"));
    fprintf(f, TEXT("</Configuration>"));

    fclose(f);
}

int CConfigurationFileHandler::Parse()
{

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // this can safely be ignored
        if (Equals(szToken, "deviceList"))
        {
            continue;
        }
        if (Equals(szToken, "Configuration"))
        {
            continue;
        }

        // ----------------------------------------------------
        // ---------------- Global Settings -------------------
        // ----------------------------------------------------
        if (Equals(szToken, "outputDir"))
        {
            Parse_OutputDir();
            continue;
        }

        if (Equals(szToken, "startup"))
        {
            Parse_IntItem(TEXT("/startup"), conf->startup);
            continue;
        }

        if (Equals(szToken, "ftpAddress"))
        {
            Parse_StringItem(TEXT("/ftpAddress"), conf->ftpSetting.ftpAddress);
            continue;
        }
        if (Equals(szToken, "ftpProtocol"))
        {
            CString protocol;
            Parse_StringItem(TEXT("/ftpProtocol"), protocol);
            conf->ftpSetting.protocol = (Equals(protocol, "FTP")) ? "FTP" : "SFTP";
            continue;
        }
        if (Equals(szToken, "ftpStartTime"))
        {
            Parse_IntItem(TEXT("/ftpStartTime"), conf->ftpSetting.ftpStartTime);
            conf->ftpSetting.ftpStartTime = abs(conf->ftpSetting.ftpStartTime);
            continue;
        }
        if (Equals(szToken, "ftpStopTime"))
        {
            Parse_IntItem(TEXT("/ftpStopTime"), conf->ftpSetting.ftpStopTime);
            conf->ftpSetting.ftpStopTime = abs(conf->ftpSetting.ftpStopTime);
            continue;
        }

        if (Equals(szToken, "publishFormat"))
        {
            Parse_StringItem(TEXT("/publishFormat"), conf->webSettings.imageFormat);
            if (!Equals(conf->webSettings.imageFormat, ".png") && !Equals(conf->webSettings.imageFormat, ".bmp") && !Equals(conf->webSettings.imageFormat, ".gif") && !Equals(conf->webSettings.imageFormat, ".jpg"))
            {
                conf->webSettings.imageFormat.Format(".png");
            }
            continue;
        }
        if (Equals(szToken, "publish"))
        {
            Parse_IntItem(TEXT("/publish"), conf->webSettings.publish);
            continue;
        }
        if (Equals(szToken, "localPublishDirectory"))
        {
            this->Parse_LocalPublishDir();
            continue;
        }

        if (Equals(szToken, "fullScanExecute"))
        {
            this->Parse_StringItem(TEXT("/fullScanExecute"), conf->externalSetting.fullScanScript);
        }
        if (Equals(szToken, "imageExecute"))
        {
            this->Parse_StringItem(TEXT("/imageExecute"), conf->externalSetting.imageScript);
        }

        if (Equals(szToken, "windImport"))
        {
            this->Parse_WindImport();
        }

        // -----------------------------------------------------
        // ------------- Scanning Instrument Settings ----------
        // -----------------------------------------------------

        // starting a new 'scanningInstrument' section
        if (Equals(szToken, "scanningInstrument"))
        {
            this->curScanner = &(conf->scanner[conf->scannerNum]);
            Parse_ScanningInstrument();
        }

    }

    return 0;
}

int CConfigurationFileHandler::Parse_ScanningInstrument()
{
    int tmpInt;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the scanningInstrument section
        if (Equals(szToken, "/scanningInstrument"))
        {
            ++conf->scannerNum;
            return 0;
        }

        // a communication section
        if (Equals(szToken, "communication"))
        {
            Parse_Communication();
        }

        // a spectrometer section
        if (Equals(szToken, "spectrometer"))
        {
            Parse_Spectrometer();
        }

        // a windmeasurement section
        if (Equals(szToken, "windmeasurement"))
        {
            Parse_WindMeasurement();
        }

        // a real-time changing of cfg.txt section
        if (Equals(szToken, "realtimesetup"))
        {
            this->Parse_RealTimeSetup();
        }

        // an observatory name for the scanning instrument
        if (Equals(szToken, "observatory"))
        {
            Parse_StringItem(TEXT("/observatory"), curScanner->observatory);
            continue;
        }

        // a volcano name for the scanning instrument
        if (Equals(szToken, "volcano"))
        {
            Parse_StringItem(TEXT("/volcano"), curScanner->volcano);
            continue;
        }

        // Additional information about the source
        if (Equals(szToken, "sourceInfo"))
        {
            Parse_SourceInfo();
            continue;
        }

        // a site name for the scanning instrument
        if (Equals(szToken, "site"))
        {
            Parse_StringItem(TEXT("/site"), curScanner->site);
            continue;
        }

        // the latitude for the scanning instrument
        if (Equals(szToken, "instr_latitude"))
        {
            Parse_FloatItem(TEXT("/instr_latitude"), curScanner->gps.m_latitude);
            continue;
        }
        // the longitude for the scanning instrument
        if (Equals(szToken, "instr_longitude"))
        {
            Parse_FloatItem(TEXT("/instr_longitude"), curScanner->gps.m_longitude);
            continue;
        }
        // the altitude for the scanning instrument
        if (Equals(szToken, "instr_altitude"))
        {
            Parse_FloatItem(TEXT("/instr_altitude"), curScanner->gps.m_altitude);
            continue;
        }
        // the compass-direction for the scanning instrument
        if (Equals(szToken, "instr_compass"))
        {
            Parse_FloatItem(TEXT("/instr_compass"), curScanner->compass);
            continue;
        }

        // the type of the electronics box
        if (Equals(szToken, "electronics"))
        {
            Parse_IntItem(TEXT("/electronics"), tmpInt);
            switch (tmpInt)
            {
            case 0: curScanner->electronicsBox = BOX_AXIS; break;
            case 2: curScanner->electronicsBox = BOX_AXIOMTEK; break; // yes this is actually correct...
            default: curScanner->electronicsBox = BOX_MOXA; break;
            }
            continue;
        }

        // plot options
        if (Equals(szToken, "plotColumn"))
        {
            Parse_IntItem(TEXT("/plotColumn"), curScanner->plotColumn);
            continue;
        }
        if (Equals(szToken, "plotColumnHistory"))
        {
            Parse_IntItem(TEXT("/plotColumnHistory"), curScanner->plotColumnHistory);
            continue;
        }
        if (Equals(szToken, "minColumn"))
        {
            Parse_IntItem(TEXT("/minColumn"), curScanner->minColumn);
            continue;
        }
        if (Equals(szToken, "maxColumn"))
        {
            Parse_IntItem(TEXT("/maxColumn"), curScanner->maxColumn);
            continue;
        }
        if (Equals(szToken, "plotFluxHistory"))
        {
            Parse_IntItem(TEXT("/plotFluxHistory"), curScanner->plotFluxHistory);
            continue;
        }
        if (Equals(szToken, "minFlux"))
        {
            Parse_IntItem(TEXT("/minFlux"), curScanner->minFlux);
            continue;
        }
        if (Equals(szToken, "maxFlux"))
        {
            Parse_IntItem(TEXT("/maxFlux"), curScanner->maxFlux);
            continue;
        }
    }
    return 0;
}

int CConfigurationFileHandler::Parse_Communication()
{
    CConfigurationSetting::CommunicationSetting* curComm = &(curScanner->comm);

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the communication section
        if (Equals(szToken, "/communication"))
        {
            return 0;
        }

        if (Equals(szToken, "connection"))
        {
            Parse_IntItem(TEXT("/connection"), curComm->connectionType);
            continue;
        }

        if (Equals(szToken, "port"))
        {
            Parse_LongItem(TEXT("/port"), curComm->port);
            continue;
        }

        if (Equals(szToken, "baudrate"))
        {
            Parse_LongItem(TEXT("/baudrate"), curComm->baudrate);
            continue;
        }

        if (Equals(szToken, "medium"))
        {
            Parse_Medium(curComm);
            continue;
        }

        if (Equals(szToken, "radioID"))
        {
            Parse_StringItem(TEXT("/radioID"), curComm->radioID);
            continue;
        }

        if (Equals(szToken, "IP"))
        {
            // Parse_IPNumber(TEXT("/IP"), curComm->ftpIP[0], curComm->ftpIP[1], curComm->ftpIP[2], curComm->ftpIP[3]);
            Parse_StringItem(TEXT("/IP"), curComm->ftpHostName); // Parsing the old 'IP' configuration makes sure that the transit from using IP to using host-names is smooth
            continue;
        }

        if (Equals(szToken, "host"))
        {
            Parse_StringItem(TEXT("/host"), curComm->ftpHostName); // Parsing the old 'IP' configuration makes sure that the transit from using IP to using host-names is smooth
            continue;
        }

        if (Equals(szToken, "ftpPort"))
        {
            Parse_IntItem(TEXT("/ftpPort"), curComm->ftpPort);
            continue;
        }

        if (Equals(szToken, "username"))
        {
            Parse_StringItem(TEXT("/username"), curComm->ftpUserName);
            continue;
        }

        if (Equals(szToken, "password"))
        {
            Parse_StringItem(TEXT("/password"), curComm->ftpPassword);
            continue;
        }

        if (Equals(szToken, "directory"))
        {
            Parse_StringItem(TEXT("/directory"), curComm->directory);
            continue;
        }

        if (Equals(szToken, "flowControl"))
        {
            Parse_IntItem(TEXT("/flowControl"), curComm->flowControl);
            continue;
        }

        if (Equals(szToken, "timeout"))
        {
            Parse_LongItem(TEXT("/timeout"), curComm->timeout);
            continue;
        }

        if (Equals(szToken, "queryPeriod"))
        {
            Parse_LongItem(TEXT("/queryPeriod"), curComm->queryPeriod);
            continue;
        }
        if (Equals(szToken, "sleepTime"))
        {
            long sleepT;
            Parse_LongItem(TEXT("/sleepTime"), sleepT);
            curComm->sleepTime.hour = sleepT / 3600;
            curComm->sleepTime.minute = (sleepT % 3600) / 60;
            curComm->sleepTime.second = (sleepT % 3600) % 60;

            curComm->sleepTime.hour = max(min(curComm->sleepTime.hour, 23), 0);
            curComm->sleepTime.minute = max(min(curComm->sleepTime.minute, 59), 0);
            curComm->sleepTime.second = max(min(curComm->sleepTime.second, 59), 0);
            continue;
        }
        if (Equals(szToken, "wakeupTime"))
        {
            long wakeupT;
            Parse_LongItem(TEXT("/wakeupTime"), wakeupT);
            curComm->wakeupTime.hour = wakeupT / 3600;
            curComm->wakeupTime.minute = (wakeupT % 3600) / 60;
            curComm->wakeupTime.second = (wakeupT % 3600) % 60;

            curComm->wakeupTime.hour = max(min(curComm->wakeupTime.hour, 23), 0);
            curComm->wakeupTime.minute = max(min(curComm->wakeupTime.minute, 59), 0);
            curComm->wakeupTime.second = max(min(curComm->wakeupTime.second, 59), 0);
            continue;
        }
    }

    return 0;
}

int CConfigurationFileHandler::Parse_Spectrometer()
{
    CConfigurationSetting::SpectrometerSetting* curSpec = &(curScanner->spec[curScanner->specNum]);

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the spectrometer section
        if (Equals(szToken, "/spectrometer"))
        {
            ++curScanner->specNum;
            return 0;
        }

        // found the serial number 
        if (Equals(szToken, "serialNumber"))
        {
            if (curSpec != nullptr)
                Parse_StringItem(TEXT("/serialNumber"), curSpec->serialNumber);
            continue;
        }

        // found a custom spectrometer
        if (Equals(szToken, "customSpectrometer"))
        {
            Parse_CustomSpectrometerModel(curSpec);
            continue;
        }

        // found the model number 
        if (Equals(szToken, "model"))
        {
            std::string tmpStr2;
            if (curSpec != nullptr)
            {
                Parse_StringItem(TEXT("/model"), tmpStr2);
                curSpec->modelName = tmpStr2;
            }
            continue;
        }

        if (Equals(szToken, "channel", 7))
        {
            if (curSpec != nullptr)
                Parse_Channel();
            continue;
        }

        // ---- This section is here for backward compatibility only ----------
        if (Equals(szToken, "specie", 6))
        {
            if (curSpec != nullptr)
            {
                Parse_Specie(&curSpec->channel[curSpec->channelNum]);
            }
            continue;
        }

        // ---- This section is here for backward compatibility only ----------
        if (Equals(szToken, "fitLow"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem("/fitLow", curSpec->channel[curSpec->channelNum].fitWindow.fitLow);
            }
            ++curSpec->channelNum;
            continue;
        }

        // ---- This section is here for backward compatibility only ----------
        if (Equals(szToken, "fitHigh"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem("/fitHigh", curSpec->channel[curSpec->channelNum].fitWindow.fitHigh);
            }
            continue;
        }
    }
    return 0;
}

int CConfigurationFileHandler::Parse_CustomSpectrometerModel(CConfigurationSetting::SpectrometerSetting* curSpec)
{
    if (nullptr == curSpec)
    {
        return 1;
    }

    SpectrometerModel thisModel;

    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the spectrometer section
        if (Equals(szToken, "/customSpectrometer"))
        {
            // Save the values
            curSpec->modelName = thisModel.modelName;

            const int currentModelIndex = CSpectrometerDatabase::GetInstance().GetModelIndex(thisModel.modelName);

            if (currentModelIndex >= 0)
            {
                SpectrometerModel existingModel = CSpectrometerDatabase::GetInstance().GetModel(currentModelIndex);

                if (std::abs(existingModel.maximumIntensityForSingleReadout - thisModel.maximumIntensityForSingleReadout) > 1.0)
                {
                    CString errorMessage;
                    errorMessage.Format("Could not configure new custom spectrometer model with name %s, such a model already exists with a different maximum intensity.", thisModel.modelName);
                    throw ConfigurationFileException(errorMessage);
                }

                // The model already exists in the database, but with the same properties, don't insert again into database.
            }
            else
            {
                // Insert the new model into the database
                CSpectrometerDatabase::GetInstance().AddModel(thisModel);
            }

            return 0;
        }

        // found the model name 
        if (Equals(szToken, "name"))
        {
            Parse_StringItem("/name", thisModel.modelName);
            continue;
        }

        // found the maximum intensity 
        if (Equals(szToken, "maxIntensity"))
        {
            Parse_FloatItem("/maxIntensity", thisModel.maximumIntensityForSingleReadout);
            continue;
        }

        // found the number of channels 
        if (Equals(szToken, "numChannels"))
        {
            Parse_IntItem("/numChannels", thisModel.numberOfChannels);
            continue;
        }
    }

    return 0;
}

int CConfigurationFileHandler::Parse_Channel()
{
    // TODO: this should really be cleaned up.
    CConfigurationSetting::SpectrometerSetting* curSpec = &(curScanner->spec[curScanner->specNum]);
    CConfigurationSetting::SpectrometerChannelSetting* curChannel = &curSpec->channel[curSpec->channelNum];
    int tmpInt;

    // find the number for this channel.
    if (char* pt = strstr(szToken, "number"))
    {
        if (pt = strstr(pt, "\""))
        {
            if (char* pt2 = strstr(pt + 1, "\""))
            {
                pt2[0] = 0; // remove the second quote
            }
            if (sscanf(pt + 1, "%d", &tmpInt))
            {
                curChannel = &curSpec->channel[tmpInt];
            }
        }
    }

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        if (Equals(szToken, "/channel"))
        {
            ++curSpec->channelNum;
            return 0;
        }

        if (Equals(szToken, "calibration", strlen("calibration")))
        {
            if (curSpec != nullptr)
            {
                Parse_Calibration(curChannel);
            }
            continue;
        }

        if (Equals(szToken, "Reference", 9))
        {
            if (curSpec != nullptr)
            {
                Parse_Reference(curChannel);
            }
            continue;
        }

        if (Equals(szToken, "specie", 6))
        {
            if (curSpec != nullptr)
            {
                Parse_Specie(curChannel);
            }
            continue;
        }

        if (Equals(szToken, "fitLow"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem(TEXT("/fitLow"), curChannel->fitWindow.fitLow);
            }
            continue;
        }

        if (Equals(szToken, "fitHigh"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem(TEXT("/fitHigh"), curChannel->fitWindow.fitHigh);
            }
            continue;
        }

        if (Equals(szToken, "dark"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem(TEXT("/dark"), tmpInt);
            }
            curChannel->darkSettings.m_darkSpecOption = (Configuration::DARK_SPEC_OPTION)tmpInt;
            continue;
        }

        if (Equals(szToken, "offsetOption"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem(TEXT("/offsetOption"), tmpInt);
            }
            curChannel->darkSettings.m_offsetOption = (Configuration::DARK_MODEL_OPTION)tmpInt;
            continue;
        }

        if (Equals(szToken, "darkcurrentOption"))
        {
            if (curSpec != nullptr)
            {
                Parse_IntItem(TEXT("/darkcurrentOption"), tmpInt);
            }
            curChannel->darkSettings.m_darkCurrentOption = (Configuration::DARK_MODEL_OPTION)tmpInt;
            continue;
        }

        if (Equals(szToken, "offsetPath"))
        {
            if (curSpec != nullptr)
            {
                this->Parse_StringItem(TEXT("/offsetPath"), curChannel->darkSettings.m_offsetSpec);
            }
            continue;
        }

        if (Equals(szToken, "darkCurrentPath"))
        {
            if (curSpec != nullptr)
            {
                this->Parse_StringItem(TEXT("/darkCurrentPath"), curChannel->darkSettings.m_darkCurrentSpec);
            }
            continue;
        }
    }
    return 0;
}

int CConfigurationFileHandler::Parse_Calibration(CConfigurationSetting::SpectrometerChannelSetting* curSpec)
{
    auto& calibrationSettings = curSpec->autoCalibration;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        if (Equals(szToken, "/calibration"))
        {
            return 0;
        }

        if (Equals(szToken, "enable", strlen("enable")))
        {
            Parse_IntItem(TEXT("/enable"), calibrationSettings.enable);
            continue;
        }

        if (Equals(szToken, "generateReferences", strlen("generateReferences")))
        {
            Parse_IntItem(TEXT("/generateReferences"), calibrationSettings.generateReferences);
            continue;
        }

        if (Equals(szToken, "filterReferences", strlen("filterReferences")))
        {
            Parse_IntItem(TEXT("/filterReferences"), calibrationSettings.filterReferences);
            continue;
        }

        if (Equals(szToken, "intervalHours", strlen("intervalHours")))
        {
            Parse_IntItem(TEXT("/intervalHours"), calibrationSettings.intervalHours);
            calibrationSettings.intervalHours = max(calibrationSettings.intervalHours, 0);
            continue;
        }

        if (Equals(szToken, "intervalTimeOfDayLow", strlen("intervalTimeOfDayLow")))
        {
            Parse_IntItem(TEXT("/intervalTimeOfDayLow"), calibrationSettings.intervalTimeOfDayLow);
            calibrationSettings.intervalTimeOfDayLow = max(0, min(86399, calibrationSettings.intervalTimeOfDayLow));
            continue;
        }

        if (Equals(szToken, "intervalTimeOfDayHigh", strlen("intervalTimeOfDayHigh")))
        {
            Parse_IntItem(TEXT("/intervalTimeOfDayHigh"), calibrationSettings.intervalTimeOfDayHigh);
            calibrationSettings.intervalTimeOfDayHigh = max(0, min(86399, calibrationSettings.intervalTimeOfDayHigh));
            continue;
        }

        if (Equals(szToken, "solarSpectrumFile", strlen("solarSpectrumFile")))
        {
            Parse_StringItem(TEXT("/solarSpectrumFile"), calibrationSettings.solarSpectrumFile);
            continue;
        }

        if (Equals(szToken, "initialCalibrationFile", strlen("initialCalibrationFile")))
        {
            Parse_StringItem(TEXT("/initialCalibrationFile"), calibrationSettings.initialCalibrationFile);
            continue;
        }

        if (Equals(szToken, "instrumentLineshapeFile", strlen("instrumentLineshapeFile")))
        {
            Parse_StringItem(TEXT("/instrumentLineshapeFile"), calibrationSettings.instrumentLineshapeFile);
            continue;
        }

        if (Equals(szToken, "initialCalibrationType", strlen("initialCalibrationType")))
        {
            Parse_IntItem(TEXT("/initialCalibrationType"), calibrationSettings.initialCalibrationType);
            continue;
        }

        if (Equals(szToken, "instrumentLineShapeFitRegionLow", strlen("instrumentLineShapeFitRegionLow")))
        {
            Parse_FloatItem(TEXT("/instrumentLineShapeFitRegionLow"), calibrationSettings.instrumentLineShapeFitRegion.low);
            continue;
        }

        if (Equals(szToken, "instrumentLineShapeFitRegionHigh", strlen("instrumentLineShapeFitRegionHigh")))
        {
            Parse_FloatItem(TEXT("/instrumentLineShapeFitRegionHigh"), calibrationSettings.instrumentLineShapeFitRegion.high);
            continue;
        }

        if (Equals(szToken, "instrumentLineShapeFitOption", strlen("instrumentLineShapeFitOption")))
        {
            Parse_IntItem(TEXT("/instrumentLineShapeFitOption"), calibrationSettings.instrumentLineShapeFitOption);
            continue;
        }
    }
    return 0;
}

int CConfigurationFileHandler::Parse_Specie(CConfigurationSetting::SpectrometerChannelSetting* curChannel)
{
    novac::CReferenceFile reference;

    // find if there is a name for this specie
    reference.m_specieName = ParseAttribute(szToken, "name");

    while (szToken = NextToken())
    {

        if (Equals(szToken, "/specie"))
        {
            curChannel->fitWindow.reference.push_back(reference);
            return 0;
        }

        reference.m_path = std::string(szToken);
    }

    return 0;
}

int CConfigurationFileHandler::Parse_Reference(CConfigurationSetting::SpectrometerChannelSetting* curChannel)
{
    novac::CReferenceFile reference;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the Reference section
        if (Equals(szToken, "/Reference"))
        {
            curChannel->fitWindow.reference.push_back(reference);
            return 0;
        }

        // found the specie name 
        if (Equals(szToken, "name"))
        {
            if (curChannel != nullptr)
            {
                Parse_StringItem("/name", reference.m_specieName);
            }
            continue;
        }

        // found the path to the reference file 
        if (Equals(szToken, "path"))
        {
            if (curChannel != nullptr)
            {
                Parse_StringItem("/path", reference.m_path);
            }
            continue;
        }

        // found the shift
        if (Equals(szToken, "shift"))
        {
            if (curChannel != nullptr)
            {
                Parse_ShiftOrSqueeze("/shift", reference.m_shiftOption, reference.m_shiftValue, reference.m_shiftMaxValue);
            }
            continue;
        }

        // found the squeeze
        if (Equals(szToken, "squeeze"))
        {
            if (curChannel != nullptr)
            {
                Parse_ShiftOrSqueeze("/squeeze", reference.m_squeezeOption, reference.m_squeezeValue, reference.m_squeezeMaxValue);
            }
            continue;
        }
    }

    return 0;
}

int CConfigurationFileHandler::Parse_ShiftOrSqueeze(const CString& label, novac::SHIFT_TYPE& option, double& lowValue, double& highValue)
{
    char* pt = nullptr;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of this section
        if (Equals(szToken, label))
        {
            return 0;
        }
        // convert the string to lowercase
        _strlwr(szToken);

        if (pt = strstr(szToken, "fix to"))
        {
            int ret = sscanf(szToken, "fix to %lf", &lowValue);
            option = novac::SHIFT_TYPE::SHIFT_FIX;
        }
        else if (pt = strstr(szToken, "free"))
        {
            option = novac::SHIFT_TYPE::SHIFT_FREE;
        }
        else if (pt = strstr(szToken, "limit"))
        {
            int ret = sscanf(szToken, "limit from %lf to %lf", &lowValue, &highValue);
            option = novac::SHIFT_TYPE::SHIFT_LIMIT;
        }
        else if (pt = strstr(szToken, "link"))
        {
            int ret = sscanf(szToken, "link %lf", &lowValue);
            option = novac::SHIFT_TYPE::SHIFT_LINK;
        }
    }
    return 0;
}

int CConfigurationFileHandler::Parse_OutputDir()
{
    return Parse_Directory(TEXT("/outputDir"), conf->outputDirectory);
}

int CConfigurationFileHandler::Parse_LocalPublishDir()
{
    return Parse_Directory(TEXT("/localPublishDirectory"), conf->webSettings.localDirectory);
}

int CConfigurationFileHandler::Parse_Medium(CConfigurationSetting::CommunicationSetting* curComm)
{
    CString tmpStr;
    int ret = Parse_StringItem(TEXT("/medium"), tmpStr);
    if (Equals(tmpStr, "Cable"))
    {
        curComm->medium = MEDIUM_CABLE;
        return ret;
    }
    if (Equals(tmpStr, "Freewave"))
    {
        curComm->medium = MEDIUM_FREEWAVE_SERIAL_MODEM;
        return ret;
    }
    if (Equals(tmpStr, "Satteline"))
    {
        curComm->medium = MEDIUM_SATTELINE_SERIAL_MODEM;
        return ret;
    }
    // FAIL
    return 0;
}

/** Parses one 'windmeasurement' section */
int	CConfigurationFileHandler::Parse_WindMeasurement()
{
    CConfigurationSetting::WindSpeedMeasurementSetting* wind = &(curScanner->windSettings);

    // the fact that there is a windmeasurement section
    //	in the configuration file means that we should automatically
    //	do wind measurements
    wind->automaticWindMeasurements = true;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the spectrometer section
        if (Equals(szToken, "/windmeasurement"))
        {
            return 0;
        }

        // found the interval between the measurements
        if (Equals(szToken, "interval"))
        {
            Parse_IntItem("/interval", wind->interval);
            continue;
        }

        // found the duration of each measurement
        if (Equals(szToken, "duration"))
        {
            Parse_IntItem("/duration", wind->duration);
            continue;
        }

        // found the maxangle of each measurement
        if (Equals(szToken, "maxangle"))
        {
            Parse_FloatItem("/maxangle", wind->maxAngle);
            continue;
        }

        // found the time of required stability before performing a measurement
        if (Equals(szToken, "stableperiod"))
        {
            Parse_IntItem("/stableperiod", wind->stablePeriod);
            continue;
        }

        // found the minimum required column 
        if (Equals(szToken, "minpeakcolumn"))
        {
            Parse_FloatItem("/minpeakcolumn", wind->minPeakColumn);
            continue;
        }

        // found the desired angle
        if (Equals(szToken, "desiredAngle"))
        {
            Parse_FloatItem("/desiredAngle", wind->desiredAngle);
            continue;
        }

        // found the settings for using calculated wind-fields...
        if (Equals(szToken, "useCalcWind"))
        {
            Parse_IntItem("/useCalcWind", wind->useCalculatedPlumeParameters);
            continue;
        }

        // found the desired angle
        if (Equals(szToken, "switchRange"))
        {
            Parse_FloatItem("/switchRange", wind->SwitchRange);
            continue;
        }
    }

    return 0;
}

/** Parses one 'realtimesetup' section */
int CConfigurationFileHandler::Parse_RealTimeSetup()
{
    CConfigurationSetting::SetupChangeSetting* scs = &(curScanner->scSettings);

    // the fact that there is a real-time setup-section
    //	in the configuration file means that we should automatically
    //	do real-time changes of the configuration-file
    scs->automaticSetupChange = 1;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the spectrometer section
        if (Equals(szToken, "/realtimesetup"))
        {
            return 0;
        }

        // should we use the calculated plume-heights and plume-directions?
        if (Equals(szToken, "usecalculatedplumeparam"))
        {
            Parse_IntItem("/usecalculatedplumeparam", scs->useCalculatedPlumeParameters);
            continue;
        }

        // the measurement mode
        if (Equals(szToken, "mode"))
        {
            Parse_IntItem("/mode", scs->mode);
            continue;
        }

        // the tolerance for changes in wind-direction
        if (Equals(szToken, "winddirtol"))
        {
            Parse_FloatItem("/winddirtol", scs->windDirectionTolerance);
            continue;
        }
    }
    return 0;
}

/** Parses the 'sourceInfo' section */
int	CConfigurationFileHandler::Parse_SourceInfo()
{
    bool correct = false; // this is only true if the source is correctly configured
    int replace = -1; // this is non-negative if we should replace the already existing information with the one just read in
    Common common;
    CString name;
    double latitude, longitude, altitude;

    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the volcano-info section
        if (Equals(szToken, "/sourceInfo"))
        {
            if (correct)
            {
                curScanner->volcano.Format(name);
                if (replace == -1)
                {
                    g_volcanoes.m_name[g_volcanoes.m_volcanoNum].Format(name);
                    g_volcanoes.m_simpleName[g_volcanoes.m_volcanoNum].Format(common.SimplifyString(name));
                    g_volcanoes.m_peakHeight[g_volcanoes.m_volcanoNum] = altitude;
                    g_volcanoes.m_peakLatitude[g_volcanoes.m_volcanoNum] = latitude;
                    g_volcanoes.m_peakLongitude[g_volcanoes.m_volcanoNum] = longitude;
                    g_volcanoes.m_volcanoNum++;
                }
                else
                {
                    g_volcanoes.m_name[replace].Format(name);
                    g_volcanoes.m_simpleName[replace].Format(common.SimplifyString(name));
                    g_volcanoes.m_peakHeight[replace] = altitude;
                    g_volcanoes.m_peakLatitude[replace] = latitude;
                    g_volcanoes.m_peakLongitude[replace] = longitude;
                }
            }
            return 0;
        }

        // found the name of the source
        if (Equals(szToken, "name"))
        {
            Parse_StringItem("/name", name);
            correct = true;
            // Check if this source already exists in our list, if so then just replace that information
            //	otherwise add it as a new source
            for (unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k)
            {
                if (Equals(name, g_volcanoes.m_name[k]))
                {
                    replace = k;
                    break;
                }
            }
            continue;
        }

        // found the latitude of the source
        if (Equals(szToken, "latitude"))
        {
            Parse_FloatItem("/latitude", latitude);
            correct = true;
            continue;
        }

        // found the longitude of the source
        if (Equals(szToken, "longitude"))
        {
            Parse_FloatItem("/longitude", longitude);
            correct = true;
            continue;
        }

        // found the altitude of the source
        if (Equals(szToken, "altitude"))
        {
            Parse_FloatItem("/altitude", altitude);
            continue;
        }
    }
    return 0;
}

/** Parses the 'windImport' - section */
int CConfigurationFileHandler::Parse_WindImport()
{
    // the actual reading loop
    while (szToken = NextToken())
    {
        if (IsEmptyLineOrStartOfComment(szToken))
        {
            continue;
        }

        // the end of the windImport section
        if (Equals(szToken, "/windImport"))
        {
            return 0;
        }

        // found the name of the wind-field file
        if (Equals(szToken, "path"))
        {
            Parse_StringItem("/path", conf->windSourceSettings.windFieldFile);
            continue;
        }

        // found the reload interval of the wind-field file
        if (Equals(szToken, "reloadInterval"))
        {
            Parse_LongItem("/reloadInterval", conf->windSourceSettings.windFileReloadInterval);
            continue;
        }

        // found the enabled flag wind-field file
        if (Equals(szToken, "enabled"))
        {
            Parse_IntItem("/enabled", conf->windSourceSettings.enabled);
            continue;
        }
    }
    return 0;
}

void CConfigurationFileHandler::CheckSettings(CConfigurationSetting& configuration)
{

    // -------- FTP - SETTINGS -------------------
    // upload times
    if (configuration.ftpSetting.ftpStopTime == configuration.ftpSetting.ftpStartTime)
    {
        // start-time is same as stoptime, change to have at least 3 hours uploading-time
        if (configuration.ftpSetting.ftpStopTime > 86400 / 2)
        {
            configuration.ftpSetting.ftpStopTime += 3 * 3600;
        }
        else
        {
            configuration.ftpSetting.ftpStartTime -= 3 * 3600;
        }
    }
    else if (configuration.ftpSetting.ftpStopTime > configuration.ftpSetting.ftpStartTime)
    {
        // stopTime > startTime
        if (configuration.ftpSetting.ftpStopTime - configuration.ftpSetting.ftpStartTime < 3 * 3600)
        {
            // less than 3 hours uploading time! change!
            configuration.ftpSetting.ftpStopTime = (configuration.ftpSetting.ftpStartTime + 3 * 3600) % 86400;
        }
    }
    else
    {
        // stopTime < starTime
        if ((86400 - configuration.ftpSetting.ftpStartTime) + configuration.ftpSetting.ftpStopTime < 3 * 3600)
        {
            // less than 3 hours uploading time! change!
            configuration.ftpSetting.ftpStopTime = (configuration.ftpSetting.ftpStartTime + 3 * 3600) % 86400;
        }
    }
}

} // namespace FileHandler
