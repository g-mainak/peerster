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
		QString getHostname();
		quint16 getPort();
		bool notDisabled();

	public slots:
		void lookedUp(QHostInfo);

	private:
		QString hostname;
		QHostAddress ip;
		quint16 port;
		bool disabled;
};

#endif 