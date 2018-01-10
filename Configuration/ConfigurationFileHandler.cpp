#include "StdAfx.h"
#include "configurationfilehandler.h"
#include "../VolcanoInfo.h"
#include "../resource.h"

using namespace FileHandler;

extern CVolcanoInfo g_volcanoes;

CConfigurationFileHandler::CConfigurationFileHandler(void)
{
	conf = NULL;
}

CConfigurationFileHandler::~CConfigurationFileHandler(void)
{
}

/** Reads the configuration file and stores the found values in the 
		supplied reference 'conf'. If fileName is not-null the reader will 
		read from that file, otherwise it will read from 'configuration.xml' 
		in the same directory as the executable. */
int CConfigurationFileHandler::ReadConfigurationFile(CConfigurationSetting &configuration, const CString *fileName){
	CString tmp_fileName;
	CFileException exceFile;
	CStdioFile file;

	this->conf = &configuration;

	// 1. Get the filename
	if(fileName == NULL){
		Common common;
		common.GetExePath();
		tmp_fileName.Format("%sconfiguration.xml", common.m_exePath);
	}else{
		tmp_fileName.Format("%s", *fileName);
	}

	// 2. Open the file
	if(!file.Open(tmp_fileName, CFile::modeRead | CFile::typeText, &exceFile)){
		return 1;
	}
	this->m_File = &file;

	// 2b. Reset the current configuration
	conf->scannerNum = 0;
	this->nLinesRead = 0;

	// 3. Parse the file
	if(Parse()){
		// error in parsing
		file.Close();
		return 1;
	}else{
		// parsing was ok
		file.Close();
		CheckSettings();
		return 0;
	}
}

/** Writes the configuration file using the values stored in the 
		supplied reference 'conf'. If fileName is not-null the reader will 
		write to that file, otherwise it will write to 'configuration.xml' 
		in the same directory as the executable. */
