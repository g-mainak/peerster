#ifndef PEERSTER_FILESEARCH_HH
#define PEERSTER_FILESEARCH_HH

#include <QDebug>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include "FileMetaData.hh"

class FileSearch : public QDialog
{
	Q_OBJECT

	public:
		FileSearch(QString, QString, FileMetaData*, QWidget *parent=0);
		QVariantMap createFileSearchLocal();
		void startTimer();

	public slots:
		void search();
		void refreshList(QVariantList, QVariantList, QString);
		void downloadFile(QListWidgetItem *item);

	signals:
		void searchReply(QVariantMap);
		void searchRequest(QVariantMap);
		void searchParams(QString, QByteArray);

	private:
		QListWidget *qlw;
		QString text;
		QString identifier;
		QTimer *timer;
		quint32 budget;
		FileMetaData *metadata;
};

class MListWidgetItem : public QListWidgetItem
{
	public:
		MListWidgetItem(const QString & title, const QByteArray & hash, const QString & origin, QListWidget * parent);
		QString text();

		QString title;
		QByteArray hash;
		QString origin;
};

#endif