#ifndef PEERSTER_MTEXTEDIT_HH
#define PEERSTER_MTEXTEDIT_HH

#include <QTextEdit>
#include <QKeyEvent>

class MTextEdit : public QTextEdit
{
	Q_OBJECT

    public:
    	MTextEdit(QWidget *);
    	void keyPressEvent(QKeyEvent *);

    signals:
    	void messageSent(QString);
};

#endif // PEERSTER_MTEXTEDIT_HH