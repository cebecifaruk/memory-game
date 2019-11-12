#include "gametable.h"
#include <QPushButton>
#include <QFile>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QValidator>

GameTable::GameTable(class GameTable *prev, class GameTable *next, QWidget *parent):
    prev(prev),next(next),QWidget(parent)
{
    /* Variable Declarations */
    unsigned i,j,k;
    selectedBox=NULL;
    flag=false;
    totalCorrectAnswer=0;

    /* Set Number of Players, rows , and columns*/
    {
        parent->hide();
        QDialog *dialog = new QDialog(this);
        QHBoxLayout dialogLayout;
        QLineEdit *row = new QLineEdit();
        QLineEdit *column = new QLineEdit();
        QLineEdit *numberOfPlayers = new QLineEdit();
        row->setValidator(new QIntValidator(0,10));
        column->setValidator(new QIntValidator(0,10));
        numberOfPlayers->setValidator(new QIntValidator(2,5));
        QPushButton *OK = new QPushButton("OK");
        dialogLayout.addWidget(new QLabel("Row:"));
        dialogLayout.addWidget(row);
        dialogLayout.addWidget(new QLabel("Column:"));
        dialogLayout.addWidget(column);
        dialogLayout.addWidget(new QLabel("Player Number:"));
        dialogLayout.addWidget(numberOfPlayers);
        dialogLayout.addWidget(OK);
        dialog->setLayout(&dialogLayout);

        /* To check rules of game*/
        QObject::connect(OK,&QPushButton::clicked,[=](){
            if(row->text().toInt()*column->text().toInt()%2
               || row->text().toInt()*column->text().toInt()<4
               || numberOfPlayers->text().toInt() < 2) {
                OK->setText( "OK  (row*column must be even and more than 4, also Player Nmber must be between one and six)");
                OK->setStyleSheet("color:red");
                return;
            }
            this->row=row->text().toInt();
            this->column=column->text().toInt();
            this->numberOfPlayers=numberOfPlayers->text().toInt();
            parent->show();
            dialog->close();

        });
        /* Wait until dialog is closed*/
        dialog->exec();
    }

    /* File Operations */
    QFile *list=new QFile(":/resources/list.txt");
    QFile *css=new QFile(":/resources/style.css");
    if(!(list->open(QIODevice::ReadOnly | QIODevice::Text) && css->open(QIODevice::ReadOnly | QIODevice::Text))) return;

    /*Random initailaizer */
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    /* Layout Setup */
    layout= new QGridLayout;
    this->setLayout(layout);
    this->setStyleSheet(css->readAll());

    /* Score Table */
    QHBoxLayout *scoreLayout= new QHBoxLayout;
    playerList = new struct player[numberOfPlayers];
    for(i=0;i<numberOfPlayers;i++) {

        (playerList+i)->name = new QLineEdit;
        (playerList+i)->scorePanel = new QLCDNumber;
        (playerList+i)->time = new QLCDNumber;
        (playerList+i)->timer = new QTimer;

        (playerList+i)->name->setText(QString("Player ") + QString::number(i+1));
        (playerList+i)->scorePanel->display(0);
        (playerList+i)->time->setDigitCount(6);
        (playerList+i)->time->display("0:0");
        (playerList+i)->second=0;

        scoreLayout->addWidget((playerList+i)->name);
        scoreLayout->addWidget((playerList+i)->scorePanel);
        scoreLayout->addWidget(new QLabel("Time:"));
        scoreLayout->addWidget((playerList+i)->time);

        //TODO: SizePolicy

        connect((playerList+i)->timer,&QTimer::timeout,[=]() {
            ((playerList+i)->second)++;
            (playerList+i)->time->display(QString::number((unsigned int)(playerList+i)->second/60)+":"+QString::number((unsigned int)(playerList+i)->second%60));
        });
    }
    layout->addLayout(scoreLayout,0,0,1,column);
    currentPlayer=numberOfPlayers-1;
    changeCurrentPlayer();


    /* Boxes */
    boxList = new struct box[row*column];
    for(i=0;i<row;i++)
        for(j=0;j<column;j++) {
            struct box *box = boxList+i*column+j;
            box->button = new QPushButton();
            box->button->setIconSize(QSize(64,64));
            connect (box->button,&QPushButton::clicked,[=](){
               clickedBox(box);
            });
            layout->addWidget(box->button,i+1,j,1,1);
        }



    /* To randomize cards*/
    unsigned int lineNumber;
    for (lineNumber=0;!list->atEnd();lineNumber++,list->readLine());


    QVector<unsigned int> usedCards,usedLines;

    for (i=0;i<row*column/2;i++) {

        label1:
        list->seek(0);
        for (j=0;j<(qrand()%lineNumber-1);j++,list->readLine());
        if (usedLines.indexOf(j+1)>=0)
            goto label1;
        usedLines.append(j+1);
        QString buffer = list->readLine();
        buffer.remove("\n");
        for (j=0;j<2;j++) {
            label2:
            k=qrand()%(row*column);
            if(usedCards.indexOf(k)>=0)
                goto label2;
            usedCards.append(k);
            (boxList+k)->text = buffer;
            if(buffer.length()>6 && buffer[buffer.length()-4] == '.')
                (boxList+k)->icon=new QIcon(QPixmap(buffer));
            else (boxList+k)->icon=NULL;

            (boxList+k)->specialNumber = i;
        }
    }


}


