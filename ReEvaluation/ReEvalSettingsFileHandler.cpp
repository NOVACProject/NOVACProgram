#include "StdAfx.h"
#include "reevalsettingsfilehandler.h"

using namespace FileHandler;

CReEvalSettingsFileHandler::CReEvalSettingsFileHandler(void)
{
}

CReEvalSettingsFileHandler::~CReEvalSettingsFileHandler(void)
{
}

/** Writes the supplied re-evaluation settings to a file.
		@param reeval - the settings to be written to file
		@param fileName - the name and path of the file to which to write
		@param overWrite - if true the file will be overwritten, if false, the file will be appended */
RETURN_CODE CReEvalSettingsFileHandler::WriteSettings(const ReEvaluation::CReEvaluator &reeval, const CString &fileName, bool overWrite){
	FILE *f = fopen(fileName, "w");
	if(f == NULL){
		MessageBox(NULL, "Cannot open file for writing. No settings saved", "Error", MB_OK);
		return FAIL;
	}

	fprintf(f, TEXT("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"));
	fprintf(f, TEXT("<!-- These are settings for the re-evaluation in the NovacProgram -->\n\n"));
	fprintf(f, TEXT("<ReEvalSettings_Misc>\n"));

	// 1. What to do with spectra with too low intensity
	fprintf(f, "\t<LowIntensity>\n");
	fprintf(f, "\t\t<Type>%d</Type>\n",								(int)reeval.m_ignore_Lower.m_type);
	fprintf(f, "\t\t<Intensity>%.2lf</Intensity>\n",	(double)reeval.m_ignore_Lower.m_intensity);
	fprintf(f, "\t\t<Channel>%d</Channel>\n",					(int)reeval.m_ignore_Lower.m_channel);
	fprintf(f, "\t</LowIntensity>\n");

	// 2. What to do with spectra with too high intensity
	fprintf(f, "\t<HighIntensity>\n");
	fprintf(f, "\t\t<Type>%d</Type>\n",								(int)reeval.m_ignore_Upper.m_type);
	fprintf(f, "\t\t<Intensity>%.2lf</Intensity>\n",	(double)reeval.m_ignore_Upper.m_intensity);
	fprintf(f, "\t\t<Channel>%d</Channel>\n",					(int)reeval.m_ignore_Upper.m_channel);
	fprintf(f, "\t</HighIntensity>\n");

	// 3. How to get the sky-spectrum
	fprintf(f, "\t<SkySpectrum>\n");
	fprintf(f, "\t\t<Option>%d</Option>\n",						(int)reeval.m_skyOption);
	fprintf(f, "\t\t<Index>%d</Index>\n",							(int)reeval.m_skyIndex);
	if(reeval.m_skySpectrum.GetLength() > 1){
		fprintf(f, "\t\t<Path>%s</Path>\n",							reeval.m_skySpectrum);
	}
	fprintf(f, "\t</SkySpectrum>\n");

	// 4. How to get the dark-spectrum
	fprintf(f, "\t<DarkSpectrum>\n");
	fprintf(f, "\t\t<Option>%d</Option>\n",							(int)reeval.m_darkSettings.m_darkSpecOption);
	fprintf(f, "\t\t<OffsetOption>%d</OffsetOption>\n",	(int)reeval.m_darkSettings.m_offsetOption);
	fprintf(f, "\t\t<DCOption>%d</DCOption>\n",					(int)reeval.m_darkSettings.m_darkCurrentOption);
	fprintf(f, "\t\t<Index>%d</Index>\n",							(int)reeval.m_skyIndex);
	if(reeval.m_darkSettings.m_offsetSpec.GetLength() > 1){
		fprintf(f, "\t\t<Offset_Path>%s</Offset_Path>\n",			reeval.m_darkSettings.m_offsetSpec);
	}
	if(reeval.m_darkSettings.m_darkCurrentSpec.GetLength() > 1){
		fprintf(f, "\t\t<DC_Path>%s</DC_Path>\n",							reeval.m_darkSettings.m_darkCurrentSpec);
	}
	fprintf(f, "\t</DarkSpectrum>\n");

	// 5. Misc...
	fprintf(f, "\t<Average>%d</Average>\n",									(int)reeval.m_averagedSpectra);


	fprintf(f, TEXT("</ReEvalSettings_Misc>\n"));
	fclose(f);

	return SUCCESS;
}


/** Reads the desired re-evaluation settings from the file.
		@param reeval - will be filled with the read settings if successfull
		@param fileName - the name and path of the file to read from
		@return SUCCESS on success */
