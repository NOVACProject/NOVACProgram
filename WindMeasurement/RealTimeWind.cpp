#include "StdAfx.h"
#include "realtimewind.h"
#include "../VolcanoInfo.h"
#include "../Evaluation/Spectrometer.h"

extern CConfigurationSetting g_settings;	// <-- The settings
extern CWinThread* g_comm;					// <-- the communication controller

namespace WindSpeedMeasurement
{

    bool CRealTimeWind::IsTimeForWindMeasurement(const Evaluation::CSpectrometer* spectrometer)
    {
        CString dateStr;
        CString timeStr;
        Common common;
        common.GetDateText(dateStr);
        common.GetTimeText(timeStr);

        CString debugFile;
        debugFile.Format("%sOutput\\%s\\Debug_WindSpeedMeas.txt", (LPCTSTR)g_settings.outputDirectory, (LPCTSTR)dateStr);

        const CString serial = spectrometer->m_scanner.spec[0].serialNumber;

        const std::string volcanoName = spectrometer->m_scanner.volcano;
        const int thisVolcano = IndexOfVolcano(volcanoName);

        // 0. Local handles, to get less dereferencing...
        const CConfigurationSetting::WindSpeedMeasurementSetting* windSettings = &spectrometer->m_scanner.windSettings;

        // 1. Check the settings for the spectrometer, if there's only one channel or
        //     if the settings says that we should not perform any wind-measurements, then don't
        if (false == windSettings->automaticWindMeasurements)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Automatic wind measurements are not setup for this volcano. \n", (LPCTSTR)timeStr, (LPCSTR)serial);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        if (spectrometer->m_scanner.spec[0].channelNum == 1)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Automatic wind measurements can only be done using multi-channel spectrometers. \n", (LPCTSTR)timeStr, (LPCSTR)serial);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        if (spectrometer->m_history == nullptr)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: No instrument history saved for this instrument. \n", (LPCTSTR)timeStr, (LPCSTR)serial);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2. Check the history of the spectrometer

        // 2a. Have we recieved any scans today?
        const int nScansToday = spectrometer->m_history->GetNumScans();
        if (nScansToday < windSettings->stablePeriod)
        {
            // there must be enough scans received from the instrument today for us to make any wind-measurement
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Too few scans today (%d)\n", (LPCTSTR)timeStr, (LPCSTR)serial, nScansToday);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false; // <-- too few scans recieved today
        }

        // 2b. How long time has passed since the last ws-measurement?
        const int sPassed = spectrometer->m_history->SecondsSinceLastWindMeas();
        if (sPassed > 0 && sPassed < windSettings->interval)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Little time since last measurement (%d seconds).\n", (LPCTSTR)timeStr, (LPCSTR)serial, sPassed);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2c. What's the average time between the start of two scans today?
        const double timePerScan = spectrometer->m_history->GetScanInterval();
        if (timePerScan < 0)
        {
            return false; // <-- no scans have arrived today
        }

        // 2d. Get the plume centre variation over the last few scans
        //			If any scan missed the plume, return false
        double centreMin, centreMax;
        if (false == spectrometer->m_history->GetPlumeCentreVariation(windSettings->stablePeriod, 0, centreMin, centreMax))
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: At least one of the last %d scans has missed the plume\n", (LPCTSTR)timeStr, (LPCSTR)serial, windSettings->stablePeriod);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2e. If there's a substantial variation in plume centre over the last
        //      few scans then don't make any measurement
        if (std::abs(centreMax - centreMin) > 3.00)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Too large variation in plume centre (%lf degrees) \n", (LPCTSTR)timeStr, (LPCSTR)serial, centreMax - centreMin);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2g. If the plumeCentre is at lower angles than the options for the
        //      windmeasurements allow, then don't measure
        const double plumeCentre = spectrometer->m_history->GetPlumeCentre(windSettings->stablePeriod);
        if (std::abs(plumeCentre) > std::abs(windSettings->maxAngle))
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Plume centre too low (plumeCentre: %lf, maxAngle: %lf)\n", (LPCTSTR)timeStr, (LPCSTR)serial, plumeCentre, windSettings->maxAngle);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2h. Check so that the maximum column averaged over the last few scans is at least
        //      50 ppmm
        const double maxColumn = spectrometer->m_history->GetColumnMax(windSettings->stablePeriod);
        if (maxColumn < windSettings->minPeakColumn)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Plume too weak (maxColumn: %lf) \n", (LPCTSTR)timeStr, (LPCSTR)serial, maxColumn);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        // 2i. Check so that the average exposure-time over the last few scans is not
        //      too large. This to ensure that we can measure fast enough
        const double expTime = spectrometer->m_history->GetExposureTime(windSettings->stablePeriod);
        if (expTime < 0 || expTime > 600)
        {
            FILE* f = fopen(debugFile, "a+");
            if (f != nullptr)
            {
                fprintf(f, "%s\t%s\t No measurement: Too long exposure times (%lf ms) \n", (LPCTSTR)timeStr, (LPCSTR)serial, expTime);
                fclose(f);
                UploadToNOVACServer(debugFile, thisVolcano, false);
            }
            return false;
        }

        FILE* f = fopen(debugFile, "a+");
        if (f != nullptr)
        {
            fprintf(f, "%s\t%s\t Ok to do wind-speed measurement!!\n", (LPCTSTR)timeStr, (LPCSTR)serial);
            fclose(f);
            UploadToNOVACServer(debugFile, thisVolcano, false);
        }

