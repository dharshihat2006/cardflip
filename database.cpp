#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database::Database(QObject *parent)
    : QObject(parent) {}
Database::~Database()
{
    close();
}

bool Database::open()
{
    if (QSqlDatabase::isDriverAvailable("QSQLITE") == false) {
        qDebug() << "SQLite driver missing";
        return false;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    // place the DB file next to the executable (or choose full path)
    m_db.setDatabaseName("cardflipgame_NEW.db");  // Changed name

    if (!m_db.open()) {
        qDebug() << "Failed to open DB:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

void Database::close()
{
    if (m_db.isOpen()) m_db.close();
}

bool Database::createTables()
{
    QSqlQuery q;
    // users table
    bool ok1 = q.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "display_name TEXT"
        ")"
        );

    // scores table
    bool ok2 = q.exec(
        "CREATE TABLE IF NOT EXISTS scores ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "level INTEGER NOT NULL, "
        "score INTEGER NOT NULL, "
        "time_left INTEGER NOT NULL, "
        "created_at TEXT DEFAULT (datetime('now')), "
        "FOREIGN KEY(user_id) REFERENCES users(id)"
        ")"
        );

    if (!ok1 || !ok2){
        qDebug() << "createTables error:" << q.lastError().text();
        return false;
    }

    qDebug() << "Tables created successfully.";
    return true;
}
