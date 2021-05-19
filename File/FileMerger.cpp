#include "StdAfx.h"
#include "filemerger.h"

FileMerger::FileMerger(void)
{
	this->m_errorMsg = NULL;
}

FileMerger::~FileMerger(void)
{
	if(m_errorMsg != NULL)
	{
		delete m_errorMsg;
		m_errorMsg = NULL;
	}
}


/** Specifies the full path and filename
	of the file to output the data to */
void FileMerger::SetOutputFile(CString &outputFile)
{
	this->m_outputFile = CString(outputFile);
}

/** Specifies the full path and filename of the 
	files to merge */
void FileMerger::SetFilesToMerge(CList <CString, CString&> &filesToMerge)
{
	this->m_filesToMerge.RemoveAll();
	POSITION p = filesToMerge.GetHeadPosition();
	while(p != NULL)
		m_filesToMerge.AddTail(filesToMerge.GetNext(p));
}

const CString *FileMerger::GetErrorMessage()
{
	return this->m_errorMsg;
}

int FileMerger::MergeFiles()
{
#define BUFFERSIZE 4095
	char buffer[BUFFERSIZE];
	char newLine[] = "\r\n\0";

	if(m_errorMsg != NULL)
		delete m_errorMsg;
	m_errorMsg = new CString();

	// Check the input files
	if(m_filesToMerge.GetCount() <= 0)
		return 1;

	// Check the output file
	if(m_outputFile.GetLength() <= 3)
		return 2;

	// Open the output file
	FILE *fout = fopen(m_outputFile, "wb");
	if(fout == NULL)
		return 3;

	// Do the merging...
	POSITION pos = m_filesToMerge.GetHeadPosition();
	while(pos != NULL)
	{
		CString curFile = m_filesToMerge.GetNext(pos);

		FILE *fin = fopen(curFile, "rb");
		if(fin == NULL)
		{
			m_errorMsg->AppendFormat("Cannot open file: " + curFile + " for reading");
			continue;
		}

		size_t nBytesRead;
		do
		{
			nBytesRead = fread(buffer, sizeof(char), BUFFERSIZE, fin);
			fwrite(buffer, sizeof(char), nBytesRead, fout);
		}while(nBytesRead > 0);
		
		// add a new-line between each file
		fwrite(newLine, sizeof(char), strlen(newLine), fout);

		fclose(fin);
	}

	fclose(fout);

	return 0;  // success
}