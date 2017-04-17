#include "ref_edit.h"
#include "ui_ref_edit.h"
#include <QSplitter>
#include <QStandardItem>
#include <QItemSelectionModel>

#include "u_log.h"
#include "universaldelegate.h"
//#include "read_only_table_col.h"
//#include "mainwindow.h"

RefEdit::RefEdit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RefEdit)
{
    ui->setupUi(this);
    ui->tvRefList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    setWindowModality(Qt::WindowModal);
    this->setWindowIcon(QIcon(":/Notes.ico"));

    ui->frame_1->setLayout(ui->verticalLayout);
    ui->frame_2->setLayout(ui->gridLayout);

    int w = this->width();
    int h = this->height();
    QSplitter* sp1 = new QSplitter(Qt::Horizontal, this);
    sp1->setGeometry(0, 0, w, h);
    sp1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp1->addWidget(ui->frame_1);
    sp1->addWidget(ui->frame_2);
    sp1->move(w / 3, 0);

    this->setCentralWidget(sp1);

    query = new QSqlQuery();
    refListModel = new QStandardItemModel();
    refContentModel = new QStandardItemModel();
    refContentSelModel = new QItemSelectionModel(refContentModel);

//    ui->tvRefContent->setModel(refContentModel);
//    QItemSelectionModel* refContentSelModel = new QItemSelectionModel(refContentModel);
//    ui->tvRefContent->setSelectionModel(refContentSelModel);

    updateRefList();

    // сменен справочник - обновляем список содержимого
    connect(ui->tvRefList->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(refListCurrentChanged(QModelIndex, QModelIndex)));
    // сменена строка содержимого - разборки на тему удаленных записей
    connect(ui->tvRefContent->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(refContentCurrentChanged(QModelIndex,QModelIndex)));
    // отредактирована ячейка - анализируем
    connect(refContentModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(refContentItemChanged(QStandardItem*)));
    // кнопки Добавить / Удалить / Сохранить / Восстановить
    connect(ui->pbnAddItem, SIGNAL(clicked()), this, SLOT(addRef()));
    connect(ui->pbnRemoveItem, SIGNAL(clicked()), this, SLOT(removeRef()));
    connect(ui->pbnSave, SIGNAL(clicked()), this, SLOT(saveRefs()));
    connect(ui->pbnRestore, SIGNAL(clicked()), this, SLOT(restoreRefs()));
}

RefEdit::~RefEdit()
{
    query->finish();
    delete ui;
}

