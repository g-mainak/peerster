#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QHash>
#include <QVector>
#include <QLineEdit>
#include "mtextedit.hh"
#include "netsocket.hh"
#include "peer.hh"
#include "routingtable.hh"
#include "point2pointview.hh"
#include <QListView>
#include <QModelIndex>

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
		void transmitRouteRumorMessage(QVariantMap* = NULL);
		void gotNewPeer();
		void coinFlip();
		void transmitPrivateMessage(QVariantMap);

	private:
		QTimer *timeout;
		QTimer *entropy;
		QTimer *routeRumor;
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
};



#endif // PEERSTER_MAIN_HH
