#ifndef CONSUMERS_EDIT_H
#define CONSUMERS_EDIT_H

#include <QMainWindow>
#include <QtSql>
#include <QWidget>
#include <QStandardItemModel>
#include "universaldelegate.h"

namespace Ui {
class ConsumersEdit;
}

class ConsumersEdit : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConsumersEdit(QWidget *parent = 0);
    ~ConsumersEdit();
    void setProjectId(int newProjectId);

private:
    Ui::ConsumersEdit *ui;
    QSqlQuery* query; // общий

    // сетка списка потребителей
    QStandardItemModel* tvConsumersModel;
    QItemSelectionModel* tvConsumersSelectionModel;
    UniversalDelegate* tvConsumersDelegate;

    int currentProjectId;
    int currentConsumerId;
    QString currentConsumerName;
    int consumer_type_idx;// ссылка на тип потребителя
    int meter_type_idx;// ссылка на тип измерителя
    int consumer_record_state_idx;// столбец состояния записи
    QList<QPair<int, QString> > cbxConsumerTypesVals;// содержание справочника типов потребителей
    QList<QPair<int, QString> > cbxRunModeTypesVals;// содержание справочника типов режимов работы
    QList<QPair<int, QString> > cbxMeterTypesVals;//cbxRunModeTypesVals содержание справочника типов измерителей

    // сетка графика - списка режимов работы
    QStandardItemModel* tvRunModesModel;
    QItemSelectionModel* tvRunModeSelectionModel;
    UniversalDelegate* tvRunModeDelegate;
    int run_mode_idx; // режим
    int run_mode_val_idx;// расход по режиму
    int run_mode_record_state_idx;// столбец состояния записи

    // действия для списка потребителей
    void addConsumer();
    void removeConsumer();
    void saveConsumers();
    void restoreConsumers();
    bool validConsumersData();// проверка корректности данных
    void checkConsumerButtons(); // проверка возможности операций

    // действия для списка режимов работы
    void addRunMode();
    void removeRunMode();
    void saveRunModes();
    void restoreRunModes();
    bool validRunModesData();// проверка корректности данных
    void checkRunModeButtons(); // проверка возможности операций

    // обновления справочников и списков
    void updateConsumerTypes();
    void updateRunModeTypes();
    void updateMeterTypes();
    void updateConsumersList();
    void updateRunModesList();

protected:
    virtual void showEvent(QShowEvent *);

public slots:
    // динамическое обновление ссылок на справочники
    void refTableChanged(QString refTableName);

private slots:
    // события в списке потребителей
    void cellConsumersChanged(QStandardItem *item);// изменен элемент
    void cellConsumerEditorClosed(QWidget* w); // закрыт редактор ячейки
    void tvConsumersRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);// сменилась текущая строка
//    void tvCellEdited(QWidget* w);//, QAbstractItemDelegate::EndEditHint);
    // кнопки списка цистерн
    void on_pbnAddConsumer_clicked();
    void on_pbnRemoveConsumer_clicked();
    void on_pbnSaveConsumersList_clicked();
    void on_pbnRestoreConsumersList_clicked();
    // события в списке замеров
    void cellRunModesChanged(QStandardItem *item);// изменен элемент
    void cellRunModeEditorClosed(QWidget* w);// закрыт редактор ячейки
    void tvRunModeRowChanged(QModelIndex idxCurrent, QModelIndex idxPrev);// сменилась текущая строка
    // кнопки списка замеров
    void on_pbnAddRunMode_clicked();
    void on_pbnRemoveRunMode_clicked();
    void on_pbnSaveRunModesList_clicked();
    void on_pbnRestoreRunModesList_clicked();

};

#endif // CONSUMERS_EDIT_H
