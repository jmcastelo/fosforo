


#include "uniformwidget.h"
#include <QValidator>



template <typename T>
UniformParameterWidget<T>::UniformParameterWidget(UniformParameter<T>* theUniformParameter, QWidget* parent) :
    ParameterWidget<T> { theUniformParameter, parent },
    mUniformParameter { theUniformParameter }
{
    ParameterWidget<T>::mGroupBox->setTitle(mUniformParameter->name());
    // ParameterWidget<T>::setTitle(mUniformParameter->name());

    // Set up line edits

    foreach (Number<T>* number, mUniformParameter->numbers())
    {
        FocusLineEdit* lineEdit = new FocusLineEdit;
        lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        lineEdit->setText(QString::number(number->value()));

        mLineEdits.append(lineEdit);
    }

    setValidators();

    ParameterWidget<T>::mLastFocusedWidget = mLineEdits[0];
    ParameterWidget<T>::mSelectedNumber = mUniformParameter->number(0);
    mLastIndex = 0;

    // Set up widgets and layouts

    mStackedLayout = new QStackedLayout;

    mScrollBar = new QScrollBar(Qt::Horizontal);
    mScrollBar->setFocusPolicy(Qt::ClickFocus);
    mScrollBar->setVisible(false);

    /*QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(mStackedLayout);
    layout->addWidget(mScrollBar);
    layout->addWidget(ParameterWidget<T>::mPresetsComboBox);*/

    ParameterWidget<T>::mMainLayout->addLayout(mStackedLayout);
    ParameterWidget<T>::mMainLayout->addWidget(mScrollBar);
    ParameterWidget<T>::mMainLayout->addWidget(ParameterWidget<T>::mPresetsComboBox);

    setItemsLayouts();
    setDefaultLayoutFormat();
    // ParameterWidget<T>::mGroupBox->setLayout(layout);
    // ParameterWidget<T>::mGroupBox->setVisible(mUniformParameter->editable());
    // ParameterWidget<T>::setLayout(layout);
    // ParameterWidget<T>::setVisible(mUniformParameter->editable());

    // Connections

    for (int i = 0; i < mUniformParameter->numbers().size(); i++)
    {
        ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::editingFinished, this, [=, this]() {
            if (std::is_same<T, float>::value) {
                mUniformParameter->setValue(i, mLineEdits[i]->text().toFloat());
            }
            else if (std::is_same<T, int>::value) {
                mUniformParameter->setValue(i, mLineEdits[i]->text().toInt());
            }
            else if (std::is_same<T, unsigned int>::value) {
                mUniformParameter->setValue(i, mLineEdits[i]->text().toUInt());
            }
        });

        ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this]() {
            ParameterWidget<T>::mSelectedNumber = mUniformParameter->number(i);
            ParameterWidget<T>::mLastFocusedWidget = mLineEdits[i];
            mLastIndex = i;
        });
        ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusOut, this, [=, this]() {
            mLineEdits[i]->setText(QString::number(mUniformParameter->number(i)->value()));
            mLineEdits[i]->setCursorPosition(0);
        });

        ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, &UniformParameterWidget::focusIn);
        ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusOut, this, &UniformParameterWidget::focusOut);

        ParameterWidget<T>::connect(mUniformParameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant newValue) {
            if (std::is_same<T, float>::value) {
                mLineEdits[i]->setText(QString::number(newValue.toFloat()));
            }
            else if (std::is_same<T, int>::value) {
                mLineEdits[i]->setText(QString::number(newValue.toInt()));
            }
            else if (std::is_same<T, unsigned int>::value) {
                mLineEdits[i]->setText(QString::number(newValue.toUInt()));
            }

            mLineEdits[i]->setCursorPosition(0);

            if (!mLineEdits[i]->hasFocus()) {
                mLineEdits[i]->focusIn();
            }
        });
    }

    ParameterWidget<T>::connect(mScrollBar, &QScrollBar::valueChanged, mStackedLayout, &QStackedLayout::setCurrentIndex);
}



template <typename T>
void UniformParameterWidget<T>::setValidators()
{
    int i = 0;

    foreach (Number<T>* number, mUniformParameter->numbers())
    {
        QValidator* validator = nullptr;

        if (std::is_same<T, float>::value) {
            validator = new QDoubleValidator(number->inf(), number->sup(), 5, mLineEdits[i]);
        }
        else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value) {
            validator = new QIntValidator(number->inf(), number->sup(), mLineEdits[i]);
        }

        if (validator) {
            mLineEdits[i]->setValidator(validator);
        }

        i++;
    }
}



template <typename T>
QString UniformParameterWidget<T>::name()
{
    return mUniformParameter->name();
}



template <typename T>
void UniformParameterWidget<T>::setName(QString theName)
{
    mUniformParameter->setName(theName);
    ParameterWidget<T>::mGroupBox->setTitle(theName);
}



