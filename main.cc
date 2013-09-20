
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QHostInfo>
#include <assert.h>
#include <QTimer>
#include "main.hh"

ChatDialog::ChatDialog()
{
	identifier = QHostInfo::localHostName() + QString::number(rand());
	qDebug() << identifier;
	setWindowTitle("Peerster" + identifier);
	seqNum = 1;
	srand(time(NULL));

	forward = true;
	QStringList args = QCoreApplication::arguments();
	for(int i=1; i< args.size(); i++)
		if (args.at(i) == "-noforward")
			forward = false;

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	textinput = new MTextEdit(this);
	textinput->setFocus(); // Set focus here.
	connect(textinput, SIGNAL(messageSent(QString)), this, SLOT(transmitOriginalMessage(QString)));

	tableview = new Point2PointView(this);
	tableview->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(tableview, SIGNAL(privateMessageSignal(QVariantMap)), this, SLOT(transmitPrivateMessage(QVariantMap)));
	connect(&rt, SIGNAL(inserted(RoutingTable*)), tableview, SLOT(refreshTable(RoutingTable*)));

	hostinput = new QLineEdit(this);
	connect(hostinput, SIGNAL(returnPressed()), this, SLOT(gotNewPeer()));


	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->addWidget(textview);
	vLayout->addWidget(textinput);
	vLayout->addWidget(hostinput);

	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addLayout(vLayout);
	hLayout->addWidget(tableview);
	setLayout(hLayout);

	// Create a UDP network socket
	if (!socket.bind())
		exit(1);
	connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

	entropy = new QTimer(this);
    connect(entropy, SIGNAL(timeout()), this, SLOT(ping()));
    entropy->start(10000);

    timeout = new QTimer(this);
    timeout->setSingleShot(true);

    routeRumor = new QTimer(this);
    connect(routeRumor, SIGNAL(timeout()), this, SLOT(transmitRouteRumorMessage()));
    routeRumor->start(60000);
    emit transmitRouteRumorMessage();
}

void ChatDialog::gotNewPeer()
{
	socket.addPeer(hostinput->text());
	hostinput->clear();
}

QVariantMap ChatDialog::createStatusMap()
{
	QVariantMap qvm, nestedQvm;
	QHashIterator<QString, quint32> i(prevMessageIds);
	while (i.hasNext())
	{
		i.next();
		QList<quint32> list = prevMessageIds.values(i.key());
		qSort(list.begin(), list.end());
		quint32 j = 0;
		while(list.at(j) == j + 1)
			j++;
		nestedQvm.insert(i.key(), j + 1);
	}
	qvm["Want"] = nestedQvm;
	// qDebug() << "Nested QVM:" << qvm;
	return qvm;
}

QVariantMap ChatDialog::createRumorMap(QString message)
{
	QVariantMap qvm;
	qvm["ChatText"]= message;
	qvm["Origin"] = identifier;
	qvm["SeqNo"] = seqNum++;
	return qvm;
}

QVariantMap ChatDialog::createRouteRumorMap()
{
	QVariantMap qvm;
	qvm["Origin"] = identifier;
	qvm["SeqNo"] = seqNum;
	return qvm;
}


//Serialize into a QByteArray using a QDataStream object
QByteArray ChatDialog::serialize(QVariantMap qvm)
{
	QByteArray array;
	QDataStream out(&array, QIODevice::WriteOnly);
	out << qvm;
	return array;
}

void ChatDialog::transmitPrivateMessage(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	socket.transmit(serialize(qvm), peerIndex);
	// qDebug() << "Transmitted PM " + QString::number(qvm.value("HopLimit").toInt()) + "to " + QString::number(peerIndex);
}

void ChatDialog::transmitOriginalMessage(QString message)
{
	textview->append(QString("Me: ") + message);
	QVariantMap qvm = createRumorMap(message);
	QByteArray array = serialize(qvm);
	insertIntoPrevMessages(identifier, seqNum - 1, qvm);
	if (forward) startRumorMongering();
}

