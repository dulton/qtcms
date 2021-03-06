#include "bubbleprotocolex.h"
#include <guid.h>
#include <QtEndian>
#include "h264wh.h"
#include <QElapsedTimer>
#include <QCoreApplication>

int cbXBubbleFoundFile(QString evName,QVariantMap evMap,void*pUser);
int cbXBubbleRecFileSearchFail(QString evName,QVariantMap evMap,void*pUser);
int cbXBubbleRecFileSearchFinished(QString evName,QVariantMap evMap,void*pUser);

BubbleProtocolEx::BubbleProtocolEx():
    m_nRef(0),
    m_tCurrentConnectStatus(BUBBLE_DISCONNECTED),
    m_tHistoryConnectStatus(BUBBLE_DISCONNECTED),
    m_bStop(true),
	m_nSleepSwitch(0),
	m_nPosition(0),
    m_nSecondPosition(0),
    m_bBlock(false),
    m_pTcpSocket(NULL),
    m_bIsSupportHttp(true),
    m_bWaitForConnect(true),
	m_MdWorkObj(NULL)
{
	m_sEventList << "LiveStream"
		<<"SocketError"
		<<"StateChangeed"
		<<"foundFile"
		<<"recFileSearchFinished"
		<<"RecordStream"
		<<"SocketError"
		<<"StateChanged"
		<<"recFileSearchFail"
		<<"ConnectRefuse"
		<<"Authority"
		<<"MDSignal";
	connect(this,SIGNAL(sgBackToMainThread(QVariantMap)),this,SLOT(slBackToMainThread(QVariantMap)),Qt::BlockingQueuedConnection);
	connect(&m_tCheckoutBlockTimer,SIGNAL(timeout()),this,SLOT(slCheckoutBlock()));
	m_tCheckoutBlockTimer.start(5000);

	m_tPreviewCode.append((char)0x00);//Message
	m_tPreviewCode.append((char)0x01);//Live Stream
	m_tPreviewCode.append((char)0x02);//Heartbeat
	m_tPreviewCode.append((char)0x08);//Connect Refuse

	m_tRemoteCode.append((char)0x01);//Record Stream
	m_tRemoteCode.append((char)0x02);//Heart Beat
	m_tRemoteCode.append((char)0x09);//undefined
	//注册远程文件搜索回调事件
	m_tSearchRemoteFile.registerEvent("foundFile",cbXBubbleFoundFile,this);
	m_tSearchRemoteFile.registerEvent("recFileSearchFail",cbXBubbleRecFileSearchFail,this);
	m_tSearchRemoteFile.registerEvent("recFileSearchFinished",cbXBubbleRecFileSearchFinished,this);
    m_nBuiltTreadId=QThread::currentThreadId();
	//
	m_tBuffer.setSize(1.5*1024*1024);
}

BubbleProtocolEx::~BubbleProtocolEx()
{
	m_bStop=true;
	int nCount=0;
    Qt::HANDLE nCurrentThreadId=QThread::currentThreadId();
	QVariantMap tItem;
	tItem.insert("Timer",false);
	if (nCurrentThreadId==m_nBuiltTreadId)
	{
		slBackToMainThread(tItem);
	}else{
		emit sgBackToMainThread(tItem);
	}
	while(QThread::isRunning()){
		QTime dieTime=QTime::currentTime().addMSecs(10);
		while(QTime::currentTime()<dieTime){
			QCoreApplication::processEvents(QEventLoop::AllEvents,100);
			msleep(1);
		}
		msleep(1);
		nCount++;
		if (nCount>5000&&nCount%1000==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_tDeviceInfo.sEseeId<<"terminate this thread had caused more time than 5s,there may be out of control,"<<"please check position at:"<<m_nPosition;
		}
	}

}

long __stdcall BubbleProtocolEx::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}else if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister *>(this);
	}else if (IID_IRemotePreview==iid)
	{
		*ppv=static_cast<IRemotePreview*>(this);
	}else if (IID_IRemotePlayback==iid)
	{
		*ppv=static_cast<IRemotePlayback*>(this);
	}else if (IID_IDeviceConnection==iid)
	{
		*ppv = static_cast<IDeviceConnection *>(this);
	}
	else if (IID_IRemoteMotionDetection==iid)
	{
		*ppv = static_cast<IRemoteMotionDetection *>(this);
	}
	else if (IID_IProtocolPTZ == iid)
	{
		*ppv = static_cast<IProtocolPTZ *>(this);
	}
	else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall BubbleProtocolEx::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall BubbleProtocolEx::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

QStringList BubbleProtocolEx::eventList()
{
	return m_sEventList;
}

int BubbleProtocolEx::queryEvent( QString eventName,QStringList& eventParams )
{
    Q_UNUSED(eventParams);
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		//fix eventParams
		return IEventRegister::OK;
	}
}

int BubbleProtocolEx::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagBubbleProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

void BubbleProtocolEx::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagBubbleProInfo tProInfo=m_tEventMap.value(sEvent);
		if (NULL!=tProInfo.proc)
		{
			tProInfo.proc(sEvent,tInfo,tProInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is  undefined";
	}
}

