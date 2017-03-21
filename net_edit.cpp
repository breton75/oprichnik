#include "net_edit.h"
#include "ui_net_edit.h"

#include <QSplitter>
#include "../common/u_log.h"

NetEdit::NetEdit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NetEdit)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowIcon(QIcon("./Network.png"));
    ui->menubar->setVisible(false);
    ui->statusbar->setVisible(false);

    int w = this->width();
    int h = this->height();
    int twoThirdW = w * 2 / 3;
    int twoThirdH = h * 2 / 3;

    // компоновщики
    ui->frame_1->setLayout(ui->glSensorsList);
//    ui->frame_2->setLayout(ui->verticalLayout);
//    ui->frame_3->setLayout(ui->glMeasures);
    ui->frame_4->setLayout(ui->horizontalLayout);

    // "резинки" панелей
    QSplitter* sp1 = new QSplitter(Qt::Horizontal, this);
    sp1->setGeometry(0, 0, twoThirdW, twoThirdH);
    sp1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp1->addWidget(ui->frame_1);
    //sp1->addWidget(ui->frame_2);
    sp1->addWidget(ui->frame_3);
    //connect(sp1, SIGNAL(splitterMoved(int,int)), this, SLOT(onSomeSplitterMoved(int,int)));

    //fix("Форма цистерн - один сплиттер есть, создаем второй");
    QSplitter* sp = new QSplitter(Qt::Vertical, this);
    sp->setGeometry(this->rect());
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp->addWidget(sp1);//sp2);
    sp->addWidget(ui->frame_4);
    //connect(sp, SIGNAL(splitterMoved(int,int)), this, SLOT(onSomeSplitterMoved(int,int)));
    this->setCentralWidget(sp);

    //ui->frame_2->setVisible(false);
    ui->frameRButtons->setVisible(false); // не готово
    ui->pbnSaveSensorParams->setVisible(false);
    ui->pbnClearSensorParams->setVisible(false);

    query =  new QSqlQuery();

    //---------------------- сетка списка потребителей ---------------------------
    currentSensorId = -1;
    currentSensorName = "";
    currentProjectId = 1;
    // индекс скрытого столбца - состояния записи
    sensors_record_state_idx = 15; // последний столбец

    // стандартные модели
    tvSensorsModel = new QStandardItemModel(ui->tvSensors);
    tvSensorsSelectionModel = new QItemSelectionModel(tvSensorsModel);
//    UniversalDelegate* tvSensorsDelegate;
    ui->tvSensors->setModel(tvSensorsModel);
    ui->tvSensors->setSelectionModel(tvSensorsSelectionModel);
    ui->tvSensors->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку

    // контекстное меню
    QAction* act = new QAction(QObject::tr("Добавить датчик"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(on_pbnAddSensor_clicked()));
    ui->tvSensors->addAction(act);

//    ui->tvSensors->addAction(new QAction(QObject::tr("Menu 2"), ui->tvSensors));
//    ui->tvSensors->addAction(new QAction(QObject::tr("Menu 3"), ui->tvSensors));
    ui->tvSensors->setContextMenuPolicy(Qt::ActionsContextMenu);
    //ui->tvSensors->actions().clear();

    // сменена строка содержимого - выдаем список измерений, и разобраться - удалена запись или нет
    connect(tvSensorsSelectionModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvSensorsRowChanged(QModelIndex,QModelIndex)));

    // обновление списков
    updateSensorTypes();
    updateTanksList();
    updateConsumersList();
    updateSensorsList();
}

NetEdit::~NetEdit()
{
    delete ui;
}

void NetEdit::setProjectId(int newProjectId)
{
    currentProjectId = newProjectId;
    if(this->isVisible()) updateSensorsList();
}

//void NetEdit::resizeEvent(QResizeEvent *)
//{
//    //
//}

void NetEdit::showEvent(QShowEvent *)
{
    updateSensorsList();
}

