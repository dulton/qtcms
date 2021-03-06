#ifndef ONVIFPROTOCOL_H
#define ONVIFPROTOCOL_H

#include "OnvifProtocol_global.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <IEventRegister.h>
#include <IDeviceSearch.h>
#include <IRemotePreview.h>
#include <IDeviceConnection.h>
#include <IProtocolPTZ.h>
#include <IRemoteMotionDetection.h>
#include "onvif.h"

#include "DeviceSearch.h"
#include "WorkerThread.h"

#pragma comment(lib, "libonvifc.lib")

// void cbSearchHook(const char *bind_host, unsigned char *ip,unsigned short port, char *name, char *location, char *firmware, void* customCtx);

class  OnvifProtocol:public QObject,
	public IEventRegister,
	public IDeviceSearch,
	public IRemotePreview,
	public IDeviceConnection,
	public IProtocolPTZ,
	public IRemoteMotionDetection
{
	Q_OBJECT
public:
	OnvifProtocol();
	~OnvifProtocol();

public:
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
	
	//interface for device search
	virtual int Start();
	virtual int Stop();
	virtual int Flush();
	virtual int setInterval(int nInterval);
	virtual IEventRegister * QueryEventRegister();

	//interface for device connection
	virtual int setDeviceHost(const QString &sIpAddr);
	virtual int setDevicePorts(const QVariantMap &tPorts);
	virtual int setDeviceId(const QString &sEseeId);
	virtual int setDeviceAuthorityInfomation(QString sUserName,QString sPassword);
	virtual int connectToDevice();
	virtual int authority();
	virtual int disconnect();
	virtual int getCurrentStatus();
	virtual QString getDeviceHost();
	virtual QString getDeviceid();
	virtual QVariantMap getDevicePorts();

	//interface for remote preview
	virtual int getLiveStream(int nChannel,int nStream);
	virtual int stopStream();
	virtual int pauseStream(bool bPause);
	virtual int getStreamCount();
	virtual int getStreamInfo(int nStreamId,QVariantMap &tStreamInfo);

	//interface for protocol ptz
	virtual int PTZUp(const int &nChl, const int &nSpeed);
	virtual int PTZDown(const int &nChl, const int &nSpeed);
	virtual int PTZLeft(const int &nChl, const int &nSpeed);
	virtual int PTZRight(const int &nChl, const int &nSpeed);
	virtual int PTZIrisOpen(const int &nChl, const int &nSpeed);
	virtual int PTZIrisClose(const int &nChl, const int &nSpeed);
	virtual int PTZFocusFar(const int &nChl, const int &nSpeed);
	virtual int PTZFocusNear(const int &nChl, const int &nSpeed);
	virtual int PTZZoomIn(const int &nChl, const int &nSpeed);
	virtual int PTZZoomOut(const int &nChl, const int &nSpeed);
	virtual int PTZAuto(const int &nChl, bool bOpend);
	virtual int PTZStop(const int &nChl, const int &nCmd);

	//interface for remote motion detection
	virtual int startMotionDetection();
	virtual int stopMotionDetection();
public:
	//�ص�����
	int cbCConnectStatus(QVariantMap evMap);
	int cbCLiveStream(QVariantMap evMap);
	int cbCAuthority(QVariantMap evMap);
	int cbMotionDetection(QVariantMap evMap);
signals:
	void sigConnectToDevice(int *result);
	void sigAuthority(int *result);
	void sigDisconnect(int *result);
	void sigGetLiveStream(int chl, int stream, int *result);
	void sigPauseStream(bool bPause, int *result);
	void sigStopStream(int *result);
	void sigGetStreamCount(int *count);
	void sigGetStreamInfo(int nStreamId, QVariantMap &info, int *result);
	void sigPtzCtrl(NVP_PTZ_CMD cmd, int chl, int speed, bool bopen, int *result);
	void sigAddEvent(const QMultiMap<QString,tagOnvifProInfo> &eventMap);
	void sigMotionDetect(bool bEnable, int* result);
private:
	void eventProcCall(QString sEvent,QVariantMap tInfo);
	void sleepEx(uint millisecond);
	void StatusChanged(ConnectStatus emStatus);

private:
	int m_nRef;
	QMutex m_csRef;

	QThread m_workThread;
	QStringList m_sEventList;
	QMultiMap<QString,tagOnvifProInfo> m_tEventMap;

	DeviceSearch *m_pDeviceSeach;
	WorkerThread *m_pWorkThread;
	DeviceInfo m_tDeviceInfo;
	volatile bool m_bSearchStoping;
	ConnectStatus m_emStatus;
	QMutex m_tStatusLock;
};

#endif // ONVIFPROTOCOL_H
