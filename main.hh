#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QUdpSocket>
#include <QKeyEvent>

class MTextEdit : public QTextEdit
{
	Q_OBJECT

    public:
    	MTextEdit(QWidget *parent);
    	void keyPressEvent(QKeyEvent *);

    signals:
    	void messageSent(QString message);
};

class NetSocket : public QUdpSocket
{
	Q_OBJECT

	public:
		NetSocket();

		// Bind this socket to a Peerster-specific default port.
		bool bind();
		void broadcastOnRevolvingFrequencies(QByteArray);

	private:
		int myPortMin, myPortMax;
};

class ChatDialog : public QDialog
{
	Q_OBJECT

	public:
		ChatDialog();

	private:
		QVariantMap createMap(QString);
		QByteArray serialize(QVariantMap);


	public slots:
		void transmitMessage(QString message);
		void receiveMessage();

	private:
		QTextEdit *textview;
		MTextEdit *textinput;
		NetSocket socket;
};



#endif // PEERSTER_MAIN_HH