void ChatDialog::transmitRumorMessage(QVariantMap qvm, quint16 peer)
{
	QByteArray array = serialize(qvm);
	socket.transmit(array, peer);
	qDebug() << "TRANSMITTED rumor message " << qvm.value("ChatText") << "to " << peer; 
    connect(timeout, SIGNAL(timeout()), this, SLOT(coinFlip()));
    timeout->start(1000);
}

void ChatDialog::transmitRouteRumorMessage(QVariantMap *qvm)
{
	QByteArray array = serialize(qvm ? *qvm : createRouteRumorMap());
	socket.transmitAll(array);
	qDebug() << "TRANSMITTED route rumor message to all peers"; 
}

void ChatDialog::coinFlip()
{
	if (rand() % 2){}
		//startRumorMongering();
}

void ChatDialog::ping()
{
	transmitStatusMessage(socket.randomPeer());
	// for (int i = 0; i < socket.getNumPeers(); ++i)
 //    	{
 //    		qDebug() << "Peer " << socket.getPeer(i)->getIp() << socket.getPeer(i)->getPort();
 //    	}
}
void ChatDialog::transmitStatusMessage(quint16 peer)
{
	QByteArray array = serialize(createStatusMap());
	socket.transmit(array, peer);
	qDebug() << "TRANSMITTED status message to " << peer; 
}

void ChatDialog::receiveMessage()
{
	QByteArray datagram;
    datagram.resize(socket.pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    socket.readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);
	QVariantMap qvm;
	QDataStream in(&datagram, QIODevice::ReadOnly);
	in >> qvm;
	quint16 peerIndex = socket.findPeer(sender, senderPort);
	bool containsLastIp;
	if (containsLastIp = qvm.contains("LastIP"))
		socket.findPeer(QHostAddress(qvm.value("LastIP").toUInt()), qvm.value("LastPort").toInt());

	if (qvm.size() == 1)
		receiveStatus(qvm, peerIndex);
	else if (qvm.size() >= 2 || qvm.size() <= 5)
	{
		qvm.insert("LastIP", sender.toIPv4Address());
		qvm.insert("LastPort", senderPort);
		if (qvm.contains("Dest"))
			receivePrivateMessage(qvm);
		else if (qvm.contains("Origin"))
			receiveRumor(qvm, peerIndex, containsLastIp);
		else
			qDebug() << "Malformed message received. Ignoring it.";
	}
	else
	{
		qDebug() << qvm.size();
		qDebug() << "Malformed message received. Ignoring it";
	}
}



void ChatDialog::receiveStatus(QVariantMap foreignStatusMap, quint16 peer)
{
	qDebug() << "Received Status Message from " << peer;
	disconnect(timeout, 0, 0, 0);
	QVariantMap statusMap = createStatusMap();
	switch (compare(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()))
	{
		case 1: qDebug() << "We're AHEAD";
			if (forward)
				startRumorMongering(findAhead(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()), peer);
			break;
		case -1: qDebug() << "We're BEHIND";
			transmitStatusMessage(peer);
			break;
		case 0: qDebug() << "We're SAME";
			if (rand() % 2)
				return;
			else
				startRumorMongering();
			break;
		default: qDebug() << "Malformed Status message. Ignoring.";
			break;
	}
}

void ChatDialog::startRumorMongering()
{
	startRumorMongering(lastMessage, socket.randomPeer());
}

void ChatDialog::startRumorMongering(QVariantMap qvm, quint16 peer)
{
	if (!qvm.empty())
	{
		QString origin = qvm.value("origin").toString();
		quint32 sequence = qvm.value("message_sequence").toUInt();
		QVariantMap message = getPrevMessage(origin, sequence);
		transmitRumorMessage(message, peer);
	}
}

bool ChatDialog::newMessage(QVariantMap qvm)
{
	return (prevMessages.contains(qvm.value("Origin").toString() + "-|42|-" + QString(qvm.value("SeqNo").toUInt())));
}

