#ifndef FUELTABLES_EDIT_H
#define FUELTABLES_EDIT_H

#include <QMainWindow>
#include <QtSql>
#include <QWidget>
#include <QStandardItemModel>
#include "universaldelegate.h"

namespace Ui {
class FuelTables;
}

class FuelTables : public QMainWindow
{
    Q_OBJECT

public:
    explicit FuelTables(QWidget *parent = 0);
    ~FuelTables();
    void userStyleChanged();

private:
    Ui::FuelTables *ui;
    QSqlQuery* query; // общий

    // сетка списка потребителей
    QStandardItemModel* tvFuelsModel;
    QItemSelectionModel* tvFuelsSelectionModel;
    int currentFuelId;
    QString currentFuelName;
    int fuel_record_state_idx;// столбец состояния записи

    // сетка графика - списка режимов работы
    QStandardItemModel* tvRecalcsModel;
    QItemSelectionModel* tvRecalcSelectionModel;
    UniversalDelegate* tvRecalcDelegate;
    int recalc_record_state_idx;// столбец состояния записи
    int recalc_temp_idx;
    int recalc_density_idx;

    // действия для списка потребителей
    void addFuel();
    void removeFuel();
    void saveFuels();
    void restoreFuels();
    bool validFuelsData();// проверка корректности данных
    void checkFuelButtons(); // проверка возможности операций

    // действия для списка режимов работы
    void addRecalc();
    void removeRecalc();
    void saveRecalcs();
    void restoreRecalcs();
    bool validRecalcsData();// проверка корректности данных
    void checkRecalcButtons(); // проверка возможности операций

    // обновление списков
    void updateFuelsList();
    void updateRecalcsList();

public slots:
    // динамическое обновление ссылок на справочники
    void refTableChanged(QString refTableName);

private slots:
    // события в списке потребителей
    void cellFuelsChanged(QStandardItem *item);// изменен элемент
    void tvFuelsRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);// сменилась текущая строка
    // кнопки списка цистерн
    void on_pbnAddFuel_clicked();
    void on_pbnRemoveFuel_clicked();
    void on_pbnSaveFuelsList_clicked();
    void on_pbnRestoreFuelsList_clicked();
    // события в списке замеров
    void cellRecalcChanged(QStandardItem *item);// изменен элемент
    void tvRecalcRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);// сменилась текущая строка
    // кнопки списка замеров
    void on_pbnAddRecalc_clicked();
    void on_pbnRemoveRecalc_clicked();
    void on_pbnSaveRecalcsList_clicked();
    void on_pbnRestoreRecalcsList_clicked();
};

#endif // FUELTABLES_EDIT_H
