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
#include <QStackedLayout>
#include <QGraphicsOpacityEffect>
#include <QChronoTimer>
#include <QElapsedTimer>
#include <QQueue>



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
    // void updateTimeMeasured(double uspf, double currentFPS);

/*protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;*/

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

    // TimerThread* updateTimer;
    // std::chrono::time_point<std::chrono::steady_clock> updateStart;
    // std::chrono::time_point<std::chrono::steady_clock> updateEnd;

    // TimerThread* iterationTimer;
    // std::chrono::time_point<std::chrono::steady_clock> iterationStart;
    // std::chrono::time_point<std::chrono::steady_clock> iterationEnd;

    // int numUpdates = 0;
    // double updateFPS = 60.0;
    // std::chrono::microseconds updateTime;

    // int numIterations = 0;
    // double iterationFPS = 60.0;
    // std::chrono::microseconds iterationTime;

    std::chrono::time_point<std::chrono::steady_clock> stepStart;
    std::chrono::time_point<std::chrono::steady_clock> stepEnd;
    std::chrono::time_point<std::chrono::steady_clock> multiStepStart;
    unsigned int numSteps = 0;
    std::chrono::nanoseconds stepTime;
    std::chrono::milliseconds multiStepTime;

    // QQueue<qint64> timestamps;

    void closeAll();

private slots:
    // void computeUpdateFPS();
    // void computeIterationFPS();

    void setIterationTimerInterval(double fps);

    void measureFps();

    void setIterationState(bool state);

    void setSize(int with, int height);

    void showMidiWidget();

    void onOutputWindowClose();
    void onControlWidgetClose();
};


#endif // APPLICATIONCONTROLLER_H
