#include "WorkerThread.h"
#include "h264wh.h"
#include <onvifLibInterface.h>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>
#pragma comment(lib, "onvifLibEx.lib")
#define  CHECK_NVP_CONTEXT(context, ret)								  \
	if (!context)                    							  		  \
	{															  		  \
		qDebug()<<__FUNCTION__<<__LINE__<<"rtsp_context is null"; 		  \
		if (ret)                                                          \
		{                                                                 \
			*ret = 1;                                                     \
		}                                                                 \
		return 1;												  		  \
	}
	

#define  CHECK_RTSP_CONTEXT(context, ret)							  	  \
	if (!context)                    							  		  \
	{															  		  \
		qDebug()<<__FUNCTION__<<__LINE__<<"rtsp_context is null"; 		  \
		if (ret)                                                          \
		{                                                                 \
			*ret = 1;                                                     \
		}                                                                 \
		return 1;												  		  \
	}

#define CHECK_RESULT(ret, result)										  \
	if (ret)													 		  \
	{															 		  \
		qDebug()<<__FUNCTION__<<__LINE__<<"ret = "<<ret;		 		  \
		if (result)                                                       \
		{                                                                 \
			*result = 1;                                                  \
		}                                                                 \
		return 1;												 		  \
	}															 		  \
	else														 		  \
	{															 		  \
		qDebug()<<__FUNCTION__<<__LINE__<<"ret = 0";;			 		  \
		if (result)                                                       \
		{                                                                 \
			*result = 1;                                                  \
		}                                                                 \
		return 0;												 		  \
	}															 

WorkerThread::WorkerThread()
	: QObject(),
	m_enStatus(CONNECT_STATUS_DISCONNECTED),
	m_rtspContext(NULL),
	m_nvpContext(NULL),
	m_nvpVerify(NULL),
	m_bIgnoreEvent(false),
	m_bMotionDetect(false)
{
	m_sEventList<<"LiveStream"<<"Authority"<<"CurrentStatus"<<"MDSignal";

	QString sIp;
	getLocalIp(sIp);
	char*  cIp;
	QByteArray tByte = sIp.toLatin1();    
	cIp=tByte.data();
	ONVIF_CLIENT_initEx(1, 1, 1, false, 2,cIp,0);
}

WorkerThread::~WorkerThread()
{
	//if (m_nvpContext)
	//{
	//	NVP_ONVIF_delete(m_nvpContext);
	//	m_nvpContext = NULL;
	//}
	onvifDeleteContext();
	if (m_rtspContext)
	{
		MINIRTSP_delete(m_rtspContext);
		m_rtspContext = NULL;
	}
	ONVIF_CLIENT_deinitEx();
	if (m_bMotionDetect==true)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"the system may be crash as it do not call CancelEvent() function";
		abort();
	}
}

