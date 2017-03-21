#ifndef TANKS_EDIT_H
#define TANKS_EDIT_H

#include <QMainWindow>
#include <QtSql>
#include "universaldelegate.h"
//#include <QSplitter>
#include <QGraphicsScene>
#include <QDragMoveEvent>
#include <QTimer>
//#include <QRectF>

namespace Ui {
class TanksEdit;
}

class TanksEdit : public QMainWindow
{
    Q_OBJECT

public:
    explicit TanksEdit(QWidget *parent = 0);
    ~TanksEdit();
    void setProjectId(int newProjectId);
    void userStyleChanged();

private:
    Ui::TanksEdit *ui;
    QGraphicsScene *scene;  // Графическая сцена
    QSize oldSceneSize;
    QTimer sceneResizeTimer;

//    QSplitter* sp;
//    QSplitter* sp1;
//    QSplitter* sp2;
    QSqlQuery* query; // общий
    // сетка списка цистерн
    QStandardItemModel* tvTanksModel;
    QItemSelectionModel* tvTanksSelectionModel;
    UniversalDelegate* tvTanksDelegate;

    int currentProjectId;
    int currentTankId;
    QString currentTankName;
    int tank_type_idx;// ссылка на тип цистерны
    int sensor_type_idx;// ссылка на тип датчика
    int tank_record_state_idx;// столбец состояния записи
    QList<QPair<int, QString> > cbxTankTypesVals;
    QList<QPair<int, QString> > cbxSensorTypesVals;
    // сетка графика - списка замеров
    QStandardItemModel* tvMeasuresModel;
    QItemSelectionModel* tvMeasureSelectionModel;
    UniversalDelegate* tvMeasureDelegate;
    int measure_idx; // замер
    int measure_val_idx;// значение
    int measure_record_state_idx;// столбец состояния записи

    //int fTest;// debug var

    void fix(QString msg); // тестовое сообщение

    // действия для списка цистерн
    void addTank();
    void removeTank();
    void saveTanks();
    void restoreTanks();
    void importTanksFromFile();
    bool validTanksData();// проверка корректности данных
    void checkTankButtons(); // проверка возможности операций

    // действия для списка замеров
    void addMeasure();
    void removeMeasure();
    void saveMeasures();
    void restoreMeasures();
    bool validMeasuresData();// проверка корректности данных
    void checkMeasureButtons(); // проверка возможности операций

    // обновления
    void updateTankTypes();
    void updateSensorTypes();
    void updateTanksList();
    void updateMeasuresList();

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);

public slots:
    // динамическое обновление ссылок на справочники
    void refTableChanged(QString refTableName);

private slots:
    // события в списке цистерн
    void cellTanksChanged(QStandardItem *item);
    void comboEditorClosed(QWidget* w);
    void tvTanksRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);
//    void tvCellEdited(QWidget* w);//, QAbstractItemDelegate::EndEditHint);
    // кнопки списка цистерн
    void on_pbnAddTank_clicked();
    void on_pbnRemoveTank_clicked();
    void on_pbnSaveTanksList_clicked();
    void on_pbnRestoreTanksList_clicked();
    void on_tvTanks_clicked(const QModelIndex &index);
    // события в списке замеров
    void cellMeasuresChanged(QStandardItem *item);
    void cellEditorClosed(QWidget* w);
    void tvMeasureRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);
    // кнопки списка замеров
    void on_pbnAddMeasure_clicked();
    void on_pbnRemoveMeasure_clicked();
    void on_pbnSaveMeasuresList_clicked();
    void on_pbnRestoreMeasuresList_clicked();

    void grPointDragMove(QDragMoveEvent * ev);
    void grItemDragMove(QGraphicsSceneDragDropEvent * ev);

    //void on_pushButton_clicked();

    void onSomeSplitterMoved(int newPos, int splitterIndex);
    void onSceneResize();

    void on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);
    void on_pbnLoadTanksFromFile_clicked();
}; // pbnRestoreMeasuresList

#endif // TANKS_EDIT_H
