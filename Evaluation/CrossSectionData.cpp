#include "StdAfx.h"
#include "CrossSectionData.h"
#include "../Common/Common.h"

using namespace Evaluation;

CCrossSectionData::CCrossSectionData(void)
{
	m_length = 0;
	m_crossSection.SetSize(2048); // make an initial guess how long the cross sections are
	m_waveLength.SetSize(2048); // make an initial guess how long the cross sections are
}

CCrossSectionData::~CCrossSectionData(void)
{
}

/** Sets the reference information at the given pixel */
void CCrossSectionData::SetAt(int index, double wavel, double value){
	this->m_waveLength.SetAtGrow(index, wavel);
	this->m_crossSection.SetAtGrow(index, value);
}

/** Sets the cross-section information to the values in the 
	supplied array */
void CCrossSectionData::Set(double *wavelength, double *crossSection, unsigned long pointNum){
	this->m_length = pointNum;
	for(unsigned int k = 0; k < pointNum; ++k){
		this->m_waveLength.SetAt(k, wavelength[k]);
		this->m_crossSection.SetAt(k, crossSection[k]);
	}
}

/** Sets the cross-section information to the values in the 
	supplied array */
void CCrossSectionData::Set(double *crossSection, unsigned long pointNum){
	this->m_length = pointNum;
	for(unsigned int k = 0; k < pointNum; ++k){
		double lambda = (double)k;

		this->m_waveLength.SetAt(k, lambda);
		this->m_crossSection.SetAt(k, crossSection[k]);
	}
}

/** Sets the cross-section information to the values in the 
	supplied array */
void CCrossSectionData::Set(MathFit::CVector &crossSection, unsigned long pointNum){
	this->m_length = pointNum;
	for(unsigned int k = 0; k < pointNum; ++k){
		double value = crossSection.GetAt(k);

		this->m_crossSection.SetAt(k, value);
	}
}

/** Gets the cross section at the given pixel */
double CCrossSectionData::GetAt(unsigned int index) const{
	if(index > m_length)
		return 0.0;
	else{
		return m_crossSection.GetAt(index);
	}
}

unsigned long CCrossSectionData::GetSize() const{
	return this->m_length;
}

/** Gets the wavelength at the given pixel */
double CCrossSectionData::GetWavelengthAt(unsigned int index) const{
	if(index > this->m_length)
		return 0.0;
	else{
		return m_waveLength.GetAt(index);
	}
}

/** Reads the cross section from a file */
int CCrossSectionData::ReadCrossSectionFile(const CString &fileName){
	CFileException exceFile;
	CStdioFile fileRef;
	CString szLine;
	int valuesReadNum, nColumns;
	double fValue1[MAX_SPECTRUM_LENGTH];
	double fValue2[MAX_SPECTRUM_LENGTH];

	if(!fileRef.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile))
	{
		CString str;
		str.Format("ERROR: Cannot open reference file: %s", (LPCSTR)fileName);
		ShowMessage(str);
		return 1; /* Failed to open the file */
	}

	valuesReadNum = 0;
	nColumns = 1;

	// read reference spectrum into the 'fValue's array
	while(fileRef.ReadString(szLine))
	{
		// this construction enables us to read files with both one or two columns
		nColumns = sscanf(szLine, "%lf\t%lf", &fValue1[valuesReadNum], &fValue2[valuesReadNum]);

		// check so that we actually could read the data
		if(nColumns < 1 || nColumns > 2)
			break;

		++valuesReadNum;

		// if we have read as much as we can, return
		if(valuesReadNum == MAX_SPECTRUM_LENGTH)
			break;
	}
	if (valuesReadNum == 0) {
		return 1; // failed to read any lines
	}
	fileRef.Close();

	m_length = valuesReadNum;
	m_waveLength.SetSize(m_length);
	m_crossSection.SetSize(m_length);

	if(nColumns == 1){
		// If there's no wavelength column in the cross-section file
		for(int index = 0; index < valuesReadNum; ++index){
			double lambda = (double)index;
			
			m_waveLength.SetAt(index, lambda);
			m_crossSection.SetAt(index, fValue1[index]);
		}
	}else{
		// If there's a wavelength column in the cross-section file
		for(int index = 0; index < valuesReadNum; ++index){
			m_waveLength.SetAt(index, fValue1[index]);
			m_crossSection.SetAt(index, fValue2[index]);
		}
	}

	return 0;
}

/** Assignment operator*/
CCrossSectionData &CCrossSectionData::operator=(const CCrossSectionData &xs2){
	this->m_length = xs2.m_length;
	
	// make sure the arrays are the same size
	this->m_crossSection.SetSize(xs2.m_crossSection.GetSize());
	this->m_waveLength.SetSize(xs2.m_waveLength.GetSize());

	// copy the data of the arrays
	for(unsigned int i = 0; i < m_length; ++i){
		double lambda = xs2.GetWavelengthAt(i);
		double value = xs2.GetAt(i);

		this->m_crossSection.SetAt(i, value);
		this->m_waveLength.SetAt(i, lambda);
	}
	
	return *this;
}