int CConfigurationFileHandler::WriteConfigurationFile(CConfigurationSetting &configuration, const CString *fileName){
	CString indent, str, tmp_fileName;
	CString defaultFTPpwd;
	Common common;
	CString modelStr;

	defaultFTPpwd.Format("iht-1inks.");
	
	// 1. Get the filename
	if(fileName == NULL){
		common.GetExePath();
		tmp_fileName.Format("%sconfiguration.xml", common.m_exePath);
	}else{
		tmp_fileName.Format("%s", *fileName);
	}

	this->conf = &configuration;

	// 2. Open the file
	FILE *f = fopen(tmp_fileName, "w");
	if(NULL == f){
		MessageBox(NULL, TEXT(common.GetString(ERROR_COULD_NOT_OPEN_EVALCONFIG)) + "." + common.GetString(MSG_NO_CHANGES_WILL_BE_SAVED), common.GetString(MSG_ERROR), MB_OK);
		return 1;
	}

	// 3. Write the header information 
	fprintf(f, TEXT("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"));
	fprintf(f, TEXT("<!-- This is the configuration file for the evaluation of spectra in the NOVAC project.\n 	All spectrometers that are used must be defined here or their spectra will NOT be evaluated -->\n\n"));
	fprintf(f, TEXT("<Configuration>\n"));

	// 4. Write the global information

	// 4a. The output directory
	if(strlen(conf->outputDirectory) > 0)
		fprintf(f, TEXT("\t<outputDir>") + conf->outputDirectory + TEXT("</outputDir>\n"));

	// 4b. The startup method
	str.Format("\t<startup>%d</startup>\n", conf->startup);
	fprintf(f, str);
	// 4c. The ftp server setting - address
	str.Format("\t<ftpAddress>%s</ftpAddress>\n", conf->ftpSetting.ftpAddress);
	fprintf(f, str);
	 // 4d. The ftp server setting - username
	str.Format("\t<ftpUserName>%s</ftpUserName>\n", conf->ftpSetting.userName);
	fprintf(f, str);
	// 4e. The ftp server setting - password
	if(!Equals(conf->ftpSetting.password, defaultFTPpwd)){
		str.Format("\t<ftpPassword>%s</ftpPassword>\n", conf->ftpSetting.password);
		fprintf(f, str);
	}
	// 4e2. The settings for when to upload to the FTP-server...
	str.Format("\t<ftpStartTime>%d</ftpStartTime>\n", conf->ftpSetting.ftpStartTime);
	fprintf(f, str);
	str.Format("\t<ftpStopTime>%d</ftpStopTime>\n", conf->ftpSetting.ftpStopTime);
	fprintf(f, str);
	

	// 4f. Write if we should publish results
	if(conf->webSettings.publish){
		if(strlen(conf->webSettings.localDirectory) > 0){
			str.Format("\t<publish>1</publish>\n");
			str.AppendFormat("\t<localPublishDirectory>%s</localPublishDirectory>\n", conf->webSettings.localDirectory);
			fprintf(f, str);
		}
		str.Format("\t<publishFormat>%s</publishFormat>\n", conf->webSettings.imageFormat);
		fprintf(f, str);
	}

	// 4g. Write if we should call external softwares
	if(strlen(conf->externalSetting.fullScanScript) > 1){
		str.Format("\t<fullScanExecute>%s</fullScanExecute>\n", conf->externalSetting.fullScanScript);
		fprintf(f, str);
	}
	if(strlen(conf->externalSetting.imageScript) > 1){
		str.Format("\t<imageExecute>%s</imageExecute>\n", conf->externalSetting.imageScript);
		fprintf(f, str);
	}

	// 4h. If there is a wind-field file which should be re-read every now and then
	if(conf->windSourceSettings.windFieldFile.GetLength() > 3){
		str.Format("\t<windImport>\n");
		str.AppendFormat("\t\t<path>%s</path>\n",						conf->windSourceSettings.windFieldFile);
		str.AppendFormat("\t\t<reloadInterval>%d</reloadInterval>\n",	conf->windSourceSettings.windFileReloadInterval);
		str.AppendFormat("\t\t<enabled>%d</enabled>\n", conf->windSourceSettings.enabled);
		str.AppendFormat("\t</windImport>\n");
		fprintf(f, str);
	}

	// 5. Begin the device list
	fprintf(f, TEXT("\t<deviceList>\n"));

	// 6. Write the information about the scanning Instruments

	bool hasDoubleSpectrometer = false;
	for(unsigned int i = 0; i < conf->scannerNum; ++i){
		indent.Format("\t\t\t\t");

		fprintf(f, TEXT("\t\t<scanningInstrument>\n"));

		// observatory
		fprintf(f, TEXT("\t\t\t<observatory>" + conf->scanner[i].observatory + "</observatory>\n"));

		// If the user has configured a volcano, then also write its additional information
		bool ownerConfiguredSource = false;
		for(unsigned int k = g_volcanoes.m_preConfiguredVolcanoNum; k < g_volcanoes.m_volcanoNum; ++k){
			if(Equals(g_volcanoes.m_name[k], conf->scanner[i].volcano)){
				fprintf(f, TEXT("\t\t\t<sourceInfo>\n"));
				str.Format("\t\t\t\t<name>%s</name>\n",										g_volcanoes.m_name[k]);
				str.AppendFormat("\t\t\t\t<latitude>%lf</latitude>\n",		g_volcanoes.m_peakLatitude[k]);
				str.AppendFormat("\t\t\t\t<longitude>%lf</longitude>\n",	g_volcanoes.m_peakLongitude[k]);
				str.AppendFormat("\t\t\t\t<altitude>%lf</altitude>\n",		g_volcanoes.m_peakHeight[k]);
				fprintf(f, str);
				fprintf(f, TEXT("\t\t\t</sourceInfo>\n"));
				ownerConfiguredSource = true;
				break;
			}
		}
		if(!ownerConfiguredSource){
			// volcano
			fprintf(f, TEXT("\t\t\t<volcano>" + conf->scanner[i].volcano + "</volcano>\n"));
		}

		// site
		fprintf(f, TEXT("\t\t\t<site>" + conf->scanner[i].site + "</site>\n"));

		// position
		str.Format("\t\t\t<instr_latitude>%lf</instr_latitude>\n",			conf->scanner[i].gps.m_latitude);
		str.AppendFormat("\t\t\t<instr_longitude>%lf</instr_longitude>\n",	conf->scanner[i].gps.m_longitude);
		str.AppendFormat("\t\t\t<instr_altitude>%lf</instr_altitude>\n",	conf->scanner[i].gps.m_altitude);
		str.AppendFormat("\t\t\t<instr_compass>%lf</instr_compass>\n",		conf->scanner[i].compass);
		fprintf(f, str);

		// type
		str.Format("\t\t\t<type>%d</type>\n", conf->scanner[i].instrumentType);
		fprintf(f, str);

		// electronicsBox
		str.Format("\t\t\t<electronics>%d</electronics>\n", conf->scanner[i].electronicsBox);
		fprintf(f, str);

		// --- First write the spectrometer information
		for(unsigned int j = 0; j < conf->scanner[i].specNum; ++j)
		{
			CConfigurationSetting::SpectrometerSetting &spec = conf->scanner[i].spec[j];
			fprintf(f, TEXT("\t\t\t<spectrometer>\n"));
			
			// serial Number
			str.Format("%s<serialNumber>%s</serialNumber>\n", indent, spec.serialNumber);
			fprintf(f, str);

			// model Number
			CSpectrometerModel::ToString(spec.model, modelStr);
			str.Format("%s<model>%s</model>\n", indent, modelStr);
			fprintf(f, str);
			
			if(conf->scanner[i].spec[j].channelNum > 1)
				hasDoubleSpectrometer = true;

			// The channels
			for(unsigned int l = 0; l < conf->scanner[i].spec[j].channelNum; ++l){
				str.Format("%s<channel number=\"%d\">\n", indent, l);
				fprintf(f, str);

				// The species
				Evaluation::CFitWindow &window = spec.channel[l].fitWindow;
				for(int k = 0; k < window.nRef; ++k){
					fprintf(f, TEXT(indent + "\t<Reference>\n"));
					fprintf(f, TEXT(indent + "\t\t<name>" + window.ref[k].m_specieName + "</name>\n"));
					fprintf(f, TEXT(indent + "\t\t<path>" + window.ref[k].m_path + "</path>\n"));
					// writing the shift
					fprintf(f, TEXT(indent + "\t\t<shift>"));
					if(window.ref[k].m_shiftOption == Evaluation::SHIFT_FIX)
						str.Format("fix to %.2lf", window.ref[k].m_shiftValue);
					else if(window.ref[k].m_shiftOption == Evaluation::SHIFT_FREE)
						str.Format("free");
					else if(window.ref[k].m_shiftOption == Evaluation::SHIFT_LIMIT)
						str.Format("limit from %.2lf to %.2lf", window.ref[k].m_shiftValue, window.ref[k].m_shiftMaxValue);
					else if(window.ref[k].m_shiftOption == Evaluation::SHIFT_LINK)
						str.Format("link to %.0lf", window.ref[k].m_shiftValue);
					fprintf(f, str);
					fprintf(f, "</shift>\n");
					// writing the squeeze
					fprintf(f, TEXT(indent + "\t\t<squeeze>"));
					if(window.ref[k].m_squeezeOption == Evaluation::SHIFT_FIX)
						str.Format("fix to %.2lf", window.ref[k].m_squeezeValue);
					else if(window.ref[k].m_squeezeOption == Evaluation::SHIFT_FREE)
						str.Format("free");
					else if(window.ref[k].m_squeezeOption == Evaluation::SHIFT_LIMIT)
						str.Format("limit from %.2lf to %.2lf", window.ref[k].m_squeezeValue, window.ref[k].m_squeezeMaxValue);
					else if(window.ref[k].m_squeezeOption == Evaluation::SHIFT_LINK)
						str.Format("link to %.0lf", window.ref[k].m_squeezeValue);
					fprintf(f, str);
					fprintf(f, "</squeeze>\n");
					fprintf(f, TEXT(indent + "\t</Reference>\n"));
				}

				// fitLow
				str.Format("\t%s<fitLow>%ld</fitLow>\n", indent, spec.channel[l].fitWindow.fitLow);
				fprintf(f, str);

				// fitHigh
				str.Format("\t%s<fitHigh>%ld</fitHigh>\n", indent, spec.channel[l].fitWindow.fitHigh);
				fprintf(f, str);

				// The options for the dark-current
				if(spec.channel[l].m_darkSettings.m_darkSpecOption != MEASURE){
					// How to get the dark-spectrum
					str.Format("\t%s<dark>%d</dark>\n", indent, (int)spec.channel[l].m_darkSettings.m_darkSpecOption);
					fprintf(f, str);

					// How to use the offset spectrum
					str.Format("\t%s<offsetOption>%d</offsetOption>\n", indent, (int)spec.channel[l].m_darkSettings.m_offsetOption);
					fprintf(f, str);

					// The path of the offset-spectrum
					if(strlen(spec.channel[l].m_darkSettings.m_offsetSpec) > 1){
						str.Format("\t%s<offsetPath>%s</offsetPath>\n", indent, spec.channel[l].m_darkSettings.m_offsetSpec);
						fprintf(f, str);
					}

					// How to use the dark-current spectrum
					str.Format("\t%s<darkcurrentOption>%d</darkcurrentOption>\n", indent, (int)spec.channel[l].m_darkSettings.m_darkCurrentOption);
					fprintf(f, str);

					// The path of the dark-current spectrum
					if(strlen(spec.channel[l].m_darkSettings.m_darkCurrentSpec) > 1){
						str.Format("\t%s<darkCurrentPath>%s</darkCurrentPath>\n", indent, spec.channel[l].m_darkSettings.m_darkCurrentSpec);
						fprintf(f, str);
					}

				}
				// End of channel section
				str.Format("%s</channel>\n", indent);
				fprintf(f, str);
			}

			// End of spectrometer section
			fprintf(f, TEXT("\t\t\t</spectrometer>\n"));
		}

		// --- Then write the communication information
		{
			CConfigurationSetting::CommunicationSetting &comm = conf->scanner[i].comm;
			fprintf(f, TEXT("\t\t\t<communication>\n"));

			// connection type
			str.Format("%s<connection>%d</connection>\n", indent, comm.connectionType);
			fprintf(f, str);

			if(comm.connectionType == SERIAL_CONNECTION){
				// port
				str.Format("%s<port>%d</port>\n", indent, comm.port);
				fprintf(f, str);

				// baudrate
				str.Format("%s<baudrate>%d</baudrate>\n", indent, comm.baudrate);
				fprintf(f, str);

				// flowControl
				str.Format("%s<flowControl>%d</flowControl>\n", indent, comm.flowControl);
				fprintf(f, str);

				// The communication medium
				str.Format("%s<medium>", indent);
				switch(comm.medium){
					case MEDIUM_CABLE: str.AppendFormat("Cable"); break;
					case MEDIUM_FREEWAVE_SERIAL_MODEM: str.AppendFormat("Freewave"); break;
					case MEDIUM_SATTELINE_SERIAL_MODEM: str.AppendFormat("Satteline"); break;
				}
				str.AppendFormat("</medium>\n");
				fprintf(f, str);

				// radioID / callbook number
				str.Format("%s<radioID>%s</radioID>\n", indent, comm.radioID);
				fprintf(f, str);

			}
			if(comm.connectionType == FTP_CONNECTION){
				// IP-address of the scanning system
				str.Format("%s<IP>%d.%d.%d.%d</IP>\n", indent, comm.ftpIP[0], comm.ftpIP[1], comm.ftpIP[2], comm.ftpIP[3]);
				fprintf(f, str);

				// user name to log in on the scanning system
				str.Format("%s<username>%s</username>\n", indent, comm.ftpUserName);
				fprintf(f, str);

				// password to log in on the scanning system
				str.Format("%s<password>%s</password>\n", indent, comm.ftpPassword);
				fprintf(f, str);
			}

			// timeout
			str.Format("%s<timeout>%d</timeout>\n", indent, comm.timeout);
			fprintf(f, str);

			// query period
			str.Format("%s<queryPeriod>%d</queryPeriod>\n", indent, comm.queryPeriod);
			fprintf(f, str);
			// sleep time
			str.Format("%s<sleepTime>%d</sleepTime>\n", indent,
				comm.sleepTime.hour*3600 + comm.sleepTime.minute*60 + comm.sleepTime.second);
			fprintf(f, str);
			//wake up time
			str.Format("%s<wakeupTime>%d</wakeupTime>\n", indent,
				comm.wakeupTime.hour*3600 + comm.wakeupTime.minute*60 + comm.wakeupTime.second);
			fprintf(f, str);

			fprintf(f, TEXT("\t\t\t</communication>\n"));
		}

		// Write the motor information	
		//{
		//	indent.Format("\t\t\t\t");
		//	fprintf(f, TEXT("\t\t\t<motor>\n"));

		//	str.Format("%s<stepsperround1>%d</stepsperround1>\n", indent, conf->scanner[i].motor[0].stepsPerRound);
		//	fprintf(f, str);

		//	str.Format("%s<motorstepscomp1>%d</motorstepscomp1>\n", indent, conf->scanner[i].motor[0].motorStepsComp);
		//	fprintf(f, str);

		//	if(conf->scanner[i].instrumentType == INSTR_HEIDELBERG){
		//		str.Format("%s<stepsperround2>%d</stepsperround2>\n", indent, conf->scanner[i].motor[1].stepsPerRound);
		//		fprintf(f, str);

		//		str.Format("%s<motorstepscomp2>%d</motorstepscomp2>\n", indent, conf->scanner[i].motor[1].motorStepsComp);
		//		fprintf(f, str);
		//	}

		//	fprintf(f, TEXT("\t\t\t</motor>\n"));
		//}

		// .. Third: write the wind measurement settings, if applicable
		bool doWindMeasurements	= (hasDoubleSpectrometer || conf->scanner[i].instrumentType == INSTR_HEIDELBERG)&& conf->scanner[i].windSettings.automaticWindMeasurements;
		if(doWindMeasurements){
			indent.Format("\t\t\t\t");
			fprintf(f, TEXT("\t\t\t<windmeasurement>\n"));
			
			str.Format("%s<interval>%d</interval>\n", indent, conf->scanner[i].windSettings.interval);
			fprintf(f, str);

			str.Format("%s<duration>%d</duration>\n", indent, conf->scanner[i].windSettings.duration);
			fprintf(f, str);

			str.Format("%s<maxangle>%.2lf</maxangle>\n", indent, conf->scanner[i].windSettings.maxAngle);
			fprintf(f, str);

			str.Format("%s<stableperiod>%d</stableperiod>\n", indent, conf->scanner[i].windSettings.stablePeriod);
			fprintf(f, str);

			str.Format("%s<minpeakcolumn>%.2lf</minpeakcolumn>\n", indent, conf->scanner[i].windSettings.minPeakColumn);
			fprintf(f, str);

			str.Format("%s<desiredAngle>%.1lf</desiredAngle>\n", indent, conf->scanner[i].windSettings.desiredAngle);
			fprintf(f, str);

			str.Format("%s<useCalcWind>%d</useCalcWind>\n", indent, conf->scanner[i].windSettings.useCalculatedPlumeParameters);
			fprintf(f, str);
			
			str.Format("%s<switchRange>%.1lf</switchRange>\n", indent, conf->scanner[i].windSettings.SwitchRange);
			fprintf(f, str);

			fprintf(f, TEXT("\t\t\t</windmeasurement>\n"));
		}

		// ... Write the Real-time setup changes section, if applicable
		if(conf->scanner[i].instrumentType == INSTR_HEIDELBERG && conf->scanner[i].scSettings.automaticSetupChange){
			indent.Format("\t\t\t\t");
			fprintf(f, TEXT("\t\t\t<realtimesetup>\n"));
			
			str.Format("%s<usecalculatedplumeparam>%d</usecalculatedplumeparam>\n", indent, conf->scanner[i].scSettings.useCalculatedPlumeParameters);
			fprintf(f, str);

			str.Format("%s<mode>%d</mode>\n", indent, conf->scanner[i].scSettings.mode);
			fprintf(f, str);

			str.Format("%s<winddirtol>%.2lf</winddirtol>\n", indent, conf->scanner[i].scSettings.windDirectionTolerance);
			fprintf(f, str);

			fprintf(f, TEXT("\t\t\t</realtimesetup>\n"));
		}
			

		fprintf(f, TEXT("\t\t</scanningInstrument>\n"));

	}

	// Write the ending lines
	fprintf(f, TEXT("\t</deviceList>\n"));
	fprintf(f, TEXT("</Configuration>"));

	fclose(f);

	return 0;
}

