#include "mainwindow.h"
#include "loginpage.h"
#include <QApplication>
#include "database.h"
#include <QMessageBox>
#include "menupage.h"
#include <QDebug>
#include "gamepage.h"
#include "leaderboardpage.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Database db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, "Database Error",
                              "Cannot connect to database!\nThe application will now exit.");
        qDebug() << "Failed to open database connection";

        return -1;
    }

    qDebug() << "Database opened successfully";
    LoginPage loginPage(&db);
    loginPage.showMaximized();

    // Create menu page but DON'T show it yet
    MenuPage menuPage(&db);

    QObject::connect(&loginPage, &LoginPage::loginSuccessful,
                     &menuPage, &MenuPage::setUser);

    QObject::connect(&loginPage, &LoginPage::loginSuccessful, [&]() {
        loginPage.hide();
        // Force menu page to be maximized when shown
        menuPage.showMaximized();
        menuPage.activateWindow(); // Ensure it gets focus
        menuPage.raise(); // Bring to front
    });

    QObject::connect(&menuPage, &MenuPage::logoutRequested, [&]() {
        menuPage.hide();
        loginPage.clearFields();
        loginPage.showMaximized();
    });

    QObject::connect(&menuPage, &MenuPage::startGame, [&](int userId, int level) {
        menuPage.hide();
        GamePage gamePage(&db, userId, level);
        gamePage.showMaximized();
        gamePage.exec();
        // When game ends, show menu maximized again
        menuPage.showMaximized();
    });

    QObject::connect(&menuPage, &MenuPage::showLeaderboard, [&]() {
        LeaderboardPage leaderboardPage(&db);
        leaderboardPage.showMaximized();
        leaderboardPage.exec();
    });

    int result = a.exec();
    db.close();
    return result;
}
