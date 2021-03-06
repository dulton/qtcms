#include "RemotePlaybackThread.h"


RemotePlaybackThread::RemotePlaybackThread(void):LpIDeviceGroupRemotePlayback(NULL)
{
}


RemotePlaybackThread::~RemotePlaybackThread(void)
{
	if (NULL!=LpIDeviceGroupRemotePlayback)
	{
		LpIDeviceGroupRemotePlayback->Release();
	}

}

int RemotePlaybackThread::SetParm( QString m_nsUserName,QString m_nsUserPwd,uint m_nuiPort,QString m_nHostAddress,QString m_nsEseeId)
{
	m_sUserName.clear();
	m_sUserPwd.clear();
	m_HostAddress.clear();
	m_sEseeId.clear();
	m_sUserName=m_nsUserName;
	m_sUserPwd=m_nsUserPwd;
	m_uiPort=m_nuiPort;
	m_HostAddress=m_nHostAddress;
	m_sEseeId=m_nsEseeId;
	return 0;
}

int RemotePlaybackThread::SetIDeviceGroupRemotePlaybackParm( IDeviceGroupRemotePlayback *lpIDeviceGroupRemotePlayback )
{
	if (NULL!=lpIDeviceGroupRemotePlayback)
	{
		lpIDeviceGroupRemotePlayback->QueryInterface(IID_IDeviceGroupRemotePlayback,(void**)&LpIDeviceGroupRemotePlayback);
	}
	return 0;
}

void RemotePlaybackThread::startSearchRecFileSlots( int nChannel,int nTypes,const QString & startTime,const QString & endTime )
{
	int nRet=1;
	if (NULL==LpIDeviceGroupRemotePlayback||startTime.isEmpty()||endTime.isEmpty())
	{
		goto finishRearch;
		return;
	}
	IDeviceClient *m_nIDeviceClient=NULL;
	LpIDeviceGroupRemotePlayback->QueryInterface(IID_IDeviceClient,(void**)&m_nIDeviceClient);
	if (NULL==m_nIDeviceClient)
	{
		goto finishRearch;
		return;
	}
	m_nIDeviceClient->checkUser(m_sUserName,m_sUserPwd);
	m_nIDeviceClient->setDeviceHost(m_HostAddress);
	m_nIDeviceClient->setDeviceId(m_sEseeId);
	m_nIDeviceClient->setDevicePorts(m_uiPort);
		nRet=m_nIDeviceClient->connectToDevice();
		if (1==nRet)
		{
			m_nIDeviceClient->Release();
			goto finishRearch;
		}
	m_nIDeviceClient->Release();
	IDeviceSearchRecord *m_DeviceSearchRecord = NULL;
	LpIDeviceGroupRemotePlayback->QueryInterface(IID_IDeviceSearchRecord,(void**)&m_DeviceSearchRecord);
	if (NULL==m_DeviceSearchRecord)
	{
		goto finishRearch;
	}
	{
		QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
		QDateTime end   = QDateTime::fromString(endTime,   "yyyy-MM-dd hh:mm:ss");
		nRet =m_DeviceSearchRecord->startSearchRecFile(nChannel,nTypes,start,end);
	}	
	if(0!=nRet){
		m_DeviceSearchRecord->Release();
		goto finishRearch;
	}
	m_DeviceSearchRecord->Release();
	return;
finishRearch:
	emit finishSearchRecFileSig();
	return;
}

void RemotePlaybackThread::GroupPlaySlots( int nTypes,const QString & startTime,const QString & endTime )
{
	int nRet=1;
	if (NULL==LpIDeviceGroupRemotePlayback||startTime.isEmpty()||endTime.isEmpty())
	{
		return;
	}
	IDeviceClient *m_nIDeviceClient=NULL;
	LpIDeviceGroupRemotePlayback->QueryInterface(IID_IDeviceClient,(void**)&m_nIDeviceClient);
	if (NULL==m_nIDeviceClient)
	{
		return ;
	}
	m_nIDeviceClient->checkUser(m_sUserName,m_sUserPwd);
	nRet=m_nIDeviceClient->closeAll();
	if (1==nRet)
	{
		m_nIDeviceClient->Release();
		return;
	}
	m_nIDeviceClient->setDeviceHost(m_HostAddress);
	m_nIDeviceClient->setDeviceId(m_sEseeId);
	m_nIDeviceClient->setDevicePorts(m_uiPort);
	nRet=m_nIDeviceClient->connectToDevice();
	if (1==nRet)
	{
		m_nIDeviceClient->Release();
		return;
	}
	m_nIDeviceClient->Release();


	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end   = QDateTime::fromString(endTime,   "yyyy-MM-dd hh:mm:ss");
	nRet=LpIDeviceGroupRemotePlayback->GroupPlay(nTypes,start,end);
	return;
}


