


#include "operationwidget.h"
#include "parameters/uniformmat4parameter.h"
#include "factory.h"

#include <limits>



OperationWidget::OperationWidget(QUuid id, ImageOperation* operation, bool midiEnabled, bool editMode, Factory *factory, QWidget* parent) :
    QWidget { parent },
    mId { id },
    mOperation { operation },
    mFactory { factory },
    mMidiEnabled { midiEnabled }
{
    mMidiSignals = new MidiSignals(this);

    mOpBuilder = new OperationBuilder(mOperation);
    mOpBuilder->installEventFilter(this);
    mOpBuilder->setVisible(false);

    connect(mOpBuilder, &OperationBuilder::operationSetUp, this, &OperationWidget::recreate);

    mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);

    // Header widget

    headerWidget = new QWidget;
    headerWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    headerWidget->setObjectName("header");
    headerWidget->setStyleSheet("QWidget#header { border: 1px dotted gray; }");

    // Header toolbar

    headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Enable action

    enableAction = headerToolBar->addAction(mOperation->enabled() ? QIcon(QPixmap(":/icons/circle-green.png")) : QIcon(QPixmap(":/icons/circle-grey.png")), mOperation->enabled() ? "Enabled" : "Disabled", this, &OperationWidget::enableOperation);
    enableAction->setCheckable(true);
    enableAction->setChecked(mOperation->enabled());

    // Output action

    outputAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", this, [=, this](bool checked) {
        emit outputChanged(checked ? mId : QUuid());
    });
    outputAction->setCheckable(true);

    // Replace operation action and menu

    replaceOpAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/view-refresh-2.png")), "Replace", this, &OperationWidget::populateAvailOpsMenu);
    mAvailOpsMenu = new QMenu("Replace with");
    replaceOpAction->setMenu(mAvailOpsMenu);

    connect(mAvailOpsMenu, &QMenu::triggered, this, &OperationWidget::replaceOperation);

    // Edit action

    editAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/applications-development.png")), "Edit", this, &OperationWidget::toggleEditMode);
    editAction->setCheckable(true);
    editAction->setChecked(editMode);

    // Connect action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/network-connect.png")), "Connect", this, &OperationWidget::connectTo);

    // Equalize blend factors action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/preferences-desktop.png")), "Equalize blend factors", this, [=, this]() {
        emit equalizeBlendFactors(mId);
    });

    // Toggle body action

    toggleBodyAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/go-down.png")), "Hide", this, &OperationWidget::toggleBody);
    toggleBodyAction->setCheckable(true);
    toggleBodyAction->setChecked(true);

    headerToolBar->addSeparator();

    // Copy action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/edit-copy.png")), "Copy", this, [=, this]() {
        emit copy(mId);
    });

    // Delete action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/dialog-close.png")), "Delete", this, [=, this]() {
        emit remove(mId);
    });

    // Operation name label

    opNameLabel = new QLabel;
    opNameLabel->setMargin(8);
    opNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    opNameLabel->setStyleSheet("QLabel { font-size: 14pt; }");

    // Operation name line edit

    opNameLineEdit = new QLineEdit;
    opNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    opNameLineEdit->setStyleSheet("QLineEdit { font-size: 14pt; }");

    connect(opNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        operation->setName(name);
        opNameLabel->setText(name);

        opNameLineEdit->setFixedWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(name));

        headerWidget->adjustSize();
        selParamWidget->adjustSize();
        adjustSize();
    });

    QVBoxLayout* headerLayout = new QVBoxLayout;
    headerLayout->setContentsMargins(1, 1, 1, 1);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(opNameLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(opNameLineEdit, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addWidget(headerToolBar, 0, Qt::AlignLeft | Qt::AlignVCenter);

    headerWidget->setLayout(headerLayout);

    mainLayout->addWidget(headerWidget);

    // Grid widget containing parameter widgets

    gridWidget = new GridWidget;

    connect(gridWidget, &GridWidget::itemRowColChanged, this, &OperationWidget::updateWidgetRowCol);

    mainLayout->addWidget(gridWidget, 1, Qt::AlignCenter);

    // Selected parameter widgets

    // Parameter name widgets

    paramNameLabel = new QLabel;
    paramNameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    paramNameLabel->setText("Parameter name");

    paramNameLineEdit = new QLineEdit;
    paramNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    paramNameLineEdit->setPlaceholderText("Parameter name");

    // Slider

    selParamSlider = new QSlider(Qt::Horizontal);
    selParamSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    selParamSlider->setRange(0, 100'000);

    // Midi link button

    midiLinkButton = new QPushButton;
    midiLinkButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    midiLinkButton->setCheckable(true);
    midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");

    // Limits widgets

    selParamMinLineEdit = new FocusLineEdit;
    selParamMinLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamMinLineEdit->setPlaceholderText("Minimum");

    selParamMaxLineEdit = new FocusLineEdit;
    selParamMaxLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamMaxLineEdit->setPlaceholderText("Maximum");

    selParamInfLineEdit = new FocusLineEdit;
    selParamInfLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamInfLineEdit->setPlaceholderText("Lowest");

    selParamSupLineEdit = new FocusLineEdit;
    selParamSupLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    selParamSupLineEdit->setPlaceholderText("Highest");

    // Layout combo box

    layoutComboBox = new QComboBox;
    layoutComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layoutComboBox->setVisible(false);

    // Mat4 combo box

    mat4TypeComboBox = new QComboBox;
    mat4TypeComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mat4TypeComboBox->setVisible(false);
    mat4TypeComboBox->addItems(QList<QString>{ "Translation", "Rotation", "Scaling", "Orthographic", "Mat4" });

    // Presets widgets

    removePresetButton = new QPushButton;
    removePresetButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    removePresetButton->setFixedSize(32, 32);
    removePresetButton->setStyleSheet("QPushButton { image: url(:/icons/list-remove.png); background-color: transparent; border: 0; }");

    addPresetButton = new QPushButton;
    addPresetButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    addPresetButton->setFixedSize(32, 32);
    addPresetButton->setStyleSheet("QPushButton { image: url(:/icons/list-add.png); background-color: transparent; border: 0; }");

    presetNameLineEdit = new QLineEdit;
    presetNameLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    presetNameLineEdit->setMaximumWidth(100);
    presetNameLineEdit->setPlaceholderText("Preset name");

    QGridLayout* presetsLayout = new QGridLayout;
    presetsLayout->addWidget(removePresetButton, 0, 0);
    presetsLayout->addWidget(addPresetButton, 0, 1);
    presetsLayout->addWidget(presetNameLineEdit, 0, 2);

    // Selected parameter layout

    QVBoxLayout* selParamLayout = new QVBoxLayout;

    QHBoxLayout* paramNameLayout = new QHBoxLayout;
    paramNameLayout->addWidget(paramNameLabel, 1, Qt::AlignLeft);
    paramNameLayout->addWidget(paramNameLineEdit, 1, Qt::AlignLeft);
    paramNameLayout->addWidget(midiLinkButton, 0, Qt::AlignRight);

    selParamLayout->addLayout(paramNameLayout);

    selParamLayout->addWidget(selParamSlider);

    QHBoxLayout* limitsLayout = new QHBoxLayout;
    limitsLayout->addWidget(selParamInfLineEdit);
    limitsLayout->addWidget(selParamMinLineEdit);
    limitsLayout->addWidget(selParamMaxLineEdit);
    limitsLayout->addWidget(selParamSupLineEdit);

    selParamLayout->addLayout(limitsLayout);

    QHBoxLayout* combosLayout = new QHBoxLayout;
    combosLayout->addLayout(presetsLayout);
    combosLayout->addWidget(layoutComboBox);
    combosLayout->addWidget(mat4TypeComboBox);

    selParamLayout->addLayout(combosLayout);

    // Selected parameter widget

    selParamWidget = new QWidget;
    selParamWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    selParamWidget->setObjectName("selParamWidget");
    selParamWidget->setStyleSheet("QWidget#selParamWidget { border-top: 1px dotted gray; }");

    selParamWidget->setLayout(selParamLayout);

    mainLayout->addWidget(selParamWidget, 0, Qt::AlignTop);

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // toggleEditMode(editMode);
}



OperationWidget::~OperationWidget()
{
    delete mOpBuilder;
    qDeleteAll(mAvailOpsActions);
}



void OperationWidget::setup()
{
    lastFocusedWidget = nullptr;

    // Parameter widgets

    foreach (auto parameter, mOperation->uniformParameters<float>())
    {
        UniformParameterWidget<float>* widget = new UniformParameterWidget<float>(parameter);

        if (mEditMode || (!mEditMode && parameter->editable())) {
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            widget->setUpVisible();
        }

        widget->setCheckable(mEditMode);

        floatParamWidgets.append(widget);
        uniformFloatParamWidgets.append(widget);

        setFocusedWidget<float>(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<int>())
    {
        UniformParameterWidget<int>* widget = new UniformParameterWidget<int>(parameter);

        if (mEditMode || (!mEditMode && parameter->editable())) {
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            widget->setUpVisible();
        }

        widget->setCheckable(mEditMode);

        intParamWidgets.append(widget);
        uniformIntParamWidgets.append(widget);

        setFocusedWidget<int>(widget);
    }

    foreach (auto parameter, mOperation->uniformParameters<unsigned int>())
    {
        UniformParameterWidget<unsigned int>* widget = new UniformParameterWidget<unsigned int>(parameter);

        if (mEditMode || (!mEditMode && parameter->editable())) {
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            widget->setUpVisible();
        }

        widget->setCheckable(mEditMode);

        uintParamWidgets.append(widget);
        uniformUintParamWidgets.append(widget);

        setFocusedWidget<unsigned int>(widget);
    }

    foreach (auto parameter, mOperation->mat4UniformParameters())
    {
        UniformMat4ParameterWidget* widget = new UniformMat4ParameterWidget(parameter);

        if (mEditMode || (!mEditMode && parameter->editable())) {
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
            widget->setUpVisible();
        }

        widget->setCheckable(mEditMode);

        mat4ParamWidgets.append(widget);
        uniformMat4ParamWidgets.append(widget);

        setFocusedWidget<float>(widget);
    }

    foreach (auto parameter, mOperation->optionsParameters<GLenum>())
    {
        OptionsParameterWidget<GLenum>* widget = new OptionsParameterWidget<GLenum>(parameter);

        if (mEditMode || (!mEditMode && parameter->editable())) {
            gridWidget->addWidget(widget->widget(), parameter->row(), parameter->col());
        }

        widget->setCheckable(mEditMode);

        glenumOptionsWidgets.append(widget);
    }

    // Selected parameter widget

    if (!floatParamWidgets.empty() || !intParamWidgets.empty() || !uintParamWidgets.empty() || !mat4ParamWidgets.empty())
    {
        connectParamWidgets<float>(floatParamWidgets);
        connectParamWidgets<int>(intParamWidgets);
        connectParamWidgets<unsigned int>(uintParamWidgets);
        connectParamWidgets<float>(mat4ParamWidgets);

        connectUniformParamWidgets<float>(uniformFloatParamWidgets);
        connectUniformParamWidgets<int>(uniformIntParamWidgets);
        connectUniformParamWidgets<unsigned int>(uniformUintParamWidgets);

        connectUniformFloatParamWidgets();

        connectUniformMat4ParamWidgets();

        if (lastFocusedWidget) {
            lastFocusedWidget->setFocus();
        }
    }

    // Operation name controls

    opNameLabel->setText(mOperation->name());

    opNameLineEdit->setText(mOperation->name());
    opNameLineEdit->setFixedWidth(20 + opNameLineEdit->fontMetrics().horizontalAdvance(mOperation->name()));

    // Midi link button

    midiLinkButton->setVisible(mMidiEnabled);

    // Once widgets set on grid, optimize its layout to set it with proper row and column spans and sizes
    // Operation widget must be visible: show it

    if (!isVisible()) {
        show();
    }

    gridWidget->optimizeLayout();

    headerWidget->adjustSize();
    selParamWidget->adjustSize();
    adjustSize();
}



void OperationWidget::recreate()
{
    minValidator = nullptr;
    maxValidator = nullptr;

    infValidator = nullptr;
    supValidator = nullptr;

    gridWidget->clear();

    qDeleteAll(floatParamWidgets);
    floatParamWidgets.clear();
    uniformFloatParamWidgets.clear();

    qDeleteAll(intParamWidgets);
    intParamWidgets.clear();
    uniformIntParamWidgets.clear();

    qDeleteAll(uintParamWidgets);
    uintParamWidgets.clear();
    uniformUintParamWidgets.clear();

    qDeleteAll(mat4ParamWidgets);
    mat4ParamWidgets.clear();
    uniformMat4ParamWidgets.clear();

    qDeleteAll(glenumOptionsWidgets);
    glenumOptionsWidgets.clear();

    setup();
}



template <typename T>
void OperationWidget::setFocusedWidget(ParameterWidget<T>* widget)
{
    if (widget->parameter() == lastFocusedParameter)
        lastFocusedWidget = widget->lastFocusedWidget();
}



template <typename T>
void OperationWidget::connectParamWidgets(QList<ParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &ParameterWidget<T>::focusIn, this, [=, this](){
            lastFocusedParameter = widget->parameter();
            lastFocusedWidget = widget->lastFocusedWidget();
        });
    }
}



template <typename T>
void OperationWidget::connectUniformParamWidgets(QList<UniformParameterWidget<T>*> widgets)
{
    foreach (auto widget, widgets)
    {
        connect(widget, &UniformParameterWidget<T>::focusIn, this, [=, this](){
            if (mEditMode) {
                updateSelParamEditControls<T>(widget);
            }
            else {
                updateSelParamControls<T>(widget);
            }

            updateMidiButton<T>(widget);

            setSelParamNameWidgets<T>(widget);
        });

        connect(widget, &UniformParameterWidget<T>::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up layout controller combo box

                if (layoutComboBoxConn)
                    disconnect(layoutComboBoxConn);

                layoutComboBox->clear();

                if (widget->availableLayoutFormats().size() == 0)
                    layoutComboBox->setVisible(false);
                else
                {
                    layoutComboBox->setVisible(true);

                    layoutComboBox->addItems(widget->availableLayoutFormats());
                    layoutComboBox->setCurrentIndex(widget->layoutFormatIndex());

                    layoutComboBoxConn = connect(layoutComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                        widget->setLayoutFormatIndex(index);
                        gridWidget->optimizeLayout();
                        adjustSize();
                    });
                }
            }
        });
    }
}



