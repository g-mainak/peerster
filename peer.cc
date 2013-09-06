#include "peer.hh"

// Peer::Peer()
// {
// 	qDebug() << "Error! Peer called with 0 arhuments!";
// }

// Peer::Peer(const Peer& p)
// {
// 	hostname = p.hostname;
// 	ip = p.ip;
// 	port = p.port;
// 	disabled = p.disabled;
// }

Peer::Peer(QString str)
{
	QStringList list = str.split(QRegExp(":"));
	assert(list.size() == 2);
	if (QHostAddress(list.at(0)).isNull())
	{
		hostname = list.at(0);
		QHostInfo::lookupHost(hostname, this, SLOT(lookedUp(QHostInfo)));
		disabled = true;
	}
	else
	{
		ip = QHostAddress(list.at(0));
		disabled = false;
		// hostname
	}
	port = list.at(1).toUInt();
	qDebug() << "Peer initiated with "<<hostname<<ip<<port;
}

Peer::Peer(QHostAddress address, quint16 port)
{
	ip = address;
	this->port = port;
	disabled = false;
	qDebug() << "Peer initiated with "<<hostname<<ip<<port;
}

void Peer::lookedUp(QHostInfo info)
{
	if (!info.addresses().isEmpty())
	{
    	ip = info.addresses().first();
    	disabled = false;
    }
}


QHostAddress Peer::getIp()
{
	return ip;
}

quint16 Peer::getPort()
{
	return port;
}