#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QSqlDatabase>
class Database: public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool open();
    void close();

    // utility
    bool createTables(); // users + scores
    QSqlDatabase db() const { return m_db; }

private:
    QSqlDatabase m_db;
};

#endif // DATABASE_H
