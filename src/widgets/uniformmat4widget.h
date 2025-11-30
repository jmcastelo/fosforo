#ifndef UNIFORMMAT4WIDGET_H
#define UNIFORMMAT4WIDGET_H



#include "parameterwidget.h"
#include "focuswidgets.h"
#include "../parameters/uniformmat4parameter.h"

#include <QGroupBox>
#include <QFormLayout>



class UniformMat4ParameterWidget : public ParameterWidget<float>
{
public:
    UniformMat4ParameterWidget(UniformMat4Parameter* theUniformMat4Parameter, QWidget* parent = nullptr);

    QString name();
    void setName(QString theName);

    void setMin(float theMin);
    void setMax(float theMax);
    void setInf(float theInf);
    void setSup(float theSup);

    void setRow(int i);
    void setCol(int i);

    void setValueFromIndex(int index);

    UniformMat4Parameter* parameter() { return mUniformMat4Parameter; }
    int typeIndex() const;

    // QGroupBox* widget();

private:
    UniformMat4Parameter* mUniformMat4Parameter;
    QList<FocusLineEdit*> mLineEdits;
    int mLastIndex;

    void setValidators();
};



#endif // UNIFORMMAT4WIDGET_H
