#include "netsocket.hh"

NetSocket::NetSocket()
{
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;

	QStringList args = QCoreApplication::arguments();
	for(int i=1; i< args.size(); i++)
		peers << new Peer(args.at(i));
}

bool NetSocket::bind()
{
	bool flag = false;
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) 
	{
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			flag = true;
		}
		else
		{
			Peer *n = new Peer(QHostAddress::LocalHost, p);
			peers << n;
		}
	}
	return flag;
	// qDebug() << "Oops, no ports in my default range " << myPortMin
	// 	<< "-" << myPortMax << " available";
	// return false;
}

quint16 NetSocket::randomPeer()
{
	return (rand() % peers.size());
}

void NetSocket::transmitAll(QByteArray array)
{
	for(int i = 0; i < peers.size(); i++) 
		if (peers[i]->notDisabled())
			if (!writeDatagram(array, peers[i]->getIp(), peers[i]->getPort()))
			{	
				qDebug() << "Could not send datagram. Sorry!";
			}
}

void NetSocket::transmit(QByteArray array, quint16 peerNumber)
{
	if (peers[peerNumber]->notDisabled())
		if (!writeDatagram(array, peers[peerNumber]->getIp(), peers[peerNumber]->getPort()))
			{	
				qDebug() << "Could not send datagram. Sorry!";
			}
}

quint16 NetSocket::findPeer(QHostAddress address, quint16 port)
{
	for (int i = 0; i < peers.size(); ++i)
    	if (peers.at(i)->getIp() == address && peers.at(i)->getPort() == port)
    		return i;
    peers << new Peer(address, port);
    return peers.size() - 1;
}

void NetSocket::addPeer(QString str)
{
	peers << new Peer(str);
}

Peer *NetSocket::getPeer(quint16 peerIndex)
{
	if (peerIndex < peers.size())
		return peers.at(peerIndex);
	else
		return NULL;
}

quint16 NetSocket::getNumPeers()
{
	return peers.size();
}