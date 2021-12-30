#pragma once
#include "Common/Common.h"

/**
    The class <b>CUserSettings</b> stores the preferences for the
    current user.
    This can be e.g. the unit to use on the flux.
*/
class CUserSettings
{
public:

    // ----------------------------------------------------------------
    // ------------------ READING/WRITING THE SETTINGS ----------------
    // ----------------------------------------------------------------

    /** Writes the settings to file */
    void WriteToFile();

    /** Reads the settings from file */
    void ReadSettings(const CString* fileName = nullptr);

    // -----------------------------------------------------------
    // --------------------- THE SETTINGS ------------------------
    // -----------------------------------------------------------

    // the unit to use of the fluxes
    FLUX_UNIT m_fluxUnit = UNIT_TONDAY;

    // the unit to use of the columns
    COLUMN_UNIT m_columnUnit = UNIT_PPMM;

    // The preferred language
    LANGUAGES m_language = LANGUAGE_ENGLISH;

private:
    /** The name of the user-settings file */
    CString m_userSettingsFile;
};
