#include "RecordPlayerView.h"
#include <QtCore>
#include <QtGui/QPainter>
#include <QSettings>
#include <QMouseEvent>
#include "guid.h"
#include "IVideoDisplayOption.h"
#include "ICommunicate.h"
#include "IUserManagerEx.h"
#include <QApplication>
#include <QDomDocument>

bool RecordPlayerView::m_bGlobalAudioStatus = false;
bool RecordPlayerView::m_bSuspensionVisable = false;
SuspensionWnd* RecordPlayerView::ms_susWnd = NULL;
int RecordPlayerView::ms_playStatus = 4;//stop local play
QMap<quintptr, QRect> RecordPlayerView::ms_rectMap;

RecordPlayerView::RecordPlayerView(QWidget *parent)
	: QWidget(parent),
	m_pLocalPlayer(NULL),
	_bIsFocus(false),
	m_bPlaying(false),
	m_bPressed(false)
{
	this->lower();
	this->setAttribute(Qt::WA_PaintOutsidePaintEvent);
	m_pWindowsStretchAction = m_WindowMenu.addAction(tr("Suit For Window"));
	m_pWindowsStretchAction->setCheckable(true);
	m_pWindowsStretchAction->setChecked(false);

	connect(m_pWindowsStretchAction,SIGNAL(triggered(bool)),this,SLOT(slSuitForWindow(bool)));

	if (!ms_susWnd){
		ms_susWnd = new SuspensionWnd(this);
		connect(ms_susWnd, SIGNAL(sigClose()), this, SLOT(slCloseSusWnd()));
		ms_susWnd->setWindowFlags(Qt::Window);
		ms_susWnd->setWindowFlags(this->windowFlags() &~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
		ms_susWnd->setCbFunc(cbReciveMsg, this);
		ms_susWnd->setWindowTitle(tr("Zoom"));
	}
}


RecordPlayerView::~RecordPlayerView(void)
{
// 	destroySusWnd();
}


void RecordPlayerView::paintEvent( QPaintEvent * e)
{
	QPainter p(this);

	QString image;
	QColor LineColor(45,49,54);
	QColor LineCurColor;
	QColor FontColor(32,151,219);
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);
	QString sBackground;

	image = IniFile.value("background/background-image", NULL).toString();
//	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());
	LineCurColor.setNamedColor(IniFile.value("background/background-color-current", QVariant("")).toString());
	FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
	FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
	FontFamily = IniFile.value("font/font-family", NULL).toString();
	sBackground=IniFile.value("text/background", QVariant("")).toString();

	QRect rcClient = contentsRect();

	QPixmap pix;
	QString PixPaht = sAppPath + image;
	bool ret = pix.load(PixPaht);

	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

	p.drawPixmap(rcClient,pix);

	QPen pen = QPen(LineColor, 2);
	p.setPen(pen);

	p.drawRect(rcClient);
	if (_bIsFocus)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		pen.setWidth(2);
		pen.setColor(LineCurColor);
		p.setPen(pen);
		p.drawRect(QRect(x,y ,width , height));
	}
	else
	{
		p.drawRect(rcClient);
	}

	//QFont font(FontFamily, FontSize, QFont::Bold);
	QFont font(FontFamily, FontSize, QFont::Bold|QFont::System);
	p.setFont(font);

	pen.setColor(FontColor);

	p.setPen(pen);

	p.drawText(rcClient, Qt::AlignCenter, sBackground);
}

void RecordPlayerView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void RecordPlayerView::mousePressEvent(QMouseEvent *ev)
{
	if (Qt::LeftButton == ev->button()){
		m_bPressed = true;
		m_pressPoint = ev->pos();

//		qDebug()<<__FUNCTION__<<m_pressPoint;

		if (ms_susWnd && ms_susWnd->isVisible()){
			//validation
			if (verify(100, 0)){
				if ((QWidget*)this != ms_susWnd->getTopWnd() ){
					closeSuspensionWnd();
				}
// 				clearOriginRect();
				return;
			}
			//notify play module current window need to zoom
			ICommunicate *pCom = NULL;
			m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
			if (pCom){
				QVariantMap msg;
				msg.insert("SusWnd", (quintptr)ms_susWnd);
				msg.insert("CurWnd", (quintptr)this);
	// 			if (ms_rectMap.contains((quintptr)this) && (QWidget*)this != ms_susWnd->getTopWnd()/* && ms_susWnd->isVisible()*/){
	// 				msg.insert("ZoRect", ms_rectMap[(quintptr)this]);
	// 				ms_susWnd->setDrawRect(ms_rectMap[(quintptr)this]);
	// 			}
				if (ms_rectMap.contains((quintptr)this)){
					QRect drawRect = ms_rectMap[(quintptr)this];
					//Coordinate Conversion
					float widthRate = (float)ms_susWnd->width()/this->width();
					float heightRate = (float)ms_susWnd->height()/this->height();
					drawRect.setCoords(drawRect.left()*widthRate, drawRect.top()*heightRate, drawRect.right()*widthRate, drawRect.bottom()*heightRate);
	// 				ms_rectMap[(quintptr)this] = drawRect;

					msg.insert("ZoRect", drawRect);
					ms_susWnd->setDrawRect(drawRect);
				}else{
					msg.insert("ZoRect", QRect(1, 1, 1, 1));
					ms_susWnd->setDrawRect(QRect(0, 0, 0, 0));
				}
				msg.insert("Width", ms_susWnd->width());
				msg.insert("Height", ms_susWnd->height());
				ms_susWnd->addWnd(this);
				pCom->setInfromation(QString("VedioZoom"), msg);
				pCom->Release();
			}
		}
	}

	setFocus(Qt::MouseFocusReason);
	emit SetCurrentWindSignl(this);
	if (m_bGlobalAudioStatus && NULL != m_pLocalPlayer && ev->button() == Qt::LeftButton)
	{
		m_pLocalPlayer->GroupSetVolume(0xAECBCA, this);
	}
	//if (Qt::RightButton == ev->button())
	//{
	//	if (NULL != m_pWindowsStretchAction)
	//	{
	//		m_pWindowsStretchAction->setText(tr("Suit For Window"));
	//	}
	//	m_WindowMenu.exec(QCursor::pos());
	//}
}

