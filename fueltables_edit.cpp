#include "fueltables_edit.h"
#include "ui_fueltables_edit.h"
#include <QSplitter>

#include "u_log.h"

FuelTables::FuelTables(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FuelTables)
{
    // форма
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowIcon(QIcon(":/List.ico"));

    ui->frame_1->setLayout(ui->glFuelsList);
    ui->frame_2->setLayout(ui->glRecalc);
    ui->frame_3->setLayout(ui->horizontalLayout);

    int w = this->width();
    int h = this->height();
    int twoThirdH = h * 2 / 3;

    // "резинки" панелей
    QSplitter* sp1 = new QSplitter(Qt::Horizontal, this);
    sp1->setGeometry(0, 0, w, twoThirdH);
    sp1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp1->addWidget(ui->frame_1);
    sp1->addWidget(ui->frame_2);

    QSplitter* sp = new QSplitter(Qt::Vertical, this);
    sp->setGeometry(this->rect());
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp->addWidget(sp1);
    sp->addWidget(ui->frame_3);
    this->setCentralWidget(sp);

    query =  new QSqlQuery();

    //-------------------------------------------------
    // таблица видов топлива
    currentFuelId = -1;
    currentFuelName = "";
    // индекс скрытого столбца - состояния записи
    fuel_record_state_idx = 3;

    // модели, делегаты и пр.- БД
    query = new QSqlQuery();

    // стандартные модели
    tvFuelsModel = new QStandardItemModel(ui->tvFuels);
    //tvFuelsModel->sets
    tvFuelsSelectionModel = new QItemSelectionModel(tvFuelsModel);
    // задать размер модели, заполнить модель, скрыть служебные колонки
    updateFuelsList();
    ui->tvFuels->setModel(tvFuelsModel);
    ui->tvFuels->setSelectionModel(tvFuelsSelectionModel);
    ui->tvFuels->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку
    // отлов изменений через модель
    connect(tvFuelsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellFuelsChanged(QStandardItem*)));
    // сменена строка содержимого - выдаем список измерений, и разобраться - удалена запись или нет
    connect(ui->tvFuels->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvFuelsRowChanged(QModelIndex,QModelIndex)));

    // таблица пересчета
    tvRecalcsModel = new QStandardItemModel(ui->tvRecalcs);
    tvRecalcSelectionModel = new QItemSelectionModel(tvRecalcsModel);
    ui->tvRecalcs->setModel(tvRecalcsModel);
    ui->tvRecalcs->setSelectionModel(tvRecalcSelectionModel);
    ui->tvRecalcs->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку

    recalc_record_state_idx = -1;
    recalc_temp_idx = -1;
    recalc_density_idx = -1;

    // задать размер модели, заполнить модель, скрыть служебные колонки
    updateRecalcsList();
    // создать делегат
    tvRecalcDelegate = new UniversalDelegate();
    tvRecalcDelegate->setParent(tvRecalcsModel);
    // для колонок пересчета тип делегата - дробное число // целое и дробное
    tvRecalcDelegate->setColumnDelegate(1, dtRealOrIntNumber);//dtSpinBox);
    tvRecalcDelegate->setColumnDelegate(2, dtRealOrIntNumber);
    tvRecalcDelegate->setClipping(true);
    ui->tvRecalcs->setItemDelegate(tvRecalcDelegate);

    // с графическим редактированием таблицы - разберемся позже (если будет время)
//    ui->frame_2->setVisible(false);

    // сменена строка содержимого, разобраться - удалена запись или нет
    connect(ui->tvRecalcs->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvRecalcRowChanged(QModelIndex,QModelIndex)));

    ui->tvFuels->setCurrentIndex(tvFuelsModel->index(0, 1));
}

FuelTables::~FuelTables()
{
    query->finish();
    delete ui;
}

