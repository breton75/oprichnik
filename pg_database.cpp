#include "pg_database.h"

#include "pg_scripts.h"
#include "u_log.h"
#include <QMessageBox>

//-------------------------------------------------------------
int getDBTableRecCount(QSqlQuery qry, QString tab_name) {
    int res = -1;
    try {
        if (qry.exec(QString("select * from pg_tables where schemaname = 'public' and tablename = '%1'").arg(tab_name)) == true) {
            if(qry.size() > 0) {
                qry.finish();
                if (qry.exec(QString("select count(*) as rec_count from %1").arg(tab_name)) == true) {
                    qry.first();
                    res = qry.value("rec_count").toInt();
                    //qDebug() << "Таблица " << tab_name << " - количество записей: " << res;
                } else {
                    qDebug() << "Выборка из таблицы " << tab_name << " - ошибка: " << qry.lastError().text();
                }
            } else {
                qDebug() << "Таблица " << tab_name << " не найдена";
            }
        } else {
            qDebug() << "Поиск таблицы " << tab_name << " - ошибка: " << qry.lastError().text();
        }
    } catch (...) {
//    } catch (const char* error) {
//    } catch (Exception &e) {
        //qDebug() << "Поиск таблицы " << tab_name << " - исключение: " << error;
        qDebug() << "Поиск таблицы " << tab_name << " - исключение";// << e.text();
    }
    qry.finish();
    return res;
}
//-------------------------------------------------------------
DataBase::DataBase(QObject *parent) : QObject(parent)
{
    fTablesCount = sizeof(tablesList) / sizeof(tablesList[0]);
    //QMessageBox::information(0, "Start TPO", "DataBase creation");
    db = QSqlDatabase::addDatabase("QPSQL");//, "postgres");
    db.setHostName("localhost");
    db.setDatabaseName(DBName);//DATABASE_NAME); // postgres
    db.setUserName("postgres");
    db.setPassword("postgres");
    open_db(true);
    //QMessageBox::information(0, "Start TPO", "DataBase created");
}

DataBase::~DataBase()
{
    query.finish();
    db.close();
    //db.removeDatabase("QPSQL");
}

bool DataBase::isOpen()
{
    return db.isOpen();
}

bool DataBase::open()
{
    if(db.isOpen()) return true;
    open_db(true);
    return db.isOpen();
}

void DataBase::close()
{
    if(db.isOpen()) db.close();
}