template <typename T>
void UniformParameterWidget<T>::setMin(T theMin)
{
    ParameterWidget<T>::mSelectedNumber->setMin(theMin);
}



template <typename T>
void UniformParameterWidget<T>::setMax(T theMax)
{
    ParameterWidget<T>::mSelectedNumber->setMax(theMax);
}



template <typename T>
void UniformParameterWidget<T>::setInf(T theInf)
{
    ParameterWidget<T>::mSelectedNumber->setInf(theInf);
    setValidators();
}



template <typename T>
void UniformParameterWidget<T>::setSup(T theSup)
{
    ParameterWidget<T>::mSelectedNumber->setSup(theSup);
    setValidators();
}



template <typename T>
bool UniformParameterWidget<T>::isEditable()
{
    return mUniformParameter->editable();
}



template <typename T>
void UniformParameterWidget<T>::setRow(int i)
{
    mUniformParameter->setRow(i);
}



template <typename T>
void UniformParameterWidget<T>::setCol(int i)
{
    mUniformParameter->setCol(i);
}



template <typename T>
void UniformParameterWidget<T>::setValueFromIndex(int index)
{
    mUniformParameter->setValueFromIndex(mLastIndex, index);
}



template <typename T>
void UniformParameterWidget<T>::setCurrentStack(int index)
{
    if (mLayoutFormat == LayoutFormat::Stacked && index < mStackedLayout->count())
        mStackedLayout->setCurrentIndex(index);
}



template <typename T>
int UniformParameterWidget<T>::layoutFormatIndex()
{
    return mAvailFormats.indexOf(mLayoutFormat);
}



template <typename T>
QList<QString> UniformParameterWidget<T>::availableLayoutFormats()
{
    QList<QString> formats;

    foreach (auto format, mAvailFormats)
    {
        if (format == LayoutFormat::Column)
            formats.append("Column");
        else if (format == LayoutFormat::Row)
            formats.append("Row");
        else if (format == LayoutFormat::Grid)
            formats.append("Grid");
        else if (format == LayoutFormat::Stacked)
            formats.append("Stacked");
    }

    return formats;
}



template <typename T>
void UniformParameterWidget<T>::setLayoutFormatIndex(int index)
{
    if (index >= 0 && index < mAvailFormats.size())
        setLayoutFormat(mAvailFormats.at(index));
}



template <typename T>
void UniformParameterWidget<T>::setLayoutFormat(LayoutFormat format)
{
    clearLayouts();

    mLayoutFormat = format;

    if (format == LayoutFormat::Column)
    {
        QGridLayout* gridLayout = new QGridLayout;
        gridLayout->setContentsMargins(1, 1, 1, 1);
        gridLayout->setSpacing(0);
        gridLayout->setAlignment(Qt::AlignCenter);

        int row = 0;
        /*foreach (QWidget* widget, mItemWidgets)
        {
            gridLayout->addWidget(widget, row++, 0, Qt::AlignCenter);
            widget->show();
        }*/
        foreach (QGridLayout* layout, mItemLayouts) {
            gridLayout->addLayout(layout, row++, 0, Qt::AlignCenter);
        }

        if (!mColWidget)
        {
            mColWidget = new QWidget;
            mColWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        }

        mColWidget->setLayout(gridLayout);
        mStackedLayout->addWidget(mColWidget);
        mScrollBar->setVisible(false);
    }
    else if (format == LayoutFormat::Row)
    {
        QGridLayout* gridLayout = new QGridLayout;
        gridLayout->setContentsMargins(1, 1, 1, 1);
        gridLayout->setSpacing(0);
        gridLayout->setAlignment(Qt::AlignCenter);

        int col = 0;
        /*foreach (QWidget* widget, mItemWidgets)
        {
            gridLayout->addWidget(widget, 0, col++, Qt::AlignCenter);
            widget->show();
        }*/
        foreach (QGridLayout* layout, mItemLayouts) {
            gridLayout->addLayout(layout, 0, col++, Qt::AlignCenter);
        }

        if (!mRowWidget)
        {
            mRowWidget = new QWidget;
            mRowWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        }

        mRowWidget->setLayout(gridLayout);
        mStackedLayout->addWidget(mRowWidget);
        mScrollBar->setVisible(false);
    }
    else if (format == LayoutFormat::Grid)
    {
        QGridLayout* gridLayout = new QGridLayout;
        gridLayout->setContentsMargins(1, 1, 1, 1);
        gridLayout->setSpacing(0);
        gridLayout->setAlignment(Qt::AlignCenter);

        int dim = static_cast<int>(ceil(sqrt(mUniformParameter->numItems())));

        int row = 0;
        int col = 0;

        // foreach (QWidget* widget, mItemWidgets)
        foreach (QGridLayout* layout, mItemLayouts)
        {
            // gridLayout->addWidget(widget, row, col, Qt::AlignCenter);
            gridLayout->addLayout(layout, row, col, Qt::AlignCenter);
            // widget->show();

            col++;
            if (col == dim)
            {
                row++;
                col = 0;
            }
        }

        if (!mGridWidget)
        {
            mGridWidget = new QWidget;
            mGridWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        }

        mGridWidget->setLayout(gridLayout);
        mStackedLayout->addWidget(mGridWidget);
        mScrollBar->setVisible(false);
    }
    else if (format == LayoutFormat::Stacked)
    {
        /*foreach (QWidget* widget, mItemWidgets) {
            mStackedLayout->addWidget(widget);
        }*/

        foreach (QGridLayout* layout, mItemLayouts)
        {
            QWidget* widget = new QWidget;
            widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            widget->setLayout(layout);

            mStackedLayout->addWidget(widget);
        }

        // mScrollBar->setRange(0, mItemWidgets.size() - 1);
        mScrollBar->setRange(0, mItemLayouts.size() - 1);
        mScrollBar->setValue(0);
        mScrollBar->setVisible(true);
    }

    mStackedLayout->setCurrentIndex(0);
}



