#include "gametable.h"
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QLabel>

/* It is showed game in screen at the moment */
GameTable *iterGame;

/* Control Buttons and panels */
QPushButton *next,*prev,*newGame;
QLCDNumber *highScore,*gameNumber;

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QWidget *game = new QWidget;
    game->show();

    /* Create Main layout */
    QVBoxLayout *layout = new QVBoxLayout;
    game->setLayout(layout);
    game->setStyleSheet("QPushButton{color:#8A2BE2; font:bold}");


    /*To create Controller in order to change game results and show high score and game number  */
    QHBoxLayout controlPanel;

    next= new QPushButton("Next Result");
    prev= new QPushButton("Previous Result");
    newGame= new QPushButton("New Game");
    highScore = new QLCDNumber();
    gameNumber = new QLCDNumber();

    /* To add buttons and LCDNumbers with labels to control panel */
    controlPanel.addWidget(prev);
    controlPanel.addWidget(new QLabel("Game Number:"));
    controlPanel.addWidget(gameNumber);
    controlPanel.addWidget(newGame);
    controlPanel.addWidget(new QLabel("High Score:"));
    controlPanel.addWidget(highScore);
    controlPanel.addWidget(next);
    /*Add control panel layout to main layout*/
    layout->addLayout(&controlPanel);


    /*
     * Create first game with NULLs because GameTable is inherited class from QWidget and double linked list
     * First parameter is previous game pointer and second parameter is next game pointer
     */
    iterGame =new GameTable(NULL,NULL,game);
    iterGame->gameNumber=1;
    /*to set buttons enables*/
    next->setEnabled(iterGame->next);
    prev->setEnabled(iterGame->prev);
    /* Show Game number*/
    gameNumber->display(QString::number(iterGame->gameNumber));
    /* To add widget to main layout. Game Table is a QWidget class */
    layout->addWidget(iterGame);
    /* If game is finished call the function which is below*/
    QObject::connect(iterGame,&GameTable::finished,[=](unsigned int score) {
        /*If score is more than high score, change high score*/
        if(score>highScore->value()) highScore->display(QString::number(score));
    });

    /* To connect control buttons slots*/
    QObject::connect(next,&QPushButton::clicked,[=]() {
        /*Hide and disable last game*/
        iterGame->hide();
        iterGame->stopTimer();
        layout->removeWidget(iterGame);
        /* to go to next game */
        iterGame=iterGame->next;
        next->setEnabled(iterGame->next);
        prev->setEnabled(iterGame->prev);
        /* Show next game */
        layout->addWidget(iterGame);
        gameNumber->display(QString::number(iterGame->gameNumber));
        iterGame->show();
        iterGame->startTimer();
    });

    QObject::connect(prev,&QPushButton::clicked,[=]() {
        /* To hide and disable last game*/
        iterGame->hide();
        iterGame->stopTimer();
        layout->removeWidget(iterGame);
        /* to go to next game */
        iterGame=iterGame->prev;
        next->setEnabled(iterGame->next);
        prev->setEnabled(iterGame->prev);
        /* Show next game */
        layout->addWidget(iterGame);
        gameNumber->display(QString::number(iterGame->gameNumber));
        iterGame->show();
        iterGame->startTimer();
    });

    QObject::connect(newGame,&QPushButton::clicked,[=]() {
        /*Hide and disable last game*/
        iterGame->hide();
        iterGame->stopTimer();
        /* To set flag and isFinished in order not to click and startTimer*/
        iterGame->flag=true;
        iterGame->isFinished=true;

        /* to go to next game */
        layout->removeWidget(iterGame);
        for(;iterGame->next;iterGame=iterGame->next);
        iterGame->next = new GameTable(iterGame,NULL,game);
        iterGame = iterGame->next;
        iterGame->gameNumber=iterGame->prev->gameNumber+1;
        next->setEnabled(iterGame->next);
        prev->setEnabled(iterGame->prev);
        /* Show next game */
        layout->addWidget(iterGame);
        gameNumber->display(QString::number(iterGame->gameNumber));
        iterGame->startTimer();
        /* If game send finished signal it will recieve the signal and it will decide high score*/
        QObject::connect(iterGame,&GameTable::finished,[=](unsigned int score) {
            if(score>highScore->value()) highScore->display(QString::number(score));
        });
    });

    return app.exec();
}
