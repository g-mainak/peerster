#include "mtextedit.hh"

MTextEdit::MTextEdit(QWidget *parent) : QTextEdit(parent) {}

void MTextEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		emit messageSent(this->toPlainText());
		// Clear the textinput to get ready for the next input message.
		this->clear();		
	}
	else
		QTextEdit::keyPressEvent(event);
}