void OperationWidget::connectUniformFloatParamWidgets()
{
    foreach (auto widget, uniformFloatParamWidgets)
    {
        connect(widget, &UniformParameterWidget<float>::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up Mat4 parameter switcher combo box

                if (mat4TypeComboBoxConn)
                    disconnect(mat4TypeComboBoxConn);

                if (widget->isMat4Equivalent())
                {
                    mat4TypeComboBox->setVisible(true);
                    mat4TypeComboBox->setCurrentIndex(4);

                    mat4TypeComboBoxConn = connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                        if (index >= 0 && index < 4)
                        {
                            UniformParameter<float>* parameter = widget->parameter();

                            UniformMat4Parameter* newParam = new UniformMat4Parameter(*parameter, static_cast<UniformMat4Type>(index));
                            mOpBuilder->addMat4UniformParameter(newParam);
                            mOpBuilder->removeUniformFloatParameter(parameter);

                            lastFocusedParameter = newParam;

                            if (index < 3)
                                addInterpolation();

                            recreate();
                        }
                    });
                }
                else
                    mat4TypeComboBox->setVisible(false);
            }
        });
    }
}



void OperationWidget::connectUniformMat4ParamWidgets()
{
    foreach (auto widget, uniformMat4ParamWidgets)
    {
        connect(widget, &UniformMat4ParameterWidget::focusIn, this, [=, this](){
            if (!widget->parameter()->empty())
            {
                if (mEditMode)
                    updateSelParamEditControls<float>(widget);
                else
                    updateSelParamControls<float>(widget);

                updateMidiButton<float>(widget);
            }
            else
            {
                toggleSelParamWidgets(false);
            }

            setSelParamNameWidgets<float>(widget);
        });

        connect(widget, &UniformMat4ParameterWidget::focusIn, this, [=, this](){
            if (mEditMode)
            {
                // Set up Mat4 parameter switcher combo box

                if (mat4TypeComboBoxConn)
                    disconnect(mat4TypeComboBoxConn);

                mat4TypeComboBox->setVisible(true);
                mat4TypeComboBox->setCurrentIndex(widget->typeIndex());

                mat4TypeComboBoxConn = connect(mat4TypeComboBox, &QComboBox::currentIndexChanged, this, [=, this](int index){
                    if (index >= 0 && index < 4)
                    {
                        UniformMat4Parameter* parameter = widget->parameter();
                        parameter->setType(static_cast<UniformMat4Type>(index));

                        lastFocusedParameter = parameter;

                        if (index < 3)
                            addInterpolation();
                        else if (index == 3)
                            removeInterpolation();

                        recreate();
                    }
                    else if (index == 4)
                    {
                        UniformMat4Parameter* parameter = widget->parameter();

                        UniformParameter<float>* newParam = new UniformParameter<float>(*parameter);
                        mOpBuilder->addUniformFloatParameter(newParam);
                        mOpBuilder->removeMat4UniformParameter(parameter);

                        lastFocusedParameter = newParam;

                        removeInterpolation();

                        recreate();
                    }
                });
            }
        });
    }
}