int CConfigurationFileHandler::Parse(){

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// this can safely be ignored
		if(Equals(szToken, "deviceList")){
			continue;
		}
		if(Equals(szToken, "Configuration")){
			continue;
		}

		// ----------------------------------------------------
		// ---------------- Global Settings -------------------
		// ----------------------------------------------------
		if(Equals(szToken, "outputDir")){
			Parse_OutputDir();
			continue;
		}

		if(Equals(szToken, "startup")){
			Parse_IntItem(TEXT("/startup"), conf->startup);
			continue;
		}

		if(Equals(szToken,"ftpAddress")){
			Parse_StringItem(TEXT("/ftpAddress"),conf->ftpSetting.ftpAddress);
			continue;
		}
		if(Equals(szToken,"ftpUserName")){
			Parse_StringItem(TEXT("/ftpUserName"),conf->ftpSetting.userName);
			continue;
		}
		if(Equals(szToken,"ftpPassword")){
			Parse_StringItem(TEXT("/ftpPassword"),conf->ftpSetting.password);
			continue;
		}
		if(Equals(szToken,"ftpStartTime")){
			Parse_IntItem(TEXT("/ftpStartTime"),conf->ftpSetting.ftpStartTime);
			conf->ftpSetting.ftpStartTime = abs(conf->ftpSetting.ftpStartTime);
			continue;
		}
		if(Equals(szToken,"ftpStopTime")){
			Parse_IntItem(TEXT("/ftpStopTime"),conf->ftpSetting.ftpStopTime);
			conf->ftpSetting.ftpStopTime = abs(conf->ftpSetting.ftpStopTime);
			continue;
		}

		if(Equals(szToken, "publishFormat")){
			Parse_StringItem(TEXT("/publishFormat"), conf->webSettings.imageFormat);
			if(!Equals(conf->webSettings.imageFormat, ".png") && !Equals(conf->webSettings.imageFormat, ".bmp") && !Equals(conf->webSettings.imageFormat, ".gif") && !Equals(conf->webSettings.imageFormat, ".jpg")){
				conf->webSettings.imageFormat.Format(".png");
			}
			continue;
		}
		if(Equals(szToken, "publish")){
			Parse_IntItem(TEXT("/publish"), conf->webSettings.publish);
			continue;
		}
		if(Equals(szToken, "localPublishDirectory")){
			this->Parse_LocalPublishDir();
			continue;
		}

		if(Equals(szToken, "fullScanExecute")){
			this->Parse_StringItem(TEXT("/fullScanExecute"), conf->externalSetting.fullScanScript);
		}
		if(Equals(szToken, "imageExecute")){
			this->Parse_StringItem(TEXT("/imageExecute"), conf->externalSetting.imageScript);
		}

		if(Equals(szToken, "windImport")){
			this->Parse_WindImport();
		}

		// -----------------------------------------------------
		// ------------- Scanning Instrument Settings ----------
		// -----------------------------------------------------

		// starting a new 'scanningInstrument' section
		if(Equals(szToken, "scanningInstrument")){
			this->curScanner = &(conf->scanner[conf->scannerNum]);
			Parse_ScanningInstrument();
		}

	}

	return 0;
}