RETURN_CODE CReEvalSettingsFileHandler::ReadSettings(ReEvaluation::CReEvaluator &reeval, const CString &fileName){
	char  lowIntensStr[]			= _T("LowIntensity");
	char  highIntensStr[]			= _T("HighIntensity");
	char  skySpecStr[]				= _T("SkySpectrum");
	char  darkSpecStr[]				= _T("DarkSpectrum");
	char  averageStr[]				= _T("Average");

	CFileException exceFile;
  CStdioFile file;

	// 1. Open the file
  if(!file.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile)){
    return FAIL;
  }
  this->m_File = &file;

	// parse the file
	while(szToken = NextToken()){
    // no use to parse empty lines
    if(strlen(szToken) < 3)
      continue;

		if(Equals(szToken, lowIntensStr, strlen(lowIntensStr))){
			if(SUCCESS == Parse_IntensitySection(reeval, 0)){
				continue;
			}else{
				// parsing has failed!
				file.Close();
				return FAIL;
			}
		}

		if(Equals(szToken, highIntensStr, strlen(highIntensStr))){
			if(SUCCESS == Parse_IntensitySection(reeval, 1)){
				continue;
			}else{
				// parsing has failed!
				file.Close();
				return FAIL;
			}
		}

		if(Equals(szToken, skySpecStr, strlen(skySpecStr))){
			if(SUCCESS == Parse_SkySpecSection(reeval)){
				continue;
			}else{
				// parsing has failed!
				file.Close();
				return FAIL;
			}
		}

		if(Equals(szToken, darkSpecStr, strlen(darkSpecStr))){
			if(SUCCESS == Parse_DarkSpecSection(reeval)){
				continue;
			}else{
				// parsing has failed!
				file.Close();
				return FAIL;
			}
		}

		if(Equals(szToken, averageStr, strlen(averageStr))){
			int tmpInt;
			Parse_IntItem("/Average", tmpInt);
			reeval.m_averagedSpectra = (tmpInt == 1)? true : false;
			continue;
		}
	}

	// done
	return SUCCESS;
}


RETURN_CODE CReEvalSettingsFileHandler::Parse_IntensitySection(ReEvaluation::CReEvaluator &reeval, int type){
	int tmpInt;

	// the actual reading loop
  while(szToken = NextToken()){

    // no use to parse empty lines
    if(strlen(szToken) < 3)
      continue;

    // ignore comments
    if(Equals(szToken, "!--", 3)){
      continue;
    }

		if(Equals(szToken, "/LowIntensity") || Equals(szToken, "/HighIntensity")){
			return SUCCESS;
		}

		if(Equals(szToken, "Type")){
			Parse_IntItem(TEXT("/Type"), tmpInt);
			if(type == 0)
				reeval.m_ignore_Lower.m_type = (Evaluation::IgnoreType)tmpInt;
			else
				reeval.m_ignore_Upper.m_type = (Evaluation::IgnoreType)tmpInt;
			continue;
		}

		if(Equals(szToken, "Intensity")){
			if(type == 0)
				Parse_FloatItem(TEXT("/Intensity"), reeval.m_ignore_Lower.m_intensity);
			else
				Parse_FloatItem(TEXT("/Intensity"), reeval.m_ignore_Upper.m_intensity);
			continue;
		}

		if(Equals(szToken, "Channel")){
			if(type == 0)
				Parse_LongItem(TEXT("/Channel"), reeval.m_ignore_Lower.m_channel);
			else
				Parse_LongItem(TEXT("/Channel"), reeval.m_ignore_Upper.m_channel);
			continue;
		}
	}

	return FAIL;
}


RETURN_CODE CReEvalSettingsFileHandler::Parse_SkySpecSection(ReEvaluation::CReEvaluator &reeval){
	int tmpInt;

	// the actual reading loop
  while(szToken = NextToken()){

    // no use to parse empty lines
    if(strlen(szToken) < 3)
      continue;

    // ignore comments
    if(Equals(szToken, "!--", 3)){
      continue;
    }

		if(Equals(szToken, "/SkySpectrum")){
			return SUCCESS;
		}

		if(Equals(szToken, "Option")){
			Parse_IntItem(TEXT("/Option"), tmpInt);
			reeval.m_skyOption = (SKY_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "Index")){
			Parse_LongItem(TEXT("/Index"), reeval.m_skyIndex);
			continue;
		}

		if(Equals(szToken, "Path")){
			Parse_StringItem(TEXT("/Path"), reeval.m_skySpectrum);
			continue;
		}
	}

	return FAIL;
}

RETURN_CODE CReEvalSettingsFileHandler::Parse_DarkSpecSection(ReEvaluation::CReEvaluator &reeval){
	int tmpInt;

	// the actual reading loop
  while(szToken = NextToken()){

    // no use to parse empty lines
    if(strlen(szToken) < 3)
      continue;

    // ignore comments
    if(Equals(szToken, "!--", 3)){
      continue;
    }

		if(Equals(szToken, "/DarkSpectrum")){
			return SUCCESS;
		}

		if(Equals(szToken, "Option")){
			Parse_IntItem(TEXT("/Option"), tmpInt);
			reeval.m_darkSettings.m_darkSpecOption = (DARK_SPEC_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "OffsetOption")){
			Parse_IntItem(TEXT("/OffsetOption"), tmpInt);
			reeval.m_darkSettings.m_offsetOption = (DARK_MODEL_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "DCOption")){
			Parse_IntItem(TEXT("/DCOption"), tmpInt);
			reeval.m_darkSettings.m_darkCurrentOption = (DARK_MODEL_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "Offset_Path")){
			Parse_StringItem(TEXT("/Offset_Path"), reeval.m_darkSettings.m_offsetSpec);
			continue;
		}

		if(Equals(szToken, "DC_Path")){
			Parse_StringItem(TEXT("/DC_Path"), reeval.m_darkSettings.m_darkCurrentSpec);
			continue;
		}
	}

	return FAIL;
}