void OperationWidget::addInterpolation()
{
    bool interpolationExists = false;

    foreach (auto optionsWidget, glenumOptionsWidgets)
        if (optionsWidget->name() == "Interpolation")
            interpolationExists = true;

    if (!interpolationExists)
    {
        OptionsParameter<GLenum>* newParam = new OptionsParameter<GLenum>("Interpolation", true, QList<QString>({ "Nearest neighbor", "Linear" }), QList<GLenum>({ GL_NEAREST, GL_LINEAR }), GL_NEAREST, mOperation);
        mOperation->addOptionsParameter<GLenum>(newParam);
    }
}



void OperationWidget::removeInterpolation()
{
    foreach (auto optionsWidget, glenumOptionsWidgets)
    {
        if (optionsWidget->name() == "Interpolation")
            mOperation->removeOptionsParameter<GLenum>(optionsWidget->parameter());
    }
}



void OperationWidget::toggleOutputAction(QUuid id)
{
    bool checked = (id == mId);

    outputAction->setChecked(checked);

    if (checked) {
        headerWidget->setStyleSheet("QWidget#header { border: 1px solid orange; }");
    }
    else {
        headerWidget->setStyleSheet("QWidget#header { border: 1px dotted gray; }");
    }
}



void OperationWidget::toggleMidiButton(bool show)
{
    midiLinkButton->setVisible(show);
    mMidiEnabled = show;
}



