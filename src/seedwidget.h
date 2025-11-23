#ifndef SEEDWIDGET_H
#define SEEDWIDGET_H



#include "seed.h"
#include "videoinputcontrol.h"

#include <QWidget>
#include <QUuid>
#include <QMenu>



class SeedWidget : public QWidget
{
    Q_OBJECT

public:
    SeedWidget(QUuid id, Seed* seed, VideoInputControl* videoInCtrl, QWidget* parent = nullptr);
    ~SeedWidget();

    QUuid id();

signals:
    void outputChanged(QUuid id);
    void typeChanged();
    void seedDrawn();
    void connectTo();
    void remove(QUuid id);
    void copy(QUuid id);


public slots:
    void toggleOutputAction(QUuid id);

private:
    QUuid mId;
    Seed* mSeed;

    VideoInputControl* mVideoInputControl;

    QWidget* headerWidget;

    QAction* colorAction;
    QAction* grayScaleAction;
    QAction* imageAction;
    QAction* videoAction;
    QAction* outputAction;
    QAction* fixedAction;

    QMenu* mAvailVideoMenu;
    QList<QAction*> mAvailVideoActions;

private slots:
    void setFixedSeed(bool fixed);
    void setSeedType();
    void loadSeedImage();
    void populateAvailVideoMenu();
    void selectVideo(QAction* action);
};



#endif // SEEDWIDGET_H