int CConfigurationFileHandler::Parse_ScanningInstrument(){
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

		// the end of the scanningInstrument section
		if(Equals(szToken, "/scanningInstrument")){
			++conf->scannerNum;
			return 0;
		}

		// a communication section
		if(Equals(szToken, "communication")){
			Parse_Communication();
		}

		// a spectrometer section
		if(Equals(szToken, "spectrometer")){
			Parse_Spectrometer();
		}

		// a motor section
		//if(Equals(szToken, "motor")){
		//	Parse_Motor();
		//}

		// a windmeasurement section
		if(Equals(szToken, "windmeasurement")){
			Parse_WindMeasurement();
		}

		// a real-time changing of cfg.txt section
		if(Equals(szToken, "realtimesetup")){
			this->Parse_RealTimeSetup();
		}

		// an observatory name for the scanning instrument
		if(Equals(szToken, "observatory")){
			Parse_StringItem(TEXT("/observatory"), curScanner->observatory);
			continue;
		}

		// a volcano name for the scanning instrument
		if(Equals(szToken, "volcano")){
			Parse_StringItem(TEXT("/volcano"), curScanner->volcano);
			continue;
		}

		// Additional information about the source
		if(Equals(szToken, "sourceInfo")){
			Parse_SourceInfo();
			continue;
		}

		// a site name for the scanning instrument
		if(Equals(szToken, "site")){
			Parse_StringItem(TEXT("/site"), curScanner->site);
			continue;
		}

		// the latitude for the scanning instrument
		if(Equals(szToken, "instr_latitude")){
			Parse_FloatItem(TEXT("/instr_latitude"), curScanner->gps.m_latitude);
			continue;
		}
		// the longitude for the scanning instrument
		if(Equals(szToken, "instr_longitude")){
			Parse_FloatItem(TEXT("/instr_longitude"), curScanner->gps.m_longitude);
			continue;
		}
		// the altitude for the scanning instrument
		if(Equals(szToken, "instr_altitude")){
			Parse_FloatItem(TEXT("/instr_altitude"), curScanner->gps.m_altitude);
			continue;
		}
		// the compass-direction for the scanning instrument
		if(Equals(szToken, "instr_compass")){
			Parse_FloatItem(TEXT("/instr_compass"), curScanner->compass);
			continue;
		}

		// the type of the instrument
		if(Equals(szToken, "type")){
			Parse_IntItem(TEXT("/type"), tmpInt);
			if(tmpInt == 0){
				curScanner->instrumentType = INSTR_GOTHENBURG;
			}else{
				curScanner->instrumentType = INSTR_HEIDELBERG;
			}
			continue;
		}

		// the type of the electronics box
		if(Equals(szToken, "electronics")){
			Parse_IntItem(TEXT("/electronics"), tmpInt);
			if(tmpInt == 0){
				curScanner->electronicsBox = BOX_VERSION_1;
			}else{
				curScanner->electronicsBox = BOX_VERSION_2;
			}
			continue;
		}
	}
	return 0;
}

