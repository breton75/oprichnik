#include "consumers_edit.h"
#include "ui_consumers_edit.h"
#include <QSplitter>

#include "u_log.h"
#include "ref_edit.h"

ConsumersEdit::ConsumersEdit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConsumersEdit)
{
    // форма
    ui->setupUi(this);

    setWindowModality(Qt::WindowModal);
    // ну, не выводится иконка в дочернем окне в линуксе...
//    this->window()->setAttribute(Qt::WA_SetWindowIcon, true);
//    this->setWindowIcon(QIcon("Smile.ico"));
    //this->setWindowIcon(QIcon("gimp.png"));
    setWindowIcon(QIcon(":/Consumer.png"));
////    this->setWindowFlags();
//    this->windowIcon().actualSize(QSize(48, 48), QIcon::Normal, QIcon::On);
//    //this->setWindowIcon(QPixmap("Smile.ico"));
//    //this->setWindowIcon(QPixmap(":/Retort.png"));
//    //this->setIcon(QIcon("Smile.ico"));
//    //this->setWindowIconText("TPO");
//    //this->setIconSize(QSize(48, 48));
//    //this->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
//    this->setWindowIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
//    //this->ic
////    this->set
//    setIconSize(QSize(48, 48));
    //setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);// | Qt::FramelessWindowHint);

    ui->frame_1->setLayout(ui->glConsumersList);
    ui->frame_2->setLayout(ui->glRunModes);
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

    //---------------------- сетка списка потребителей ---------------------------
    currentConsumerId = -1;
    currentConsumerName = "";
    // индексы столбцов с комбо-боксами - недостоверные значения на случай неудачи
    consumer_type_idx = -1;
    meter_type_idx = -1;
    // индекс скрытого столбца - состояния записи
    consumer_record_state_idx = 4;

    // модели, делегаты и пр.- БД

    // заполнить списки типов цистерн и датчиков
    updateConsumerTypes();
    updateMeterTypes();
    updateRunModeTypes();

    // стандартные модели
    tvConsumersModel = new QStandardItemModel(ui->tvConsumers);
    //tvConsumersModel->sets
    tvConsumersSelectionModel = new QItemSelectionModel(tvConsumersModel);
    // задать размер модели, заполнить модель, скрыть служебные колонки
    updateConsumersList();

    // создать делегат
    tvConsumersDelegate = new UniversalDelegate();//tvConsumersModel);
    tvConsumersDelegate->setClipping(true);
    tvConsumersDelegate->setParent(tvConsumersModel);
    // для колонок-ссылок тип делегата - комбо-бокс
    tvConsumersDelegate->setColumnDelegate(consumer_type_idx, dtComboBox);
    tvConsumersDelegate->setColumnDelegate(meter_type_idx, dtComboBox);
    // заполнить списки-справочники (с привязкой к конкретным столбцам)
    tvConsumersDelegate->setComboValuesForColumn(consumer_type_idx, cbxConsumerTypesVals);
    tvConsumersDelegate->setComboValuesForColumn(meter_type_idx, cbxMeterTypesVals);

    // задать делегат для таблицы
    ui->tvConsumers->setModel(tvConsumersModel);
    ui->tvConsumers->setSelectionModel(tvConsumersSelectionModel);
    ui->tvConsumers->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку
    ui->tvConsumers->setItemDelegate(tvConsumersDelegate);
    // отлов изменений через делегат
    //connect(tvConsumersDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)), this, SLOT(comboEditorClosed(QWidget*)));
    // отлов изменений через модель
    connect(tvConsumersModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellConsumersChanged(QStandardItem*)));

    // сменена строка содержимого - выдаем список измерений, и разобраться - удалена запись или нет
    connect(ui->tvConsumers->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvConsumersRowChanged(QModelIndex,QModelIndex)));
    //ui->tvConsumers->selectRow(0);

    //------------------------ сетка графиков пересчета ------------------------------------
    tvRunModesModel = new QStandardItemModel(ui->tvRunModes);
    tvRunModeSelectionModel = new QItemSelectionModel(tvRunModesModel);
    ui->tvRunModes->setModel(tvRunModesModel);
    ui->tvRunModes->setSelectionModel(tvRunModeSelectionModel);
    ui->tvRunModes->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку

    // задать размер модели, заполнить модель, скрыть служебные колонки
    updateRunModesList();
    // создать делегат
    // не работает
    tvRunModeDelegate = new UniversalDelegate();
    tvRunModeDelegate->setParent(tvRunModesModel);
    // для колонок-ссылок тип делегата - дробное
    tvRunModeDelegate->setColumnDelegate(run_mode_idx, dtComboBox);
    tvRunModeDelegate->setClipping(true);
    tvRunModeDelegate->setColumnDelegate(run_mode_val_idx, dtRealOrIntNumber);
    tvRunModeDelegate->setComboValuesForColumn(run_mode_idx, cbxRunModeTypesVals);
    ui->tvRunModes->setItemDelegate(tvRunModeDelegate);

    // с графическим редактированием таблицы - разберемся позже (если будет время)