//============================================================================
// основной список формы - список датчиков (дерево)
// одноранговым списком
void NetEdit::makeSensorsListCommon()
{
    QString qry = tr("SELECT "\
        "  s.id, "\
        "  st.description || ' - ' || coalesce(t.name, c.name) as sensor_name,"\
        "  s.sensor_type, st.name as sensor_type_name, st.description as sensor_type_desc,"\
        "  s.tank_id, t.name as tank_name,"\
        "  s.consumer_id, c.name as consumer_name,"\
        "  net_address,"\
        "  net_idx, data_type, lo_val, high_val"\
        " FROM sensors s"\
        "  JOIN sensor_types st"\
        "    ON st.id = s.sensor_type"\
        "  LEFT JOIN tanks t"\
        "    ON t.id = s.tank_id"\
        "  LEFT JOIN consumers c"\
        "    ON c.id = s.consumer_id"\
        " WHERE s.project_id = %1"\
        " ORDER BY s.id"\
        ";").arg(currentProjectId);

    query->clear();
    if(!query->exec(qry)) {
        logMsg(msgError, tr("Ошибка обновления списка датчиков:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка датчиков:";
        qDebug() << query->lastError().text();
        return;
    }

    currentSensorId = -1;

    // размер модели
    int n = query->numRowsAffected();//size();
    int colCount = query->record().count();
    //qDebug() << "qry sensors list - rec.count: " << query->record().count() << ", rows affected: " << n << ", col.count: " << colCount;

    tvSensorsModel->clear();
    tvSensorsModel->setColumnCount(colCount + 2); // данные плюс состояние плюс номер записи с индикацией состояния
    tvSensorsModel->setRowCount(n);
    tvSensorsModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "№" << "Датчик" << "ИД типа датчика" << "Тип датчика" << "Описание"
                                              << "ИД цистерны" << "Цистерна"
                                              << "ИД потребителя" << "Потребитель"
                                              << "Сетевой адрес" << "Индекс на ПИ"
                                              << "Тип данных" << "MIN значение" << "MAX значение" << "Сост.записи");
    //int i;
    //for(int i = 0; i < colCount + 2; i++) tvSensorsModel->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignCenter); // вылетает с ошибкой - Segmentation fault
    tvSensorsModel->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter); // а так - не вылетает
    tvSensorsModel->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    if(n <= 0) {
        qDebug() << "Список датчиков пуст, выходим";
        clearSensorParams();
        return;
    }

    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;

        // поля:
        // id, sensor_name, sensor_type, sensor_type_name, sensor_type_desc,
        // tank_id, tank_name, consumer_id, consumer_name,
        // net_address, net_idx, data_type, lo_val, high_val
        // + record_state

        tvSensorsModel->setItem(i, 0, new QStandardItem(query->value("id").toString()));
        // столбец 1 - номер записи с индикацией состояния записи
        tvSensorsModel->setItem(i, 2, new QStandardItem(query->value("sensor_name").toString()));
        tvSensorsModel->setItem(i, 3, new QStandardItem(query->value("sensor_type").toString()));
        tvSensorsModel->setItem(i, 4, new QStandardItem(query->value("sensor_type_name").toString()));
        tvSensorsModel->setItem(i, 5, new QStandardItem(query->value("sensor_type_desc").toString()));
        tvSensorsModel->setItem(i, 6, new QStandardItem(query->value("tank_id").toString()));
        tvSensorsModel->setItem(i, 7, new QStandardItem(query->value("tank_name").toString()));
        tvSensorsModel->setItem(i, 8, new QStandardItem(query->value("consumer_id").toString()));
        tvSensorsModel->setItem(i, 9, new QStandardItem(query->value("consumer_name").toString()));
        tvSensorsModel->setItem(i, 10, new QStandardItem(query->value("net_address").toString()));
        tvSensorsModel->setItem(i, 11, new QStandardItem(query->value("net_idx").toString()));
        tvSensorsModel->setItem(i, 12, new QStandardItem(query->value("data_type").toString()));
        tvSensorsModel->setItem(i, 13, new QStandardItem(query->value("lo_val").toString()));
        tvSensorsModel->setItem(i, 14, new QStandardItem(query->value("high_val").toString()));
        tvSensorsModel->setItem(i, 15, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        si->setIcon(QIcon(":/Apply.ico"));
        //tvSensorsModel->setVerticalHeaderItem(i, si); // это работает только у TableView, у TreeView - игнорируется
        tvSensorsModel->setItem(i, 1, si);
    }
    tvSensorsSelectionModel->reset();
    ui->tvSensors->setSelectionModel(tvSensorsSelectionModel);
    //qDebug() << "qry rec.count: " << query->record().count(); // columns count

    ui->tvSensors->hideColumn(0);// ИД
    ui->tvSensors->setColumnWidth(1, 50);
    //ui->tvSensors->setMinimumWidth(50);
    //ui->tvSensors->hideColumn(sensors_record_state_idx);// состояние записи
    if(!ui->chbxAllFields->isChecked()) for (i = 3; i < tvSensorsModel->columnCount(); ++i) ui->tvSensors->hideColumn(i);
    // кнопки - что можно / что нельзя
    ui->pbnAddSensor->setEnabled(true);
    ui->pbnRemoveSensor->setEnabled(n > 0);
    ui->pbnSaveSensorsList->setEnabled(false);
    ui->pbnRestoreSensorsList->setEnabled(false);

    currentSensorId = tvSensorsModel->item(0, 0)->data(Qt::DisplayRole).toInt();
    //currentSensorsListRow = tvSensorsModel->index(0, 1);
    ui->tvSensors->setCurrentIndex(tvSensorsModel->index(0, 1));
    //qDebug() << "tvSensors->model()->columnCount(): " << ui->tvSensors->model()->columnCount() << ", tvSensorsModel.columnCount(): " << tvSensorsModel->columnCount();
    restoreSensorParams();
    if(this->isVisible()) ui->tvSensors->setFocus();
}