int WorkerThread::ConnectToDevice(int *result)
{
	qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<"start =======";

	if (CONNECT_STATUS_CONNECTED == m_enStatus)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<"connection has existed";
		*result = 0;
		return 0;
	}
	//if (m_nvpContext)
	//{
	//	NVP_ONVIF_delete(m_nvpContext);
	//	m_nvpContext = NULL;
	//}
	onvifDeleteContext();
	m_nvpContext = NVP_ONVIF_new();
	
	if (!m_nvpContext)
	{
		*result = 1;
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<"create nvp context error";
		return 1;
	}
	m_nvpArguments.thiz = (void *)m_nvpContext;
	strncpy(m_nvpArguments.ip, m_tDeviceInfo.sIpAddr.toLatin1().data(), m_tDeviceInfo.sIpAddr.size() + 1);
	m_nvpArguments.port = m_tDeviceInfo.vPorts["media"].toInt();
	strncpy(m_nvpArguments.username, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sUsername.size() + 1);
	strncpy(m_nvpArguments.password, m_tDeviceInfo.sPassword.toLatin1().data(), m_tDeviceInfo.sPassword.size() + 1);
	m_nvpArguments.chn = 0;
	m_nvpStreamUrl.main_index = 0;
	m_nvpStreamUrl.sub_index = 1;
	m_nvpContext->SetNVPEventHook(&m_nvpArguments,NVP_AUTH_FAILED,authorityEventHook,this);
	//get rtsp url both main and sub stream
	int ret = m_nvpContext->GetRtspUri(&m_nvpArguments, &m_nvpStreamUrl);
	if (ret)
	{
		*result = 1;
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<m_tDeviceInfo.sIpAddr<<"get rtsp url fault";
		return 1;
	}
	//create rtsp context, default for sub stream
	m_rtspContext = MINIRTSP_client_new(m_nvpStreamUrl.sub_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, false);
	if (!m_rtspContext)
	{
		*result = 1;
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		//NVP_ONVIF_delete(m_nvpContext);
		//m_nvpContext = NULL;
		onvifDeleteContext();
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<m_tDeviceInfo.sIpAddr<<"create rtsp context fault";

		return 1;
	}
	//register event and data callback function
	MINIRTSP_set_event_hook(m_rtspContext, eventHook, this, 0);
	MINIRTSP_set_data_hook(m_rtspContext, (fMINIRTSP_DATA_HOOK)dataHook, this, 0);

	ret = MINIRTSP_connect(m_rtspContext);
	if (ret)
	{
		*result = 1;
		m_enStatus = CONNECT_STATUS_DISCONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<m_tDeviceInfo.sIpAddr<<"connect error";

		return 1;
	}
	else
	{
		*result = 0;
		m_enStatus = CONNECT_STATUS_CONNECTED;
		qDebug()<<__FUNCTION__<<__LINE__<<(int)this<<m_tDeviceInfo.sIpAddr<<"connect success!!!";

		return 0;
	}
}

int WorkerThread::Authority(int *ret)
{
	//create rtsp context, default for sub stream
	m_nvpVerify = MINIRTSP_client_new(m_nvpStreamUrl.sub_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, true);
	if (m_nvpVerify)
	{
		//this rtsp context is just for verify username and password
		MINIRTSP_delete(m_nvpVerify);
		m_nvpVerify = NULL;
		*ret = 0;
		return 0;
	}
	else
	{
		*ret = 1;
		return 1;
	}
}

int WorkerThread::Disconnect(int *result)
{
	m_enStatus = CONNECT_STATUS_DISCONNECTING;

	if (!m_rtspContext)
	{
		*result = 0;
		m_enStatus = CONNECT_STATUS_DISCONNECTED;

		return 0;
	}
	int ret = MINIRTSP_disconnect(m_rtspContext);
	if (ret)
	{
		*result = 1;
		m_enStatus = CONNECT_STATUS_DISCONNECTING;

		return 1;
	}
	MINIRTSP_delete(m_rtspContext);
	m_rtspContext = NULL;
	//NVP_ONVIF_delete(m_nvpContext);
	//m_nvpContext = NULL;
	onvifDeleteContext();
	*result = 0;
	m_enStatus = CONNECT_STATUS_DISCONNECTED;

	return 0;
}

ConnectStatus WorkerThread::getCurrentStatus()
{
	return m_enStatus;
}

void WorkerThread::setDeviceInfo( const DeviceInfo& devInfo )
{
	m_tDeviceInfo = devInfo;
}

void WorkerThread::setEventMap(const QMultiMap<QString,tagOnvifProInfo> &tEventMap)
{
	m_tEventMap = tEventMap;
}

