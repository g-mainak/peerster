#ifndef PEERSTER_PEER_HH
#define PEERSTER_PEER_HH

#include <QString>
#include <QHostAddress>
#include <QRegExp>
#include <QStringList>
#include <QHostInfo>
#include <assert.h>

class Peer : public QObject
{
	Q_OBJECT

	public:
		Peer();
		// Peer(const Peer&);
		Peer(QString);
		Peer(QHostAddress, quint16);
		QHostAddress getIp();
		quint16 getPort();

	public slots:
		void lookedUp(QHostInfo);

	private:
		QString hostname;
		QHostAddress ip;
		quint16 port;
		bool disabled;
};

#endif 