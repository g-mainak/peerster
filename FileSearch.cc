#include "FileSearch.hh"

FileSearch::FileSearch(QString text, QString identifier, FileMetaData *metadata, QWidget *parent) : QDialog( parent )
{
	this->text = text;
	this->identifier = identifier;
	this->budget = 2;
	this->metadata = metadata;
	qlw = new QListWidget(this);
	connect(qlw, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(downloadFile(QListWidgetItem*)));

	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->addWidget(new QLabel("Search results:"));
	vLayout->addWidget(qlw);
	setLayout(vLayout);
}

void FileSearch::startTimer()
{
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(search()));
	timer->start(1000);
	emit search();
}

void FileSearch::search()
{
	if (budget < 100 && qlw->count() < 10)
	{
		QVariantMap qvm = createFileSearchLocal();
		qDebug() << "Search: " << qvm;
		budget *= 2;
		emit searchRequest(qvm);
	}
	else
		disconnect(timer);
}

QVariantMap FileSearch::createFileSearchLocal()
{
	QVariantMap qvm;
	qvm["Search"] = text;
	qvm["Budget"] = budget;
	return qvm;
}

void FileSearch::refreshList(QVariantList names, QVariantList ids, QString origin)
{
	qDebug() << names << ids;
	for (int i=0; i<names.size(); i++)
	{
		QList<QListWidgetItem *> list = qlw->findItems(names.at(i).toString(), 0);
		bool flag = true;
		for(int j=0; j< list.size(); j++)
		{
			MListWidgetItem *it = (MListWidgetItem*)list.at(j);
			if (it->hash == ids.at(j).toByteArray() && origin == it->origin)
				flag = false;
		}
		if (flag)
			new MListWidgetItem(names.at(i).toString(), ids.at(i).toByteArray(), origin, qlw);
	}
}

void FileSearch::downloadFile(QListWidgetItem *item)
{
	MListWidgetItem * it = (MListWidgetItem*)item;
	emit searchParams(it->origin, QString(it->hash));
}

MListWidgetItem::MListWidgetItem(const QString & title, const QByteArray & hash, const QString & origin, QListWidget * parent = 0) :
	QListWidgetItem(title, parent, 1001)
{
	this->title = title;
	this->hash = hash;
	this->origin = origin;
}

QString MListWidgetItem::text()
{
	return title;
}