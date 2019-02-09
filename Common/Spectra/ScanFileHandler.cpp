#include "StdAfx.h"
#include "ScanFileHandler.h"
#include "../../SpectralEvaluation/Utils.h"

using namespace SpectrumIO;
using namespace FileHandler;

CScanFileHandler::CScanFileHandler(void)
{
	m_specReadSoFarNum = 0;
	m_initialized = false;
	m_logFileWriter = NULL;
	m_channel = 0;
	m_device.Format("");
	m_specNum = 0;
	
	m_spectrumBufferNum = 0;
}

CScanFileHandler::CScanFileHandler(FileHandler::CLogFileWriter &log){
	this->m_logFileWriter = &log;
	this->m_channel = 0;
	this->m_device.Format("");
	this->m_specReadSoFarNum = 0;
	this->m_initialized = false;
	this->m_specNum = 0;
	
	m_spectrumBufferNum = 0;
}

CScanFileHandler::~CScanFileHandler(void)
{
	m_spectrumBuffer.RemoveAll();
}

/** Checks the scan saved in the given filename
    @param fileName - the name of the file in which the spectra of the scan are saved */
RETURN_CODE CScanFileHandler::CheckScanFile(const CString *fileName){
	CSpectrumIO reader;
	reader.m_logFileWriter = this->m_logFileWriter;
	CString errMsg;
	CString strings[] = {CString("sky"), CString("zenith"), CString("dark"), CString("offset"), CString("dark_cur"), CString("darkcur")};
	int indices[] = {-1, -1, -1, -1, -1, -1};
	bool error = false;
	CSpectrum tempSpec;

	m_fileName.Format("%s", (LPCSTR)*fileName);

	// Count the number of spectra in the .pak-file
	m_specNum = reader.ScanSpectrumFile(m_fileName, strings, 6, indices);

	// Read in the spectra into the buffer, if the file is not too long
	if(m_specNum < 200){
		m_spectrumBuffer.SetSize(m_specNum);
		FILE *f = fopen(m_fileName, "rb");
		if(f != NULL){
			for(int k = 0; k < m_specNum; ++k){
				if(SUCCESS == reader.ReadNextSpectrum(f, tempSpec)){
					m_spectrumBuffer.SetAtGrow(k, tempSpec);
				}else{
					errMsg.Format("Could not read spectrum from file: %s", (LPCSTR)*fileName);
					ShowMessage(errMsg);
					this->m_lastError = reader.m_lastError;
					fclose(f);
					return FAIL;
				}
			}
			fclose(f);
		}
		m_spectrumBufferNum = m_specNum;
	}else{
		// The file's too large, don't store it in memory!
		m_spectrumBufferNum = 0;
		m_spectrumBuffer.RemoveAll();
	}

	// --------------- read the sky spectrum ----------------------
	if(indices[0] != -1){
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[0], m_sky)){
			error = true;
		}
	}else if(indices[1] != -1){
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[1], m_sky)){
			error = true;
		}
	}else if(SUCCESS != reader.ReadSpectrum(m_fileName, 0, m_sky)){
		error = true;
	}
	if(error){
		errMsg.Format("Could not read sky-spectrum in file: %s", (LPCSTR)*fileName);
		ShowMessage(errMsg);
		this->m_lastError = reader.m_lastError;
		return FAIL;
	}

	// --------------- read the dark spectrum ----------------------
	if(indices[2] != -1){
		// If there is a dark-spectrum specified, then read it!
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[2], m_dark)){
			error = true;
	}
	}else if(indices[3] == -1 && indices[4] == -1){
		// If there's no spectrum called 'dark' and no spectrum called 'dark_cur'
		//	and also no spectrum called 'offset', then we assume that something is wrong
		//	in the names and use the second spectrum in the scan as dark
		if(SUCCESS != reader.ReadSpectrum(m_fileName, 1, m_dark)){
			error = true;
		}
	}
	if(error){
		errMsg.Format("Could not read dark-spectrum in file: %s", (LPCSTR)*fileName);
		ShowMessage(errMsg);
		this->m_lastError = reader.m_lastError;
		return FAIL;
	}

	// --------------- read the offset spectrum (if any) ----------------------
	if(indices[3] != -1){
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[3], m_offset)){
			errMsg.Format("Could not read offset-spectrum in file: %s", (LPCSTR)*fileName);
			ShowMessage(errMsg);
			this->m_lastError = reader.m_lastError;
			return FAIL;
		}
	}

	// --------------- read the dark-current spectrum (if any) ----------------------
	if(indices[4] != -1){
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[4], m_darkCurrent)){
			errMsg.Format("Could not read offset-spectrum in file: %s", (LPCSTR)*fileName);
			ShowMessage(errMsg);
			this->m_lastError = reader.m_lastError;
			return FAIL;
		}
	}
	if(indices[5] != -1){
		if(SUCCESS != reader.ReadSpectrum(m_fileName, indices[5], m_darkCurrent)){
			errMsg.Format("Could not read offset-spectrum in file: %s", (LPCSTR)*fileName);
			ShowMessage(errMsg);
			this->m_lastError = reader.m_lastError;
			return FAIL;
		}
	}
	// set the start and stop time of the measurement
	if(SUCCESS == reader.ReadSpectrum(m_fileName, 0, tempSpec)){
		this->m_startTime = tempSpec.m_info.m_startTime;
		this->m_stopTime  = tempSpec.m_info.m_stopTime;

		// set the date for the measurement
        m_startTime.year  = tempSpec.m_info.m_startTime.year;
        m_startTime.month = tempSpec.m_info.m_startTime.month;
        m_startTime.day   = tempSpec.m_info.m_startTime.day;
        m_stopTime.year   = m_startTime.year;
        m_stopTime.month  = m_startTime.month;
        m_stopTime.day    = m_startTime.day;

		// get the serial number of the spectrometer
		m_device.Format(tempSpec.m_info.m_device.c_str());

		// get the channel of the spectrometer
		m_channel = tempSpec.m_info.m_channel;
	}

	// set the number of spectra that we have read from the file so far
	if(m_sky.ScanIndex() == 0 && m_dark.ScanIndex() == 1)
	  m_specReadSoFarNum = 2;
	else
		m_specReadSoFarNum = 0;

	// This CScanFileHandler object has been initialized
	m_initialized = true;

	return SUCCESS;
}

