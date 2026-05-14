#include "loginpage.h"
#include "ui_loginpage.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QWidget>
#include <QShortcut> // Add this include


LoginPage::LoginPage(Database *db,QWidget *parent):
    QDialog(parent),
    ui(new Ui::LoginPage),
    m_db(db)
{
    ui->setupUi(this);
    //open database
    //resetUsersTableForTesting();
    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut, &QShortcut::activated, this, &QDialog::close);

    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginPage::on_btnLogin_clicked);
    connect(ui->btnRegister, &QPushButton::clicked, this, &LoginPage::on_btnRegister_clicked);
}


LoginPage::~LoginPage()
{
    delete ui;
}
QByteArray LoginPage::hashPassword(const QString &password)
{
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

}

void LoginPage::on_btnLogin_clicked()
{
    if (!m_db || !m_db->db().isOpen()) {
        ui->lblStatus->setText("Database not connected");
        qDebug() << "Database not open in login click";
        return;
    }
    QString u = ui->leUsername->text().trimmed();
    QString p = ui->lePassword->text();

    if (u.isEmpty() || p.isEmpty()) {
        ui->lblStatus->setText("Enter username & password");
        return;
    }

    QByteArray hash = hashPassword(p);
    int id = checkCredentials(u, hash);
    if (id > 0) {
        ui->lblStatus->setText("Login successful");
        emit loginSuccessful(id,u);
        accept();
    } else {
        ui->lblStatus->setText("Invalid credentials");
    }
}
int LoginPage::checkCredentials(const QString &username, const QByteArray &passwordHash)
{
    QSqlQuery q;
    q.prepare("SELECT id, password_hash FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) {
        qDebug() << "checkCredentials exec error" << q.lastError().text();
        return -1;
    }
    if (q.next()) {
        QByteArray stored = q.value("password_hash").toByteArray();
        if (stored == passwordHash) {
            return q.value("id").toInt();
        }
    }
    return -1;
}

