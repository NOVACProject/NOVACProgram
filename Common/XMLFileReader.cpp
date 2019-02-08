#include "StdAfx.h"
#include "XMLFileReader.h"
#include "../Common/Common.h"
using namespace FileHandler;

CXMLFileReader::CXMLFileReader(void)
{
	this->m_File = NULL;
	this->nLinesRead = 0;
	this->szToken = NULL;
}

CXMLFileReader::~CXMLFileReader(void)
{
}

void CXMLFileReader::SetFile(CStdioFile *file)
{
	m_File = file;
}

char *CXMLFileReader::NextToken(){
	char separators[] = "<>\t";
	static char *pt = NULL;
	szToken = NULL;

	if(nLinesRead == 0){
	// if this is the first call to this function
		m_File->ReadString(szLine, 4095);
		szToken = (char*)(LPCSTR)szLine;
	  
		pt = strtok(szToken, separators);
		++nLinesRead;
		return pt;
	}else{
		// this is not the first call to this function
		if(pt = strtok(szToken, separators))
		return pt;

		szLine[0] = 0;
		while(0 == strlen(szLine)){
		if(!m_File->ReadString(szLine, 4095))
			return NULL;
		}

		szToken = (char*)(LPCSTR)szLine;

		pt = strtok(szToken, separators);
		++nLinesRead;
	}
	return pt;
}
/** General parsing of a single, simple string item */
int CXMLFileReader::Parse_StringItem(const CString &label, CString &str){
	while(szToken = NextToken()){

		if(Equals(szToken, label))
		return 1;

        str.Format(szToken);
	}

	return 0;
}

int CXMLFileReader::Parse_StringItem(const CString &label, std::string &str)
{
    while (szToken = NextToken()) {

        if (Equals(szToken, label))
            return 1;

        str = std::string(szToken);
    }

    return 0;
}


int CXMLFileReader::Parse_LongItem(const CString &label, long &number){

	while(szToken = NextToken()){

		if(Equals(szToken, label))
		return 1;

	   
		number = _tstol(szToken);
	}

	return 0;
}
/** General parsing of a single, simple float item */
int CXMLFileReader::Parse_FloatItem(const CString &label, double &number){
	while(szToken = NextToken()){

		if(Equals(szToken, label))
		return 1;

		number = _tstof(szToken);
	}

	return 0;
}

/** General parsing of a single, simple integer item */
int CXMLFileReader::Parse_IntItem(const CString &label, int &number){
	while(szToken = NextToken()){

		if(Equals(szToken, label))
		return 1;

	   
		number = _tstoi(szToken);
	}

	return 0;
}

/** General parsing of a single, simple long integer item */
int CXMLFileReader::Parse_IPNumber(const CString &label, BYTE &ip0, BYTE &ip1, BYTE &ip2, BYTE &ip3){
	while(szToken = NextToken()){
		int i0, i1, i2,i3;

		if(Equals(szToken, label))
		return 1;

		if (sscanf(szToken, "%d.%d.%d.%d", &i0, &i1, &i2, &i3) == 4) {
			ip0 = i0;
			ip1 = i1;
			ip2 = i2;
			ip3 = i3;
		}
		else {
			return 1;
		}
	}

	return 0;
}
