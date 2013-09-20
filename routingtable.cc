#include "routingtable.hh"
#include "main.hh"

void RoutingTable::insert(QString origin, QHostAddress address, quint16 port)
{
	QPair<QHostAddress,quint16> pair;
	pair.first = address;
	pair.second = port;
	table.insert(origin, pair);
	emit inserted(this);
}

QStringList RoutingTable::displayAll()
{
	QStringList list;
	QHashIterator<QString, QPair<QHostAddress,quint16> > i(table);
	while (i.hasNext()) 
	{
	    i.next();
	    list << i.key() + " " +  i.value().first.toString() + ":" + QString::number(i.value().second);
	}
	return list;
}

QPair<QHostAddress, quint16> RoutingTable::findByOrigin(QString origin)
{
	return table.value(origin);
}