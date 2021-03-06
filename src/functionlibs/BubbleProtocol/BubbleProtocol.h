#ifndef _BUBBLEPROTOCOL_HEADFILE_H_
#define _BUBBLEPROTOCOL_HEADFILE_H_


#include "BubbleProtocol_global.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include "IRemotePreview.h"
#include "IRemotePlayback.h"
#include "IDeviceConnection.h"
#include "IEventRegister.h"
#include "StreamProcess.h"
#include "IProtocolPTZ.h"

class BubbleProtocol : public QObject,
	public IEventRegister,
	public IRemotePreview,
    public IRemotePlayback,
	public IProtocolPTZ,
	public IDeviceConnection
{
	Q_OBJECT
		QThread m_workerThread;
public:
	BubbleProtocol(void);
	~BubbleProtocol(void);

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//interface for device connection
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDevicePorts(const QVariantMap & ports);
	virtual int setDeviceId(const QString & sAddress);
	virtual int setDeviceAuthorityInfomation(QString username,QString password);
	virtual int connectToDevice();
	virtual int authority();
	virtual int disconnect();
	virtual int getCurrentStatus();
	virtual QString getDeviceHost();
	virtual QString getDeviceid();
	virtual QVariantMap getDevicePorts();

	//interface for remote preview
	virtual int getLiveStream(int nChannel, int nStream);
	virtual int stopStream();
	virtual int pauseStream(bool bPaused);
	virtual int getStreamCount();
	virtual int getStreamInfo(int nStreamId,QVariantMap &streamInfo);

    //interface for remote record
    virtual int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime);
    virtual int getPlaybackStreamByTime(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime);
    virtual int getPlaybackStreamByFileName(int nChannel,const QString &sFileName);
    virtual int pausePlaybackStream(bool bPause);
    virtual int stopPlaybackStream();

 	//interface for event register
 	virtual QStringList eventList();
 	virtual int queryEvent(QString eventName,QStringList& eventParams);
 	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

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

public:
	//Custom Functions
	void sendRequire(bool bSwitch);
	void sendLiveStreamRequire(bool option);
	void sendLiveStreamRequireEx(bool option);

signals:
    void sigQuitThread();
    void sigEndStream();
    void sigWriteSocket(QByteArray block);
    void sigChildThreadToConn(QString address, quint16 port);
private slots:
    void remoteDataReady();
    void sendHeartBeat();
private:
    void eventProcCall(QString sEvent,QVariantMap param);
    int extractRecordInfo(QDomDocument*);
    int  writeBuff(QByteArray &, int, int, uint, uint);
    void setRecordInfo(Record&, QStringList);
    int  isFileExist(QString);
	int OperatePTZ( const unsigned int &uiChl, const int &nCmd, const int &nSpeed, bool bStart);
	int parseSearchData();
private:
	//member variable about device connection
	QTcpSocket m_remoteSearchfileTcpSocket;
	QByteArray             m_block;
	QHostAddress           m_hostAddress;
	QVariantMap            m_ports;
	QString                m_deviceId;
	QString                m_deviceUsername;
	QString                m_devicePassword;
    QList< QList<Stream> >   m_lstStreamList;
	bool                   m_isSupportBubble;

	QMultiMap<QString, ProcInfoItem> m_eventMap;
	QStringList   m_eventList;
    QTimer        m_timer;
    QList<Record> m_lstRecordList;
    int    m_nRecordNum;
    bool   m_bIsPostSuccessed;
	bool   m_bIsPreviewPaused;
    bool   m_bIsPreviewStopped;
    bool   m_bIsPlaybackPaused;
    bool   m_bIsPlaybackStopped;
	bool   m_bIsResearch;
	bool	m_bIsdataReady;
	int    m_channelNum;
	int    m_streamNum;
	int    m_streanCount;

	StreamProcess *m_pStreamProcess;

	int	   m_nRef;
	QMutex m_csRef;
	ReSearchInfo m_ReSearchInfo;
};

#endif
