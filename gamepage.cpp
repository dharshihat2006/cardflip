#include "gamepage.h"
#include "ui_gamepage.h"
#include <QPushButton>
#include <QGridLayout>
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QDebug>
#include <qsqlerror.h>
#include <QShortcut> // Add this include

//const int GamePage::CARD_NORMAL = 0;
//const int GamePage::CARD_BOMB = -1;
///const int GamePage::CARD_SPECIAL = -2;
GamePage::GamePage(Database *db, int userId, int level,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GamePage)
    , m_db(db)
    , m_userId(userId)
    , m_level(level)
    , m_score(0)
    , m_timeLeft(0)
    , m_pairsFound(0)
    , m_totalPairs(0)
    , m_timer(nullptr)
    , m_firstCardIndex(-1)
    , m_secondCardIndex(-1)
    , m_processing(false)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);


    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut, &QShortcut::activated, this, &QDialog::close);
    ui->lblLevel->setText(QString("Level: %1").arg(m_level + 1));
    ui->lblScore->setText("Score: 0");


    // Initialize game
    initializeGame();

    // Connect buttons
   // connect(ui->btnRestart, &QPushButton::clicked, this, &GamePage::on_btnRestart_clicked);
    //connect(ui->btnReturn, &QPushButton::clicked, this, &GamePage::on_btnReturn_clicked);
}

GamePage::~GamePage()
{

    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }

    // Clear cards safely
    for (QPushButton* card : m_cards) {
        if (card) {
            delete card;
        }
    }
    m_cards.clear();

    delete ui;
}

QString GamePage::getCardEmoji(int cardValue) const
{
    if (cardValue == CARD_BOMB) {
        return "💣";
    } else if (cardValue == CARD_SPECIAL) {
        return "⭐";
    } else if (cardValue >= CARD_NORMAL) {
        // Computer part emojis
        QStringList emojis = {
            "💻", // CPU
            "🔌", // Motherboard
            "🧠", // RAM
            "💾", // Hard Drive
            "🎮", // GPU
            "⚡", // Power Supply
            "⌨️", // Keyboard
            "🖱️", // Mouse
            "🖥️", // Monitor
            "🔊"  // Speakers
        };

        if (cardValue < emojis.size()) {
            return emojis[cardValue];
        }
    }
    return "?";
}

QString GamePage::getCardColor(int cardValue) const
{
    if (cardValue == CARD_BOMB) {
        return "#FF6B6B"; // Red
    } else if (cardValue == CARD_SPECIAL) {
        return "#4ECDC4"; // Teal
    } else if (cardValue >= CARD_NORMAL) {
        // Different colors for computer parts
        QStringList colors = {
            "#F9C80E", // Yellow - CPU
            "#6AB04C", // Green - Motherboard
            "#ED5564", // Pink - RAM
            "#577590", // Blue-gray - Hard Drive
            "#F26430", // Orange - GPU
            "#7F3F98", // Purple - Power Supply
            "#F7CAC9", // Light pink - Keyboard
            "#A9DEF9", // Light blue - Mouse
            "#FFF176", // Light yellow - Monitor
            "#B7E4C7"  // Light green - Speakers
        };

        if (cardValue < colors.size()) {
            return colors[cardValue];
        }
    }
    return "#2E86AB"; // Default blue
}
void GamePage::initializeGame()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }

    // Clear existing cards
    for (QPushButton* card : m_cards) {
        if (card) {
            delete card;
        }
    }
    m_cards.clear();
    m_cardValues.clear();
    m_cardRevealed.clear();
    m_cardMatched.clear();

    // Reset game state
    m_score = 0;
    m_pairsFound = 0;
    m_firstCardIndex = -1;
    m_secondCardIndex = -1;
    m_processing = false;

    // Get game configuration based on level
    m_totalPairs = getRequiredPairs();
    m_timeLeft = getTimeLimit();

    // Update UI
    ui->lblScore->setText(QString("Score: %1").arg(m_score));
    ui->lblTimer->setText(QString("Time: %1s").arg(m_timeLeft));

    // Create cards
    createCards();
    shuffleCards();

    // Setup timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GamePage::updateTimer);
    m_timer->start(1000); // Update every second
}