void BubbleProtocolEx::run()
{
	int nRunStep=BUBBLE_RUN_CONNECT;
	bool bRunStop=false;
	m_bStop=false;
	int nHeartBeat=0;
	m_nPosition=__LINE__;
	m_tDeviceInfo.bRemotePlayPause=false;
	while(bRunStop==false){
		switch(nRunStep){
		case BUBBLE_RUN_CONNECT:{
			//连接到设备
			if (NULL!=m_pTcpSocket)
			{
				//qDebug()<<__FUNCTION__<<__LINE__<<"m_pTcpSocket should be null on here,please check";
				/*delete m_pTcpSocket;*/
				m_pTcpSocket=NULL;
			}else{
				//keep going  
			}
			m_pTcpSocket=new QTcpSocket;
			m_bBlock=true;
			m_nPosition=__LINE__;
			m_pTcpSocket->connectToHost(m_tDeviceInfo.tIpAddr.toString(),m_tDeviceInfo.tPorts["media"].toInt());
			//等待5s，如果超过5s没有返回，则认为连接失败
			if (m_pTcpSocket->waitForConnected(5000))
			{
				//套接字连接成功
				//下一步验证bubble是否支持使用http协议层来交互
				QString sBlock="GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
				m_pTcpSocket->write(sBlock.toAscii());
				//等待2s，如果超过2s写入套接字失败，则断开连接
				m_nPosition=__LINE__;
				if (m_pTcpSocket->waitForBytesWritten(2000))
				{
					//等待验证返回,如果一直处于连接状态，但没有任何信息返回，如何处理
					nRunStep=BUBBLE_RUN_DEFAULT;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_pTcpSocket->state()<<m_pTcpSocket->error()<<"write block to m_pTcpSocket fail";
					nRunStep=BUBBLE_RUN_DISCONNECT;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_pTcpSocket->state()<<m_pTcpSocket->error()<<"connect to device fail";
				nRunStep=BUBBLE_RUN_DISCONNECT;
			}
			m_bBlock=false;
			if (nRunStep==BUBBLE_RUN_DISCONNECT)
			{
				m_bWaitForConnect=false;
			}else{
				//keep going
			}
								}
								break;
		case BUBBLE_RUN_RECEIVE:{
			//接受解析码流
			int nReceiveStep=BUBBLE_RECEIVE_HTTP;
			bool bReceiveStop=false;
			/*m_tBuffer+=m_pTcpSocket->readAll();*/
			int nResidueSize=0;
			char *pData=m_tBuffer.getEndLocation(nResidueSize);
			int nAddDataLenth=m_pTcpSocket->read(pData,nResidueSize);
			if (nAddDataLenth>0)
			{
				m_tBuffer.addDataLenth(nAddDataLenth);
			}
			m_nPosition=__LINE__;
			while(bReceiveStop==false){
				switch(nReceiveStep){
				case BUBBLE_RECEIVE_HTTP:{
					//检测bubble是否能通过http传输
					//检测m_tBuffer 是否包含HTTP/1.1 200 或者 HTTP/1.1 404
					//忽略 m_tBuffer 大小小于12的情况
					if (m_tBuffer.contains("HTTP/1.1 200")||m_tBuffer.contains("HTTP/1.1 404"))
					{
						//检测是否够1024个字节
						if (m_tBuffer.size()<1024)
						{
							//接着接收
							nReceiveStep=BUBBLE_RECEIVE_WAITMOREBUFFER;
						}else{
							//解析
							m_bBlock=true;
							m_nPosition=__LINE__;
							if (analyzeBubbleInfo())
							{
								//keep going
								nReceiveStep=BUBBLE_RECEIVE_FRAME;
								m_tCurrentConnectStatus=BUBBLE_CONNECTED;
								m_bWaitForConnect=false;
							}else{
								//解析失败，断开连接
								nReceiveStep=BUBBLE_RECEIVE_END;
							}
							m_bBlock=false;
						}
					}else{
						nReceiveStep=BUBBLE_RECEIVE_FRAME;
					}
					   }
					   break;
				case BUBBLE_RECEIVE_FRAME:{
					//解析数据帧,第一次解析，buffer 大小不小于11
					//step1：查找帧头
					//step2:判断数据帧大小，超出500k，认为数据不合法，断开连接；等待buffer长度满足数据帧的大小
					//step3:查找指令码，如果没有相应的指令码，认为数据不合法，断开连接
					int nFrameStep=0;
					bool bFrameStop=false;
					while(bFrameStop==false){
						switch(nFrameStep){
						case 0:{
							//判断
							if (m_tBuffer.size()>=11)
							{
								if (m_tBuffer.startsWith("\xab")||m_tBuffer.startsWith("\xaa"))
								{
									//step1:find帧头
									tagBubbleInfo *pBubbleInfo=(tagBubbleInfo *)m_tBuffer.data();
                                    int nBubbleSize=qToBigEndian((quint32)(pBubbleInfo->uiLength))+sizeof(pBubbleInfo->cHead)+sizeof(pBubbleInfo->uiLength);
									if (nBubbleSize<1024*500)
									{
										//判断指令码
										if (m_tBuffer.startsWith("\xab"))
										{
											//回放
											if (m_tRemoteCode.contains(pBubbleInfo->cCmd))
											{
												if (m_tBuffer.size()>=nBubbleSize)
												{
													nFrameStep=1;
												}else{
													nFrameStep=2;
												}
											}else{
												//没有查询到相关的指令码，判定为解析失败
												qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as it(pBubbleInfo->cCmd) do not match any cmd code";
												nFrameStep=3;
											}	
										}else{
											//预览
											if (m_tPreviewCode.contains(pBubbleInfo->cCmd))
											{
												if (m_tBuffer.size()>=nBubbleSize)
												{
													nFrameStep=1;
												}else{
													nFrameStep=2;
												}
											}else{
												//没有查询到相关的指令码，判定为解析失败
												qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as it(pBubbleInfo->cCmd) do not match any cmd code";
												nFrameStep=3;
											}
										}
									}else{
										//数据帧大于500k，解析为错误数据，断开连接
										qDebug()<<__FUNCTION__<<__LINE__<<"analysis data fail as the buffer size is over 500k,we will reConnect the device";
										nFrameStep=3;
									}
								}else{
									m_tBuffer.remove(0,1);
									nFrameStep=0;
								}
							}else{
								nFrameStep=2;
							}
							   }
							   break;
						case 1:{
							//find
							if (m_tBuffer.startsWith("\xab"))
							{
								//解析回放数据
								m_bBlock=true;
								m_nPosition=__LINE__;
								if (analyzeRemoteInfo())
								{
									nFrameStep=0;
								}else{
									nFrameStep=3;
								}
								m_bBlock=false;
							}else{
								//解析预览数据
								if (analyzePreviewInfo())
								{
									nFrameStep=0;
								}else{
									nFrameStep=3;
								}
							}
							   }
							   break;
						case 2:{
							//需要接着接收数据
							nFrameStep=5;
							nReceiveStep=BUBBLE_RECEIVE_WAITMOREBUFFER;
							   }
							   break;
						case 3:{
							//解析失败
							nFrameStep=5;
							nReceiveStep=BUBBLE_RECEIVE_END;
							   }
							   break;
						case 4:{
							//接着解析数据 
							//nFrameStep=5;
							//nReceiveStep=BUBBLE_RECEIVE_HTTP;
							   }
						case 5:{
							//end
							bFrameStop=true;
							   }
							   break;
						}
					}
					   }
					   break;
				case BUBBLE_RECEIVE_WAITMOREBUFFER:{
					//等待更多的数据
					m_nPosition=__LINE__;
					nRunStep=BUBBLE_RUN_DEFAULT;
					bReceiveStop=true;
					   }
					   break;
				case BUBBLE_RECEIVE_END:{
					//停止接收数据
					m_nPosition=__LINE__;
					nRunStep=BUBBLE_RUN_DISCONNECT;
					bReceiveStop=true;
										}
										break;
				}
			}
								}
								break;
		case BUBBLE_RUN_CONTROL:{
			//控制指令
			m_bBlock=true;
			m_nPosition=__LINE__;
			m_csStepCode.lock();
			int nRunControlStep=m_tStepCode.dequeue();
			m_csStepCode.unlock();
			m_bBlock=false;
			bool bFlag=false;
			switch(nRunControlStep){
			case BUBBLE_AUTHORITY:{
				//用户验证
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdAuthority();
				m_bBlock=false;
								  }
								  break;
			case BUBBLE_DISCONNECT:{
				//断开连接
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdDisConnect();
				m_bBlock=false;
								   }
								   break;
			case BUBBLE_GETLIVESTREAM:{
				//获取预览码流
				m_bBlock=true;
				m_nPosition=__LINE__;
				if (m_bIsSupportHttp)
				{
					bFlag=cmdGetLiveStreamEx();
				}else{
					bFlag=cmdGetLiveStream();
				}
				m_bBlock=false;
									  }
									  break;
			case BUBBLE_STOPSTREAM:{
				//停止预览
				m_bBlock=true;
				m_nPosition=__LINE__;
				if (m_bIsSupportHttp)
				{
					bFlag=cmdStopStream();
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"old version do not support StopStream cmd,it will disConnect the device";
					bFlag=false;
				}
				m_bBlock=false;
				
								   }
								   break;
			case BUBBLE_PAUSESTREAM:{
				//暂停预览
				m_bBlock=true;
				m_nPosition=__LINE__;
				if (m_bIsSupportHttp)
				{
					bFlag=cmdPauseStream();
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"old version do not support PauseStream cmd,it will disConnect the device";
					bFlag=false;
				}
				m_bBlock=false;
									}
									break;
			case BUBBLE_HEARTBEAT:{
				//心跳指令
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdHeartBeat();
				m_bBlock=false;
								  }
								  break;
			case BUBBLE_GETPLAYBACKSTREAMBYTIME:{
				//获取回放码流
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdGetPlayBackStreamByTime();
				m_bBlock=false;
												}
												break;
			case BUBBLE_GETPLAYBACKSTREAMBYFILENAME:{
				//获取回放码流
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdGetPlayBackStreamByFileName();
				m_bBlock=false;
													}
													break;
			case BUBBLE_PAUSEPLAYBACKSTREAM:{
				//暂停回放码流
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdPausePlayBackStream();
				m_bBlock=false;
											}
											break;
			case BUBBLE_STOPPLAYBACKSTREAM:{
				//停止回放码流
				m_bBlock=true;
				m_nPosition=__LINE__;
				bFlag=cmdStopPlayBackStream();
				m_bBlock=false;
										   }
										   break;
			case BUBBLE_PTZ:{
				//云台控制
				m_bBlock=true;
				m_nPosition=__LINE__;
				if (!m_tBubblePtz.isEmpty())
				{
					tagPtzItem tPtzItem=m_tBubblePtz.dequeue();
					int nPtzStep=tPtzItem.nAction;
					switch(nPtzStep){
					case Ptz_Up:{
						//
						int nAction=0;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								}
								break;
					case Ptz_Down:{
						//
						int nAction=1;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								  }
								  break;
					case Ptz_Left:{
						//
						int nAction=2;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								  }
								  break;
					case Ptz_Right:{
						//
						int nAction=3;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								   }
								   break;
					case Ptz_IrisOpen:{
						//
						int nAction=9;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									  }
									  break;
					case Ptz_IrisClose:{
						//
						int nAction=10;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									   }
									   break;
					case Ptz_FocusFar:{
						//
						int nAction=5;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									  }
									  break;
					case Ptz_FocusNear:{
						//
						int nAction=6;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									   }
									   break;
					case Ptz_ZoomIn:{
						//
						int nAction=7;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									}
									break;
					case Ptz_ZoomOut:{
						//
						int nAction=8;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
									 }
									 break;
					case Ptz_Auto:{
						//
						int nAction=4;
						int nChannel=tPtzItem.nChannel;
						int nStart=1;
						int nParm=tPtzItem.nSpeed;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								  }
								  break;
					case Ptz_Stop:{
						//
						int nAction=tPtzItem.nSpeed;
						int nChannel=tPtzItem.nChannel;
						int nStart=0;
						int nParm=0;
						bFlag=cmdPtz(nAction,nChannel,nStart,nParm);
								  }
								  break;
					}
				}else{
					//do nothing
				}
				
				
				m_bBlock=false;
							}
							break;
			default:{
				qDebug()<<__FUNCTION__<<__LINE__<<"there is an undefined cmd,please check! it will cause device reConnect";
					}
			}
			if (bFlag)
			{
				nRunStep=BUBBLE_RUN_DEFAULT;	
			}else{
				nRunStep=BUBBLE_RUN_DISCONNECT;
				qDebug()<<__FUNCTION__<<__LINE__<<"send cmd fail,cmd::"<<nRunControlStep;
			}
			
								}
								break;
		case BUBBLE_RUN_DISCONNECT:{
			//断开连接
			m_bWaitForConnect=false;
			if (m_tCurrentConnectStatus==BUBBLE_CONNECTTING||m_tCurrentConnectStatus==BUBBLE_CONNECTED)
			{
				m_tCurrentConnectStatus=BUBBLE_DISCONNECTING;
				QVariantMap evMap;
				evMap.insert("status",evMap);
				m_bBlock=true;
				m_nPosition=__LINE__;
				emit sgBackToMainThread(evMap);
				m_bBlock=false;
			}else{
				//do nothing
			}
			//断开套接字连接的信号

			//断开套接字的网络连接
			if (NULL!=m_pTcpSocket)
			{
				if (QAbstractSocket::UnconnectedState!=m_pTcpSocket->state())
				{
					m_bBlock=true;
					m_nPosition=__LINE__;
					m_pTcpSocket->disconnectFromHost();
					//等待2s。超过2s没有断开返回，给出错误的提示信息

					if (QAbstractSocket::UnconnectedState==m_pTcpSocket->state()||m_pTcpSocket->waitForDisconnected(2000))
					{
						//成功断开
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"try to disconnectFromHost,but it had cost 2s,please check";
					}
					m_bBlock=false;
				}else{
					//do nothing
				}
				
			}else{
				//do nothing
			}
			nRunStep=BUBBLE_RUN_END;
								   }
								   break;
		case BUBBLE_RUN_DEFAULT:{
			//缺省动作
			//检测是否需要停止线程
			m_nPosition=__LINE__;
			if (!m_bStop&&NULL!=m_pTcpSocket)
			{
				//检测连接状态
				if (QAbstractSocket::ConnectedState==m_pTcpSocket->state())
				{
					//指令优先于接受码流
					if (m_tStepCode.size()>0)
					{
						nRunStep=BUBBLE_RUN_CONTROL;
						sleepEx(3);
					}else{
						if (m_pTcpSocket->bytesAvailable()>0)
						{
							nRunStep=BUBBLE_RUN_RECEIVE;
						}else{
							QEventLoop tEventLoop;
							QTimer::singleShot(40,&tEventLoop,SLOT(quit()));
							tEventLoop.exec();
							nRunStep=BUBBLE_RUN_DEFAULT;
						}
					}
					//三分钟发射一次心脏指令
					nHeartBeat++;
					if (nHeartBeat>3000)
					{
						nHeartBeat=0;
						m_nPosition=__LINE__;
						m_bBlock=true;
						m_csStepCode.lock();
						m_tStepCode.enqueue(BUBBLE_HEARTBEAT);
						m_csStepCode.unlock();
						m_bBlock=false;
					}else{
						//do nothing
					}
				}else{
					nRunStep=BUBBLE_RUN_DISCONNECT;
					qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"turn to disconnect as current state::"<<m_pTcpSocket->state()<<m_pTcpSocket->error();
				}
			}else{
				nRunStep=BUBBLE_RUN_DISCONNECT;
				qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<"turn to disconnect as m_bStop::"<<m_bStop;
			}
								}
								break;
		case BUBBLE_RUN_END:{
			//结束
			m_nPosition=__LINE__;
			bRunStop=true;
			if (NULL!=m_pTcpSocket)
			{
				m_pTcpSocket->deleteLater();
			}else{
				//do nothing
			}
			//重置参数
			m_csStepCode.lock();
			m_tStepCode.clear();
			m_tBubblePtz.clear();
			m_csStepCode.unlock();
			m_tDeviceInfo.bPrePause=false;
			m_tDeviceInfo.bRemotePlayPause=false;
			m_tDeviceInfo.sEseeId.clear();
			m_tDeviceInfo.sPassword.clear();
			m_tDeviceInfo.sRemotePlayFile.clear();
			m_tDeviceInfo.sUserName.clear();
			m_tDeviceInfo.tIpAddr.clear();
			m_tDeviceInfo.tPorts.clear();
			m_tBuffer.clear();
							}
							break;
		}
	}
	QVariantMap evMap;
	m_tCurrentConnectStatus=BUBBLE_DISCONNECTED;
	evMap.insert("status",m_tCurrentConnectStatus);
	m_nPosition=__LINE__;
	m_bBlock=true;
	emit sgBackToMainThread(evMap);
	QEventLoop tEventLoop;
	QTimer::singleShot(10,&tEventLoop,SLOT(quit()));
	tEventLoop.exec();
	m_bBlock=false;
	m_nPosition=__LINE__;
}

int BubbleProtocolEx::setDeviceHost( const QString &sIpAddr )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		if (m_tDeviceInfo.tIpAddr.setAddress(sIpAddr))
		{
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHost fail as the input ip addr is illegal:"<<sIpAddr;
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceHost fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDevicePorts( const QVariantMap &tPorts )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		if (tPorts["media"].toInt()<=0||tPorts["media"].toInt()>=65535)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"setDevicePorts fail as the input ports is illegal :"<<tPorts["media"].toInt();
			return 1;
		}else{
			m_tDeviceInfo.tPorts=tPorts;
			return 0;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDevicePorts fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDeviceId( const QString &sEseeId )
{
	//0：设置成功
	//1：设置失败
	if (!QThread::isRunning())
	{
		m_tDeviceInfo.sEseeId=sEseeId;
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setDeviceId fail as the thread had been running,you should set it before the thread start";
		return 1;
	}
}

int BubbleProtocolEx::setDeviceAuthorityInfomation( QString sUserName,QString sPassword )
{
	//0：设置成功
	//1：设置失败
	m_tDeviceInfo.sPassword=sPassword;
	m_tDeviceInfo.sUserName=sUserName;
	return 0;
}

int BubbleProtocolEx::connectToDevice()
{
	//0：连接成功
	//1：连接失败
	//连接修改成阻塞型更为合适
	if (!QThread::isRunning())
	{
		m_csStepCode.lock();
		m_tStepCode.clear();
		m_tBubblePtz.clear();
		m_csStepCode.unlock();
		QVariantMap evMap;
		m_tCurrentConnectStatus=BUBBLE_CONNECTTING;
		evMap.insert("status",m_tCurrentConnectStatus);
		slBackToMainThread(evMap);
		QThread::start();
		m_bWaitForConnect=true;
		int nCount=0;
		//最长等待6s
		while (m_bWaitForConnect&&nCount<600)
		{
			/*sleepEx(10);*/
			msleep(10);
			QEventLoop tEventLoop;
			QTimer::singleShot(5,&tEventLoop,SLOT(quit()));
			tEventLoop.exec();
			nCount++;
		}
		if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
		{
			evMap.clear();
			evMap.insert("status",m_tCurrentConnectStatus);
			slBackToMainThread(evMap);
			return 0;
		}else{
			m_tCurrentConnectStatus=BUBBLE_DISCONNECTED;
			evMap.clear();
			evMap.insert("status",m_tCurrentConnectStatus);
			slBackToMainThread(evMap);
			m_bStop=true;
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"connectToDevice fail ,if you want to connect to device ,please wait last connect thread terminate";
		return 1;
	}
}

int BubbleProtocolEx::authority()
{
	//	0:校验成功
	//	1:校验失败
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_AUTHORITY);
		m_csStepCode.unlock();
		m_bAuthorityFlag=false;
		m_bAuthorityWaitFlag=true;
		int nCount=0;
		while(m_bAuthorityWaitFlag==true&&nCount<50){
			//QEventLoop tEventLoop;
			//QTimer::singleShot(40,&tEventLoop,SLOT(quit()));
			//tEventLoop.exec();
			msleep(10);
			nCount++;
		}
		if (m_bAuthorityFlag==true||nCount==50)
		{
			QVariantMap tItem;
			tItem.insert("Authority",true);
			eventProcCall("Authority",tItem);
			return 0;
		}else{
			QVariantMap tItem;
			tItem.insert("Authority",false);
			eventProcCall("Authority",tItem);
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"authority fail as the current connect status is not in connected";
		QVariantMap tItem;
		tItem.insert("Authority",false);
		eventProcCall("Authority",tItem);
		return 1;
	}
	QVariantMap tItem;
	tItem.insert("Authority",true);
	eventProcCall("Authority",tItem);
	return 0;
}

int BubbleProtocolEx::disconnect()
{
	//0：断开成功
	//1：断开失败
	if (QThread::isRunning())
	{
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_DISCONNECT);
		m_csStepCode.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"it had been in disconnect status,there is no need to call this function again";
	}
	return 0;
}

int BubbleProtocolEx::getCurrentStatus()
{
	return m_tCurrentConnectStatus;
}

QString BubbleProtocolEx::getDeviceHost()
{
	return m_tDeviceInfo.tIpAddr.toString();
}

QString BubbleProtocolEx::getDeviceid()
{
	return m_tDeviceInfo.sEseeId;
}

QVariantMap BubbleProtocolEx::getDevicePorts()
{
	return m_tDeviceInfo.tPorts;
}

int BubbleProtocolEx::getLiveStream( int nChannel,int nStream )
{
	//0：获取成功
	//1：获取失败
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_tDeviceInfo.nPreChannel=nChannel;
		m_tDeviceInfo.nPreStream=nStream;
		//fix me
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_GETLIVESTREAM);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getLiveStream fail as the current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::stopStream()
{
	//0：断开成功
	//1：断开失败
	if (QThread::isRunning())
	{
		//fix me
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_STOPSTREAM);
		m_csStepCode.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread had been terminate,there is no need to call this function";
	}
	return 0;
}

