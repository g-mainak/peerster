#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <unistd.h>
#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
	Q_OBJECT

	public:
		NetSocket();

		// Bind this socket to a Peerster-specific default port.
		bool bind();
		void broadcastOnRevolvingFrequencies(QByteArray);
		void transmit(QByteArray, QHostAddress, quint16);
		quint16 randomPort();

	private:
		int myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH
