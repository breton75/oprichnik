#include "tanks_edit.h"
#include "ui_tanks_edit.h"
#include <QSplitter>

#include "u_log.h"
#include "ref_edit.h"
#include <QGraphicsEllipseItem>
#include <QDragMoveEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMessageBox>
#include <QFileDialog>

/*
//================================================================================
//class TextBoard: public QWidget {
//    QString overdrawMessage;

//    void paintEvent(QPaintEvent *event) {
//        QWidget::paintEvent(event);

//        QPainter painter(this);
//        painter.setBrush(Qt::cyan);
//        painter.drawRect(rect());

//        if (overdrawMessage.length() > 0) {
//            painter.setPen( QApplication::palette().color(QPalette::ToolTipText) );
//            painter.setFont(QFont("Arial", 14));
//            painter.drawText(rect(), Qt::AlignCenter, overdrawMessage);

//            qDebug() << "Print overdraw message:" << overdrawMessage;
//        }
//    }

//public:
//    void setOverdrawMessage(const QString iOverdrawMessage) {
//        overdrawMessage=iOverdrawMessage;

//        // Обновляется внешний вид виджета
//        update();
//    }
//};
*/

const int e_size = 10;
const qreal half_size = e_size / 2;

//==========================================================================================
// компонент - точка графика (как у вэйв-формера)
class GraphicItem: public QGraphicsEllipseItem {

private:
    // точка на графике (на сцене)
    int posX;
    int posY;
    // пределы перемещения
//    int minX;
    int minY;
//    int maxX;
    int maxY;
    // координаты точки нажатия мышки внутри эллипса
    int mPosX;
    int mPosY;

public:
    explicit GraphicItem(QGraphicsItem * parent = 0): QGraphicsEllipseItem(parent) {
        // не работает
        //setAcceptDrops(true);
        setAcceptHoverEvents(true);
        posX = 0;
        posY = 0;
//        minX = 0;
        minY = 0;
//        maxX = 0;
        maxY = 0;
        mPosX = 0;
        mPosY = 0;
    }


//public:
    // задание координат эллипса на сцене и пределов его движения
    void setXY(qreal xPos, qreal yPos, qreal yMin, qreal yMax) {
        posX = xPos - half_size;
        posY = yPos - half_size;
        minY = yMin - half_size;
        maxY = yMax - half_size;
        setRect(posX, posY, e_size, e_size);
    }

    // начало drag-drop
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* pe)
    {
//        qDebug() << "mousePressEvent - item " << this->data(0).toInt() << ", posX:posY: " << posX << ":" << posY;
//        qDebug() << "mousePressEvent - mx:my: " << pe->pos().x() << ":" << pe->pos().y() << "; pos.x:pos.y: " << this->pos().x() << ":" << this->pos().y();
        mPosX = pe->pos().x() - posX;
        mPosY = pe->pos().y() - posY;
        //QApplication::setOverrideCursor(Qt::PointingHandCursor);
//        emit giMouseDown(this->data(0).toInt(), pe->pos());
        QGraphicsItem::mousePressEvent(pe);
    }

    // движение
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* pe)
    {
//        qDebug() << "item " << this->data(0).toInt() << "; mx:my: " << pe->pos().x() << ":" << pe->pos().y() << "; pos.x:pos.y: " << this->pos().x() << ":" << this->pos().y();
        // "родной" обработчик - здесь не нужен
//        QGraphicsItem::mouseMoveEvent(pe);
        // новые координаты
        int px = 0;
        int py = pe->pos().y() + pos().y() - mPosY;// - deltaY;//pe->pos().y() + this->pos().y();
        // габариты
        if (py > maxY) {
            py = maxY;
        } else
            if (py < minY) {
                py = minY;
            }
        setPos(px, py - posY);// + mPosY);
//        qDebug() << "          px:py: " << px << ":" << py;
    }

    // окончание процесса drag-drop точки графика
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* pe)
    {
//        qDebug() << "mouseReleaseEvent - item " << this->data(0).toInt();
        QApplication::restoreOverrideCursor();
        QGraphicsItem::mouseReleaseEvent(pe);
    }

    virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * ) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }

    virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * ) {
        QApplication::restoreOverrideCursor();
    }


};
//==========================================================================================

// тестовое сообщение
void TanksEdit::fix(QString msg)
{
    return;
    QMessageBox::information(0, "Контроль запуска ТПО-цистерны", msg);
}

