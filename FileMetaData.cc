#include "FileMetaData.hh"
#include <iostream>

FileMetaData::FileMetaData()
{}

void FileMetaData::newFiles(QStringList list)
{
	QCA::Initializer qcainit;
	QStringListIterator i(list);
	while (i.hasNext())
	{
		QString str  = i.next();
		QFile file(str);
		file.open(QIODevice::ReadOnly);
		QDataStream in(&file);
		char *s = (char*)malloc(8192);
		QByteArray array;
		int numBytesRead = 0;
		int numBlocks = ceil(file.size()/8192.0);
		for (int i = 0; i < numBlocks; ++i)
		{
			memset(s, 0, 8192);
			numBytesRead = in.readRawData(s, 8192);
			QByteArray qba(s, numBytesRead);
			array.append(QCA::Hash("sha256").hash(qba).toByteArray());
		}
		metadata m;
		m.fileName = str;
		m.fileSize = file.size();
		m.blocklist = array;
		m.fileId = QByteArray().append(QCA::Hash("sha256").hash(array).toByteArray());
		files.append(m);
		qDebug() << "---" << m.blocklist.constData() << "---";
		qDebug() << m.fileId.constData() << "\n";
	}
}

int FileMetaData::contains(QByteArray hash)
{
	for (int i = 0; i < files.size(); ++i) 
	{
		if (files.at(i).fileId == hash || files.at(i).blocklist.contains(hash))
		{
			return i;
		}
	}
	return -1;
}

int FileMetaData::getBlockNumber(QByteArray hash, int i)
{
	if(files.at(i).fileId == hash)
		return -1;
	return files.at(i).blocklist.indexOf(hash)/32;
}

QByteArray FileMetaData::getFileId(int position)
{
	return files.at(position).fileId;
}

QByteArray FileMetaData::getBlocklist(int position)
{
	return files.at(position).blocklist;
}

QByteArray FileMetaData::getBlock(int position, int blockNumber)
{
	QFile file(files.at(position).fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	char *s = (char*)malloc(8192);
	QByteArray array;
	int numBytesRead = 0;
	for (int i = 0; i <= blockNumber; ++i)
	{
		memset(s, 0, 8192);
		numBytesRead = in.readRawData(s, 8192);
	}
	// std::cout << s;
	return QByteArray(s, numBytesRead);
}

QByteArray FileMetaData::hash(QByteArray array)
{
	QCA::Initializer qcainit;
	return QByteArray().append(QCA::Hash("sha256").hash(array).toByteArray()); //hashToString
}

QVariantMap FileMetaData::search(QString text)
{
	QStringList list = text.split(QRegExp(" "));
	QVariantList matchNames;
	QVariantList matchIds;
	for(int i=0; i<list.size(); i++)
	{
		for(int j=0; j< files.size(); j++)
		{
			if (files.at(j).fileName.contains(list.at(i)))
			{
				matchNames << files.at(j).fileName;
				matchIds << files.at(j).fileId;
			}
		}
	}
	QVariantMap qvm;
	if (!matchNames.isEmpty())
	{
		qvm["MatchNames"] = matchNames;
		qvm["MatchIDs"] = matchIds;
	}
	return qvm;
}