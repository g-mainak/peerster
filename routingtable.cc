#include "routingtable.hh"
#include "main.hh"

void RoutingTable::insert(QString origin, QHostAddress address, quint16 port, bool indirect, quint32 seqNum)
{
	routingTableElement re;
	re.address = address;
	re.port = port;
	re.indirect = indirect;
	re.seqNum = seqNum;
	if (table.contains("Origin"))
	{
		if (table.value("Origin").seqNum < seqNum)
			table.insert(origin, re);
		else if (table.value("Origin").seqNum == seqNum)
			if (table.value("Origin").indirect && !indirect)
				table.insert(origin, re);
	}
	else
		table.insert(origin, re);
	emit inserted(this);
}

QStringList RoutingTable::displayAll()
{
	QStringList list;
	QHashIterator<QString, routingTableElement > i(table);
	while (i.hasNext()) 
	{
	    i.next();
	    routingTableElement re  = i.value();
	    list << i.key() + " " +  re.address.toString() + ":" + QString::number(re.port);
	}
	return list;
}

QPair<QHostAddress, quint16> RoutingTable::findByOrigin(QString origin)
{
	routingTableElement re = table.value(origin);
	QPair<QHostAddress, quint16> qp;
	qp.first = re.address;
	qp.second = re.port;
	return qp;
}