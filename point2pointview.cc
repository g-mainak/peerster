#include "point2pointview.hh"

Point2PointView::Point2PointView( QWidget *parent) : QListView( parent )
{
	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(createPrivateMessage(QModelIndex)));
}

void Point2PointView::refreshTable(RoutingTable *rt)
{
    QStringList list = rt->displayAll();
    model.setStringList(list);
    this->setModel(&model);
}

void Point2PointView::createPrivateMessage(QModelIndex index)
{
	QDialog dialog;
	QVariant destination = model.data(index, 0);
	MPMEdit *textinput = new MPMEdit(&dialog, destination.toString());
	textinput->setFocus();
	connect(textinput, SIGNAL(messageSent(QString, QString)), this, SLOT(createPrivateMessageMap(QString, QString)));
	dialog.exec();
}

void Point2PointView::createPrivateMessageMap(QString message, QString destination)
{
	QVariantMap qvm;
	qvm["Dest"] = destination;
	qvm["ChatText"] = message;
	qvm["HopLimit"] = (quint32)10;
	qDebug() << qvm;
	emit privateMessageSignal(qvm);
}