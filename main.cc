#include "main.hh"

ChatDialog::ChatDialog()
{
	identifier =  "m" + QString::number(rand()%100); //QHostInfo::localHostName()
	// qDebug() << identifier;
	setWindowTitle("Peerster" + identifier);
	seqNum = 1;
	srand(time(NULL));
	QCA::Initializer qcainit;

	forward = true;
	QStringList args = QCoreApplication::arguments();
	for(int i=1; i< args.size(); i++)
		if (args.at(i) == "-noforward")
		{
			forward = false;
			// qDebug() << "NOFORWARD";
		}


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
	hostinput->setPlaceholderText("Enter new host name or address...");
	connect(hostinput, SIGNAL(returnPressed()), this, SLOT(createNewPeer()));

	QPushButton *share = new QPushButton("Share File...", this);
	share->setAutoDefault(false);
	connect(share, SIGNAL(clicked()), this, SLOT(createFile()));

	QPushButton *download = new QPushButton("Download", this);
	download->setAutoDefault(false);
	connect(download, SIGNAL(clicked()), this, SLOT(initiateBlockRequest()));

	searchinput = new QLineEdit(this);
	searchinput->setPlaceholderText("Enter search terms...");
	connect(searchinput, SIGNAL(returnPressed()), this, SLOT(createSearch()));

	QVBoxLayout *vLayout1 = new QVBoxLayout();
	vLayout1->addWidget(new QLabel("Message history:"));
	vLayout1->addWidget(textview);
	vLayout1->addWidget(new QLabel("Enter chats:"));
	vLayout1->addWidget(textinput);
	vLayout1->addWidget(hostinput);

	QVBoxLayout *vLayout2 = new QVBoxLayout();
	vLayout2->addWidget(new QLabel("Known peers:"));
	vLayout2->addWidget(tableview);
	vLayout2->addWidget(searchinput);
	vLayout2->addWidget(share);
	vLayout2->addWidget(download);

	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addLayout(vLayout1);
	hLayout->addLayout(vLayout2);
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

	timeout = new QTimer(this);
	timeout->setSingleShot(true);
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
	else if (qvm.contains("Origin") && qvm.contains("Search"))
		receiveSearchRequest(qvm);
	else
		qDebug() << "Malformed message" << qvm<< " received. Ignoring it";
}

void ChatDialog::receiveStatus(QVariantMap foreignStatusMap, quint16 peer)
{
	// qDebug() << "Received Status Message from " << peer;
	disconnect(timeout, 0, 0, 0);
	QVariantMap statusMap = createStatusMap();
	int x = compare(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap());
	switch (x)
	{
		case 1: // qDebug() << "We're AHEAD";
		if (forward)
		{
			startRumorMongering(findAhead(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()), peer);
		}
		break;
		case -1: // qDebug() << "We're BEHIND";
		transmitStatusMessage(peer);
		break;
		case 0: // qDebug() << "We're SAME";
		if (rand() % 2)
		{
			// qDebug() << "RANDOM EXIT";
			return;
		}
		else
		{
			// qDebug() << "RANDOM RERUMOR";
			startRumorMongering();
		}
		break;
		default: // qDebug() << "Malformed Status message. Ignoring.";
		break;
	}
}