int WorkerThread::GetLiveStream( int chl, int streamId, int *result )
{
	int ret = -1;
	CHECK_RTSP_CONTEXT(m_rtspContext, result);

	if (MAIN_STREAM == streamId)
	{
		//release old context and switch to main stream
		m_bIgnoreEvent=true;
		ret = MINIRTSP_disconnect(m_rtspContext);
		if (0!=ret)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"trans to main stream fault when disconnect";
			*result = 1;
			m_bIgnoreEvent=false;
			return 1;
		}
		MINIRTSP_delete(m_rtspContext);
		m_bIgnoreEvent=false;
		m_rtspContext = MINIRTSP_client_new(m_nvpStreamUrl.main_uri, MINIRTSP_TRANSPORT_OVER_RTSP, m_tDeviceInfo.sUsername.toLatin1().data(), m_tDeviceInfo.sPassword.toLatin1().data(), true, false);
		if (m_rtspContext)
		{
			//register event callback function
			MINIRTSP_set_event_hook(m_rtspContext, eventHook, this, 0);
			MINIRTSP_set_data_hook(m_rtspContext, (fMINIRTSP_DATA_HOOK)dataHook, this, 0);
			ret = MINIRTSP_connect(m_rtspContext);
			if (0==ret)
			{
				if (MINIRTSP_play(m_rtspContext)==0)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"trans to main stream success";
					*result = 0;
					m_enStatus = CONNECT_STATUS_CONNECTED;
					return 0;
				}else{

				}
			}
			qDebug()<<__FUNCTION__<<__LINE__<<"trans to main stream fault when reconnect";
			m_rtspContext = NULL;
			*result = 1;
			m_enStatus = CONNECT_STATUS_DISCONNECTED;
			return 1;
		}
		else
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"rtsp context has release, please reconnect and try again";
			m_enStatus = CONNECT_STATUS_DISCONNECTED;
			*result = 1;
			return 1;
		}
	}else{
		if (MINIRTSP_play(m_rtspContext)==0)
		{
			//do nothing
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"get live fail";
			m_enStatus = CONNECT_STATUS_DISCONNECTED;
			*result = 1;
			return 1;
		}
	}

	*result = 0;
	return 0;
}

int WorkerThread::PauseStream( bool pause, int *result )
{
	CHECK_RTSP_CONTEXT(m_rtspContext, result);
	int ret = -1;
	if (pause)
	{
		ret = MINIRTSP_pause(m_rtspContext);
	}
	else
	{
		ret = MINIRTSP_play(m_rtspContext);
	}
	CHECK_RESULT(ret, result);
}

int WorkerThread::StopStream( int *result )
{
	CHECK_RTSP_CONTEXT(m_rtspContext, result);
	int ret = MINIRTSP_disconnect(m_rtspContext);
	CHECK_RESULT(ret, result);
}

int WorkerThread::GetStreamCount( int *count )
{
	int result = 0;
	CHECK_NVP_CONTEXT(m_nvpContext, &result);
	int ret = m_nvpContext->GetVideoEncoderConfigs(&m_nvpArguments, &m_nvpStreamInfo);
	*count = m_nvpStreamInfo.nr;
	CHECK_RESULT(ret, &result);
}

int WorkerThread::GetStreamInfo( int nStreamId, QVariantMap& info, int *ret )
{
	if (nStreamId < 0 || nStreamId >= m_nvpStreamInfo.nr || m_nvpStreamInfo.nr <= 0)
	{
		*ret = 1;
		return 1;
	}
	info.insert("index", m_nvpStreamInfo.entry[nStreamId].index);
	info.insert("name", QString(m_nvpStreamInfo.entry[nStreamId].name));
	info.insert("enc_type", m_nvpStreamInfo.entry[nStreamId].enc_type);
	info.insert("width", m_nvpStreamInfo.entry[nStreamId].width);
	info.insert("height", m_nvpStreamInfo.entry[nStreamId].height);
	info.insert("fps", m_nvpStreamInfo.entry[nStreamId].enc_fps);
	*ret = 0;
	return 0;
}

