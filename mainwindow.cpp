#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QException>
#include <QKeyEvent>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QStyleFactory>
#include <QProcess>
// для использования этих модулей нужно в файл проекта добавить Qt += network
//#include <QtNetwork/QHostInfo>
//#include <QtNetwork/QHostAddress>

#include "u_duty.h"
#include "u_log.h"
//#include "childwin.h"

#if defined(Q_OS_WIN)
    //#define qUsername QString::fromLocal8Bit (qgetenv ("USERNAME").constData ()).toUtf8 ()
    #define qUsername QString::fromLocal8Bit (qgetenv ("USERNAME").constData ())
#elif defined(Q_OS_UNIX)
    #define qUsername qgetenv("USER").constData ()
#endif


//------------------------------------------------------
// тестовое сообщение - отладка в run-time
void MainWindow::fix(QString msg)
{
    return;
    QMessageBox::information(0, "Контроль запуска ТПО", msg);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
    , fDB_ok(false)
{
    // сетевое имя компьютера
    // Qt += network, #include <QtNetwork/QHostInfo>
    //QHostInfo* hi = new QHostInfo();
    //qDebug() << "localHostName:" << hi->localHostName();// astra // << ", domain name: " << hi->localDomainName();// empty // << ", address: \n";// << QHostInfo::addresses();//QHostAddress::toString();

    fix("Старт ТПО");
    ui->setupUi(this);
    //setWindowModality(Qt::WindowModal);
    setWindowIcon(QIcon(":/Config.png"));

    ui->mainToolBar->setVisible(false);
    ui->menuBar->setVisible(false);
    ui->centralWidget->setTabOrder(ui->leType, ui->leProjNum);
    ui->centralWidget->setTabOrder(ui->leProjNum, ui->leWorkNum);
    ui->centralWidget->setTabOrder(ui->leWorkNum, ui->leBoardNum);
    ui->centralWidget->setTabOrder(ui->leBoardNum, ui->leDisplacement);
    ui->centralWidget->setTabOrder(ui->leDisplacement, ui->cbxFuelType);
    ui->centralWidget->setTabOrder(ui->cbxFuelType, ui->pbnTanks);
    ui->centralWidget->setTabOrder(ui->pbnTanks, ui->pbnConsumers);
    ui->centralWidget->setTabOrder(ui->pbnConsumers, ui->pbnDensToMass);
    ui->centralWidget->setTabOrder(ui->pbnDensToMass, ui->pbnDicts);
    ui->centralWidget->setTabOrder(ui->pbnDicts, ui->pbnSave);
    ui->centralWidget->setTabOrder(ui->pbnSave, ui->pbnRestore);
    ui->leType->setFocus();

    ui->cbxStyles->addItems(QStyleFactory::keys());
    connect(ui->cbxStyles, SIGNAL(activated(QString)), this, SLOT(slotStyleChange(QString)));
    if(ui->cbxStyles->count() > 0) ui->cbxStyles->setCurrentIndex(0);


    assignLog(ui->teLog);
#if defined(Q_OS_WIN)
    logMsg(msgDate, QString("Start on OS Windows. User name: %1").arg(qUsername));
#elif defined(Q_OS_UNIX)
    logMsg(msgDate, QString("Start on OS Unix. User name: %1").arg(qUsername));
#else
    logMsg(msgDate, QString("Start on undefined OS");
#endif
//    logMsg(msgInfo, QString("User name: %1").arg(qUsername));
    logMsg(msgInfo, "SQL drivers: " + QSqlDatabase::drivers().join(" "));
//    qDebug() << QSqlDatabase::drivers( );

    //int_db();

    fix("Создание БД");
    db = new DataBase(this);
    query = new QSqlQuery();
    if(db->isOpen()) {
        updateFuelTypes();
        init_conf();
    } else fConfTableRecCount = -1;

    fix("БД создали и открыли, создаем форму справочников");
    re = new RefEdit();//this);
    //re->query = &query;
    re->hide();
    connect(re, SIGNAL(tableChanged(QString)), this, SLOT(refTableChanged(QString)));
    //connect(this, SIGNAL(allDone()), re, SLOT(closeThis()));

    fix("Справочники готовы, создаем цистерны");
    te = new TanksEdit();//this);
    fix("Фоорма цистерн создана");
    //te->setWindowIcon(QIcon(":/Retort.png"));
    //fix("Цистерны - иконоку задали");
    te->hide();
    fix("Цистерны - форму скрыли");
    te->setProjectId(fProjectId);
    fix("Цистерны - задали ИД проекта");
    connect(re, SIGNAL(tableChanged(QString)), te, SLOT(refTableChanged(QString)));

    fix("Цистерны готовы, создаем потребителей");
    ce = new ConsumersEdit();//this);
    ce->hide();
    ce->setProjectId(fProjectId);
    connect(re, SIGNAL(tableChanged(QString)), ce, SLOT(refTableChanged(QString)));

    fix("Потребители готовы, создаем таблицы пересчета (тарировки)");
    fe = new FuelTables();//this);
    fe->hide();
    connect(re, SIGNAL(tableChanged(QString)), fe, SLOT(refTableChanged(QString)));

    ne = new NetEdit();
    ne->hide();

    fix("Конец конструктора");
//    connect(db, SIGNAL())
}

MainWindow::~MainWindow()
{
    ne->close();
    //ne->deleteLater();
    ne->~NetEdit();

    disconnect(re, SIGNAL(tableChanged(QString)), fe, SLOT(refTableChanged(QString)));
    fe->close();
    //fe->deleteLater();
    fe->~FuelTables();

    disconnect(re, SIGNAL(tableChanged(QString)), ce, SLOT(refTableChanged(QString)));
    ce->close();
    //ce->deleteLater();
    ce->~ConsumersEdit();

    disconnect(re, SIGNAL(tableChanged(QString)), te, SLOT(refTableChanged(QString)));
    te->close();
    //te->deleteLater();
    te->~TanksEdit();

    disconnect(re, SIGNAL(tableChanged(QString)), this, SLOT(refTableChanged(QString)));
    re->close();
    //emit allDone();
    //re->window()->close();
    //re->deleteLater();
    //re->~RefEdit();

    closeLog();
    query->finish();
//    query = QSqlQuery();
    db->close();
    db->deleteLater();
//    db->cl
//    re->query = 0;
//    tw->fQry = 0;
//    db.close();
//    db.removeDatabase("QPSQL");
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *rze)
{
    if(ui->teLog->isVisible()) {
        ui->teLog->setGeometry(380, 10, rze->size().width() - 390, rze->size().height() - 38);
    }
}

void MainWindow::showEvent(QShowEvent *)
{
    if(ui->teLog->isVisible()) {
        ui->teLog->setGeometry(380, 10, size().width() - 390, size().height() - 38);
    }
    //qDebug() << "MainWindow Window Flags: " <<  this->windowFlags();
}

void MainWindow::init_conf()
{
    fFullUpdate = true;
    fProjectId = -1;
    fProjectName = "";

    // инициализация списка проектов (переделка со старой версии)
    // по-простому
    updateProjectsList();
//    query->exec("select * from projects order by 1;");
//    if(query->size() <= 0) {
//        // добавляем хоть одну запись
//    } else {
//        //
//    }
    fix("Старт конфигурирования проекта");
    // текущий проект
    query->exec(tr("select * from ship_def where project_id = %1;").arg(fProjectId));
    //qDebug() << "init_conf, fProjectId: " << fProjectId << ", numRowsAffected: " << query->numRowsAffected() << ", size: " << query->size();
    fConfTableRecCount = query->size();
    if (fConfTableRecCount == 0) {
        fShipProjectNum = "";
        fShipManufactureNum = "";
        fShipType = "";
        fBoardNum = 0;
        fFuelType = -1;
        fDisplacement = 0;
        ui->statusBar->showMessage("Конфигурация не задана");
    } else {
        query->first();
        fShipProjectNum = query->value("ship_project").toString();
        fShipManufactureNum = query->value("ship_man_number").toString();
        fShipType = query->value("ship_type").toString();
        fBoardNum = query->value("ship_board_num").toInt();
        fFuelType = query->value("fuel_type").toInt();
        fDisplacement = query->value("displacement").toInt();
        ui->statusBar->showMessage("Конфигурация задана", 1000);
    }
    query->finish();
    fix("Конфигурация - данные получены, отображаем");
    showCurrentData();
    fix("Завершена конфигурация");

//           // цистерны с замерами
//           int n = getDBTableRecCount(query, "tanks");
//           currState += tr(", цистерн: %1").arg(n);
//           n = getDBTableRecCount(query, "tank_measures");
//           currState += tr(", замеров: %1").arg(n);
//           // потребители
//           n = getDBTableRecCount(query, "consumers");
//           currState += tr(", потребителей: %1").arg(n);
//           // пересчет плотности по температуре
//           n = getDBTableRecCount(query, "fuel_density");
//           currState += tr(", пересчетов: %1").arg(n);
//        }
}

bool MainWindow::isDB_ok()
{
    return fDB_ok;
}

void MainWindow::setDB_ok(bool newOK)
{
    fDB_ok = newOK;
}

// очистка таблиц, с переустановкой счетчиков
void MainWindow::clearDB()
{
    // очистка БД
    // таблицы без последовательностей
    if(query->exec("delete from ship_def;")) qDebug() << "Таблица ship_def очищена, удалено записей: " << query->numRowsAffected();
    else qDebug() << "Ошибка очистки таблицы ship_def: " << query->lastError().text();
    if(query->exec("delete from consumers_spend;")) qDebug() << "Таблица consumers_spend очищена, удалено записей: " << query->numRowsAffected();
    else qDebug() << "Ошибка очистки таблицы consumers_spend: " << query->lastError().text();


    // итератор в стиле С++
    QList<QString> tblListCPP;
    tblListCPP << "sensors" << "consumers" << "consumer_types"
               << "tanks_fuel_rest" << "tank_measures" << "fuel_measures" << "check_points_rest" << "check_points"
               << "fuel_density" << "tanks" << "tank_types" << "meter_types" << "run_mode_types"
               << "fuel_types" << "sensor_types";
    foreach (QString tblName, tblListCPP) {
        if(query->exec(tr("delete from %1;").arg(tblName))) {
            qDebug() << "Таблица " << tblName <<  " очищена, удалено записей: " << query->numRowsAffected();
            if(tblName != "tank_measures") // обнуляем счетчик
                query->exec(tr("select setval('%1_id_seq', 1, false);").arg(tblName));
            query->finish();
        }
        else qDebug() << "Ошибка очистки таблицы " << tblName << ": " << query->lastError().text();
    }
    return;

    // итератор в стиле STL
    QVector<QString> vecTblList;
    vecTblList << "ship_def" << "consumers_spend" << "sensors" << "consumers" << "consumer_types"
               << "tanks_fuel_rest" << "tank_measures" << "fuel_measures" << "check_points_rest" << "check_points"
               << "fuel_density" << "tanks" << "tank_types" << "meter_types" << "run_mode_types"
               << "fuel_types" << "sensor_types";
    QVector<QString>::iterator vecTbl;// = vecTblList.begin();
    for(vecTbl = vecTblList.begin(); vecTbl != vecTblList.end(); ++vecTbl) {
        QString tblName = *vecTbl;
        if(query->exec(tr("delete from %1;").arg(tblName))) {
            qDebug() << "Таблица " << tblName <<  " очищена, удалено записей: " << query->numRowsAffected();
            if(tblName != "tank_measures") // обнуляем счетчик
                query->exec(tr("select setval('%1_id_seq', 1, false);").arg(tblName));
            query->finish();
        }
        else qDebug() << "Ошибка очистки таблицы " << tblName << ": " << query->lastError().text();
    }
    return;

    // итератор в стиле Ява
    QList<QString> tblListJava;
    tblListJava << "ship_def" << "consumers_spend" << "sensors" << "consumers" << "consumer_types"
                << "tanks_fuel_rest" << "tank_measures" << "fuel_measures" << "check_points_rest" << "check_points"
                << "fuel_density" << "tanks" << "tank_types" << "meter_types" << "run_mode_types"
                << "fuel_types" << "sensor_types";

    QListIterator<QString> tbl(tblListJava);
    while (tbl.hasNext()) {
        QString tblName = tbl.next();
        if(query->exec(tr("delete from %1;").arg(tblName))) {
            qDebug() << "Таблица " << tblName <<  " очищена, удалено записей: " << query->numRowsAffected();
            if(tblName != "tank_measures") // обнуляем счетчик
                query->exec(tr("select setval('%1_id_seq', 1, false);").arg(tblName));
            query->finish();
        }
        else qDebug() << "Ошибка очистки таблицы " << tblName << ": " << query->lastError().text();
    }
    return;

    // "деревянный" вариант
    if(query->exec("delete from consumers;")) {
        qDebug() << "Таблица consumers очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('consumers_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы consumers: " << query->lastError().text();
    if(query->exec("delete from consumer_types;")) {
        qDebug() << "Таблица consumer_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('consumer_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы consumer_types: " << query->lastError().text();
    if(query->exec("delete from tanks_fuel_rest;")) {
        qDebug() << "Таблица tanks_fuel_rest очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('tanks_fuel_rest_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы tanks_fuel_rest: " << query->lastError().text();
    if(query->exec("delete from tank_measures;")) {
        qDebug() << "Таблица tank_measures очищена, удалено записей: " << query->numRowsAffected();
        //query->exec("select setval('tank_measures_id_seq', 1, false);"); // нет такой последовательности
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы tank_measures: " << query->lastError().text();
    if(query->exec("delete from fuel_measures;")) {
        qDebug() << "Таблица fuel_measures очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('fuel_measures_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы fuel_measures: " << query->lastError().text();
    if(query->exec("delete from check_points_rest;")) {
        qDebug() << "Таблица check_points_rest очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('check_points_rest_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы check_points_rest: " << query->lastError().text();
    if(query->exec("delete from check_points;")) {
        qDebug() << "Таблица check_points очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('check_points_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы check_points: " << query->lastError().text();
    if(query->exec("delete from fuel_density;")) {
        qDebug() << "Таблица fuel_density очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval(fuel_density'_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы fuel_density: " << query->lastError().text();
    if(query->exec("delete from tanks;")) {
        qDebug() << "Таблица tanks очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('tanks_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы tanks: " << query->lastError().text();
    if(query->exec("delete from tank_types;")) {
        qDebug() << "Таблица tank_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('tank_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы tank_types: " << query->lastError().text();
    if(query->exec("delete from meter_types;")) {
        qDebug() << "Таблица meter_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('meter_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы meter_types: " << query->lastError().text();
    if(query->exec("delete from run_mode_types;")) {
        qDebug() << "Таблица run_mode_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('run_mode_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы run_mode_types: " << query->lastError().text();
    if(query->exec("delete from fuel_types;")) {
        qDebug() << "Таблица fuel_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('fuel_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы fuel_types: " << query->lastError().text();
    if(query->exec("delete from sensor_types;")) {
        qDebug() << "Таблица sensor_types очищена, удалено записей: " << query->numRowsAffected();
        query->exec("select setval('sensor_types_id_seq', 1, false);");
        query->finish();
    }
    else qDebug() << "Ошибка очистки таблицы sensor_types: " << query->lastError().text();
}

// переустановка авто-счетчиков по количеству записей в соотв.таблице
void MainWindow::setSequenceValues()
{
    // select count(*) as rec_count, coalesce(max(id), 0) as max_id from check_points;
    // query->exec(tr("select setval('%1_id_seq', %2, false);").arg(tblName).arg(c));

//    QList<QString> tblListCPP;
//    tblListCPP << "consumers" << "consumer_types" << "tanks_fuel_rest" << "fuel_measures"
//                << "check_points_rest" << "check_points" << "fuel_density" << "tanks" << "tank_types"
//                << "meter_types" << "run_mode_types" << "fuel_types" << "sensor_types";
//    foreach (QString tblName, tblListCPP) {
//        if(query->exec(tr("select count(*) as rec_count, coalesce(max(id), 0) as max_id from %1;").arg(tblName))) {
//            query->next();
//            int n = query->value("rec_count").toInt();
//            if(n > 0) {
//                int c = query->value("max_id").toInt();
//                query->finish();
//                if(query->exec(tr("select setval('%1_id_seq', %2, true);").arg(tblName).arg(c))) {
//                    query->finish();
//                    qDebug() << "Таблица " << tblName <<  ", счетчик установлен в значение: " << c;
//                }
//                else qDebug() << "Ошибка поправки счетчика таблицы " << tblName << ": " << query->lastError().text();
//            }
//        }
//        else qDebug() << "Ошибка попытки поправки счетчика таблицы " << tblName << ": " << query->lastError().text();
//    }
    query->finish();
    if(query->exec("SELECT table_name, column_name, column_default FROM information_schema.columns WHERE table_schema = 'public' AND column_default like 'nextval%';")) {
        QSqlQuery* qry = new QSqlQuery();
        QString tableName, fieldName, seq_str, seqName;
        QStringList qryParts;
        while(query->next()) {
            tableName = query->value("table_name").toString();
            fieldName = query->value("column_name").toString();
            seq_str = query->value("column_default").toString();
            qryParts = seq_str.split("'");
            seqName = qryParts.at(1);
            //qDebug() << "next sequence: " << seqName << ", table: " << tableName << ", field: " << fieldName;
            //if(qry->exec(tr("with a as (SELECT MAX(%1) as max_id FROM %2) select a.max_id, ts.last_value, ts.increment_by FROM a, %3 ts")
            //                .arg(fieldName)
            //                .arg(tableName)
            //                .arg(seqName))) {
            QString qrySt = tr("with a as (SELECT MAX(%1) as max_id FROM %2) select a.max_id, ts.last_value, ts.increment_by FROM a, %3 ts;")
                    .arg(fieldName)
                    .arg(tableName)
                    .arg(seqName);
            //qDebug() << "sequence search query: " << qrySt;

            if(qry->exec(qrySt)) {
                qry->first();
                int max_id = qry->value("max_id").toInt();
                int last_val = qry->value("last_value").toInt();
                //qDebug() << "sequence: " << seqName << ", last value: " << last_val << ", max ID: " << max_id;
                if(max_id > last_val) {
                    qry->finish();
                    qrySt = tr("select setval('%1', %2, true) as last_value;").arg(seqName).arg(max_id);
                    //qDebug() << "sequence update query: " << qrySt;
                    if(qry->exec(qrySt)) {
                        qry->first();
                        last_val = qry->value("last_value").toInt();
                        logMsg(msgSuccess, tr("Последовательность %1 обновлена, last_value: %2").arg(seqName).arg(last_val), ui->teLog);
                        //qDebug() << "Последовательность " << seqName << " обновлена, last_value: " << last_val;
                    } else {
                        // error sequence update
                        logMsg(msgError, tr("Ошибка обновления последовательности %1:<br>%2").arg(seqName).arg(qry->lastError().text()), ui->teLog);
                        qDebug() << "Ошибка обновления последовательности " << seqName << ":";
                        qDebug() << qry->lastError().text();
                    }
                    qry->finish();
                }
            } else {
                // error sequence search
                logMsg(msgError, tr("Ошибка определения последовательности %1:<br>%2").arg(seqName).arg(qry->lastError().text()), ui->teLog);
                qDebug() << "Ошибка определения последовательности " << seqName << ":";
                qDebug() << qry->lastError().text();
            }
        }
    }
    query->finish();
}

void MainWindow::updateProjectsList()
{
    int cbxCurrent = ui->cbxProject->currentIndex();

    disconnect(ui->cbxProject, SIGNAL(currentIndexChanged(int)), this, SLOT(projIdChanged(int)));
    ui->cbxProject->clear();
    projIdList.clear();
    // список проектов
    if(!query->exec("select * from projects order by 1;")) { // ошибка получения списка
        logMsg(msgError, tr("Ошибка выполнения запроса:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка выполнения запроса:";
        qDebug() << query->lastError().text();
        return;
    }
    //else { qDebug() << "query exec succefull, lastError: " << query->lastError().text().trimmed(); }
    if(query->size() == 0) { // список пуст
        query->finish();
        // добавляем дежурную запись - без этого нельзя
        if(query->exec("insert into projects(name, description) values('Основной проект', 'Запись создана автоматически и должна быть изменена')")) {
            query->finish();
            fProjectName = "Основной проект";
            ui->cbxProject->addItem(fProjectName);
            fProjectId = 1;

            projIdList.append(1);
            fProjectName = "Основной проект";
            ui->cbxProject->setCurrentIndex(0);
        } else {
            logMsg(msgError, tr("Ошибка выполнения запроса:<br>%1").arg(query->lastError().text()));
            qDebug() << "Ошибка выполнения запроса:";
            qDebug() << query->lastError().text();
        }
        return;
    }

    // список проектов получен и не пуст
    while (query->next()) {
        ui->cbxProject->addItem(query->value("name").toString());
        projIdList.append(query->value("id").toInt());
    }
    query->finish();
    if((cbxCurrent >= 0) && (cbxCurrent < ui->cbxProject->count()))
        ui->cbxProject->setCurrentIndex(cbxCurrent);
    else
        ui->cbxProject->setCurrentIndex(0);
    fProjectId = projIdList.at(ui->cbxProject->currentIndex());
    fProjectName = ui->cbxProject->itemText(ui->cbxProject->currentIndex());
    //qDebug() << "updateProjectsList, fProjectId: " << fProjectId;
    connect(ui->cbxProject, SIGNAL(currentIndexChanged(int)), this, SLOT(projIdChanged(int)));
}

void MainWindow::updateFuelTypes()
{
    if(!query->exec("select * from fuel_types order by 1;")) {
        logMsg(msgError, tr("Ошибка выполнения запроса:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка выполнения запроса:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список типов топлива пуст");
        qDebug() << "Ошибка - список типов топлива пуст";
        return;
    }

    int cbxCurrent = ui->cbxFuelType->currentIndex();
    ui->cbxFuelType->clear();
    while (query->next()) {
        int idVal = query->value("id").toInt();
        ui->cbxFuelType->addItem(query->value("name").toString(), idVal);// , query->value("id").toInt()); // <- так не берет, нужна переменная
    }
    query->finish();

//    for (int i = 0; i < ui->cbxFuelType->count(); ++i) {
//        qDebug() << "fuel types cbx, item " << i << " data - user role: " << ui->cbxFuelType->itemData(i) << ", display role: " << ui->cbxFuelType->itemData(i, Qt::DisplayRole);
//    }
    if((cbxCurrent >= 0) & (cbxCurrent < ui->cbxFuelType->count()))
        ui->cbxFuelType->setCurrentIndex(cbxCurrent);
    else
        ui->cbxFuelType->setCurrentIndex(0);
}

void MainWindow::updateConfData()
{
    query->exec(tr("select * from ship_def where project_id = %1;").arg(fProjectId));
    fConfTableRecCount = query->size();
    if (fConfTableRecCount == 0) {
        fShipProjectNum = "";
        fShipManufactureNum = "";
        fShipType = "";
        fBoardNum = 0;
        fFuelType = -1;
        fDisplacement = 0;
        ui->statusBar->showMessage("Конфигурация не задана");
    } else {
        query->first();
        fShipProjectNum = query->value("ship_project").toString();
        fShipManufactureNum = query->value("ship_man_number").toString();
        fShipType = query->value("ship_type").toString();
        fBoardNum = query->value("ship_board_num").toInt();
        fFuelType = query->value("fuel_type").toInt();
        fDisplacement = query->value("displacement").toInt();
        ui->statusBar->showMessage("Конфигурация задана", 1000);
    }
    query->finish();
    showCurrentData();
}

void MainWindow::refTableChanged(QString refTableName)
{
    if(refTableName == "projects") updateProjectsList();
    else
        if(refTableName == "fuel_types") updateFuelTypes();
}

void MainWindow::projIdChanged(int newProjId)
{
    fProjectId = projIdList.at(newProjId);
    fProjectName = ui->cbxProject->itemText(newProjId);
    updateConfData();
    te->setProjectId(fProjectId);
    ce->setProjectId(fProjectId);
}

void MainWindow::showCurrentData() // вывод текущих данных
{
    fFullUpdate = true;
    ui->leProjNum->setText(fShipProjectNum);
    ui->leWorkNum->setText(fShipManufactureNum);
    ui->leType->setText(fShipType);
    ui->leBoardNum->setText(tr("%1").arg(fBoardNum));
    ui->leDisplacement->setText(tr("%1").arg(fDisplacement));
    //ui->cbxFuelType->setCurrentIndex(fFuelType);
    for (int i = 0; i <ui->cbxFuelType->count(); ++i) {
        if(ui->cbxFuelType->itemData(i) == fFuelType) {
            ui->cbxFuelType->setCurrentIndex(i);
            break;
        }
    }

    fFullUpdate = false;
    checkCorrectData();
}

//-------------------------------------------------------------------------------------------------------------
// проверка корректности данных
void MainWindow::checkCorrectData()
{
    if(fFullUpdate) return;
    //qDebug() << "Configuration - checkCorrectData";
    QString res = "";
    QString st = ui->leProjNum->text().trimmed();

    st = ui->leType->text().trimmed();
    bool fShipTypeCorrect = st != "";
    bool fShipTypeChanged = st != fShipType;
    if(!fShipTypeCorrect) res += "Тип судна задан НЕ корректно<br>";
    //if(!fShipTypeChanged) res += "Тип судна не изменился<br>";

    bool fShipProjNumCorrect = st != "";
    bool fShipProjNumChanged = st != fShipProjectNum;
    if(!fShipProjNumCorrect) res += "Наименование проекта задано НЕ корректно (пусто)<br>";
    //if(!fShipProjNumChanged) res += "Наименование проекта не изменилось<br>";

    st = ui->leWorkNum->text().trimmed();
    bool fShipManufactureNumCorrect = st != "";
    bool fShipManufactureNumChanged = st != fShipManufactureNum;
    if(!fShipManufactureNumCorrect) res += "Заводской номер задан НЕ корректно<br>";
    //if(!fShipManufactureNumChanged) res += "Заводской номер не изменился<br>";

    int newVal = 0;
    bool fShipDisplacementCorrect = try_str_to_int(ui->leDisplacement->text(), newVal) and (newVal > 0);
    bool fShipDisplacementChanged = newVal != fDisplacement;
    if(!fShipDisplacementCorrect) res += "Водоизмещение задано НЕ корректно<br>";
    //if(!fShipDisplacementChanged) res += "Водоизмещение не изменилось<br>";

    bool fShipBoardNumCorrect = try_str_to_int(ui->leBoardNum->text(), newVal) and (newVal > 0);
    bool fShipBoardNumChanged = newVal != fBoardNum;
    if(!fShipBoardNumCorrect) res += "Бортовой номер задан НЕ корректно<br>";
    //if(!fShipBoardNumChanged) res += "Бортовой номер не изменился<br>";

    newVal = ui->cbxFuelType->currentIndex();
    bool fShipFuelTypeCorrect = newVal >= 0;
    bool fShipFuelTypeChanged = newVal != fFuelType;
    if(!fShipFuelTypeCorrect) res += "Тип топлива задан НЕ корректно<br>";
    //if(!fShipFuelTypeChanged) res += "Тип топлива не изменился<br>";

    bool dataCorrect = fShipProjNumCorrect & fShipManufactureNumCorrect & fShipTypeCorrect & fShipDisplacementCorrect & fShipBoardNumCorrect & fShipFuelTypeCorrect;
    bool dataChanged = fShipProjNumChanged or fShipManufactureNumChanged or fShipTypeChanged or fShipDisplacementChanged or fShipBoardNumChanged or fShipFuelTypeChanged;
    if(!dataCorrect) res += "Данные заданы не корректно<br>";
    //if(!dataChanged) res += "Данные не изменились<br>";

    ui->pbnSave->setEnabled(dataCorrect);// and dataChanged);
    //ui->pbnRestore->setEnabled((fConfTableRecCount > 0) and dataChanged);
    ui->pbnRestore->setEnabled(fConfTableRecCount > 0);
    if(res.trimmed() != "") {
        ui->pbnSave->setToolTip(res);
        //ui->pbnRestore->setToolTip(res);
    } else {
        ui->pbnSave->setToolTip("Сохранить текущие значения");
    }

    // добавить проверку размеров справочников и возможность редактирования цистерн и потребителей
    return;
    qDebug() << dataChanged;
}

void MainWindow::saveDataToFile()
{
    //return; // переделать под множество проектов

    // было сделано по-человечески, но под UTF8 (по умолчанию)
    // потребовался KOI8-R
    // переделал "на живую нитку" ( fConfig->write(*) => ba.append(*) , find & replace )
    // по-хорошему, надо переписать
    // будет время - как только, так сразу.

    //QTextCodec * localCodec = QTextCodec::codecForLocale();
    //qDebug() << "localCodec->name(): " << localCodec->name();

    if(!query->driver()->isOpen()) return;
    QString strFilter;
    QString saveFileName = QFileDialog::getSaveFileName(0,
                                                        tr("Сохранить конфигурацию в файле"), // заголовок окна
                                                        "oprichik.cfg", // каталог/файл по умолчанию
                                                        "*.cfg", // маска расширений
                                                        &strFilter, // ???
                                                        QFileDialog::DontConfirmOverwrite
                                                        );
    if(saveFileName.isEmpty()) return;
    //qDebug() << "Файл для сохранения конфигурации: " << saveFileName;
    QFile fConfig(saveFileName);
    if(fConfig.exists()) {// такой файл есть
        QMessageBox* mb = new QMessageBox(QMessageBox::Question,
                                         "Перезапись", // заголовок диалога
                                         "Такой файл уже существует.<br>Перезаписать ?", // сообщение в поле диалога
                                         QMessageBox::Yes | QMessageBox::No
                                         );
        int rc = mb->exec();
        if(rc == QMessageBox::No) return;
    }

    if(!fConfig.open(QIODevice::WriteOnly)) {
            logMsg(msgError, tr("Ошибка открытия (создания) файла '%1':<br>%2").arg(saveFileName).arg(fConfig.errorString()));
            return;
    }

    QByteArray ba; // буфер для записи в файл

    // основная конфигурация - текущая (брать сохраненную, или текущую?)
    ba.append("[Конфигурация]\r\n");
    ba.append(QByteArray("Тип: ").append(ui->leType->text()).append("\r\n"));
    ba.append(QByteArray("Номер проекта: ").append(ui->leProjNum->text()).append("\r\n"));
    ba.append(QByteArray("Заводской номер: ").append(ui->leWorkNum->text()).append("\r\n"));
    ba.append(QByteArray("Бортовой номер: ").append(ui->leBoardNum->text()).append("\r\n"));
    ba.append(QByteArray("Водоизмещение: ").append(ui->leDisplacement->text()).append("\r\n"));

    // поправить
    ba.append(QByteArray("Тип топлива: ").append(QString::number(ui->cbxFuelType->currentIndex())).append("\r\n"));
    //int n = 0;
    QString currProjId = tr("%1").arg(fProjectId);
    if(query->exec(tr("select count(*) as tank_count from tanks where project_id = %1;").arg(currProjId))) {
        query->first();
        //ba.append(QByteArray("Количество цистерн: ").append(query->value("tank_count").toString()).append("\r\n"));
        ba.append(QByteArray("Количество цистерн: ").append(query->value("tank_count").toByteArray()).append("\r\n"));
    }
    if(query->exec(tr("select count(*) as consumer_count from consumers where project_id = %1;").arg(currProjId))) {
        query->first();
        //ba.append(QByteArray("Количество потребителей: ").append(query->value("consumer_count").toString()).append("\r\n"));
        ba.append(QByteArray("Количество потребителей: ").append(query->value("consumer_count").toByteArray()).append("\r\n"));
    }
    if(query->exec(tr("select count(*) as sensor_count from sensors where project_id = %1;").arg(currProjId))) {
        query->first();
        //ba.append(QByteArray("Количество датчиков: ").append(query->value("sensor_count").toString()).append("\r\n"));
        ba.append(QByteArray("Количество датчиков: ").append(query->value("sensor_count").toByteArray()).append("\r\n"));
    }
    ba.append(QByteArray("\r\n"));

    // справочники
    // пере-индексация:
    // в файл - пишем нумерацию по-порядку (1, 2, 3,...)
    // реальные индексы - сохраняем,
    // и ссылки на элементы в рабочих таблицах - пишем с нумерованными индексами, а не реальными

    // типы цистерн
    QList<int> idxTankTypes;
    int i = 0;
    if(!query->exec("select * from tank_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка типов цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка типов цистерн:";
        qDebug() << query->lastError().text();
    } else {
        ba.append(QByteArray("[Типы цистерн]\r\n"));
        while (query->next()) {
            idxTankTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы топлива
    i = 0;
    QList<int> idxFuelTypes;
    if(!query->exec("select * from fuel_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка типов топлива:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка типов топлива:";
        qDebug() << query->lastError().text();
    } else {
        ba.append(QByteArray("[Типы топлива]\r\n"));
        while (query->next()) {
            idxFuelTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы измерителей (датчиков)
    QList<int> idxSensorTypes;
    if(!query->exec("select * from sensor_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка типов измерителей (датчиков):<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка типов измерителей (датчиков):";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Типы измерителей (датчиков)]\r\n"));
        while (query->next()) {
            idxSensorTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы потребителей
    QList<int> idxConsumerTypes;
    if(!query->exec("select * from consumer_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка типов потребителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка типов потребителей:";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Типы потребителей]\r\n"));
        while (query->next()) {
            idxConsumerTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы измерителей
    QList<int> idxMeterTypes;
    if(!query->exec("select * from meter_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка типов измерителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка типов измерителей:";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Типы измерителей]\r\n"));
        while (query->next()) {
            idxMeterTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы режимов работы
    QList<int> idxRunModeTypes;
    if(!query->exec("select * from run_mode_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка режимов работы:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка режимов работы:";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Типы режимов работы]\r\n"));
        while (query->next()) {
            idxRunModeTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    // типы событий
    //QList<int> idxRunModeTypes;
    if(!query->exec("select * from event_types order by 1;")) {
        logMsg(msgError, tr("Ошибка получения списка событий:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка событий:";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Типы событий]\r\n"));
        while (query->next()) {
            //idxRunModeTypes.append(query->value("id").toInt());
            ba.append(QByteArray().append(tr("%1=%2 (%3)\r\n").arg(++i).arg(query->value("name").toString()).arg(query->value("description").toString())));
        }
        query->finish();
        ba.append(QByteArray("\r\n"));
    }

    //=================================================================================
    // описание цистерн
    // tr("select count(*) as tank_count from tanks where project_id = %1;").arg(currProjId))
    QSqlQuery* qryData = new QSqlQuery();
    int n;
    if(!query->exec(tr("select * from tanks where project_id = %1 order by 1;").arg(currProjId))) {
        logMsg(msgError, tr("Ошибка получения списка цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка цистерн:";
        qDebug() << query->lastError().text();
    } else {
        i = 0;
        ba.append(QByteArray("[Цистерны]\r\n"));
        while (query->next()) {
            i++;
            //qDebug() << "tank " << i << "-Latin1: " << QString(query->value("name").toString().toLatin1())
            //         << ", Local8Bit: " << QString(query->value("name").toString().toLocal8Bit())
            //         << ", string:" << QString(query->value("name").toString());
            ba.append(QByteArray().append(tr("[Цистерна %1: %2]\r\n").arg(i).arg(QString(query->value("name").toString().toLocal8Bit()))));
            n = idxTankTypes.indexOf(query->value("tank_type").toInt()) + 1;
            ba.append(QByteArray().append(tr("Тип цистерны: %1\r\n").arg(n)));
            n = idxSensorTypes.indexOf(query->value("sensor_type").toInt()) + 1;
            ba.append(QByteArray().append(tr("Тип измерителя: %1\r\n").arg(n)));
            // SELECT tank_id, measure_id, v_high, v_volume FROM public.tank_measures;
            if(qryData->exec(tr("select measure_id, v_high, v_volume from tank_measures where tank_id = %1 order by 1;").arg(query->value("id").toInt()))) {
                n = qryData->size();
                ba.append(QByteArray().append(tr("Количество замеров: %1\r\n").arg(n)));
                while (qryData->next()) {
                    ba.append(QByteArray().append(tr("\t%1=%2\r\n")
                                                      .arg(QString::number(qryData->value("v_high").toFloat(), 'f', 3))
                                                      .arg(QString::number(qryData->value("v_volume").toFloat(), 'f', 3))
                                                      ));
                }
            }
            qryData->finish();
            ba.append(QByteArray("\r\n"));
        }
        query->finish();
        //ba.append(QByteArray("\r\n"));
    }

    // описание потребителей
    if(!query->exec(tr("select * from consumers where project_id = %1 order by 1;").arg(currProjId))) {
        logMsg(msgError, tr("Ошибка получения списка потребителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка потребителей:";
        qDebug() << query->lastError().text();
    } else {
        // SELECT id, consumer_type, meter_type, name FROM public.consumers;
        i = 0;
        ba.append(QByteArray("[Потребители]\r\n"));
        while (query->next()) {
            i++;
            ba.append(QByteArray().append(tr("[Потребитель %1: %2]\r\n").arg(i).arg(query->value("name").toString())));
            n = idxConsumerTypes.indexOf(query->value("consumer_type").toInt()) + 1;
            ba.append(QByteArray().append(tr("Тип потребителя: %1\r\n").arg(n)));
            n = idxMeterTypes.indexOf(query->value("meter_type").toInt()) + 1;
            ba.append(QByteArray().append(tr("Тип измерителя: %1\r\n").arg(n)));
            if(qryData->exec(tr("select run_mode_id, spend from consumers_spend where consumer_id = %1 order by 1;").arg(query->value("id").toInt()))) {
                n = qryData->size();
                ba.append(QByteArray().append(tr("Количество режимов работы: %1\r\n").arg(n)));
                while (qryData->next()) {
                    n = idxRunModeTypes.indexOf(qryData->value("run_mode_id").toInt()) + 1;
                    ba.append(QByteArray().append(tr("\t%1=%2\r\n")
                                                      .arg(QString::number(n))
                                                      .arg(QString::number(qryData->value("spend").toFloat(), 'f', 3))
                                                      ));
                }
            }
            qryData->finish();
            ba.append(QByteArray("\r\n"));
        }
        query->finish();
        //ba.append(QByteArray("\r\n"));
    }

    // пересчет значений плотности по типам топлива
    if(!query->exec("select distinct fuel_type_id, name from fuel_density, fuel_types where fuel_types.id = fuel_density.fuel_type_id order by 1;")) {
        logMsg(msgError, tr("Ошибка получения данных пересчета плотности:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения данных пересчета плотности:";
        qDebug() << query->lastError().text();
    } else {
        int fuelTypesCount = query->size();
        ba.append(QByteArray("[Пересчет значений плотности]\r\n"));
        ba.append(tr("Типов топлива: %1\r\n").arg(fuelTypesCount));
        while(query->next()) {
            int idxFuelTypeId = query->value("fuel_type_id").toInt();
            int idxFuelType = idxFuelTypes.indexOf(idxFuelTypeId) + 1;
            QString fuelName = query->value("name").toString();
            ba.append(tr("[Тип топлива %1: %2]\r\n").arg(idxFuelType).arg(fuelName));
            QString qry = tr("select fuel_temp, fuel_density from fuel_density where fuel_type_id = %1 order by fuel_temp;").arg(idxFuelTypeId);
            if(!qryData->exec(qry)) {
                logMsg(msgError, tr("Ошибка получения данных пересчета плотности для топлива %1:<br>%2").arg(fuelName).arg(query->lastError().text()), ui->teLog);
                qDebug() << "Ошибка получения данных пересчета плотности топлива " << fuelName << ":";
                qDebug() << query->lastError().text();
            } else {
                while (qryData->next()) {
                    ba.append(QByteArray().append(tr("\t%1=%2\r\n")
                                                      .arg(QString::number(qryData->value("fuel_temp").toFloat(), 'f', 3))
                                                      .arg(QString::number(qryData->value("fuel_density").toFloat(), 'f', 3))
                                                      ));
                }
                qryData->finish();
                ba.append(QByteArray("\r\n"));
            }
        }
    }

    // датчики (измерители)
    // пока без таблиц пересчета
    // SELECT id, sensor_type, coalesce(tank_id, -1) as tank_id, coalesce(consumer_id, -1) as consumer_id, net_address, net_idx, data_type, lo_val, high_val FROM sensors WHERE project_id=%1 ORDER BY id;
    if(!query->exec(tr("select * from sensors where project_id = %1 order by 1;").arg(currProjId))) {
        logMsg(msgError, tr("Ошибка получения списка датчиков:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения  списка датчиков:";
        qDebug() << query->lastError().text();
    } else { // не готово - закрываем до лучших времен
        /*
        // SELECT id, sensor_type, tank_id, consumer_id, net_address, net_idx, data_type, lo_val, high_val FROM sensors;
        i = 0;
        ba.append(QByteArray("[Датчики]\r\n"));
        while (query->next()) {
            i++;
            ba.append(QByteArray().append(tr("[Датчик %1]\r\n").arg(i)));
            n = idxSensorTypes.indexOf(query->value("sensor_type").toInt()) + 1;
            ba.append(QByteArray().append(tr("Тип датчика: %1\r\n").arg(n)));
            if(query->value("consumer_id").isNull()) { // датчик на танке
                n = query->value("tank_id").toInt();
                ba.append(QByteArray().append(tr("Цистерна: %1\r\n").arg(n)));
            } else { // датчик на потребителе
                n = query->value("consumer_id").toInt();
                ba.append(QByteArray().append(tr("Потребитель: %1\r\n").arg(n)));
            }
            n = query->value("net_address").toInt();
            ba.append(QByteArray().append(tr("Сетевой адрес: %1\r\n").arg(n)));
            n = query->value("net_idx").toInt();
            ba.append(QByteArray().append(tr("Индекс датчика: %1\r\n").arg(n)));
            n = query->value("data_type").toInt();
            ba.append(QByteArray().append(tr("Тип данных: %1\r\n").arg(n)));
            QString val = QString::number(query->value("lo_val").toFloat(), 'f', 3);
            ba.append(QByteArray().append(tr("Мин.значение: %1\r\n").arg(val)));
            val = QString::number(query->value("high_val").toFloat(), 'f', 3);
            ba.append(QByteArray().append(tr("Макс.значение: %1\r\n").arg(val)));
            // таблица пересчета по датчику - аналогично режимам работы по потребителю, но будет позжзе
            qryData->finish();
            ba.append(QByteArray("\r\n"));
        }
        query->finish();
        //ba.append(QByteArray("\r\n"));
        */
    }

    // без перекодировки
    //fConfig.write(ba);

    // перекодировка
    QTextCodec* codec = QTextCodec::codecForName("KOI8-R");// CP866 (DOS), cp1251, "Windows-1251" (Win),
    QByteArray fConfigBA = codec->fromUnicode(ba);
    fConfig.write(fConfigBA);

    fConfig.flush();
    fConfig.close();
    logMsg(msgInfo, tr("Конфигурация проекта '%1' сохранена в файле: %2").arg(fProjectName).arg(saveFileName), ui->teLog);
}

void MainWindow::restoreDataFromFile()
{
    //return; // переделать под множество проектов
    // пока что сделано однократно, под восстановление данных из единственного файла конфигурации

    if(!query->driver()->isOpen()) return;
    QString strFilter;
    QString loadFileName = QFileDialog::getOpenFileName(0,
                                                        tr("Загрузить конфигурацию из файла"), // заголовок окна
                                                        "oprichik.cfg", // каталог/файл по умолчанию
                                                        "*.cfg", // маска расширений
                                                        &strFilter // ???
                                                        );
    if(loadFileName.isEmpty()) return;
    //qDebug() << "Файл для загрузки конфигурации: " << loadFileName;
//    QStringList fConf = QStringList().op
    QFile fConfig(loadFileName);
    if(!fConfig.open(QIODevice::ReadOnly)) {
            logMsg(msgError, tr("Ошибка открытия файла '%1':<br>%2").arg(loadFileName).arg(fConfig.errorString()));
            return;
    }

    QByteArray ba = fConfig.readAll();
    fConfig.close();
    //qDebug() << "Config file size (bytes): " << ba.size();


    //QString st_ba(ba); // без перекодировки

    // перекодировка
    QTextCodec* codec = QTextCodec::codecForName("KOI8-R");
    QString st_ba(codec->toUnicode(ba));

    QStringList confData = st_ba.split("\r\n");//(ba);
    int linesCount = confData.size();
    //qDebug() << "Config file - size (lines): " << linesCount;// << ", lines: " << confData.count();

    // тест результатов перекодировки (здесь можно сделать авто-определение кодировки: 1-я строка - [Конфигурация])
//    for (int i = 0; i < 3; ++i) {
//        ui->teLog->append(confData.at(i));
//    }

    int currLine = 0;
    QString st, st1;
    QString qry;
    QStringList partHeaders = QStringList() << "Конфигурация" << "Типы" << "Цистерны" << "Потребители" << "Пересчет";// датчики - будут позже
    QStringList confMainData = QStringList() << "Тип" << "Номер проекта" << "Заводской номер" << "Бортовой номер" << "Водоизмещение" << "Тип топлива" << "Количество цистерн" << "Количество потребителей";
    QStringList referenceTypes = QStringList() << "цистерн" << "измерителей" << "потребителей" << "режимов" << "топлива" << "событий" << "данных";
    QStringList stParts;

    QString projType = "";
    QString projNum = "";
    QString projWorkNum = "";
    QString projBoardNum = "";
    QString projDisplacement = "";
    QString projFuelType = "";

    int projDisp = 0;
    int projBoardNumber = 0;
    int projFuelTypeIdx = 0;
    int projTanksCount = 0;
    int projConsumersCount = 0;


    bool skipWriteToDB = false;//true; // отладка - писать в БД, или нет

    if(!skipWriteToDB) {
        // очистка БД
        // однократно - заполнение БД из файла
        clearDB();
    }

    // основная конфигурация
    while (currLine < linesCount) {
        st = QString(confData.at(currLine)).trimmed();
        if(st.isEmpty()) {
            currLine++;
            continue;
        }
        if(st.startsWith('[')){
            st = st.mid(1, st.length() - 2); // убрали скобки
            //qDebug() << "Header: " << st;
            stParts = st.split(" "); // разбили на слова по пробелам
            st1 = stParts.at(0); // первое слово заголовка раздела
            //qDebug() << "\tWord 1: " << st1;
            int partIdx = partHeaders.indexOf(st1);

            switch (partIdx) {

            case 0: //"Конфигурация":
            {
                currLine++;
                st = QString(confData.at(currLine)).trimmed();
                while(!st.isEmpty()) {
                    stParts = st.split(":");
                    st1 = stParts.at(0);
                    partIdx = confMainData.indexOf(st1);
                    switch (partIdx) {
                    case 0: // "Тип"
                        projType = QString(stParts.at(1)).trimmed();
                        break;
                    case 1: // "Номер проекта"
                        projNum = QString(stParts.at(1)).trimmed();
                        break;
                    case 2: // "Заводской номер"
                        projWorkNum = QString(stParts.at(1)).trimmed();
                        break;
                    case 3: // "Бортовой номер"
                        projBoardNum = QString(stParts.at(1)).trimmed();
                        projBoardNumber = projBoardNum.toInt();
                        break;
                    case 4: // "Водоизмещение"
                        projDisplacement = QString(stParts.at(1)).trimmed();
                        projDisp = projDisplacement.toInt();
                        break;
                    case 5: // "Тип топлива"
                        projFuelType = QString(stParts.at(1)).trimmed();
                        projFuelTypeIdx = projFuelType.toInt();
                        break;
                    case 6: // "Количество цистерн"
                        projTanksCount = QString(stParts.at(1)).toInt();
                        break;
                    case 7: // "Количество потребителей"
                        projConsumersCount = QString(stParts.at(1)).toInt();
                        break;
                    default:
                        break;
                    }
                    currLine++;
                    st = QString(confData.at(currLine)).trimmed();
                }
                // с конфигурацией - все, записываем в БД (после заполнения справочников - есть ссылка на тип топлива)

            }
                break;
            case 1: //"Типы" - справочники
                {
                //referenceTypes = QStringList() << "цистерн" << "измерителей" << "потребителей" << "режимов" << "топлива" << "событий";
                QString refTableName;
                st1 = stParts.at(1); // второе слово заголовка раздела
                partIdx = referenceTypes.indexOf(st1);
                switch (partIdx) {
                case 0:
                    refTableName = "tank_types";
                    break;
                case 1:
                    if(stParts.size() > 2) refTableName = "sensor_types";
                    else refTableName = "meter_types";
                    //qDebug() << "Parts size: " << stParts.size() << ", ref.table: " << refTableName;
                    break;
                case 2:
                    refTableName = "consumer_types";
                    break;
                case 3:
                    refTableName = "run_mode_types";
                    break;
                case 4:
                    refTableName = "fuel_types";
                    break;
                case 5:
                    refTableName = "event_types";
                    break;
                case 6:
                    refTableName = "data_types";
                    break;
                default:
                    refTableName = "";
                    break;
                }

                if(refTableName == "") {
                    //currLine++;
                    break;
                }
                qry = tr("insert into %1 (id, name, description) values (:id, :name, :description);").arg(refTableName);
                //qDebug() << "Fill ref.table " << refTableName << " - query text: " << qry;
                //if(skipWriteToDB) break;

                currLine++;
                st = QString(confData.at(currLine)).trimmed();
                while (st != "") {
                    int c = st.indexOf("=");
                    int idx = st.left(c).toInt();//st.split("=");
                    int d = st.indexOf("(");
                    int l = st.length();
                    QString valName = st.mid(c + 1, d - c - 2);
                    QString valDesc = st.mid(d + 1, l - d - 2);
                    //qDebug() << "insert into " << refTableName << " values - id: " << idx << ", name: " << valName << ", description: " << valDesc;
                    if(!skipWriteToDB) {
                        query->prepare(qry);
                        query->bindValue(":id", idx);
                        query->bindValue(":name", valName);
                        query->bindValue(":description", valDesc);
                        if(!query->exec()) {
                            logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                            logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3 в справочник %4:<br>%5")
                                             .arg(idx)
                                             .arg(valName)
                                             .arg(valDesc)
                                             .arg(refTableName)
                                             .arg(query->lastError().text()),
                                   ui->teLog);
                            qDebug() << "Ошибка заполнения справочника:";
                            qDebug() << "Текст запроса: " << query->executedQuery();
                            qDebug() << query->lastError().text();
                        }
                    }
                    currLine++;
                    st = QString(confData.at(currLine)).trimmed();
                }
                }
                break;
            case 2: //"Цистерны":
                {
                    for (int i = 0; i < projTanksCount; ++i) {
                        currLine++;
                        st = QString(confData.at(currLine++)).trimmed();
                        // [Цистерна 1: главная топливная цистерна]
                        st = st.mid(1, st.length() - 2); // убрали квабратные скобки
                        //qDebug() << "убрали квабратные скобки: " << st;
                        //int c = st.indexOf(":");
                        ////QString valIdx = st.mid(9, c - 9);
                        //int idx = QString(st.mid(9, c - 9)).toInt();
                        //QString valName = st.mid(c + 2, 100);
                        stParts = st.split(":");
                        QString valName = QString(stParts.at(1)).trimmed();
                        stParts = QString(stParts.at(0)).split(" ");
                        int idx = QString(stParts.at(1)).toInt();
                        //qDebug() << "Tank Id:name " << idx << valName;


                        st = QString(confData.at(currLine++)).trimmed();//Тип цистерны: 1
                        int valTankType = QString(st.right(2)).toInt();
                        st = QString(confData.at(currLine++)).trimmed();//Тип измерителя: 2
                        int valSensorType = QString(st.right(2)).toInt();
                        st = QString(confData.at(currLine++)).trimmed();//Количество замеров: 7
                        int valMeasureCount = QString(st.right(2)).toInt();
                        //qDebug() << "tank type - sensor type - measure count: " << valTankType << valSensorType << valMeasureCount;
                        if(!skipWriteToDB) {
                            query->prepare("insert into tanks(id, project_id, tank_type, sensor_type, name) values (:id, :project_id, :tank_type, :sensor_type, :name);");
                            query->bindValue(":id", idx);
                            query->bindValue(":project_id", fProjectId);
                            query->bindValue(":tank_type", valTankType);
                            query->bindValue(":sensor_type", valSensorType);
                            query->bindValue(":name", valName);
                            if(!query->exec()) {
                                logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                                logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список цистерн:<br>%5")
                                                 .arg(idx)
                                                 .arg(valTankType)
                                                 .arg(valSensorType)
                                                 .arg(valName)
                                                 .arg(query->lastError().text()),
                                       ui->teLog);
                                qDebug() << "Ошибка добавления цистерны в список:";
                                qDebug() << "Текст запроса: " << query->executedQuery();
                                qDebug() << query->lastError().text();
                            }
                        }

                        query->prepare("insert into tank_measures(tank_id, measure_id, v_high, v_volume) values (:tank_id, :measure_id, :v_high, :v_volume);");
                        query->bindValue(":tank_id", idx);
                        for (int j = 0; j < valMeasureCount; ++j) {
                            st = QString(confData.at(currLine++)).trimmed(); // 0.500=0.500
                            stParts = st.split("=");
                            double valMeasureHigh = QString(stParts.at(0)).toFloat();
                            double valMeasureVolume = QString(stParts.at(1)).toFloat();
                            //qDebug() << "tank id - measure id - high - volume: " << idx << j << valMeasureHigh << valMeasureVolume;
                            if(!skipWriteToDB) {
                                query->bindValue(":measure_id", j);
                                query->bindValue(":v_high", valMeasureHigh);
                                query->bindValue(":v_volume", valMeasureVolume);
                                if(!query->exec()) {
                                    logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                                    logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список замеров цистерн:<br>%5")
                                                     .arg(idx)
                                                     .arg(j)
                                                     .arg(valMeasureHigh)
                                                     .arg(valMeasureVolume)
                                                     .arg(query->lastError().text()),
                                           ui->teLog);
                                    qDebug() << "Ошибка заполнения списка замеров цистерн:";
                                    qDebug() << "Текст запроса: " << query->executedQuery();
                                    qDebug() << query->lastError().text();
                                }
                            }
                        }
                        //while(QString(confData.at(currLine)).trimmed() != "")  currLine++;
                    }
                }
                break;
            case 3: //"Потребители":
            {
                for (int i = 0; i < projConsumersCount; ++i) {
                    currLine++;
                    st = QString(confData.at(currLine++)).trimmed();
                    // [Потребитель 1: основная ГДУ]
                    st = st.mid(1, st.length() - 2); // убрали квабратные скобки
                    //qDebug() << "убрали квабратные скобки: " << st;
                    //int c = st.indexOf(":");
                    //QString valIdx = st.mid(12, c - 12);
                    //int idx = QString(st.mid(12, c - 12)).toInt();
                    //QString valName = st.mid(c + 2, 100);
                    stParts = st.split(":");
                    QString valName = QString(stParts.at(1)).trimmed();
                    stParts = QString(stParts.at(0)).split(" ");
                    int idx = QString(stParts.at(1)).toInt();
                    //qDebug() << "Consumer Id:name " << idx << valName;
                    st = QString(confData.at(currLine++)).trimmed();//Тип потребителя: 1
                    int valConsType = QString(st.right(2)).toInt();
                    st = QString(confData.at(currLine++)).trimmed();//Тип измерителя: 2
                    int valMeterType = QString(st.right(2)).toInt();
                    st = QString(confData.at(currLine++)).trimmed();//Количество режимов работы: 3
                    int valRunModesCount = QString(st.right(2)).toInt();
                    //qDebug() << "consumer type - meter type - run mode count: " << valConsType << valMeterType << valRunModesCount;
                    if(!skipWriteToDB) {
                        query->prepare("insert into consumers(id, project_id, consumer_type, meter_type, name) values (:id, :project_id, :consumer_type, :meter_type, :name);");
                        query->bindValue(":id", idx);
                        query->bindValue(":project_id", fProjectId);
                        query->bindValue(":consumer_type", valConsType);
                        query->bindValue(":meter_type", valMeterType);
                        query->bindValue(":name", valName);
                        if(!query->exec()) {
                            logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                            logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список потребителей:<br>%5")
                                             .arg(idx)
                                             .arg(valConsType)
                                             .arg(valMeterType)
                                             .arg(valName)
                                             .arg(query->lastError().text()),
                                   ui->teLog);
                            qDebug() << "Ошибка добавления потребителя в список:";
                            qDebug() << "Текст запроса: " << query->executedQuery();
                            qDebug() << query->lastError().text();
                        }

                        // подготовка к следующей части
                        query->prepare("insert into consumers_spend(consumer_id, run_mode_id, spend) values (:consumer_id, :run_mode_id, :spend);");
                        query->bindValue(":consumer_id", idx);
                    }

                    for (int j = 0; j < valRunModesCount; ++j) {
                        st = QString(confData.at(currLine++)).trimmed(); // 1=1.000
                        stParts = st.split("=");
                        int valRunModeId = QString(stParts.at(0)).toInt();
                        double valRunModeSpend = QString(stParts.at(1)).toFloat();
                        //qDebug() << "tank id - record id - mode id - spend: " << idx << j << valRunModeId << valRunModeSpend;
                        if(!skipWriteToDB) {
                            query->bindValue(":run_mode_id", valRunModeId);
                            query->bindValue(":spend", valRunModeSpend);
                            if(!query->exec()) {
                                logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                                logMsg(msgError, tr("Ошибка вставки значений расхода потребителя '%1', %2, %3 в список :<br>%4")
                                                 .arg(idx)
                                                 .arg(valRunModeId)
                                                 .arg(valRunModeSpend)
                                                 .arg(query->lastError().text()),
                                       ui->teLog);
                                qDebug() << "Ошибка заполнения списка расходов потребителя:";
                                qDebug() << "Текст запроса: " << query->executedQuery();
                                qDebug() << query->lastError().text();
                            }
                        }
                    }
                    //while(QString(confData.at(currLine)).trimmed() != "")  currLine++;
                }

            }
                break;
            case 4: //"Пересчет": // "Пересчет значений плотности"
                //qDebug() << "Пересчет значений плотности";
                {
                currLine++;
                int n = 0;
                st = QString(confData.at(currLine++)).trimmed();
                int fuelTypesCount = st.right(2).toInt();
                for (int i = 0; i < fuelTypesCount; ++i) {
                    st = QString(confData.at(currLine++)).trimmed();// [Тип топлива 1: летнее диз.топливо]
                    int fuelTypeIdx = st.mid(st.indexOf(":") - 2, 2).toInt();
                     if(!skipWriteToDB) {
                        qry = "insert into fuel_density(id, fuel_type_id, fuel_temp, fuel_density) values (:id, :fuel_type_id, :fuel_temp, :fuel_density);";
                        query->prepare(qry);
                        query->bindValue(":fuel_type_id", fuelTypeIdx);
                     }
                    //currLine++;
                    st = QString(confData.at(currLine++)).trimmed();
                    while (st != "") { //
                        //qDebug() << "fuel_density string: " << st;
                        n++;
                        stParts = st.split("=");
                        double fuelTemp = QString(stParts.at(0)).toFloat();
                        double fuelDens = QString(stParts.at(1)).toFloat();
                        //qDebug() << "ID - fuel temp-density: "<< n << fuelTemp << fuelDens;
                        if(!skipWriteToDB) {
                            query->bindValue(":id", n);
                            query->bindValue(":fuel_temp", fuelTemp);
                            query->bindValue(":fuel_density", fuelDens);
                            if(!query->exec()) {
                                logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                                logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список пересчетов плотности топлива:<br>%5")
                                                 .arg(n)
                                                 .arg(projFuelTypeIdx)
                                                 .arg(fuelTemp)
                                                 .arg(fuelDens)
                                                 .arg(query->lastError().text()),
                                       ui->teLog);
                                qDebug() << "Ошибка добавления записи в список пересчетов плотности топлива:";
                                qDebug() << "Текст запроса: " << query->executedQuery();
                                qDebug() << query->lastError().text();
                            }

                        }
                        st = QString(confData.at(currLine++)).trimmed();
                    }
                }

                }
                break;
            default:
                //? - не поняли, идем дальше
                qDebug() << "Неопознанная ошибка, строка: " << currLine;
                currLine++;
                break;
            }

        }
        currLine++;
    }
    // восстановление основной конфигурации - после всего, когда заполнены справочники
    query->finish();
    if(query->exec("select count(*) as rec_count from ship_def;")) {
        query->next();
        int rec_count = query->value("rec_count").toInt();
        if(rec_count == 0) {
            qry = "INSERT INTO ship_def(project_id, ship_board_num, displacement, fuel_type, tanks_count, consumers_count,   ship_project, ship_man_number, ship_type) "
                    "VALUES (:project_id, :ship_board_num, :displacement, :fuel_type, :tanks_count, :consumers_count,   :ship_project, :ship_man_number, :ship_type);";
        } else {
            // "update ... where ..."
            qry = "UPDATE ship_def "
                  "SET ship_board_num = :ship_board_num, displacement = :displacement, fuel_type = :fuel_type, tanks_count = :tanks_count, "
                  "consumers_count = :consumers_count, ship_project = :ship_project, ship_man_number = :ship_man_number, ship_type = :ship_type "
                  "WHERE project_id = :project_id;"
                  ;
        }
        query->prepare(qry);
        query->bindValue(":project_id", fProjectId);
        query->bindValue(":ship_board_num", projBoardNumber);
        query->bindValue(":displacement", projDisp);
        //
        query->bindValue(":fuel_type", projFuelTypeIdx);
        query->bindValue(":tanks_count", projTanksCount);
        query->bindValue(":consumers_count", projConsumersCount);
        query->bindValue(":ship_project", projNum);
        query->bindValue(":ship_man_number", projWorkNum);
        query->bindValue(":ship_type", projType);
        if(query->exec()) {
            logMsg(msgInfo, "Оcновная конфигурация сохранена в БД");
        } else {
            logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
            logMsg(msgError, tr("Ошибка сохранения конфигурации:<br>%1").arg(query->lastError().text()), ui->teLog);
            qDebug() << "Ошибка сохранения конфигурации:";
            qDebug() << "Текст запроса: " << query->executedQuery();
            qDebug() << query->lastError().text();
        }
        setSequenceValues();
    } else {
        logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
        logMsg(msgError, tr("Ошибка сохранения конфигурации в БД:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка сохранения конфигурации в БД:";
        qDebug() << "Текст запроса: " << query->executedQuery();
        qDebug() << query->lastError().text();
    }

}

//--------------------- корректность данных -------------------------------------------------------
void MainWindow::on_leProjNum_editingFinished()
{
    checkCorrectData();
}

void MainWindow::on_leWorkNum_editingFinished()
{
    checkCorrectData();
}

void MainWindow::on_leBoardNum_editingFinished()
{
    checkCorrectData();
}

void MainWindow::on_leType_editingFinished()
{
    checkCorrectData();
}

void MainWindow::on_leDisplacement_editingFinished()
{
    checkCorrectData();
}

void MainWindow::on_cbxFuelType_currentIndexChanged(int )
{
    checkCorrectData();
}

//--------------------- ограничение длин строк ---------------------------------------
void MainWindow::on_leProjNum_textEdited(const QString &arg1)
{
    if(arg1.length() > 16) ui->leProjNum->setText(arg1.left(16));
}

void MainWindow::on_leWorkNum_textEdited(const QString &arg1)
{
    if(arg1.length() > 16) ui->leWorkNum->setText(arg1.left(16));
}

void MainWindow::on_leType_textEdited(const QString &arg1)
{
    if(arg1.length() > 64) ui->leType->setText(arg1.left(64));
}

//------------------ подчиненные формы ----------------------------------------------

void MainWindow::on_pbnTanks_clicked()
{
    te->show();
}

void MainWindow::on_pbnConsumers_clicked()
{
    ce->show();
//    NewModel* nm = new NewModel(this);
//    nm->show();
//    nm->close();
//    nm->deleteLater();

    return;
    ui->statusBar->showMessage("Список потребителей - будет позже", 1000);
}

void MainWindow::on_pbnSensors_clicked()
{
    ne->show();
}

void MainWindow::on_pbnDensToMass_clicked()
{
    fe->show();
    return;

//    if(QApplication::keyboardModifiers() && Qt::ShiftModifier) {
//        logMsg(msgDate, "Date messge");
//        logMsg(msgDefault, "Default messge");
//        logMsg(msgFail, "Fail messge");
//        logMsg(msgInfo, "Info messge");
//        logMsg(msgError, "Error messge");
//        logMsg(msgSuccess, "Success messge");
//    } else {
//        ui->statusBar->showMessage("Таблица пересчета - будет позже", 1000);
//    }


    QTableWidgetItem* twi = new (QTableWidgetItem);
    //twi->setData(Qt::UserRole, 75);
    //qDebug() << "Data value: " << twi->data(Qt::UserRole).toInt();
    //qDebug() << "WhatsThisPropertyRole: " << twi->data(Qt::WhatsThisPropertyRole);
    for (int i = 0; i < 10; ++i) {
        twi->setData(Qt::UserRole, i);
        qDebug() << "Data value: " << twi->data(Qt::UserRole).toInt();
        qDebug() << "WhatsThisPropertyRole: " << twi->data(Qt::WhatsThisPropertyRole);
    }
//    twi->
}

void MainWindow::on_pbnDicts_clicked()
{
    if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        setSequenceValues();
        return;
    }
    re->show();
}

//----------------- сохранение и восстановление -----------------------------------------------
void MainWindow::on_pbnSave_clicked()
{
    if(!db->isOpen()) {
        logMsg(msgFail, "Сохранить данные невозможно - не открыта БД");
        return;
    }
    QString qry;
    if (fConfTableRecCount > 0) {
        qry = tr("UPDATE ship_def SET ship_board_num=:ship_board_num, displacement=:displacement, fuel_type=:fuel_type, "
              "ship_project=:ship_project, ship_man_number=:ship_man_number, ship_type=:ship_type where project_id = :project_id;").arg(fProjectId);
    } else {
        qry = "INSERT INTO public.ship_def(ship_board_num, displacement, fuel_type, tanks_count, consumers_count, ship_project, ship_man_number, ship_type)"
              "VALUES (:ship_board_num, :displacement, :fuel_type, 0, 0, :ship_project, :ship_man_number, :ship_type);";
    }

    query->prepare(qry);
    int fNewBoardNum = ui->leBoardNum->text().trimmed().toInt();
    query->bindValue(":ship_board_num", fNewBoardNum);
    int fNewDisplacement = ui->leDisplacement->text().trimmed().toInt();
    query->bindValue(":displacement", fNewDisplacement);//ui->leDisplacement->text().trimmed().toInt());
    int fNewFuelType = ui->cbxFuelType->currentIndex();
    query->bindValue(":fuel_type", fNewFuelType);//ui->cbxFuelType->currentIndex());
    QString fNewProjNum = ui->leProjNum->text().trimmed();
    query->bindValue(":ship_project", fNewProjNum);//ui->leProjNum->text().trimmed());
    QString fNewWorkNum = ui->leWorkNum->text().trimmed();
    query->bindValue(":ship_man_number", fNewWorkNum);//ui->leWorkNum->text().trimmed());
    QString fNewType = ui->leType->text().trimmed();
    query->bindValue(":ship_type", fNewType);//ui->leType->text().trimmed());
    if(query->exec()) {
        logMsg(msgInfo, "Конфигурационные данные сохранены");
        fBoardNum = fNewBoardNum;
        fDisplacement = fNewDisplacement;
        fFuelType = fNewFuelType;
        fShipProjectNum = fNewProjNum;
        fShipManufactureNum = fNewWorkNum;
        fShipType = fNewType;
        ui->pbnSave->setEnabled(false);
        ui->pbnRestore->setEnabled(false);
    } else {
        QString errMess = query->lastError().text();
        qDebug() << "Ошибка сохранения данных:";
        qDebug() << errMess;//db.lastError().text();
        logMsg(msgError, tr("Ошибка сохранения данных:<br>%1").arg(errMess));
    }
}

void MainWindow::on_pbnRestore_clicked()
{
    if(QApplication::keyboardModifiers() && Qt::ShiftModifier) {
        if(fConfTableRecCount > 0) {
            query->finish();
            query->exec("select * from ship_def;");
            query->first();
            fShipProjectNum = query->value("ship_project").toString();
            fShipManufactureNum = query->value("ship_man_number").toString();
            fShipType = query->value("ship_type").toString();
            fBoardNum = query->value("ship_board_num").toInt();
            fFuelType = query->value("fuel_type").toInt();
            fDisplacement = query->value("displacement").toInt();
//                                          ship_board_num, displacement, fuel_type, tanks_count, consumers_count, ship_project, ship_man_number, ship_type

            query->finish();
        }
    }
    showCurrentData();
}

void MainWindow::on_chbxShowLog_clicked(bool checked)
{
    ui->teLog->setVisible(checked);
    if (checked) {
        setMaximumSize(2000, 10000);
        setMinimumSize(800, 680);
//        ui->teLog->geometry().setHeight(height() - 40);
    } else {
        setMinimumSize(380, 680);
        setMaximumSize(380, 680);
    }
}

void MainWindow::on_pbnSaveToFile_clicked()
{
    saveDataToFile();
}

void MainWindow::on_pbnRestoreFromFile_clicked()
{
//    ui->statusBar->showMessage("Восстановление БД из файла - в клиентском приложении.", 3000);
//    return;

    if(QApplication::keyboardModifiers() & Qt::ShiftModifier) clearDB();
    else restoreDataFromFile();
}

// стили формы - предопределенные в ОС
void MainWindow::slotStyleChange(const QString &str)
{
    QStyle* pstyle = QStyleFactory::create(str);
    QApplication::setStyle(pstyle);

    te->userStyleChanged();
    fe->userStyleChanged();
}

// стили формы - из файла .qss (аналогично html-ному .css)
void MainWindow::on_pbnLoadStyle_clicked()
{
//    if(QApplication::keyboardModifiers() && Qt::ShiftModifier) {
    QString strFilter;
    QString loadFileName = QFileDialog::getOpenFileName(0,
                                                        tr("Загрузить стиль из файла"), // заголовок окна
                                                        "./", // каталог/файл по умолчанию
                                                        "*.qss", // маска расширений
                                                        &strFilter // ???
                                                        );
    if(loadFileName.isEmpty()) return;
    QFile f(loadFileName);
    f.open(QFile::ReadOnly);
    QString strCSS = QLatin1String(f.readAll());
    //QApplication::setStyleSheet(strCSS);
    //QApplication* qApp;// = QApplication::self();//QCoreApplication::self;
    this->setStyleSheet(strCSS);
    te->setStyleSheet(strCSS);
    re->setStyleSheet(strCSS);
    ce->setStyleSheet(strCSS);
    fe->setStyleSheet(strCSS);
    ne->setStyleSheet(strCSS);
}