int BubbleProtocolEx::pauseStream( bool bPause )
{
	//0：暂停成功
	//1：暂停失败
	//fix me
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		if (m_tDeviceInfo.bPrePause!=bPause)
		{
			m_csStepCode.lock();
			m_tStepCode.enqueue(BUBBLE_PAUSESTREAM);
			m_csStepCode.unlock();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"as it had been in bPasuse state ,there is no need to call this func again";
		}
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"as m_tCurrentConnectStatus is no connected,there is no need to call this func";
	}
	return 0;
}

int BubbleProtocolEx::getStreamCount()
{
	//fix me
	if (!m_tHttpStreamList.isEmpty())
	{
		QList<tagBubbleHttpStreamInfo> tStreamInfo=m_tHttpStreamList.at(m_tDeviceInfo.nPreChannel);
		return tStreamInfo.size();
	}else{
		return 0;
	}
}

int BubbleProtocolEx::getStreamInfo( int nStreamId,QVariantMap &tStreamInfo )
{
	//fix me
	if (!m_tHttpStreamList.isEmpty())
	{
		QList<tagBubbleHttpStreamInfo> tHttpStreamInfo=m_tHttpStreamList.at(m_tDeviceInfo.nPreChannel);
		if ((tHttpStreamInfo.size()<=nStreamId||tHttpStreamInfo.isEmpty()))
		{
			tagBubbleHttpStreamInfo tItem=tHttpStreamInfo.at(nStreamId);
			tStreamInfo.insert("name", tItem.sName);
			tStreamInfo.insert("size", tItem.sSize);
			tStreamInfo.insert("x1", tItem.sX1);
			tStreamInfo.insert("x2", tItem.sX2);
			tStreamInfo.insert("x4", tItem.sX4);
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getStreamInfo fail as tHttpStreamInfo.size()<=nStreamId or tHttpStreamInfo.isEmpty() ";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getStreamInfo fail as m_tHttpStreamList is empty";
	}
	return 1;
}

int BubbleProtocolEx::startSearchRecFile( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	return m_tSearchRemoteFile.startSearchRecFile(nChannel,nTypes,startTime,endTime,m_tDeviceInfo.tIpAddr,m_tDeviceInfo.tPorts,m_tDeviceInfo.sUserName,m_tDeviceInfo.sPassword);
}

int BubbleProtocolEx::getPlaybackStreamByTime( int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	//fix me
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		if (!(/*nChannel<0||*/nTypes<0||startTime>endTime))
		{
			m_tDeviceInfo.nRemotePlayChannel=nChannel;
			m_tDeviceInfo.nRemotePlayTypes=nTypes;
			m_tDeviceInfo.tRemotePlayStartTime=startTime;
			m_tDeviceInfo.tRemotePlayEndTime=endTime;
			m_csStepCode.lock();
			m_tStepCode.enqueue(BUBBLE_GETPLAYBACKSTREAMBYTIME);
			m_csStepCode.unlock();
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getPlaybackStreamByTime fail as input params are unCorrect";
			return 2;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getPlaybackStreamByTime fail as m_tCurrentConnectStatus is no in connected";
		return 1;
	}
}

int BubbleProtocolEx::getPlaybackStreamByFileName( int nChannel,const QString &sFileName )
{
	//	0:调用成功
	//	1:调用失败
	//	2:参数错误
	//fix me
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_tDeviceInfo.sRemotePlayFileName=sFileName;
		if (!(nChannel<0||sFileName.isEmpty()||false==checkRemoteFileIsExist()))
		{
			m_csStepCode.lock();
			m_tStepCode.enqueue(BUBBLE_GETPLAYBACKSTREAMBYFILENAME);
			m_csStepCode.unlock();
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getPlaybackStreamByFileName fail as the input params are unCorrect";
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getPlaybackStreamByFileName fail as m_tCurrentConnectStatus is no in connected";
		return 1;
	}
}

int BubbleProtocolEx::pausePlaybackStream( bool bPause )
{
	//	0:调用成功
	//	1:调用失败
	//fix me
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		if (m_tDeviceInfo.bRemotePlayPause!=bPause)
		{
			m_tDeviceInfo.bRemotePlayPause=bPause;
			m_csStepCode.lock();
			m_tStepCode.enqueue(BUBBLE_PAUSEPLAYBACKSTREAM);
			m_csStepCode.unlock();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"as it had been in bPause state ,there is no need to call this func";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"as m_tCurrentConnectStatus is no in connected,there is no need to call this func";
	}
	return 0;
}

int BubbleProtocolEx::stopPlaybackStream()
{
	//	0:调用成功
	//	1:调用失败
	//fix me
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		m_tStepCode.enqueue(BUBBLE_STOPPLAYBACKSTREAM);
		m_csStepCode.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"as m_tCurrentConnectStatus is no in connected,there is no need to call this func";
	}
	return 0;
}

