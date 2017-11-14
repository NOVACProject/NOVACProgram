#include "StdAfx.h"
#include "txtfile.h"

using namespace SpectrumIO;

CTXTFile::CTXTFile(void)
{
}

CTXTFile::~CTXTFile(void)
{
}

/** Reads a spectrum from a TXT-file */
RETURN_CODE CTXTFile::ReadSpectrum(CSpectrum &spec, const CString &fileName){
	double col1, col2;

	// Open the file
	FILE *f = fopen(fileName, "r");
	if(f == NULL)
		return FAIL;

	// Clear all the information in the spectrum
	spec.Clear();

	// Simply read the spectrum, one pixel at a time
	int length = 0;
	while(length < MAX_SPECTRUM_LENGTH){
		int nCols = fscanf(f, "%lf %lf", &col1, &col2);
		if(nCols == 0)
			break;
		else if(nCols == 1)
			spec.m_data[length] = col1;
		else if(nCols == 2)
			spec.m_data[length] = col2;

		++length;
	}
	spec.m_length = length;

	// close the file before we return
	fclose(f);
	return SUCCESS;
}

/** Writes a spectrum to a TXT-file */
RETURN_CODE CTXTFile::WriteSpectrum(const CSpectrum *spec, const CString &fileName){
	if(spec == NULL)
		return FAIL;
	return WriteSpectrum(*spec, fileName);
}

/** Writes a spectrum to a TXT-file */
RETURN_CODE CTXTFile::WriteSpectrum(const CSpectrum &spec, const CString &fileName){
	// Open the file
	FILE *f = fopen(fileName, "w");
	if(NULL == f)
		return FAIL;

	// Write the spectrum, one data-point at a time
	for(int k = 0; k < spec.m_length; ++k){
		if(0 > fprintf(f, "%lf\r\n", spec.m_data[k])){
			// an error has occured, try to close the file
			fclose(f);
			return FAIL;
		}
	}

	fclose(f); // <-- close the file before we return
	return SUCCESS;
}


