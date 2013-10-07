#ifndef PEERSTER_FILETRANSFERDIALOG_HH
#define PEERSTER_FILETRANSFERDIALOG_HH

#include <QDebug>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class FileTransferDialog : public QDialog
{
	Q_OBJECT

	public:
		FileTransferDialog( QWidget *parent=0);

	public slots:
		void generateSearchRequest();

	signals:
		void searchParams(QString, QByteArray);

private:
	QLineEdit *lineEdit1; 
	QLineEdit *lineEdit2;

};

#endif