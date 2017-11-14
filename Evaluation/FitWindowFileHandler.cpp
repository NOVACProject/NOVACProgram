#include "StdAfx.h"
#include "FitWindowFileHandler.h"

using namespace FileHandler;

CFitWindowFileHandler::CFitWindowFileHandler(void)
{
}

CFitWindowFileHandler::~CFitWindowFileHandler(void)
{
}

/** Reads the desired fit window from the file.
		@param window - will be filled with the read fit-window settings if successfull
		@param fileName - the name and path of the file to read from
		@param index - the zero-based index of the fit-window to read (each .nfs file
			can contain several fit-windows)
		@return SUCCESS on success */
RETURN_CODE CFitWindowFileHandler::ReadFitWindow(Evaluation::CFitWindow &window, const CString &fileName, int index){
	CFileException exceFile;
	CStdioFile file;
	Evaluation::CFitWindow tmpWindow;
	int curWindow = 0;

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

		if(Equals(szToken, "fitWindow", 9)){
			if(SUCCESS == Parse_FitWindow(tmpWindow)){

				if(index == curWindow){
					window = tmpWindow;
					file.Close();
					return SUCCESS;
				}else{
					++curWindow;
				}

			}else{
				// parse_fit window has failed!
				file.Close();
				return FAIL;
			}
		}
	}

	// should not get here
	return FAIL;
}
/** Parses a fit-window section of the .nfs file */
RETURN_CODE CFitWindowFileHandler::Parse_FitWindow(Evaluation::CFitWindow &window){
	window.Clear(); // <-- Reset the data before we start reading from the file.

	// find the name for this fit window
	if(char *pt = strstr(szToken, "name")){
		if(pt = strstr(pt, "\"")){
			if(char *pt2 = strstr(pt+1, "\""))
				pt2[0] = 0; // remove the second quote
			char tmpStr[512];
			if(sscanf(pt+1, "%s", &tmpStr)){
				window.name.Format("%s", tmpStr);
			}
		}
	}

	// parse the file
	while(szToken = NextToken()){
		// no use to parse empty lines
		if(strlen(szToken) < 2)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// end of fit-window section
		if(Equals(szToken, "/fitWindow")){
			return SUCCESS;
		}

		if(Equals(szToken, "fitLow")){
			Parse_IntItem(TEXT("/fitLow"), window.fitLow);
			continue;
		}
		if(Equals(szToken, "fitHigh")){
			Parse_IntItem(TEXT("/fitHigh"), window.fitHigh);
			continue;
		}
		if(Equals(szToken, "polyOrder")){
			Parse_IntItem(TEXT("/polyOrder"), window.polyOrder);
			continue;
		}
		if(Equals(szToken, "fitType")){
			Parse_IntItem(TEXT("/fitType"), (int&)window.fitType); // TODO: Will this be ok????
			continue;
		}
		if(Equals(szToken, "channel")){
			Parse_IntItem(TEXT("/channel"), window.channel);
			continue;
		}
		if(Equals(szToken, "specLength")){
			Parse_IntItem(TEXT("/specLength"), window.specLength);
			continue;
		}
		if(Equals(szToken, "fOptShift")){
			Parse_IntItem(TEXT("/fOptShift"), window.findOptimalShift);
			continue;
		}
		if(Equals(szToken, "UV")){
			Parse_IntItem(TEXT("/UV"), window.UV);
			continue;
		}
		if(Equals(szToken, "shiftSky")){
			Parse_IntItem(TEXT("/shiftSky"), window.shiftSky);
			continue;
		}
		if(Equals(szToken, "interlaceStep")){
			Parse_IntItem(TEXT("/interlaceStep"), window.interlaceStep);
			continue;
		}
		if(Equals(szToken, "interlaced")){
			Parse_IntItem(TEXT("/interlaced"), window.interlaceStep);
			window.interlaceStep += 1;
			continue;
		}
		//if(Equals(szToken, "nRef")){
		//	Parse_IntItem(TEXT("/nRef"), window.nRef);
	//		continue;
	//	}

		if(Equals(szToken, "ref", 3)){
			Parse_Reference(window);
			continue;
		}
	}

	return FAIL;
}

