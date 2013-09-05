#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QHash>
#include "mtextedit.hh"
#include "netsocket.hh"

class ChatDialog : public QDialog
{
	Q_OBJECT

	public:
		ChatDialog();
		QVariantMap createStatusMap();
		QVariantMap createRumorMap(QString);
		void receiveStatus(QVariantMap, QHostAddress, quint16);
		void receiveRumor(QVariantMap, QHostAddress, quint16);
		void transmitRumorMessage(QVariantMap, QHostAddress, quint16);
		void transmitStatusMessage(QHostAddress, quint16);
		void startRumorMongering(QVariantMap, QHostAddress, quint16);
		void startRumorMongering();

	private:
		QByteArray serialize(QVariantMap);
		int compare(const QVariantMap, const QVariantMap);
		bool newMessage(QVariantMap);
		QVariantMap findAhead(QVariantMap, QVariantMap);
		void insertIntoPrevMessages(QString, quint32, QVariantMap);
		QVariantMap getPrevMessage(QString, quint32);

	public slots:
		void receiveMessage();
		void transmitOriginalMessage(QString);

	private:
		QTextEdit *textview;
		MTextEdit *textinput;
		NetSocket socket;
		quint32 seqNum;
		QMultiHash<QString, quint32> prevMessageIds;
		QHash<QString, QVariantMap> prevMessages;
		QString identifier;
		QVariantMap lastMessage;
};



#endif // PEERSTER_MAIN_HH