void RecordPlayerView::setLocalPlayer(ILocalPlayerEx* pPlayer)
{
	if (NULL != pPlayer)
	{
		m_pLocalPlayer = pPlayer;
	}
}

int RecordPlayerView::AudioEnabled(bool bEnabled)
{
	if (NULL == m_pLocalPlayer)
	{
		return 1;
	}
	m_pLocalPlayer->GroupSetVolume(0xAECBCB,this);
	m_pLocalPlayer->GroupEnableAudio(bEnabled);
	m_bGlobalAudioStatus = bEnabled;

	return 0;
}



void RecordPlayerView::SetFocus( bool flags )
{
	bool history=_bIsFocus;
	_bIsFocus=flags;
	if (_bIsFocus==true||history==true)
	{
		update();
	}
}

void RecordPlayerView::slSuitForWindow( bool checked )
{
	if (NULL != m_pLocalPlayer)
	{
		IVideoDisplayOption * pi;
		m_pLocalPlayer->QueryInterface(IID_IVideoDisplayOption,(void **)&pi);
		if (NULL != pi)
		{
			pi->enableWindowStretch(this,!checked);
			pi->Release();
		}
	}
	m_pWindowsStretchAction->setChecked(checked);
}

void RecordPlayerView::changeEvent( QEvent * )
{
	if (NULL != m_pWindowsStretchAction)
	{
		m_pWindowsStretchAction->setText(tr("Suit For Window"));
	}
	if (ms_susWnd){
		ms_susWnd->setWindowTitle(tr("Zoom"));
	}
}

void RecordPlayerView::mouseReleaseEvent( QMouseEvent *ev )
{
	if (Qt::LeftButton == ev->button()){
		QRect mainRect = this->rect();
		QRect drawRect(m_pressPoint, ev->pos());
		m_bPressed = false;
		drawRect = drawRect.intersected(mainRect);

	// 	qDebug()<<__FUNCTION__<<ev->pos();

		clearOriginRect();
		if (drawRect.width()*drawRect.height() < 1000){
// 			clearOriginRect();
			return;
		}
		//if release point in current window
		if (drawRect.width()*drawRect.height()/1000 /*&& mainRect.contains(drawRect) */
			&& (QWidget*)this != ms_susWnd->getTopWnd() 
			&& ms_playStatus < 4 && m_bPlaying
			&& !ms_susWnd->isVisible()){
			//validation
			if (verify(100, 0)){
// 				clearOriginRect();
				return;
			}
			//Coordinate Conversion
			QRect subRect = drawRect;
			float widthRate = (float)ms_susWnd->width()/this->width();
			float heightRate = (float)ms_susWnd->height()/this->height();
			subRect.setCoords(drawRect.left()*widthRate, drawRect.top()*heightRate, drawRect.right()*widthRate, drawRect.bottom()*heightRate);

			ms_susWnd->addWnd(this);
			ms_susWnd->setDrawRect(subRect);
			ms_susWnd->show();
			ms_susWnd->setOriginGeog(ms_susWnd->geometry());
			//notify play module current window need to zoom
			ICommunicate *pCom = NULL;
			m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
			if (pCom){
				QVariantMap msg;
				msg.insert("SusWnd", (quintptr)ms_susWnd);
				msg.insert("CurWnd", (quintptr)this);
				msg.insert("ZoRect", subRect);
				msg.insert("Width", ms_susWnd->width());
				msg.insert("Height", ms_susWnd->height());
				pCom->setInfromation(QString("VedioZoom"), msg);
				pCom->Release();
				ms_rectMap[(quintptr)this] = drawRect;
				m_bSuspensionVisable = true;
			}
		}
	}
}

