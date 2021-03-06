#include "BufferQueue.h"
#include <QDebug>

BufferQueue::BufferQueue():m_nQueueMaxSize(0),
	m_nRecordStatus(0),
	m_nLoseFrameCount(0)
{
	m_uiAvailableSize=CHLBUFFERSIZE*1024*1024;
	m_tDataQueue.prealloc(NODELISTSIZE);
}


BufferQueue::~BufferQueue()
{
}
bool BufferQueue::testFrame( QVariantMap tItem )
{
	int nType=tItem["frametype"].toUInt();
	if (nType!=FT_Audio&&nType!=FT_IFrame&&nType!=FT_PFrame)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"frame type undefined:"<<tItem;
		return false;
	}else{
		return true;
	}
}
bool BufferQueue::enqueue( QVariantMap tFrameInfo )
{
	//I:tagFrameHead+tagVideoConfigFrame
	//P:tagFrameHead
	//A:tagFrameHead+tagAudioConfigFrame

	// test frame isCorrect;

	if (testFrame(tFrameInfo))
	{
		//keep going
	}else{
		return false;
	}
	m_tEnqueueDataLock.lock();
	m_tDataLock.lock();
	if (m_nQueueMaxSize>=3)
	{
		bool bFlag=false;
		int nCount=0;
		int nIframeCount=0;
		while(bFlag==false){
			RecBufferNode *pRecBufferNode=NULL;
			if (m_tDataQueue.size()>0)
			{
				pRecBufferNode=(RecBufferNode*)m_tDataQueue.front();
				tagFrameHead *pFrameHead=NULL;
				pRecBufferNode->getDataPointer(&pFrameHead);
				if (pFrameHead->uiType==IFRAME||pFrameHead->uiType==AFRMAE||pFrameHead->uiType==PFRAME)
				{
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"undefined type,it must out of memory range,it will abort the system,please preallocation more memory";
					abort();
				}
				if (NULL!=pFrameHead)
				{
					if (pFrameHead->uiType==IFRAME)
					{
						if (m_nRecordStatus!=0&&nCount!=0)
						{
							qDebug()<<__FUNCTION__<<__LINE__<<"wind:"<<pFrameHead->uiChannel<<"m_nRecordStatus"<<m_nRecordStatus<<"remove I Frame"<<"and total num:"<<nCount;
						}else{
							//do nothing
						}
						nIframeCount=nIframeCount+1;
						if (nIframeCount==2)
						{
							bFlag=true;
						}else{
							//do nothing
						}
					}else{
						//keep going
					}
					nCount++;
					if (bFlag==false)
					{
						RecBufferNode *pRemoveRecBufferNode=NULL;
						tagFrameHead *pRemoveFrameHead=NULL;
						pRemoveRecBufferNode=(RecBufferNode*)m_tDataQueue.dequeue();
						pRemoveRecBufferNode->getDataPointer(&pRemoveFrameHead);
						if (pRemoveFrameHead->uiType==IFRAME)
						{
							m_nQueueMaxSize--;
						}else{
							//do nothing
						}
						int nDataLength=pRemoveFrameHead->uiLength;
						int nFrameHeadLength=sizeof(tagFrameHead);
						int nApplyLength=nDataLength+nFrameHeadLength-sizeof(char*);
						if (pRemoveFrameHead->uiType==IFRAME)
						{
							nApplyLength=nApplyLength+sizeof(tagVideoConfigFrame);
						}else if (pRemoveFrameHead->uiType==AFRMAE)
						{
							nApplyLength=nApplyLength+sizeof(tagAudioConfigFrame);
						}else{
							//do nothing
						}
						if (NULL!=pRemoveRecBufferNode)
						{
							pRemoveRecBufferNode->release();
							pRemoveRecBufferNode=NULL;
						}else{
							bFlag=true;
						}
					}else{
						//do nothing
					}
				}else{
					//do nothing
					bFlag=true;
				}
			}else{
				bFlag=true;
			}
		}	
	}else{
		//keep going
	}
	int nDataLength=tFrameInfo["length"].toInt();
	int nFrameHeadLength=sizeof(tagFrameHead);
	int nApplyLength=nDataLength+nFrameHeadLength-sizeof(char*);
	if (tFrameInfo["frametype"].toUInt()==IFRAME)
	{
		nApplyLength=nApplyLength+sizeof(tagVideoConfigFrame);
		m_nQueueMaxSize++;
	}else if (tFrameInfo["frametype"].toUInt()==AFRMAE)
	{
		nApplyLength=nApplyLength+sizeof(tagAudioConfigFrame);
	}else{
		//do nothing
	}
	RecBufferNode *pRecBufferNode=new RecBufferNode;;
	tagFrameHead *pFrameHead=NULL;
	pFrameHead=(tagFrameHead*)m_tAllocation.applySpace(nApplyLength);
	if (pFrameHead!=NULL)
	{
		pFrameHead->uiChannel=tFrameInfo["winid"].toUInt();
		pFrameHead->uiExtension=0;//
		pFrameHead->uiGentime=QDateTime::currentDateTime().toTime_t();
		pFrameHead->uiLength=nDataLength;
		pFrameHead->uiPts=tFrameInfo["pts"].toULongLong()/1000;
		pFrameHead->uiRecType=0;//
		pFrameHead->uiSessionId=0;//
		pFrameHead->uiType=tFrameInfo["frametype"].toUInt();
		char *pData=(char*)tFrameInfo["data"].toUInt();
		memcpy(&pFrameHead->pBuffer,pData,nDataLength);
		if (pFrameHead->uiType==IFRAME)
		{
			tagVideoConfigFrame *pVideoConfigFrame=(tagVideoConfigFrame*)((char*)pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*));
			pVideoConfigFrame->uiHeight=tFrameInfo["height"].toInt();
			pVideoConfigFrame->uiWidth=tFrameInfo["width"].toInt();
			pVideoConfigFrame->ucReversed;//未填充
			pVideoConfigFrame->ucVideoDec;//未填充
		}else if (pFrameHead->uiType==AFRMAE)
		{
			tagAudioConfigFrame *pAudioConfigFrame=(tagAudioConfigFrame*)((char*)pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*));
			pAudioConfigFrame->uiSamplebit=tFrameInfo["samplewidth"].toInt();
			pAudioConfigFrame->uiSamplerate=tFrameInfo["samplerate"].toInt();
			pAudioConfigFrame->uiChannels=tFrameInfo["audiochannel"].toInt();
		}else{
			//do nothing
		}
		pRecBufferNode->setDataPointer(pFrameHead);
		m_tDataQueue.enqueue(pRecBufferNode);
		m_tDataLock.unlock();
		m_tEnqueueDataLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"enqueue fail as allocation fail,there may be run out of memory";
		abort();
	}
	m_tDataLock.unlock();
	m_tEnqueueDataLock.unlock();
	return false;
}