void WorkerThread::recFrameData( void* pdata, unsigned int size, unsigned int timestamp, int datatype )
{
	QVariantMap tStreamInfo;
	stMINIRTSP_DATA_PROPERTY frameInfo;
	int ret = MINIRTSP_lookup_data(m_rtspContext, datatype, &frameInfo);

 	tStreamInfo.insert("pts", (quint64)timestamp*1000);
 	tStreamInfo.insert("length", size);
 	tStreamInfo.insert("data", (quintptr)pdata);
 	if (MD_TYPE_H264 == datatype)
 	{
 		//vedio
		if (isIFrame((char*)pdata,size))
		{
			tStreamInfo.insert("frametype", TYPE_VEDIO);
		}else{
			tStreamInfo.insert("frametype", TYPE_PFRAME);
		}
		int nWidth=0;
		int nHeight=0;
		if (GetWidthHeight((char*)pdata, size, &nWidth, &nHeight)==0)
		{
			tStreamInfo.insert("width", nWidth);
			tStreamInfo.insert("height", nHeight);
		}else{
			tStreamInfo.insert("width", frameInfo.h264.width);
			tStreamInfo.insert("height", frameInfo.h264.height);
		}
 		tStreamInfo.insert("vcodec", "H264");
 	}
 	else
 	{
 		//audio
 		tStreamInfo.insert("frametype", TYPE_AUDIO);
 		tStreamInfo.insert("samplerate", frameInfo.g711.sampleRate);
 		tStreamInfo.insert("samplewidth", frameInfo.g711.sampleSize);
 		tStreamInfo.insert("audiochannel", frameInfo.g711.channel);
 		tStreamInfo.insert("acodec", "g711");
 	}
 	tagOnvifProInfo tProInfo = m_tEventMap.value("LiveStream");
 	if (tProInfo.proc)
 	{
 		tProInfo.proc(QString("LiveStream"),tStreamInfo,tProInfo.pUser);
 	}else{
 		qDebug()<<__FUNCTION__<<__LINE__<<"LiveStream is not register";
 	}
}

int WorkerThread::PtzCtrl( NVP_PTZ_CMD cmd, int chl, int speed, bool bopen, int *result )
{
	Q_UNUSED(chl);
	if (!m_nvpContext)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"nvp context is null";
		*result = 1;
		return 1;
	}
	stNVP_PTZ_CONTROL ptzCtrl;
	memset(&ptzCtrl, 0, sizeof(stNVP_PTZ_CONTROL));
	ptzCtrl.index = 0;
	ptzCtrl.cmd = cmd;
	ptzCtrl.speed = bopen ? (speed + 1)*0.125 : 0.0f;
	ptzCtrl.step = 1;
	ptzCtrl.repeat = false;
	int ret = m_nvpContext->ControlPTZ(&m_nvpArguments, &ptzCtrl);
	CHECK_RESULT(ret, result);
}

void WorkerThread::recEventHook( int eventType,void *rParam )
{
	if (m_bIgnoreEvent==true)
	{
		return;
	}else{
		//keep going
	}
	if (eventType==MINIRTSP_EVENT_CONNECTED||eventType==MINIRTSP_EVENT_DISCONNECTED)
	{
		ConnectStatus tCurrentStatus;
		if (eventType==MINIRTSP_EVENT_CONNECTED)
		{
			tCurrentStatus=CONNECT_STATUS_CONNECTED;
		}else{
			tCurrentStatus=CONNECT_STATUS_DISCONNECTED;
		}
		if (tCurrentStatus!=m_enStatus)
		{
			m_enStatus=tCurrentStatus;
			QVariantMap tStreamInfo;
			tStreamInfo.insert("CurrentStatus",tCurrentStatus);
			tagOnvifProInfo tProInfo=m_tEventMap.value("CurrentStatus");
			if (tProInfo.proc)
			{
				tProInfo.proc(QString("CurrentStatus"),tStreamInfo,tProInfo.pUser);
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"LiveStream is not register";
			}
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
}

void WorkerThread::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return ;
	}else{
		tagOnvifProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return ;
	}
}

void WorkerThread::recAuthorityEventHook( int eventType,void *rParam )
{
	QVariantMap tAuthorityInfo;
	tagOnvifProInfo tProInfo=m_tEventMap.value("Authority");
	if (tProInfo.proc)
	{
		tProInfo.proc(QString("Authority"),tAuthorityInfo,tProInfo.pUser);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"LiveStream is not register";
	}
}