        // We've passed all tests, it's now ok to make a wind-speed measurement
        return true;
    }

    void CRealTimeWind::StartWindSpeedMeasurement(const Evaluation::CSpectrometer* spec)
    {
        static CString message;
        CString dateTime, directory;
        Common common;
        int stepsPerRound = 200;
        double maxExpTime = 300.0; // the maximum allowed exposure-time
        int readOutTime = 100; // time to read out one spectrum from the detector
        int stablePeriod = spec->m_scanner.windSettings.stablePeriod;
        double plumeCentre = spec->m_history->GetPlumeCentre(stablePeriod);
        int motorPosition = (int)((plumeCentre / 2) * (stepsPerRound / 360.0));
        double avgExpTime = std::min(fabs(spec->m_history->GetExposureTime(stablePeriod)), maxExpTime);
        int sum1 = (int)std::min(std::max(1000.0 / (avgExpTime + readOutTime), 1.0), 15.0); // <-- calculate the number of co-adds assuming 100ms to read out each spectrum

        int	repetitions = (int)(spec->m_scanner.windSettings.duration / (sum1 * (avgExpTime + readOutTime) / 1000));
        repetitions = std::max(std::min(repetitions, 800), 400);

        // 0. Simple error checking...
        if (fabs(motorPosition) > 50 || fabs(plumeCentre) > 90)
            return; // illegal angle...
        if (fabs(plumeCentre) > fabs(spec->m_scanner.windSettings.maxAngle))
            return; // attempt to make a measurement outside of allowed range...

        // allocate the strings	
        CString* serialNumber = new CString();
        CString* fileName = new CString();

        // 1. Get the serial-number of the selected spectrometer
        serialNumber->Format(spec->m_scanner.spec[0].serialNumber);

        // 2. Get the directory where to temporarily store the cfgonce.txt
        if (strlen(g_settings.outputDirectory) > 0) {
            directory.Format("%sTemp\\%s\\", (LPCTSTR)g_settings.outputDirectory, (LPCTSTR)serialNumber);
            if (CreateDirectoryStructure(directory)) {
                common.GetExePath();
                fileName->Format("%s\\cfgonce.txt", (LPCTSTR)common.m_exePath);
            }
            else {
                fileName->Format("%scfgonce.txt", (LPCTSTR)directory);
            }
        }
        else {
            common.GetExePath();
            fileName->Format("%s\\cfgonce.txt", (LPCTSTR)common.m_exePath);
        }

        FILE* f = fopen(*fileName, "w");
        if (f == nullptr) {
            ShowMessage("Could not open cfgonce.txt for writing. Wind speed measurement failed!!");
            return;
        }

        // 3. Write the configuration-file

        // 3a. A small header 
        common.GetDateTimeText(dateTime);
        fprintf(f, "%%-------------Modified at %s------------\n\n", (LPCTSTR)dateTime);

        // 3c. Write the Spectrum transfer information
        fprintf(f, "%% The following channels defines which channels in the spectra that will be transferred\n");
        fprintf(f, "STARTCHN=0\n");
        fprintf(f, "STOPCHN=684\n\n");

        // 3d. Don't use real-time collection
        fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
        fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
        fprintf(f, "REALTIME=0\n\n");

        // 3e. Write the motor information
        fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
        fprintf(f, "STEPSPERROUND=%d\n", stepsPerRound);
        fprintf(f, "MOTORSTEPCOMP=%d\n", spec->m_scanner.motor[0].motorStepsComp);
        fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
        fprintf(f, "SKIPMOTOR=0\n");
        if (spec->m_scanner.coneAngle == 90.0)
            fprintf(f, "DELAY=%d\n\n", 200);
        else
            fprintf(f, "DELAY=%d\n\n", 400);

        // 3f. Write the geometry (compass, tilt...)
        fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
        fprintf(f, "COMPASS=%.1lf 0.0 0.0\n\n", spec->m_scanner.compass);

        // 3g. Write other things
        fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
        fprintf(f, "PERCENT=%.2lf\n\n", 0.8);
        fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
        fprintf(f, "MAXINTTIME=%.0lf\n\n", maxExpTime);
        fprintf(f, "%% The pixel where we want to measure the intensity of the spectra \n");
        fprintf(f, "CHANNEL=670\n\n");
        fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
        fprintf(f, "DEBUG=1\n\n");

        // 3h. Write the measurement information
        fprintf(f, "%% sum1 is inside the specrometer [1 to 15]\n%%-----pos----time-sum1-sum2--chn--basename----- repetitions\n");

        // 3i. The sky-measurement
        fprintf(f, "MEAS=%d -1 15 1 257 sky 1 0\n", motorPosition);

        // 3j. The dark-measurement
        fprintf(f, "MEAS=100 0 15 1 257 dark 1 0\n");

        // 3k. The actual measurement
        fprintf(f, "MEAS=%d 0 %d 1 257 wind %d 0\n", motorPosition, sum1, repetitions);

        // 3l. Another dark-measurement
        fprintf(f, "MEAS=100 0 15 1 257 dark 1 0\n");

        // Close the file
        fclose(f);

        // 4. Tell the communication controller that we want to upload a file
        if (g_comm != nullptr)
        {
            g_comm->PostThreadMessage(WM_UPLOAD_CFGONCE, (WPARAM)serialNumber, (LPARAM)fileName);

            // 4b. Tell the user what we've done...
            message.Format("Telling spectrometer %s to perform wind-measurement", (LPCTSTR)serialNumber);
            ShowMessage(message);
        }
    }

}