template <typename T>
void UniformParameterWidget<T>::setItemsLayouts()
{
    // Set each item on its grid layout

    QGridLayout* itemGridLayout = nullptr;

    QPair<int, int> colsRowsPerItem = mUniformParameter->colsRowsPerItem();

    int row = 0;
    int col = 0;

    foreach (FocusLineEdit* lineEdit, mLineEdits)
    {
        if (row == 0 && col == 0)
        {
            itemGridLayout = new QGridLayout;
            itemGridLayout->setContentsMargins(1, 1, 1, 1);
            itemGridLayout->setSpacing(0);
            itemGridLayout->setAlignment(Qt::AlignCenter);
        }

        itemGridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

        col++;
        if (col == colsRowsPerItem.first)
        {
            row++;
            col = 0;
        }

        if (row == colsRowsPerItem.second)
        {
            row = 0;
            col = 0;

            /*QWidget* widget = new QWidget;
            widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            widget->setLayout(itemGridLayout);

            mItemWidgets.append(widget);*/
            mItemLayouts.append(itemGridLayout);
        }
    }

    // Allow specific layout formats depending on the number of item widgets

    mAvailFormats.clear();

    // if (mItemWidgets.size() == 2)
    if (mItemLayouts.size() == 2)
        mAvailFormats = QList<LayoutFormat>({ LayoutFormat::Column, LayoutFormat::Row, LayoutFormat::Stacked });
    // else if (mItemWidgets.size() > 2)
    else if (mItemLayouts.size() > 2)
        mAvailFormats = QList<LayoutFormat>({ LayoutFormat::Column, LayoutFormat::Row, LayoutFormat::Grid, LayoutFormat::Stacked });
}



template <typename T>
void UniformParameterWidget<T>::clearLayouts()
{
    while(mStackedLayout->count() > 0)
    {
        int index = mStackedLayout->count() - 1;
        QWidget* widget = mStackedLayout->widget(index);
        if (widget) {
            mStackedLayout->removeWidget(widget);
        }
    }

    if (mColWidget) {
        removeLayout(mColWidget->layout());
    }
    if (mRowWidget) {
        removeLayout(mRowWidget->layout());
    }
    if (mGridWidget) {
        removeLayout(mGridWidget->layout());
    }
}



template <typename T>
void UniformParameterWidget<T>::removeLayout(QLayout* layout)
{
    if (layout)
    {
        QLayoutItem* child;
        while ((child = layout->takeAt(0)) != nullptr)
        {
            QWidget* widget = child->widget();
            layout->removeWidget(widget);
            delete child;
        }

        delete layout;
    }
}



template <typename T>
void UniformParameterWidget<T>::setDefaultLayoutFormat()
{
    QPair<int, int> colsRowsPerItem = mUniformParameter->colsRowsPerItem();

    // Any single item
    if (mUniformParameter->numItems() <= 1)
        mLayoutFormat = LayoutFormat::Column;
    // Multiple items with one row
    else if (colsRowsPerItem.second <= 1)
    {
        // Multiple items with one row and one column
        // Multiple items with one row and less columns per item than items
        if (colsRowsPerItem.first <= 1 || colsRowsPerItem.first < mUniformParameter->numItems())
            mLayoutFormat = LayoutFormat::Grid;
        // Multiple items with one row and more than one column
        else
            mLayoutFormat = LayoutFormat::Column;
    }
    // Multiple items with more than one row
    else
        mLayoutFormat = LayoutFormat::Stacked;

    setLayoutFormat(mLayoutFormat);
}



template <typename T>
bool UniformParameterWidget<T>::isMat4Equivalent()
{
    return mUniformParameter->isMat4Equivalent();
}



template class UniformParameterWidget<float>;
template class UniformParameterWidget<int>;
template class UniformParameterWidget<unsigned int>;