//    ui->frame_2->setVisible(false);

    // сменена строка содержимого, разобраться - удалена запись или нет
    connect(ui->tvRunModes->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvRunModeRowChanged(QModelIndex,QModelIndex)));

    ui->tvConsumers->setCurrentIndex(tvConsumersModel->index(0, 1));
}

ConsumersEdit::~ConsumersEdit()
{
    query->finish();
    delete ui;
}

void ConsumersEdit::setProjectId(int newProjectId)
{
    currentProjectId = newProjectId;
    if(this->isVisible()) updateConsumersList();
}

// динамическое обновление ссылок на справочники
void ConsumersEdit::refTableChanged(QString refTableName)
{
    if(refTableName == "consumer_types") {
        updateConsumerTypes();
        tvConsumersDelegate->setComboValuesForColumn(consumer_type_idx, cbxConsumerTypesVals);
    }
    else if(refTableName == "meter_types") {
        updateMeterTypes();
        tvConsumersDelegate->setComboValuesForColumn(meter_type_idx, cbxMeterTypesVals);
    }
    else if(refTableName == "run_mode_types") {
        updateRunModeTypes();
        tvRunModeDelegate->setComboValuesForColumn(run_mode_idx, cbxRunModeTypesVals);
    }
    else return;
    updateConsumersList();
}

//----------------------------------------------------------------------------------
// справочник - список типов потребителей
void ConsumersEdit::updateConsumerTypes()
{
    if(!query->driver()->isOpen()) return;

    query->finish();
    if(!query->exec("select * from consumer_types order by 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов потребителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов потребителей:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список типов потребителей пуст", ui->teLog);
        qDebug() << "Ошибка - список типов потребителей пуст";
        return;
    }

    cbxConsumerTypesVals.clear();
    QPair<int, QString> parVal;
    while (query->next()) {
        parVal.first = query->value("id").toInt(); // i++;//
        parVal.second = query->value("name").toString();
        cbxConsumerTypesVals.append(parVal);
    }
    query->finish();
}

// справочник - список режимов работы
void ConsumersEdit::updateRunModeTypes()
{
    if(!query->driver()->isOpen()) return;

    query->finish();
    if(!query->exec("select * from run_mode_types order by 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка режимов работы:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка режимов работы:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список режимов работы пуст", ui->teLog);
        qDebug() << "Ошибка - список режимов работы пуст";
        return;
    }

    cbxRunModeTypesVals.clear();
    QPair<int, QString> parVal;
    while (query->next()) {
        parVal.first = query->value("id").toInt(); // i++;//
        parVal.second = query->value("name").toString();
        cbxRunModeTypesVals.append(parVal);
    }
    query->finish();
}