QUuid OperationWidget::id()
{
    return mId;
}



MidiSignals* OperationWidget::midiSignals()
{
    return mMidiSignals;
}



void OperationWidget::toggleSelParamWidgets(bool visible)
{
    selParamSlider->setVisible(visible);
    selParamMinLineEdit->setVisible(visible);
    selParamMaxLineEdit->setVisible(visible);
    selParamInfLineEdit->setVisible(visible);
    selParamSupLineEdit->setVisible(visible);
    removePresetButton->setVisible(visible);
    addPresetButton->setVisible(visible);
    presetNameLineEdit->setVisible(visible);
    layoutComboBox->setVisible(visible);
}



void OperationWidget::enableOperation(bool checked)
{
    mOperation->enable(checked);

    enableAction->setIcon(checked ? QIcon(QPixmap(":/icons/circle-green.png")) : QIcon(QPixmap(":/icons/circle-grey.png")));
    enableAction->setText(checked ? "Enabled" : "Disabled");
}



/*void OperationWidget::closeEvent(QCloseEvent* event)
{
    mOpBuilder->close();
    event->accept();
}*/



bool OperationWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mOpBuilder && event->type() == QEvent::Close)
    {
        editAction->setChecked(false);
        toggleEditMode(false);
        return false;
    }

    return QWidget::eventFilter(obj, event);
}



