#include "routingtable.hh"
#include "main.hh"

void RoutingTable::insert(QString origin, QHostAddress address, quint16 port, bool indirect, quint32 seqNum)
{
	routingTableElement re;
	re.address = address;
	re.port = port;
	re.indirect = indirect;
	re.seqNum = seqNum;
	if (table.contains(origin))
	{
		// qDebug() << "Contains Origin";
		if (table.value(origin).seqNum < seqNum)
		{
			table.insert(origin, re);
			// qDebug() << "seqnum less";
		}
		else if (table.value(origin).seqNum == seqNum)
		{
			// qDebug() << "Same Sequence";
			if (table.value(origin).indirect && !indirect)
			{
				table.insert(origin, re);
				// qDebug() << "indirect";
			}
		}
	}
	else
	{
		table.insert(origin, re);
		// qDebug() << "Did not contain origin";
		routingTableElement rep = table.value(origin);
		// qDebug() << rep.address << rep.port;
	}
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

bool RoutingTable::alreadyInTable(QVariantMap qvm)
{
	QString origin = qvm.value("Origin").toString();
	if (table.contains(origin))
	{
		// qDebug() << "Origin" << table.value(origin).seqNum << qvm.value("SeqNo").toUInt();
		if (table.value(origin).seqNum == qvm.value("SeqNo").toUInt())
		{
			// qDebug() << "same Sequence";
			if (!table.value(origin).indirect)
			{
				// qDebug() << "indirect";
				return true;
			}
		}
	}
	return false;
}