#ifndef UNIFORMWIDGET_H
#define UNIFORMWIDGET_H



#include "parameterwidget.h"
#include "../parameters/uniformparameter.h"
#include "layoutformat.h"
#include "focuswidgets.h"

#include <QGridLayout>
#include <QStackedLayout>
#include <QGroupBox>
#include <QScrollBar>



template <typename T>
class UniformParameterWidget : public ParameterWidget<T>
{
public:
    UniformParameterWidget(UniformParameter<T>* theUniformParameter, QWidget* parent = nullptr);

    QString name();
    void setName(QString theName);

    void setMin(T theMin);
    void setMax(T theMax);
    void setInf(T theInf);
    void setSup(T theSup);

    bool isEditable();

    void setRow(int i);
    void setCol(int i);

    void setValueFromIndex(int index);

    void setCurrentStack(int index);

    int layoutFormatIndex();
    void setLayoutFormatIndex(int index);

    QList<QString> availableLayoutFormats();

    bool isMat4Equivalent();
    UniformParameter<T>* parameter() const { return mUniformParameter; }

private:
    UniformParameter<T>* mUniformParameter;

    LayoutFormat mLayoutFormat;
    QList<LayoutFormat> mAvailFormats;

    QList<FocusLineEdit*> mLineEdits;

    QStackedLayout* mStackedLayout;
    QScrollBar* mScrollBar;

    // QList<QWidget*> mItemWidgets;
    QList<QGridLayout*> mItemLayouts;

    QWidget* mColWidget = nullptr;
    QWidget* mRowWidget = nullptr;
    QWidget* mGridWidget = nullptr;

    int mLastIndex;

    void setValidators();
    void setItemsLayouts();
    void clearLayouts();
    void removeLayout(QLayout* layout);
    void setDefaultLayoutFormat();
    void setLayoutFormat(LayoutFormat format);
};



#endif // UNIFORMWIDGET_H
