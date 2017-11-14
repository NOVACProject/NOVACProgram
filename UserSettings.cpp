#include "StdAfx.h"
#include "usersettings.h"

/** The global instance of user settings */
CUserSettings g_userSettings;

CUserSettings::CUserSettings(void)
{
	m_fluxUnit = UNIT_TONDAY;
	m_columnUnit = UNIT_PPMM;
	m_language = LANGUAGE_ENGLISH;
}

CUserSettings::~CUserSettings(void)
{
}

/** Writes the settings to file */
void CUserSettings::WriteToFile(){

	FILE *f = fopen(m_userSettingsFile, "w");
	if(f == NULL)
		return;

	fprintf(f, "LANGUAGE=%d\n", m_language);
	fprintf(f, "FLUXUNIT=%d\n", m_fluxUnit);
	fprintf(f, "COLUMNUNIT=%d\n", m_columnUnit);

	fclose(f);
}

/** Reads the settings from file */
void CUserSettings::ReadSettings(const CString *fileName){
	char nl[2]={ 0x0a, 0 };
	char lf[2]={ 0x0d, 0 };
	char *pt;
	char txt[256];

	if(fileName != NULL){
		m_userSettingsFile.Format("%s", *fileName);
	}
	
	FILE *f = fopen(m_userSettingsFile, "r");
	if(f == NULL)
		return;

	while(fgets(txt,sizeof(txt)-1, f)){
		if(strlen(txt)>4 && txt[0]!='%'){
			pt = txt;
			if(pt = strstr(txt,nl))
				pt[0]=0;

			pt = txt;
			if(pt = strstr(txt,lf))
				pt[0] = 0;

			// 1. Parse the flux unit
			if(pt = strstr(txt,"FLUXUNIT=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_fluxUnit);
			}

			// 2. Parse the column unit
			if(pt = strstr(txt,"COLUMNUNIT=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_columnUnit);
			}

			// 3. Parse the language
			if(pt = strstr(txt,"LANGUAGE=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_language);
			}
		}
	}
	fclose(f);
}

