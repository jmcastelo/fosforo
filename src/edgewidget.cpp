


#include "edgewidget.h"
#include "widgets/focuswidgets.h"



EdgeWidget::EdgeWidget(Number<float>* blendFactor, bool srcIsOp, Factory *factory, QWidget *parent) :
    QWidget { parent },
    mBlendFactor { blendFactor },
    mFactory { factory }
{
    // setAttribute(Qt::WA_DeleteOnClose, false);

    // Blend factor line edit

    FocusLineEdit* lineEdit = new FocusLineEdit;
    lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QDoubleValidator* validator = new QDoubleValidator(0.0, 1.0, 6, lineEdit);
    validator->setLocale(QLocale::English);
    lineEdit->setValidator(validator);
    lineEdit->setText(QString::number(mBlendFactor->value()));

    // Header toolbar

    headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Midi link

    mMidiSignals = new MidiSignals(this);

    midiLinkAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/circle-grey.png")), "Midi link");
    midiLinkAction->setCheckable(true);
    midiLinkAction->setVisible(false);


    // Set edge type

    mTypeAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/edit-undo.png")), "Set as predge");
    mTypeAction->setCheckable(true);
    mTypeAction->setVisible(srcIsOp);

    // Insert operation

    mInsertOpAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/go-bottom.png")), "Insert", this, &EdgeWidget::populateAvailOpsMenu);
    mAvailOpsMenu = new QMenu("Operation");
    mInsertOpAction->setMenu(mAvailOpsMenu);

    connect(mAvailOpsMenu, &QMenu::triggered, this, &EdgeWidget::insertOperation);

    // Delete

    mRemoveAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/dialog-close.png")), "Delete");

    // Slider

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    slider->setRange(0, mBlendFactor->indexMax());
    slider->setValue(mBlendFactor->index());

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(lineEdit);
    headerLayout->addWidget(headerToolBar);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(0);
    layout->addLayout(headerLayout);
    layout->addWidget(slider);

    connect(midiLinkAction, &QAction::toggled, this, [=, this](bool checked){
        if (checked)
        {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-orange.png")));
            emit mMidiSignals->linkWait(mBlendFactor);
        }
        else
        {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-grey.png")));
            emit mMidiSignals->linkBreak(mBlendFactor);
        }
    });

    connect(mTypeAction, &QAction::triggered, this, &EdgeWidget::typeActionToggled);

    connect(mRemoveAction, &QAction::triggered, this, &EdgeWidget::remove);

    connect(mBlendFactor, &Number<float>::linked, this, [=, this](bool set){
        if (set) {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-green.png")));
        }
        else {
            midiLinkAction->setIcon(QIcon(QPixmap(":/icons/circle-grey.png")));
        }

        slider->setRange(0, mBlendFactor->indexMax());
    });

    connect(slider, &QAbstractSlider::sliderMoved, mBlendFactor, &Number<float>::setValueFromIndex);

    connect(mBlendFactor, &Number<float>::indexChanged, slider, &QAbstractSlider::setValue);

    connect(mBlendFactor, &Number<float>::valueChanged, this, [=, this](QVariant value){
        lineEdit->setText(QString::number(value.toFloat()));
        mBlendFactor->setIndex();
    });

    connect(lineEdit, &QLineEdit::returnPressed, this, [=, this](){
        mBlendFactor->setValue(lineEdit->text().toFloat());
        mBlendFactor->setIndex();
    });

    connect(lineEdit, &FocusLineEdit::focusOut, this, [=, this](){
        lineEdit->setText(QString::number(mBlendFactor->value()));
    });

    setLayout(layout);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    adjustAllSizes();
}



const QString EdgeWidget::name()
{
    return mName;
}



void EdgeWidget::setName(QString name)
{
    mName = name;
}



void EdgeWidget::setBlendFactor(float factor)
{
    mBlendFactor->setValue(factor);
}



MidiSignals* EdgeWidget::midiSignals()
{
    return mMidiSignals;
}



void EdgeWidget::toggleMidiAction(bool show)
{
    midiLinkAction->setVisible(show);
    adjustAllSizes();
}



void EdgeWidget::toggleTypeAction(bool predge)
{
    mTypeAction->setChecked(predge);
    emit edgeTypeChanged(predge);
}



void EdgeWidget::adjustAllSizes()
{
    headerToolBar->adjustSize();
    adjustSize();
}



void EdgeWidget::populateAvailOpsMenu()
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



void EdgeWidget::insertOperation(QAction* action)
{
    if (mAvailOpsActions.contains(action))
    {
        int index = mAvailOpsActions.indexOf(action);

        emit operationInsert(index);
        emit remove();
    }
}