//========================== список справочников ========================================================
// обновить
void RefEdit::updateRefList()
{
    QString msg;
    if(!query->driver()->isOpen()) {
        msg = tr("Нет соединения с БД проекта, ошибка:<br>").arg(query->driver()->lastError().text());
        logMsg(msgError, msg);
        qDebug() << "Нет соединения с БД проекта, ошибка:";
        qDebug() << query->driver()->lastError().text();
        return;
    }
    query->finish();
    query->clear();
    int i = -1;

    QString st = "select "
                "  c.relname as table_name,"
                "  obj_description(c.oid) as table_desc "
                "from pg_catalog.pg_class c"
                "       join"
                "     pg_catalog.pg_namespace n"
                "       on (n.oid = c.relnamespace) "
                "where c.relkind in ('v', 'r')"
                "  and n.nspname = 'public' "
                "  and c.relname like '%_types' "
                "order by 2;";
    if(!query->exec(st)) {
        logMsg(msgError, tr("Ошибка получения списка справочников:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка получения списка справочников:";
        qDebug() << query->lastError().text();
        return;
    }
    int n = query->size();

    ui->tvRefList->blockSignals(true);
    refListModel->blockSignals(true);
    refContentModel->blockSignals(true);

    refListModel->clear();
    refContentModel->clear();

    refListModel->setColumnCount(2);
    refListModel->setRowCount(n);
    refListModel->setHorizontalHeaderLabels(QStringList() << "Имя таблицы" << "Справочник");

    // пока что список проектов редактируем здесь, и ставим первым
    i++;
    refListModel->setItem(i, 0, new QStandardItem("projects"));
    refListModel->setItem(i, 1, new QStandardItem("Список проектов"));
    QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
    si->setToolTip(tr("Row %1").arg(i + 1));
    si->setData(QIcon(":/List.ico"), Qt::DecorationRole);
    refListModel->setVerticalHeaderItem(i, si);

//    refListModel->
    while (query->next()) {
        i++;
        refListModel->setItem(i, 0, new QStandardItem(query->value("table_name").toString()));
        refListModel->setItem(i, 1, new QStandardItem(query->value("table_desc").toString()));
        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Row %1").arg(i + 1));
        si->setData(QIcon(":/List.ico"), Qt::DecorationRole);
        refListModel->setVerticalHeaderItem(i, si);
        //refListModel->verticalHeaderItem(i)->setData(QIcon("Thumbs up.ico"), Qt::DecorationRole);
//        refListModel->verticalHeaderItem(i)->setToolTip(tr("Row %1").arg(i));
    }
    query->finish();

    ui->tvRefList->setModel(refListModel);
    QItemSelectionModel* refListSelModel = new QItemSelectionModel(refListModel);
    ui->tvRefList->setSelectionModel(refListSelModel);
    ui->tvRefList->hideColumn(0);// DB table name

    refContentModel->blockSignals(false);
    refListModel->blockSignals(false);
    ui->tvRefList->blockSignals(false);

    if(n > 0) {
        ui->tvRefList->setCurrentIndex(refListModel->index(0, 1));
        fRefTableName = refListModel->item(0, 0)->data(Qt::DisplayRole).toString();
        fRefRusName = refListModel->item(0, 1)->data(Qt::DisplayRole).toString();
        updateRefContent();
    }
}

// сменен текущий справочник (сдвинут курсор по таблице)
void RefEdit::refListCurrentChanged(QModelIndex currIdx, QModelIndex prevIdx)
{
//    qDebug() << "refListCurrentChanged, current row: "  << currIdx.row() << ", prev.row: " << prevIdx.row();
    if(currIdx.row() == prevIdx.row()) return;
    fRefTableName = refListModel->item(currIdx.row(), 0)->data(Qt::DisplayRole).toString();
    fRefRusName = refListModel->item(currIdx.row(), 1)->data(Qt::DisplayRole).toString();
    ui->statusbar->showMessage(tr("Row: %1, Table selected: %2").arg(currIdx.row() + 1).arg(fRefTableName), 1000);
    updateRefContent();
//    qDebug() << "refListCurrentChanged, содержимое таблицы "  << fRefTableName << " обновлено" ;
}

//========================== список элементов справочника ========================================================
// обновить список элементов справочника
void RefEdit::updateRefContent()
{
    if (fRefTableName == "") return;

    query->finish();
    query->clear();
    QString st = tr("select * from %1 order by 1;").arg(fRefTableName);
    if(!query->exec(st)) {
        QString errSt = query->lastError().text();
        qDebug() << "Ошибка получения данных таблицы " << fRefTableName << " :";
        qDebug() << errSt;
        logMsg(msgError, tr("Ошибка получения данных таблицы %1:<br>%2").arg(fRefTableName).arg(errSt));
        return;
    }
    int n = query->size();

    // отключаем обработку сигналов модели
    disconnect(refContentModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(refContentItemChanged(QStandardItem*)));

    refContentModel->clear();
    refContentModel->setColumnCount(4);
    refContentModel->setRowCount(n);
    refContentModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "Наименование" << "Описание" << "Состояние");
    int i = -1;

    while (query->next()) {
        i++;
        refContentModel->setItem(i, 0, new QStandardItem(query->value("id").toString()));
        refContentModel->setItem(i, 1, new QStandardItem(query->value("name").toString()));
        refContentModel->setItem(i, 2, new QStandardItem(query->value("description").toString()));
        refContentModel->setItem(i, 3, new QStandardItem(tr("%1").arg(rsUnchanged)));
        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        refContentModel->setVerticalHeaderItem(i, si);
    }
    query->finish();

    ui->tvRefContent->setModel(refContentModel);
    refContentSelModel->setModel(refContentModel);
    ui->tvRefContent->setSelectionModel(refContentSelModel);
    if(n > 0) {
        ui->tvRefContent->setCurrentIndex(refContentModel->index(0, 1));
        ui->tvRefContent->setFocus();
    }
    ui->tvRefContent->hideColumn(0);// ИД
    ui->tvRefContent->hideColumn(3);// состояние записи
    ui->tvRefContent->resizeColumnToContents(1); // растягиваем / сжимаем под содержимое

    ui->pbnAddItem->setEnabled(true);
    ui->pbnRemoveItem->setEnabled(n > 0);
    ui->pbnSave->setEnabled(false);
    ui->pbnRestore->setEnabled(false);

    connect(refContentModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(refContentItemChanged(QStandardItem*)));
}

// добавить элемент
void RefEdit::addRef()
{
    int n = refContentModel->rowCount();
    refContentModel->appendRow(QList<QStandardItem*>()
                               << new QStandardItem("-1")
                               << new QStandardItem("")
                               << new QStandardItem("")
                               << new QStandardItem(tr("%1").arg(rsAdded)));
//    refContentModel->setRowCount(n + 1);

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    refContentModel->setVerticalHeaderItem(n, si);
    refContentModel->submit();

    ui->tvRefContent->setCurrentIndex(refContentModel->index(n, 1));
    ui->tvRefContent->setFocus();

    ui->pbnAddItem->setEnabled(false);
    ui->pbnRemoveItem->setEnabled(true);
    ui->pbnSave->setEnabled(false);
    ui->pbnRestore->setEnabled(true);
//    ui->tvRefContent->blockSignals(false);
}

// удалить элемент / восстановить (отменить удаление)
void RefEdit::removeRef()
{
    int r = ui->tvRefContent->currentIndex().row();
    //qDebug() << "refContentModel->rowDeleted: " << r;
    int recordState = refContentModel->item(r, 3)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        refContentModel->item(r, 3)->setData(rsDeleted, Qt::DisplayRole);
        refContentModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        refContentModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveItem->setText("Отменить");
        ui->pbnRemoveItem->setToolTip("Отменить удаление элемента из списка");
        ui->pbnRemoveItem->setStatusTip("Отменить удаление элемента из списка");
    } else { // восстановление
        if(refContentModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            refContentModel->item(r, 3)->setData(rsAdded, Qt::DisplayRole);
            refContentModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            refContentModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            refContentModel->item(r, 3)->setData(rsChanged, Qt::DisplayRole);
            refContentModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            refContentModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveItem->setText("Удалить");
        ui->pbnRemoveItem->setToolTip("Удалить элемент из списка");
        ui->pbnRemoveItem->setStatusTip("Удалить элемент из списка");
    }
    checkRefsButtons();
}

// сохранить изменения
void RefEdit::saveRefs()
{
    int n = refContentModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int ru = 0;
    int rd = 0;
    int errCount = 0;
    bool nothingToDo;

    QString qryInsert = tr("insert into %1(name, description) values(:name, :description);").arg(fRefTableName);
    QString qryUpdate = tr("update %1 set name = :name, description = :description where id = :id").arg(fRefTableName);
    QString qryDelete = tr("delete from %1 where id = :id").arg(fRefTableName);
    query->finish();

    for (int i = 0; i < n; ++i) {
        nothingToDo = false;
        //int recordState = refContentModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        //switch (recordState) {
        switch (refContentModel->item(i, 3)->data(Qt::DisplayRole).toInt()) {
        case rsAdded:
            ri++;
            query->prepare(qryInsert);
            query->bindValue(":name", refContentModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed());
            query->bindValue(":description", refContentModel->item(i, 2)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsChanged:
            ru++;
            query->prepare(qryUpdate);
            query->bindValue(":id", refContentModel->item(i, 0)->data(Qt::DisplayRole).toInt());
            query->bindValue(":name", refContentModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed());
            query->bindValue(":description", refContentModel->item(i, 2)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsDeleted:
            {
                int delId = refContentModel->item(i, 0)->data(Qt::DisplayRole).toInt();
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
    refContentModel->submit();
    QString resultMess = tr("Обновлены данные таблицы '%1'<br>").arg(fRefRusName);
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount));
    else emit tableChanged(fRefTableName);
    updateRefContent();
    //qDebug() << "Обновление данных таблицы " << fRefRusName <<  " закончено";
}

// отменить изменения
void RefEdit::restoreRefs()
{
    updateRefContent();
}

// проверить допустимость операций
void RefEdit::checkRefsButtons()
{
    //qDebug() << "refContentModel->itemChanged - checkRefsButtons";
    bool validData = validRefContent();
    bool validAndNotEmpty = validData & (refContentModel->rowCount() > 0);
    ui->pbnAddItem->setEnabled(validData);
    ui->pbnSave->setEnabled(validAndNotEmpty);
    ui->pbnRemoveItem->setEnabled(refContentModel->rowCount() > 0);
    ui->pbnRestore->setEnabled(ui->tvRefContent->currentIndex().row() >= 0);//refContentModel->currentRow() >= 0);
}

// проверка корректности данных - все наименования не пустые (кроме удаленных)
bool RefEdit::validRefContent()
{
    int n = refContentModel->rowCount();
    if(n == 0) return true;

    bool vrc = true;
    for (int i = 0; i < n; ++i) {
        int recordState = refContentModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        if(recordState == rsDeleted) {
            continue;
        } else {
            if(refContentModel->item(i, 1)->data(Qt::DisplayRole).toString().trimmed().isEmpty()) {
                vrc = false;
                break;
            }
        }
    }
    return vrc;
}

// сменен текущий элемент справочника (сдвинут курсор по таблице)
void RefEdit::refContentCurrentChanged(QModelIndex currIdx, QModelIndex prevIdx){
    if(currIdx.row() == prevIdx.row()) return;
    int recordState = refContentModel->item(currIdx.row(), 3)->data(Qt::DisplayRole).toInt();
//    qDebug() << "refContentCurrentChanged, current row: "  << currIdx.row() << ", prev.row: " << prevIdx.row() << ", state: " << recordState;
    if (recordState == rsDeleted) {
        ui->pbnRemoveItem->setText("Отменить");
        ui->pbnRemoveItem->setToolTip("Отменить удаление элемента из списка");
        ui->pbnRemoveItem->setStatusTip("Отменить удаление элемента из списка");
    } else {
        ui->pbnRemoveItem->setText("Удалить");
        ui->pbnRemoveItem->setToolTip("Удалить элемент из списка");
        ui->pbnRemoveItem->setStatusTip("Удалить элемент из списка");
    }
    QCoreApplication::processEvents();
}

// изменен (отредактирован) текущий элемент справочника
void RefEdit::refContentItemChanged(QStandardItem *item)
{
    int r = item->row();
    //qDebug() << "refContentModel->itemChanged: " << r << ":" << item->column();
    int recordState = refContentModel->item(r, 3)->data(Qt::DisplayRole).toInt();
    if((recordState != rsAdded) & (recordState != rsDeleted) & (recordState != rsChanged)) {
        refContentModel->item(r, 3)->setData(rsChanged, Qt::DisplayRole);
        refContentModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        refContentModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkRefsButtons();
}

//void RefEdit::closeThis()
//{
//    this->close();
//    this->destroy();
//    this->~RefEdit();
//}

//====================================================================