/* To check score */
void GameTable::clickedBox(struct box *clicked)
{
    /* If flag is open do nothing */
    if(flag) return;
    if(clicked->icon) clicked->button->setIcon(QIcon(*clicked->icon));
    else clicked->button->setText(clicked->text);
    /* To check selected button */
    if (!selectedBox) {
        selectedBox=clicked;
        return;
    }

    if(selectedBox==clicked);
    else if (selectedBox){
        QTimer *timer=new QTimer;
        timer->start(1000);
        flag=true;
        connect(timer,&QTimer::timeout, [=]() {
            if(selectedBox->specialNumber == clicked->specialNumber) {
                (playerList+currentPlayer)->scorePanel->display((playerList+currentPlayer)->scorePanel->value()+10);
                selectedBox->button->setEnabled(0);
                clicked->button->setEnabled(0);

                totalCorrectAnswer++;
                if(totalCorrectAnswer >= row*column/2) {
                    stopTimer();
                    QString buffer;
                    unsigned int maxScore=0,k,l;
                    for (k=0;k<numberOfPlayers;k++)
                        if(maxScore <= (playerList+k)->scorePanel->value()) {
                            maxScore = (playerList+k)->scorePanel->value();
                            l=k;
                            buffer = (playerList+k)->name->text();
                        }
                    (playerList+l)->name->setText(playerList->name->text()+" (Winner)");
                    (playerList+l)->name->setStyleSheet("color:blue;font:bold");
                    isFinished=true;
                    selectedBox=NULL;
                    flag=false;
                    finished((playerList+l)->scorePanel->value());
                    delete timer;
                    return;

                }
            }
            else {
                if(selectedBox->icon) selectedBox->button->setIcon(blank);
                else selectedBox->button->setText("");
                if(clicked->icon) clicked->button->setIcon(blank);
                else clicked->button->setText("");
            }
            selectedBox=NULL;
            flag=false;
            this->changeCurrentPlayer();
            delete timer;
        });

    }
}

/* To cahnge current player*/

void GameTable::changeCurrentPlayer()
{
    /*Stop timer of current player*/
    stopTimer();
    (playerList+currentPlayer)->name->setStyleSheet("color:black;font:normal");
    if (currentPlayer<numberOfPlayers-1) currentPlayer++;
    else currentPlayer = 0;
    (playerList+currentPlayer)->name->setStyleSheet("color:red;font:bold");
    startTimer();
}

void GameTable::stopTimer()
{
    (playerList+currentPlayer)->timer->stop();
}

void GameTable::startTimer()
{
    if(isFinished) return;
    (playerList+currentPlayer)->timer->start(1000);
}


