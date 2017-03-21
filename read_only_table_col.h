#ifndef READ_ONLY_TABLE_COL
#define READ_ONLY_TABLE_COL

// запрет редактирования отдельной колонки таблицы
// использование:
//
//  ui->tableView->setItemDelegateForColumn(0, new NonEditTableColumnDelegate(this));
//  ui->tableWidget->setItemDelegateForColumn(1, new NonEditTableColumnDelegate(this));
//
// для всей таблицы - не нужен, для этого используется:
//
//  ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);



#include <QItemDelegate>


class NonEditTableColumnDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    NonEditTableColumnDelegate(QObject * parent = 0) : QItemDelegate(parent) {}
    virtual QWidget * createEditor ( QWidget *, const QStyleOptionViewItem &,
                                     const QModelIndex &) const
    {
        return 0;
    }
};
#endif // READ_ONLY_TABLE_COL

