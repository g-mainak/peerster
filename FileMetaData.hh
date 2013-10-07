#ifndef PEERSTER_FILEMETADATA_HH
#define PEERSTER_FILEMETADATA_HH

#include <QFile>
#include <QStringList>
#include <QDebug>
#include <cmath>
#include <QtCrypto>
#include <QByteArray>

struct metadata
{
	QString fileName;
	quint32 fileSize;
	QByteArray blocklist;
	QByteArray fileId;
};

class FileMetaData
{
	public:
		FileMetaData();
		void newFiles(QStringList);
		int contains(QByteArray);
		QByteArray getBlockReply(quint32);
		int getBlockNumber(QByteArray, quint32);
		QByteArray getData(quint32);
		int getBlockNumber(QByteArray hash, int i);
		QByteArray getFileId(int position);
		QByteArray getBlock(int position, int blockNumber);
		QByteArray getBlocklist(int position);
		static QByteArray hash(QByteArray array);
		QVariantMap search(QString);

	private:
		QVector<metadata> files;
};

#endif