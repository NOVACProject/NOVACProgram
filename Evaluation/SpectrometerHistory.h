#pragma once

#include <afxtempl.h>
#include <memory>

#include "../Common/Common.h"
#include <SpectralEvaluation/DateTime.h>
#include <SpectralEvaluation/Calibration/InstrumentCalibration.h>

#include "ScanResult.h"

namespace Evaluation {

    /** <b>CSpectrometerHistory</b> is a data structure, used by the
            CEvaluationController, for remembering what has happened at each
            instrument. Used e.g. for controlling when to perform automatic
            wind measurements.  */

    class CSpectrometerHistory
    {

    public:
        /** Default constructor */
        CSpectrometerHistory(void);

        /** Default desctructor */
        ~CSpectrometerHistory(void);

        // ----------------------------------------------------------------------
        // -------------------- PUBLIC METHODS ----------------------------------
        // ----------------------------------------------------------------------

        /** Appends a new set of results. */
        void AppendScanResult(const CScanResult& result, const std::string& specie);

        /** Appends the information that a wind-speed measurement has been performed
                at a specific time. */
        void AppendWindMeasurement(int year, int month, int day, int hour, int minute, int second);

        /** Appends the information that a composition measurement has been performed
                at a specific time. */
        void AppendCompMeasurement(int year, int month, int day, int hour, int minute, int second);

        /** Appends the information that an automatic instrument calibration has been
        *   performed at the given time and had the provided result.
        *   @param timeOfScan Start the time the scan on which the measurement is based was started.
        *   @param calibration The resulting calibration. This will only copy out relevant information and not touch the pointer. */
        void AppendInstrumentCalibration(const novac::CDateTime& timeOfScan, const std::unique_ptr<novac::InstrumentCalibration>& calibration);

        /** @return the number of seconds passed since the last scan arrived.
            @return -1 if no scans has arrived */
        int SecondsSinceLastScan() const;

        /** Returns the number of seconds passed since the last wind-measurement
                arrived. Return -1 if no wind-measurement has arrived */
        int SecondsSinceLastWindMeas() const;

        /** Returns the number of seconds passed since the last instrument calibration
            was performed. Return -1 if no calibration has been performed. */
        int SecondsSinceLastInstrumentCalibration() const;

        /** Returns the number of seconds passed since the last composition measurement
                arrived. Return -1 if no composition measurement has arrived */
        int SecondsSinceLastCompMeas() const;

        /** Returns the average number of seconds passed between the
                starting of the scans that are collected today.
                Return -1 if no scans has arrived */
        double GetScanInterval();

        /** Returns the range of zenith-angles for the last scan
                (Normally these are from -90 to +90)
                If no scan collected today has been received then alpha_min will be +999.0
                and alpha_max will be -999.0 */
        void GetAlphaRange(double& alpha_min, double& alpha_max);

        /** Returns the number of scans that are both:
                1) arrived from this spectrometer today AND
                2) are collected today. */
        int GetNumScans() const;

        /** Returns the average plume-centre during the
                last 'scansToAverage' scans today. Returns -180 if there are
                fewer than 'scansToAverage' scans collected today OR
                if any of the last 'scansToAverage' misses the plume. */
        double GetPlumeCentre(int scansToAverage, int motor = 0);

        /** Returns the average plume completeness during the
                last 'scansToAverage' scans today. Returns -1 if there are
                fewer than 'scansToAverage' scans collected today OR
                if any of the last 'scansToAverage' misses the plume. */
        double GetPlumeCompleteness(int scansToAverage);

        /** Returns the average positions of the edges of the plume during the
                last 'scansToAverage' scans today. Returns false if there are
                fewer than 'scansToAverage' scans collected today OR
                if any of the last 'scansToAverage' misses the plume. */
        bool GetPlumeEdges(int scansToAverage, double& lowEdge, double& highEdge);

        /** Returns the maximum and minimum values of the plume-centre
                over the last 'scansToAverage' scans today. Returns false if
                fewer than 'scansToAverage' scans are collected today OR
                if any of the last 'scansToAverage' misses the plume. */
        bool GetPlumeCentreVariation(int scansToAverage, int motor, double& centreMin, double& centreMax);

        /** Returns the average exposure-time during the last
                'scansToAverage' scans today. returns -1 if there are fewer
                than 'scansToAverage' scans collected today */
        double GetExposureTime(int scansToAverage);

        /** Returns the peak column averaged over the last
                'scansToAverage' scans today. returns -1 if there are fewer
                than 'scansToAverage' scans collected today */
        double GetColumnMax(int scansToAverage);

        // ----------------------------------------------------------------------
        // ----------------------- PUBLIC DATA ----------------------------------
        // ----------------------------------------------------------------------

        /** The maximum length of our memory of the history */
        static const int MAX_SPECTROMETER_HISTORY = 128;

    private:
        // ----------------------------------------------------------------------
        // --------------------- PRIVATE METHODS --------------------------------
        // ----------------------------------------------------------------------


        // ----------------------------------------------------------------------
        // ---------------------- PRIVATE DATA ----------------------------------
        // ----------------------------------------------------------------------

        /** Class to store information about scans */
        struct CScanInfo {
            CScanInfo();

            novac::CDateTime   startTime;         // <-- the start-time of the scan (usually GMT)
            novac::CDateTime   arrived;           // <-- the time when the scan-info arrived (local PC-time)
            double      plumeCentre[2];    // <-- the position of the plume, in scanAngles [degrees], for each of the two motors
            double      plumeEdge[2];      // <-- the edges of the plulm, in scanAngles [degrees] for the first motor
            double      plumeCompleteness; // <-- how complete is the plume (from 0.5 to 1.0)
            int         exposureTime;      // <-- the exposure-time in ms
            double      maxColumn;         // <-- the maximum (good) column-value in the scan
            float       alpha[2];          // <-- the minimum and maximum scanAngle in the scan
        };

        /** Class to store information about wind-measurements*/
        struct CWMInfo {
            novac::CDateTime startTime; // <-- the start-time of the scan (usually GMT)
            novac::CDateTime arrived;   // <-- the time when the scan-info arrived (local PC-time)
        };

        /** Class to store information about instrument calibrations performed. */
        struct CCalibrationInfo {
            novac::CDateTime startTime; // <-- the start-time of the scan (usually GMT)
            novac::CDateTime arrived;   // <-- the time when the scan-info arrived (local PC-time)
            double instrumentFwhm = 0.0;
            std::vector<double> pixelToWavelengthPolynomial;
        };

        /** This is the list of scan-informations for the last X scans.
                The results are sorted in descending order wrt starttime of the scans */
        CList <CScanInfo, CScanInfo&> m_scanInfo;

        /** The time and date of the last wind-speed measurements performed.
                These results are sorted in descending order wrt starttime. */
        CList <CWMInfo, CWMInfo&> m_windMeasurementTimes;

        /** The time and date of the last composition measurements performed.
                These results are sorted in descending order wrt starttime. */
        CList <CWMInfo, CWMInfo&> m_compMeasurementTimes;

        /** The tiem and date of each instrument calibration performed.
            These are sorted in descending order wrt start time */
        std::vector<CCalibrationInfo> m_instrumentCalibration;
    };
}