void GamePage::createCards()
{
    QWidget *gridContainer = findChild<QWidget*>("gridContainer");
    if (!gridContainer) {
        qDebug() << "Grid container not found!";
        return;
    }

    // Clear existing layout
    QLayout *existingLayout = gridContainer->layout();
    if (existingLayout) {
        QLayoutItem *item;
        while ((item = existingLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete existingLayout;
    }

    int gridSize = getGridSize();
    QGridLayout *gridLayout = new QGridLayout(gridContainer);
    gridLayout->setSpacing(5);
    gridLayout->setContentsMargins(5, 5, 5, 5);

    // Create card values (pairs)
    for (int i = 0; i < m_totalPairs; i++) {
        m_cardValues.push_back(i);
        m_cardValues.push_back(i); // Add pair
    }

    // Add special cards
    m_cardValues.push_back(CARD_SPECIAL);
    m_cardValues.push_back(CARD_SPECIAL);

    // Add bomb card
    m_cardValues.push_back(CARD_BOMB);

    // Create card buttons
    for (size_t i = 0; i < m_cardValues.size(); i++) {
        QPushButton *card = new QPushButton(gridContainer);
        card->setFixedSize(90, 90);

        // Set initial face-down state
        card->setText("");
        card->setStyleSheet(
            "QPushButton {"
            "    background-color: #2E86AB;"
            "    border: 2px solid #1B5B7A;"
            "    border-radius: 8px;"
            "    font-weight: bold;"
            "    font-size: 24px;"
            "    color: white;"
            "}"
            "QPushButton:hover {"
            "    background-color: #3FA7D6;"
            "}"
            );

        connect(card, &QPushButton::clicked, this, &GamePage::onCardClicked);

        int row = i / gridSize;
        int col = i % gridSize;
        gridLayout->addWidget(card, row, col);

        m_cards.push_back(card);
        m_cardRevealed.push_back(false);
        m_cardMatched.push_back(false);
    }

    gridContainer->setLayout(gridLayout);
}

void GamePage::shuffleCards()
{for (int i = static_cast<int>(m_cardValues.size()) - 1; i > 0; i--) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(m_cardValues[i], m_cardValues[j]);
    }
}

void GamePage::onCardClicked()
{
    if (m_processing) return;

    QPushButton *clickedCard = qobject_cast<QPushButton*>(sender());
    if (!clickedCard) return;

    // Find card index
    int index = -1;
    for (size_t i = 0; i < m_cards.size(); i++) {
        if (m_cards[i] == clickedCard) {
            index = i;
            break;
        }
    }

    if (index == -1 || m_cardRevealed[index] || m_cardMatched[index]) {
        return;
    }

    if (m_cardValues[index] == CARD_BOMB) {
        handleBombCard(index);
        return;
    }

    flipCard(index);
}
void GamePage::handleBombCard(int index)
{ m_cardRevealed[index] = true;
    m_cardMatched[index] = true;

    // Show bomb emoji
    m_cards[index]->setText("💣");
    m_cards[index]->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF6B6B;"
        "    border: 2px solid #C53030;"
        "    border-radius: 8px;"
        "    font-weight: bold;"
        "    font-size: 24px;"
        "    color: white;"
        "}"
        );

    // Apply bomb effect
    m_timeLeft = qMax(0, m_timeLeft - 2);
    ui->lblTimer->setText(QString("Time: %1s").arg(m_timeLeft));

    m_score += 5; // Small points for finding bomb
    ui->lblScore->setText(QString("Score: %1").arg(m_score));

    // Disable the bomb card
    m_cards[index]->setEnabled(false);

    // Check if time ran out due to bomb
    if (m_timeLeft <= 0) {
        checkGameOver();
    }
}
void GamePage::flipCard(int index)
{
    m_cardRevealed[index] = true;

    // Show card emoji and color
    int cardValue = m_cardValues[index];
    QString emoji = getCardEmoji(cardValue);
    QString color = getCardColor(cardValue);

    m_cards[index]->setText(emoji);
    m_cards[index]->setStyleSheet(
        "QPushButton {"
        "    background-color: " + color + ";"
                  "    border: 2px solid #1B5B7A;"
                  "    border-radius: 8px;"
                  "    font-weight: bold;"
                  "    font-size: 24px;"
                  "    color: " + (color == "#F9C80E" ? "#333" : "white") + ";"
                                                    "}"
        );

    // Handle card selection logic
    if (m_firstCardIndex == -1) {
        m_firstCardIndex = index;
    } else {
        m_secondCardIndex = index;
        m_processing = true;
        QTimer::singleShot(75, this, &GamePage::checkMatch);
    }
}