int CConfigurationFileHandler::Parse_Communication(){
	CConfigurationSetting::CommunicationSetting *curComm = &(curScanner->comm);

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 2)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the communication section
		if(Equals(szToken, "/communication")){
			return 0;
		}

		if(Equals(szToken, "connection")){
			Parse_IntItem(TEXT("/connection"), curComm->connectionType);
			continue;
		}

	 if(Equals(szToken, "port")){
		 Parse_LongItem(TEXT("/port"),curComm->port);
			continue;
		}

	 if(Equals(szToken, "baudrate")){
		 Parse_LongItem(TEXT("/baudrate"),curComm->baudrate);
			continue;
		}

	 if(Equals(szToken, "medium")){
		 Parse_Medium(curComm);
			continue;
		}

	 if(Equals(szToken, "radioID")){
		 Parse_StringItem(TEXT("/radioID"),curComm->radioID);
			continue;
		}

	 if(Equals(szToken, "IP")){
		 Parse_IPNumber(TEXT("/IP"),curComm->ftpIP[0], curComm->ftpIP[1], curComm->ftpIP[2], curComm->ftpIP[3]);
			continue;
		}

	 if(Equals(szToken, "username")){
		 Parse_StringItem(TEXT("/username"),curComm->ftpUserName);
			continue;
		}

		if(Equals(szToken, "password")){
			Parse_StringItem(TEXT("/password"),curComm->ftpPassword);
			continue;
		}

	 if(Equals(szToken, "flowControl")){
		 Parse_IntItem(TEXT("/flowControl"),curComm->flowControl);
		 continue;
	 }

	

	 if(Equals(szToken, "timeout")){
		 Parse_LongItem(TEXT("/timeout"),curComm->timeout);
		 continue;
	 }

	 if(Equals(szToken, "queryPeriod")){
		 Parse_LongItem(TEXT("/queryPeriod"),curComm->queryPeriod);
		 continue;
	 }
	 if(Equals(szToken, "sleepTime")){
		 long sleepT;
		 Parse_LongItem(TEXT("/sleepTime"),sleepT);
		 curComm->sleepTime.hour = sleepT/3600;
		 curComm->sleepTime.minute = (sleepT%3600)/60;
		 curComm->sleepTime.second = (sleepT%3600)%60;

		 curComm->sleepTime.hour	 = max(min(curComm->sleepTime.hour, 23), 0);
		 curComm->sleepTime.minute = max(min(curComm->sleepTime.minute, 59), 0);
		 curComm->sleepTime.second = max(min(curComm->sleepTime.second, 59), 0);
		 continue;
	 }
	 if(Equals(szToken, "wakeupTime")){
		 long wakeupT;
		 Parse_LongItem(TEXT("/wakeupTime"),wakeupT);
		 curComm->wakeupTime.hour = wakeupT/3600;
		 curComm->wakeupTime.minute = (wakeupT%3600)/60;
		 curComm->wakeupTime.second = (wakeupT%3600)%60;

		 curComm->wakeupTime.hour	 = max(min(curComm->wakeupTime.hour, 23), 0);
		 curComm->wakeupTime.minute = max(min(curComm->wakeupTime.minute, 59), 0);
		 curComm->wakeupTime.second = max(min(curComm->wakeupTime.second, 59), 0);
		 continue;
	 }
	}

	return 0;
}

