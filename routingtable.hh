#ifndef PEERSTER_ROUTING_TABLE_HH
#define PEERSTER_ROUTING_TABLE_HH

#include <QString>
#include <QHostAddress>
#include <QStringList>
#include <QDialog>

class RoutingTable : public QObject
{
	Q_OBJECT

	struct routingTableElement
	{
		QHostAddress address; 
		quint16 port;
		bool indirect;
		quint32 seqNum;
	};

	public:
		void insert(QString, QHostAddress, quint16, bool, quint32);
		QStringList displayAll();
		QPair<QHostAddress, quint16> findByOrigin(QString origin);
		bool alreadyInTable(QVariantMap);

	private:
		QHash<QString, routingTableElement > table;

	signals:
		void inserted(RoutingTable *);
};

#endif 