bool BufferQueue::setSize( int nMax )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"setSize fail as this func is wait for fill out";
	abort();
	return false;
}

bool BufferQueue::isEmpty()
{
	m_tDataLock.lock();
	if (m_tDataQueue.isEmpty())
	{
		m_tDataLock.unlock();
		return true;
	}else{
		m_tDataLock.unlock();
		return false;
	}
}

void BufferQueue::clear()
{
	m_tEnqueueDataLock.lock();
	m_tDataLock.lock();
	while(!m_tDataQueue.isEmpty()){
		RecBufferNode *pRecBufferNode=NULL;
		pRecBufferNode=(RecBufferNode*)m_tDataQueue.dequeue();
		if (NULL!=pRecBufferNode)
		{
			pRecBufferNode->release();
			pRecBufferNode=NULL;
		}else{
			//do nothing
		}
	}
	m_nQueueMaxSize=0;
	m_uiAvailableSize=CHLBUFFERSIZE*1024*1024;
	m_tDataLock.unlock();
	m_tEnqueueDataLock.unlock();
}

RecBufferNode* BufferQueue::dequeue()
{
	RecBufferNode* pRecBufferNode=NULL;
	m_tDataLock.lock();
	if (m_tDataQueue.size()>0)
	{
		pRecBufferNode=(RecBufferNode*)m_tDataQueue.dequeue();
		tagFrameHead *pFrameHead=NULL;
		pRecBufferNode->getDataPointer(&pFrameHead);
		if (NULL!=pFrameHead)
		{
			if (pFrameHead->uiType==IFRAME)
			{
				if (m_nQueueMaxSize>0)
				{
					m_nQueueMaxSize--;
				}else{
					//do nothing
				}
			}else{
				//do nothing
			}
			//int nDataLength=tFrameInfo["length"].toInt();
			int nDataLength=pFrameHead->uiLength;
			int nFrameHeadLength=sizeof(tagFrameHead);
			int nApplyLength=nDataLength+nFrameHeadLength-sizeof(char*);
			if (pFrameHead->uiType==IFRAME)
			{
				nApplyLength=nApplyLength+sizeof(tagVideoConfigFrame);
			}else if (pFrameHead->uiType==AFRMAE)
			{
				nApplyLength=nApplyLength+sizeof(tagAudioConfigFrame);
			}else{
				//do nothing
			}
		}else{
			//
		}
	}else{

	}
	m_tDataLock.unlock();
	return pRecBufferNode;
}

RecBufferNode * BufferQueue::front()
{
	RecBufferNode* pRecBufferNode=NULL;
	m_tDataLock.lock();
	if (m_tDataQueue.size()>0)
	{
		pRecBufferNode=(RecBufferNode*)m_tDataQueue.front();
		pRecBufferNode->addRef();
	}else{

	}
	m_tDataLock.unlock();
	return pRecBufferNode;
}

void BufferQueue::enqueueDataLock()
{
	m_tEnqueueDataLock.lock();
}

void BufferQueue::enqueueDataUnLock()
{
	m_tEnqueueDataLock.unlock();
}

void BufferQueue::setRecordStatus(int nRecordStatus)
{
	m_nRecordStatus=nRecordStatus;
}

int BufferQueue::getSize()
{
	return m_nQueueMaxSize;
}