int CConfigurationFileHandler::Parse_Spectrometer(){
	CConfigurationSetting::SpectrometerSetting *curSpec = &(curScanner->spec[curScanner->specNum]);

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the spectrometer section
		if(Equals(szToken, "/spectrometer")){
			++curScanner->specNum;
			return 0;
		}

		// found the serial number 
		if(Equals(szToken, "serialNumber")){
			if(curSpec != NULL)
				Parse_StringItem(TEXT("/serialNumber"), curSpec->serialNumber);
			continue;
		}

		// found the model number 
		if(Equals(szToken, "model")){
			CString tmpStr2;
			if(curSpec != NULL){
				Parse_StringItem(TEXT("/model"), tmpStr2);
				curSpec->model = CSpectrometerModel::GetModel(tmpStr2);
			}
			continue;
		}
		
		if(Equals(szToken, "channel", 7)){
			if(curSpec != NULL)
				Parse_Channel();
			continue;
		}

		// ---- This section is here for backward compatibility only ----------
		if(Equals(szToken, "specie", 6)){
			if(curSpec != NULL)
				Parse_Specie(&curSpec->channel[curSpec->channelNum]);
			continue;
		}

		// ---- This section is here for backward compatibility only ----------
		if(Equals(szToken, "fitLow")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/fitLow"), curSpec->channel[curSpec->channelNum].fitWindow.fitLow);
			++curSpec->channelNum;
			continue;
		}

		// ---- This section is here for backward compatibility only ----------
		if(Equals(szToken, "fitHigh")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/fitHigh"), curSpec->channel[curSpec->channelNum].fitWindow.fitHigh);
			continue;
		}
	}
	return 0;
}

int CConfigurationFileHandler::Parse_Channel(){
	CConfigurationSetting::SpectrometerSetting *curSpec = &(curScanner->spec[curScanner->specNum]);
	CConfigurationSetting::SpectrometerChannelSetting *curChannel = &curSpec->channel[curSpec->channelNum];
	int tmpInt;

	// find the number for this channel.
	if(char *pt = strstr(szToken, "number")){
		if(pt = strstr(pt, "\"")){
			if(char *pt2 = strstr(pt+1, "\""))
				pt2[0] = 0; // remove the second quote
			if(sscanf(pt+1, "%d", &tmpInt)){
				curChannel = &curSpec->channel[tmpInt];
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

		if(Equals(szToken, "/channel")){
			++curSpec->channelNum;
			return 0;
		}

		if(Equals(szToken, "Reference", 9)){
			if(curSpec != NULL)
				Parse_Reference(curChannel);
			continue;
		}

		if(Equals(szToken, "specie", 6)){
			if(curSpec != NULL)
				Parse_Specie(curChannel);
			continue;
		}

		if(Equals(szToken, "fitLow")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/fitLow"), curChannel->fitWindow.fitLow);
			continue;
		}

		if(Equals(szToken, "fitHigh")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/fitHigh"), curChannel->fitWindow.fitHigh);
			continue;
		}

		if(Equals(szToken, "dark")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/dark"), tmpInt);
			curChannel->m_darkSettings.m_darkSpecOption = (DARK_SPEC_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "offsetOption")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/offsetOption"), tmpInt);
			curChannel->m_darkSettings.m_offsetOption = (DARK_MODEL_OPTION)tmpInt;
			continue;
		}
		
		if(Equals(szToken, "darkcurrentOption")){
			if(curSpec != NULL)
				Parse_IntItem(TEXT("/darkcurrentOption"), tmpInt);
			curChannel->m_darkSettings.m_darkCurrentOption = (DARK_MODEL_OPTION)tmpInt;
			continue;
		}

		if(Equals(szToken, "offsetPath")){
			if(curSpec != NULL)
				this->Parse_StringItem(TEXT("/offsetPath"), curChannel->m_darkSettings.m_offsetSpec);
			continue;
		}

		if(Equals(szToken, "darkCurrentPath")){
			if(curSpec != NULL)
				this->Parse_StringItem(TEXT("/darkCurrentPath"), curChannel->m_darkSettings.m_darkCurrentSpec);
			continue;
		}
	}
	return 0;
}

/** Parses the 'Specie' section */
int CConfigurationFileHandler::Parse_Specie(CConfigurationSetting::SpectrometerChannelSetting *curChannel){
	// find if there is a name for this specie
	if(char *pt = strstr(szToken, "name")){
		if(pt = strstr(pt, "\"")){
			if(char *pt2 = strstr(pt+1, "\""))
				pt2[0] = 0;		 //remove the second quote
			curChannel->fitWindow.ref[curChannel->fitWindow.nRef].m_specieName.Format("%s", pt+1);
		}
	}

	while(szToken = NextToken()){

		if(Equals(szToken, "/specie")){
			++curChannel->fitWindow.nRef;
			return 0;
		}

		// Read the path of the reference file.
		curChannel->fitWindow.ref[curChannel->fitWindow.nRef].m_path.Format(szToken);

	}

	return 0;
}

/** Parses the 'reference' section */
int CConfigurationFileHandler::Parse_Reference(CConfigurationSetting::SpectrometerChannelSetting *curChannel){
	int refIndex = curChannel->fitWindow.nRef;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the Reference section
		if(Equals(szToken, "/Reference")){
			++curChannel->fitWindow.nRef;
			return 0;
		}

		// found the specie name 
		if(Equals(szToken, "name")){
			if(curChannel != NULL)
				Parse_StringItem(TEXT("/name"), curChannel->fitWindow.ref[refIndex].m_specieName);
			continue;
		}

		// found the path to the reference file 
		if(Equals(szToken, "path")){
			if(curChannel != NULL)
				Parse_StringItem(TEXT("/path"), curChannel->fitWindow.ref[refIndex].m_path);
			continue;
		}

		// found the shift
		if(Equals(szToken, "shift")){
			Evaluation::CReferenceFile &ref = curChannel->fitWindow.ref[refIndex];
			if(curChannel != NULL)
				Parse_ShiftOrSqueeze(TEXT("/shift"), ref.m_shiftOption, ref.m_shiftValue, ref.m_shiftMaxValue);
			continue;
		}

		// found the squeeze
		if(Equals(szToken, "squeeze")){
			Evaluation::CReferenceFile &ref = curChannel->fitWindow.ref[refIndex];
			if(curChannel != NULL)
				Parse_ShiftOrSqueeze(TEXT("/squeeze"), ref.m_squeezeOption, ref.m_squeezeValue, ref.m_squeezeMaxValue);
			continue;
		}
	}

	return 0;
}

/** Parses a shift or squeeze section */
int CConfigurationFileHandler::Parse_ShiftOrSqueeze(const CString &label, Evaluation::SHIFT_TYPE &option, double &lowValue, double &highValue){
	char *pt = NULL;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of this section
		if(Equals(szToken, label)){
			return 0;
		}
		// convert the string to lowercase
		_strlwr(szToken);

		if(pt = strstr(szToken, "fix to")){
			sscanf(szToken, "fix to %lf", &lowValue);
			option = Evaluation::SHIFT_FIX;
		}else if(pt = strstr(szToken, "free")){
			option = Evaluation::SHIFT_FREE;
		}else if(pt = strstr(szToken, "limit")){
			sscanf(szToken, "limit from %lf to %lf", &lowValue, &highValue);
			option = Evaluation::SHIFT_LIMIT;
		}else if(pt = strstr(szToken, "link")){
			sscanf(szToken, "link %lf", &lowValue);
			option = Evaluation::SHIFT_LINK;
		}
	}
	return 0;
}

