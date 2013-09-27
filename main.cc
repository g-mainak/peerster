#include "main.hh"

ChatDialog::ChatDialog()
{
	identifier = QHostInfo::localHostName() + QString::number(rand());
	qDebug() << identifier;
	setWindowTitle("Peerster" + identifier);
	seqNum = 1;

	forward = true;
	QStringList args = QCoreApplication::arguments();
	for(int i=1; i< args.size(); i++)
		if (args.at(i) == "-noforward")
		{
			forward = false;
			qDebug() << "NOFORWARD";
		}

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
	connect(hostinput, SIGNAL(returnPressed()),
                this, SLOT(gotNewPeer()));

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

  routeRumor = new QTimer(this);
  connect(routeRumor, SIGNAL(timeout()), this, SLOT(transmitRouteRumorMessage()));
  routeRumor->start(60000);
  emit transmitRouteRumorMessage();
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

	if (qvm.contains("Want") && qvm.size() == 1)
		receiveStatus(qvm, socket.findPeer(sender, senderPort));
	else if (qvm.contains("Origin") && qvm.contains("SeqNo"))
		receiveRumor(qvm, socket.findPeer(sender, senderPort));
	else if (qvm.contains("Dest"))
		receivePrivateMessage(qvm);
	else
		qDebug() << "Malformed message" << qvm<< " received. Ignoring it";
}

void ChatDialog::receiveStatus(QVariantMap foreignStatusMap, quint16 peer)
{
	qDebug() << "Received Status Message from " << peer;
	QVariantMap statusMap = createStatusMap();
	int x = compare(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap());
	switch (x)
	{
		case 1: qDebug() << "We're AHEAD";
			if (forward)
			{
				startRumorMongering(findAhead(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()), peer);
			}
			break;
		case -1: qDebug() << "We're BEHIND";
			transmitStatusMessage(peer);
			break;
		case 0: qDebug() << "We're SAME";
			if (rand() % 2)
			{
				qDebug() << "RANDOM EXIT";
				return;
			}
			else
			{
				qDebug() << "RANDOM RERUMOR";
				startRumorMongering();
			}
			break;
		default: qDebug() << "Malformed Status message. Ignoring.";
			break;
	}
}

void ChatDialog::receiveRumor(QVariantMap qvm, quint16 peer)
{
	Peer *p = socket.getPeer(peer);
	bool indirect = true;
	if ((indirect = qvm.contains("LastIP")))
		socket.findPeer(QHostAddress(qvm.value("LastIP").toUInt()), qvm.value("LastPort").toInt());
	qvm.insert("LastIP", p->getIp().toIPv4Address());
	qvm.insert("LastPort", p->getPort());
	qDebug() << "Received rumor message" << qvm << " from " << peer;
	if (!newMessage(qvm))
	{
		if (qvm.contains("ChatText"))
			textview->append(qvm.value("Origin").toString() + QString(": ") + qvm.value("ChatText").toString());
		rt.insert(qvm.value("Origin").toString(), p->getIp(), p->getPort(), indirect, qvm.value("SeqNo").toUInt());
		insertIntoPrevMessages(qvm.value("Origin").toString(), qvm.value("SeqNo").toUInt(), qvm);
		transmitStatusMessage(peer);
		if (!qvm.contains("ChatText")) // Route rumor
			transmitRouteRumorMessage(qvm);
		else if (forward)
			startRumorMongering();
	}
	else
		qDebug()<<"Same mesg";
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

void ChatDialog::transmitStatusMessage(quint16 peer)
{
	QByteArray array = serialize(createStatusMap());
	socket.transmit(array, peer);
	qDebug() << "TRANSMITTED status message "<< createStatusMap() <<"to " << peer; 
}

void ChatDialog::transmitOriginalMessage(QString message)
{
	textview->append(QString("Me: ") + message);
	QVariantMap qvm = createRumorMap(message);
	QByteArray array = serialize(qvm);
	qDebug() << "TRANSMITTING original message" << qvm;
	insertIntoPrevMessages(identifier, seqNum - 1, qvm);
	if (forward) startRumorMongering();
}

void ChatDialog::transmitRumorMessage(QVariantMap qvm, quint16 peer)
{
	QByteArray array = serialize(qvm);
	socket.transmit(array, peer);
	qDebug() << "TRANSMITTED rumor message " << qvm << "to " << peer; 
}

void ChatDialog::transmitRouteRumorMessage()
{
	transmitRouteRumorMessage(createRumorMap(""));
}

void ChatDialog::transmitRouteRumorMessage(QVariantMap qvm)
{
	qvm.remove("ChatText");
	insertIntoPrevMessages(qvm.value("Origin").toString(), qvm.value("SeqNo").toUInt(), qvm);
	QByteArray array = serialize(qvm);
	socket.transmitAll(array);
	qDebug() << "TRANSMITTED route rumor message to all peers"; 
}

void ChatDialog::transmitPrivateMessage(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	if (forward)
		socket.transmit(serialize(qvm), peerIndex);
	qDebug() << "TRANSMITTED PM " + QString::number(qvm.value("HopLimit").toInt()) + "to " + QString::number(peerIndex);
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
	qDebug() << "CSM";
	while (i.hasNext())
	{
		i.next();
		QList<quint32> list = prevMessageIds.values(i.key());
		qSort(list.begin(), list.end());
		quint32 j = 0;
		qDebug() << i.key() << list;
		while(list.at(j) == j + 1)
			j++;
		nestedQvm.insert(i.key(), j + 1);
	}
	qDebug() << "/CSM";
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

//Serialize into a QByteArray using a QDataStream object
QByteArray ChatDialog::serialize(QVariantMap qvm)
{
	QByteArray array;
	QDataStream out(&array, QIODevice::WriteOnly);
	out << qvm;
	return array;
}


void ChatDialog::ping()
{
	transmitStatusMessage(socket.randomPeer());
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
	return (prevMessages.contains(qvm.value("Origin").toString() + "-|42|-" + QString::number(qvm.value("SeqNo").toUInt())));
}

void ChatDialog::insertIntoPrevMessages(QString origin, quint32 sequence, QVariantMap message)
{
	if(!prevMessageIds.contains(origin, sequence))
		prevMessageIds.insert(origin, sequence);
	QString key = origin + QString("-|42|-") + QString::number(sequence);
	prevMessages.insert(key, message);
	QVariantMap qvm;
	qvm.insert("origin", origin);
	qvm.insert("message_sequence", sequence);
	lastMessage = qvm;
	// qDebug() << "MESSAGE TABLE";
	// QHashIterator<QString, QVariantMap > i(prevMessages);
	// while (i.hasNext()) {
 //    i.next();
 //    qDebug() << i.key() << ": " << i.value() << endl;
	// }
}

QVariantMap ChatDialog::getPrevMessage(QString origin, quint32 sequence)
{
	QString key = origin + QString("-|42|-") + QString::number(sequence);
	qDebug() << "KEY: " << key << prevMessages.contains(key);
	if (prevMessages.contains(key))
		return prevMessages.value(key);
	else
		return QVariantMap();
}


int ChatDialog::compare(QVariantMap current, QVariantMap foreign)
{
	QMapIterator<QString, QVariant> i(current);
	bool ahead =false, behind = false;
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
			qvm.insert(QString("message_sequence"), (y==0)?1:y); // you don't have y, 
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