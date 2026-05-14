#ifndef LEADERBOARDPAGE_H
#define LEADERBOARDPAGE_H

#include <QDialog>
#include "database.h"


namespace Ui {
class LeaderboardPage;
}

class LeaderboardPage : public QDialog
{
    Q_OBJECT

public:
    explicit LeaderboardPage(Database *db,QWidget *parent = nullptr);
    ~LeaderboardPage();
private slots:
    void loadLeaderboard();

private:
    Ui::LeaderboardPage *ui;
    Database *m_db;
};

#endif // LEADERBOARDPAGE_H
