#include "combodelegate.h"
#include <QComboBox>

//#include <QTableWidget>

ComboDelegate::ComboDelegate(QObject* parent): QItemDelegate(parent)
{
    //selSignalColumn = signalColumnIdx;
//    QAbstractItemView *tv = qobject_cast<QAbstractItemView*>(parent);
//    if(tv) parentWidget = tv;
//    QWidget *tw = qobject_cast<QWidget*>(parent);
//    QTableView
//    QTableWidget tw;
//    tw.setite
//    tw->setmo
    selSignalColumn = 1;
}

//QWidget *ComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    if (index.column() == selSignalColumn) {
//            QComboBox *box = new QComboBox(parent);
////            box->setEditable(true);
//            box->setAutoCompletion(true);
//            box->setEditable(false);
////            box->setAutoCompletion(false);
//            box->addItems(QStringList() << "раз" << "два" << "три");
//            //box->setModel(const_cast<QAbstractItemModel*>(model));
////            box->setModelColumn(index.column());
////            box->installEventFilter(const_cast<SignalDelegate*>(this));
//            return box;
//    } else {
//        return QItemDelegate::createEditor(parent, option, index);
//    }
//}

//void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
//{
//    if (index.column() == selSignalColumn) {
//            QComboBox* box = qobject_cast<QComboBox*>(editor);

////            const QAbstractItemModel *model = index.model();

////            if (!box || !model)
//            if (!box)
//                QItemDelegate::setEditorData(editor, index);
//            else
//                box->setCurrentIndex(index.row());

//    } else {
//        QItemDelegate::setEditorData(editor, index);
//    }
//}

//void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
//{
//    if (index.column() == selSignalColumn) {
//            QComboBox *box = qobject_cast<QComboBox*>(editor);
//            if (!box)
//                return QItemDelegate::setModelData(editor, model, index);

//            model->setData(index, box->currentText(), Qt::DisplayRole);
//            model->setData(index, box->currentText(), Qt::EditRole);
//    } else {
//        return QItemDelegate::setModelData(editor, model, index);
//    }
//}

