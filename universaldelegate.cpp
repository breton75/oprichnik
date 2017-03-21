#include "universaldelegate.h"

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QRect>

//! Универсальный делегат для ввода любых значний в зависимости от UserRole
UniversalDelegate::UniversalDelegate(QObject *parent)
             : QItemDelegate(parent)
{
}

QWidget *UniversalDelegate::createEditor(
            QWidget *parent,
            const QStyleOptionViewItem& /* option */,
            const QModelIndex& index) const {
//Для начала построю делегат для всего столбца
int aCurUserRole;
    aCurUserRole = aColumnDelegate.value(index.column(),-1);
if (aCurUserRole == -1) //А если все-таки делегат для конкретной ячейки, тогда узнаю из нее, какой нужен
    aCurUserRole = aCellDelegate.value(index,0);

    if (aCurUserRole==1) //чекбокс
    {
        QCheckBox *editor = new QCheckBox(parent);
        connect(editor,SIGNAL(stateChanged(int)),this,SLOT(ChangeComboBoxText(int)));
        return editor;
    }

    if (aCurUserRole==2) //спинбокс
    {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(100);

        return editor;
    }

    if (aCurUserRole==3) //комбобокс
    {
        QComboBox *editor = new QComboBox(parent);

        QList< QPair<int,QString> > aValues;

        if (aColumnDelegate.contains(index.column()))
            aValues = aValuesOfComboForColumn.value(index.column());
        else
            aValues = aValuesOfComboForCell.value(index);

        QList< QPair<int,QString> >::const_iterator it=aValues.begin();
        while (it != aValues.end())
            {
                const QPair <int,QString> aPair = *it;
                editor->addItem(aPair.second);
                ++it;
            }
            return editor;
    }

    if (aCurUserRole==4) //ввод денег
    {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setMinimum(0.00);
        editor->setMaximum(999999999);
        editor->setDecimals(2);
        editor->setSingleStep(0.1);
        return editor;
    }

    if (aCurUserRole==5) //ввод дробных и целых чисел
    {
        QLineEdit *editor = new QLineEdit(parent);
        return editor;
    }

    if (aCurUserRole==6) //ввод целых чисел
    {
        QLineEdit *editor = new QLineEdit(parent);
        return editor;
    }

    if (aCurUserRole==7) //пустрой делегат, нередактируемый
    {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setReadOnly(true);
        return editor;
    }

    //строка ввода
        QLineEdit *editor = new QLineEdit(parent);
        return editor;
}