/** Parses the 'outputDir' section */
int CConfigurationFileHandler::Parse_OutputDir(){
	int ret = Parse_StringItem(TEXT("/outputDir"), conf->outputDirectory);
	if(conf->outputDirectory.GetAt(max(0, (int)strlen(conf->outputDirectory)-1)) != '\\')
		conf->outputDirectory.AppendFormat("\\");
	return ret;
}

/** Parses the section about the local directory where to publish results */
int CConfigurationFileHandler::Parse_LocalPublishDir(){
	int ret = Parse_StringItem(TEXT("/localPublishDirectory"), conf->webSettings.localDirectory);
	if(conf->webSettings.localDirectory.GetAt(max(0, (int)strlen(conf->webSettings.localDirectory)-1)) != '\\')
		conf->webSettings.localDirectory.AppendFormat("\\");
	return ret;
}

int CConfigurationFileHandler::Parse_Medium(CConfigurationSetting::CommunicationSetting *curComm){
	CString tmpStr;
	int ret = Parse_StringItem(TEXT("/medium"), tmpStr);
	if(Equals(tmpStr, "Cable")){
		curComm->medium = MEDIUM_CABLE; 
		return ret;
	}
	if(Equals(tmpStr, "Freewave")){
		curComm->medium = MEDIUM_FREEWAVE_SERIAL_MODEM;
		return ret;
	}
	if(Equals(tmpStr, "Satteline")){
		curComm->medium = MEDIUM_SATTELINE_SERIAL_MODEM;
		return ret;
	}
	// FAIL
	return 0;
}

/** Parses one 'windmeasurement' section */
int	CConfigurationFileHandler::Parse_WindMeasurement(){
	CConfigurationSetting::WindSpeedMeasurementSetting *wind = &(curScanner->windSettings);

	// the fact that there is a windmeasurement section
	//	in the configuration file means that we should automatically
	//	do wind measurements
	wind->automaticWindMeasurements = true;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the spectrometer section
		if(Equals(szToken, "/windmeasurement")){
			return 0;
		}

		// found the interval between the measurements
		if(Equals(szToken, "interval")){
			if(wind != NULL)
				Parse_IntItem("/interval", wind->interval);
			continue;
		}

		// found the duration of each measurement
		if(Equals(szToken, "duration")){
			if(wind != NULL)
				Parse_IntItem("/duration", wind->duration);
			continue;
		}

		// found the maxangle of each measurement
		if(Equals(szToken, "maxangle")){
			if(wind != NULL)
				Parse_FloatItem("/maxangle", wind->maxAngle);
			continue;
		}

		// found the time of required stability before performing a measurement
		if(Equals(szToken, "stableperiod")){
			if(wind != NULL)
				Parse_IntItem("/stableperiod", wind->stablePeriod);
			continue;
		}	

		// found the minimum required column 
		if(Equals(szToken, "minpeakcolumn")){
			if(wind != NULL)
				Parse_FloatItem("/minpeakcolumn", wind->minPeakColumn);
			continue;
		}

		// found the desired angle
		if(Equals(szToken, "desiredAngle")){
			if(wind != NULL)
				Parse_FloatItem("/desiredAngle", wind->desiredAngle);
			continue;
		}

		// found the settings for using calculated wind-fields...
		if(Equals(szToken, "useCalcWind")){
			if(wind != NULL)
				Parse_IntItem("/useCalcWind", wind->useCalculatedPlumeParameters);
			continue;
		}

		// found the desired angle
		if(Equals(szToken, "switchRange")){
			if(wind != NULL)
				Parse_FloatItem("/switchRange", wind->SwitchRange);
			continue;
		}
		// found the motor steps comp - THIS IS OUTDATED, THE PARAMETER IS NOW
		//	STORED IN THE MOTOR-PROPERTIES OF EACH OF THE SCANNERS
		//if(Equals(szToken, "motorstepscomp")){
		//	if(wind != NULL)
		//		Parse_IntItem("/motorstepscomp", curScanner->motor[0].motorStepsComp);
		//	continue;
		//}

	}
	return 0;
}

/** Parses one 'realtimesetup' section */
int CConfigurationFileHandler::Parse_RealTimeSetup(){
	CConfigurationSetting::SetupChangeSetting *scs = &(curScanner->scSettings);

	// the fact that there is a real-time setup-section
	//	in the configuration file means that we should automatically
	//	do real-time changes of the configuration-file
	scs->automaticSetupChange	= 1;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the spectrometer section
		if(Equals(szToken, "/realtimesetup")){
			return 0;
		}

		// should we use the calculated plume-heights and plume-directions?
		if(Equals(szToken, "usecalculatedplumeparam")){
			if(scs != NULL)
				Parse_IntItem("/usecalculatedplumeparam", scs->useCalculatedPlumeParameters);
			continue;
		}

		// the measurement mode
		if(Equals(szToken, "mode")){
			if(scs != NULL)
				Parse_IntItem("/mode", scs->mode);
			continue;
		}

		// the tolerance for changes in wind-direction
		if(Equals(szToken, "winddirtol")){
			if(scs != NULL)
				Parse_FloatItem("/winddirtol", scs->windDirectionTolerance);
			continue;
		}
	}
	return 0;
}

