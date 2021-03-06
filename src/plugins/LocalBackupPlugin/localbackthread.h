#ifndef LOCALBACKTHREAD_H
#define LOCALBACKTHREAD_H

#include <QThread>
#include <QDateTime>
#include <QStringList>
#include <QVariantMap>
#include "globalsetting.h"
#include "sqlite3.h"
#include "IStorage.h"

class LocalBackThread : public QThread
{
	Q_OBJECT

public:
	LocalBackThread(QObject *parent = 0);
	~LocalBackThread();
	void setBackupPath(QString sPath);
	int startLocalFileBackUp(int nTypes, QString sChannel, QDateTime startTime, QDateTime endTime);
	void stopLocalFileBackUp();
signals:
	void sendMsg(QString evName, QVariantMap item);
protected:
	void run();
private:
	bool getFileList(QStringList &list);
	void createSQLCmd(QString &sql);
	int execCommand(sqlite3 *pdb, const char* cmd, char*** pppRet, int* row, int* col, char** pMsg);
	void appendTimePath(QList<TimePath> &tpList, const uint &start, const QString &path, qint32 &insertPos);
	int countPts(QMap<int,int> ptsMap);
	void lockFile(QString fileName, bool locked);
	sqlite3* initDataBase(char *dbPath);
	void sleepEx(int msec);
	void initFileSystem();
	bool checkDisk(QString sPath);

private:
	QMap<int, FdStatusInfo> m_chlFdMap;
	QMap<QString, sqlite3*> m_sqlMap;
	QString m_sDiskList;
	QString m_sBackupPath;
	int m_nTypes;
	QString m_sChls;
	uint m_nStartSec;
	uint m_nEndSec;
	bool m_bStop;
	IStorage *m_pStorage;
};

#endif // LOCALBACKTHREAD_H