void UniversalDelegate::setEditorData(
                QWidget *editor,
                const QModelIndex &index) const {

//Для начала построю делегат для всего столбца
int aCurUserRole;
    aCurUserRole = aColumnDelegate.value(index.column(),-1);
if (aCurUserRole == -1) //А если все-таки делегат для конкретной ячейки, тогда узнаю из нее, какой нужен
    aCurUserRole = aCellDelegate.value(index,0);

    if (aCurUserRole==0) //строка ввода
    {
        if (index.model()->data(index, Qt::EditRole).isNull()) return;
        const QString value = index.model()->data(
                         index, Qt::EditRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    if (aCurUserRole==1) //чекбокс
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
                if (index.model()->data(index, Qt::EditRole).toInt()!=1)
                {
                    checkBox->setChecked(false);
                    checkBox->setText(QString::fromUtf8("Нет"));
                }
                else
                {
                    checkBox->setChecked(true);
                    checkBox->setText(QString::fromUtf8("Да"));
                }
    }

    if (aCurUserRole==2) //спинбокс
    {
        const int value = index.model()->data(index, Qt::EditRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }

    if (aCurUserRole==3) //комбобокс
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);

        const int value = index.model()->data(index, Qt::EditRole).toInt();

        QVector<int> aIndexes;
        if (aColumnDelegate.contains(index.column()))
            aIndexes = aIndexesOfComboForColumn.value(index.column());
        else
            aIndexes = aIndexesOfComboForCell.value(index);

        comboBox->setCurrentIndex(aIndexes.indexOf(value));
    }

    if (aCurUserRole==4) //ввод денег
    {
        const double value = index.model()->data(
                             index, Qt::EditRole).toDouble();
        QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
        doubleSpinBox->setValue(value);
    }

    if (aCurUserRole==5) //ввод дробных и целых чисел
    {
        if (index.model()->data(index, Qt::EditRole).isNull()) return;
        const QString value = index.model()->data(
                             index, Qt::EditRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    if (aCurUserRole==6) //ввод целых чисел
    {
        if (index.model()->data(index, Qt::EditRole).isNull()) return;
        const QString value = index.model()->data(
                             index, Qt::EditRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText(value);
    }

    if (aCurUserRole==7) //пустрой делегат, нередактируемый
    {
        return;
    }

}

void UniversalDelegate::setModelData(
            QWidget *editor,
            QAbstractItemModel *model,
            const QModelIndex& index) const {

//Для начала построю делегат для всего столбца
int aCurUserRole;
    aCurUserRole = aColumnDelegate.value(index.column(),-1);
if (aCurUserRole == -1) //А если все-таки делегат для конкретной ячейки, тогда узнаю из нее, какой нужен
    aCurUserRole = aCellDelegate.value(index,0);

    if (aCurUserRole==0) //строка ввода
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text());
    }

    if (aCurUserRole==1) //чекбокс
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        if (checkBox->isChecked())
            model->setData(index, "1");
        else
            model->setData(index, "0");
    }

    if (aCurUserRole==2) //спинбокс
    {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        const int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }

    if (aCurUserRole==3) //комбобокс
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);

        QVector<int> aIndexes;
        if (aColumnDelegate.contains(index.column()))
            aIndexes = aIndexesOfComboForColumn.value(index.column());
        else
            aIndexes = aIndexesOfComboForCell.value(index);

        model->setData(index, aIndexes.at(comboBox->currentIndex()), Qt::EditRole);
    }
    if (aCurUserRole==4) //ввод денег
    {
        QDoubleSpinBox *doubleSpinBox = static_cast<QDoubleSpinBox*>(editor);
        doubleSpinBox->interpretText();
        model->setData(index, doubleSpinBox->text());
    }

    if (aCurUserRole==5) //ввод дробных и целых чисел
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);

        if (lineEdit->text().isEmpty())
        {
            model->setData(index, QVariant(QString::null));
            return;
        }

        const QString aValue = QString::number(lineEdit->text().toDouble());
        model->setData(index, aValue);
    }

    if (aCurUserRole==6) //ввод целых чисел
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);

        if (lineEdit->text().isEmpty())
        {
            model->setData(index, QVariant(QString::null));
            return;
        }

        const QString aValue = QString::number(lineEdit->text().toInt());
        model->setData(index, aValue);
    }

    if (aCurUserRole==7) //пустрой делегат, нередактируемый
    {
        return;
    }

}

void UniversalDelegate::updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex& /* index */) const {
    editor->setGeometry(option.rect);
}


void UniversalDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{

int aCurUserRole;
    aCurUserRole = aColumnDelegate.value(index.column(),-1);
if (aCurUserRole == -1) //А если все-таки делегат для конкретной ячейки, тогда узнаю из нее, какой нужен
    aCurUserRole = aCellDelegate.value(index,0);

    if (aCurUserRole==0 || aCurUserRole==2 || aCurUserRole==4 || aCurUserRole==5 || aCurUserRole==6) //строка ввода
    {
     if (option.state & QStyle::State_Editing)
         return; //Передаю управление QLineEdit

     if (index.data(Qt::EditRole).isNull())
         return; //Пустая ячейка

     //Прорисовываю текст редактируемой ячейки
     const QString aText = index.data(Qt::EditRole).toString();
     painter->drawText(
         QRect(option.rect.left()+1,option.rect.top(),option.rect.width(),option.rect.height()),
         Qt::AlignLeft | Qt::AlignVCenter,
         aText
     );

    }

    if (aCurUserRole==1) //чекбокс
    {
     if (option.state & QStyle::State_Selected)
        {
         return; //Передаю управление QCheckBox
        }

     QString aText;
                if (index.model()->data(index, Qt::EditRole).toInt()!=1)
                    aText = QString::fromUtf8("Нет");
                else
                    aText = QString::fromUtf8("Да");

         painter->drawText(
         QRect(option.rect.left()+1,option.rect.top(),option.rect.width(),option.rect.height()),
         Qt::AlignLeft | Qt::AlignVCenter,
         aText
     );

    }

    if (aCurUserRole==3) //комбобокс
    {

     if (option.state & QStyle::State_Editing)
         return; //Передаю управление QComboBox

     if (index.data(Qt::EditRole).isNull())
         return; //Пустая ячейка

     QString aText;
     const int aIntValue = index.data(Qt::EditRole).toInt();

        QList< QPair<int,QString> > aValues;

        if (aColumnDelegate.contains(index.column()))
            aValues = aValuesOfComboForColumn.value(index.column());
        else
            aValues = aValuesOfComboForCell.value(index);

                QList< QPair<int,QString> >::const_iterator it=aValues.begin();
                    while (it != aValues.end())
                        {
                            const QPair <int,QString> aPair = *it;
                            if (aPair.first == aIntValue)
                                {
                                    aText = aPair.second;
                                    break;
                                }
                        ++it;
                        }

     painter->drawText(
         QRect(option.rect.left()+1,option.rect.top(),option.rect.width(),option.rect.height()),
         Qt::AlignLeft | Qt::AlignVCenter,
         aText
     );

    }

    if (aCurUserRole==7) //пустрой делегат, нередактируемый
    {
        return;
    }

}


void UniversalDelegate::setComboValuesForCell(const QModelIndex index,const QList< QPair<int,QString> > aValues)
{
    aValuesOfComboForCell[index] = aValues;

    QVector<int> aIndexes;
    QList< QPair<int,QString> >::const_iterator it=aValues.begin();
while (it != aValues.end())
{
    const QPair <int,QString> aPair = *it;
        aIndexes.append(aPair.first);
    ++it;
}
aIndexesOfComboForCell[index] = aIndexes;

}

void UniversalDelegate::setComboValuesForColumn(const int aColumn,const QList< QPair<int,QString> > aValues)
{
    aValuesOfComboForColumn[aColumn] = aValues;

    QVector<int> aIndexes;
    QList< QPair<int,QString> >::const_iterator it=aValues.begin();
while (it != aValues.end())
{
    const QPair <int,QString> aPair = *it;
        aIndexes.append(aPair.first);
    ++it;
}
aIndexesOfComboForColumn[aColumn] = aIndexes;

}

QList< QPair<int,QString> > UniversalDelegate::getComboValuesForCell(const QModelIndex index) const
{
QList< QPair<int,QString> > aValues;
return aValuesOfComboForCell.value(index,aValues);
}

int UniversalDelegate::getCurrentDelegate(const QModelIndex index) const
{
return aCellDelegate.value(index,0);
}


void UniversalDelegate::Clear()
{
    aValuesOfComboForCell.clear();
    aIndexesOfComboForCell.clear();
    aValuesOfComboForColumn.clear();
    aIndexesOfComboForColumn.clear();
    aCellDelegate.clear();
    aColumnDelegate.clear();
}

void UniversalDelegate::ChangeComboBoxText(int aValue)
{
    QCheckBox* aCheckBox = qobject_cast<QCheckBox*>(sender());
    if (aValue != 0)
    {
        aCheckBox->setText(QString::fromUtf8("Да"));
    }
    else
        aCheckBox->setText(QString::fromUtf8("Нет"));
}