template <class T>
void OperationWidget::setSelParamNameWidgets(ParameterWidget<T>* widget)
{
    // Name line edit

    paramNameLabel->setText(widget->name());

    paramNameLineEdit->setText(widget->name());
    paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(widget->name()));

    if (paramNameLineEditConn)
        disconnect(paramNameLineEditConn);

    paramNameLineEditConn = connect(paramNameLineEdit, &QLineEdit::textEdited, this, [=, this](QString name){
        widget->setName(name);

        paramNameLabel->setText(name);

        paramNameLineEdit->setFixedWidth(20 + paramNameLineEdit->fontMetrics().horizontalAdvance(name));

        gridWidget->optimizeLayout();
    });
}



template <class T>
void OperationWidget::updateSelParamControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Slider

    selParamSlider->setVisible(true);

    foreach (auto conn, selParamSliderConns) {
        disconnect(conn);
    }

    selParamSlider->setRange(0, number->indexMax());
    selParamSlider->setValue(number->index());

    selParamSliderConns.clear();
    selParamSliderConns.resize(3);

    selParamSliderConns[0] = connect(selParamSlider, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    selParamSliderConns[1] = connect(number, &Number<T>::indexChanged, selParamSlider, &QAbstractSlider::setValue);
    selParamSliderConns[2] = connect(number, &Number<T>::indexMaxChanged, this, [=, this](int indexMax) {
        selParamSlider->setRange(0, indexMax);
    });

    // Minimum

    selParamMinLineEdit->setVisible(true);

    foreach (auto conn, selParamMinLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(minValidator, selParamMinLineEdit, number->inf(), number->max());

    selParamMinLineEdit->setText(QString::number(number->min()));

    selParamMinLineEditConns.clear();
    selParamMinLineEditConns.resize(4);

    selParamMinLineEditConns[0] = connect(number, &Number<T>::minChanged, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    selParamMinLineEditConns[1] = connect(selParamMinLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            number->setMin(selParamMinLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            number->setMin(selParamMinLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            number->setMin(selParamMinLineEdit->text().toUInt());
    });

    selParamMinLineEditConns[2] = connect(selParamMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    selParamMinLineEditConns[3] = connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
    });

    // Maximum

    selParamMaxLineEdit->setVisible(true);

    foreach (auto conn, selParamMaxLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), number->sup());

    selParamMaxLineEdit->setText(QString::number(number->max()));

    selParamMaxLineEditConns.clear();
    selParamMaxLineEditConns.resize(4);

    selParamMaxLineEditConns[0] = connect(number, &Number<T>::maxChanged, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    selParamMaxLineEditConns[1] = connect(selParamMaxLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value)
            number->setMax(selParamMaxLineEdit->text().toFloat());
        else if (std::is_same<T, int>::value)
            number->setMax(selParamMaxLineEdit->text().toInt());
        else if (std::is_same<T, unsigned int>::value)
            number->setMax(selParamMaxLineEdit->text().toUInt());
    });

    selParamMaxLineEditConns[2] = connect(selParamMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    selParamMaxLineEditConns[3] = connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
    });
}