bool LoginPage::registerUser(const QString &username, const QByteArray &passwordHash)
{
    if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Database not available";
        return false;
    }

    // Try to insert directly and catch the unique constraint error
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash, display_name) VALUES (:username, :password_hash, :display_name)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":display_name", username);

    if (query.exec()) {
        qDebug() << "User registered successfully:" << username;
        return true;
    } else {
        QString error = query.lastError().text();
        qDebug() << "Registration error:" << error;

        // Check if it's a unique constraint error
        if (error.contains("UNIQUE", Qt::CaseInsensitive) ||
            error.contains("constraint", Qt::CaseInsensitive)) {
            qDebug() << "Username already exists (unique constraint):" << username;
            return false;
        } else {
            qDebug() << "Other database error:" << error;
            return false;
        }
    }
}
void LoginPage::on_btnRegister_clicked()
{
    QString username = ui->leUsername->text().trimmed();

    QString password = ui->lePassword->text();

    qDebug() << "\n=== NEW REGISTRATION ATTEMPT ===";
    qDebug() << "Username:" << username;
    qDebug() << "Password provided:" << !password.isEmpty();
    ui->lblStatus->setText("");

    // Basic validation
    if (username.isEmpty()) {
        ui->lblStatus->setText("Please enter a username");
        qDebug() << "Validation failed: Empty username";
        return;
    }

    if (password.isEmpty()) {
        ui->lblStatus->setText("Please enter a password");
        qDebug() << "Validation failed: Empty password";
        return;
    }

    if (username.length() < 3) {
        ui->lblStatus->setText("Username must be at least 3 characters");
        qDebug() << "Validation failed: Username too short";
        return;
    }

    if (password.length() < 4) {
        ui->lblStatus->setText("Password must be at least 4 characters");
        qDebug() << "Validation failed: Password too short";
        return;
    }

    // Database check
    if (!m_db) {
        ui->lblStatus->setText("Database error: No connection");
        qDebug() << "Database error: m_db is null";
        return;
    }

    if (!m_db->db().isOpen()) {
        ui->lblStatus->setText("Database error: Not open");
        qDebug() << "Database error: Not open";
        return;
    }

    qDebug() << "Database is available and open";

    // Hash password
    QByteArray hash = hashPassword(password);
    qDebug() << "Password hashed, attempting registration...";

    // Attempt registration
    if (registerUser(username, hash)) {
        ui->lblStatus->setText("✓ Registration successful! Please login.");
        ui->lePassword->clear();
        qDebug() << "✓ REGISTRATION SUCCESSFUL for:" << username;
    } else {
        ui->lblStatus->setText("✗ Registration failed - Username may be taken");
        qDebug() << "✗ REGISTRATION FAILED for:" << username;

        // Debug: List all current users
       // debugListAllUsers();
    }

    qDebug() << "=== REGISTRATION PROCESS COMPLETE ===\n";
}
void LoginPage::clearFields()
{
    ui->leUsername->clear();
    ui->lePassword->clear();
    ui->lblStatus->clear();
}
/*void LoginPage::debugDatabaseState(const QString &testUsername)
{
    if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Database not connected for debug";
        return;
    }

    qDebug() << "=== DATABASE DEBUG INFO ===";
    qDebug() << "Testing username:" << testUsername;
    qDebug() << "Database is open:" << m_db->db().isOpen();
    qDebug() << "Database name:" << m_db->db().databaseName();

    // Check specific username
    QSqlQuery specificCheck;
    specificCheck.prepare("SELECT username FROM users WHERE username = :username");
    specificCheck.bindValue(":username", testUsername);

    if (specificCheck.exec()) {
        if (specificCheck.next()) {
            QString foundUsername = specificCheck.value("username").toString();
            qDebug() << "FOUND username in database:" << foundUsername;
        } else {
            qDebug() << "Username NOT FOUND in database:" << testUsername;
        }
    } else {
        qDebug() << "Error checking specific username:" << specificCheck.lastError().text();
    }

    // List all usernames
    QSqlQuery userQuery("SELECT username FROM users ORDER BY username");
    qDebug() << "All existing usernames:";
    bool hasUsers = false;
    while (userQuery.next()) {
        qDebug() << "  -" << userQuery.value("username").toString();
        hasUsers = true;
    }
    if (!hasUsers) {
        qDebug() << "  No users in database";
    }
    qDebug() << "=== END DEBUG INFO ===";
}*/
  /*void LoginPage::resetUsersTableForTesting()
{
    if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Database not connected";
        return;
    }

    QSqlQuery query;
    if (query.exec("DELETE FROM users")) {
        qDebug() << "Users table cleared for testing";
    } else {
        qDebug() << "Error clearing users table:" << query.lastError().text();
    }

    // Also reset the auto-increment counter
    if (query.exec("DELETE FROM sqlite_sequence WHERE name='users'")) {
        qDebug() << "Auto-increment reset";
    }
}
void LoginPage::debugListAllUsers()
{
    if (!m_db || !m_db->db().isOpen()) {
        qDebug() << "Cannot list users - database not connected";
        return;
    }

    QSqlQuery query("SELECT id, username FROM users ORDER BY id");
    qDebug() << "=== CURRENT USERS IN DATABASE ===";
    int count = 0;
    while (query.next()) {
        count++;
        int id = query.value("id").toInt();
        QString user = query.value("username").toString();
        qDebug() << "User" << count << ": ID =" << id << ", Username =" << user;
    }

    if (count == 0) {
        qDebug() << "No users found in database";
    } else {
        qDebug() << "Total users:" << count;
    }
    qDebug() << "=== END USER LIST ===";
}*/
// Call this after registration to verify the user was saved
void verifyUserExists(const QString &username) {
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) as count FROM users WHERE username = ?");
    q.addBindValue(username);
    if (q.exec() && q.next()) {
        qDebug() << "User" << username << "exists in DB:" << (q.value("count").toInt() > 0);
    }
}



