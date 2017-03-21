#ifndef NET_EDIT_H
#define NET_EDIT_H

#include <QMainWindow>
#include <QtSql>
#include <QStandardItemModel>
#include <QMenu>
#include "universaldelegate.h"

namespace Ui {
class NetEdit;
}

class NetEdit : public QMainWindow
{
    Q_OBJECT

public:
    explicit NetEdit(QWidget *parent = 0);
    ~NetEdit();
    void setProjectId(int newProjectId);

protected:
    //virtual void resizeEvent(QResizeEvent *);
    virtual void showEvent(QShowEvent *);


private:
    Ui::NetEdit *ui;

    QSqlQuery* query; // общий

    // сетка списка датчиков
    QStandardItemModel* tvSensorsModel;
    QItemSelectionModel* tvSensorsSelectionModel;
    UniversalDelegate* tvSensorsDelegate;
    QMenu* cmnuSensors; // контекстное меню для общего списка

    //QList<QPair<int, QString> > cbxConsumers;// содержание справочника  потребителей
    //QList<QPair<int, QString> > cbxTanks;// содержание справочника  потребителей
    QList<int> cbxConsumers;// содержание справочника  потребителей
    QList<int> cbxTanks;// содержание справочника  потребителей
    QList<int> cbxSensorTypes;// содержание справочника типов датчиков

    int currentProjectId;
    int currentSensorId;
    //QModelIndex currentSensorsListRow;
    int sensors_record_state_idx;// индекс столбца состояния записи
    QString currentSensorName;

    bool fSensorParamsChanged;

    // формирование списка датчиков
    void makeSensorsListCommon();
    void makeSensorsListByDevs();
    void makeSensorsListByTypes();
    void updateSensorsList();

    // действия для списка датчиков
    void addSensor();
    void removeSensor();
    void saveSensors();
    void restoreSensors();
    bool validSensorsData();// проверка корректности данных
    void checkSensorsButtons(); // проверка возможности операций

    // редактирование датчика
    void setPlaceList(); // показ либо списка цистерн, либо списка потребителей
    void clearSensorParams(); // очистить все поля
    void saveSensorParams(int cr); // сохранить (записать) введенные данные; cr = current row
    void restoreSensorParams(); // восстановить исходные данные
    bool validSensorParamsData();// проверка корректности данных
    void checkSensorParamsButtons(); // проверка возможности операций


    // действия для панели редактируемого датчика
//    void setEditValues(); // установка значений редакторов


    // обновления справочников и списков
    void updateSensorTypes();
    void updateTanksList();
    void updateConsumersList();

public slots:
    // динамическое обновление ссылок на справочники
    void refTableChanged(QString refTableName);

private slots:
    void tvSensorsRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);

    void on_pbnAddSensor_clicked();
    void on_pbnRemoveSensor_clicked();
    void on_pbnSaveSensorsList_clicked();
    void on_pbnRestoreSensorsList_clicked();
    void on_rbnTank_clicked();
    void on_rbnConsumer_clicked();
    void on_edAddress_editingFinished();
    void on_pbnClearSensorParams_clicked();
    void on_pbnRestoreSensorParams_clicked();
    void on_pbnSaveSensorParams_clicked();
    void on_chbxAllFields_clicked(bool checked);
    // sensor params editing
    void on_cbxSensorTypes_currentIndexChanged(int index);
    void on_rbnTank_toggled(bool checked);
    void on_rbnConsumer_toggled(bool checked);
    void on_cbxConsumersList_currentIndexChanged(int index);
    void on_cbxTanksList_currentIndexChanged(int index);
    void on_edAddress_valueChanged(int arg1);
    void on_edIndex_valueChanged(int arg1);
    void on_edLowVal_valueChanged(double arg1);
    void on_edHiVal_valueChanged(double arg1);
};

#endif // NET_EDIT_H
