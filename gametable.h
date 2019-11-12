#ifndef GAMETABLE_H
#define GAMETABLE_H

#include <QGridLayout>
#include <QPushButton>

#include <QLineEdit>
#include <QLCDNumber>
#include <QGridLayout>
#include <QTimer>
#include <QIcon>

class GameTable : public QWidget
{
    Q_OBJECT

    unsigned int row, column;

    QGridLayout *layout;
    /* Blank icon */
    QIcon blank;

    unsigned int numberOfPlayers,currentPlayer,totalCorrectAnswer;

    /* Player properties */
    struct player{
        QLineEdit *name;
        QLCDNumber *scorePanel;
        QLCDNumber *time;
        QTimer *timer;
        unsigned int second;
    }*playerList;

    /* box is a card */
    struct box{
        QPushButton *button;
        QString text;
        unsigned int specialNumber;
        QIcon *icon;
    }*boxList,*selectedBox;


    void changeCurrentPlayer();

public:
    /*They are to control game is finished and control timers*/
    bool flag,isFinished;
    class GameTable *prev,*next;

    unsigned int gameNumber;

    GameTable(class GameTable *prev=0, class GameTable *next=0, QWidget *parent=0);
    void stopTimer();
    void startTimer();

signals:
    void finished(unsigned int);
public slots:
    void clickedBox(struct box *);
};

#endif // GAMETABLE_H
