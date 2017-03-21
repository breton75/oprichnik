#ifndef DELEGATE_H
#define DELEGATE_H

#include <QtGui>
#include <QObject>
#include <QItemDelegate>

//! Универсальный делегат для ввода любых значний в зависимости от UserRole
// 0 - строка ввода
// 1 - чекбокс
// 2 - спинбокс
// 3 - комбобокс
// 4 - ввод денег
// 5 - ввод дробных и целых чисел
// 6 - ввод целых чисел
// 7 - пустрой делегат, нередактируемый
enum DelegatType {
        dtInputString = 0,
        dtCheckBox = 1,
        dtSpinBox = 2,
        dtComboBox = 3,
        dtMoney = 4,
        dtRealOrIntNumber = 5,
        dtIntNumber = 6,
        dtEmpty = 7
};

// состояние записи таблицы
enum RowState {
    rsAdded = -1,
    rsUnchanged = 0,
    rsChanged = 1,
    rsDeleted = 2
};

class UniversalDelegate : public QItemDelegate {
     Q_OBJECT
 public:
     UniversalDelegate(QObject *parent = 0);
     QWidget *createEditor(
                 QWidget *parent,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const;
     void setEditorData(QWidget *editor,
                        const QModelIndex &index) const;
     void setModelData(QWidget *editor,
                       QAbstractItemModel *model,
                       const QModelIndex &index) const;
     void updateEditorGeometry(
             QWidget *editor,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const;
     void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
     void setComboValuesForCell(const QModelIndex index,const QList< QPair<int,QString> > aValues);
     void setComboValuesForColumn(const int aColumn,const QList< QPair<int,QString> > aValues);
     void setColumnDelegate(const int aColumn,const int aDelegate) {aColumnDelegate[aColumn] = aDelegate;}
     void setCellDelegate(const QModelIndex index,const int aDelegate) {aCellDelegate[index] = aDelegate;}

     QList< QPair<int,QString> > getComboValuesForCell(const QModelIndex index) const;
     int getCurrentDelegate(const QModelIndex index) const;

     void Clear();

 private slots:
     void ChangeComboBoxText(int);

 private:
     QMap <QModelIndex,QList< QPair<int,QString> > > aValuesOfComboForCell;
     QMap <QModelIndex,QVector<int> > aIndexesOfComboForCell;

     QMap <int,QList< QPair<int,QString> > > aValuesOfComboForColumn;
     QMap <int,QVector<int> > aIndexesOfComboForColumn;

     QMap <int,int> aColumnDelegate;
     QMap <QModelIndex,int> aCellDelegate;
};

#endif
