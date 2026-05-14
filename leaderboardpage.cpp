#include "leaderboardpage.h"
#include "ui_leaderboardpage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QShortcut> // Add this include

LeaderboardPage::LeaderboardPage(Database *db,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LeaderboardPage)
    , m_db(db)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut, &QShortcut::activated, this, &QDialog::close);

    // Connect signals
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Rank" << "Username" << "Level" << "Score" << "Time Left");

    // Style the table
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    ui->tableWidget->setColumnWidth(0, 60);   // Rank
    ui->tableWidget->setColumnWidth(1, 150);  // Username
    ui->tableWidget->setColumnWidth(2, 100);  // Level
    ui->tableWidget->setColumnWidth(3, 100);  // Score
    ui->tableWidget->setColumnWidth(4, 100);  // Time Left
    // Connect signals
    //connect(ui->btnRefresh, &QPushButton::clicked, this, &LeaderboardPage::loadLeaderboard);
    // Load initial data
    connect(ui->cbLevelFilter, SIGNAL(currentIndexChanged(int)),
            this, SLOT(loadLeaderboard()));
    loadLeaderboard();
}

LeaderboardPage::~LeaderboardPage()
{
    delete ui;
}
void LeaderboardPage::loadLeaderboard()
{
    if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Database not available for leaderboard";
        return;
    }

    // Clear existing data
    ui->tableWidget->setRowCount(0);

    // Build query based on filter
    QString queryStr =
        "SELECT u.username, s.level, s.score, s.time_left "
        "FROM scores s "
        "JOIN users u ON s.user_id = u.id ";

    int levelFilter = ui->cbLevelFilter->currentIndex() - 1; // -1 for "All Levels"

    if (levelFilter >= 0) {
        queryStr += "WHERE s.level = :level ";
    }

    queryStr += "ORDER BY s.score DESC, s.time_left DESC LIMIT 50";

    QSqlQuery q;
    q.prepare(queryStr);

    if (levelFilter >= 0) {
        q.bindValue(":level", levelFilter);
    }

    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            ui->tableWidget->insertRow(row);

            // Rank
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));

            // Username
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(q.value("username").toString()));

            // Level
            int level = q.value("level").toInt();
            QString levelName;
            switch (level) {
            case 0: levelName = "Easy"; break;
            case 1: levelName = "Medium"; break;
            case 2: levelName = "Hard"; break;
            default: levelName = "Unknown";
            }
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(levelName));

            // Score
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(q.value("score").toInt())));

            // Time Left
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(q.value("time_left").toInt()) + "s"));

            // Color coding for top 3 ranks
            if (row == 0) {
                // Gold for 1st place
                for (int col = 0; col < 5; col++) {
                    if (ui->tableWidget->item(row, col)) {
                        ui->tableWidget->item(row, col)->setBackground(QColor(255, 215, 0));
                    }
                }
            } else if (row == 1) {
                // Silver for 2nd place
                for (int col = 0; col < 5; col++) {
                    if (ui->tableWidget->item(row, col)) {
                        ui->tableWidget->item(row, col)->setBackground(QColor(192, 192, 192));
                    }
                }
            } else if (row == 2) {
                // Bronze for 3rd place
                for (int col = 0; col < 5; col++) {
                    if (ui->tableWidget->item(row, col)) {
                        ui->tableWidget->item(row, col)->setBackground(QColor(205, 127, 50));
                    }
                }
            }

            row++;
        }
    } else {
        qDebug() << "Leaderboard query error:" << q.lastError().text();
    }
}
