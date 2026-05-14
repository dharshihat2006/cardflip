#ifndef GAMEPAGE_H
#define GAMEPAGE_H

#include <QDialog>
#include "database.h"
#include <QTimer>
#include <QPushButton>
#include <QGridLayout>
#include <vector>

namespace Ui {
class GamePage;
}

class GamePage : public QDialog
{
    Q_OBJECT

public:
    explicit GamePage(Database *db, int userId, int level,QWidget *parent = nullptr);
    ~GamePage();

private slots:
    void on_btnRestart_clicked();

    void on_btnReturn_clicked();
    void onCardClicked();
    void updateTimer();

private:
    Ui::GamePage *ui;
    Database *m_db;
    int m_userId;
    int m_level;
    int m_score;
    int m_timeLeft;
    int m_pairsFound;
    int m_totalPairs;

    QTimer *m_timer;
    std::vector<QPushButton*> m_cards;
    std::vector<int> m_cardValues;
    std::vector<bool> m_cardRevealed;
    std::vector<bool> m_cardMatched;

    QVector<QPixmap> m_cardImages;
    QPixmap m_cardBackImage;
    QPixmap m_bombImage;
    QPixmap m_specialImage;

    int m_firstCardIndex;
    int m_secondCardIndex;
    bool m_processing;

    enum CardType {
        CARD_NORMAL = 0,   // Normal cards (positive values)
        CARD_BOMB = -1,    // Bomb card (single, activates immediately)
        CARD_SPECIAL = -2  // Special card (needs pairing)
    };
    void initializeGame();
    void createCards();
    void shuffleCards();
    void flipCard(int index);
    void checkMatch();
    void checkGameOver();
    void saveScore();
    void handleBombCard(int index);
    QString getCardEmoji(int cardValue) const;
    QString getCardColor(int cardValue) const;

    int getGridSize() const;
    int getTimeLimit() const;
    int getRequiredPairs() const;
};

#endif // GAMEPAGE_H