void DataBase::open_db(bool withCheckTables)
{
    if(db.isOpen()) return;
    int err_count = 0;
    //QString db_error;
    QString msg;
    try {
        // пробуем открыть БД
        if (!db.open()) {
            // проверяем наличие БД
            msg = db.lastError().text();
            if(msg.contains(" not exist")) {
                msg = "БД проекта на сервере отсутствует - создаем";
                qDebug() << msg;
                logMsg(msgInfo, msg);
                // подключаемся к системной БД
                db.setDatabaseName("postgres");
                if(!db.open()) {
                    // что-то серьезное - здесь не разберемся
                    msg = "Ошибка подключения к БД postgres на сервере:<br>" + db.lastError().text();
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка подключения к БД postgres на сервере:";
                    qDebug() << db.lastError().text();
                    qDebug() << "Выходим.";
                    return;
                }
                // создаем БД проекта
                query = QSqlQuery(db);
                if (!query.exec(qryCreateDB)) {
                    msg = "Ошибка создания БД проекта на сервере:<br>" + query.lastError().text();
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка создания БД проекта на сервере:";
                    qDebug() << query.lastError().text();
                    qDebug() << "Выходим.";
                    return;
                } else {
                    query.exec(qryCreateDB_SetComment);
                }
                msg = "БД проекта создана, подключаемся";
                logMsg(msgInfo, msg);
                qDebug() << msg;
                query.finish();
                db.close();
                db.setDatabaseName(DBName);//DATABASE_NAME);
                // переподключаемся
                if (!db.open()) {
                    // неустранимо, выходим
                    msg = "Ошибка подключения к БД проекта:<br>" + db.lastError().text() + "<br>Попробуйте перезапустить программу.";
                    logMsg(msgError, msg);
                    qDebug() << "Ошибка подключения к БД " << DBName << ":";// DATABASE_NAME
                    qDebug() << db.lastError().text();
                    qDebug() << "Попробуйте перезапустить программу.";
                    qDebug() << "Выходим.";
                    return;
                }
                // подключились - создаем таблицы все подряд
                msg = "Подключение к БД проекта - успешно. Создаем таблицы.";
                logMsg(msgInfo, msg);
                query = QSqlQuery(db);
                for (int i = 0; i < fTablesCount; ++i) {
                    if (query.exec(tablesList[i][1])) {
                        msg = "Таблица " + tablesList[i][0] + " создана";
                        logMsg(msgInfo, msg);
                        qDebug() << "Таблица " << tablesList[i][0] << " создана";
                    } else {
                        err_count++;
                        msg = "Ошибка создания таблицы " + tablesList[i][0] + ":<br>" + query.lastError().text();
                        logMsg(msgFail, msg);
                        qDebug() << "Ошибка создания таблицы " << tablesList[i][0] << ":";
                        qDebug() << query.lastError().text();
                    }
                }
                // здесь нужно добавить заполнение справочников
                if(err_count > 0) {
                    qDebug() << "Есть ошибки (" << err_count << " штук)";
                }
                return;
            } else {
                // не подключились, но причина в другом; разбираться здесь не будем, просто выходим
                msg = "Ошибка подключения к БД проекта:<br>" + db.lastError().text() + "<br>Попробуйте перезапустить программу.";
                logMsg(msgError, msg);
                qDebug() << "Ошибка подключения к БД:";
                qDebug() << db.lastError().text();
                qDebug() << "Попробуйте перезапустить программу.";
                qDebug() << "Выходим.";
                return;
            }
        }

        if(!withCheckTables) return;
        // подключились штатно, проверяем наличие таблиц
        query = QSqlQuery(db);

//        qDebug() << "DB tables:";
//        foreach (QString tbl, db.tables()) {
//            qDebug() << tbl;
//        }


        for (int i = 0; i < fTablesCount; ++i) {
            if(db.tables().indexOf(tablesList[i][0]) >= 0) continue; // есть такая - идем дальше
            if (query.exec(tablesList[i][1])) {
                msg = "Таблица " + tablesList[i][0] + " создана";
                logMsg(msgInfo, msg);
                 qDebug() << "Таблица " << tablesList[i][0] << " создана";
                 //qDebug() << "Количество таблиц в БД: " << db.tables().size();
            } else {
                err_count++;
                qDebug() << "Ошибка создания таблицы " << tablesList[i][0] << ":";
                msg = "Ошибка создания таблицы " + tablesList[i][0] + ":<br>" + query.lastError().text();
                logMsg(msgFail, msg);
                qDebug() << query.lastError().text();
            }
        }
        if(err_count > 0) {
            msg = tr("Инициализация БД прошла с ошибками (количество: %1)").arg(err_count);
            logMsg(msgFail, msg);
            qDebug() << "Есть ошибки (" << err_count << " штук)";
        }
        query.finish();
    } catch (QException e) {
        msg = tr("Инициализация БД проекта - исключение:<br>").arg(e.what());
        logMsg(msgError, msg);
        qDebug() << "Инициализация БД проекта - исключение:";
        qDebug() << e.what();
        qDebug() << "DB error:";
        qDebug() << db.lastError().text();
    }
}

bool DataBase::checkForTable(QString tableName, QString tableCreateScript)
{
    bool rc = false;
    if(!db.isOpen()) {
        qDebug() << "Нет связи с БД.";
        return rc;
    }
    rc = db.tables().indexOf(tableName) >= 0;
    if(rc) return rc;

    QString db_error;
    QSqlQuery* qry = new QSqlQuery(db);
    rc = qry->exec(tableCreateScript);
    if (!rc) {
        db_error = qry->lastError().text();
        qDebug() << "Невозможно создать таблицу '" + tableName + "', ошибка: ";
        qDebug() << db_error;//db.lastError().text();
        qDebug() << "";
        qDebug() << "Script:";
        qDebug() << tableCreateScript;
    } else {
        qDebug() << "Таблица '"<<  tableName << "' создана.";
    }
    return rc;
}

