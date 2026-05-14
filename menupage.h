#ifndef MENUPAGE_H
#define MENUPAGE_H

#include <QDialog>
#include "database.h"

namespace Ui {
class MenuPage;
}

class MenuPage : public QDialog
{
    Q_OBJECT

public:
    explicit MenuPage(Database *db,QWidget *parent = nullptr);
    ~MenuPage();
    void setUser(int userId, const QString &username);
signals:
    void startGame(int userId, int level);
    void showLeaderboard();
    void logoutRequested();
private slots:
    void on_btnStart_clicked();

    void on_btnLeaderboard_clicked();

    void on_btnLogout_clicked();

private:
    Ui::MenuPage *ui;
    Database *m_db;
    int m_userId;
    QString m_username;
};

#endif // MENUPAGE_H
