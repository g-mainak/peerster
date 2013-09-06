#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <unistd.h>
#include <QUdpSocket>
#include "peer.hh"

class NetSocket : public QUdpSocket
{
	Q_OBJECT

	public:
		NetSocket();

		// Bind this socket to a Peerster-specific default port.
		bool bind();
		void broadcastOnRevolvingFrequencies(QByteArray);
		void transmit(QByteArray, quint16);
		quint16 randomPeer();
		quint16 findPeer(QHostAddress, quint16);

		QVector<Peer*> peers;
	private:
		int myPortMin, myPortMax;
};

#endif // PEERSTER_NETSOCKET_HH