void BubbleProtocolEx::sleepEx( int nTime )
{
	if (m_nSleepSwitch<5)
	{
		msleep(nTime);
		m_nSleepSwitch++;
	}else{
		QEventLoop tEventLoop;
		QTimer::singleShot(5,&tEventLoop,SLOT(quit()));
		tEventLoop.exec();
		m_nSleepSwitch=0;
	}
	return;
}

void BubbleProtocolEx::slCheckoutBlock()
{
	if (m_bBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<m_tDeviceInfo.tIpAddr.toString()<<m_tDeviceInfo.sEseeId<<"block at ::"<<m_nPosition<<"please check!";
	}else{
		//keep going
	}
}

void BubbleProtocolEx::slBackToMainThread( QVariantMap evMap )
{
	if (evMap.contains("status"))
	{
		//状态改变，抛出状态
		if (evMap.value("status").toInt()!=m_tHistoryConnectStatus)
		{
			m_nSecondPosition=__LINE__;
			eventProcCall("StateChangeed",evMap);
			m_tHistoryConnectStatus=(tagBubbleConnectStatusInfo)evMap.value("status").toInt();
		}else{
			//do nothing
		}
	}else if (evMap.contains("Timer")&&m_tCheckoutBlockTimer.isActive())
	{
		m_tCheckoutBlockTimer.stop();
	}
	else{
		//do nothing
	}
}

bool BubbleProtocolEx::analyzeBubbleInfo()
{
	if (m_tBuffer.contains("HTTP/1.1 200"))
	{
		m_bIsSupportHttp=true;
		//fix me
		int nPos=m_tBuffer.indexOf("<");
		QString sXml=m_tBuffer.mid(nPos,m_tBuffer.indexOf("#")-nPos);
		QString sTemp=sXml;
		sXml=checkXML(sTemp);
		QDomDocument *pDom=new QDomDocument();
		if (!(NULL==pDom||pDom->setContent(sXml)==false))
		{
			QDomNode tRoot=pDom->firstChild();
			int nChannelCount=tRoot.toElement().attribute("vin").toInt();
			for (int i=0;i<nChannelCount;i++)
			{
				QList<tagBubbleHttpStreamInfo> tList;
				QString tName=QString("vin%1").arg(i);
				QDomNodeList tNodeList=pDom->elementsByTagName(tName);
				QDomNode tSubNode=tNodeList.at(0);
				int nStreamNum=tSubNode.toElement().attribute("stream").toInt();
				for(int j=0;j<nStreamNum;j++){
					QString sSubName=QString("stream%1").arg(j);
					QDomNodeList tNodeList1=pDom->elementsByTagName(sSubName);
					QDomNode tNode=tNodeList1.at(0);
					tagBubbleHttpStreamInfo tStem;
					tStem.sName=tNode.toElement().attribute("name");
					tStem.sSize=tNode.toElement().attribute("size");
					tStem.sX1=tNode.toElement().attribute("x1");
					tStem.sX2=tNode.toElement().attribute("x2");
					tStem.sX4=tNode.toElement().attribute("x4");
					tList.append(tStem);
				}
				m_tHttpStreamList.append(tList);
			}
			delete pDom;
			pDom=NULL;
			nPos=m_tBuffer.indexOf("HTTP/1.1 200");
			if (1024+nPos<=m_tBuffer.size())
			{
				m_tBuffer.remove(0,1024+nPos);
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"there are unCorrect data,please check,as for debug,i will throw the error";
				throw(1024+nPos);
				m_tBuffer.clear();
				return false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"analyzeBubbleInfo fail as pDom is null or pDom->setContent(sXml) fail";
			if (NULL!=pDom)
			{
				delete pDom;
				pDom=NULL;
			}else{
				// do nothing
			}
			return false;
		}
	}else{
		m_bIsSupportHttp=false;
		int nPos=m_tBuffer.indexOf("HTTP/1.1 404");
		if (1024+nPos>m_tBuffer.size())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"there are unCorrect data,please check,as for debug,i will throw the error";
			throw(1024+nPos);
			m_tBuffer.clear();
			return false;
		}else{
			m_tBuffer.remove(0,1024+nPos);
		}
		
		return true;
	}
}

