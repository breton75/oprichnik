#include "net_edit.h"
#include "ui_net_edit.h"

#include <QSplitter>
#include "u_log.h"

NetEdit::NetEdit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NetEdit)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowIcon(QIcon(":/Network.png"));
    ui->menubar->setVisible(false);
    ui->statusbar->setVisible(false);
    ui->chbxAllFields->setVisible(false);

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
    //ui->pbnSaveSensorParams->setVisible(false);
    ui->pbnClearSensorParams->setVisible(false);

    query =  new QSqlQuery();

    //---------------------- сетка списка потребителей ---------------------------
    currentSensorId = -1;
    currentSensorName = "";
    currentProjectId = 1;
    // индекс скрытого столбца - состояния записи
    colIdx_RecordState = 16; // последний столбец

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
    updateDataTypes();
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
        "  s.data_type, dt.name as data_type_name,"\
        "  s.tank_id, t.name as tank_name,"\
        "  s.consumer_id, c.name as consumer_name,"\
        "  net_address,"\
        "  net_idx, lo_val, high_val"\
        " FROM sensors s"\
        "  JOIN data_types dt"\
        "    ON dt.id = s.data_type"\
        "  JOIN sensor_types st"\
        "    ON st.id = s.sensor_type"\
        "  LEFT JOIN tanks t"\
        "    ON t.id = s.tank_id"\
        "  LEFT JOIN consumers c"\
        "    ON c.id = s.consumer_id"\
        " WHERE s.project_id = %1"\
        " ORDER BY s.id"\
        ";").arg(currentProjectId);
    colIdx_RecordState = 16;
    colIdx_SensorId = 0;
    colIdx_RecNo = 1;
    colIdx_SensorName = 2;
    colIdx_SensorTypeId = 3;
    colIdx_SensorTypeName = 4;
    colIdx_SensorTypeDesc = 5;
    colIdx_TankId = 6;
    colIdx_TankName = 7;
    colIdx_ConsumerId = 8;
    colIdx_ConsumerName = 9;
    colIdx_NetAddress = 10;
    colIdx_NetIdx = 11;
    colIdx_DataTypeId = 12;
    colIdx_DataTypeName = 13;
    colIdx_LoVal = 14;
    colIdx_HiVal = 15;

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
    //colIdx_RecordState = tvSensorsModel->columnCount() - 1;
    tvSensorsModel->setRowCount(n);
    tvSensorsModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "№" << "Датчик"
                                              << "ИД типа датчика" << "Тип датчика" << "Описание"
                                              << "ИД цистерны" << "Цистерна"
                                              << "ИД потребителя" << "Потребитель"
                                              << "Сетевой адрес" << "Индекс на ПИ"
                                              << "ИД типа данных" << "Тип данных"
                                              << "MIN значение" << "MAX значение" << "Сост.записи");
    int i;
    //for(int i = 0; i < colCount + 2; i++) tvSensorsModel->horizontalHeaderItem(i)->setTextAlignment(Qt::AlignCenter); // вылетает с ошибкой - Segmentation fault
    tvSensorsModel->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter); // а так - не вылетает
    tvSensorsModel->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);
    if(n <= 0) {
        //qDebug() << "Список датчиков пуст, выходим";
        ui->tvSensors->hideColumn(0);// ИД
        if(!ui->chbxAllFields->isChecked()) for (i = 3; i < tvSensorsModel->columnCount(); ++i) ui->tvSensors->hideColumn(i);
        clearSensorParams();
        return;
    }

    // заполнить модель
    i = -1;
    while (query->next()) {
        i++;
        tvSensorsModel->setItem(i, colIdx_SensorId, new QStandardItem(query->value("id").toString()));
        // столбец 1 - номер записи с индикацией состояния записи
        tvSensorsModel->setItem(i, colIdx_SensorName, new QStandardItem(query->value("sensor_name").toString()));
        tvSensorsModel->setItem(i, colIdx_SensorTypeId, new QStandardItem(query->value("sensor_type").toString()));
        tvSensorsModel->setItem(i, colIdx_SensorTypeName, new QStandardItem(query->value("sensor_type_name").toString()));
        tvSensorsModel->setItem(i, colIdx_SensorTypeDesc, new QStandardItem(query->value("sensor_type_desc").toString()));
        tvSensorsModel->setItem(i, colIdx_TankId, new QStandardItem(query->value("tank_id").toString()));
        tvSensorsModel->setItem(i, colIdx_TankName, new QStandardItem(query->value("tank_name").toString()));
        tvSensorsModel->setItem(i, colIdx_ConsumerId, new QStandardItem(query->value("consumer_id").toString()));
        tvSensorsModel->setItem(i, colIdx_ConsumerName, new QStandardItem(query->value("consumer_name").toString()));
        tvSensorsModel->setItem(i, colIdx_NetAddress, new QStandardItem(query->value("net_address").toString()));
        tvSensorsModel->setItem(i, colIdx_NetIdx, new QStandardItem(query->value("net_idx").toString()));
        tvSensorsModel->setItem(i, colIdx_DataTypeId, new QStandardItem(query->value("data_type").toString()));
        tvSensorsModel->setItem(i, colIdx_DataTypeName, new QStandardItem(query->value("data_type_name").toString()));
        tvSensorsModel->setItem(i, colIdx_LoVal, new QStandardItem(query->value("lo_val").toString()));
        tvSensorsModel->setItem(i, colIdx_HiVal, new QStandardItem(query->value("high_val").toString()));
        tvSensorsModel->setItem(i, colIdx_RecordState, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        si->setIcon(QIcon(":/Apply.ico"));
        //tvSensorsModel->setVerticalHeaderItem(i, si); // это работает только у TableView, у TreeView - игнорируется
        tvSensorsModel->setItem(i, colIdx_RecNo, si);
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

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);

    tvSensorsModel->appendRow(QList<QStandardItem*>()
                      << new QStandardItem("-1") // 0= id
                      << si // new QStandardItem("0")  //1= record number
                      << new QStandardItem("")   //2= sensor_name
                      << new QStandardItem("-1") //3= sensor_type
                      << new QStandardItem("")   //4= sensor_type_name
                      << new QStandardItem("")   //5= sensor_type_desc
                      << new QStandardItem("-1") //6= tank_id
                      << new QStandardItem("")   //7= tank_name
                      << new QStandardItem("-1") //8= consumer_id
                      << new QStandardItem("")   //9= consumer_name
                      << new QStandardItem("-1") //10= net_address
                      << new QStandardItem("0")  //11= net_idx
                      << new QStandardItem("-1") //12= data_type
                      << new QStandardItem("")   //13= data_type_name
                      << new QStandardItem("1")  //14= lo_val
                      << new QStandardItem("2")  //15= high_val
                      << new QStandardItem(tr("%1").arg(rsAdded))); //16=record_state
    tvSensorsModel->submit();
    tvSensorsSelectionModel->reset();

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
    int recordState = tvSensorsModel->item(r, colIdx_RecordState)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvSensorsModel->item(r, colIdx_RecordState)->setData(rsDeleted, Qt::DisplayRole);
//        tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
//        tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        tvSensorsModel->item(r, 1)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        tvSensorsModel->item(r, 1)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        ui->pbnRemoveSensor->setText("Отменить");
        ui->pbnRemoveSensor->setToolTip("Отменить удаление записи");
        ui->pbnRemoveSensor->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvSensorsModel->item(r, colIdx_SensorId)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvSensorsModel->item(r, colIdx_RecordState)->setData(rsAdded, Qt::DisplayRole);
            //tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            //tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
            tvSensorsModel->item(r, colIdx_RecNo)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvSensorsModel->item(r, colIdx_RecNo)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvSensorsModel->item(r, colIdx_RecordState)->setData(rsChanged, Qt::DisplayRole);
            //tvSensorsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            //tvSensorsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
            tvSensorsModel->item(r, colIdx_RecNo)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvSensorsModel->item(r, colIdx_RecNo)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveSensor->setText("Удалить");
        ui->pbnRemoveSensor->setToolTip("Удалить запись");
        ui->pbnRemoveSensor->setStatusTip("Удалить запись");
    }
    //checkSensorButtons();
}