void GamePage::checkMatch()
{
    int firstValue = m_cardValues[m_firstCardIndex];
    int secondValue = m_cardValues[m_secondCardIndex];

    if (firstValue == secondValue) {
        // Cards match
        m_cardMatched[m_firstCardIndex] = true;
        m_cardMatched[m_secondCardIndex] = true;

        m_cards[m_firstCardIndex]->setEnabled(false);
        m_cards[m_secondCardIndex]->setEnabled(false);

        // Different scoring based on card type
        if (firstValue >= CARD_NORMAL) {
            // Normal pair
            m_pairsFound++;
            m_score += 20;
        } else if (firstValue == CARD_SPECIAL) {
            // Special pair
            m_timeLeft += 5; // +5 seconds for special pair
            ui->lblTimer->setText(QString("Time: %1s").arg(m_timeLeft));
            m_score += 30; // More points for special pair
        }

        // Check if level completed (only count normal pairs)
        if (firstValue >= CARD_NORMAL) {
            if (m_pairsFound >= m_totalPairs) {
                m_score += m_timeLeft * 2; // Bonus points for remaining time
                checkGameOver();
            }
        }
    } else {
        // No match - flip cards back after delay
        int firstIndex = m_firstCardIndex;
        int secondIndex = m_secondCardIndex;

        QTimer::singleShot(500, this, [this, firstIndex, secondIndex]() {
            if (firstIndex >= 0 && firstIndex < static_cast<int>(m_cardRevealed.size()) &&
                secondIndex >= 0 && secondIndex < static_cast<int>(m_cardRevealed.size()) &&
                m_cardRevealed[firstIndex] && m_cardRevealed[secondIndex] &&
                !m_cardMatched[firstIndex] && !m_cardMatched[secondIndex]) {

                m_cardRevealed[firstIndex] = false;
                m_cardRevealed[secondIndex] = false;

                // Reset cards to face-down state
                if (m_cards[firstIndex]) {
                    m_cards[firstIndex]->setText("");
                    m_cards[firstIndex]->setStyleSheet(
                        "QPushButton {"
                        "    background-color: #2E86AB;"
                        "    border: 2px solid #1B5B7A;"
                        "    border-radius: 8px;"
                        "    font-weight: bold;"
                        "    font-size: 24px;"
                        "    color: white;"
                        "}"
                        "QPushButton:hover {"
                        "    background-color: #3FA7D6;"
                        "}"
                        );
                }

                if (m_cards[secondIndex]) {
                    m_cards[secondIndex]->setText("");
                    m_cards[secondIndex]->setStyleSheet(
                        "QPushButton {"
                        "    background-color: #2E86AB;"
                        "    border: 2px solid #1B5B7A;"
                        "    border-radius: 8px;"
                        "    font-weight: bold;"
                        "    font-size: 24px;"
                        "    color: white;"
                        "}"
                        "QPushButton:hover {"
                        "    background-color: #3FA7D6;"
                        "}"
                        );
                }
            }
        });
    }

    // Update score display
    ui->lblScore->setText(QString("Score: %1").arg(m_score));

    // Reset selection
    m_firstCardIndex = -1;
    m_secondCardIndex = -1;
    m_processing = false;

    // Check timer
    if (m_timeLeft <= 0) {
        checkGameOver();
    }
}


void GamePage::updateTimer()
{
    m_timeLeft--;
    ui->lblTimer->setText(QString("Time: %1s").arg(m_timeLeft));

    if (m_timeLeft <= 0) {
        if (m_timer) {
            m_timer->stop();
        }
        checkGameOver();
    }
}

// The rest of the functions remain the same...





void GamePage::checkGameOver()
{
    if (m_timer) {
        m_timer->stop();
    }
    bool won = m_pairsFound >= m_totalPairs;
    QString message;

    if (won) {
        message = QString("Congratulations! You completed the level!\nFinal Score: %1").arg(m_score);
    } else {
        message = QString("Time's up! You found %1/%2 pairs.\nFinal Score: %3")
        .arg(m_pairsFound).arg(m_totalPairs).arg(m_score);
    }

    // Save score to database
    saveScore();

    QMessageBox::information(this, "Game Over", message);
}

void GamePage::saveScore()
{if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Database not available for saving score";
        return;
    }

    QSqlQuery q;
    q.prepare("INSERT INTO scores (user_id, level, score, time_left) VALUES (:user_id, :level, :score, :time_left)");
    q.bindValue(":user_id", m_userId);
    q.bindValue(":level", m_level);
    q.bindValue(":score", m_score);
    q.bindValue(":time_left", m_timeLeft);

    if (!q.exec()) {
        qDebug() << "Failed to save score:" << q.lastError().text();
    } else {
        qDebug() << "Score saved successfully";
    }
}
void GamePage::on_btnRestart_clicked()
{
    initializeGame();

}


void GamePage::on_btnReturn_clicked()
{
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }

    // Close the game dialog and return to menu
    this->accept();
}
int GamePage::getGridSize() const
{
    switch (m_level) {
    case 0: return 4; // Easy: 4x4 grid
    case 1: return 5; // Medium: 5x5 grid
    case 2: return 6; // Hard: 6x6 grid
    default: return 4;
    }
}

int GamePage::getTimeLimit() const
{
    switch (m_level) {
    case 0: return 90;  // Easy: 90 seconds
    case 1: return 60;  // Medium: 60 seconds
    case 2: return 45;  // Hard: 45 seconds
    default: return 60;
    }
}

int GamePage::getRequiredPairs() const
{
    switch (m_level) {
    case 0: return 6;  // Easy: 6 pairs
    case 1: return 8;  // Medium: 8 pairs
    case 2: return 10; // Hard: 10 pairs
    default: return 6;
    }
}