template <class T>
void OperationWidget::updateSelParamEditControls(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // Slider

    selParamSlider->setVisible(true);

    foreach (auto conn, selParamSliderConns) {
        disconnect(conn);
    }

    selParamSlider->setRange(0, number->indexMax());
    selParamSlider->setValue(number->index());

    selParamSliderConns.clear();
    selParamSliderConns.resize(3);

    selParamSliderConns[0] = connect(selParamSlider, &QAbstractSlider::sliderMoved, widget, &ParameterWidget<T>::setValueFromIndex);
    selParamSliderConns[1] = connect(number, &Number<T>::indexChanged, selParamSlider, &QAbstractSlider::setValue);
    selParamSliderConns[2] = connect(number, &Number<T>::indexMaxChanged, this, [=, this](int indexMax) {
        selParamSlider->setRange(0, indexMax);
    });

    // Inf (lowest)

    selParamInfLineEdit->setVisible(true);

    foreach (auto conn, selParamInfLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(infValidator, selParamInfLineEdit, std::numeric_limits<T>::lowest(), number->sup());

    selParamInfLineEdit->setText(QString::number(number->inf()));

    selParamInfLineEditConns.clear();
    selParamInfLineEditConns.resize(3);

    selParamInfLineEditConns[0] = connect(selParamInfLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        T inf = number->inf();

        if (std::is_same<T, float>::value)
            inf = selParamInfLineEdit->text().toFloat();
        else if (std::is_same<T, int>::value)
            inf = selParamInfLineEdit->text().toInt();
        else if (std::is_same<T, unsigned int>::value)
            inf = selParamInfLineEdit->text().toUInt();

        widget->setInf(inf);

        setValidator<T>(minValidator, selParamMinLineEdit, inf, number->max());
        setValidator<T>(supValidator, selParamSupLineEdit, inf, std::numeric_limits<T>::max());
    });

    selParamInfLineEditConns[1] = connect(selParamInfLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamInfLineEdit->setText(QString::number(number->inf()));
    });

    selParamInfLineEditConns[2] = connect(selParamInfLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamInfLineEdit;
    });

    // Minimum

    selParamMinLineEdit->setVisible(true);

    foreach (auto conn, selParamMinLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(minValidator, selParamMinLineEdit, number->inf(), number->max());

    selParamMinLineEdit->setText(QString::number(number->min()));

    selParamMinLineEditConns.clear();
    selParamMinLineEditConns.resize(4);

    selParamMinLineEditConns[0] = connect(number, &Number<T>::minChanged, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
        number->setMin(number->min());
    });

    selParamMinLineEditConns[1] = connect(selParamMinLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value) {
            number->setMin(selParamMinLineEdit->text().toFloat());
        }
        else if (std::is_same<T, int>::value) {
            number->setMin(selParamMinLineEdit->text().toInt());
        }
        else if (std::is_same<T, unsigned int>::value) {
            number->setMin(selParamMinLineEdit->text().toUInt());
        }
    });

    selParamMinLineEditConns[2] = connect(selParamMinLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMinLineEdit->setText(QString::number(number->min()));
    });

    selParamMinLineEditConns[3] = connect(selParamMinLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMinLineEdit;
    });

    // Sup (highest)

    selParamSupLineEdit->setVisible(true);

    foreach (auto conn, selParamSupLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(supValidator, selParamSupLineEdit, number->inf(), std::numeric_limits<T>::max());

    selParamSupLineEdit->setText(QString::number(number->sup()));

    selParamSupLineEditConns.clear();
    selParamSupLineEditConns.resize(3);

    selParamSupLineEditConns[0] = connect(selParamSupLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        T sup = number->sup();

        if (std::is_same<T, float>::value) {
            sup = selParamSupLineEdit->text().toFloat();
        }
        else if (std::is_same<T, int>::value) {
            sup = selParamSupLineEdit->text().toInt();
        }
        else if (std::is_same<T, unsigned int>::value) {
            sup = selParamSupLineEdit->text().toUInt();
        }

        widget->setSup(sup);
        setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), sup);
        setValidator<T>(infValidator, selParamInfLineEdit, std::numeric_limits<T>::lowest(), sup);
    });

    selParamSupLineEditConns[1] = connect(selParamSupLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamSupLineEdit->setText(QString::number(number->sup()));
    });

    selParamSupLineEditConns[2] = connect(selParamSupLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamSupLineEdit;
    });

    // Maximum

    selParamMaxLineEdit->setVisible(true);

    foreach (auto conn, selParamMaxLineEditConns) {
        disconnect(conn);
    }

    setValidator<T>(maxValidator, selParamMaxLineEdit, number->min(), number->sup());

    selParamMaxLineEdit->setText(QString::number(number->max()));

    selParamMaxLineEditConns.clear();
    selParamMaxLineEditConns.resize(4);

    selParamMaxLineEditConns[0] = connect(number, &Number<T>::maxChanged, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
        number->setMax(number->max());
    });

    selParamMaxLineEditConns[1] = connect(selParamMaxLineEdit, &FocusLineEdit::editingFinished, this, [=, this](){
        if (std::is_same<T, float>::value) {
            number->setMax(selParamMaxLineEdit->text().toFloat());
        }
        else if (std::is_same<T, int>::value) {
            number->setMax(selParamMaxLineEdit->text().toInt());
        }
        else if (std::is_same<T, unsigned int>::value) {
            number->setMax(selParamMaxLineEdit->text().toUInt());
        }
    });

    selParamMaxLineEditConns[2] = connect(selParamMaxLineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        selParamMaxLineEdit->setText(QString::number(number->max()));
    });

    selParamMaxLineEditConns[3] = connect(selParamMaxLineEdit, &FocusLineEdit::focusIn, this, [=, this](){
        lastFocusedWidget = selParamMaxLineEdit;
    });

    // Presets

    removePresetButton->setVisible(true);
    addPresetButton->setVisible(true);
    presetNameLineEdit->setVisible(true);

    if (removePresetButtonConn)
        disconnect(removePresetButtonConn);

    removePresetButtonConn = connect(removePresetButton, &QPushButton::clicked, this, [=, this](){
        widget->removeCurrentPreset();

        gridWidget->optimizeLayout();
    });

    if (addPresetButtonConn)
        disconnect(addPresetButtonConn);

    addPresetButtonConn = connect(addPresetButton, &QPushButton::clicked, this, [=, this](){
        QString name = presetNameLineEdit->text();
        widget->addPreset(name);

        gridWidget->optimizeLayout();
    });
}



