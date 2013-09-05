#include "main.hh"

NetSocket::NetSocket()
{
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

quint16 NetSocket::randomPort()
{
	quint32 lowerPort = localPort() - 1;
	quint32 higherPort = localPort() + 1;

	if (localPort() == myPortMin)
		lowerPort = higherPort;
	else if (localPort() == myPortMax)
		higherPort = lowerPort;

	srand(time(NULL));
	return (rand() % 2) ? lowerPort: higherPort;
}

void NetSocket::broadcastOnRevolvingFrequencies(QByteArray array)
{
	for (int i = myPortMin; i <= myPortMax; ++i)
		if (!writeDatagram(array, QHostAddress(QHostAddress::LocalHost), i))
		{	// Error!
		}
}

void NetSocket::transmit(QByteArray array, QHostAddress address, quint16 port)
{
	if (!writeDatagram(array, address, port))
		{	// Error!
		}
}