void NetEdit::saveSensors()
{
    int n = tvSensorsModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int ru = 0;
    int rd = 0;
    int errCount = 0;

    QString qryInsert = tr("insert into sensors(project_id, sensor_type, tank_id, consumer_id, net_address, net_idx, data_type, lo_val, high_val) "\
                        "values (%1, :sensor_type, :tank_id, :consumer_id, :net_address, :net_idx, :data_type, :lo_val, :high_val);").arg(currentProjectId);
//    QString qryInsert = "insert into sensors(project_id, data_type, sensor_type, tank_id, consumer_id, net_address, net_idx, data_type, lo_val, high_val) "\
//                        "values (:project_id, :data_type, :sensor_type, :tank_id, :consumer_id, :net_address, :net_idx, :data_type, :lo_val, :high_val);";
    QString qryUpdate = "update sensors set sensor_type=:sensor_type, tank_id=:tank_id, consumer_id=:consumer_id, net_address=:net_address, net_idx=:net_idx, "\
                        "data_type=:data_type, lo_val=:lo_val, high_val=:high_val where id = :id;";
    QString qryDelete = "delete from sensors where id = :id;";// and project_id = :project_id;";
    QString tankName, consName;//, tankId, consId;
    int tankId, consId;
    query->finish();

    for (int i = 0; i < n; ++i) {
        bool nothingToDo = false;
        switch (tvSensorsModel->item(i, colIdx_RecordState)->data(Qt::DisplayRole).toInt()) {
        //int rs = tvSensorsModel->item(i, sensors_record_state_idx)->data(Qt::DisplayRole).toInt();
        //switch (rs) {
        case rsAdded:
            ri++;
            qDebug() << "Insert query:\n" << qryInsert;
            query->prepare(qryInsert);
            query->bindValue(":project_id", currentProjectId);
            query->bindValue(":sensor_type", tvSensorsModel->item(i, colIdx_SensorTypeId)->data(Qt::DisplayRole).toInt());
            tankId = tvSensorsModel->item(i, colIdx_TankId)->data(Qt::DisplayRole).toInt();
            tankName = tvSensorsModel->item(i, colIdx_TankName)->data(Qt::DisplayRole).toString().trimmed();
            if(!tankName.isEmpty()) query->bindValue(":tank_id", tankId);
            consId = tvSensorsModel->item(i, colIdx_ConsumerId)->data(Qt::DisplayRole).toInt();
            consName = tvSensorsModel->item(i, colIdx_ConsumerName)->data(Qt::DisplayRole).toString().trimmed();
            if(!consName.isEmpty()) query->bindValue(":consumer_id", consId);
            query->bindValue(":net_address", tvSensorsModel->item(i, colIdx_NetAddress)->data(Qt::DisplayRole).toInt());
            query->bindValue(":net_idx", tvSensorsModel->item(i, colIdx_NetIdx)->data(Qt::DisplayRole).toInt());
            query->bindValue(":data_type", tvSensorsModel->item(i, colIdx_DataTypeId)->data(Qt::DisplayRole).toInt());
            query->bindValue(":lo_val", tvSensorsModel->item(i, colIdx_LoVal)->data(Qt::DisplayRole).toFloat());
            query->bindValue(":high_val", tvSensorsModel->item(i, colIdx_HiVal)->data(Qt::DisplayRole).toFloat());
            break;
        case rsChanged:
            ru++;
            query->prepare(qryUpdate);
            query->bindValue(":id", tvSensorsModel->item(i, colIdx_SensorId)->data(Qt::DisplayRole).toInt());
            query->bindValue(":data_type", tvSensorsModel->item(i, colIdx_DataTypeId)->data(Qt::DisplayRole).toInt());
            query->bindValue(":sensor_type", tvSensorsModel->item(i, colIdx_SensorTypeId)->data(Qt::DisplayRole).toInt());
            tankId = tvSensorsModel->item(i, colIdx_TankId)->data(Qt::DisplayRole).toInt();
            tankName = tvSensorsModel->item(i, colIdx_TankName)->data(Qt::DisplayRole).toString().trimmed();
            if(!tankName.isEmpty()) query->bindValue(":tank_id", tankId);
            consId = tvSensorsModel->item(i, colIdx_ConsumerId)->data(Qt::DisplayRole).toInt();
            consName = tvSensorsModel->item(i, colIdx_ConsumerName)->data(Qt::DisplayRole).toString().trimmed();
            if(!consName.isEmpty()) query->bindValue(":consumer_id", consId);
            query->bindValue(":net_address", tvSensorsModel->item(i, colIdx_NetAddress)->data(Qt::DisplayRole).toInt());
            query->bindValue(":net_idx", tvSensorsModel->item(i, colIdx_NetIdx)->data(Qt::DisplayRole).toInt());
            query->bindValue(":lo_val", tvSensorsModel->item(i, colIdx_LoVal)->data(Qt::DisplayRole).toFloat());
            query->bindValue(":high_val", tvSensorsModel->item(i, colIdx_HiVal)->data(Qt::DisplayRole).toFloat());
            break;
        case rsDeleted:
            {
                int delId = tvSensorsModel->item(i, colIdx_SensorId)->data(Qt::DisplayRole).toInt();
                if (delId < 0) {
                    nothingToDo = true;
                } else {
                    rd++;
                    // удалить записи из зависимых (связных) таблиц
                    if(!query->exec(tr("delete from ... where sensor_id = %1").arg(delId))) {
                        //err_ocuped = true;
                        errCount++;
                        QString errSt = query->lastError().text();
                        qDebug() << "Ошибка удаления данных о ... " << tvSensorsModel->item(i, 2)->data(Qt::DisplayRole).toString().trimmed() << ":";
                        qDebug() << errSt;
                        logMsg(msgError, tr("Ошибка удаления данных о датчике '%1':<br>%2")
                                        .arg(tvSensorsModel->item(i, colIdx_SensorName)->data(Qt::DisplayRole).toString().trimmed())
                                        .arg(errSt), ui->teLog);
                        nothingToDo = true; // не смогли - датчик не удаляем
                        // хотя из-за наличия ссылки на него - и не получится
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
            errCount++;
            QString errSt = query->lastError().text();
            qDebug() << "Ошибка сохранения данных:";
            qDebug() << errSt;
            qDebug() << "Текст запроса:";
            qDebug() << query->lastQuery();
            logMsg(msgError, tr("Ошибка сохранения данных:<br>%1").arg(errSt), ui->teLog);
            logMsg(msgError, tr("Текст запроса:<br>%1").arg(query->lastQuery()), ui->teLog);
            //break;
        }
        query->finish();
    }
    tvSensorsModel->submit();
    QString resultMess = "Обновлены данные списка датчиков<br>";
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    // обновление основной конфигурации
    updateSensorsList();
}

void NetEdit::restoreSensors()
{
    updateSensorsList();
}

bool NetEdit::validSensorsData()
{
    bool vd;
    for (int i = 0; i < tvSensorsModel->rowCount(); ++i) {
        vd = (tvSensorsModel->item(i, colIdx_SensorTypeId)->data(Qt::DisplayRole).toInt() >= 0) // sensor_type
                && ((tvSensorsModel->item(i, colIdx_TankId)->data(Qt::DisplayRole).toInt() >= 0)
                     || (tvSensorsModel->item(i, colIdx_ConsumerId)->data(Qt::DisplayRole).toInt() >= 0)) // tankId/consId
                && (tvSensorsModel->item(i, colIdx_NetAddress)->data(Qt::DisplayRole).toInt() >= 0) //net_address
                && (tvSensorsModel->item(i, colIdx_NetIdx)->data(Qt::DisplayRole).toInt() >= 0) // net_index
                && (tvSensorsModel->item(i, colIdx_DataTypeId)->data(Qt::DisplayRole).toInt() >= 0) // data_type
                && (tvSensorsModel->item(i, colIdx_LoVal)->data(Qt::DisplayRole).toFloat()
                    < tvSensorsModel->item(i, colIdx_HiVal)->data(Qt::DisplayRole).toFloat()); // lo_val < high_val
        if(!vd) break;
    }
    return vd;
}

void NetEdit::checkSensorsButtons()
{
    bool dataValid = validSensorsData();
    ui->pbnAddSensor->setEnabled(dataValid);
    ui->pbnRemoveSensor->setEnabled(tvSensorsSelectionModel->currentIndex().row() >= 0);
    ui->pbnSaveSensorsList->setEnabled(dataValid);
    ui->pbnRestoreSensorsList->setEnabled(true);
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
    // 2 - sensor_name
    if(tvSensorsModel->item(cr, colIdx_RecordState)->data(Qt::DisplayRole).toInt() == rsAdded) {
        QString placeName = ui->rbnTank->isChecked()? ui->cbxTanksList->currentText() : ui->cbxConsumersList->currentText();
        tvSensorsModel->item(cr, colIdx_SensorName)->setText(ui->cbxSensorTypes->currentText().append(" - ").append(placeName));
    }
    tvSensorsModel->item(cr, colIdx_SensorTypeId)->setData(cbxSensorTypes.at(ui->cbxSensorTypes->currentIndex()), Qt::DisplayRole);
    tvSensorsModel->item(cr, colIdx_SensorTypeName)->setText(ui->cbxSensorTypes->currentText());

    if(ui->rbnTank->isChecked()) { // датчик на танке
        tvSensorsModel->item(cr, colIdx_TankId)->setData(ui->cbxTanksList->currentData(), Qt::DisplayRole); // tank_id
        tvSensorsModel->item(cr, colIdx_TankName)->setText(ui->cbxTanksList->currentText()); // tank_name
        tvSensorsModel->item(cr, colIdx_ConsumerId)->setData(0, Qt::DisplayRole); // consumer_id
        tvSensorsModel->item(cr, colIdx_ConsumerName)->setText(""); // consumer_name
    } else { // датчик на потребителе
        tvSensorsModel->item(cr, colIdx_TankId)->setData(0, Qt::DisplayRole); // tank_id
        tvSensorsModel->item(cr, colIdx_TankName)->setText(""); // tank_name
        tvSensorsModel->item(cr, colIdx_ConsumerId)->setData(ui->cbxConsumersList->currentData(), Qt::DisplayRole); // consumer_id
        tvSensorsModel->item(cr, colIdx_ConsumerName)->setText(ui->cbxConsumersList->currentText()); // consumer_name
    }

    tvSensorsModel->item(cr, colIdx_NetAddress)->setText(ui->edAddress->text());   // net_address
    tvSensorsModel->item(cr, colIdx_NetIdx)->setText(ui->edIndex->text());     // net_idx
    tvSensorsModel->item(cr, colIdx_DataTypeId)->setData(ui->cbxDataTypes->currentIndex() + 1, Qt::DisplayRole);
    tvSensorsModel->item(cr, colIdx_DataTypeName)->setText(ui->cbxDataTypes->currentText());
    //tvSensorsModel->item(cr, colIdx_LoVal)->setText(ui->edLowVal->text());    // lo_val
    tvSensorsModel->item(cr, colIdx_LoVal)->setData(ui->edLowVal->value(), Qt::DisplayRole);
    //tvSensorsModel->item(cr, colIdx_HiVal)->setText(ui->edHiVal->text());     // high_val
    tvSensorsModel->item(cr, colIdx_HiVal)->setData(ui->edHiVal->value(), Qt::DisplayRole);

    int rs = tvSensorsModel->item(cr, colIdx_RecordState)->data(Qt::DisplayRole).toInt();;
    if(rs == rsUnchanged) {
        tvSensorsModel->item(cr, colIdx_RecordState)->setData(rsChanged, Qt::DisplayRole);
        tvSensorsModel->item(cr, 1)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvSensorsModel->item(cr, 1)->setToolTip(tr("Запись %1 изменена").arg(cr + 1));
        ui->pbnRemoveSensor->setText("Удалить");
    } else
        if(rs == rsDeleted) {
            logMsg(msgFail, "Запись удалена, при сохранении изменения будут потеряны!", ui->teLog);
            ui->pbnRemoveSensor->setText("Восстановить");
        } else {
            ui->pbnRemoveSensor->setText("Удалить");
        }

    ui->pbnSaveSensorsList->setEnabled(true);
    ui->pbnRestoreSensorsList->setEnabled(true);
    ui->pbnAddSensor->setEnabled(true);
    ui->pbnRemoveSensor->setEnabled(true);
}

// восстановить исходные данные датчика
void NetEdit::restoreSensorParams()
{
    int cr = ui->tvSensors->currentIndex().row();
    if(cr < 0) {
        clearSensorParams();
        return;
    }
    int n = tvSensorsModel->item(cr, colIdx_SensorTypeId)->data(Qt::DisplayRole).toInt();
    n = cbxSensorTypes.indexOf(n);
    if((ui->cbxSensorTypes->count() > 0) && (n >= 0) &&  (n < ui->cbxSensorTypes->count())) ui->cbxSensorTypes->setCurrentIndex(n);
    else ui->cbxSensorTypes->setCurrentIndex(0);
    QString fVal = tvSensorsModel->item(cr, colIdx_ConsumerName)->data(Qt::DisplayRole).toString().trimmed();
    bool consumerEmpty = fVal.isEmpty();
    ui->rbnTank->setChecked(consumerEmpty);
    ui->rbnConsumer->setChecked(!consumerEmpty);
    setPlaceList();
    if(!consumerEmpty) { // датчик на потребителе
        //fVal = tvSensorsModel->item(cr, colIdx_ConsumerId)->data(Qt::DisplayRole).toString().trimmed();
        //n = fVal.toInt();
        ui->cbxConsumersList->setCurrentIndex(cbxConsumers.indexOf(tvSensorsModel->item(cr, colIdx_ConsumerId)->data(Qt::DisplayRole).toInt()));//n));
    } else { // датчик на танке
        //fVal = tvSensorsModel->item(cr, colIdx_TankId)->data(Qt::DisplayRole).toString().trimmed();
        //n = fVal.toInt();
        ui->cbxTanksList->setCurrentIndex(cbxTanks.indexOf(tvSensorsModel->item(cr, colIdx_TankId)->data(Qt::DisplayRole).toInt()));//n));
    }

    ui->edAddress->setValue(tvSensorsModel->item(cr, colIdx_NetAddress)->data(Qt::DisplayRole).toInt());
    ui->edIndex->setValue(tvSensorsModel->item(cr, colIdx_NetIdx)->data(Qt::DisplayRole).toInt());
    ui->edLowVal->setValue(tvSensorsModel->item(cr, colIdx_LoVal)->data(Qt::DisplayRole).toFloat());
    ui->edHiVal->setValue(tvSensorsModel->item(cr, colIdx_HiVal)->data(Qt::DisplayRole).toFloat());
    fSensorParamsChanged = false;
}

// проверка корректности данных
bool NetEdit::validSensorParamsData()
{
    bool sensorDeleted = tvSensorsModel->item(tvSensorsSelectionModel->currentIndex().row(), colIdx_RecordState)->data(Qt::DisplayRole) == rsDeleted;
    if(sensorDeleted) return true;

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
    //bool dataValid = validSensorParamsData();
    //ui->pbnAddSensor->setEnabled(dataValid);
}

//-----------------------------------------------------------------
void NetEdit::updateDataTypes()
{
    query->clear();
    if(!query->exec("SELECT id, name FROM data_types ORDER BY 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов данных:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов данных:";
        qDebug() << query->lastError().text();
        return;
    }
    int n = query->size();
    ui->cbxDataTypes->clear(); // комбо-бокс
    cbxDataTypes.clear(); // список индексов по-порядку
    while (query->next()) {
        int idVal = query->value("id").toInt();
        ui->cbxDataTypes->addItem(query->value("name").toString(), idVal);
        cbxDataTypes << idVal;
    }
    if(n > 0) ui->cbxSensorTypes->setCurrentIndex(0);
}

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
    } else
    if(refTableName == "data_types") {
        updateDataTypes();
    } else return;

    if(this->isVisible()) updateSensorsList();
}

// смена датчика в списке - обновление параметров и проверка - удален / нет
void NetEdit::tvSensorsRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    if(fSensorParamsChanged) {
        saveSensorParams(idxPrev.row());
    }
    //qDebug() << "tvSensorsRowChanged from " << idxPrev.row() << " to " << idxCurrent.row() << ", current row: " << ui->tvSensors->currentIndex().row();
    currentSensorId = tvSensorsModel->data(tvSensorsModel->index(idxCurrent.row(), colIdx_SensorId), Qt::DisplayRole).toInt();
    currentSensorName = tvSensorsModel->data(tvSensorsModel->index(idxCurrent.row(), colIdx_SensorName), Qt::DisplayRole).toInt();

    int r = ui->tvSensors->currentIndex().row();
    //qDebug() << "model row count: " << tvSensorsModel->rowCount() << "current index row: " << r;
    //int recordState = tvSensorsModel->item(r, sensors_record_state_idx)->data(Qt::DisplayRole).toInt();
    QStandardItem* ci = tvSensorsModel->item(r, colIdx_RecordState);
    int recordState;
    if(ci) recordState = ci->data(Qt::DisplayRole).toInt();
    else recordState = rsAdded;
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
    checkSensorsButtons();
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

