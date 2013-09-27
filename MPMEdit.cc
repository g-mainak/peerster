#include "MPMEdit.hh"

MPMEdit::MPMEdit(QWidget *parent, QString string) : QTextEdit(parent) 
{
	QStringList list = string.split(QRegExp(" "));
	destination = list.at(0);
}

void MPMEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		emit messageSent(this->toPlainText(), destination);
		// Clear the textinput to get ready for the next input message.
		this->clear();		
	}
	else
		QTextEdit::keyPressEvent(event);
}