//============================================ форма =============================
TanksEdit::TanksEdit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TanksEdit)
    //, fTest(0)
    , currentProjectId(-1)
{
    // форма
    ui->setupUi(this);
    //fix("Форма цистерн - старт конструктора");
    setWindowModality(Qt::WindowModal);
    //fix("Форма цистерн - задали модальность");
    setWindowIcon(QIcon(":/Retort.png"));
    //fix("Форма цистерн - задали иконку, создаем компоновщики");
    //setWindowIcon(QIcon::fromTheme("fly-astra"));//"accessories-text-editor")); - exception
    //qDebug() << "TanksEdit Window Flags: " <<  this->windowFlags();

    int w = this->width();
    int h = this->height();
    int twoThirdW = w * 2 / 3;
    int twoThirdH = h * 2 / 3;

    ui->frame_1->setLayout(ui->glRefList);
    ui->frame_2->setLayout(ui->verticalLayout);
    ui->frame_3->setLayout(ui->glMeasures);
    ui->frame_4->setLayout(ui->horizontalLayout);
    //fix("Форма цистерн - задали компоновщики, создаем сплиттеры");

    // "резинки" панелей
    QSplitter* sp1 = new QSplitter(Qt::Horizontal, this);
    sp1->setGeometry(0, 0, twoThirdW, twoThirdH);
    sp1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp1->addWidget(ui->frame_1);
    sp1->addWidget(ui->frame_2);
    sp1->addWidget(ui->frame_3);
    connect(sp1, SIGNAL(splitterMoved(int,int)), this, SLOT(onSomeSplitterMoved(int,int)));

    //fix("Форма цистерн - один сплиттер есть, создаем второй");
    QSplitter* sp = new QSplitter(Qt::Vertical, this);
    sp->setGeometry(this->rect());
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sp->addWidget(sp1);//sp2);
    sp->addWidget(ui->frame_4);
    connect(sp, SIGNAL(splitterMoved(int,int)), this, SLOT(onSomeSplitterMoved(int,int)));
    this->setCentralWidget(sp);

    //fix("Форма цистерн - сплиттеры есть, создаем справочники");
    // график - будет позже
    ui->frame_2->setVisible(false);
    ui->pbnLoadTanksFromFile->setVisible(false);

    //---------------------- сетка списка цистерн ---------------------------
    currentTankId = -1;
    currentTankName = "";
    // индексы столбцов с комбо-боксами
    tank_type_idx = -1;
    sensor_type_idx = -1;
    // индекс скрытого столбца - состояния записи
    tank_record_state_idx = 4;

    // модели, делегаты и пр.- БД
    query = new QSqlQuery();
    // заполнить списки типов цистерн и датчиков
    //fix("Форма цистерн - обновляем типы цистерн");
    updateTankTypes();
    //fix("Форма цистерн - типы цистерн есть, создаем типы датчиков");
    updateSensorTypes();
    //fix("Форма цистерн - датчики есть, создаем модель списка цистерн");

    // стандартные модели
    tvTanksModel = new QStandardItemModel(ui->tvTanks);
    ui->tvTanks->setModel(tvTanksModel);
    //tvTanksModel->sets
    tvTanksSelectionModel = new QItemSelectionModel(tvTanksModel);
    ui->tvTanks->setSelectionModel(tvTanksSelectionModel);
    // задать размер модели, заполнить модель, скрыть служебные колонки
    //fix("Форма цистерн - модель списка цистерн есть, создаем список цистерн");
    updateTanksList();

    fix("Форма цистерн - список цистерн есть, создаем делегаты");
    // создать делегат
    tvTanksDelegate = new UniversalDelegate();//tvTanksModel);
    tvTanksDelegate->setClipping(true);
    tvTanksDelegate->setParent(tvTanksModel);
    // для колонок-ссылок тип делегата - комбо-бокс
    tvTanksDelegate->setColumnDelegate(tank_type_idx, dtComboBox);
    tvTanksDelegate->setColumnDelegate(sensor_type_idx, dtComboBox);
    // заполнить списки-справочники (с привязкой к конкретным столбцам)
    tvTanksDelegate->setComboValuesForColumn(tank_type_idx, cbxTankTypesVals);
    tvTanksDelegate->setComboValuesForColumn(sensor_type_idx, cbxSensorTypesVals);

    fix("Форма цистерн - делегаты созданы, привязываем");
    // задать делегат для таблицы
    ui->tvTanks->setItemDelegate(tvTanksDelegate);
    ui->tvTanks->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку
    // отлов изменений через делегат
    //connect(tvTanksDelegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)), this, SLOT(comboEditorClosed(QWidget*)));
    // отлов изменений через модель
    //connect(tvTanksModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellTanksChanged(QStandardItem*)));

    // сменена строка содержимого - выдаем список измерений, и разобраться - удалена запись или нет
    connect(ui->tvTanks->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvTanksRowChanged(QModelIndex,QModelIndex)));
    //ui->tvTanks->selectRow(0);

    //------------------------ сетка графиков пересчета ------------------------------------
    fix("Форма цистерн - с цистернами все, создаем модель замеров");
    tvMeasuresModel = new QStandardItemModel(ui->tvMeasures);
    tvMeasureSelectionModel = new QItemSelectionModel(tvMeasuresModel);
    ui->tvMeasures->setModel(tvMeasuresModel);
    ui->tvMeasures->setSelectionModel(tvMeasureSelectionModel);
    ui->tvMeasures->setSelectionMode(QAbstractItemView::SingleSelection);// только одну ячейку

    fix("Форма цистерн - модель замеров есть, получаем список замеров");
    // задать размер модели, заполнить модель, скрыть служебные колонки
    updateMeasuresList();
    fix("Форма цистерн - список замеров есть, создаем делегат");
    // создать делегат
    tvMeasureDelegate = new UniversalDelegate();
    tvMeasureDelegate->setParent(tvMeasuresModel);
    // для колонок-ссылок тип делегата - дробное
    tvMeasureDelegate->setColumnDelegate(measure_idx, dtRealOrIntNumber);
    tvMeasureDelegate->setClipping(true);
    tvMeasureDelegate->setColumnDelegate(measure_val_idx, dtRealOrIntNumber);//dtMoney);// с деньгами - бред полный
    fix("Форма цистерн - делегат есть, привязываем");
    ui->tvMeasures->setItemDelegate(tvMeasureDelegate);

    // с графическим редактированием таблицы - разберемся позже (если будет время)
//    ui->frame_2->setVisible(false);

    // сменена строка содержимого, разобраться - удалена запись или нет
    connect(ui->tvMeasures->selectionModel(), SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(tvMeasureRowChanged(QModelIndex,QModelIndex)));

    fix("Форма цистерн - со списком замеров все, выбираем первую ячейку");
    ui->tvTanks->setCurrentIndex(tvTanksModel->index(0, 1));

    fix("Форма цистерн - выбрали, создаем сцену");
    w = ui->frame_2->width() - 4;
    h = ui->frame_2->height() - 4;
    scene = new QGraphicsScene(0, 0, w, h);
    //scene->set
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing, true);
    scene->addRect(5, 5, w - 10, h - 10, QPen(Qt::black), QBrush(Qt::NoBrush));
    //scene->setm

    fix("Форма цистерн - сцена есть, привязываем");
//    ui->graphicsView->setMatrix();

    //connect(scene, SIGNAL(dragMoveEvent(QDragMoveEvent *)), this, SLOT(grPointDragMove(QDragMoveEvent *)));
    connect(&sceneResizeTimer, SIGNAL(timeout()), this, SLOT(onSceneResize()));
    fix("Форма цистерн - создали.");
}

TanksEdit::~TanksEdit()
{
    query->finish();
    delete ui;
}

void TanksEdit::setProjectId(int newProjectId)
{
    currentProjectId = newProjectId;
    if(this->isVisible()) updateTanksList();
}

void TanksEdit::userStyleChanged()
{
    ui->tvTanks->verticalHeader()->resetDefaultSectionSize();
            // resizeColumnToContents(1);
}

void TanksEdit::resizeEvent(QResizeEvent *)
{
    if (!this->isVisible()) return;
    int w = ui->tvMeasures->width() / 2 - 8;//re->size().width();
    ui->tvMeasures->setColumnWidth(0, w);
    ui->tvMeasures->setColumnWidth(1, w);
    scene->setSceneRect(5, 5, ui->frame_2->width() - 10, ui->frame_2->height() - 10);
}

void TanksEdit::showEvent(QShowEvent *)
{
    int w = ui->tvMeasures->width() / 2 - 8;//re->size().width();
    ui->tvMeasures->setColumnWidth(0, w);
    ui->tvMeasures->setColumnWidth(1, w);
    updateTanksList();
    updateMeasuresList();
    //qDebug() << "TanksEdit Window Flags: " <<  this->windowFlags();
}

// динамическое обновление ссылок на справочники
void TanksEdit::refTableChanged(QString refTableName)
{
    if(refTableName == "tank_types") {
        updateTankTypes();
        tvTanksDelegate->setComboValuesForColumn(tank_type_idx, cbxTankTypesVals);
    }
    else if(refTableName == "sensor_types") {
        updateSensorTypes();
        tvTanksDelegate->setComboValuesForColumn(sensor_type_idx, cbxSensorTypesVals);
    }
    else return;
    updateTanksList();
}

//================== обновление списков =========================================
void TanksEdit::updateTankTypes()
{
    if(!query->driver()->isOpen()) return;

    query->finish();
    if(!query->exec("select * from tank_types order by 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов цистерн:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список типов цистерн пуст", ui->teLog);
        qDebug() << "Ошибка - список типов цистерн пуст";
        return;
    }

    cbxTankTypesVals.clear();
    QPair<int, QString> parVal;
    while (query->next()) {
        parVal.first = query->value("id").toInt(); // i++;//
        parVal.second = query->value("name").toString();
        cbxTankTypesVals.append(parVal);
    }
    query->finish();
}

