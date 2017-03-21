#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>

class DataBase : public QObject
{
    Q_OBJECT
public:
    explicit DataBase(QObject *parent = 0);
    ~DataBase();

    bool isOpen();
    bool open();
    void close();

private:
    QSqlDatabase db;
    QSqlQuery query;
    int fTablesCount;
    void open_db(bool withCheckTables = false);
    bool checkForTable(QString tableName, QString tableCreateScript);

signals:

public slots:
};

#endif // DATABASE_H