RETURN_CODE CFitWindowFileHandler::Parse_Reference(Evaluation::CFitWindow &window){
	int nRef = window.nRef;

	// find the name for this reference.
	if(char *pt = strstr(szToken, "name")){
		if(pt = strstr(pt, "\"")){
			if(char *pt2 = strstr(pt+1, "\""))
				pt2[0] = 0; // remove the second quote
			char tmpStr[512];
			if(sscanf(pt+1, "%s", &tmpStr)){
				window.ref[nRef].m_specieName.Format("%s", tmpStr);
			}
		}
	}

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		if(Equals(szToken, "/ref")){
			++window.nRef;
			return SUCCESS;
		}

		if(Equals(szToken, "path")){
			Parse_StringItem(TEXT("/path"), window.ref[nRef].m_path);
			continue;
		}

		if(Equals(szToken, "shiftOption")){
			int tmpInt;
			Parse_IntItem(TEXT("/shiftOption"), tmpInt);
			switch(tmpInt){
				case 0: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FREE; break;
				case 1: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_FIX; break;
				case 2: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LINK; break;
				case 3: window.ref[nRef].m_shiftOption = Evaluation::SHIFT_LIMIT; break;
			}
			continue;
		}

		if(Equals(szToken, "shiftValue")){
			Parse_FloatItem(TEXT("/shiftValue"), window.ref[nRef].m_shiftValue);
			continue;
		}

		if(Equals(szToken, "squeezeOption")){
			int tmpInt;
			Parse_IntItem(TEXT("/squeezeOption"), tmpInt);
			switch(tmpInt){
				case 0: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FREE; break;
				case 1: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_FIX; break;
				case 2: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LINK; break;
				case 3: window.ref[nRef].m_squeezeOption = Evaluation::SHIFT_LIMIT; break;
			}
			continue;
		}

		if(Equals(szToken, "squeezeValue")){
			Parse_FloatItem(TEXT("/squeezeValue"), window.ref[nRef].m_squeezeValue);
			continue;
		}

		if(Equals(szToken, "columnOption")){
			int tmpInt;
			Parse_IntItem(TEXT("/columnOption"), tmpInt);
			switch(tmpInt){
				case 0: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FREE; break;
				case 1: window.ref[nRef].m_columnOption = Evaluation::SHIFT_FIX; break;
				case 2: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LINK; break;
				case 3: window.ref[nRef].m_columnOption = Evaluation::SHIFT_LIMIT; break;
			}
			continue;
		}

		if(Equals(szToken, "columnValue")){
			Parse_FloatItem(TEXT("/columnValue"), window.ref[nRef].m_columnValue);
			continue;
		}
	}

	return FAIL;
}

/** Writes the supplied fit-window to a file.
		@param window - hte fit window to be written to file
		@param fileName - the name and path of the file to which to write
		@param overWrite - if true the file will be overwritten, if false, the file will be appended */
RETURN_CODE CFitWindowFileHandler::WriteFitWindow(const Evaluation::CFitWindow &window, const CString &fileName, bool overWrite){
	FILE *f = NULL;
	CString indent;

	// Open the file
	if(overWrite)
		f = fopen(fileName, "w");
	else
		f = fopen(fileName, "a+");

	// Check so that we could read from the file.
	if(NULL == f)
		return FAIL;

	fprintf(f, "<fitWindow name=\"%s\">\n", window.name);
	indent.Format("\t");

	fprintf(f, "%s<fitLow>%d</fitLow>\n", indent, window.fitLow);
	fprintf(f, "%s<fitHigh>%d</fitHigh>\n", indent, window.fitHigh);
	fprintf(f, "%s<polyOrder>%d</polyOrder>\n", indent, window.polyOrder);
	fprintf(f, "%s<fitType>%d</fitType>\n", indent, window.fitType);

	fprintf(f, "%s<channel>%d</channel>\n", indent, window.channel);
	fprintf(f, "%s<specLength>%d</specLength>\n", indent, window.specLength);

	fprintf(f, "%s<fOptShift>%d</fOptShift>\n",					indent, window.findOptimalShift);
	fprintf(f, "%s<UV>%d</UV>\n",												indent, window.UV);
	fprintf(f, "%s<shiftSky>%d</shiftSky>\n",						indent, window.shiftSky);
	fprintf(f, "%s<interlaceStep>%d</interlaceStep>\n", indent, window.interlaceStep);

	fprintf(f, "%s<nRef>%d</nRef>\n", indent, window.nRef);

	for(int i = 0; i < window.nRef; ++i){
		fprintf(f, "%s<ref name=\"%s\">\n", indent, window.ref[i].m_specieName);
		fprintf(f, "%s\t<path>%s</path>\n", indent, window.ref[i].m_path);

		fprintf(f, "%s\t<shiftOption>%d</shiftOption>\n", indent, window.ref[i].m_shiftOption);
		if(window.ref[i].m_shiftOption != Evaluation::SHIFT_FREE)
			fprintf(f, "%s\t<shiftValue>%lf</shiftValue>\n", indent, window.ref[i].m_shiftValue);

		fprintf(f, "%s\t<squeezeOption>%d</squeezeOption>\n", indent, window.ref[i].m_squeezeOption);
		if(window.ref[i].m_squeezeOption != Evaluation::SHIFT_FREE)
			fprintf(f, "%s\t<squeezeValue>%lf</squeezeValue>\n", indent, window.ref[i].m_squeezeValue);

		fprintf(f, "%s\t<columnOption>%d</columnOption>\n", indent, window.ref[i].m_columnOption);
		if(window.ref[i].m_columnOption != Evaluation::SHIFT_FREE)
			fprintf(f, "%s\t<columnValue>%lf</columnValue>\n", indent, window.ref[i].m_columnValue);

		fprintf(f, "%s</ref>\n", indent);
	}

	fprintf(f, "</fitWindow>\n");

	// close the file again
	fclose(f);

	return SUCCESS;
}

