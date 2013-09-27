#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QHash>
#include <QVector>
#include <QLineEdit>
#include <unistd.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QHostInfo>
#include <assert.h>
#include <QTimer>
#include "mtextedit.hh"
#include "netsocket.hh"
#include "peer.hh"
#include "routingtable.hh"
#include "point2pointview.hh"

class ChatDialog : public QDialog
{
	Q_OBJECT

	public:
		ChatDialog();
		QVariantMap createStatusMap();
		QVariantMap createRumorMap(QString);
		QVariantMap createRouteRumorMap();
		void receiveStatus(QVariantMap, quint16);
		void receiveRumor(QVariantMap, quint16);
		void receivePrivateMessage(QVariantMap);
		void transmitRumorMessage(QVariantMap, quint16);
		void transmitStatusMessage(quint16);
		void startRumorMongering(QVariantMap, quint16);
		void startRumorMongering();

	private:
		QByteArray serialize(QVariantMap);
		int compare(const QVariantMap, const QVariantMap);
		bool newMessage(QVariantMap);
		QVariantMap findAhead(QVariantMap, QVariantMap);
		void insertIntoPrevMessages(QString, quint32, QVariantMap);
		QVariantMap getPrevMessage(QString, quint32);

	public slots:
		void ping();
		void receiveMessage();
		void transmitOriginalMessage(QString);
		void transmitRouteRumorMessage();
		void transmitRouteRumorMessage(QVariantMap);
		void transmitPrivateMessage(QVariantMap);
		void gotNewPeer();
		void coinFlip();

	private:
		bool forward;
		QTextEdit *textview;
		MTextEdit *textinput;
		QLineEdit *hostinput;
		Point2PointView *tableview;
		NetSocket socket;
		quint32 seqNum;
		QMultiHash<QString, quint32> prevMessageIds;
		QHash<QString, QVariantMap> prevMessages;
		QString identifier;
		QVariantMap lastMessage;
		RoutingTable rt;
		QTimer *entropy;
		QTimer *routeRumor;
		QTimer *timeout;
};



#endif // PEERSTER_MAIN_HH