void TanksEdit::updateSensorTypes()
{
    if(!query->driver()->isOpen()) return;
    query->finish();
    if(!query->exec("select * from sensor_types order by 1;")) {
        logMsg(msgError, tr("Ошибка обновления списка типов датчиков:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка обновления списка типов датчиков:";
        qDebug() << query->lastError().text();
        return;
    }
    if(query->size() == 0) {
        query->finish();
        logMsg(msgError, "Ошибка - список типов датчиков пуст", ui->teLog);
        qDebug() << "Ошибка - список типов датчиков пуст";
        return;
    }

    cbxSensorTypesVals.clear();
    QPair<int, QString> parVal;
    while (query->next()) {
        parVal.first = query->value("id").toInt();
        parVal.second = query->value("name").toString();
        cbxSensorTypesVals.append(parVal);
    }
    query->finish();
}

void TanksEdit::updateTanksList()
{
    fix("Форма цистерн - получаем список");
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
    fix("Форма цистерн - запрос к БД в порядке, чистим");


    QString st = tr("select id, tank_type, sensor_type, name from tanks where project_id = %1 order by 1;").arg(currentProjectId);// "select * from tanks order by 1;";
    if(!query->exec(st)) {
        logMsg(msgError, tr("Ошибка получения списка цистерн:<br>%1").arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка цистерн:";
        qDebug() << query->lastError().text();
        return;
    }
    fix("Форма цистерн - получили список из БД, обрабатываем, отключаем сигнал");

    int n = query->size();
    if(n <= 0) {
        //logMsg(msgError, "Список цистерн пуст, выходим", ui->teLog);
        //qDebug() << "Список цистерн пуст, выходим";
        //fix("Список цистерн пуст, выходим");
        // индексы колонок все-таки определяем
        tank_type_idx = query->record().indexOf("tank_type");
        sensor_type_idx = query->record().indexOf("sensor_type");
        return;
    } else fix(tr("Список цистерн - размер: %1").arg(n));

    // отключаем обработку сигналов модели
    if(tvMeasuresModel) disconnect(tvTanksModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellTanksChanged(QStandardItem*)));
    fix("Форма цистерн - отключили сигнал модели");
    // задать размер модели
    tvTanksModel->clear();
    tvTanksModel->setColumnCount(5);
    tvTanksModel->setRowCount(n);
    tvTanksModel->setHorizontalHeaderLabels(QStringList() << "ИД" << "Тип цистерны" << "Тип датчика" << "Наименование" << "Состояние");
    fix("Форма цистерн - задали параметры сетки, добавляем");
    if(tvMeasuresModel) tvMeasuresModel->clear();
    currentTankId = -1;
    // заполнить модель
    int i = -1;
    fix(tr("Форма цистерн - старт прохода по списку (list size: %1)").arg(query->size()));
    while (query->next()) {
        i++;
        fix(tr("Список цистерн - добавляем запись %1").arg(i + 1));
        tvTanksModel->setItem(i, 0, new QStandardItem(query->value("id").toString()));
        tvTanksModel->setItem(i, 1, new QStandardItem(query->value("tank_type").toString()));
        tvTanksModel->setItem(i, 2, new QStandardItem(query->value("sensor_type").toString()));
        tvTanksModel->setItem(i, 3, new QStandardItem(query->value("name").toString()));
        tvTanksModel->setItem(i, 4, new QStandardItem(tr("%1").arg(rsUnchanged)));

        fix(tr("Список цистерн - запись %1 добавили, задаем заголовок строки").arg(i + 1));
        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvTanksModel->setVerticalHeaderItem(i, si);
        fix(tr("Список цистерн - запись %1 добавлена полностью.").arg(i + 1));
    }
    fix("Форма цистерн - заполнили модель, разбираемся с моделью");
    // индексы столбцов с комбо-боксами
    tank_type_idx = query->record().indexOf("tank_type");
    sensor_type_idx = query->record().indexOf("sensor_type");
    query->finish();

    ui->tvTanks->setModel(tvTanksModel);
    tvTanksSelectionModel->reset();//setModel(tvTanksModel);
    ui->tvTanks->setSelectionModel(tvTanksSelectionModel);
    ui->tvTanks->hideColumn(0);// ИД
    ui->tvTanks->hideColumn(tank_record_state_idx);// состояние записи
    // кнопки - что можно / что нельзя
    ui->pbnAddTank->setEnabled(true);
    ui->pbnRemoveTank->setEnabled(n > 0);
    ui->pbnSaveTanksList->setEnabled(false);
    ui->pbnRestoreTanksList->setEnabled(false);

    connect(tvTanksModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellTanksChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvTanks->setCurrentIndex(tvTanksModel->index(0, 1));
    ui->tvTanks->setFocus();
}

//============================================ танки наши быстры =============================
void TanksEdit::addTank()
{
    int n = tvTanksModel->rowCount();
    tvTanksModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem("-1")
                       << new QStandardItem("-1")
                       << new QStandardItem("")
                       << new QStandardItem(tr("%1").arg(rsAdded)));
    //refContentModel->setRowCount(n + 1);

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvTanksModel->setVerticalHeaderItem(n, si);
    tvTanksModel->submit();

    ui->tvTanks->setCurrentIndex(tvTanksModel->index(n, 1));
    ui->tvTanks->setFocus();

    ui->pbnAddTank->setEnabled(false);
    ui->pbnRemoveTank->setEnabled(true);
    ui->pbnSaveTanksList->setEnabled(false);
    ui->pbnRestoreTanksList->setEnabled(true);
}

void TanksEdit::removeTank()
{
    int r = ui->tvTanks->currentIndex().row();
    int recordState = tvTanksModel->item(r, tank_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvTanksModel->item(r, tank_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvTanksModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvTanksModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveTank->setText("Отменить");
        ui->pbnRemoveTank->setToolTip("Отменить удаление записи");
        ui->pbnRemoveTank->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvTanksModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvTanksModel->item(r, tank_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvTanksModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvTanksModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvTanksModel->item(r, tank_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvTanksModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvTanksModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveTank->setText("Удалить");
        ui->pbnRemoveTank->setToolTip("Удалить запись");
        ui->pbnRemoveTank->setStatusTip("Удалить запись");
    }
    checkTankButtons();
}

void TanksEdit::saveTanks()
{
    int n = tvTanksModel->rowCount();
    if(n == 0) return;

    int ri = 0;
    int ru = 0;
    int rd = 0;
    int errCount = 0;

    QString qryInsert = "insert into tanks(project_id, tank_type, sensor_type, name) values(:project_id, :tank_type, :sensor_type, :name);";
    QString qryUpdate = "update tanks set tank_type = :tank_type, sensor_type = :sensor_type, name = :name where id = :id;";// and project_id = :project_id;";
    QString qryDelete = "delete from tanks where id = :id;";// and project_id = :project_id;";
    query->finish();

    for (int i = 0; i < n; ++i) {
        bool nothingToDo = false;
        //int recordState = tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toInt();
        //switch (recordState) {
        switch (tvTanksModel->item(i, tank_record_state_idx)->data(Qt::DisplayRole).toInt()) {
        case rsAdded:
            ri++;
            query->prepare(qryInsert);
            query->bindValue(":project_id", currentProjectId);
            query->bindValue(":tank_type", tvTanksModel->item(i, 1)->data(Qt::DisplayRole).toInt());
            query->bindValue(":sensor_type", tvTanksModel->item(i, 2)->data(Qt::DisplayRole).toInt());
            query->bindValue(":name", tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsChanged:
            ru++;
            query->prepare(qryUpdate);
            query->bindValue(":id", tvTanksModel->item(i, 0)->data(Qt::DisplayRole).toInt());
            query->bindValue(":tank_type", tvTanksModel->item(i, 1)->data(Qt::DisplayRole).toInt());
            query->bindValue(":sensor_type", tvTanksModel->item(i, 2)->data(Qt::DisplayRole).toInt());
            //int k = tvTanksModel->item(i, 2)->data(Qt::DisplayRole).toInt();
            //query->bindValue(":sensor_type", k);
            query->bindValue(":name", tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed());
            break;
        case rsDeleted:
            {
                int delId = tvTanksModel->item(i, 0)->data(Qt::DisplayRole).toInt();
                if (delId < 0) {
                    nothingToDo = true;
                } else {
                    rd++;
                    // удалить записи из таблицы замеров
                    if(!query->exec(tr("delete from tank_measures where tank_id = %1").arg(delId))) {
                        //err_ocuped = true;
                        errCount++;
                        QString errSt = query->lastError().text();
                        qDebug() << "Ошибка удаления данных о замерах цистерны " << tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed() << ":";
                        qDebug() << errSt;
                        logMsg(msgError, tr("Ошибка удаления данных о замерах цистерны '%1':<br>%2")
                                        .arg(tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed())
                                        .arg(errSt), ui->teLog);
                        nothingToDo = true; // не смогли - цистерну не удаляем
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
    tvTanksModel->submit();
    QString resultMess = "Обновлены данные списка цистерн<br>";
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    // обновление основной конфигурации
    query->exec(tr("update ship_def set tanks_count = %1;").arg(n));
    updateTanksList();
    //qDebug() << "Обновление данных списка цистерн закончено";
}

void TanksEdit::restoreTanks()
{
    //restoreTanks();
    updateTanksList();
}

void TanksEdit::importTanksFromFile()
{
    if(!query->driver()->isOpen()) return;
    QString strFilter;
    QString loadFileName = QFileDialog::getOpenFileName(0,
                                                        tr("Загрузить конфигурацию из файла"), // заголовок окна
                                                        "oprichik.cfg", // каталог/файл по умолчанию
                                                        "*.cfg", // маска расширений
                                                        &strFilter // ???
                                                        );
    if(loadFileName.isEmpty()) return;

    QFile fConfig(loadFileName);
    int n;
    if(!fConfig.open(QIODevice::ReadOnly)) {
            logMsg(msgError, tr("Ошибка открытия файла '%1':<br>%2").arg(loadFileName).arg(fConfig.errorString()));
            return;
    }

    if(query->exec(tr("delete from tank_measures where tank_id in (select id from tanks where project_id = %1);").arg(currentProjectId))) {
        n = query->numRowsAffected();
        qDebug() << "Таблица замеров цистерн очищена, удалено записей: " << n;
        logMsg(msgInfo, tr("Таблица замеров цистерн очищена, удалено записей: %1").arg(n));
        query->finish();
    }
    else {
        logMsg(msgError, tr("Ошибка очистки таблицы замеров цистерн:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка очистки таблицы замеров цистерн: " << query->lastError().text();
        fConfig.close();
        return;
    }

    // здесь возможны ошибки - ссылки на цистерны в таблицах:
    // - остатки по контрольным точкам check_points_rest
    // - текущий остаток топлива в баках tanks_fuel_rest
    // поэтому пока не удаляем, а обновляем по порядку ИД
//    if(query->exec(tr("delete from tanks where project_id = %1;").arg(currentProjectId))) {
//        n = query->numRowsAffected();
//        qDebug() << "Таблица цистерн очищена, удалено записей: " << n;
//        logMsg(msgInfo, tr("Таблица цистерн очищена, удалено записей: %1").arg(n));
//        query->finish();
//    }
//    else {
//        logMsg(msgError, tr("Ошибка очистки таблицы цистерн:<br>%1").arg(query->lastError().text()));
//        qDebug() << "Ошибка очистки таблицы цистерн: " << query->lastError().text();
//        return;
//    }

    // получаем список ИД цистерн по проекту
    if(!query->exec(tr("select * from tanks where project_id = %1 order by 1;").arg(currentProjectId))) {
        logMsg(msgError, tr("Ошибка получения списка цистерн:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка получения списка цистерн цистерн: " << query->lastError().text();
        fConfig.close();
        return;
    }
    QList<int> tankIds;
    while (query->next()) {
        tankIds << query->value("id").toInt();
    }
    int currTankNum = 0;
    int currTankCount = tankIds.count();
    int currTankId;
    if(!query->exec("select max(id) as max_id from tanks;")) {
        logMsg(msgError, tr("Ошибка получения списка цистерн:<br>%1").arg(query->lastError().text()));
        qDebug() << "Ошибка получения списка цистерн цистерн: " << query->lastError().text();
        fConfig.close();
        return;
    }
    query->next();
    int maxTankId = query->value("max_id").toInt() + 1;

    QByteArray ba = fConfig.readAll();
    fConfig.close();
    qDebug() << "Config file size (bytes): " << ba.size();

    // перекодировка
    QTextCodec* codec = QTextCodec::codecForName("KOI8-R");
    QString st_ba(codec->toUnicode(ba));

    QStringList confData = st_ba.split("\r\n");//(ba);
    int linesCount = confData.size();
    qDebug() << "Config file - lines: " << linesCount;// << ", lines: " << confData.count();

    int currLine = 0;
    int errCount;
    QString st, st1, errSt;
    QString qry;
    QStringList stParts;
    int projTanksCount = 0;

    // импортируем только цистерны и замеры;
    // при несовпадении типов топлива - остается ошибка

    // ищем начало раздела "[Цистерны]"
    st = QString(confData.at(currLine)).trimmed();
    while((currLine < linesCount) && (st != "[Цистерны]")) {
        if(st.startsWith("Количество цистерн")) {
            stParts = st.split(":");
            projTanksCount = QString(stParts.at(1)).trimmed().toInt();
        }
        currLine++;
        st = QString(confData.at(currLine)).trimmed();
    }
    if(currLine >= linesCount) { // не нашли - кодировка?
        logMsg(msgError, "В файле конфигурации не найден раздел цистерн.<br>Возможно, ошибка кодировки.");
        qDebug() << "В файле конфигурации не найден раздел цистерн.\nВозможно, ошибка кодировки.";
        return;
    }
    // нашли - мы на строке "[Цистерны]"
    currLine++; // [Цистерна 1: ЦРТ №1]

    while (currLine < linesCount) {
        st = QString(confData.at(currLine++)).trimmed();
        if(st.isEmpty()) {
            //currLine++;
            continue;
        }

        if(st.startsWith('[')){
            st = st.mid(1, st.length() - 2); // убрали скобки: "Цистерна 1: ЦРТ №1"
            stParts = st.split(":"); // разбили пополам
            QString currTankName = QString(stParts.at(1)).trimmed();
            QString currQryType; // тип запроса - обновление / добавление
            if(currTankNum < currTankCount) {
                currTankId = tankIds.at(currTankNum);
                qry = tr("update tanks set tank_type = :tank_type, sensor_type = :sensor_type, name = '%1' where id = %2;").arg(currTankName).arg(currTankId);
                currQryType = "обновления";
            }
            else {
                currTankId = maxTankId++;
                qry = tr("INSERT INTO tanks(id, project_id, tank_type, sensor_type, name) VALUES (%1, %2, :tank_type, :sensor_type, '%3');").arg(currTankId).arg(currentProjectId).arg(currTankName);
                currQryType = "добавления";
            }
            query->prepare(qry);

            st = QString(confData.at(currLine++)).trimmed(); // Тип цистерны: 1
            stParts = st.split(":");
            int currTankTypeId = QString(stParts.at(1)).trimmed().toInt();
            query->bindValue(":tank_type", currTankTypeId);
            st = QString(confData.at(currLine++)).trimmed(); // Тип измерителя: 5
            stParts = st.split(":");
            int currSensorId = QString(stParts.at(1)).trimmed().toInt();
            query->bindValue(":sensor_type", currSensorId);

            if(!query->exec()) {// сбой - идем до следующей цистерны
                //errSt = tr("Ошибка %1 цистерны %2: %3<br>%4").arg(currQryType).arg(currTankId).arg(currTankName).arg(query->lastError().text());
                errSt = tr("Ошибка %1 значений цистерны %2, %3, %4, %5, '%6' в список цистерн:<br>%7")
                        .arg(currQryType)
                        .arg(currTankId)
                        .arg(currentProjectId)
                        .arg(currTankTypeId)
                        .arg(currSensorId)
                        .arg(currTankName)
                        .arg(query->lastError().text());
                logMsg(msgError, errSt);
                logMsg(msgError, tr("Текст запроса:<br>").arg(query->lastQuery()));
                qDebug() << errSt;
                qDebug() << "Текст запроса:\n" << query->lastQuery();

                st = QString(confData.at(currLine++)).trimmed();
                while((currLine < linesCount) && (!st.isEmpty())) {
                    st = QString(confData.at(currLine++)).trimmed();
                }
                continue;
            }
            // дальше - замеры
            st = QString(confData.at(currLine++)).trimmed(); // Количество замеров: 21
            stParts = st.split(":");
            int currTankMeasuresCount = QString(stParts.at(1)).trimmed().toInt();
            //for (int i = 0; i < currTankMeasuresCount; ++i) {
            //    st = QString(confData.at(currLine++)).trimmed();
            //}
            errCount = 0;
            query->prepare("insert into tank_measures(tank_id, measure_id, v_high, v_volume) values (:tank_id, :measure_id, :v_high, :v_volume);");
            query->bindValue(":tank_id", currTankId);
            for (int i = 0; i < currTankMeasuresCount; ++i) {
                st = QString(confData.at(currLine++)).trimmed(); // 0.500=0.500
                if(st.isEmpty()) break;
                stParts = st.split("=");
                double valMeasureHigh = QString(stParts.at(0)).toFloat();
                double valMeasureVolume = QString(stParts.at(1)).toFloat();
                //qDebug() << "tank id - measure id - high - volume: " << idx << j << valMeasureHigh << valMeasureVolume;
                query->bindValue(":measure_id", i);
                query->bindValue(":v_high", valMeasureHigh);
                query->bindValue(":v_volume", valMeasureVolume);
                if(!query->exec()) {
                    errCount++;
                    logMsg(msgError, tr("Ошибка вставки значений '%1', %2, %3, %4 в список замеров цистерны '%5':<br>%6")
                                     .arg(currTankId)
                                     .arg(i)
                                     .arg(valMeasureHigh)
                                     .arg(valMeasureVolume)
                                     .arg(currTankName)
                                     .arg(query->lastError().text()));
                    logMsg(msgError, tr("Текст запроса: %1").arg(query->executedQuery()));
                    qDebug() << "Ошибка заполнения списка замеров цистерны'"  << currTankName << "':";
                    qDebug() << query->lastError().text();
                    qDebug() << "Текст запроса: \n" << query->executedQuery();
                }
            }
            if(errCount > 0) {
                qDebug() << "Добавление замеров цистерны " << currTankName << " - возникло ошибок: " << errCount;
                logMsg(msgFail, tr("Добавление замеров цистерны %1 - возникло ошибок: %2").arg(currTankName).arg(errCount));
            }

            if(++currTankNum >= projTanksCount) break;
            if(currLine >= linesCount) break;
        }
    }
    updateTanksList();
}


// проверка корректности данных
bool TanksEdit::validTanksData()
{
    bool vd = true;
    for (int i = 0; i < tvTanksModel->rowCount(); ++i) {
        if(tvTanksModel->item(i, 1)->data(Qt::DisplayRole).toInt() < 0
           || tvTanksModel->item(i, 2)->data(Qt::DisplayRole).toInt() < 0
           || tvTanksModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed().isEmpty()
          )
        {
            vd = false;
            break;
        }
    }
    return vd;
}

// проверка возможности операций
void TanksEdit::checkTankButtons()
{
    bool dataValid = validTanksData();
    ui->pbnAddTank->setEnabled(dataValid);
    ui->pbnSaveTanksList->setEnabled(dataValid);
    ui->pbnRemoveTank->setEnabled(tvTanksModel->rowCount() > 0 && ui->tvTanks->currentIndex().row() >= 0);
    ui->pbnRestoreTanksList->setEnabled(true);
}

//--------------- события сетки-списка цистерн ------------------------
void TanksEdit::cellTanksChanged(QStandardItem *item)
{
    int r = item->row();
    int recordState = tvTanksModel->item(r, tank_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvTanksModel->item(r, tank_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvTanksModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvTanksModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkTankButtons();
}

void TanksEdit::comboEditorClosed(QWidget* w)
{
//    QComboBox* cbx = qobject_cast<QComboBox*>(w);
//    if(cbx) qDebug() << "Editor closed, selectde value: " << cbx->currentIndex();

//    QModelIndex mi = ui->tableView->indexAt(w->pos());
//    QVariant cellVal = model->data(mi);
//    qDebug() << "tvCellEdited, editor class: " << w->metaObject()->className()
//             << ", position: " << w->x() << ":" << w->y() << ", cell value: " << cellVal.toString();

    QPoint p = w->pos();
    QPoint cell = QPoint(ui->tvTanks->rowAt(p.y()) , ui->tvTanks->columnAt(p.x()));
    QModelIndex idx = ui->tvTanks->indexAt(p); //cellPos);//
    if(!idx.isValid()) return;
    int recordState = tvTanksModel->item(cell.x(), tank_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvTanksModel->item(cell.x(), tank_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvTanksModel->verticalHeaderItem(cell.x())->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvTanksModel->verticalHeaderItem(cell.x())->setToolTip(tr("Запись %1 изменена").arg(cell.x() + 1));
    }
    qDebug() << "Item " << cell.x() << ":" << cell.y() << " data now: " << tvTanksModel->data(idx, Qt::DisplayRole).toString();
    checkTankButtons();
}

void TanksEdit::tvTanksRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    currentTankId = tvTanksModel->data(tvTanksModel->index(idxCurrent.row(), 0), Qt::DisplayRole).toInt();
    currentTankName = tvTanksModel->data(tvTanksModel->index(idxCurrent.row(), 3), Qt::DisplayRole).toString();
    ui->leTank->setText(currentTankName);
    //qDebug() << "tvTanksRowChanged: "  << idxCurrent.row() << ", currentTankId: " << currentTankId << ", name: " << currentTankName;
    // обновить список измерений
    updateMeasuresList();
    // разобраться - удалена запись или нет
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    int recordState = tvTanksModel->item(idxCurrent.row(), tank_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveTank->setText("Отменить");
        ui->pbnRemoveTank->setToolTip("Отменить удаление записи");
        ui->pbnRemoveTank->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveTank->setText("Удалить");
        ui->pbnRemoveTank->setToolTip("Удалить запись");
        ui->pbnRemoveTank->setStatusTip("Удалить запись");
    }
}

//--------------- кнопки и щелчки ----------------------------------------
void TanksEdit::on_pbnAddTank_clicked()
{
    addTank();
}

void TanksEdit::on_pbnRemoveTank_clicked()
{
    removeTank();
}

void TanksEdit::on_pbnSaveTanksList_clicked()
{
    saveTanks();
}

void TanksEdit::on_pbnRestoreTanksList_clicked()
{
    restoreTanks();
}

void TanksEdit::on_tvTanks_clicked(const QModelIndex &index)
{
    return;
    QString st = tvTanksModel->data(tvTanksModel->index(index.row(), 3), Qt::DisplayRole).toString();
    ui->leTank->setText(st);
}

//============================================ графики уровней =============================
//-------------------------------- действия --------------------------------------------
void TanksEdit::updateMeasuresList()
{
//    qDebug() << "Обновление " << fTest++ << "замеров уровня, ИД цистерны: " << currentTankId;

    disconnect(tvMeasuresModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellMeasuresChanged(QStandardItem*)));
//    qDebug() << "disconnect";

    tvMeasuresModel->clear();
    tvMeasuresModel->setColumnCount(4);
    tvMeasuresModel->setHorizontalHeaderLabels(QStringList() << "ИД замера" << "Высота столба, м" << "Масса, т" << "Состояние");
    ui->tvMeasures->hideColumn(0);
    ui->tvMeasures->hideColumn(3);

    measure_idx = 1;
    measure_val_idx = 2;
    measure_record_state_idx = 3;

    if(currentTankId < 0) {
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

    QString st = "SELECT measure_id, v_high, v_volume FROM tank_measures WHERE tank_id = :tank_id ORDER BY 1;";
    query->prepare(st);
    query->bindValue(":tank_id", currentTankId);
    if(!query->exec()) {
        logMsg(msgError, tr("Ошибка получения списка замеров цистерны '%1':<br>%2").arg(currentTankName).arg(query->lastError().text()), ui->teLog);
        qDebug() << "Ошибка получения списка замеров:";
        qDebug() << query->lastError().text();
        return;
    }

    int n = query->size();
    // проверка на пустость не нужна - ничего не добавится, но привязки сделаются
    //if (n == 0) return;
    // отключаем обработку сигналов модели
    disconnect(tvMeasuresModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellMeasuresChanged(QStandardItem*)));
    // задать размер модели
    tvMeasuresModel->setRowCount(n);
    // заполнить модель
    int i = -1;
    while (query->next()) {
        i++;
        tvMeasuresModel->setItem(i, 0, new QStandardItem(query->value("measure_id").toString()));
        //tvMeasuresModel->setItem(i, 1, new QStandardItem(query->value("v_high").toString()));
        tvMeasuresModel->setItem(i, 1, new QStandardItem(QString::number(query->value("v_high").toFloat(), 'f', 3)));
        tvMeasuresModel->setItem(i, 2, new QStandardItem(QString::number(query->value("v_volume").toFloat(), 'f', 3)));
        tvMeasuresModel->setItem(i, 3, new QStandardItem(tr("%1").arg(rsUnchanged)));

        QStandardItem* si = new QStandardItem(tr("%1").arg(i + 1));
        si->setToolTip(tr("Запись %1").arg(i + 1));
        si->setData(QIcon(":/Apply.ico"), Qt::DecorationRole);
        tvMeasuresModel->setVerticalHeaderItem(i, si);
    }
    // индексы столбцов с редакторами чисел
    measure_idx = query->record().indexOf("v_high");
    measure_val_idx = query->record().indexOf("v_volume");
    query->finish();

    ui->tvMeasures->setModel(tvMeasuresModel);
    tvMeasureSelectionModel->setModel(tvMeasuresModel);
    ui->tvMeasures->setSelectionModel(tvMeasureSelectionModel);
    ui->tvMeasures->hideColumn(0);// ИД
    ui->tvMeasures->hideColumn(measure_record_state_idx);// состояние записи
//    ui->tvMeasures->resizeColumnToContents(1);
    // кнопки - что можно / что нельзя
    ui->pbnAddMeasure->setEnabled(true);
    ui->pbnRemoveMeasure->setEnabled(n > 0);
    ui->pbnSaveMeasuresList->setEnabled(false);
    ui->pbnRestoreMeasuresList->setEnabled(false);

    connect(tvMeasuresModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(cellMeasuresChanged(QStandardItem*)));
//    qDebug() << "connect";
//    connect(tvMeasuresModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(cellMeasuresChanged(QStandardItem*)));

    if(!this->isVisible()) return;
    ui->tvMeasures->setCurrentIndex(tvMeasuresModel->index(0, 1));
    ui->tvMeasures->setFocus();
}

void TanksEdit::addMeasure()
{
    int n = tvMeasuresModel->rowCount();
    QString prevMeasure = n > 0 ? QString::number(tvMeasuresModel->item(n - 1, 1)->data(Qt::DisplayRole).toFloat() + 0.1, 'f', 3) : "0.000";
    //QString prevMeasureVal = n > 0 ? tvMeasuresModel->item(n - 1, 2)->data(Qt::DisplayRole).toString() : "0.0";
    QString prevMeasureVal = n > 0 ? QString::number(tvMeasuresModel->item(n - 1, 2)->data(Qt::DisplayRole).toFloat() + 0.1, 'f', 3) : "0.000";

    tvMeasuresModel->appendRow(QList<QStandardItem*>()
                       << new QStandardItem("-1")
                       << new QStandardItem(prevMeasure)
                       << new QStandardItem(prevMeasureVal)
                       << new QStandardItem(tr("%1").arg(rsAdded)));

    QStandardItem* si = new QStandardItem(tr("%1").arg(n + 1));
    si->setToolTip(tr("Запись %1 добавлена").arg(n + 1));
    si->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
    tvMeasuresModel->setVerticalHeaderItem(n, si);
    tvMeasuresModel->submit();

    ui->tvMeasures->setCurrentIndex(tvMeasuresModel->index(n, 1));
    ui->tvMeasures->setFocus();

    //ui->pbnAddMeasure->setEnabled(false);
    ui->pbnRemoveMeasure->setEnabled(true);
    ui->pbnSaveMeasuresList->setEnabled(true);
    ui->pbnRestoreMeasuresList->setEnabled(true);
}

void TanksEdit::removeMeasure()
{
    int r = ui->tvMeasures->currentIndex().row();
    int recordState = tvMeasuresModel->item(r, measure_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState != rsDeleted) { // удаление
        tvMeasuresModel->item(r, measure_record_state_idx)->setData(rsDeleted, Qt::DisplayRole);
        tvMeasuresModel->verticalHeaderItem(r)->setData(QIcon(":/Minus.ico"), Qt::DecorationRole);
        tvMeasuresModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 удалена").arg(r + 1));
        ui->pbnRemoveMeasure->setText("Отменить");
        ui->pbnRemoveMeasure->setToolTip("Отменить удаление записи");
        ui->pbnRemoveMeasure->setStatusTip("Отменить удаление записи");
    } else { // восстановление
        if(tvMeasuresModel->item(r, 0)->data(Qt::DisplayRole).toInt() < 0) { // элемент добавлен в список
            tvMeasuresModel->item(r, measure_record_state_idx)->setData(rsAdded, Qt::DisplayRole);
            tvMeasuresModel->verticalHeaderItem(r)->setData(QIcon(":/Plus.ico"), Qt::DecorationRole);
            tvMeasuresModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 добавлена").arg(r + 1));
        } else { // элемент уже был в списке
            tvMeasuresModel->item(r, measure_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
            tvMeasuresModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
            tvMeasuresModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
        }
        ui->pbnRemoveMeasure->setText("Удалить");
        ui->pbnRemoveMeasure->setToolTip("Удалить запись");
        ui->pbnRemoveMeasure->setStatusTip("Удалить запись");
    }
    checkMeasureButtons();
}

void TanksEdit::saveMeasures()
{
    int n = tvMeasuresModel->rowCount();
    if(n == 0) return;

    int ri = 0;
//    int ru = 0;
//    int rd = 0;
    int errCount = 0;

    // удалить все чохом, и добавить все по порядку
    query->finish();
    query->prepare("delete from tank_measures where tank_id = :tank_id;");
    query->bindValue(":tank_id", currentTankId);
    if(!query->exec()) {
        QString errSt = query->lastError().text();
        qDebug() << "Ошибка очистки данных:";
        qDebug() << errSt;
        logMsg(msgError, tr("Ошибка очистки данных списка замеров цистерны '%1'<br>%2").arg(currentTankName).arg(errSt), ui->teLog);
        // неизвестно, что там есть, поэтому выходим
        return;
    }
    query->finish();

    query->prepare("insert into tank_measures(tank_id, measure_id, v_high, v_volume) values(:tank_id, :measure_id, :v_high, :v_volume);");
    query->bindValue(":tank_id", currentTankId);
    for (int i = 0; i < n; ++i) {
        if(tvMeasuresModel->item(i, measure_record_state_idx)->data(Qt::DisplayRole).toInt() == rsDeleted) continue;
        query->bindValue(":measure_id", i);
        query->bindValue(":v_high", tvMeasuresModel->item(i, 1)->data(Qt::DisplayRole).toFloat());
        query->bindValue(":v_volume", tvMeasuresModel->item(i, 2)->data(Qt::DisplayRole).toFloat());
        if (!query->exec()) {
            errCount++;
            QString errSt = query->lastError().text();
            qDebug() << "Ошибка сохранения данных:";
            qDebug() << errSt;
            logMsg(msgError, tr("Ошибка сохранения данных списка замеров цистерны '%1'<br>%2").arg(currentTankName).arg(errSt), ui->teLog);
            //break;
        } else ri++;
    }
    query->finish();

//    // просто обновление - не работает, бардак с ИД замера
/*
//    QString qryInsert = "insert into tank_measures(tank_id, measure_id, v_high, v_volume) values(:tank_id, :measure_id, :v_high, :v_volume);";
//    QString qryUpdate = "update tank_measures set measure_id = :measure_id, v_high = :v_high, v_volume = :v_volume where tank_id = :tank_id and measure_id = :measure_id;";
//    QString qryDelete = "delete from tank_measures where tank_id = :tank_id and measure_id = :measure_id;";
//    query->finish();

//    for (int i = 0; i < n; ++i) {
//        bool nothingToDo = false;
//        switch (tvMeasuresModel->item(i, measure_record_state_idx)->data(Qt::DisplayRole).toInt()) {
//        case rsAdded:
//            ri++;
//            query->prepare(qryInsert);
//            query->bindValue(":tank_id", currentTankId);
//            query->bindValue(":measure_id", tvMeasuresModel->item(i, 0)->data(Qt::DisplayRole).toInt());
//            query->bindValue(":v_high", tvMeasuresModel->item(i, 1)->data(Qt::DisplayRole).toFloat());
//            query->bindValue(":v_volume", tvMeasuresModel->item(i, 2)->data(Qt::DisplayRole).toFloat());
//            break;
//        case rsChanged:
//            ru++;
//            query->prepare(qryUpdate);
//            query->bindValue(":tank_id", currentTankId);
//            break;
//        case rsDeleted:
//            {
//                int delId = tvMeasuresModel->item(i, 0)->data(Qt::DisplayRole).toInt();
//                if (delId < 0) {
//                    nothingToDo = true;
//                } else {
//                    rd++;
//                    query->prepare(qryDelete);
//                    query->bindValue(":tank_id", currentTankId);
//                }
//                break;
//            }
//        default: // rsUnchanged
//            nothingToDo = true;
//            break;
//        }

//        if(nothingToDo) continue;

//        if (!query->exec()) {
//            errCount++;
//            QString errSt = query->lastError().text();
//            qDebug() << "Ошибка сохранения данных:";
//            qDebug() << errSt;
//            logMsg(msgError, tr("Ошибка сохранения данных списка замеров цистерны '%1'<br>%2").arg(currentTankName).arg(errSt), ui->teLog);
//            //break;
//        }
//        query->finish();
//    }
*/
    tvMeasuresModel->submit();// без этого - вылазит ошибка при обновлении
    QString resultMess = QString("Обновлены данные списка замеров цистерны '%1'<br>").arg(currentTankName);
    if(ri > 0) resultMess += tr("Добавлено записей: %1'<br>").arg(ri);
//    if(ru > 0) resultMess += tr("Обновлено записей: %1'<br>").arg(ru);
//    if(rd > 0) resultMess += tr("Удалено записей: %1'<br>").arg(rd);
    logMsg(msgInfo, resultMess, ui->teLog);
    if(errCount > 0) //resultMess += tr("При этом возникло ошибок: %1'<br>").arg(errCount);
        logMsg(msgFail, tr("При этом возникло ошибок: %1'<br>").arg(errCount), ui->teLog);
    updateMeasuresList();
}

void TanksEdit::restoreMeasures()
{
    updateMeasuresList();
    checkMeasureButtons();
}

bool TanksEdit::validMeasuresData()
{
    // ? проверять возрастание высот и объемов? (в текущем варианте - не проверяется)
    bool vd = true;
    bool val1ok; // корректность значения высоты
    bool val2ok; // корректность значения объема
    for (int i = 0; i < tvMeasuresModel->rowCount(); ++i) {
        if(tvMeasuresModel->item(i, 1)->data(Qt::DisplayRole).toFloat(&val1ok) <= 0
           || !val1ok
           || tvMeasuresModel->item(i, 2)->data(Qt::DisplayRole).toFloat(&val2ok) <= 0
           || !val2ok
           || tvMeasuresModel->item(i, 3)->data(Qt::DisplayRole).toString().trimmed().isEmpty()
          )
        {
            vd = false;
            break;
        }
    }
    return vd;
}

void TanksEdit::checkMeasureButtons()
{
    bool dataValid = (currentTankId >= 0) & validMeasuresData();
    ui->pbnAddMeasure->setEnabled(dataValid);
    ui->pbnSaveMeasuresList->setEnabled(dataValid);
    ui->pbnRemoveMeasure->setEnabled(ui->tvMeasures->currentIndex().row() >= 0);
    ui->pbnRestoreMeasuresList->setEnabled(true);
}

//---------------------------------- события модели графиков -------------------------------
void TanksEdit::cellMeasuresChanged(QStandardItem *item)
{
    int c = item->column();
    if((c <= 0) || (c > 2)) return;
    int r = item->row();

    bool ok;
    float newVal = item->data(Qt::EditRole).toFloat(&ok);

    if(!ok) {
        qDebug() << "Uncorrect float data: " << newVal;
        ui->pbnSaveMeasuresList->setEnabled(false);
        ui->pbnAddMeasure->setEnabled(false);
        return;
    } else {
        tvMeasuresModel->item(r, c)->setData(QString::number(newVal, 'f', 3), Qt::DisplayRole);
    }

    int recordState = tvMeasuresModel->item(r, measure_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvMeasuresModel->item(r, measure_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvMeasuresModel->verticalHeaderItem(r)->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvMeasuresModel->verticalHeaderItem(r)->setToolTip(tr("Запись %1 изменена").arg(r + 1));
    }
    checkMeasureButtons();
}

void TanksEdit::cellEditorClosed(QWidget *w)
{
    QPoint p = w->pos();
    QPoint cell = QPoint(ui->tvMeasures->rowAt(p.y()) , ui->tvMeasures->columnAt(p.x()));
    QModelIndex idx = ui->tvMeasures->indexAt(p); //cellPos);//
    if(!idx.isValid()) return;
    int recordState = tvMeasuresModel->item(cell.x(), measure_record_state_idx)->data(Qt::DisplayRole).toInt();
    if(recordState == rsUnchanged) {
        tvMeasuresModel->item(cell.x(), measure_record_state_idx)->setData(rsChanged, Qt::DisplayRole);
        tvMeasuresModel->verticalHeaderItem(cell.x())->setData(QIcon(":/Modify.ico"), Qt::DecorationRole);
        tvMeasuresModel->verticalHeaderItem(cell.x())->setToolTip(tr("Запись %1 изменена").arg(cell.x() + 1));
    }
    checkMeasureButtons();
}

void TanksEdit::tvMeasureRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev)
{
    // разобраться - удалена запись или нет
    if(idxCurrent.row() == idxPrev.row()) return; // уже разобрались
    int recordState = tvMeasuresModel->item(idxCurrent.row(), measure_record_state_idx)->data(Qt::DisplayRole).toInt();
    if (recordState == rsDeleted) {
        ui->pbnRemoveMeasure->setText("Отменить");
        ui->pbnRemoveMeasure->setToolTip("Отменить удаление записи");
        ui->pbnRemoveMeasure->setStatusTip("Отменить удаление записи");
    } else {
        ui->pbnRemoveMeasure->setText("Удалить");
        ui->pbnRemoveMeasure->setToolTip("Удалить запись");
        ui->pbnRemoveMeasure->setStatusTip("Удалить запись");
    }
}

//---------------------------------- кнопки графиков ---------------------------------------
void TanksEdit::on_pbnAddMeasure_clicked()
{
    addMeasure();
}

void TanksEdit::on_pbnRemoveMeasure_clicked()
{
    removeMeasure();
}

void TanksEdit::on_pbnSaveMeasuresList_clicked()
{
    saveMeasures();
}

void TanksEdit::on_pbnRestoreMeasuresList_clicked()
{
    restoreMeasures();
}

void TanksEdit::grPointDragMove(QDragMoveEvent *ev)
{
    ui->statusBar->showMessage(tr("Drag pos %1:%2").arg(ev->pos().x()).arg(ev->pos().y()));
}

void TanksEdit::grItemDragMove(QGraphicsSceneDragDropEvent *ev)
{
    ui->statusBar->showMessage(tr("ItemDrag pos %1:%2").arg(ev-> pos().x()).arg(ev->pos().y()));
}

//void TanksEdit::on_pushButton_clicked()
//{
//    qDebug() << scene->sceneRect().left() << scene->sceneRect().top() << scene->sceneRect().width() << scene->sceneRect().height();// << scene->width() << scene->height();
//}

void TanksEdit::onSomeSplitterMoved(int newPos, int splitterIndex)
{
    if(!ui->frame_2->isVisible()) return;

    if(!sceneResizeTimer.isActive()) {
        oldSceneSize = ui->graphicsView->size();
    }
    sceneResizeTimer.start(100);
    return;

    QRectF oldSceneRect = scene->sceneRect();
    //scene->setSceneRect(ui->graphicsView->rect());
    scene->setSceneRect(0, 0, ui->graphicsView->width() - 4, ui->graphicsView->height() - 4);
    QRectF newSceneRect = scene->sceneRect();
    float scaleH = newSceneRect.height() / oldSceneRect.height();
    float scaleW = newSceneRect.width() / oldSceneRect.width();
    float scaleVal = scaleW < scaleH ? scaleW : scaleH;
    qDebug() << "scaleVal: " << scaleVal;
//    scene->
    ui->graphicsView->scale(scaleVal, scaleVal);
    return;

//    foreach (QGraphicsItem* it, scene->items()) {
//        //it->setScale(scaleVal);
//        it->scale(scaleVal);
//    }

//    scene->fit
//    scene->items()
    //scene->sca
    //scene->setSceneRect(ui->graphicsView->sceneRect());
    qDebug() << "SplitterMoved, frame2 size: " << ui->frame_2->width() << " x " << ui->frame_2->height()
             << "; graphicsView size: " << ui->graphicsView->width() << " x "  << ui->graphicsView->height()
             << "; scene size: " << scene->width() << " x "  << scene->height()
                ;
    return;
    qDebug() << "newPos: " << newPos << ", splitterIndex: " << splitterIndex;
}

void TanksEdit::onSceneResize()
{
    sceneResizeTimer.stop();
    QSize newSceneSize = ui->graphicsView->size();
    //qreal newScale = newSceneSize.width() == oldSceneSize.width() ? newSceneSize.height() / oldSceneSize.height() : newSceneSize.width() / oldSceneSize.width();
    //ui->graphicsView->scale(newScale, newScale);
    //qDebug() << "onSceneResize, scale: " << newScale;
    qreal scaleW = newSceneSize.width() * 1.0 / oldSceneSize.width();
    qreal scaleH = newSceneSize.height() * 1.0 / oldSceneSize.height();

    qDebug() << "onSceneResize, scale x: " << scaleW << ", y: " << scaleH;
    ui->graphicsView->scale(scaleW, scaleH);

}

void TanksEdit::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
{
    qDebug() << "on_graphicsView_rubberBandChanged, viewportRect: " << viewportRect
             << ", fromScenePoint: " << fromScenePoint
             << ", toScenePoint: " << toScenePoint
                ;
}

void TanksEdit::on_pbnLoadTanksFromFile_clicked()
{
    importTanksFromFile();
}
