#ifndef _SUBWEBWIEW_HEADFILE_H_
#define _SUBWEBWIEW_HEADFILE_H_
#include <IActivities.h>

#include <QtWebKit/QtWebKit>
#include <qwfw.h>
#include <QWebInspector>
class SubWebView: public QWebView
{
	Q_OBJECT
public:
	explicit SubWebView(QString nurl,QSize mSize,QWidget *parent = 0);
	virtual ~SubWebView(void);
public:
	QString EventProcsScripte(QString sEvent,QVariantMap eventParam);
	bool IsLoad;
	static volatile bool bIsbuilding;
protected:
	virtual void keyPressEvent(QKeyEvent* ev);
signals:
	void LoadOrChangeUrl(const QString &text);
	void CloseAllPage();
	public slots:
		void OnLoad(bool bOk);
		void OnstatusBarMessage(const QString &text);
		void OnurlChanged(const QUrl & url);
		void OnRefressMessage();
private:
	QString m_sApplicationPath;
	QString m_url;
	QString statusBarMessage;
	
private:
	IActivities *m_Activity;
	QSize m_Size;

#ifdef __USE_WEB_DEBUGER__
	QWebInspector m_webinspector;
#endif
};


#endif
