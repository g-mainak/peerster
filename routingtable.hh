#ifndef PEERSTER_ROUTING_TABLE_HH
#define PEERSTER_ROUTING_TABLE_HH

#include <QString>
#include <QHostAddress>
#include <QStringList>
#include <QDialog>

class RoutingTable : public QObject
{
	Q_OBJECT

	public:
		void insert(QString, QHostAddress, quint16);
		QStringList displayAll();
		QPair<QHostAddress, quint16> findByOrigin(QString origin);

	private:
		QHash<QString, QPair<QHostAddress,quint16> > table;

	signals:
		void inserted(RoutingTable *);
};

#endif 