#ifndef PEERSTER_NETSOCKET_HH
#define PEERSTER_NETSOCKET_HH

#include <unistd.h>
#include <QUdpSocket>
#include <QCoreApplication>
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
		void addPeer(QString);
		Peer* getPeer(quint16);
		quint16 getNumPeers();
		void transmitAll(QByteArray);

	private:
		int myPortMin, myPortMax;
		QVector<Peer*> peers;
};

#endif // PEERSTER_NETSOCKET_HH