void RecordPlayerView::recMsg( QVariantMap msg )
{
	QString evName = msg["EvName"].toString();
	ICommunicate *pCom = NULL;
	m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
	if (pCom){
		msg.remove("EvName");
		pCom->setInfromation(evName, msg);
		pCom->Release();
	}
	if ("ZoomRect" == evName){
		QRect rect = msg["ZoRect"].toRect();
		if (rect.topLeft().x() == rect.bottomRight().x()){
			ms_rectMap[msg["CurWnd"].toUInt()] = msg["ZoRect"].toRect();
		}else{
			//Coordinate Conversion
			float wideRate = (float)this->width()/ms_susWnd->width();
			float heightRate = (float)this->height()/ms_susWnd->height();
			rect.setCoords(rect.left()*wideRate, rect.top()*heightRate, rect.right()*wideRate, rect.bottom()*heightRate);
			ms_rectMap[msg["CurWnd"].toUInt()] = rect;
		}
	}
}

void RecordPlayerView::setPlayStatus( int status )
{
	ms_playStatus = status;
}

void RecordPlayerView::setPlayingFlag( bool bPlaying )
{
// 	qDebug()<<(int)this<<"origin flag:"<<m_bPlaying<<" set flag:"<<bPlaying;
	m_bPlaying = bPlaying;
}

void RecordPlayerView::mouseMoveEvent( QMouseEvent *ev )
{
	QRect mainRect = this->rect();
	QRect drawRect(m_pressPoint, ev->pos());
	if (m_bPressed && mainRect.contains(drawRect) /*&& drawRect.width()*drawRect.height()/1000*/ && ms_playStatus < 4 && m_bPlaying){
		//notify play module current window need to zoom
		if (m_pLocalPlayer){
			ICommunicate *pCom = NULL;
			m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
			if (pCom){
				QVariantMap msg;
				msg.insert("CurWnd", (quintptr)this);
				msg.insert("ZoRect", drawRect);
				pCom->setInfromation(QString("RectToOrigion"), msg);
				pCom->Release();
			}
		}
	}
}

void RecordPlayerView::slCloseSusWnd()
{
	ICommunicate *pCom = NULL;
	m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
	if (pCom){
		pCom->setInfromation(QString("CloseWnd"), QVariantMap());
		pCom->Release();
	}
	ms_susWnd->hide();
	ms_rectMap.clear();
	m_bSuspensionVisable = false;
}

void RecordPlayerView::showSusWnd( bool enabled )
{
	if (enabled && m_bSuspensionVisable){
		ms_susWnd->show();
	}else{
		ms_susWnd->hide();
	}
}

void RecordPlayerView::destroySusWnd()
{
	if (ms_susWnd){
// 		ms_susWnd->close();
		slCloseSusWnd();
		delete ms_susWnd;
		ms_susWnd = NULL;
	}
}

void RecordPlayerView::closeSuspensionWnd()
{
	if (ms_susWnd){
		ms_susWnd->close();
	}
}

bool RecordPlayerView::verify( quint64 mainCode, quint64 subCode )
{
	int ret = -1;
	IUserManagerEx *pUserMgr = NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IUserMangerEx,(void **)&pUserMgr);
	if (pUserMgr){
		ret = pUserMgr->checkUserLimit(mainCode, subCode);
		if (ret){
			QVariantMap vmap;
			vmap.insert("MainPermissionCode", qint64(mainCode));
			vmap.insert("SubPermissionCode", qint64(subCode));
			vmap.insert("ErrorCode", ret);
			emit sigValidateFail(vmap);
			ret = pUserMgr->checkUserLimit(mainCode, subCode);
			if (ret==2){
				QVariantMap vmap;
				vmap.insert("MainPermissionCode", qint64(mainCode));
				vmap.insert("SubPermissionCode", qint64(subCode));
				vmap.insert("ErrorCode", ret);
				emit sigValidateFail(vmap);
			}
		}
		pUserMgr->Release();
	}
	return ret;
}

void RecordPlayerView::clearOriginRect()
{
	if (m_pLocalPlayer){
		ICommunicate *pCom = NULL;
		m_pLocalPlayer->QueryInterface(IID_ICommunicate, (void**)&pCom);
		if (pCom){
			QVariantMap msg;
			msg.insert("CurWnd", (quintptr)this);
			msg.insert("ZoRect", QRect(1, 1, 1, 1));
			pCom->setInfromation(QString("RectToOrigion"), msg);
			pCom->Release();
		}
	}
}

void cbReciveMsg( QVariantMap evMap, void* pUser )
{
	((RecordPlayerView*)pUser)->recMsg(evMap);
}