void FuelTables::userStyleChanged()
{
    qDebug() << "FuelTables - verticalHeader width: " << ui->tvFuels->verticalHeader()->width();
    //ui->tvFuels->verticalHeader()->resetDefaultSectionSize();
    //ui->tvFuels->verticalHeader()->resizeContentsPrecision();
    //ui->tvFuels->verticalHeader()->ResizeToContents();
    ui->tvFuels->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //ui->tvFuels->resizeColumnToContents(0);
    qDebug() << "FuelTables - verticalHeader width after reset: " << ui->tvFuels->verticalHeader()->width();
}

// динамическое обновление ссылок на справочники
void FuelTables::refTableChanged(QString refTableName)
{
    if(refTableName == "fuel_types") updateFuelsList();
}

//-----------------------------------------------------------------------
void FuelTables::updateFuelsList()
{
    QString msg;
    if(!query->driver()->isOpen()) {
        msg = tr("Нет соединения с БД проекта, ошибка:<br>").arg(query->driver()->lastError().text());
        logMsg(msgError, msg, ui->teLog);
        qDebug() << "Нет соединения с БД проекта, ошибка:";
        qDebug() << query->driver()->lastError().text();
        return;
    }
    query->finish();
    query->clear();


    QString st = "select * from fuel_types order by 1;";
    if(!query->exec(st)) {
        logMsg(msgError, tr("Ошибка получения списка видов топлива:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка видов топлива:";
        qDebug() << query->lastError().text();
        return;
    }

    int n = query->size();
    // отключаем обработку сигналов модели
    disconnect(tvFuelsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellFuelsChanged(QStandardItem*)));
    // задать размер модели
    tvFuelsModel->clear();
    tvFuelsModel->setColumnCount(4);
    tvFuelsModel->setRowCount(n);
    tvFuelsModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "Наименование" << "Описание" << "Состояние");
    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;
        tvFuelsModel->setItem(i, 0, new QStandardItem(query->value("id").toString()));
        tvFuelsModel->setItem(i, 1, new QStandardItem(query->value("name").toString()));
        tvFuelsModel->setItem(i, 2, new QStandardItem(query->value("description").toString()));
        tvFuelsModel->setItem(i, 3, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvFuelsModel->setVerticalHeaderItem(i, si);
    }
    // индексы столбцов с комбо-боксами
    query->finish();

    ui->tvFuels->setModel(tvFuelsModel);
    tvFuelsSelectionModel->setModel(tvFuelsModel);
    ui->tvFuels->setSelectionModel(tvFuelsSelectionModel);
    ui->tvFuels->hideColumn(0);// ИД
    ui->tvFuels->hideColumn(fuel_record_state_idx);// состояние записи
    ui->tvFuels->resizeColumnToContents(1);
    // кнопки - что можно / что нельзя
    ui->pbnAddFuel->setEnabled(true);
    ui->pbnRemoveFuel->setEnabled(n > 0);
    ui->pbnSaveFuelsList->setEnabled(false);
    ui->pbnRestoreFuelsList->setEnabled(false);

    connect(tvFuelsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellFuelsChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvFuels->setCurrentIndex(tvFuelsModel->index(0, 1));
    ui->tvFuels->setFocus();
}

void FuelTables::addFuel()
{
    int n = tvFuelsModel->rowCount();
    tvFuelsModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem("")
                       << new QStandardItem("")
                       << new QStandardItem(tr("%1").arg(rsAdded)));

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvFuelsModel->setVerticalHeaderItem(n, si);
    tvFuelsModel->submit();

    ui->tvFuels->setCurrentIndex(tvFuelsModel->index(n, 1));
    ui->tvFuels->setFocus();

    ui->pbnAddFuel->setEnabled(false);
    ui->pbnRemoveFuel->setEnabled(true);
    ui->pbnSaveFuelsList->setEnabled(false);
    ui->pbnRestoreFuelsList->setEnabled(true);
}

