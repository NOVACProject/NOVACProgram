#pragma once
#include <afxtempl.h>


class CNodeControlInfo
{
	class CControlInfo
	{
	public:
		int m_mainIndex;
		int m_mode;
		CString m_spectrometerID;
		CString m_cfgFilePath;
		CControlInfo(void);
		~CControlInfo(void);
		CControlInfo(int mainIndex,int mode,CString spectrometerID,CString cfgFilePath);
	};
public:
	//-------------------------public functions---------------------//
	//--------------------------------------------------------------//
	//--------------------------------------------------------------//
	CNodeControlInfo(void);
	~CNodeControlInfo(void);
	/**get the total number of nodes
	*return total number of nodes
	*/
	int GetNodeSum();
	/**get node status - run , off, sleep, run in special mode
	*@param mainIndex - the index in the array , also in the configuration file
	*return the status number
	*/
	int GetNodeStatus(int mainIndex);
	void GetNodeCfgFilePath(int mainIndex,CString& filePath);
	/**set node status - run , off, sleep, run in special mode
	*@param mainIndex - the index in the array , also in the configuration file
	*@param status		- the node operating status
	*@param filePath	- the file to be uploaded
	*/
	void SetNodeStatus(int mainIndex,int status, CString& filePath);
	/**fill node info into the private array*/
	void FillinNodeInfo(int mainIndex,int mode,const CString spectrometerID,CString cfgFilePath);
	int	 GetMainIndex(CString spectrometerID);
private:
	
	//-------------------------private variables---------------------//
	//--------------------------------------------------------------//
	//--------------------------------------------------------------//
	CArray<CControlInfo*,CControlInfo*> m_controlInfoArray;
	/**total number of nodes*/
	int m_totalNodeNum;
};
