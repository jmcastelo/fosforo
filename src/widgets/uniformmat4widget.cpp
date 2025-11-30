


#include "uniformmat4widget.h"



UniformMat4ParameterWidget::UniformMat4ParameterWidget(UniformMat4Parameter* theUniformMat4Parameter, QWidget* parent) :
    ParameterWidget<float> { theUniformMat4Parameter, parent },
    mUniformMat4Parameter { theUniformMat4Parameter }
{
    mGroupBox->setTitle(mUniformMat4Parameter->name());
    // setTitle(mUniformMat4Parameter->name());

    // Set up line edits

    foreach (Number<float>* number, mUniformMat4Parameter->numbers())
    {
        FocusLineEdit* lineEdit = new FocusLineEdit;
        lineEdit->setText(QString::number(number->value()));

        mLineEdits.append(lineEdit);
    }

    setValidators();

    if (!mLineEdits.empty())
    {
        ParameterWidget<float>::mLastFocusedWidget = mLineEdits[0];
        ParameterWidget<float>::mSelectedNumber = mUniformMat4Parameter->number(0);
    }
    else
    {
        ParameterWidget<float>::mLastFocusedWidget = ParameterWidget<float>::mGroupBox;
        // ParameterWidget<float>::mLastFocusedWidget = this;
    }

    mLastIndex = 0;

    // Set up form layout

    QFormLayout* formLayout = new QFormLayout;

    int i = 0;
    foreach (QString numberName, mUniformMat4Parameter->numberNames()) {
        formLayout->addRow(numberName, mLineEdits[i++]);
    }

    // Layout

    /*QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addWidget(mPresetsComboBox);

    mGroupBox->setLayout(layout);*/
    // setLayout(layout);

    mMainLayout->addLayout(formLayout);
    mMainLayout->addWidget(mPresetsComboBox);

    // Connections

    for (int i = 0; i < mUniformMat4Parameter->numbers().size(); i++)
    {
        ParameterWidget<float>::connect(mLineEdits[i], &FocusLineEdit::editingFinished, this, [=, this](){
            mUniformMat4Parameter->setValue(i, mLineEdits[i]->text().toFloat());
        });

        ParameterWidget<float>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){
            ParameterWidget<float>::mSelectedNumber = mUniformMat4Parameter->number(i);
            ParameterWidget<float>::mLastFocusedWidget = mLineEdits[i];
            mLastIndex = i;
        });
        ParameterWidget<float>::connect(mLineEdits[i], &FocusLineEdit::focusOut, this, [=, this](){
            mLineEdits[i]->setText(QString::number(mUniformMat4Parameter->number(i)->value()));
            mLineEdits[i]->setCursorPosition(0);
        });

        ParameterWidget<float>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, &UniformMat4ParameterWidget::focusIn);
        ParameterWidget<float>::connect(mLineEdits[i], &FocusLineEdit::focusOut, this, &UniformMat4ParameterWidget::focusOut);

        ParameterWidget<float>::connect(mUniformMat4Parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant newValue){
            mLineEdits[i]->setText(QString::number(newValue.toFloat()));
            mLineEdits[i]->setCursorPosition(0);

            if (!mLineEdits[i]->hasFocus()) {
                mLineEdits[i]->focusIn();
            }
        });
    }
}



void UniformMat4ParameterWidget::setValidators()
{
    int i = 0;

    foreach (Number<float>* number, mUniformMat4Parameter->numbers())
    {
        QDoubleValidator* validator =  new QDoubleValidator(number->inf(), number->sup(), 5, mLineEdits[i]);
        mLineEdits[i]->setValidator(validator);
        i++;
    }
}



QString UniformMat4ParameterWidget::name()
{
    return mUniformMat4Parameter->name();
}



void UniformMat4ParameterWidget::setName(QString theName)
{
    mUniformMat4Parameter->setName(theName);
    ParameterWidget<float>::mGroupBox->setTitle(theName);
    // setTitle(theName);
}



void UniformMat4ParameterWidget::setMin(float theMin)
{
    ParameterWidget<float>::mSelectedNumber->setMin(theMin);
}



void UniformMat4ParameterWidget::setMax(float theMax)
{
   ParameterWidget<float>::mSelectedNumber->setMax(theMax);
}



void UniformMat4ParameterWidget::setInf(float theInf)
{
    ParameterWidget<float>::mSelectedNumber->setInf(theInf);
    setValidators();
}



void UniformMat4ParameterWidget::setSup(float theSup)
{
    ParameterWidget<float>::mSelectedNumber->setSup(theSup);
    setValidators();
}



void UniformMat4ParameterWidget::setRow(int i)
{
    mUniformMat4Parameter->setRow(i);
}



void UniformMat4ParameterWidget::setCol(int i)
{
    mUniformMat4Parameter->setCol(i);
}



void UniformMat4ParameterWidget::setValueFromIndex(int index)
{
    mUniformMat4Parameter->setValueFromIndex(mLastIndex, index);
}



int UniformMat4ParameterWidget::typeIndex() const
{
    return mUniformMat4Parameter->typeIndex();
}



/*QGroupBox* UniformMat4ParameterWidget::widget()
{
    return ParameterWidget<float>::mGroupBox;
}*/
