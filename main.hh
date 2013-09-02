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

class ChatDialog : public QDialog
{
	Q_OBJECT

	public:
		ChatDialog();

	public slots:
		void transmitMessage(QString message);

	private:
		QTextEdit *textview;
		MTextEdit *textinput;
};

class NetSocket : public QUdpSocket
{
	Q_OBJECT

	public:
		NetSocket();

		// Bind this socket to a Peerster-specific default port.
		bool bind();

	private:
		int myPortMin, myPortMax;
};


#endif // PEERSTER_MAIN_HH
