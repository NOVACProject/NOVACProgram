#include "StdAfx.h"
#include "CfgTxtFileHandler.h"

using namespace FileHandler;

// ----------------- FIRST START BY DEFINING THE CLASS CMeasLine -----------
CCfgTxtFileHandler::CMeasLine::CMeasLine(){
	sprintf(baseName, "\0");
	this->chn = 0;
	this->expTime = -1;
	this->flag = 0;
	this->pos[0] = 0;
	this->pos[1] = 0;
	this->repetitions = 1;
	this->sum1 = 15;
	this->sum2 = 1;
}
CCfgTxtFileHandler::CMeasLine::~CMeasLine(){

}

// -----------------  THE CLASS CCfgTxtFileHandler -----------
CCfgTxtFileHandler::CCfgTxtFileHandler(void)
{
	this->Clear();
}

CCfgTxtFileHandler::~CCfgTxtFileHandler(void)
{
}

void CCfgTxtFileHandler::Clear(){
	this->m_batteryLimit = 9.0;
	this->m_compass = 0.0;
	this->m_coneAngle = 90.0;
	this->m_ftpServerIp[0] = 192;
	this->m_ftpServerIp[1] = 168;
	this->m_ftpServerIp[2] = 1;
	this->m_ftpServerIp[3] = 2;
	this->m_ftpServerPassword.Format("iht-1inks.");
	this->m_ftpServerTimeOut = 30000;
	this->m_ftpServerUserName.Format("novacUser");
	this->m_maxIntTime = 5000;
	this->m_minIntTime = 20;
	this->m_measurements.SetSize(50);
	this->m_measurementNum = 0;
	this->m_motorDelay = 400;
	this->m_motorSkip = 0;
	this->m_motorStepsComp[0] = 0;
	this->m_motorStepsComp[1] = 0;
	this->m_motorStepsPerRound[0] = 200;
	this->m_motorStepsPerRound[1] = 200;
	this->m_nMotors = 1;
	this->m_percent = 0.70;
	this->m_powerSave = 0;
	this->m_startChn = 0;
	this->m_stopChn = 2047;
	this->m_stratoAngle = 85;
	this->m_tiltX = 0;
	this->m_tiltY = 0;
}

/** Reads and parses a given cfg.txt - file. 
	The read in parameters will be stored in this object.*/
int CCfgTxtFileHandler::ReadCfgTxt(const CString &fileName){
	char nl[2]={ 0x0a, 0 };
	char lf[2]={ 0x0d, 0 };
	char *pt;
	char txt[256];

	this->Clear();

	// Open the cfg.txt - file
	FILE *f = fopen(fileName, "r");
	if(f == nullptr) {
		return 0;
    }

	while(fgets(txt,sizeof(txt)-1, f)){
		if(strlen(txt)>4 && txt[0]!='%'){
			pt = txt;
			if(pt = strstr(txt,nl))
				pt[0]=0;

			pt = txt;
			if(pt = strstr(txt,lf))
				pt[0] = 0;
		
			// 1. Parse the motor parameters...
			if(pt = strstr(txt,"STEPSPERROUND=")){
				pt = strstr(txt,"=");
				m_nMotors = sscanf(&pt[1],"%d %d",&m_motorStepsPerRound[0], &m_motorStepsPerRound[1]);
			}

			if(pt = strstr(txt,"MOTORSTEPSCOMP=")){
				pt = strstr(txt,"=");
				m_nMotors = sscanf(&pt[1],"%d %d",&m_motorStepsComp[0], &m_motorStepsComp[1]);
			}
			
			if(pt = strstr(txt,"DELAY=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_motorDelay);
			}

			if(pt = strstr(txt,"SKIPMOTOR=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_motorSkip);
			}
			
			// 2. Parse the channel-settings
			if(pt = strstr(txt,"STARTCHN=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_startChn);
			}
			if(pt = strstr(txt,"STOPCHN=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_stopChn);
			}

			// 3. Parse the exposure-time settings
			if(pt = strstr(txt,"PERCENT=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf",&m_percent);
			}
			if(pt = strstr(txt,"MAXINTTIME=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_maxIntTime);
			}
			if(pt = strstr(txt,"MININTTIME=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_minIntTime);
			}

			// 4. Parse the stratospheric-measurement settings
			if(pt = strstr(txt,"STRATOANGLE=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_stratoAngle);
			}

			// 5. Parse the energy-options...
			if(pt = strstr(txt,"POWERSAVE=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_powerSave);
			}
			if(pt = strstr(txt,"BATTERYLIMIT=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf",&m_batteryLimit);
			}
			
			// 6. Parse the FTP-server settings...
			if(pt = strstr(txt,"FTPTIMEOUT=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%d",&m_ftpServerTimeOut);
			}
			if(pt=strstr(txt,"SERVER=")){
				char username[1024], password[1024];
				pt=strstr(txt,"=");
				int nFields = sscanf(&pt[1],"%d.%d.%d.%d %s %s", 
							&m_ftpServerIp[0], &m_ftpServerIp[1], &m_ftpServerIp[2], &m_ftpServerIp[3], 
							username, password);
				if(nFields == 6){
					this->m_ftpServerUserName.Format("%s", username);
					this->m_ftpServerPassword.Format("%s", password);
				}else{
				}
			}

			// 6. Parse the geometry
			if(pt = strstr(txt,"CONEANGLE=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf",&m_coneAngle);
			}
			if(pt = strstr(txt,"COMPASS=")){
				pt = strstr(txt,"=");
				sscanf(&pt[1],"%lf %lf %lf",&m_compass, &m_tiltX, &m_tiltY);
			}
			
			// --- Finally parse the measurement lines ---
			if(pt = strstr(txt,"MEAS=")){
				pt = strstr(txt,"=");
				CMeasLine meas;
				int nFields = 0;
				if(m_nMotors == 1){
					nFields = sscanf(&pt[1],"%d %d %d %d %d %d %d %d ", &meas.pos, &meas.expTime, &meas.sum1, &meas.sum2, &meas.chn, &meas.baseName, &meas.repetitions, &meas.flag);
					if(nFields == 8){
						m_measurements.SetAtGrow(m_measurementNum, meas);
					}
				}else if(m_nMotors == 2){
					// ...
				}
			}
		}
	}

	// remember to close the file...
	fclose(f);
	
	return 1;
}

/** Writes a cfg.txt - file. */
int CCfgTxtFileHandler::WriteCfgTxt(const CString &fileName){

	return 0;
}
