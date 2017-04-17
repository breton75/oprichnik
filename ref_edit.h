#ifndef REF_EDIT_H
#define REF_EDIT_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QtSql>
#include <QStandardItemModel>

namespace Ui {
    class RefEdit;
}

class RefEdit : public QMainWindow
{
    Q_OBJECT

public:
    explicit RefEdit(QWidget *parent = 0);
    ~RefEdit();

    void updateRefList();

private slots:
    void addRef();
    void removeRef();
    void saveRefs();
    void restoreRefs();
    void checkRefsButtons();

    void refListCurrentChanged(QModelIndex currIdx, QModelIndex prevIdx);// сменен текущий справочник (сдвинут курсор по таблице)
    void refContentCurrentChanged(QModelIndex currIdx, QModelIndex prevIdx);// сменен текущий элемент справочника (сдвинут курсор по таблице)
    void refContentItemChanged(QStandardItem *item);// изменен (отредактирован) текущий элемент справочника

//public slots:
//    void closeThis();

private:
    Ui::RefEdit *ui;
    QSqlQuery* query;
    QStandardItemModel* refListModel;
    QStandardItemModel* refContentModel;
    QItemSelectionModel* refContentSelModel;

    QString fRefTableName; // имя таблицы текущего справочника в БД
    QString fRefRusName; // имя текущего справочника по-русски

    void updateRefContent();
    bool validRefContent();// проверка корректности данных - все наименования не пустые (кроме удаленных)

protected:
//    virtual void resizeEvent(QResizeEvent* re);

signals:
//    void pleaseCheckButtons();
    void tableChanged(QString tableName);
//    void allDone();
};

#endif // REF_EDIT_H
