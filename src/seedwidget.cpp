


#include "seedwidget.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QFileDialog>



SeedWidget::SeedWidget(QUuid id, Seed* seed, VideoInputControl* videoInCtrl, QWidget* parent) :
    QWidget{ parent },
    mId { id },
    mSeed { seed },
    mVideoInputControl { videoInCtrl }
{
    // Header widget

    headerWidget = new QWidget;
    headerWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    headerWidget->setObjectName("header");
    headerWidget->setStyleSheet("QWidget#header { border: 1px dotted gray; }");

    QToolBar* headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Seed type actions

    colorAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/color-chooser.png")), "Random: color", this, &SeedWidget::setSeedType);
    colorAction->setCheckable(true);
    colorAction->setData(QVariant(0));

    grayScaleAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/gray-chooser.png")), "Random: grayscale", this, &SeedWidget::setSeedType);
    grayScaleAction->setCheckable(true);
    grayScaleAction->setData(QVariant(1));

    headerToolBar->addSeparator();

    imageAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/image-x-generic.png")), "Image", this, &SeedWidget::setSeedType);
    imageAction->setEnabled(mSeed->type() == 2);
    imageAction->setCheckable(true);
    imageAction->setData(QVariant(2));

    headerToolBar->addAction(QIcon(QPixmap(":/icons/folder-image.png")), "Load image", this, &SeedWidget::loadSeedImage);

    headerToolBar->addSeparator();

    videoAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/camera-web.png")), "Video", this, &SeedWidget::populateAvailVideoMenu);
    mAvailVideoMenu = new QMenu("Inputs");
    videoAction->setMenu(mAvailVideoMenu);

    connect(mAvailVideoMenu, &QMenu::triggered, this, &SeedWidget::selectVideo);

    QActionGroup* type = new QActionGroup(this);
    type->addAction(colorAction);
    type->addAction(grayScaleAction);
    type->addAction(imageAction);

    colorAction->setChecked(mSeed->type() == 0);
    grayScaleAction->setChecked(mSeed->type() == 1);
    imageAction->setChecked(mSeed->type() == 2);

    headerToolBar->addSeparator();

    // Set as output action

    outputAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", this, [=, this](bool checked) {
        emit outputChanged(checked ? mId : QUuid());
    });
    outputAction->setCheckable(true);

    // Set fixed action

    fixedAction = headerToolBar->addAction(mSeed->fixed() ? QIcon(QPixmap(":/icons/document-encrypt.png")) : QIcon(QPixmap(":/icons/document-decrypt.png")), mSeed->fixed() ? "Fixed" : "Not fixed", this, &SeedWidget::setFixedSeed);
    fixedAction->setCheckable(true);
    fixedAction->setChecked(mSeed->fixed());

    headerToolBar->addSeparator();

    // Connect action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/network-connect.png")), "Connect", this, &SeedWidget::connectTo);

    headerToolBar->addSeparator();

    // Copy action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/edit-copy.png")), "Copy", this, [=, this]() {
        emit copy(mId);
    });

    // Remove action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/dialog-close.png")), "Delete", this, [=, this]() {
        mVideoInputControl->unuseCamera(mSeed->videoDevId());
        emit remove();
    });

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(headerToolBar);

    headerWidget->setLayout(headerLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(headerWidget);

    setLayout(mainLayout);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}



SeedWidget::~SeedWidget()
{
    qDeleteAll(mAvailVideoActions);
}



QUuid SeedWidget::id()
{
    return mId;
}



void SeedWidget::setFixedSeed(bool fixed)
{
    mSeed->setFixed(fixed);

    fixedAction->setIcon(fixed ? QIcon(QPixmap(":/icons/document-encrypt.png")) : QIcon(QPixmap(":/icons/document-decrypt.png")));
    fixedAction->setText(fixed ? "Fixed" : "Not fixed");
}



void SeedWidget::setSeedType()
{
    QAction* action = qobject_cast<QAction*>(sender());
    int type = action->data().toInt();

    mSeed->setType(type);

    emit typeChanged();

    if (type != 3) {
        mVideoInputControl->unuseCamera(mSeed->videoDevId());
        mSeed->setVideoDevId(QByteArray());
    }
}



void SeedWidget::loadSeedImage()
{
    QString filename = QFileDialog::getOpenFileName(this->parentWidget(), "Load image", QDir::homePath(), "Images (*.bmp *.jpeg *.jpg *.png *.tif *.tiff)");

    if (!filename.isEmpty())
    {
        mSeed->loadImage(filename);
        imageAction->setEnabled(true);
    }
}



void SeedWidget::toggleOutputAction(QUuid id)
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



void SeedWidget::populateAvailVideoMenu()
{
    mAvailVideoMenu->clear();

    qDeleteAll(mAvailVideoActions);
    mAvailVideoActions.clear();

    foreach (QString description, mVideoInputControl->cameraDescriptions()) {
        mAvailVideoActions.append(new QAction(description));
    }

    mAvailVideoMenu->addActions(mAvailVideoActions);
    mAvailVideoMenu->exec(QCursor::pos());
}



void SeedWidget::selectVideo(QAction* action)
{
    if (mAvailVideoActions.contains(action))
    {
        int index = mAvailVideoActions.indexOf(action);

        QByteArray id = mSeed->videoDevId();

        mSeed->setVideoDevId(mVideoInputControl->cameraId(index));
        mVideoInputControl->useCamera(index);

        mVideoInputControl->unuseCamera(id);

        mSeed->setType(3);
        emit typeChanged();

        colorAction->setChecked(false);
        grayScaleAction->setChecked(false);
        imageAction->setChecked(false);
    }
}