bool BubbleProtocolEx::analyzePreviewInfo()
{
	//每次只解析一帧数据便返回
	//进入函数之前 必须完成 数据头，数据长度，命令的检测
	unsigned int uiBufferSize=m_tBuffer.size();
	tagBubbleInfo *pBubbleInfo=NULL;
	pBubbleInfo=(tagBubbleInfo*)m_tBuffer.data();
    unsigned int uiBubbleLength=qToBigEndian((quint32)(pBubbleInfo->uiLength))+sizeof(pBubbleInfo->cHead)+sizeof(pBubbleInfo->uiLength);
	if (uiBubbleLength<=uiBufferSize)
	{
		if (m_tBuffer.startsWith("\xaa"))
		{
			if (pBubbleInfo->cCmd=='\x00')
			{
				//Message
				//暂时不解析这部分的内容
				tagBubbleReceiveMessage *pReceiveMessage=(tagBubbleReceiveMessage*)(pBubbleInfo->pLoad);
				if (pReceiveMessage->cMessage=='\x03')
				{
					tagBubbleAuthorityReceive *pBubbleAuthorityReceive=(tagBubbleAuthorityReceive*)(&pReceiveMessage->cParameters);
					if (pBubbleAuthorityReceive->cVerified=='\x01')
					{
						m_bAuthorityWaitFlag=false;
						m_bAuthorityFlag=true;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"authority fail as illegal username or password";
						m_bAuthorityFlag=false;
						m_bAuthorityWaitFlag=false;
					}
				}else if (pReceiveMessage->cMessage=='\x04')
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"device channels number";
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"pReceiveMessage->cMessage is an undefined cMessage"<<pReceiveMessage->cMessage;
				}
				m_tBuffer.remove(0,uiBubbleLength);
				return true;
			}else if (pBubbleInfo->cCmd=='\x01')
			{
				//Live Stream
				tagBubbleLiveStream *pLiveStream=(tagBubbleLiveStream *)pBubbleInfo->pLoad;
				QVariantMap tStreamInfo;
				if (0==pLiveStream->cType)
				{
					//音频
					tagBubbleLiveStreamAudio *pLiveStreamAudio=(tagBubbleLiveStreamAudio*)(pLiveStream->pData);
					tStreamInfo.insert("channel", pLiveStream->cChannel);
					tStreamInfo.insert("pts", pLiveStreamAudio->ui64Pts);
					tStreamInfo.insert("length", pLiveStreamAudio->uiEntries * pLiveStreamAudio->uiPackSize);
                    tStreamInfo.insert("data", (quintptr)((char*)pLiveStreamAudio + sizeof(tagBubbleLiveStreamAudio)));
					tStreamInfo.insert("frametype", pLiveStream->cType);
					tStreamInfo.insert("samplerate", pLiveStreamAudio->uiSampleRate);
					tStreamInfo.insert("samplewidth", pLiveStreamAudio->uiSampleWidth);
					tStreamInfo.insert("audiochannel", 1);
					tStreamInfo.insert("acodec", pLiveStreamAudio->cEnCode);
					tStreamInfo.insert("gentime", pLiveStreamAudio->uiGtime);
					m_nSecondPosition=__LINE__;
					if (tStreamInfo.value("length").toInt()>uiBubbleLength)
					{
						return false;
					}else{
						//keep going
					}
					eventProcCall("LiveStream",tStreamInfo);
					m_tBuffer.remove(0,uiBubbleLength);
					return true;
				}else if (1==pLiveStream->cType||2==pLiveStream->cType)
				{
					//视频
					int nWidth=0;
					int nHeight=0;
                    GetWidthHeight(pLiveStream->pData,qToBigEndian((quint32)(pLiveStream->uiLength)),&nWidth,&nHeight);
					tStreamInfo.insert("channel", pLiveStream->cChannel);
                    tStreamInfo.insert("pts", (unsigned long long)qToBigEndian((quint32)(pBubbleInfo->uiTicket))*1000);
                    tStreamInfo.insert("length", qToBigEndian((quint32)(pLiveStream->uiLength)));
                    tStreamInfo.insert("data", (quintptr)(pLiveStream->pData));
					tStreamInfo.insert("frametype", pLiveStream->cType);
					tStreamInfo.insert("width", nWidth);
					tStreamInfo.insert("height", nHeight);
					tStreamInfo.insert("vcodec", "H264");
					m_nSecondPosition=__LINE__;
					if (tStreamInfo.value("length").toInt()>uiBubbleLength)
					{
						return false;
					}else{
						//keep going
					}
					eventProcCall("LiveStream",tStreamInfo);
					m_tBuffer.remove(0,uiBubbleLength);
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail as pLiveStream->cType is an undefined type"<<pLiveStream->cType;
				}
			}else if (pBubbleInfo->cCmd=='\x02')
			{
				//Heartbeat
				if (uiBubbleLength==11)
				{
					m_tBuffer.remove(0,11);
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail as the data is unCorrect";
				}
			}
			else if (pBubbleInfo->cCmd=='\x08')
			{
				//Connect Refuse
				if (uiBubbleLength==11)
				{
					QVariantMap tItem;
					tItem.insert("ConnectRefuse",true);
					eventProcCall("ConnectRefuse",tItem);
					m_tBuffer.remove(0,11);
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail as the data is unCorrect";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail  as it cmd is not \x00 , \x01,\x02,\x08";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail as it is unExpect data";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"analyzePreviewInfo fail as buffer size is not enough";
	}
	return false;
}

bool BubbleProtocolEx::analyzeRemoteInfo()
{
	//每次只解析一帧数据便返回
	//进入函数之前 必须完成 数据头，数据长度，命令的检测
	unsigned int uiBufferSize=m_tBuffer.size();
	tagBubbleInfo *pBufferInfo=NULL;
	pBufferInfo=(tagBubbleInfo *)m_tBuffer.data();
    unsigned int uiBubbleLength=qToBigEndian((quint32)(pBufferInfo->uiLength))+sizeof(pBufferInfo->cHead)+sizeof(pBufferInfo->uiLength);
	if (uiBubbleLength<=uiBufferSize)
	{
		if (m_tBuffer.startsWith("\xab"))
		{
			if (pBufferInfo->cCmd=='\x01')
			{
				//Record Stream
				tagBubbleRecordStream *pRecordStream=(tagBubbleRecordStream*)pBufferInfo->pLoad;
				QVariantMap tStreamInfo;
				QDateTime tTime1=QDateTime::currentDateTime();
				QDateTime tTime2=QDateTime::currentDateTimeUtc();
				int nTimeDifference=qAbs(tTime1.time().hour()-tTime2.time().hour())*3600;
                if (qToBigEndian((quint32)(pRecordStream->uiLength))-132<uiBubbleLength)
				{
					//音频
					if (0==pRecordStream->cType)
					{
                        int nLength=qToBigEndian((quint32)(pRecordStream->uiLength))-132;
						tStreamInfo.insert("length"         ,nLength);
						tStreamInfo.insert("frametype"      ,pRecordStream->cType);
						tStreamInfo.insert("channel"        ,pRecordStream->cChannel);
						tStreamInfo.insert("audioSampleRate",pRecordStream->uiAudioSampleRate);
						tStreamInfo.insert("audioFormat"    ,pRecordStream->cAudioFormat);
						tStreamInfo.insert("audioDataWidth" ,pRecordStream->uiAudioDataWidth);
						tStreamInfo.insert("pts"            ,pRecordStream->nU64TSP);
						tStreamInfo.insert("gentime"        ,pRecordStream->uiGenTime - nTimeDifference);
						int nOffSet=sizeof(pRecordStream->uiLength)+sizeof(pRecordStream->cType)+sizeof(pRecordStream->cChannel)+128+4;
                        tStreamInfo.insert("data",(quintptr)((char*)pRecordStream+nOffSet));
						tStreamInfo.insert("samplerate"		,pRecordStream->uiAudioSampleRate);
						tStreamInfo.insert("samplewidth"	,pRecordStream->uiAudioDataWidth);
						tStreamInfo.insert("audiochannel"	,pRecordStream->uiChannel);
						tStreamInfo.insert("acodec"			,pRecordStream->cAudioFormat);
						m_nSecondPosition=__LINE__;
						eventProcCall("RecordStream",tStreamInfo);
						m_tBuffer.remove(0,uiBubbleLength);
						return true;
					}else if (1==pRecordStream->cType||2==pRecordStream->cType)
					{
						//视频
                        int nLength=qToBigEndian((quint32)(pRecordStream->uiLength))-128;
						tStreamInfo.insert("length"       ,nLength);
						tStreamInfo.insert("frametype"    ,pRecordStream->cType);
						tStreamInfo.insert("channel"      ,pRecordStream->cChannel);
						tStreamInfo.insert("width"        ,pRecordStream->uiFrameWidth);
						tStreamInfo.insert("height"       ,pRecordStream->uiFrameHeight);
						tStreamInfo.insert("framerate"    ,pRecordStream->uiFrameRate);
						tStreamInfo.insert("pts"          ,pRecordStream->nU64TSP);
						tStreamInfo.insert("gentime"      ,pRecordStream->uiGenTime - nTimeDifference);
						int nOffSet=sizeof(pRecordStream->uiLength)+sizeof(pRecordStream->cType)+sizeof(pRecordStream->cChannel)+128;
                        tStreamInfo.insert("data",(quintptr)((char*)pRecordStream+nOffSet));
						m_nSecondPosition=__LINE__;
						eventProcCall("RecordStream",tStreamInfo);
						m_tBuffer.remove(0,uiBubbleLength);
						return true;
					}else{
						//未定义类型
						qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as pRecordStream->cType is a undefined type"<<pRecordStream->cType;
					}
				}else{
                    qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as pRecordStream->uiLength is  larger than uiBubbleLength :"<<qToBigEndian((quint32)(pRecordStream->uiLength));
				}
			}else if (pBufferInfo->cCmd=='\x02')
			{
				//Heart Beat
				//do nothing
				if (uiBubbleLength==11)
				{
					m_tBuffer.remove(0,uiBubbleLength);
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as the datas are unCorrect";
				}
			}else if (pBufferInfo->cCmd=='\x09')
			{
				if (uiBubbleLength==18)
				{
					m_tBuffer.remove(0,uiBubbleLength);
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as the datas are unCorrect";
				}
			}
			else{
				qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as it cmd is not \x01 or \x02";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as it is unExpect data";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"analyzeRemoteInfo fail as buffer size is not enough ";
	}
	return false;
}

bool BubbleProtocolEx::cmdAuthority()
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[100];
		QByteArray tBlock;
		qint64 nLen=0;
		tagBubbleMessageInfo *pMessageInfo=NULL;
		tagBubbleAuthoritySend *pAuthoritySend=NULL;
		memset(cBuffer,0,100);
		tagBubbleInfo *pBubbleInfo=(tagBubbleInfo*)cBuffer;
		pBubbleInfo->cHead=(char)0xaa;

		QDateTime tTime=QDateTime::currentDateTime();
        pBubbleInfo->uiTicket=qToBigEndian((quint32)tTime.toMSecsSinceEpoch());
		pBubbleInfo->cCmd=(char)0x00;

		pMessageInfo=(tagBubbleMessageInfo*)pBubbleInfo->pLoad;
		pMessageInfo->cMessage=(char)0x00;
		memset(pMessageInfo->cReverse,(char)0x00,3);
		pMessageInfo->uiLength=sizeof(tagBubbleMessageInfo)+sizeof(tagBubbleAuthoritySend)-sizeof(pMessageInfo->uiLength)-sizeof(pMessageInfo->cParameters);

		pAuthoritySend=(tagBubbleAuthoritySend*)pMessageInfo->cParameters;
		qstrcpy(pAuthoritySend->cUserName,m_tDeviceInfo.sUserName.toUtf8().data());
		qstrcpy(pAuthoritySend->cPassWord,m_tDeviceInfo.sPassword.toUtf8().data());

		pBubbleInfo->uiLength=sizeof(tagBubbleInfo)+sizeof(tagBubbleMessageInfo)+sizeof(tagBubbleAuthoritySend)-sizeof(pMessageInfo->cParameters)-sizeof(pBubbleInfo->cHead)-sizeof(pBubbleInfo->uiLength)-sizeof(pBubbleInfo->pLoad);
		nLen=(qint64)(pBubbleInfo->uiLength+sizeof(pBubbleInfo->cHead)+sizeof(pBubbleInfo->uiLength));
        pBubbleInfo->uiLength=qToBigEndian((quint32)(pBubbleInfo->uiLength));
        pMessageInfo->uiLength=qToBigEndian((quint32)(pMessageInfo->uiLength));
		tBlock.append(cBuffer,nLen);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdAuthority fail as it write buffer to socket fail within 2s"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdAuthority fail as m_pTcpSocket write buffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdAuthority fail as m_pTcpSocket is null,please check";
	}
	return false;
}

bool BubbleProtocolEx::cmdDisConnect()
{
	m_bStop=true;
	return true;
}

bool BubbleProtocolEx::cmdGetLiveStream()
{
	if (NULL!=m_pTcpSocket)
	{
		if (sendLiveStreamCmd(true))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetLiveStream fail as sendLiveStreamCmd fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetLiveStream fail as m_pTcpSocket is null,please check";
	}
	return false;
}

bool BubbleProtocolEx::cmdGetLiveStreamEx()
{
	if (NULL!=m_pTcpSocket)
	{
		if (sendLiveStreamCmdEx(true))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetLiveStreamEx fail as sendLiveStreamCmdEx fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetLiveStreamEx fail as m_pTcpSocket is null,please check";
	}
	return false;
}

bool BubbleProtocolEx::cmdStopStream()
{
	if (NULL!=m_pTcpSocket)
	{
		if (sendLiveStreamCmdEx(false))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdStopStream fail as sendLiveStreamCmdEx fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdStopStream fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdPauseStream()
{
	if (NULL!=m_pTcpSocket)
	{
		if (sendLiveStreamCmdEx(m_tDeviceInfo.bPrePause))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdPauseStream fail as sendLiveStreamCmdEx fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdPauseStream fail as m_pTcpSocket is null,please check";
	}
	return false;
}

bool BubbleProtocolEx::cmdHeartBeat()
{
	//暂时不区分 预览和回放的心脏指令，此指令仅仅用于检测是否还保持连接，不保证对方正确接收
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[100];
		qint64 nLength=0;
		tagBubbleInfo *pBubbleInfo=(tagBubbleInfo*)cBuffer;

		pBubbleInfo->cHead=(char)0xaa;
		pBubbleInfo->cCmd=(char)0x02;
		QDateTime tTime=QDateTime::currentDateTime();
        pBubbleInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
		pBubbleInfo->pLoad[0]=(char)0x02;

		pBubbleInfo->uiLength=sizeof(tagBubbleInfo)-sizeof(pBubbleInfo->cHead)-sizeof(pBubbleInfo->uiLength);
		nLength=(quint64)(pBubbleInfo->uiLength+sizeof(pBubbleInfo->cHead)+sizeof(pBubbleInfo->uiLength));
        pBubbleInfo->uiLength=qToBigEndian((quint32)(pBubbleInfo->uiLength));
		QByteArray tBlock;
		tBlock.append(cBuffer,nLength);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdHeartBeat fail as it write buffer to socket fail within 2s"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdHeartBeat fail as m_pTcpSocket write buffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdHeartBeat fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdGetPlayBackStreamByTime()
{
	if (NULL!=m_pTcpSocket)
	{
		if (!(/*m_tDeviceInfo.nRemotePlayChannel<0||*/m_tDeviceInfo.nRemotePlayTypes<0||m_tDeviceInfo.tRemotePlayStartTime>m_tDeviceInfo.tRemotePlayEndTime))
		{
			QDateTime tTime1=QDateTime::currentDateTime();
			QDateTime tTime2=QDateTime::currentDateTimeUtc();
			int nTimeDifference=qAbs(tTime1.time().hour()-tTime2.time().hour())*3600;
			uint uiStartTime=m_tDeviceInfo.tRemotePlayStartTime.toTime_t();
			uint uiEndTime=m_tDeviceInfo.tRemotePlayEndTime.toTime_t();

			QString sSendHttpHead="GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
			if (-1!=m_pTcpSocket->write(sSendHttpHead.toAscii()))
			{
				if (m_pTcpSocket->waitForBytesWritten(2000))
				{
					uiStartTime=uiStartTime+nTimeDifference;
					uiEndTime=uiEndTime+nTimeDifference;
					char cBuffer[50];
					uint uiLength=0;
					tagBubbleInfo *pBubbleInfo=NULL;
					memset(cBuffer,0,sizeof(cBuffer));
					pBubbleInfo=(tagBubbleInfo *)cBuffer;
					pBubbleInfo->cHead=(char)0xab;
					pBubbleInfo->cCmd=(char)0x05;
					uiLength=sizeof(tagBubbleInfo)-sizeof(pBubbleInfo->pLoad)+sizeof(tagBubbleRemotePlayRecordRequireV2);
                    pBubbleInfo->uiLength=qToBigEndian((quint32)(uiLength-sizeof(pBubbleInfo->cHead)-sizeof(pBubbleInfo->uiLength)));
					QDateTime tTime=QDateTime::currentDateTime();
                    pBubbleInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
					tagBubbleRemotePlayRecordRequireV2 *pRecordRequireV2=(tagBubbleRemotePlayRecordRequireV2 *)pBubbleInfo->pLoad;
					pRecordRequireV2->nChannels=m_tDeviceInfo.nRemotePlayChannel;
					pRecordRequireV2->nTypes=m_tDeviceInfo.nRemotePlayTypes;
					pRecordRequireV2->uiStart=uiStartTime;
					pRecordRequireV2->uiEnd=uiEndTime;
					QByteArray tBlock;
					tBlock.append(cBuffer,uiLength);
					if (-1!=m_pTcpSocket->write(tBlock))
					{
						if (m_pTcpSocket->waitForBytesWritten(2000))
						{
							return true;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as it write cBuffer to socket fail within 2s"<<m_pTcpSocket->error();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as it write cBuffer to socket fail"<<m_pTcpSocket->error();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as it write sSendHttpHead to socket fail within 2s"<<m_pTcpSocket->error();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as it write sSendHttpHead to socket fail"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as the input params are error"<<m_tDeviceInfo.nRemotePlayChannel<<m_tDeviceInfo.nRemotePlayTypes<<m_tDeviceInfo.tRemotePlayStartTime<<m_tDeviceInfo.tRemotePlayEndTime;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByTime fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdGetPlayBackStreamByFileName()
{
	if (NULL!=m_pTcpSocket)
	{
		if (!(m_tDeviceInfo.nRemotePlayChannel<0||m_tDeviceInfo.sRemotePlayFileName.isEmpty()||false==checkRemoteFileIsExist()))
		{
			QString sSendHttpHead="GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
			if (-1!=m_pTcpSocket->write(sSendHttpHead.toAscii()))
			{
				if (m_pTcpSocket->waitForBytesWritten(2000))
				{
					QStringList tList1=m_tDeviceInfo.sRemotePlayFileName.split("_",QString::SkipEmptyParts);
					if (tList1.size()==3)
					{
						unsigned int uiStartTime=QDateTime::fromString(tList1.at(0),"yyyyMMddhhmmss").toTime_t();
						unsigned int uiEndTime=QDateTime::fromString(tList1.at(1),"yyyyMMddhhmmss").toTime_t();
						QDateTime tTime1=QDateTime::currentDateTime();
						QDateTime tTime2=QDateTime::currentDateTimeUtc();
						int nTimeDifference=qAbs(tTime1.time().hour()-tTime2.time().hour())*3600;
						uiStartTime +=nTimeDifference;
						uiEndTime+=nTimeDifference;
						QString sPlayBackType=tList1.at(2);
						int nTmp=0;
						if (sPlayBackType.contains("T"))
						{
							nTmp += 1;
						}
						if (sPlayBackType.contains("M"))
						{
							nTmp += 2;
						}
						if (sPlayBackType.contains("S"))
						{
							nTmp += 4;
						}
						if (sPlayBackType.contains("H"))
						{
							nTmp += 8;
						}
						if (0 == nTmp)
						{
							nTmp = 15;
						}
						char cBuffer[50];
						uint uiLength=0;
						tagBubbleInfo *pBubbleInfo=NULL;
						memset(cBuffer,0,sizeof(cBuffer));
						pBubbleInfo=(tagBubbleInfo *)cBuffer;
						pBubbleInfo->cHead=(char)0xab;
						pBubbleInfo->cCmd=(char)0x05;
						uiLength=sizeof(tagBubbleInfo)-sizeof(pBubbleInfo->pLoad)+sizeof(tagBubbleRemotePlayRecordRequireV2);
                        pBubbleInfo->uiLength=qToBigEndian((quint32)(uiLength-sizeof(pBubbleInfo->cHead)-sizeof(pBubbleInfo->uiLength)));
						QDateTime tTime=QDateTime::currentDateTime();
                        pBubbleInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
						tagBubbleRemotePlayRecordRequireV2 *pRecordRequireV2=(tagBubbleRemotePlayRecordRequireV2*)pBubbleInfo->pLoad;
						pRecordRequireV2->nChannels=m_tDeviceInfo.nRemotePlayChannel;
						pRecordRequireV2->nTypes=nTmp;
						pRecordRequireV2->uiStart=uiStartTime;
						pRecordRequireV2->uiEnd=uiEndTime;
						QByteArray tBlock;
						tBlock.append(cBuffer,uiLength);
						if (-1!=m_pTcpSocket->write(tBlock))
						{
							if (m_pTcpSocket->waitForBytesWritten(2000))
							{
								return true;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as it write cBuffer to socket fail within 2s"<<m_pTcpSocket->error();
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as it write cBuffer to socket fail"<<m_pTcpSocket->error();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as sRemotePlayFileName is error"<<tList1;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as it write sSendHttpHead to socket fail within 2s"<<m_pTcpSocket->error();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as it write sSendHttpHead to socket fail"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as the input params are error";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdGetPlayBackStreamByFileName fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdPausePlayBackStream()
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[50];
		tagBubbleInfo *pBufferInfo=NULL;
		memset(cBuffer,0,sizeof(cBuffer));
		pBufferInfo=(tagBubbleInfo *)cBuffer;
		pBufferInfo->cHead=(char)0xab;
		if (m_tDeviceInfo.bRemotePlayPause)
		{
			//pause
			pBufferInfo->cCmd=(char)0x02;
		}else{
			//continue
			pBufferInfo->cCmd=(char)0x03;
		}
        pBufferInfo->uiLength=qToBigEndian((quint32)(sizeof(pBufferInfo->cCmd)+sizeof(pBufferInfo->uiTicket)));
		QDateTime tTime=QDateTime::currentDateTime();
        pBufferInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
		QByteArray tBlock;
		quint64 uiLength=(quint64)(sizeof(tagBubbleInfo)-1);
		tBlock.append(cBuffer,uiLength);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdPausePlayBackStream fail as it write cBuffer to socket fail within 2s"<<m_pTcpSocket->error();
				}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdPausePlayBackStream fail as it write cBuffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdPausePlayBackStream fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdPtz(int nAction,int nChannel,int nStart,int nParm)
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[100];
		tagBubbleInfo *pBufferInfo=NULL;
		memset(cBuffer,0,sizeof(cBuffer));
		pBufferInfo=(tagBubbleInfo *)cBuffer;
		pBufferInfo->cHead=(char)0xaa;
		pBufferInfo->cCmd=(char)0x00;
		QDateTime tTime=QDateTime::currentDateTime();
		pBufferInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
		tagBubbleMessageInfo *pBubbleMessageInfo=(tagBubbleMessageInfo*)pBufferInfo->pLoad;
		pBubbleMessageInfo->cMessage=(char)0x02;
		pBubbleMessageInfo->uiLength=sizeof(tagBubbleMessageInfo)-sizeof(char)-sizeof(pBubbleMessageInfo->uiLength)+sizeof(tagBubblePtz);
		pBubbleMessageInfo->uiLength=qToBigEndian((quint32)pBubbleMessageInfo->uiLength);
		pBufferInfo->uiLength=sizeof(tagBubbleInfo)-sizeof(char)-sizeof(pBufferInfo->cHead)-sizeof(pBufferInfo->uiLength)+sizeof(tagBubbleMessageInfo)-sizeof(char)+sizeof(tagBubblePtz);
		quint32 uiLength=pBufferInfo->uiLength+sizeof(pBufferInfo->uiLength)+sizeof(pBufferInfo->cHead);
		pBufferInfo->uiLength=qToBigEndian((quint32)pBufferInfo->uiLength);
		tagBubblePtz *pBubblePtz=(tagBubblePtz*)pBubbleMessageInfo->cParameters;
		pBubblePtz->cAction=(char)nAction;
		pBubblePtz->cChannel=(char)nChannel;
		pBubblePtz->cParam=(char)nParm;
		pBubblePtz->cStart=(char)nStart;
		QByteArray tBlock;
		tBlock.append(cBuffer,uiLength);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdPtz fail as it write cBuffer to socket fail within 2s"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdPtz fail as it write cBuffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdPtz fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::cmdStopPlayBackStream()
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[50];
		tagBubbleInfo *pBufferInfo=NULL;
		memset(cBuffer,0,sizeof(cBuffer));
		pBufferInfo=(tagBubbleInfo*)cBuffer;
		pBufferInfo->cHead=(char)0xab;
		pBufferInfo->cCmd=(char)0x04;
        pBufferInfo->uiLength=qToBigEndian((quint32)(sizeof(pBufferInfo->cCmd)+sizeof(pBufferInfo->uiTicket)));
		QDateTime tTime=QDateTime::currentDateTime();
        pBufferInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
		quint64 uiLength=(quint64)(sizeof(tagBubbleInfo)-1);
		QByteArray tBlock;
		tBlock.append(cBuffer,uiLength);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"cmdStopPlayBackStream fail as it write cBuffer to socket fail within 2s"<<m_pTcpSocket->error();
				}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"cmdStopPlayBackStream fail as it write cBuffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"cmdStopPlayBackStream fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::sendLiveStreamCmdEx( bool flags )
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[100];
		qint64 nLength=0;
		tagBubbleInfo *pBubbleInfo=NULL;

		memset(cBuffer,0,100);

		pBubbleInfo=(tagBubbleInfo*)cBuffer;
		pBubbleInfo->cHead=(char)0xaa;
		pBubbleInfo->cCmd=(char)0x0a;
		QDateTime tTime=QDateTime::currentDateTime();
        unsigned int uiTicket=(unsigned int)(tTime.toMSecsSinceEpoch());
        pBubbleInfo->uiTicket=qToBigEndian((quint32)uiTicket);

		tagBubbleLiveStreamRequireEx *pLiveStreamRequireEx=(tagBubbleLiveStreamRequireEx*)pBubbleInfo->pLoad;
		pLiveStreamRequireEx->uiChannel=m_tDeviceInfo.nPreChannel;
		pLiveStreamRequireEx->uiStream=m_tDeviceInfo.nPreStream;
		pLiveStreamRequireEx->uiOperation=flags;
		pLiveStreamRequireEx->uiReversed=0;

		nLength=qint64(sizeof(tagBubbleInfo)+sizeof(tagBubbleLiveStreamRequireEx)-sizeof(pBubbleInfo->pLoad));
		pBubbleInfo->uiLength=sizeof(tagBubbleInfo)-sizeof(pBubbleInfo->pLoad)+sizeof(tagBubbleLiveStreamRequireEx)-sizeof(pBubbleInfo->cHead)-sizeof(pBubbleInfo->uiLength);
        pBubbleInfo->uiLength=qToBigEndian((quint32)(pBubbleInfo->uiLength));
		QByteArray tBlock;
		tBlock.append(cBuffer,nLength);

		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmdEx fail as it write buffer to socket fail within 2s"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmdEx fail as m_pTcpSocket write buffer to socket fail"<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmdEx fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::sendLiveStreamCmd( bool flags )
{
	if (NULL!=m_pTcpSocket)
	{
		char cBuffer[100];
		qint64 nLen=0;
		tagBubbleLiveStreamRequire *pStreamRequire=NULL;
		tagBubbleInfo *pBufferInfo=(tagBubbleInfo*)cBuffer;
		pBufferInfo->cHead=(char)0xaa;
		QDateTime tTime=QDateTime::currentDateTime();
        pBufferInfo->uiTicket=qToBigEndian((quint32)(tTime.toMSecsSinceEpoch()));
		pBufferInfo->cCmd=(char)0x04;
		pBufferInfo->uiLength=sizeof(tagBubbleInfo)-sizeof(pBufferInfo->pLoad)-sizeof(pBufferInfo->cHead)+sizeof(tagBubbleLiveStreamRequire)-sizeof(pBufferInfo->uiLength);
		pStreamRequire=(tagBubbleLiveStreamRequire*)pBufferInfo->pLoad;
		pStreamRequire->cChannel=m_tDeviceInfo.nPreChannel;
		pStreamRequire->cOperation=flags;
		nLen=(qint64)(pBufferInfo->uiLength+sizeof(pBufferInfo->cHead)+sizeof(pBufferInfo->uiLength));
        pBufferInfo->uiLength=qToBigEndian((quint32)(pBufferInfo->uiLength));

		QByteArray tBlock;
		tBlock.append(cBuffer,nLen);
		if (-1!=m_pTcpSocket->write(tBlock))
		{
			if (m_pTcpSocket->waitForBytesWritten(2000))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmd fail as it write to socket fail within 2s"<<m_pTcpSocket->error();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmd fail as m_pTcpSocket write buffer to socket fail."<<m_pTcpSocket->error();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"sendLiveStreamCmd fail as m_pTcpSocket is null";
	}
	return false;
}

bool BubbleProtocolEx::checkRemoteFileIsExist()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"the remoteFile is not exist";
	return false;
}

QString BubbleProtocolEx::checkXML( QString sSource )
{
	int i=0;
	int nPos=sSource.lastIndexOf(QString("vin%1").arg(i));
	while (nPos!=-1)
	{
		if (!sSource.mid(--nPos,1).contains('/'))
		{
			sSource.insert(++nPos,'/');
		}
		++i;
		nPos=sSource.lastIndexOf(QString("vin%1").arg(i));
	}
	return sSource;
}

int BubbleProtocolEx::cbFoundFile( QVariantMap &evmap )
{
	eventProcCall("foundFile",evmap);
	return 0;
}

int BubbleProtocolEx::cbRecFileSearchFinished( QVariantMap &evmap )
{
	eventProcCall("recFileSearchFinished",evmap);
	return 0;
}

int BubbleProtocolEx::cbRecFileSearchFail( QVariantMap &evmap )
{
	eventProcCall("recFileSearchFail",evmap);
	return 0;
}

int BubbleProtocolEx::PTZUp( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Up;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZDown( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Down;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZLeft( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Left;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZRight( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Right;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZIrisOpen( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_IrisOpen;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZIrisClose( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_IrisClose;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZFocusFar( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_FocusFar;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZFocusNear( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_FocusNear;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZZoomIn( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_ZoomIn;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZZoomOut( const int &nChl, const int &nSpeed )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_ZoomOut;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nSpeed;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZAuto( const int &nChl, bool bOpend )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Auto;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=bOpend;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int BubbleProtocolEx::PTZStop( const int &nChl, const int &nCmd )
{
	if (m_tCurrentConnectStatus==BUBBLE_CONNECTED)
	{
		m_csStepCode.lock();
		tagPtzItem tPtzItem;
		tPtzItem.nAction=Ptz_Stop;
		tPtzItem.nChannel=nChl;
		tPtzItem.nSpeed=nCmd;
		m_tBubblePtz.enqueue(tPtzItem);
		m_tStepCode.enqueue(BUBBLE_PTZ);
		m_csStepCode.unlock();
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"PTZUp as current connect status is not in connected";
		return 1;
	}
}

int cbXBubbleFoundFile( QString evName,QVariantMap evMap,void*pUser )
{
	if ("foundFile"==evName)
	{
		((BubbleProtocolEx*)pUser)->cbFoundFile(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"evName is not match the func,evName:"<<evName;
		return 1;
	}
}

int cbXBubbleRecFileSearchFail( QString evName,QVariantMap evMap,void*pUser )
{
	if ("recFileSearchFail"==evName)
	{
		((BubbleProtocolEx*)pUser)->cbRecFileSearchFail(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"evName is not match the func,evName:"<<evName;
		return 1;
	}
}

int cbXBubbleRecFileSearchFinished( QString evName,QVariantMap evMap,void*pUser )
{
	if ("recFileSearchFinished"==evName)
	{
		((BubbleProtocolEx*)pUser)->cbRecFileSearchFinished(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"evName is not match the func,evName:"<<evName;
		return 1;
	}
}
