#ifndef PEERSTER_FILEDOWNLOAD_HH
#define PEERSTER_FILEDOWNLOAD_HH

#include <QFile>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include <QtCrypto>
#include <QByteArray>

struct download
{
	QString origin;
	QString fileName;
	quint32 numblocks;
	QList<int> blocks;
	QByteArray blocklist;
	QByteArray fileId;
	QByteArray fileContents;
};

class FileDownload
{
	public:
		FileDownload();
		void newDownload(QString, QByteArray);
		int find(QString, QByteArray);
		int insert(int, QVariantMap);
		QByteArray getBlockRequest(int, int);

	private:
		QVector<download> downloads;
};

#endif