void WorkerThread::MotionDetection( bool bEnable, int *result )
{
	if (bEnable){
		if (!m_nvpContext){
			return;
		}
		m_nvpSubscribe.event = NVP_EVENT_MD;
		m_nvpSubscribe.hook = nvpMDEventHook;
		m_nvpSubscribe.hook_custom = (void *)this;
		m_nvpSubscribe.keeplive_time = 60;
		int ret = m_nvpContext->SubscribeEvent(&m_nvpArguments, &m_nvpSubscribe);
		if (ret){
			*result = 1;
			return;
		}
		*result = 0;
		m_bMotionDetect = bEnable;
	}else{
		if (!m_nvpContext){
			return;
		}
		int ret;
		if (m_bMotionDetect==true)
		{
			ret = m_nvpContext->CancelEvent(&m_nvpArguments, &m_nvpSubscribe);
		}else{
			//do nothing
		}

		if (ret){
			*result = 1;
			return;
		}
		*result = 0;
		m_bMotionDetect = bEnable;
	}
}

void WorkerThread::recNvpMDEventHook( int eventType, void *rParam )
{
	if (m_bMotionDetect){
		QVariantMap info;
		if (NVP_EVENT_MD == eventType){
			info.insert("signal", QVariant(true));
		}else{
			info.insert("signal", QVariant(false));
		}
		tagOnvifProInfo tProinfo = m_tEventMap.value("MDSignal");
		if (tProinfo.proc){
			tProinfo.proc(QString("MDSignal"), info, tProinfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"MDSignal callback is not register";
		}
	}
}

bool WorkerThread::getLocalIp( QString &sIp )
{
	QList<QNetworkInterface> tHostInterface;
	tHostInterface=QNetworkInterface::allInterfaces();
	QList<QNetworkInterface>::const_iterator it;
	for (it=tHostInterface.constBegin();it!=tHostInterface.constEnd();it++)
	{
		if (it->hardwareAddress()!=NULL&&it->hardwareAddress().count()==17&&it->flags().testFlag(QNetworkInterface::IsLoopBack)!=true)
		{
			QList<QNetworkAddressEntry>tHostEntry=it->addressEntries();
			QList<QNetworkAddressEntry>::const_iterator tItem;
			for (tItem=tHostEntry.constBegin();tItem!=tHostEntry.constEnd();tItem++)
			{
				if (tItem->ip().protocol()==QAbstractSocket::IPv4Protocol)
				{
					sIp=tItem->ip().toString();
					return true;
				}
			}
		}
	}
	return false;
}

void WorkerThread::onvifDeleteContext()
{
	if (m_nvpContext)
	{
		if (m_bMotionDetect==true)
		{
			m_bMotionDetect=false;
			 m_nvpContext->CancelEvent(&m_nvpArguments, &m_nvpSubscribe);
		}else{
			//do nothing
		}
		NVP_ONVIF_delete(m_nvpContext);
		m_nvpContext = NULL;
	}
}

void eventHook( int eventType, int lParam, void *rParam, void *customCtx )
{
	((WorkerThread*)customCtx)->recEventHook(eventType,rParam);
}

void dataHook( void *pdata, unsigned int dataSize, unsigned int timestamp, int dataType, void *customCtx )
{
	((WorkerThread*)customCtx)->recFrameData(pdata, dataSize, timestamp, dataType);
}

void authorityEventHook( int nEvent, unsigned int lparam, unsigned int rparam, void *custom /* top-level-param */ )
{
	((WorkerThread*)custom)->recAuthorityEventHook(nEvent,custom);
}

void nvpMDEventHook( int nEvent, unsigned int lparam, unsigned int rparam, void *custom )
{
	((WorkerThread*)custom)->recNvpMDEventHook(nEvent,custom);
}
