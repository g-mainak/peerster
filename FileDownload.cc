#include "FileDownload.hh"

FileDownload::FileDownload()
{}

void FileDownload::newDownload(QString origin, QByteArray fileId)
{
	download d;
	d.origin = origin;
	d.fileId = fileId;
	downloads.append(d);
}

int FileDownload::find(QString origin, QByteArray hash)
{
	for(int i=0; i<downloads.size(); i++)
	{
		if (downloads.at(i).origin == origin)
		{
			download d = downloads.at(i);
			if (d.fileId == hash || d.blocklist.contains(hash))
				return i;
		}
	}
	return -1;
}

int FileDownload::insert(int position, QVariantMap qvm)
{
	QByteArray incomingHash = qvm.value("BlockReply").toByteArray();
	download d = downloads.at(position);
	if (incomingHash == d.fileId)
	{
		d.blocklist = qvm.value("Data").toByteArray();
		d.numblocks = d.blocklist.size()/32;
		for(quint32 i=0; i<d.numblocks; i++)
			d.blocks << i;
	}
	else if (d.blocklist.contains(incomingHash))
	{
		int blockNum = d.blocklist.indexOf(incomingHash)/32;
		QByteArray data = qvm.value("Data").toByteArray();
		if (blockNum < 0)
			qDebug() << "ERROR";
		else
		{
			d.fileContents.replace(blockNum*8192, data.size(), data);
			d.numblocks--;
			d.blocks.removeOne(blockNum);
			if (!d.numblocks)
			{
				QFile file("file");
				file.open(QIODevice::WriteOnly);
				QDataStream out(&file);
				out.writeRawData(d.fileContents, d.fileContents.size());
			}
		}
	}
	downloads.replace(position, d);
	return (d.blocks.isEmpty())?-1:d.blocks.first();
}

QByteArray FileDownload::getBlockRequest(int downloadNumber, int blockNumber)
{
	return downloads.at(downloadNumber).blocklist.mid(blockNumber*32, 32);
}