/** Returns the next spectrum in the scan */
int CScanFileHandler::GetNextSpectrum(CSpectrum &spec){
	CSpectrumIO reader;
	reader.m_logFileWriter = NULL;	// nowhere to output the error messages

	if(m_spectrumBufferNum == m_specNum){
		// We've read in the spectra into the buffer, just read it from there
		// instead of reading from the file itself.
		if(m_specReadSoFarNum >= m_spectrumBufferNum){
			this->m_lastError = SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND;
			++m_specReadSoFarNum; // <-- go to the next spectum
			return 0;
		}else{
			spec = m_spectrumBuffer.GetAt(m_specReadSoFarNum);
		}
	}else{
		// read the next spectrum in the file
		if(SUCCESS != reader.ReadSpectrum(m_fileName, m_specReadSoFarNum, spec)){
			// if there was an error reading the spectrum, set the error-flag
			this->m_lastError = reader.m_lastError;
			++m_specReadSoFarNum; // <-- go to the next spectum
			return 0;
		}
	}

	++m_specReadSoFarNum;

	// set the start and stop time of the measurement
	if(this->m_stopTime < spec.m_info.m_stopTime)
		this->m_stopTime = spec.m_info.m_stopTime;
	if(spec.m_info.m_startTime < this->m_startTime)
		this->m_startTime = spec.m_info.m_startTime;

	// Extract the spectrometer-model from the serial-number of the spectrometer
	if(Contains(spec.m_info.m_device, "D2J")){
		spec.m_info.m_specModel = S2000;
	}else if(Contains(spec.m_info.m_device, "I2J")){
		spec.m_info.m_specModel = S2000;
	}else if(Contains(spec.m_info.m_device, "USB2")){
		spec.m_info.m_specModel = USB2000;
	}else if(Contains(spec.m_info.m_device, "USB4C")){
		spec.m_info.m_specModel = USB4000;
	}else if(Contains(spec.m_info.m_device, "HR2")){
		spec.m_info.m_specModel = HR2000;
	}else if(Contains(spec.m_info.m_device, "HR4")){
		spec.m_info.m_specModel = HR4000;
	}else if(Contains(spec.m_info.m_device, "QE")){
		spec.m_info.m_specModel = QE65000;
	}else if (Contains(spec.m_info.m_device, "MAYAPRO")) {
		spec.m_info.m_specModel = MAYAPRO;
	}

	return 1;
}

