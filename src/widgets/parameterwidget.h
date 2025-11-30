#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H



#include "../parameters/number.h"
#include "../parameters/baseuniformparameter.h"
#include "focuswidgets.h"

#include <QObject>
#include <QGroupBox>
#include <QVBoxLayout>



// Parameter widget signals class

class ParameterWidgetSignals : public QObject
{
    Q_OBJECT

public:
    explicit ParameterWidgetSignals(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void focusIn();
    void focusOut();
};



// Parameter widget base class

template <typename T>
class ParameterWidget : public ParameterWidgetSignals
{
public:
    ParameterWidget(BaseUniformParameter<T>* parameter, QObject* parent = nullptr) :
        ParameterWidgetSignals(parent),
        mParameter { parameter }
    {
        mGroupBox = new FocusGroupBox;
        mGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        mGroupBox->setStyleSheet("QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; font-size: 18pt; margin: 7px; }");
        // mGroupBox->setCheckable(false);
        mGroupBox->setVisible(false);

        connect(mGroupBox, &FocusGroupBox::focusIn, this, &ParameterWidgetSignals::focusIn);

        mPresetsComboBox = new QComboBox;
        mPresetsComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        mPresetsComboBox->setEditable(false);
        mPresetsComboBox->setPlaceholderText("Presets");
        mPresetsComboBox->addItems(mParameter->presetNames());
        mPresetsComboBox->setVisible(false);

        connect(mPresetsComboBox, &QComboBox::activated, this, [=, this](int index){
            if (index >= 0)
            {
                QString name = mPresetsComboBox->currentText();
                mParameter->setPreset(name);
            }
        });

        mMainLayout = new QVBoxLayout;
        mGroupBox->setLayout(mMainLayout);

        // mPresetsComboBox->setVisible(mPresetsComboBox->count() != 0);
    }

    Number<T>* selectedNumber() { return mSelectedNumber; }

    QWidget* lastFocusedWidget() { return mLastFocusedWidget; }

    QGroupBox* widget() { return mGroupBox; }

    void setCheckable(bool checkable)
    {
        mGroupBox->setCheckable(checkable);

        if (checkable)
        {
            mGroupBox->setChecked(mParameter->editable());
            connect(mGroupBox, &QGroupBox::toggled, mParameter, &Parameter::setEditable);
        }
        else
        {
            disconnect(mGroupBox, &QGroupBox::toggled, mParameter, &Parameter::setEditable);
        }
    }

    void setUpVisible()
    {
        mGroupBox->setVisible(true);
        mPresetsComboBox->setVisible(mPresetsComboBox->count() != 0);
    }

    void setRow(int i) { mParameter->setRow(i); }
    void setCol(int i) { mParameter->setCol(i); }

    QString name() { return mParameter->name(); }

    void setName(QString theName)
    {
        mParameter->setName(theName);
        mGroupBox->setTitle(theName);
    }

    virtual void setValueFromIndex(int index) = 0;

    virtual void setMin(T theMin) = 0;
    virtual void setMax(T theMax) = 0;
    virtual void setInf(T theInf) = 0;
    virtual void setSup(T theSup) = 0;

    bool isEditable() { return mParameter->editable(); }

    Parameter* parameter() const { return mParameter; }

    void addPreset(QString name)
    {
        if (!name.isEmpty())
        {
            mParameter->addPreset(name);

            mPresetsComboBox->addItem(name);
            mPresetsComboBox->setCurrentText(name);
            mPresetsComboBox->setVisible(mPresetsComboBox->count() != 0);
        }
    }

    void removeCurrentPreset()
    {
        QString name = mPresetsComboBox->currentText();
        if (!name.isEmpty())
            mParameter->removePreset(name);

        int index = mPresetsComboBox->currentIndex();
        if (index >= 0)
            mPresetsComboBox->removeItem(index);

        mPresetsComboBox->setVisible(mPresetsComboBox->count() != 0);
    }

protected:
    BaseUniformParameter<T>* mParameter;
    Number<T>* mSelectedNumber;
    QWidget* mLastFocusedWidget;
    FocusGroupBox* mGroupBox;
    QComboBox* mPresetsComboBox;
    QVBoxLayout* mMainLayout;
};



#endif // PARAMETERWIDGET_H