/** Parses the 'sourceInfo' section */
int	CConfigurationFileHandler::Parse_SourceInfo(){
	bool correct = false; // this is only true if the source is correctly configured
	int replace = -1; // this is non-negative if we should replace the already existing information with the one just read in
	Common common;
	CString name;
	double latitude, longitude, altitude;

	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the volcano-info section
		if(Equals(szToken, "/sourceInfo")){
			if(correct){
				curScanner->volcano.Format(name);
				if(replace == -1){
					g_volcanoes.m_name[g_volcanoes.m_volcanoNum].Format(name);
					g_volcanoes.m_simpleName[g_volcanoes.m_volcanoNum].Format(common.SimplifyString(name));
					g_volcanoes.m_peakHeight[g_volcanoes.m_volcanoNum]		= altitude;
					g_volcanoes.m_peakLatitude[g_volcanoes.m_volcanoNum]	= latitude;
					g_volcanoes.m_peakLongitude[g_volcanoes.m_volcanoNum]	= longitude;
					g_volcanoes.m_volcanoNum++;
				}else{
					g_volcanoes.m_name[replace].Format(name);
					g_volcanoes.m_simpleName[replace].Format(common.SimplifyString(name));
					g_volcanoes.m_peakHeight[replace]			= altitude;
					g_volcanoes.m_peakLatitude[replace]		= latitude;
					g_volcanoes.m_peakLongitude[replace]	= longitude;
				}
			}
			return 0;
		}

		// found the name of the source
		if(Equals(szToken, "name")){
			Parse_StringItem("/name", name);
			correct = true;
			// Check if this source already exists in our list, if so then just replace that information
			//	otherwise add it as a new source
			for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
				if(Equals(name, g_volcanoes.m_name[k])){
					replace = k;
					break;
				}
			}
			continue;
		}

		// found the latitude of the source
		if(Equals(szToken, "latitude")){
			Parse_FloatItem("/latitude", latitude);
			correct = true;
			continue;
		}

		// found the longitude of the source
		if(Equals(szToken, "longitude")){
			Parse_FloatItem("/longitude", longitude);
			correct = true;
			continue;
		}

		// found the altitude of the source
		if(Equals(szToken, "altitude")){
			Parse_FloatItem("/altitude", altitude);
			continue;
		}
	}
	return 0;
}

/** Parses the 'windImport' - section */
int CConfigurationFileHandler::Parse_WindImport(){
	// the actual reading loop
	while(szToken = NextToken()){

		// no use to parse empty lines
		if(strlen(szToken) < 3)
			continue;

		// ignore comments
		if(Equals(szToken, "!--", 3)){
			continue;
		}

		// the end of the windImport section
		if(Equals(szToken, "/windImport")){
			return 0;
		}

		// found the name of the wind-field file
		if(Equals(szToken, "path")){
			Parse_StringItem("/path", conf->windSourceSettings.windFieldFile);
			continue;
		}

		// found the reload interval of the wind-field file
		if(Equals(szToken, "reloadInterval")){
			Parse_LongItem("/reloadInterval", conf->windSourceSettings.windFileReloadInterval);
			continue;
		}

		// found the enabled flag wind-field file
		if (Equals(szToken, "enabled")) {
			Parse_IntItem("/enabled", conf->windSourceSettings.enabled);
			continue;
		}
	}
	return 0;
}

int CConfigurationFileHandler::CheckSettings(){

	// -------- FTP - SETTINGS -------------------
	// upload times
	if(conf->ftpSetting.ftpStopTime == conf->ftpSetting.ftpStartTime){
		// start-time is same as stoptime, change to have at least 3 hours uploading-time
		if(conf->ftpSetting.ftpStopTime > 86400 / 2){
			conf->ftpSetting.ftpStopTime	+= 3 * 3600;
		}else{
			conf->ftpSetting.ftpStartTime -= 3 * 3600;
		}
	}else if(conf->ftpSetting.ftpStopTime > conf->ftpSetting.ftpStartTime){
		// stopTime > startTime
		if(conf->ftpSetting.ftpStopTime - conf->ftpSetting.ftpStartTime < 3 * 3600){
			// less than 3 hours uploading time! change!
			conf->ftpSetting.ftpStopTime = (conf->ftpSetting.ftpStartTime + 3* 3600) % 86400;
		}
	}else{
		// stopTime < starTime
		if((86400 - conf->ftpSetting.ftpStartTime) + conf->ftpSetting.ftpStopTime < 3* 3600){
			// less than 3 hours uploading time! change!
			conf->ftpSetting.ftpStopTime = (conf->ftpSetting.ftpStartTime + 3* 3600) % 86400;
		}
	}

	return 0;
}

//int CConfigurationFileHandler::Parse_Motor(){
//
//	// the actual reading loop
//	while(szToken = NextToken()){
//
//		// no use to parse empty lines
//		if(strlen(szToken) < 3)
//			continue;
//
//		// ignore comments
//		if(Equals(szToken, "!--", 3)){
//			continue;
//		}
//
//		// the end of the motor section
//		if(Equals(szToken, "/motor")){
//			return 0;
//		}
//
//		// the number of steps per round for motor 1
//		if(Equals(szToken, "stepsperround1")){
//			Parse_IntItem("/stepsperround1", curScanner->motor[0].stepsPerRound);
//			continue;
//		}
//
//		// the number of steps per round for motor 2
//		if(Equals(szToken, "stepsperround2")){
//			Parse_IntItem("/stepsperround2", curScanner->motor[1].stepsPerRound);
//			continue;
//		}
//
//		// the motor steps compensation for motor 1
//		if(Equals(szToken, "motorstepscomp1")){
//			Parse_IntItem("/motorstepscomp1", curScanner->motor[0].motorStepsComp);
//			continue;
//		}
//
//		// the motor steps compensation for motor 2
//		if(Equals(szToken, "motorstepscomp2")){
//			Parse_IntItem("/motorstepscomp2", curScanner->motor[1].motorStepsComp);
//			continue;
//		}
//	}
//
//	return 0;
//}