#include "netsocket.hh"

NetSocket::NetSocket()
{
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
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
	srand(time(NULL));
	return (rand() % peers.size());
}

void NetSocket::broadcastOnRevolvingFrequencies(QByteArray array)
{
	for (int i = myPortMin; i <= myPortMax; ++i)
		if (!writeDatagram(array, QHostAddress(QHostAddress::LocalHost), i))
		{	// Error!
		}
}

void NetSocket::transmit(QByteArray array, quint16 peerNumber)
{
	if (!writeDatagram(array, peers[peerNumber]->getIp(), peers[peerNumber]->getPort()))
		{	// Error!
		}
}

quint16 NetSocket::findPeer(QHostAddress address, quint16 port)
{
	for (int i = 0; i < peers.size(); ++i)
    	if (peers.at(i)->getIp() == address && peers.at(i)->getPort() == port)
    		return i;
    Peer p(address, port);
    peers << &p;
    return peers.size() - 1;
}