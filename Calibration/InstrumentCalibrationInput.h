#pragma once

enum class InstrumentCalibrationInputOption
{
    NovacInstrumentCalibrationFile = 0, // i.e. input is one file containing both wavelength calibration and instrument line shape
    WavelengthAndSlitFunctionFile = 1,  // i.e. input is two files containing the wavelength calibration and instrument line shape respectively
    WavelengthCalibrationFile = 2       // i.e. input is one file containing the wavelength calibration. The instrument line shape needs to be determined from the measurement.
};

struct InstrumentCalibrationInput
{
    InstrumentCalibrationInput() :
        initialCalibrationFile(""),
        instrumentLineshapeFile(""),
        calibrationOption(InstrumentCalibrationInputOption::NovacInstrumentCalibrationFile)
    {
    }

    /** The wavelength calibration file _or_ the full novac instrument calibration file, depending on the option */
    CString initialCalibrationFile;

    /** The instrument line shape file (optional) */
    CString instrumentLineshapeFile;

    /** The option for how to interpret the initialCalibrationFile and the instrumentLineshapeFile */
    InstrumentCalibrationInputOption calibrationOption;
};