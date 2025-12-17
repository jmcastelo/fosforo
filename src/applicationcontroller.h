#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H



#include "factory.h"
#include "rendermanager.h"
#include "nodemanager.h"
#include "outputwindow.h"
#include "graphwidget.h"
#include "configparser.h"
#include "controlwidget.h"
#include "plotswidget.h"
#include "midicontrol.h"
#include "midilistwidget.h"
#include "midilinkmanager.h"
#include "overlay.h"
#include "videoinputcontrol.h"

#include <QObject>
#include <QTimer>



class ApplicationController : public QObject
{
    Q_OBJECT

public:
    ApplicationController();
    ~ApplicationController();

signals:
    void outputTextureChanged(GLuint id);

    void iterationPerformed();
    void iterationTimeMeasured(double uspf, double currentFPS);

private:
    Factory* factory;
    RenderManager* renderManager;
    NodeManager* nodeManager;
    OutputWindow* outputWindow;
    GraphWidget* graphWidget;
    ConfigurationParser* configParser;
    ControlWidget* controlWidget;
    PlotsWidget* plotsWidget;
    MidiControl* midiControl;
    MidiListWidget* midiListWidget;
    MidiLinkManager midiLinkManager;
    Overlay* overlay;
    VideoInputControl* videoInControl;

    std::chrono::time_point<std::chrono::steady_clock> stepStart;
    std::chrono::time_point<std::chrono::steady_clock> stepEnd;
    std::chrono::time_point<std::chrono::steady_clock> multiStepStart;
    unsigned int numSteps = 0;
    std::chrono::nanoseconds stepTime;
    std::chrono::milliseconds multiStepTime;

    QTimer updateViewTimer;

    void closeAll();

private slots:
    void setIterationTimerInterval(double fps);

    void measureFps();

    void setIterationState(bool state);

    void setSize(int with, int height);

    void showMidiWidget();

    void onOutputWindowClose();
    void onControlWidgetClose();
};


#endif // APPLICATIONCONTROLLER_H
