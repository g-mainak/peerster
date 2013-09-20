#ifndef PEERSTER_P2PVIEW_HH
#define PEERSTER_P2PVIEW_HH

#include <QListView>
#include <QStringListModel>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QVariantMap>
#include "MPMEdit.hh"
#include "routingtable.hh"

class Point2PointView : public QListView
{
	Q_OBJECT

	public:
		Point2PointView( QWidget *parent=0);

	public slots:
		void refreshTable(RoutingTable *);
		void createPrivateMessage(QModelIndex);
		void createPrivateMessageMap(QString, QString);

	signals:
    	void privateMessageSignal(QVariantMap);

	private:
		QStringListModel model;

};

#endif