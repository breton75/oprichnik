#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

#include "pg_database.h"
//#include "tankswindow.h"
#include "tanks_edit.h"
#include "ref_edit.h"
#include "consumers_edit.h"
#include "fueltables_edit.h"
#include "net_edit.h"

const QString DB_NAME = "drain_db";
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(bool DB_ok READ isDB_ok() WRITE setDB_ok)

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //QSqlDatabase db;// can_db
    DataBase* db;
    QSqlQuery* query;

    TanksEdit* te;
    RefEdit* re;
    ConsumersEdit* ce;
    FuelTables* fe;
    NetEdit* ne;

    void init_conf();
    bool isDB_ok();
    void setDB_ok(bool newOK);
    void clearDB();
    void setSequenceValues();

private:
    Ui::MainWindow *ui;
    // конфигурация БД
    bool fDB_ok; // существует, настроена, все таблицы созданы и заполнены
    bool fFullUpdate; // полное обновление данных

    // кол-во записей в конф.таблице
    int fConfTableRecCount;
    // конф.параметры
    int fProjectId;

    void fix(QString msg); // тестовое сообщение

    QList<int> projIdList;
    QString fProjectName;
    QString fShipProjectNum;
    QString fShipManufactureNum;
    QString fShipType;
    int fBoardNum;
    int fFuelType;
    int fDisplacement;

//    QString fOldShipProjectNum;
//    QString fOldShipManufactureNum;
//    QString fOldShipType;
//    int fOldDisplacement;

    // обновить список проектов
    void updateProjectsList();
    // обновить список видов топлива
    void updateFuelTypes();
    // обновить конфигурацию по проекту
    void updateConfData();
    // вывод текущих значений
    void showCurrentData();
    // проверка корректности введенных данных
    void checkCorrectData();
    void saveDataToFile();
    void restoreDataFromFile();

protected:
    virtual void resizeEvent(QResizeEvent* rze);
    virtual void showEvent(QShowEvent *);


private slots:
    // динамическое обновление ссылок на справочники
    void refTableChanged(QString refTableName);
    void projIdChanged(int newProjId);
    // изменение строк редактирования
    void on_leProjNum_editingFinished();
    void on_leWorkNum_editingFinished();
    void on_leBoardNum_editingFinished();
    void on_leType_editingFinished();
    void on_leDisplacement_editingFinished();

    void on_leProjNum_textEdited(const QString &arg1);
    void on_leWorkNum_textEdited(const QString &arg1);
    void on_leType_textEdited(const QString &arg1);
    // вызов подчиненных форм
    void on_pbnTanks_clicked();
    void on_pbnConsumers_clicked();
    void on_pbnNetParams_clicked();
    void on_pbnDensToMass_clicked();
    void on_pbnDicts_clicked();
    // сохранение данных в БД
    void on_pbnSave_clicked();
    void on_pbnRestore_clicked();

    void on_cbxFuelType_currentIndexChanged(int);
    void on_chbxShowLog_clicked(bool checked);
    void on_pbnSaveToFile_clicked();
    void on_pbnRestoreFromFile_clicked();

    void slotStyleChange(const QString& str);
    void on_pbnLoadStyle_clicked();
};

#endif // MAINWINDOW_H