// дерево по устройствам
void NetEdit::makeSensorsListByDevs()
{
    return;
    // иерархия:
    QModelIndex idx = tvSensorsModel->index(0, 0);
    tvSensorsModel->setData(idx, "Parent item");
    tvSensorsModel->insertRows(0, 3, idx);
    tvSensorsModel->insertColumns(0, 4, idx);
    for (int nRow = 0; nRow < 3; ++nRow) {
        for (int nCol = 0; nCol < 4; ++nCol) {
            tvSensorsModel->setData(tvSensorsModel->index(nRow, nCol, idx), "Child text");
        }
    }
    // нужны другие статусы записи, кроме уже имеющихся
}

// дерево по типам датчиков
void NetEdit::makeSensorsListByTypes()
{
    //
}

void NetEdit::updateSensorsList()
{
    tvSensorsModel->clear();
    if(ui->rbnSensorsAll->isChecked()) makeSensorsListCommon();
    else if(ui->rbnSensorsByDevs->isChecked()) makeSensorsListByDevs();
    else if(ui->rbnSensorsByTypes->isChecked()) makeSensorsListByTypes();
}

void NetEdit::addSensor()
{
    int n = tvSensorsModel->rowCount();
    tvSensorsModel->appendRow(QList<QStandardItem*>()
                      << new QStandardItem("-1") // id
                      << new QStandardItem("")   // sensor_name
                      << new QStandardItem("-1") // sensor_type
                      << new QStandardItem("")   // sensor_type_name
                      << new QStandardItem("")   // sensor_type_desc
                      << new QStandardItem("-1") // tank_id
                      << new QStandardItem("")   // tank_name
                      << new QStandardItem("-1") // consumer_id
                      << new QStandardItem("")   // consumer_name
                      << new QStandardItem("-1") // net_address
                      << new QStandardItem("0")  // net_idx
                      << new QStandardItem("-1") // data_type
                      << new QStandardItem("0")  // lo_val
                      << new QStandardItem("0")  // high_val
                      << new QStandardItem(tr("%1").arg(rsAdded))); //record_state

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvSensorsModel->setVerticalHeaderItem(n, si);
    tvSensorsModel->submit();

    ui->tvSensors->setCurrentIndex(tvSensorsModel->index(n, 1));
    //ui->tvSensors->setFocus();
    ui->pbnAddSensor->setEnabled(false);
    ui->pbnRemoveSensor->setEnabled(true);
    ui->pbnSaveSensorsList->setEnabled(false);
    ui->pbnRestoreSensorsList->setEnabled(true);
    currentSensorId = -1;
    currentSensorName = "";
    restoreSensorParams();
    ui->cbxSensorTypes->setFocus();
}