template <class T>
void OperationWidget::setValidator(QValidator* validator, QLineEdit* lineEdit, T bottom, T top)
{
    if (validator)
    {
        delete validator;
        validator = nullptr;
    }

    if (std::is_same<T, float>::value) {
        validator = new QDoubleValidator(bottom, top, 5);
    }
    else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value) {
        if (top > std::numeric_limits<int>::max()) {
            top = std::numeric_limits<int>::max();
        }
        validator = new QIntValidator(bottom, top);
    }

    if (validator) {
        lineEdit->setValidator(validator);
    }
}



template <typename T>
void OperationWidget::updateMidiButton(ParameterWidget<T>* widget)
{
    Number<T>* number = widget->selectedNumber();

    // MIDI Link button

    foreach (auto conn, midiLinkButtonConns) {
        disconnect(conn);
    }

    midiLinkButton->setChecked(number->midiLinked());

    if (number->midiLinked()) {
        midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-green.png); background-color: transparent; border: 0; }");
    } else {
        midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");
    }

    midiLinkButtonConns.clear();
    midiLinkButtonConns.resize(2);

    midiLinkButtonConns[0] = connect(midiLinkButton, &QPushButton::clicked, this, [=, this](bool checked){
        if (checked)
        {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-orange.png); background-color: transparent; border: 0; }");
            emit mMidiSignals->linkWait(number);
        }
        else
        {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");
            emit mMidiSignals->linkBreak(number);
        }
    });

    midiLinkButtonConns[1] = connect(number, &Number<T>::linked, this, [=, this](bool set){
        if (set) {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-green.png); background-color: transparent; border: 0; }");
        } else {
            midiLinkButton->setStyleSheet("QPushButton { image: url(:/icons/circle-grey.png); background-color: transparent; border: 0; }");
        }
    });
}