// справочник - список типов измерителей
void ConsumersEdit::updateMeterTypes()
{
    if(!query->driver()->isOpen()) return;

    query->finish();
    if(!query->exec("select * from meter_types order by 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов измерителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов измерителей:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список типов измерителей пуст", ui->teLog);
        qDebug() << "Ошибка - список типов измерителей пуст";
        return;
    }

    cbxMeterTypesVals.clear();
    QPair<int, QString> parVal;
    while (query->next()) {
        parVal.first = query->value("id").toInt(); // i++;//
        parVal.second = query->value("name").toString();
        cbxMeterTypesVals.append(parVal);
    }
    query->finish();
}

// список потребителей
void ConsumersEdit::updateConsumersList()
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


    QString st = tr("select id, consumer_type, meter_type, name from consumers where project_id = %1 order by 1;").arg(currentProjectId);// "select * from consumers order by 1;";
    if(!query->exec(st)) {
        logMsg(msgError, tr("Ошибка получения списка потребителей:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка потребителей:";
        qDebug() << query->lastError().text();
        return;
    }

    int n = query->size();
    // отключаем обработку сигналов модели
    disconnect(tvConsumersModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellConsumersChanged(QStandardItem*)));
    // задать размер модели
    tvConsumersModel->clear();
    tvConsumersModel->setColumnCount(5);
    tvConsumersModel->setRowCount(n);
    tvConsumersModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "Потребитель" << "Тип измерителя" << "Наименование" << "Состояние");
    //if(tvRunModesModel) tvRunModesModel->clear();
    currentConsumerId = -1;
    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;
        tvConsumersModel->setItem(i, 0, new QStandardItem(query->value("id").toString()));
        tvConsumersModel->setItem(i, 1, new QStandardItem(query->value("consumer_type").toString()));
        tvConsumersModel->setItem(i, 2, new QStandardItem(query->value("meter_type").toString()));
        tvConsumersModel->setItem(i, 3, new QStandardItem(query->value("name").toString()));
        tvConsumersModel->setItem(i, 4, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvConsumersModel->setVerticalHeaderItem(i, si);
    }
    // индексы столбцов с комбо-боксами
    consumer_type_idx = query->record().indexOf("consumer_type");
    meter_type_idx = query->record().indexOf("meter_type");
    query->finish();

    ui->tvConsumers->setModel(tvConsumersModel);
    tvConsumersSelectionModel->setModel(tvConsumersModel);
    ui->tvConsumers->setSelectionModel(tvConsumersSelectionModel);
    ui->tvConsumers->hideColumn(0);// ИД
    ui->tvConsumers->hideColumn(consumer_record_state_idx);// состояние записи
    // кнопки - что можно / что нельзя
    ui->pbnAddConsumer->setEnabled(true);
    ui->pbnRemoveConsumer->setEnabled(n > 0);
    ui->pbnSaveConsumersList->setEnabled(false);
    ui->pbnRestoreConsumersList->setEnabled(false);

    connect(tvConsumersModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellConsumersChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvConsumers->setCurrentIndex(tvConsumersModel->index(0, 1));
    ui->tvConsumers->setFocus();
}