void ChatDialog::receiveRumor(QVariantMap qvm, quint16 peer)
{
	Peer *p = socket.getPeer(peer);
	bool indirect = true;
	if ((indirect = qvm.contains("LastIP")))
		socket.findPeer(QHostAddress(qvm.value("LastIP").toUInt()), qvm.value("LastPort").toInt());
	if (!newMessage(qvm))
	{
		qvm.insert("LastIP", p->getIp().toIPv4Address());
		qvm.insert("LastPort", p->getPort());
	// qDebug() << "Received rumor message" << qvm << " from " << peer;
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
	// else
	// 	qDebug()<<"Same mesg";
}

void ChatDialog::receivePrivateMessage(QVariantMap qvm)
{
	if (qvm.value("Dest") == identifier)
	{
		if (qvm.contains("BlockRequest"))
			receiveBlockRequest(qvm);
		else if (qvm.contains("BlockReply"))
			receiveBlockReply(qvm);
		else if (qvm.contains("SearchReply"))
			receiveSearchReply(qvm);
		else
			textview->append(QString("PM: ") + qvm.value("ChatText").toString());
	}
	else if (forward)
	{
		qvm["HopLimit"] = qvm["HopLimit"].toInt() - 1;
		if (qvm["HopLimit"].toInt() > 0)
			transmitPrivateMessage(qvm);
	}
}

void ChatDialog::receiveBlockRequest(QVariantMap qvm)
{
	// qDebug() << "RECEIVED BR" << qvm;
	int position = 0;
	if ((position = metadata.contains(qvm.value("BlockRequest").toByteArray())) != -1)
	{
		qDebug() << "Found";
		int blockNumber = metadata.getBlockNumber(qvm.value("BlockRequest").toByteArray(), position);
		QByteArray data, blockReply;
		if (blockNumber == -1)
		{
			qDebug() << "Full File";
			data = metadata.getBlocklist(position);
			blockReply = metadata.hash(data);
		}
		else
		{	
			qDebug() << "Block" << blockNumber;
			data = metadata.getBlock(position, blockNumber);
			blockReply = metadata.hash(data);
		}
		QVariantMap qvma = createBlockReply(qvm.value("Origin").toString(), data, blockReply);
		transmitBlockReply(qvma);
	}
}

void ChatDialog::receiveBlockReply(QVariantMap qvm)
{
	// qDebug() << "BLOCKREPLY!" << qvm;
	if (FileMetaData::hash(qvm.value("Data").toByteArray()) == qvm.value("BlockReply").toByteArray())
	{
		int position = downloadList.find(qvm.value("Origin").toString(), qvm.value("BlockReply").toByteArray());
		qDebug() << "Position" << position;
		if (position != -1)
		{
			int next = downloadList.insert(position, qvm);
			if (next != -1)
			{
				QVariantMap nextBlockRequest;
				nextBlockRequest["BlockRequest"] = downloadList.getBlockRequest(position, next);
				nextBlockRequest["Dest"] = qvm.value("Origin").toString();
				nextBlockRequest["Origin"] = identifier;
				nextBlockRequest["HopLimit"] = (quint32)10;
				qDebug() << "transmitting";
				transmitBlockRequest(nextBlockRequest);
			}
		}
	}
	else
		qDebug() << "File corrupted!";
}

void ChatDialog::receiveSearchRequest(QVariantMap qvm)
{
	if (qvm.value("Origin").toString() != identifier)
	{
		QVariantMap searchReply = metadata.search(qvm.value("Search").toString());
		if (!searchReply.isEmpty())
		{
			if (qvm.contains("Origin")){
				searchReply["Dest"] = qvm.value("Origin").toString();
				searchReply["Origin"] = identifier;
				searchReply["HopLimit"] = (quint32)10;
				searchReply["SearchReply"] = qvm.value("Search").toString();
				transmitSearchReply(searchReply);
			}
			else // Local
				receiveSearchReply(searchReply);
		}
		if (!qvm.contains("Origin"))
		{
			qvm["Origin"] = identifier;
		}
		qvm["Budget"] = qvm.value("Budget").toUInt() - 1;
		if (qvm.value("Budget").toUInt())
			transmitSearchRequest(qvm);
	}
}

void ChatDialog::receiveSearchReply(QVariantMap qvm)
{
		qDebug() << "RECEIVED Search Reply" << qvm;
		search->refreshList(qvm.value("MatchNames").toList(), qvm.value("MatchIDs").toList(), qvm.value("Origin").toString());
}

void ChatDialog::transmitStatusMessage(quint16 peer)
{
	QByteArray array = serialize(createStatusMap());
	socket.transmit(array, peer);
	// qDebug() << "TRANSMITTED status message "<< createStatusMap() <<"to " << peer; 
}

void ChatDialog::transmitOriginalMessage(QString message)
{
	textview->append(QString("Me: ") + message);
	QVariantMap qvm = createRumorMap(message);
	QByteArray array = serialize(qvm);
	// qDebug() << "TRANSMITTING original message" << qvm;
	insertIntoPrevMessages(identifier, seqNum - 1, qvm);
	if (forward) startRumorMongering();
}

void ChatDialog::transmitRumorMessage(QVariantMap qvm, quint16 peer)
{
	QByteArray array = serialize(qvm);
	socket.transmit(array, peer);
	// qDebug() << "TRANSMITTED rumor message " << qvm << "to " << peer; 
	connect(timeout, SIGNAL(timeout()), this, SLOT(coinFlip()));
	timeout->start(1000);
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
	// qDebug() << "TRANSMITTED route rumor message to all peers"; 
}

void ChatDialog::transmitPrivateMessage(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	if (forward)
		socket.transmit(serialize(qvm), peerIndex);
	// qDebug() << "TRANSMITTED PM " + QString::number(qvm.value("HopLimit").toInt()) + "to " + QString::number(peerIndex);
}

void ChatDialog::transmitBlockRequest(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	if (forward)
		socket.transmit(serialize(qvm), peerIndex);
	qDebug() << "TRANSMITTED Block Request " << qvm;
}

void ChatDialog::transmitBlockReply(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	qDebug() << "QP" << qp;
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	if (forward)
		socket.transmit(serialize(qvm), peerIndex);
	qDebug() << "TRANSMITTED Block Reply " << qvm.value("BlockReply");
}

void ChatDialog::transmitSearchRequest(QVariantMap qvm)
{
	int numPeers = socket.getNumPeers();
	quint32 minBudget = qvm.value("Budget").toUInt()/numPeers;
	quint32 numExtra = qvm.value("Budget").toUInt()%numPeers;
	qvm["Budget"] = minBudget + 1;
	QByteArray array = serialize(qvm);
	socket.transmitEvenly(array, numExtra, true);
	if (minBudget)
	{
		qvm["Budget"] = minBudget;
		array = serialize(qvm);
		socket.transmitEvenly(array, numExtra, false);
	}
	qDebug() << "TRANSMITTED Search request with budget" << qvm.value("Budget").toUInt();
}

void ChatDialog::transmitSearchReply(QVariantMap qvm)
{
	QPair<QHostAddress, quint16> qp = rt.findByOrigin(qvm.value("Dest").toString());
	qDebug() << "QP" << qp;
	quint16 peerIndex = socket.findPeer(qp.first, qp.second);
	if (forward)
		socket.transmit(serialize(qvm), peerIndex);
	qDebug() << "TRANSMITTED Search Reply " << qvm.value("SearchReply");
}

void ChatDialog::createNewPeer()
{
	socket.addPeer(hostinput->text());
	hostinput->clear();
}

QVariantMap ChatDialog::createStatusMap()
{
	QVariantMap qvm, nestedQvm;
	QList<QString> keys = prevMessageIds.uniqueKeys();
	// qDebug() << "CSM";
	foreach( const QString& k, keys ) {
		QList<quint32> list = prevMessageIds.values(k);
		qSort(list.begin(), list.end());
		// qDebug() << k << list;
		quint32 j = 0;
		while(list.at(j) == j + 1)
			j++;
		nestedQvm.insert(k, j + 1);
	}
	// qDebug() << "/CSM";
	qvm["Want"] = nestedQvm;
	// // qDebug() << "Nested QVM:" << qvm;
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

void ChatDialog::createFile()
{
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	QStringList fileNames;
	if (fileDialog->exec())
		fileNames = fileDialog->selectedFiles();
	metadata.newFiles(fileNames);
}

void ChatDialog::createBlockRequest(QString destination, QString blockRequest)
{
	QVariantMap qvm;
	qvm["Dest"] = destination;
	qvm["Origin"] = identifier;
	qvm["HopLimit"] = (quint32)10;
	qvm["BlockRequest"] = QByteArray().append(blockRequest);
	downloadList.newDownload(destination, QByteArray().append(blockRequest));
	transmitBlockRequest(qvm);
}

QVariantMap ChatDialog::createBlockReply(QString destination, QByteArray data, QByteArray blockReply)
{
	QVariantMap qvm;
	qvm["Dest"] = destination;
	qvm["Origin"] = identifier;
	qvm["HopLimit"] = (quint32)10;
	qvm["BlockReply"] = blockReply;
	qvm["Data"] = data;
	return qvm;
}

void ChatDialog::createSearch()
{
	search = new FileSearch(searchinput->text(), identifier, &metadata, this);
	connect(search, SIGNAL(searchRequest(QVariantMap)), this, SLOT(receiveSearchRequest(QVariantMap)));
	connect(search, SIGNAL(searchParams(QString, QString)), this, SLOT(createBlockRequest(QString, QString)));
	search->startTimer();
	search->exec();
	searchinput->clear();
}

void ChatDialog::initiateBlockRequest()
{
	FileTransferDialog *ftd = new FileTransferDialog(this);
	connect(ftd, SIGNAL(searchParams(QString, QString)), this, SLOT(createBlockRequest(QString, QString)));
	ftd->exec();
}

//Serialize into a QByteArray using a QDataStream object
QByteArray ChatDialog::serialize(QVariantMap qvm)
{
	QByteArray array;
	QDataStream out(&array, QIODevice::WriteOnly);
	out << qvm;
	return array;
}

void ChatDialog::coinFlip()
{
	if (rand() % 2)
		startRumorMongering();
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
	// // qDebug() << "MESSAGE TABLE";
	// QHashIterator<QString, QVariantMap > i(prevMessages);
	// while (i.hasNext()) {
 //    i.next();
 //    // qDebug() << i.key() << ": " << i.value() << endl;
	// }
}

QVariantMap ChatDialog::getPrevMessage(QString origin, quint32 sequence)
{
	QString key = origin + QString("-|42|-") + QString::number(sequence);
	// qDebug() << "KEY: " << key << prevMessages.contains(key);
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
		// qDebug() << i.key() << "<Current, Foreign>" << x << y;
		if (x > y)
		{
			QVariantMap qvm;
			qvm.insert(QString("origin"), i.key());
			qvm.insert(QString("message_sequence"), (y==0)?1:y); // you don't have y, 
			return qvm;
		}
	}
	// qDebug() << "Message does not exist! Canceling.";
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