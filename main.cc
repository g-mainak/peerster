
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
	identifier = QHostInfo::localHostName() + QString(rand());
	qDebug() << identifier;
	setWindowTitle("Peerster" + identifier);
	seqNum = 1;

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	textinput = new MTextEdit(this);
	textinput->setFocus(); // Set focus here.
	connect(textinput, SIGNAL(messageSent(QString)), this, SLOT(transmitOriginalMessage(QString)));

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textinput);
	setLayout(layout);

	// Create a UDP network socket
	if (!socket.bind())
		exit(1);
	connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

	for (int i = 0; i < socket.peers.size(); ++i)
    	{
    		qDebug() << socket.peers.at(i)->getIp() << socket.peers.at(i)->getPort();
    	}

	QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ping()));
    timer->start(10000);
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


//Serialize into a QByteArray using a QDataStream object
QByteArray ChatDialog::serialize(QVariantMap qvm)
{
	QByteArray array;
	QDataStream out(&array, QIODevice::WriteOnly);
	out << qvm;
	return array;
}

void ChatDialog::transmitOriginalMessage(QString message)
{
	textview->append(QString("Me: ") + message);
	QVariantMap qvm = createRumorMap(message);
	QByteArray array = serialize(qvm);
	insertIntoPrevMessages(identifier, seqNum - 1, qvm);
	startRumorMongering();
}

void ChatDialog::transmitRumorMessage(QVariantMap qvm, quint16 peer)
{
	QByteArray array = serialize(qvm);
	socket.transmit(array, peer);
	qDebug() << "TRANSMITTED rumor message " << qvm.value("ChatText") << "to " << peer; 
}

void ChatDialog::ping()
{
	transmitStatusMessage(socket.randomPeer());
	for (int i = 0; i < socket.peers.size(); ++i)
    	{
    		qDebug() << socket.peers.at(i)->getIp() << socket.peers.at(i)->getPort();
    	}
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
	
	if (qvm.size() == 1)
		receiveStatus(qvm, sender, senderPort);
	else if (qvm.size() == 3)
		receiveRumor(qvm, sender, senderPort);
	else {}
		// Error!
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
	// qDebug() << current;
	// qDebug() << foreign;
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
	return QVariantMap(); //ERROR!
}

void ChatDialog::receiveStatus(QVariantMap foreignStatusMap, QHostAddress sender, quint16 senderPort)
{
	qDebug() << "Received Status Message from " << sender << " : " << senderPort;
	QVariantMap statusMap = createStatusMap();
	switch (compare(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()))
	{
		case 1: qDebug() << "We're AHEAD";
			startRumorMongering(findAhead(statusMap.value("Want").toMap(), foreignStatusMap.value("Want").toMap()), socket.findPeer(sender, senderPort));
			break;
		case -1: qDebug() << "We're BEHIND";
			transmitStatusMessage(socket.findPeer(sender, senderPort));
			break;
		case 0: qDebug() << "We're SAME";
			if (rand() % 2)
				return;
			else
				startRumorMongering();
			break;
		default: //Error!
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
	return (prevMessages.contains(qvm.value("Origin").toString() + "|" + QString(qvm.value("SeqNo").toUInt())));
}

void ChatDialog::insertIntoPrevMessages(QString origin, quint32 sequence, QVariantMap message)
{
	prevMessageIds.insert(origin, sequence);
	QString key = origin + QString("|") + QString(sequence);
	prevMessages.insert(key, message);
	QVariantMap qvm;
	qvm.insert("origin", origin);
	qvm.insert("message_sequence", sequence);
	lastMessage = qvm;
}

QVariantMap ChatDialog::getPrevMessage(QString origin, quint32 sequence)
{
	QString key = origin + QString("|") + QString(sequence);
	if (prevMessages.contains(key))
		return prevMessages.value(key);
	else
		return QVariantMap();
}

void ChatDialog::receiveRumor(QVariantMap qvm, QHostAddress sender, quint16 senderPort)
{
	qDebug() << "Received rumor message" << qvm.value("ChatText").toString() << " from " << sender << ":" << senderPort;
	if (!newMessage(qvm))
	{
		textview->append(qvm.value("Origin").toString() + QString(": ") + qvm.value("ChatText").toString());
		insertIntoPrevMessages(qvm.value("Origin").toString(), qvm.value("SeqNo").toUInt(), qvm);
		transmitStatusMessage(socket.findPeer(sender, senderPort));
		startRumorMongering();
	}
	else
		qDebug()<<"Same mesg";
	// TODO:
	//   Pick a random neighbour (+- 1)
	//   Send rumor to that neighbor
	//   Done.
	//   Technically, there will be a timeout step here
}

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	// NetSocket sock;
	// if (!sock.bind())
	// 	exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();

}