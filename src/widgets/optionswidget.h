#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"
#include "../parameters/optionsparameter.h"

#include <QGroupBox>
#include <QVBoxLayout>



template <class T>
class OptionsParameterWidget : public ParameterWidgetSignals
{
public:
    OptionsParameterWidget(OptionsParameter<T>* theOptionsParameter, QObject* parent = nullptr) :
        ParameterWidgetSignals { parent },
        mOptionsParameter { theOptionsParameter }
    {
        mGroupBox = new QGroupBox;
        mGroupBox->setTitle(mOptionsParameter->name());
        mGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        mGroupBox->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; font-size: 18pt; margin: 7px; }");
        mGroupBox->setCheckable(false);
        // mGroupBox->setVisible(mOptionsParameter->editable());
        mGroupBox->setVisible(false);

        // Set up combo box

        mComboBox = new FocusComboBox;
        mComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        for (QString valueName : mOptionsParameter->valueNames()) {
            mComboBox->addItem(valueName);
        }
        mComboBox->setCurrentIndex(mOptionsParameter->indexOf());

        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(mComboBox);
        mGroupBox->setLayout(layout);

        // Connections

        connect(mComboBox, QOverload<int>::of(&QComboBox::activated), this, [=, this](int index){
            mOptionsParameter->setValue(index);
        });
    }

    QString name() { return mOptionsParameter->name(); }
    void setName(QString theName)
    {
        mOptionsParameter->setName(theName);
        mGroupBox->setTitle(theName);
    }

    void setRow(int i) { mOptionsParameter->setRow(i); }
    void setCol(int i) { mOptionsParameter->setCol(i); }

    QGroupBox* widget() { return mGroupBox; }

    void setCheckable(bool checkable)
    {
        mGroupBox->setCheckable(checkable);

        if (checkable)
        {
            mGroupBox->setChecked(mOptionsParameter->editable());
            connect(mGroupBox, &QGroupBox::toggled, mOptionsParameter, &Parameter::setEditable);
        }
        else
        {
            disconnect(mGroupBox, &QGroupBox::toggled, mOptionsParameter, &Parameter::setEditable);
        }
    }

    void toggleVisibility(bool visible) { mGroupBox->setVisible(visible); }

    OptionsParameter<T>* parameter() { return mOptionsParameter; }

private:
    OptionsParameter<T>* mOptionsParameter;
    QGroupBox* mGroupBox;
    FocusComboBox* mComboBox;
};



#endif // OPTIONSWIDGET_H