// таблица расходов по режимам работы
void ConsumersEdit::updateRunModesList()
{
    // отключаем обработку сигналов модели
    disconnect(tvRunModesModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellRunModesChanged(QStandardItem*)));

    tvRunModesModel->clear();
    tvRunModesModel->setColumnCount(4);
    tvRunModesModel->setHorizontalHeaderLabels(QStringList() << "ИД расхода" << "Режим работы" << "Расход" << "Состояние");
    ui->tvRunModes->hideColumn(0);
    ui->tvRunModes->hideColumn(3);

    run_mode_idx = 1;// режим
    run_mode_val_idx = 2;// расход по режиму
    run_mode_record_state_idx = 3;// столбец состояния записи

    if(currentConsumerId < 0) {
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

    QString st = "SELECT consumer_id, run_mode_id, spend FROM consumers_spend WHERE consumer_id = :consumer_id ORDER BY 1;";
    query->prepare(st);
    query->bindValue(":consumer_id", currentConsumerId);
    if(!query->exec()) {
        logMsg(msgError, tr("Ошибка получения списка расходов топлива потребителя '%1':<br>%2").arg(currentConsumerName).arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка расходов:";
        qDebug() << query->lastError().text();
        return;
    }


    int n = query->size();
    // проверка на пустость не нужна - ничего не добавится, но привязки сделаются
    //if (n == 0) return;
    // отключаем обработку сигналов модели
//    disconnect(tvRunModesModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellRunModesChanged(QStandardItem*)));
    // задать размер модели
    tvRunModesModel->setRowCount(n);
    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;
//    SELECT consumer_id, run_mode_id, spend FROM consumers_spend WHERE consumer_id = :consumer_id ORDER BY 1;
        tvRunModesModel->setItem(i, 0, new QStandardItem(i));
        tvRunModesModel->setItem(i, 1, new QStandardItem(query->value("run_mode_id").toString()));
        tvRunModesModel->setItem(i, 2, new QStandardItem(QString::number(query->value("spend").toFloat(), 'f', 3)));
        tvRunModesModel->setItem(i, 3, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvRunModesModel->setVerticalHeaderItem(i, si);
    }
    // индексы столбцов с редакторами чисел
    run_mode_idx = query->record().indexOf("run_mode_id");
    run_mode_val_idx = query->record().indexOf("spend");
    query->finish();

    ui->tvRunModes->setModel(tvRunModesModel);
    tvRunModeSelectionModel->setModel(tvRunModesModel);
    ui->tvRunModes->setSelectionModel(tvRunModeSelectionModel);
    ui->tvRunModes->hideColumn(0);// ИД
    ui->tvRunModes->hideColumn(run_mode_record_state_idx);// состояние записи
    // кнопки - что можно / что нельзя
    ui->pbnAddRunMode->setEnabled(true);
    ui->pbnRemoveRunMode->setEnabled(n > 0);
    ui->pbnSaveRunModesList->setEnabled(false);
    ui->pbnRestoreRunModesList->setEnabled(false);

    connect(tvRunModesModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellRunModesChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvRunModes->setCurrentIndex(tvRunModesModel->index(0, 1));
    ui->tvRunModes->setFocus();
}

void ConsumersEdit::showEvent(QShowEvent *)
{
    updateConsumersList();
    updateRunModesList();
}

//=========================== Список потребителей =======================================================
//------------------ действия для списка потребителей ----------------------------------------------------------
void ConsumersEdit::addConsumer()
{
    int n = tvConsumersModel->rowCount();
    tvConsumersModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem("-1")
                       << new QStandardItem("-1")
                       << new QStandardItem("")
                       << new QStandardItem(tr("%1").arg(rsAdded)));
    //refContentModel->setRowCount(n + 1);

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvConsumersModel->setVerticalHeaderItem(n, si);
    tvConsumersModel->submit();

    ui->tvConsumers->setCurrentIndex(tvConsumersModel->index(n, 1));
    ui->tvConsumers->setFocus();

    ui->pbnAddConsumer->setEnabled(false);
    ui->pbnRemoveConsumer->setEnabled(true);
    ui->pbnSaveConsumersList->setEnabled(false);
    ui->pbnRestoreConsumersList->setEnabled(true);
}

void ConsumersEdit::removeConsumer()
{
    int r = ui->tvConsumers->currentIndex().row();
    int recordState = tvConsumersModel->item(r, consumer_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvConsumersModel->item(r, consumer_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvConsumersModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvConsumersModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveConsumer->setText("Отменить");
        ui->pbnRemoveConsumer->setToolTip("Отменить удаление записи");
        ui->pbnRemoveConsumer->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvConsumersModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvConsumersModel->item(r, consumer_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvConsumersModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvConsumersModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvConsumersModel->item(r, consumer_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvConsumersModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvConsumersModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveConsumer->setText("Удалить");
        ui->pbnRemoveConsumer->setToolTip("Удалить запись");
        ui->pbnRemoveConsumer->setStatusTip("Удалить запись");
    }
    checkConsumerButtons();
}

void ConsumersEdit::saveConsumers()
{
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
}

void ConsumersEdit::restoreConsumers()
{
    updateConsumersList();
}

// проверка корректности данных
bool ConsumersEdit::validConsumersData()
{
    bool vd = true;
    for (int i = 0; i < tvConsumersModel->rowCount(); ++i) {
        if(tvConsumersModel->item(i, 1)->data(Qt::DisplayRole).toInt() < 0
           || tvConsumersModel->item(i, 2)->data(Qt::DisplayRole).toInt() < 0
           || tvConsumersModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed().isEmpty()
          )
        {
            vd = false;
            break;
        }
    }
    return vd;
}

// проверка возможности операций
void ConsumersEdit::checkConsumerButtons()
{
    bool dataValid = validConsumersData();
    ui->pbnAddConsumer->setEnabled(dataValid);
    ui->pbnSaveConsumersList->setEnabled(dataValid);
    ui->pbnRemoveConsumer->setEnabled(tvConsumersModel->rowCount() > 0 && ui->tvConsumers->currentIndex().row() >= 0);
    ui->pbnRestoreConsumersList->setEnabled(true);
}

//------------------ события в списке потребителей ----------------------------------------------
void ConsumersEdit::cellConsumersChanged(QStandardItem *item)
{
    int r = item->row();
    int recordState = tvConsumersModel->item(r, consumer_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvConsumersModel->item(r, consumer_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvConsumersModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvConsumersModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkConsumerButtons();
}

void ConsumersEdit::cellConsumerEditorClosed(QWidget *w)
{
    QPoint p = w->pos();
    QPoint cell = QPoint(ui->tvConsumers->rowAt(p.y()) , ui->tvConsumers->columnAt(p.x()));
    QModelIndex idx = ui->tvConsumers->indexAt(p); //cellPos);//
    if(!idx.isValid()) return;
    int recordState = tvConsumersModel->item(cell.x(), consumer_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvConsumersModel->item(cell.x(), consumer_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvConsumersModel->verticalHeaderItem(cell.x())->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvConsumersModel->verticalHeaderItem(cell.x())->setToolTip(tr("Запись %1 изменена").arg(cell.x() + 1));
    }
    qDebug() << "Item " << cell.x() << ":" << cell.y() << " data now: " << tvConsumersModel->data(idx, Qt::DisplayRole).toString();
    checkConsumerButtons();
}

void ConsumersEdit::tvConsumersRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    currentConsumerId = tvConsumersModel->data(tvConsumersModel->index(idxCurrent.row(), 0), Qt::DisplayRole).toInt();
    currentConsumerName = tvConsumersModel->data(tvConsumersModel->index(idxCurrent.row(), 3), Qt::DisplayRole).toString();
    ui->leConsumer->setText(currentConsumerName);
//    qDebug() << "tvConsumersRowChanged: "  << idxCurrent.row();
    // обновить список измерений
    updateRunModesList();
    // разобраться - удалена запись или нет
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    int recordState = tvConsumersModel->item(idxCurrent.row(), consumer_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveConsumer->setText("Отменить");
        ui->pbnRemoveConsumer->setToolTip("Отменить удаление записи");
        ui->pbnRemoveConsumer->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveConsumer->setText("Удалить");
        ui->pbnRemoveConsumer->setToolTip("Удалить запись");
        ui->pbnRemoveConsumer->setStatusTip("Удалить запись");
    }
}

//------------------ нажатия кнопок списка потребителей ----------------------------------------------------------
void ConsumersEdit::on_pbnAddConsumer_clicked()
{
    addConsumer();
}

void ConsumersEdit::on_pbnRemoveConsumer_clicked()
{
    removeConsumer();
}

void ConsumersEdit::on_pbnSaveConsumersList_clicked()
{
    saveConsumers();
}

void ConsumersEdit::on_pbnRestoreConsumersList_clicked()
{
    if(QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        this->windowIcon().actualSize(QSize(48, 48), QIcon::Normal, QIcon::On);
        return;
    }
    updateConsumersList();
}

//================================= Список расходов по режимам работы ===========================================
//------------------ действия для списка режимов работы ----------------------------------------------------------
void ConsumersEdit::addRunMode()
{
    int n = tvRunModesModel->rowCount();

    tvRunModesModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem("-1")
                       << new QStandardItem("1.000")
                       << new QStandardItem(tr("%1").arg(rsAdded)));

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvRunModesModel->setVerticalHeaderItem(n, si);
    tvRunModesModel->submit();

    ui->tvRunModes->setCurrentIndex(tvRunModesModel->index(n, 1));
    ui->tvRunModes->setFocus();

    //ui->pbnAddRunMode->setEnabled(false);
    ui->pbnRemoveRunMode->setEnabled(true);
    ui->pbnSaveRunModesList->setEnabled(true);
    ui->pbnRestoreRunModesList->setEnabled(true);
}

void ConsumersEdit::removeRunMode()
{
    int r = ui->tvRunModes->currentIndex().row();
    int recordState = tvRunModesModel->item(r, run_mode_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvRunModesModel->item(r, run_mode_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvRunModesModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvRunModesModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveRunMode->setText("Отменить");
        ui->pbnRemoveRunMode->setToolTip("Отменить удаление записи");
        ui->pbnRemoveRunMode->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvRunModesModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvRunModesModel->item(r, run_mode_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvRunModesModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvRunModesModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvRunModesModel->item(r, run_mode_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvRunModesModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvRunModesModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveRunMode->setText("Удалить");
        ui->pbnRemoveRunMode->setToolTip("Удалить запись");
        ui->pbnRemoveRunMode->setStatusTip("Удалить запись");
    }
    checkRunModeButtons();
}

void ConsumersEdit::saveRunModes()
{
    int n = tvRunModesModel->rowCount();
    //if(n == 0) return; // если  удалили все - это нужно сделать в БД

    int ri = 0;
//    int ru = 0;
//    int rd = 0;
    int errCount = 0;

    // удалить все чохом, и добавить все по порядку
    query->finish();
    query->prepare("delete from consumers_spend where consumer_id = :consumer_id;");
    query->bindValue(":consumer_id", currentConsumerId);
    if(!query->exec()) {
        QString errSt = query->lastError().text();
        qDebug() << "Ошибка очистки данных списка расходов потребителя:";
        qDebug() << errSt;
        logMsg(msgError, tr("Ошибка очистки данных списка расходов потребителя '%1'<br>%2").arg(currentConsumerName).arg(errSt), ui->teLog);
        // неизвестно, что там есть, поэтому выходим
        return;
    }
    qDebug() << "Cleared table consumers_spend, deleted rows: " << query->numRowsAffected();
    query->finish();

    query->prepare("INSERT INTO consumers_spend(consumer_id, run_mode_id, spend) VALUES (:consumer_id, :run_mode_id, :spend);");
    query->bindValue(":consumer_id", currentConsumerId);
    for (int i = 0; i < n; ++i) {
        if(tvRunModesModel->item(i, run_mode_record_state_idx)->data(Qt::DisplayRole).toInt() == rsDeleted) continue;
        query->bindValue(":run_mode_id", tvRunModesModel->item(i, 1)->data(Qt::DisplayRole).toInt());
        query->bindValue(":spend", tvRunModesModel->item(i, 2)->data(Qt::DisplayRole).toFloat());
        if (!query->exec()) {
            errCount++;
            QString errSt = query->lastError().text();
            qDebug() << "Ошибка сохранения данных:";
            qDebug() << errSt;
            logMsg(msgError, tr("Ошибка сохранения данных списка расходов потребителя '%1'<br>%2").arg(currentConsumerName).arg(errSt), ui->teLog);
            //break;
        } else ri++;
    }
    query->finish();
    tvRunModesModel->submit();// без этого - вылазит ошибка при обновлении
    QString resultMess = QString("Обновлены данные списка расходов потребителя  '%1'<br>").arg(currentConsumerName);
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
//    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
//    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    updateRunModesList();
}

void ConsumersEdit::restoreRunModes()
{
    updateRunModesList();
}

// проверка корректности данных
bool ConsumersEdit::validRunModesData()
{
    bool vd = true;
    bool ok;
    for (int i = 0; i < tvRunModesModel->rowCount(); ++i) {
        if(tvRunModesModel->item(i, 1)->data(Qt::DisplayRole).toInt() < 0
           || tvRunModesModel->item(i, 2)->data(Qt::DisplayRole).toFloat(&ok) <= 0
           || !ok
           || tvRunModesModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed().isEmpty()
          )
        {
            vd = false;
            break;
        }
    }
    return vd;
}

// проверка возможности операций
void ConsumersEdit::checkRunModeButtons()
{
    bool dataValid = (currentConsumerId >= 0) & validRunModesData();
    ui->pbnAddRunMode->setEnabled(dataValid);
    ui->pbnSaveRunModesList->setEnabled(dataValid);
    ui->pbnRemoveRunMode->setEnabled(ui->tvRunModes->currentIndex().row() >= 0);
    ui->pbnRestoreRunModesList->setEnabled(true);
}

//----------------------------------------------------------------------------
void ConsumersEdit::cellRunModesChanged(QStandardItem *item)
{
    int c = item->column();
    if(c > 2) return;
    int r = item->row();

    if(c == 2) {// расход топлива по режиму работы
        bool ok;
        float newVal = item->data(Qt::EditRole).toFloat(&ok);

        if(!ok || (newVal < 0)) {
            qDebug() << "Uncorrect float data: " << newVal;
            ui->pbnSaveRunModesList->setEnabled(false);
            ui->pbnAddRunMode->setEnabled(false);
            return;
        } else {
            tvRunModesModel->item(r, c)->setData(QString::number(newVal, 'f', 3), Qt::DisplayRole);
        }
    }

    int recordState = tvRunModesModel->item(r, run_mode_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvRunModesModel->item(r, run_mode_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvRunModesModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvRunModesModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkRunModeButtons();
}

void ConsumersEdit::cellRunModeEditorClosed(QWidget *w)
{
    QPoint p = w->pos();
    QPoint cell = QPoint(ui->tvRunModes->rowAt(p.y()) , ui->tvRunModes->columnAt(p.x()));
    QModelIndex idx = ui->tvRunModes->indexAt(p); //cellPos);//
    if(!idx.isValid()) return;

    int recordState = tvRunModesModel->item(cell.x(), run_mode_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvRunModesModel->item(cell.x(), run_mode_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvRunModesModel->verticalHeaderItem(cell.x())->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvRunModesModel->verticalHeaderItem(cell.x())->setToolTip(tr("Запись %1 изменена").arg(cell.x() + 1));
    }
    checkRunModeButtons();
}

void ConsumersEdit::tvRunModeRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    // разобраться - удалена запись или нет
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    int recordState = tvRunModesModel->item(idxCurrent.row(), run_mode_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveRunMode->setText("Отменить");
        ui->pbnRemoveRunMode->setToolTip("Отменить удаление записи");
        ui->pbnRemoveRunMode->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveRunMode->setText("Удалить");
        ui->pbnRemoveRunMode->setToolTip("Удалить запись");
        ui->pbnRemoveRunMode->setStatusTip("Удалить запись");
    }
}

//----------------------------------------------------------------------------
void ConsumersEdit::on_pbnAddRunMode_clicked()
{
    addRunMode();
}

void ConsumersEdit::on_pbnRemoveRunMode_clicked()
{
    removeRunMode();
}

void ConsumersEdit::on_pbnSaveRunModesList_clicked()
{
    saveRunModes();
}

void ConsumersEdit::on_pbnRestoreRunModesList_clicked()
{
    updateRunModesList();
}