void NetEdit::removeSensor()
{
    int r = ui->tvSensors->currentIndex().row();
    int recordState = tvSensorsModel->item(r, sensors_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvSensorsModel->item(r, sensors_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
//        tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
//        tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        tvSensorsModel->item(r, 1)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvSensorsModel->item(r, 1)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveSensor->setText("Отменить");
        ui->pbnRemoveSensor->setToolTip("Отменить удаление записи");
        ui->pbnRemoveSensor->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvSensorsModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvSensorsModel->item(r, sensors_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            //tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            //tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
            tvSensorsModel->item(r, 1)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvSensorsModel->item(r, 1)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvSensorsModel->item(r, sensors_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            //tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            //tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
            tvSensorsModel->item(r, 1)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvSensorsModel->item(r, 1)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveSensor->setText("Удалить");
        ui->pbnRemoveSensor->setToolTip("Удалить запись");
        ui->pbnRemoveSensor->setStatusTip("Удалить запись");
    }
    //checkSensorButtons();
}

void NetEdit::saveSensors()
{
/*
    int n = tvConsumersModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int ru = 0;
    int rd = 0;
    int errCount = 0;

// id, consumer_type, meter_type, name
    QString qryInsert = "insert into consumers(project_id, consumer_type, meter_type, name) values(:project_id, :consumer_type, :meter_type, :name);";
    QString qryUpdate = "update consumers set consumer_type = :consumer_type, meter_type = :meter_type, name = :name where id = :id;";// and project_id = :project_id;";
    QString qryDelete = "delete from consumers where id = :id;";// and project_id = :project_id;";
    query->finish();

    for (int i = 0; i < n; ++i) {
        bool nothingToDo = false;
        //int recordState = tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        //switch (recordState) {
        switch (tvConsumersModel->item(i, consumer_record_state_idx)->data(Qt::DisplayRole).toInt()) {
        case rsAdded:
            ri++;
            query->prepare(qryInsert);
            query->bindValue(":project_id", currentProjectId);
            query->bindValue(":consumer_type", tvConsumersModel->item(i, 1)->data(Qt::DisplayRole).toInt());
            query->bindValue(":meter_type", tvConsumersModel->item(i, 2)->data(Qt::DisplayRole).toInt());
            query->bindValue(":name", tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsChanged:
            ru++;
            query->prepare(qryUpdate);
            query->bindValue(":id", tvConsumersModel->item(i, 0)->data(Qt::DisplayRole).toInt());
            query->bindValue(":consumer_type", tvConsumersModel->item(i, 1)->data(Qt::DisplayRole).toInt());
            query->bindValue(":meter_type", tvConsumersModel->item(i, 2)->data(Qt::DisplayRole).toInt());
            //int k = tvConsumersModel->item(i, 2)->data(Qt::DisplayRole).toInt();
            //query->bindValue(":meter_type", k);
            query->bindValue(":name", tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsDeleted:
            {
                int delId = tvConsumersModel->item(i, 0)->data(Qt::DisplayRole).toInt();
                if (delId < 0) {
                    nothingToDo = true;
                } else {
                    rd++;
                    // удалить записи из таблицы расходов
                    if(!query->exec(tr("delete from consumers_spend where consumer_id = %1").arg(delId))) {
                        //err_ocuped = true;
                        errCount++;
                        QString errSt = query->lastError().text();
                        qDebug() << "Ошибка удаления данных о расходах потребителя " << tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed() << ":";
                        qDebug() << errSt;
                        logMsg(msgError, tr("Ошибка удаления данных о расходах потребителя '%1':<br>%2")
                                        .arg(tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed())
                                        .arg(errSt), ui->teLog);
                        nothingToDo = true; // не смогли - потребителя не удаляем
                        // хотя из-за наличия ссылки на нее - и не получится

                    } else {
                        query->prepare(qryDelete);
                        query->bindValue(":id", delId);
                    }
                }
                break;
            }
        default: // rsUnchanged
            nothingToDo = true;
            break;
        }

        if(nothingToDo) continue;

        if (!query->exec()) {
            //err_ocuped = true;
            errCount++;
            QString errSt = query->lastError().text();
            qDebug() << "Ошибка сохранения данных:";
            qDebug() << errSt;
            logMsg(msgError, tr("Ошибка сохранения данных:<br>%1").arg(errSt), ui->teLog);
            //break;
        }
        query->finish();
    }
    tvConsumersModel->submit();
    QString resultMess = "Обновлены данные списка потребителей<br>";
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    // обновление основной конфигурации
    query->exec(tr("update ship_def set consumers_count = %1;").arg(n));
    updateConsumersList();
    //qDebug() << "Обновление данных списка потребителей закончено";
*/
}

void NetEdit::restoreSensors()
{
    updateSensorsList();
}

bool NetEdit::validSensorsData()
{
    return false;
}

void NetEdit::checkSensorsButtons()
{
    //
}

//-----------------------------------------------------------------
// очистить все поля
void NetEdit::clearSensorParams()
{
    if(ui->cbxSensorTypes->count() > 0) ui->cbxSensorTypes->setCurrentIndex(0);
    ui->rbnTank->setChecked(true);
    setPlaceList();
    if(ui->cbxTanksList->count() > 0) ui->cbxTanksList->setCurrentIndex(0);
    if(ui->cbxConsumersList->count() > 0) ui->cbxConsumersList->setCurrentIndex(0);
    ui->edAddress->setValue(0);
    ui->edIndex->setValue(0);
    ui->edLowVal->setValue(0.0);
    ui->edHiVal->setValue(0.0);
}

// показ либо списка цистерн, либо списка потребителей
void NetEdit::setPlaceList()
{
    if(ui->rbnTank->isChecked()) {
        ui->cbxConsumersList->setVisible(false);
        ui->cbxTanksList->setVisible(true);
        ui->lblPlace->setText("Цистерна");
    } else {
        ui->cbxConsumersList->setVisible(true);
        ui->cbxTanksList->setVisible(false);
        ui->lblPlace->setText("Потребитель");
    }
    QApplication::processEvents();
}

// сохранить (записать) введенные данные
// записываем данные только в модель; в БД - по сохранению всего списка датчиков
void NetEdit::saveSensorParams(int cr)
{
    if(cr < 0) return;
//    qDebug() << "current sensor type index: " << ui->cbxSensorTypes->currentIndex()
//             << ", value: " << cbxSensorTypes.at(ui->cbxSensorTypes->currentIndex())
//             << ", user data: " << ui->cbxSensorTypes->currentData().toInt(); // совпадает с cbxSensorTypes.at(ui->cbxSensorTypes->currentIndex())
//    return;

    //int cr = ui->tvSensors->currentIndex().row();
    // поля:
    // id, <added: record num,> sensor_name, sensor_type, sensor_type_name, sensor_type_desc,// 0-5
    // tank_id, tank_name, consumer_id, consumer_name, // 6-9
    // net_address, net_idx, data_type, lo_val, high_val // 10-14
    // + record_state // 15

    // 2 - sensor_name
    tvSensorsModel->item(cr, 3)->setData(cbxSensorTypes.at(ui->cbxSensorTypes->currentIndex()), Qt::DisplayRole); // sensor_type // ui->cbxSensorTypes->currentData();
    tvSensorsModel->item(cr, 4)->setText(ui->cbxSensorTypes->currentText());//, Qt::DisplayRole);// sensor_type_name
    // 5 - sensor_type_desc - ? и как получить имя датчика: <sensor_type_desc> - <place> ?

    if(ui->rbnTank->isChecked()) { // датчик на танке
        tvSensorsModel->item(cr, 6)->setData(ui->cbxTanksList->currentData(), Qt::DisplayRole); // tank_id
        tvSensorsModel->item(cr, 7)->setText(ui->cbxTanksList->currentText()); // tank_name
        tvSensorsModel->item(cr, 8)->setData(0, Qt::DisplayRole); // consumer_id
        tvSensorsModel->item(cr, 9)->setText(""); // consumer_name
    } else { // датчик на потребителе
        tvSensorsModel->item(cr, 6)->setData(0, Qt::DisplayRole); // tank_id
        tvSensorsModel->item(cr, 7)->setText(""); // tank_name
        tvSensorsModel->item(cr, 8)->setData(ui->cbxConsumersList->currentData(), Qt::DisplayRole); // consumer_id
        tvSensorsModel->item(cr, 9)->setText(ui->cbxConsumersList->currentText()); // consumer_name
    }

    tvSensorsModel->item(cr, 10)->setText(ui->edAddress->text());   // net_address
    tvSensorsModel->item(cr, 11)->setText(ui->edIndex->text());     // net_idx
    // 12 - data_type - ?
    //tvSensorsModel->item(cr, 13)->setText(ui->edLowVal->text());    // lo_val
    tvSensorsModel->item(cr, 13)->setData(ui->edLowVal->value(), Qt::DisplayRole);
    //tvSensorsModel->item(cr, 14)->setText(ui->edHiVal->text());     // high_val
    tvSensorsModel->item(cr, 14)->setData(ui->edHiVal->value(), Qt::DisplayRole);

    int rs = tvSensorsModel->item(cr, sensors_record_state_idx)->data(Qt::DisplayRole).toInt();;
    if(rs == rsUnchanged) {
        tvSensorsModel->item(cr, sensors_record_state_idx)->setData(rsChanged);
        tvSensorsModel->item(cr, 1)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvSensorsModel->item(cr, 1)->setToolTip(tr("Запись %1 изменена").arg(cr + 1));
    } else
        if(rs == rsDeleted) {
            logMsg(msgFail, "Запись удалена, при сохранении изменения будут потеряны!", ui->teLog);
        }

    ui->pbnSaveSensorsList->setEnabled(true);
    ui->pbnRestoreSensorsList->setEnabled(true);
}

// восстановить исходные данные датчика
void NetEdit::restoreSensorParams()
{
    fSensorParamsChanged = false;
    if(currentSensorId < 0) {
        clearSensorParams();
        return;
    }

    int cr = ui->tvSensors->currentIndex().row();
    // поля:
    // id, sensor_name, sensor_type, sensor_type_name, sensor_type_desc,// 0-4
    // tank_id, tank_name, consumer_id, consumer_name, // 5-8
    // net_address, net_idx, data_type, lo_val, high_val // 9-13
    // + record_state // 14


    int n = tvSensorsModel->item(cr, 3)->data(Qt::DisplayRole).toInt();//tvSensorsModel->item(currentSensorId, 0)->data(Qt::DisplayRole).toInt();;
    n = cbxSensorTypes.indexOf(n);
    if((ui->cbxSensorTypes->count() > 0) && (n >= 0) &&  (n < ui->cbxSensorTypes->count())) ui->cbxSensorTypes->setCurrentIndex(n);
    else ui->cbxSensorTypes->setCurrentIndex(0);
    QString fVal = tvSensorsModel->item(cr, 7)->data(Qt::DisplayRole).toString().trimmed(); // tank_id
    bool tankEmpty = fVal.isEmpty();
    ui->rbnTank->setChecked(!tankEmpty);
    ui->rbnConsumer->setChecked(tankEmpty);
    setPlaceList();
    if(tankEmpty) { // датчик на потребителе
        fVal = tvSensorsModel->item(cr, 8)->data(Qt::DisplayRole).toString().trimmed();
        n = fVal.toInt();
        ui->cbxConsumersList->setCurrentIndex(cbxConsumers.indexOf(n));
    } else { // датчик на танке
        fVal = tvSensorsModel->item(cr, 6)->data(Qt::DisplayRole).toString().trimmed();
        n = fVal.toInt();
        ui->cbxTanksList->setCurrentIndex(cbxTanks.indexOf(n));
    }

    ui->edAddress->setValue(tvSensorsModel->item(cr, 10)->data(Qt::DisplayRole).toInt());
    ui->edIndex->setValue(tvSensorsModel->item(cr, 11)->data(Qt::DisplayRole).toInt());
    ui->edLowVal->setValue(tvSensorsModel->item(cr, 13)->data(Qt::DisplayRole).toFloat());
    ui->edHiVal->setValue(tvSensorsModel->item(cr, 14)->data(Qt::DisplayRole).toFloat());
}

// проверка корректности данных
bool NetEdit::validSensorParamsData()
{
    bool vd = ui->cbxSensorTypes->currentIndex() >= 0; // корректен тип датчика
    bool val1ok, val2ok; // корректны числовые поля
    int netAddr, netIdx;
    float valMin, valMax;
    if(vd) {
        // корректно место установки датчика
        vd = ((ui->rbnTank->isChecked()) && (ui->cbxTanksList->currentIndex() >= 0)) || ((ui->rbnConsumer->isChecked()) && (ui->cbxConsumersList->currentIndex() >= 0));
        if(vd) {
            // корректны сетевые параметры
            netAddr = ui->edAddress->text().toInt(&val1ok);
            netIdx = ui->edIndex->text().toInt(&val2ok);
            vd = (netAddr >= 0) && val1ok && (netIdx >= 0) && val2ok;
            if(vd) {
                // корректен диапазон показаний
                valMin = ui->edLowVal->text().toFloat(&val1ok);
                valMax = ui->edHiVal->text().toFloat(&val2ok);
                vd = val1ok && val2ok && (valMax > valMin);
            }
        }
    }

    return vd;
}

// проверка возможности операций
void NetEdit::checkSensorParamsButtons()
{
    //
}

// установка значений редакторов
//void NetEdit::setEditValues()
//{
//    if(currentSensorId < 0) {
//        clearSensorParams();
//        return;
//    }
//}

//-----------------------------------------------------------------
void NetEdit::updateSensorTypes()
{
    int c = ui->cbxSensorTypes->currentIndex();
    query->clear();
    if(!query->exec("SELECT id, name FROM sensor_types ORDER BY 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов датчиков:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов датчиков:";
        qDebug() << query->lastError().text();
        return;
    }
    int n = query->size();
    ui->cbxSensorTypes->clear(); // комбо-бокс
    cbxSensorTypes.clear(); // список индексов по-порядку
    while (query->next()) {
        int idVal = query->value("id").toInt();
        ui->cbxSensorTypes->addItem(query->value("name").toString(), idVal);
        cbxSensorTypes << idVal;
    }
    if((c >= 0) && (c < n)) ui->cbxSensorTypes->setCurrentIndex(c);
    else if(n > 0) ui->cbxSensorTypes->setCurrentIndex(0);
}

void NetEdit::updateTanksList()
{
    query->clear();
    if(!query->exec(tr("SELECT id, name FROM tanks WHERE project_id = %1 ORDER BY 1;").arg(currentProjectId))) {
        logMsg(msgError, tr("Ошибка обновления списка цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка цистерн:";
        qDebug() << query->lastError().text();
        return;
    }
    ui->cbxTanksList->clear(); // комбо-бокс
    cbxTanks.clear(); // список индексов по-порядку
    while (query->next()) {
        int idVal = query->value("id").toInt();
        ui->cbxTanksList->addItem(query->value("name").toString(), idVal);
        cbxTanks<< idVal;
    }
}

void NetEdit::updateConsumersList()
{
    query->clear();
    if(!query->exec(tr("SELECT id, name FROM consumers WHERE project_id = %1 ORDER BY 1;").arg(currentProjectId))) {
        logMsg(msgError, tr("Ошибка обновления списка цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка цистерн:";
        qDebug() << query->lastError().text();
        return;
    }
    ui->cbxConsumersList->clear(); // комбо-бокс
    cbxConsumers.clear();
    while (query->next()) {
        int idVal = query->value("id").toInt();
        ui->cbxConsumersList->addItem(query->value("name").toString(), idVal);
        cbxConsumers<< idVal;
    }
}

// динамическое обновление ссылок на справочники
void NetEdit::refTableChanged(QString refTableName)
{
    if(refTableName == "sensor_types") {
        updateSensorTypes();
        if(this->isVisible()) updateSensorsList();
    }
}

// смена датчика в списке - обновление параметров и проверка - удален / нет
void NetEdit::tvSensorsRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    if(fSensorParamsChanged) {
        saveSensorParams(idxPrev.row());
    }
    //qDebug() << "tvSensorsRowChanged from " << idxPrev.row() << " to " << idxCurrent.row() << ", current row: " << ui->tvSensors->currentIndex().row();
    currentSensorId = tvSensorsModel->data(tvSensorsModel->index(idxCurrent.row(), 0), Qt::DisplayRole).toInt();
    currentSensorName = tvSensorsModel->data(tvSensorsModel->index(idxCurrent.row(), 1), Qt::DisplayRole).toInt();

    int r = ui->tvSensors->currentIndex().row();
    int recordState = tvSensorsModel->item(r, sensors_record_state_idx)->data(Qt::DisplayRole).toInt();
    //qDebug() << "tvSensorsRowChanged to " << r << ", record state index: " << sensors_record_state_idx << ", record state value: " << recordState;
    if(recordState == rsDeleted) { // удален
        ui->pbnRemoveSensor->setText("Отменить");
        ui->pbnRemoveSensor->setToolTip("Отменить удаление записи");
        ui->pbnRemoveSensor->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveSensor->setText("Удалить");
        ui->pbnRemoveSensor->setToolTip("Удалить запись");
        ui->pbnRemoveSensor->setStatusTip("Удалить запись");
    }

    restoreSensorParams();
    return;
    qDebug() << idxPrev;
}

//===========================================================================
void NetEdit::on_pbnAddSensor_clicked()
{
    addSensor();
}

void NetEdit::on_pbnRemoveSensor_clicked()
{
    removeSensor();
}

void NetEdit::on_pbnSaveSensorsList_clicked()
{
    saveSensors();
}

void NetEdit::on_pbnRestoreSensorsList_clicked()
{
    restoreSensors();
}

void NetEdit::on_rbnTank_clicked()
{
    setPlaceList();
}

void NetEdit::on_rbnConsumer_clicked()
{
    setPlaceList();
}

void NetEdit::on_edAddress_editingFinished()
{
    //checkSensorButtons();
}

void NetEdit::on_pbnClearSensorParams_clicked()
{
    clearSensorParams();
}

void NetEdit::on_pbnRestoreSensorParams_clicked()
{
    restoreSensorParams();
}

void NetEdit::on_pbnSaveSensorParams_clicked()
{
    saveSensorParams(ui->tvSensors->currentIndex().row());
}


void NetEdit::on_chbxAllFields_clicked(bool checked)
{
    if(checked) for (int i = 3; i < ui->tvSensors->model()->columnCount(); ++i) ui->tvSensors->showColumn(i);
    else for (int i = 3; i < ui->tvSensors->model()->columnCount(); ++i) ui->tvSensors->hideColumn(i);
    //ui->tvSensors->adjustSize();
    ui->tvSensors->doItemsLayout();
}

//------------------------------------------------------------
void NetEdit::on_cbxSensorTypes_currentIndexChanged(int index)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << index;
}

void NetEdit::on_rbnTank_toggled(bool checked)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << checked;
}

void NetEdit::on_rbnConsumer_toggled(bool checked)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << checked;
}

void NetEdit::on_cbxConsumersList_currentIndexChanged(int index)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << index;
}

void NetEdit::on_cbxTanksList_currentIndexChanged(int index)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << index;
}

void NetEdit::on_edAddress_valueChanged(int arg1)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << arg1;
}

void NetEdit::on_edIndex_valueChanged(int arg1)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << arg1;
}

void NetEdit::on_edLowVal_valueChanged(double arg1)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << arg1;
}

void NetEdit::on_edHiVal_valueChanged(double arg1)
{
    fSensorParamsChanged = true;
    return;
    qDebug() << arg1;
}

