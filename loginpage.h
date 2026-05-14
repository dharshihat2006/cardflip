#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QDialog>
#include <QWidget>
#include "database.h"

namespace Ui {
class LoginPage;
}

class LoginPage : public QDialog
{
    Q_OBJECT

public:
    explicit LoginPage(Database *db,QWidget *parent = nullptr);
    ~LoginPage();
    void clearFields();
    //void verifyUserExists(const QString &username);

signals:
    void loginSuccessful(int userId, const QString &username);


private slots:
    void on_btnLogin_clicked();

    void on_btnRegister_clicked();

private:
    Ui::LoginPage *ui;
    Database *m_db;
    int checkCredentials(const QString &username, const QByteArray &passwordHash);
    bool registerUser(const QString &username, const QByteArray &passwordHash);
   // void debugDatabaseState(const QString &testUsername);
    QByteArray hashPassword(const QString &password);
   /// void resetUsersTableForTesting();
    //void debugListAllUsers();

};

#endif // LOGINPAGE_H
