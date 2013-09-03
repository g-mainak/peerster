
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <assert.h>

#include "main.hh"

MTextEdit::MTextEdit(QWidget *parent) : QTextEdit(parent) {}

void MTextEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		emit messageSent(this->toPlainText());
		// Clear the textinput to get ready for the next input message.
		this->clear();		
	}
	else
		QTextEdit::keyPressEvent(event);
}

ChatDialog::ChatDialog()
{
	setWindowTitle("Peerster");


	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	textinput = new MTextEdit(this);
	textinput->setFocus(); // Set focus here.
	connect(textinput, SIGNAL(messageSent(QString)), this, SLOT(transmitMessage(QString)));

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
}

QVariantMap ChatDialog::createMap(QString message)
{
	QVariantMap qvm;
	qvm["ChatText"]= message;
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

void ChatDialog::transmitMessage(QString message)
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	qDebug() << socket.hasPendingDatagrams() << "pedning";
	qDebug() << "FIX: send message to other peers: " << message;
	textview->append(message);
	QByteArray array = serialize(createMap(message));
	qDebug() << "ARRAY:" << array;
	socket.broadcastOnRevolvingFrequencies(array);
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
	assert(qvm.size() == 1);
	assert(qvm.value("ChatText").type() == QVariant::String);
	textview->append(qvm.value("ChatText").toString());

}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four Peerster instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

void NetSocket::broadcastOnRevolvingFrequencies(QByteArray array)
{
	for (int i = myPortMin; i <= myPortMax; ++i)
	{
		int bytes = writeDatagram(array, QHostAddress(QHostAddress::LocalHost), i);
		qDebug() << "Wrote " << bytes << " bytes to port " << i;
	}
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