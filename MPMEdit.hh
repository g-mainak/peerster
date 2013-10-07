#ifndef PEERSTER_MPMEDIT_HH
#define PEERSTER_MPMEDIT_HH

#include <QTextEdit>
#include <QKeyEvent>
#include <QStringList>
#include <QRegExp>

class MPMEdit : public QTextEdit
{
	Q_OBJECT

    public:
    	MPMEdit(QWidget *, QString);
    	void keyPressEvent(QKeyEvent *);

    signals:
    	void messageSent(QString, QString);

   	private:
   		QString destination;
};

#endif // PEERSTER_MPMEDIT_HH