/** Returns the desired spectrum in the scan */
int CScanFileHandler::GetSpectrum(CSpectrum &spec, long specNo){
	CSpectrumIO reader;
	reader.m_logFileWriter = NULL;	// nowhere to output the error messages

	if(m_spectrumBufferNum == m_specNum){
		// We've read in the spectra into the buffer, just read it from there
		// instead of reading from the file itself.
		if(m_specReadSoFarNum >= m_spectrumBufferNum){
			this->m_lastError = SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND;
			++m_specReadSoFarNum; // <-- go to the next spectum
			return 0;
		}else{
			spec = m_spectrumBuffer.GetAt(m_specReadSoFarNum);
		}
	}else{
		// read the sky and the dark spectra
		if(SUCCESS != reader.ReadSpectrum(m_fileName, specNo, spec)){
			this->m_lastError = reader.m_lastError;
			return 0;
		}
	}

	// set the start and stop time of the measurement
	if(this->m_stopTime < spec.m_info.m_stopTime)
		this->m_stopTime = spec.m_info.m_stopTime;
	if(spec.m_info.m_startTime < this->m_startTime)
		this->m_startTime = spec.m_info.m_startTime;

	return 1;
}
/** Gets the dark spectrum of the scan */
int CScanFileHandler::GetDark(CSpectrum &spec) const{
	spec = m_dark;
	return 0;
}

/** Gets the sky spectrum of the scan */
int CScanFileHandler::GetSky(CSpectrum &spec) const{
	spec = m_sky;
	return 0;
}

/** Gets the offset spectrum of the scan - if any */
int CScanFileHandler::GetOffset(CSpectrum &spec) const{
	spec = m_offset;
	return 0;
}

/** Gets the dark-current spectrum of the scan - if any */
int CScanFileHandler::GetDarkCurrent(CSpectrum &spec) const{
	spec = m_darkCurrent;
	return 0;
}

/** Retrieves GPS-information from the spectrum files */
const CGPSData &CScanFileHandler::GetGPS() const{
	return m_dark.GPS();
}

/** Retrieves compass-information from the spectrum files */
double CScanFileHandler::GetCompass() const{
	return m_dark.Compass();
}


/** Resets the m_specReadSoFarNum to 0 */
void  CScanFileHandler::ResetCounter(){
	m_specReadSoFarNum = 0;

	if(m_sky.ScanIndex() == 0)
		m_specReadSoFarNum = 1;

	if(m_dark.ScanIndex() == m_specReadSoFarNum)
		m_specReadSoFarNum += 1;
	
	if(m_offset.ScanIndex() == m_specReadSoFarNum || m_darkCurrent.ScanIndex() == m_specReadSoFarNum)
		m_specReadSoFarNum += 1;

	if(m_offset.ScanIndex() == m_specReadSoFarNum || m_darkCurrent.ScanIndex() == m_specReadSoFarNum)
		m_specReadSoFarNum += 1;
}

int CScanFileHandler::GetSpectrumNumInFile(){
	return 	m_specNum;
}
/** Returns the interlace steps for the spectra in this scan-file.
			@return the interlace steps for the spectra in this scan.
			@return -1 if the function 'CheckScanFile' has not been called */
int	CScanFileHandler::GetInterlaceSteps() const{
	if(!m_initialized)
		return -1;

	return this->m_sky.m_info.m_interlaceStep;
}

/** Returns the length of the spectra in this scan-file.
			@return the spectrum-length for the spectra in this scan.
			@return -1 if the function 'CheckScanFile' has not been called */
int	CScanFileHandler::GetSpectrumLength() const{
	if(!m_initialized)
		return -1;

	return this->m_sky.m_length;	
}

/** Returns the start-channel for the spectra in this scan-file.
		This is the pixel on the detector for which corresponds to the first
			datapoint in the spectra (normally 0). 
			@return the start-channel for the spectra in this scan.
			@return -1 if the function 'CheckScanFile' has not been called */
int	CScanFileHandler::GetStartChannel() const{
	if(!m_initialized)
		return -1;

	return this->m_sky.m_info.m_startChannel;
}