void OperationWidget::focusInEvent(QFocusEvent *event)
{
    if (lastFocusedWidget)
        lastFocusedWidget->setFocus(Qt::MouseFocusReason);
    event->accept();
}



void OperationWidget::updateWidgetRowCol(QWidget* widget, int row, int col)
{
    adjustSize();

    foreach (auto paramWidget, floatParamWidgets)
    {
        if (paramWidget->widget() == widget)
        // if (paramWidget == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, intParamWidgets)
    {
        if (paramWidget->widget() == widget)
        // if (paramWidget == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, uintParamWidgets)
    {
        if (paramWidget->widget() == widget)
        // if (paramWidget == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, mat4ParamWidgets)
    {
        if (paramWidget->widget() == widget)
        // if (paramWidget == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
    foreach (auto paramWidget, glenumOptionsWidgets)
    {
        if (paramWidget->widget() == widget)
        {
            paramWidget->setRow(row);
            paramWidget->setCol(col);
            return;
        }
    }
}



void OperationWidget::toggleBody(bool visible)
{
    toggleBodyAction->setIcon(visible ? QIcon(QPixmap(":/icons/go-down.png")) : QIcon(QPixmap(":/icons/go-up.png")));
    toggleBodyAction->setText(visible ? "Hide" : "Show");

    selParamWidget->setVisible(visible);
    gridWidget->setVisible(visible);

    if (visible)
        mainLayout->setStretchFactor(gridWidget, 1);
    else
        mainLayout->setStretchFactor(gridWidget, 0);

    adjustSize();
}



void OperationWidget::toggleEditMode(bool mode)
{
    mEditMode = mode;

    mOpBuilder->setVisible(mEditMode);

    opNameLabel->setVisible(!mEditMode);
    opNameLineEdit->setVisible(mEditMode);

    if (mEditMode) {
        mainLayout->setAlignment(gridWidget, Qt::AlignTop | Qt::AlignLeft);
    } else {
        mainLayout->setAlignment(gridWidget, Qt::AlignCenter);
    }
    gridWidget->setEditMode(mEditMode);

    paramNameLabel->setVisible(!mEditMode);
    paramNameLineEdit->setVisible(mEditMode);

    selParamInfLineEdit->setVisible(mEditMode);
    selParamSupLineEdit->setVisible(mEditMode);

    if (!mEditMode && mat4TypeComboBox->isVisible())
        mat4TypeComboBox->setVisible(false);

    if (!mEditMode && layoutComboBox->isVisible())
        layoutComboBox->setVisible(false);

    removePresetButton->setVisible(mEditMode);
    addPresetButton->setVisible(mEditMode);
    presetNameLineEdit->setVisible(mEditMode);

    recreate();
}



void OperationWidget::populateAvailOpsMenu()
{
    mAvailOpsMenu->clear();

    qDeleteAll(mAvailOpsActions);
    mAvailOpsActions.clear();

    mFactory->scan();

    foreach (QString opName, mFactory->availableOperationNames()) {
        mAvailOpsActions.append(new QAction(opName));
    }

    mAvailOpsMenu->addActions(mAvailOpsActions);
    mAvailOpsMenu->exec(QCursor::pos());
}



void OperationWidget::replaceOperation(QAction* action)
{
    if (mAvailOpsActions.contains(action))
    {
        int index = mAvailOpsActions.indexOf(action);

        ImageOperation* operation = mFactory->createReplaceOp(mId, mOperation, index);

        mOpBuilder->setOperation(operation);

        ImageOperation* oldOperation = mOperation;
        mOperation = operation;
        mFactory->deleteOperation(oldOperation);

        recreate();
    }
}