void FuelTables::removeFuel()
{
    int r = ui->tvFuels->currentIndex().row();
    int recordState = tvFuelsModel->item(r, fuel_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvFuelsModel->item(r, fuel_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvFuelsModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvFuelsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveFuel->setText("Отменить");
        ui->pbnRemoveFuel->setToolTip("Отменить удаление записи");
        ui->pbnRemoveFuel->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvFuelsModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvFuelsModel->item(r, fuel_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvFuelsModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvFuelsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvFuelsModel->item(r, fuel_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvFuelsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvFuelsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveFuel->setText("Удалить");
        ui->pbnRemoveFuel->setToolTip("Удалить запись");
        ui->pbnRemoveFuel->setStatusTip("Удалить запись");
    }
    checkFuelButtons();
}

void FuelTables::saveFuels()
{
    int n = tvFuelsModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int ru = 0;
    int rd = 0;
    int errCount = 0;
    bool nothingToDo;

    QString qryInsert = "insert into fuel_types (name, description) values(:name, :description);";
    QString qryUpdate = "update fuel_types set name = :name, description = :description where id = :id";
    QString qryDelete = "delete from fuel_types where id = :id";
    query->finish();

    for (int i = 0; i < n; ++i) {
        nothingToDo = false;
        //int recordState = tvFuelModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        //switch (recordState) {
        switch (tvFuelsModel->item(i, 3)->data(Qt::DisplayRole).toInt()) {
        case rsAdded:
            ri++;
            query->prepare(qryInsert);
            query->bindValue(":name", tvFuelsModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed());
            query->bindValue(":description", tvFuelsModel->item(i, 2)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsChanged:
            ru++;
            query->prepare(qryUpdate);
            query->bindValue(":id", tvFuelsModel->item(i, 0)->data(Qt::DisplayRole).toInt());
            query->bindValue(":name", tvFuelsModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed());
            query->bindValue(":description", tvFuelsModel->item(i, 2)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsDeleted:
            {
                int delId = tvFuelsModel->item(i, 0)->data(Qt::DisplayRole).toInt();
                if (delId < 0) {
                    nothingToDo = true;
                } else {
                    rd++;
                    query->prepare(qryDelete);
                    query->bindValue(":id", delId);
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
            logMsg(msgError, tr("Ошибка сохранения данных:<br>%1").arg(errSt));
            //break;
        }
        query->finish();
    }
    tvFuelsModel->submit();
    QString resultMess = tr("Обновлены данные таблицы 'Типы топлива'<br>");
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount));
    updateFuelsList();
    //qDebug() << "Обновление данных таблицы " << fRefRusName <<  " закончено";
}

void FuelTables::restoreFuels()
{
    updateFuelsList();
}

bool FuelTables::validFuelsData()
{
    int n = tvFuelsModel->rowCount();
    if(n == 0) return true;

    bool vd = true;
    for (int i = 0; i < n; ++i) {
        int recordState = tvFuelsModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        if(recordState == rsDeleted) {
            continue;
        } else {
            if(tvFuelsModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed().isEmpty()) {
                vd = false;
                break;
            }
        }
    }
    return vd;
}

void FuelTables::checkFuelButtons()
{
    bool validData = validFuelsData();
    bool validAndNotEmpty = validData & (tvFuelsModel->rowCount() > 0);
    ui->pbnAddFuel->setEnabled(validData);
    ui->pbnRemoveFuel->setEnabled(tvFuelsModel->rowCount() > 0);
    ui->pbnSaveFuelsList->setEnabled(validAndNotEmpty);
    ui->pbnRestoreFuelsList->setEnabled(true);
}

//-----------------------------------------------------------------------
void FuelTables::cellFuelsChanged(QStandardItem *item)
{
    int r = item->row();
    //qDebug() << "tvFuelsModel->itemChanged: " << r << ":" << item->column();
    int recordState = tvFuelsModel->item(r, 3)->data(Qt::DisplayRole).toInt();
    if((recordState != rsAdded) & (recordState != rsDeleted) & (recordState != rsChanged)) {
        tvFuelsModel->item(r, 3)->setData(rsChanged, Qt::DisplayRole);
        tvFuelsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvFuelsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkFuelButtons();
}

void FuelTables::tvFuelsRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    currentFuelId = tvFuelsModel->data(tvFuelsModel->index(idxCurrent.row(), 0), Qt::DisplayRole).toInt();
    currentFuelName = tvFuelsModel->data(tvFuelsModel->index(idxCurrent.row(), 1), Qt::DisplayRole).toString();
    ui->leFuel->setText(currentFuelName);
    //qDebug() << "tvFuelsRowChanged: "  << idxCurrent.row() << ", ИД топлива: " << currentFuelId;
    // обновить таблицу пересчета
    updateRecalcsList();
    // разобраться - удалена запись или нет
    int recordState = tvFuelsModel->item(idxCurrent.row(), fuel_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveFuel->setText("Отменить");
        ui->pbnRemoveFuel->setToolTip("Отменить удаление записи");
        ui->pbnRemoveFuel->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveFuel->setText("Удалить");
        ui->pbnRemoveFuel->setToolTip("Удалить запись");
        ui->pbnRemoveFuel->setStatusTip("Удалить запись");
    }
}

//-----------------------------------------------------------------------
void FuelTables::on_pbnAddFuel_clicked()
{
    addFuel();
}

void FuelTables::on_pbnRemoveFuel_clicked()
{
    removeFuel();
}

void FuelTables::on_pbnSaveFuelsList_clicked()
{
    saveFuels();
}

void FuelTables::on_pbnRestoreFuelsList_clicked()
{
    restoreFuels();
}

//============================================================================
void FuelTables::updateRecalcsList()
{
    // отключаем обработку сигналов модели
    disconnect(tvRecalcsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellRecalcChanged(QStandardItem*)));

    tvRecalcsModel->clear();
    tvRecalcsModel->setColumnCount(4);
    tvRecalcsModel->setHorizontalHeaderLabels(QStringList() << "ИД пересчета" << "Температура" << "Плотность" << "Состояние");
    ui->tvRecalcs->hideColumn(0);
    ui->tvRecalcs->hideColumn(3);

    recalc_record_state_idx = 3;// столбец состояния записи

    if(currentFuelId < 0) {
        return;
    }

    QString msg;
    if(!query->driver()->isOpen()) {
        msg = tr("Нет соединения с БД проекта, ошибка:<br>").arg(query->driver()->lastError().text());
        logMsg(msgError, msg, ui->teLog);
        qDebug() << "Нет соединения с БД проекта, ошибка:";
        qDebug() << query->driver()->lastError().text();
        return;
    }
    query->finish();
    query->clear();

//    SELECT id, fuel_type_id, fuel_temp, fuel_density FROM fuel_density;

    QString st = "SELECT id, fuel_type_id, fuel_temp, fuel_density FROM fuel_density WHERE fuel_type_id = :fuel_type_id ORDER BY 1;";
    query->prepare(st);
    query->bindValue(":fuel_type_id", currentFuelId);
    if(!query->exec()) {
        logMsg(msgError, tr("Ошибка получения списка пересчета плотности топлива '%1':<br>%2").arg(currentFuelName).arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка пересчета плотности топлива:";
        qDebug() << query->lastError().text();
        return;
    }


    int n = query->size();
    // проверка на пустость не нужна - ничего не добавится, но привязки сделаются
    //if (n == 0) return;
    // задать размер модели
    tvRecalcsModel->setRowCount(n);
    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;
        tvRecalcsModel->setItem(i, 0, new QStandardItem(i));
        tvRecalcsModel->setItem(i, 1, new QStandardItem(QString::number(query->value("fuel_temp").toFloat(), 'f', 0)));
        tvRecalcsModel->setItem(i, 2, new QStandardItem(QString::number(query->value("fuel_density").toFloat(), 'f', 3)));
        tvRecalcsModel->setItem(i, 3, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvRecalcsModel->setVerticalHeaderItem(i, si);
    }
    // индексы столбцов с редакторами чисел
    recalc_temp_idx = query->record().indexOf("fuel_temp");
    recalc_density_idx = query->record().indexOf("fuel_density");
    query->finish();

    ui->tvRecalcs->setModel(tvRecalcsModel);
    tvRecalcSelectionModel->setModel(tvRecalcsModel);
    ui->tvRecalcs->setSelectionModel(tvRecalcSelectionModel);
    ui->tvRecalcs->hideColumn(0);// ИД
    ui->tvRecalcs->hideColumn(recalc_record_state_idx);// состояние записи
    // кнопки - что можно / что нельзя
    ui->pbnAddRecalc->setEnabled(true);
    ui->pbnRemoveRecalc->setEnabled(n > 0);
    ui->pbnSaveRecalcsList->setEnabled(false);
    ui->pbnRestoreRecalcsList->setEnabled(false);

    connect(tvRecalcsModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellRecalcChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvRecalcs->setCurrentIndex(tvRecalcsModel->index(0, 1));
    ui->tvRecalcs->setFocus();
}

void FuelTables::addRecalc()
{
    int n = tvRecalcsModel->rowCount();
    QString prevRecalcTemp = n > 0 ? QString::number(tvRecalcsModel->item(n - 1, 1)->data(Qt::DisplayRole).toInt() + 1) : "0";
    //QString prevRecalcVal = n > 0 ? tvRecalcsModel->item(n - 1, 2)->data(Qt::DisplayRole).toString() : "0.0";
    QString prevRecalcDens = n > 0 ? tvRecalcsModel->item(n - 1, 2)->data(Qt::DisplayRole).toString() : "0.100";

    tvRecalcsModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem(prevRecalcTemp)
                       << new QStandardItem(prevRecalcDens)
                       << new QStandardItem(tr("%1").arg(rsAdded)));

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvRecalcsModel->setVerticalHeaderItem(n, si);
    tvRecalcsModel->submit();

    ui->tvRecalcs->setCurrentIndex(tvRecalcsModel->index(n, 1));
    ui->tvRecalcs->setFocus();

    //ui->pbnAddRecalc->setEnabled(false);
    ui->pbnRemoveRecalc->setEnabled(true);
    ui->pbnSaveRecalcsList->setEnabled(true);
    ui->pbnRestoreRecalcsList->setEnabled(true);
}

void FuelTables::removeRecalc()
{
    int r = ui->tvRecalcs->currentIndex().row();
    int recordState = tvRecalcsModel->item(r, recalc_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvRecalcsModel->item(r, recalc_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvRecalcsModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvRecalcsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveRecalc->setText("Отменить");
        ui->pbnRemoveRecalc->setToolTip("Отменить удаление записи");
        ui->pbnRemoveRecalc->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvRecalcsModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvRecalcsModel->item(r, recalc_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvRecalcsModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvRecalcsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvRecalcsModel->item(r, recalc_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvRecalcsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvRecalcsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveRecalc->setText("Удалить");
        ui->pbnRemoveRecalc->setToolTip("Удалить запись");
        ui->pbnRemoveRecalc->setStatusTip("Удалить запись");
    }
    checkRecalcButtons();
}

void FuelTables::saveRecalcs()
{
    int n = tvRecalcsModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int errCount = 0;

    // удалить все чохом, и добавить все по порядку
    query->finish();
    query->prepare("delete from fuel_density where fuel_type_id = :fuel_type_id;");
    query->bindValue(":fuel_type_id", currentFuelId);
    if(!query->exec()) {
        QString errSt = query->lastError().text();
        qDebug() << "Ошибка очистки данных:";
        qDebug() << errSt;
        logMsg(msgError, tr("Ошибка очистки данных пересчета плотности топлива '%1'<br>%2").arg(currentFuelName).arg(errSt), ui->teLog);
        // неизвестно, что там есть, поэтому выходим
        return;
    }
    query->finish();

    query->prepare("INSERT INTO fuel_density(fuel_type_id, fuel_temp, fuel_density) VALUES (:fuel_type_id, :fuel_temp, :fuel_density);");
    query->bindValue(":fuel_type_id", currentFuelId);
    for (int i = 0; i < n; ++i) {
        if(tvRecalcsModel->item(i, recalc_record_state_idx)->data(Qt::DisplayRole).toInt() == rsDeleted) continue;
        query->bindValue(":fuel_temp", tvRecalcsModel->item(i, 1)->data(Qt::DisplayRole).toFloat());
        query->bindValue(":fuel_density", tvRecalcsModel->item(i, 2)->data(Qt::DisplayRole).toFloat());
        if (!query->exec()) {
            errCount++;
            QString errSt = query->lastError().text();
            qDebug() << "Ошибка сохранения данных:";
            qDebug() << errSt;
            logMsg(msgError, tr("Ошибка сохранения данных пересчета плотности топлива '%1'<br>%2").arg(currentFuelName).arg(errSt), ui->teLog);
            //break;
        } else ri++;
    }
    query->finish();

    tvRecalcsModel->submit();// без этого - вылазит ошибка при обновлении
    QString resultMess = QString("Обновлены данные пересчета плотности топлива '%1'<br>").arg(currentFuelName);
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    updateRecalcsList();
}

void FuelTables::restoreRecalcs()
{
    updateRecalcsList();
}

bool FuelTables::validRecalcsData()
{
    bool vd = true;
    bool val1ok;
    bool val2ok;
    for (int i = 0; i < tvRecalcsModel->rowCount(); ++i) {
        if(tvRecalcsModel->item(i, 1)->data(Qt::DisplayRole).toFloat(&val1ok) <= -100 // температура
           || !val1ok
           || tvRecalcsModel->item(i, 2)->data(Qt::DisplayRole).toFloat(&val2ok) <= 0 // плотность
           || !val2ok
          )
        {
            qDebug() << "invalid Recalc data, row: " << i;
            vd = false;
            break;
        }
    }
    return vd;
}

void FuelTables::checkRecalcButtons()
{
    bool dataValid = (currentFuelId >= 0) & validRecalcsData();
    ui->pbnAddRecalc->setEnabled(dataValid);
    ui->pbnSaveRecalcsList->setEnabled(dataValid);
    ui->pbnRemoveRecalc->setEnabled(ui->tvRecalcs->currentIndex().row() >= 0);
    ui->pbnRestoreRecalcsList->setEnabled(true);
}

//-----------------------------------------------------------------------
void FuelTables::cellRecalcChanged(QStandardItem *item)
{
    int c = item->column();
    if((c > 2) || (c <= 0)) return;
    int r = item->row();
    qDebug() << "cellRecalcChanged to " << r << ":" << c;

    bool ok;
    float newVal = item->data(Qt::EditRole).toFloat(&ok);

    if(!ok) {
        qDebug() << "Uncorrect float data: " << newVal;
        ui->pbnSaveRecalcsList->setEnabled(false);
        ui->pbnAddRecalc->setEnabled(false);
        return;
    }

    if(c == 1) tvRecalcsModel->item(r, c)->setData(QString::number(newVal, 'f', 0), Qt::DisplayRole); // температура
    else tvRecalcsModel->item(r, c)->setData(QString::number(newVal, 'f', 3), Qt::DisplayRole); // плотность

    int recordState = tvRecalcsModel->item(r, recalc_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvRecalcsModel->item(r, recalc_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvRecalcsModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvRecalcsModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkRecalcButtons();
}

void FuelTables::tvRecalcRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    // разобраться - удалена запись или нет
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    int recordState = tvRecalcsModel->item(idxCurrent.row(), recalc_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveRecalc->setText("Отменить");
        ui->pbnRemoveRecalc->setToolTip("Отменить удаление записи");
        ui->pbnRemoveRecalc->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveRecalc->setText("Удалить");
        ui->pbnRemoveRecalc->setToolTip("Удалить запись");
        ui->pbnRemoveRecalc->setStatusTip("Удалить запись");
    }
}

//-----------------------------------------------------------------------
void FuelTables::on_pbnAddRecalc_clicked()
{
    addRecalc();
}

void FuelTables::on_pbnRemoveRecalc_clicked()
{
    removeRecalc();
}

void FuelTables::on_pbnSaveRecalcsList_clicked()
{
    saveRecalcs();
}

void FuelTables::on_pbnRestoreRecalcsList_clicked()
{
    restoreRecalcs();
}
