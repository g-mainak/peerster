#include "FileTransferDialog.hh"

FileTransferDialog::FileTransferDialog(QWidget *parent) : QDialog(parent) 
{
	// Add the lineEdits with their respective labels
	lineEdit1 = new QLineEdit();
	lineEdit1->setPlaceholderText("Enter destination");
	lineEdit2 = new QLineEdit();
	lineEdit2->setPlaceholderText("Enter Hash");
	QPushButton *ok = new QPushButton("OK");

	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->addWidget(lineEdit1);
	vLayout->addWidget(lineEdit2);
	vLayout->addWidget(ok);
	setLayout(vLayout);

	connect(ok, SIGNAL(clicked()), this, SLOT(generateSearchRequest()));
}

void FileTransferDialog::generateSearchRequest()
{
	this->done(0);
	emit searchParams(lineEdit1->text(), QByteArray::fromHex(lineEdit2->text().toUtf8()));
}