void ChatDialog::insertIntoPrevMessages(QString origin, quint32 sequence, QVariantMap message)
{
	prevMessageIds.insert(origin, sequence);
	QString key = origin + QString("-|42|-") + QString(sequence);
	prevMessages.insert(key, message);
	QVariantMap qvm;
	qvm.insert("origin", origin);
	qvm.insert("message_sequence", sequence);
	lastMessage = qvm;
}

QVariantMap ChatDialog::getPrevMessage(QString origin, quint32 sequence)
{
	QString key = origin + QString("-|42|-") + QString(sequence);
	if (prevMessages.contains(key))
		return prevMessages.value(key);
	else
		return QVariantMap();
}

void ChatDialog::receiveRumor(QVariantMap qvm, quint16 peer, bool indirect)
{
	qDebug() << "Received rumor message" << qvm.value("ChatText").toString() << " from " << peer;
	if (qvm.size() == 5) //Chat rumour
	{
		if (!newMessage(qvm))
		{
			Peer *p = socket.getPeer(peer);
			if (qvm.value("Origin").toString() != identifier)
				rt.insert(qvm.value("Origin").toString(), p->getIp(), p->getPort(), indirect, qvm.value("SeqNum").toUInt());
			textview->append(qvm.value("Origin").toString() + QString(": ") + qvm.value("ChatText").toString());
			insertIntoPrevMessages(qvm.value("Origin").toString(), qvm.value("SeqNo").toUInt(), qvm);
			transmitStatusMessage(peer);
			if (forward) startRumorMongering();
		}
		else
			qDebug()<<"Same mesg";
	}
	else if (qvm.size() == 4) // Route rumour
	{
		Peer *p = socket.getPeer(peer);
		if (qvm.value("Origin").toString() != identifier)
			rt.insert(qvm.value("Origin").toString(), p->getIp(), p->getPort(), indirect, qvm.value("SeqNum").toUInt());
		transmitStatusMessage(peer);
		transmitRouteRumorMessage(&qvm);
	}
}

void ChatDialog::receivePrivateMessage(QVariantMap qvm)
{
	if (qvm.value("Dest") == identifier)
		textview->append(QString("PM: ") + qvm.value("ChatText").toString());
	else if (forward)
	{
		qvm["HopLimit"] = qvm["HopLimit"].toInt() - 1;
		transmitPrivateMessage(qvm);
	}

}

int ChatDialog::compare(QVariantMap current, QVariantMap foreign)
{
	QMapIterator<QString, QVariant> i(current);
	bool ahead = false, behind = false;
	while (i.hasNext())
	{
		i.next();
		quint32 x = i.value().toInt();
		quint32 y = foreign.value(i.key(), 0).toInt();
		if (x > y)
			ahead = true;
		else if (x < y)
			behind = true;
	}
	QMapIterator<QString, QVariant> j(foreign);
	while (j.hasNext())
	{
		j.next();
		quint32 x = j.value().toInt();
		quint32 y = current.value(j.key(), 0).toInt();
		if (x > y)
			behind = true;
		else if (x < y)
			ahead = true;
	}
	return (ahead) ? 1 : ((behind) ? -1 : 0);
}

QVariantMap ChatDialog::findAhead(QVariantMap current, QVariantMap foreign)
{
	QMapIterator<QString, QVariant> i(current);
	while (i.hasNext())
	{
		i.next();
		quint32 x = i.value().toInt();
		quint32 y = foreign.value(i.key(), 0).toInt();
		qDebug() << i.key() << "<Current, Foreign>" << x << y;
		if (x > y)
		{
			QVariantMap qvm;
			qvm.insert(QString("origin"), i.key());
			qvm.insert(QString("message_sequence"), (y==0)?1:y);
			return qvm;
		}
	}
	qDebug() << "Message does not exist! Canceling.";
	return QVariantMap();
}
int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	return app.exec();

}