#include "mainwindow.h"



MainWindow::MainWindow()
{
    iterationTimer = new TimerThread(iterationFPS, this);
    updateTimer = new TimerThread(updateFPS, this);

    videoInControl = new VideoInputControl();

    factory = new Factory(videoInControl);

    renderManager = new RenderManager(factory, videoInControl);

    nodeManager = new NodeManager(factory);

    overlay = new Overlay();

    morphoWidget = new MorphoWidget(renderManager->texWidth(), renderManager->texHeight(), overlay);
    morphoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    morphoWidget->setMinimumSize(0, 0);

    plotsWidget = new PlotsWidget(renderManager);
    plotsWidget->setVisible(false);

    graphWidget = new GraphWidget(factory, nodeManager);
    graphWidget->setMinimumSize(0, 0);
    graphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    configParser = new ConfigurationParser(factory, nodeManager, renderManager, graphWidget, &midiLinkManager);

    controlWidget = new ControlWidget(iterationFPS, updateFPS, graphWidget, nodeManager, renderManager, plotsWidget);
    controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlWidget->setMinimumSize(0, 0);

    controlWidgetOpacityEffect = new QGraphicsOpacityEffect(controlWidget);
    controlWidgetOpacityEffect->setOpacity(opacity);
    controlWidget->setGraphicsEffect(controlWidgetOpacityEffect);

    stackedWidget = new QWidget(this);
    stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    stackedLayout = new QStackedLayout(stackedWidget);
    stackedLayout->setSizeConstraint(QLayout::SetNoConstraint);
    stackedLayout->setStackingMode(QStackedLayout::StackAll);
    stackedLayout->addWidget(controlWidget);
    stackedLayout->addWidget(morphoWidget);

    stackedWidget->setLayout(stackedLayout);

    setCentralWidget(stackedWidget);

    stackedLayout->setCurrentWidget(controlWidget);

    midiListWidget = new MidiListWidget();

    connect(&midiControl, &MidiControl::inputPortsChanged, midiListWidget, &MidiListWidget::populatePortsTable);
    connect(&midiControl, &MidiControl::inputPortOpen, &midiLinkManager, &MidiLinkManager::setupMidi);
    connect(&midiControl, &MidiControl::ccInputMessageReceived, &midiLinkManager, &MidiLinkManager::updateMidiLinks);
    connect(midiListWidget, &MidiListWidget::portSelected, &midiControl, &MidiControl::openPort);
    connect(midiListWidget, &MidiListWidget::multiLinkButtonChecked, &midiLinkManager, &MidiLinkManager::setMultiLink);
    connect(midiListWidget, &MidiListWidget::clearLinksButtonClicked, &midiLinkManager, &MidiLinkManager::clearLinks);
    connect(&midiLinkManager, &MidiLinkManager::multiLinkSet, midiListWidget, &MidiListWidget::toggleMultiLinkButton);
    connect(&midiLinkManager, &MidiLinkManager::midiLinkSet, midiListWidget, &MidiListWidget::checkPort);
    connect(&midiLinkManager, &MidiLinkManager::midiEnabled, nodeManager, &NodeManager::midiEnabled);
    connect(&midiLinkManager, &MidiLinkManager::midiEnabled, factory, &Factory::setMidiEnabled);

    midiControl.setInputPorts();

    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::beat);
    connect(iterationTimer, &TimerThread::timeout, this, &MainWindow::computeIterationFPS);

    connect(updateTimer, &TimerThread::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    connect(updateTimer, &TimerThread::timeout, this, &MainWindow::computeUpdateFPS);

    connect(this, &MainWindow::iterationPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    connect(this, &MainWindow::iterationTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    connect(this, &MainWindow::updateTimeMeasured, controlWidget, &ControlWidget::updateUpdateMetricsLabels);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [&]() {
        renderManager->init(morphoWidget->context());
        // plotsWidget->init(morphoWidget->context());

        numIterations = 0;
        numUpdates = 0;

        iterationStart = std::chrono::steady_clock::now();
        updateStart = std::chrono::steady_clock::now();

        iterationTimer->start();
        updateTimer->start();
    });
    connect(morphoWidget, &MorphoWidget::supportedTexFormats, controlWidget, &ControlWidget::populateTexFormatComboBox);
    connect(morphoWidget, &MorphoWidget::scaleTransformChanged, plotsWidget, &PlotsWidget::transformSources);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(plotsWidget, &PlotsWidget::selectedPointChanged, morphoWidget, &MorphoWidget::setCursor);
    connect(plotsWidget, &PlotsWidget::drawCursor, morphoWidget, &MorphoWidget::setDrawingCursor);

    connect(controlWidget, &ControlWidget::iterateStateChanged, this, &MainWindow::setIterationState);
    connect(controlWidget, &ControlWidget::updateStateChanged, morphoWidget, &MorphoWidget::setUpdate);

    connect(renderManager, &RenderManager::texturesChanged, nodeManager, &NodeManager::onTexturesChanged);

    connect(nodeManager, &NodeManager::outputTextureChanged, renderManager, &RenderManager::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, morphoWidget, &MorphoWidget::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    // connect(nodeManager, &NodeManager::outputFBOChanged, plotsWidget, &PlotsWidget::setFBO);
    connect(nodeManager, &NodeManager::sortedOperationsChanged, renderManager, &RenderManager::setSortedOperations);
    connect(nodeManager, &NodeManager::parameterValueChanged, overlay, &Overlay::addMessage);
    connect(nodeManager, &NodeManager::midiSignalsCreated, &midiLinkManager, &MidiLinkManager::addMidiSignals);
    connect(nodeManager, &NodeManager::midiSignalsRemoved, &midiLinkManager, &MidiLinkManager::removeMidiSignals);

    connect(controlWidget, &ControlWidget::iterationFPSChanged, this, &MainWindow::setIterationTimerInterval);
    connect(controlWidget, &ControlWidget::updateFPSChanged, this, &MainWindow::setUpdateTimerInterval);
    connect(controlWidget, &ControlWidget::startRecording, this, &MainWindow::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, this, &MainWindow::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, this, &MainWindow::takeScreenshot);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &MainWindow::setSize);
    connect(controlWidget, &ControlWidget::showMidiWidget, this, &MainWindow::showMidiWidget);
    connect(controlWidget, &ControlWidget::overlayToggled, overlay, &Overlay::enable);
    connect(controlWidget, &ControlWidget::readConfig, configParser, &ConfigurationParser::read);
    connect(controlWidget, &ControlWidget::writeConfig, configParser, &ConfigurationParser::write);

    connect(configParser, &ConfigurationParser::newImageSizeRead, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(configParser, &ConfigurationParser::newImageSizeRead, this, &MainWindow::setSize);

    setWindowTitle("Fosforo");
    setWindowIcon(QIcon(QPixmap(":/icons/logo-small.png")));
    resize(renderManager->texWidth(), renderManager->texHeight());
}



MainWindow::~MainWindow()
{
    if (recorder) {
        delete recorder;
    }

    delete plotsWidget;
    delete controlWidget;
    delete configParser;
    delete nodeManager;
    delete renderManager;
    delete morphoWidget;
    delete overlay;
    delete factory;
    delete midiListWidget;
    delete videoInControl;
    delete iterationTimer;
    delete updateTimer;
}



void MainWindow::beat()
{
    if (recorder)
    {
        if (recorder->isRecording())
        {
            iterate();
            recorder->sendVideoFrame(renderManager->outputImage());
        }
    }
    else
    {
        iterate();
    }
}



void MainWindow::iterate()
{
    if (renderManager->active())
    {
        renderManager->iterate();
        plotsWidget->updatePlots();
        // update();
    }
}



void MainWindow::computeIterationFPS()
{
    iterationEnd = std::chrono::steady_clock::now();
    iterationTime = std::chrono::duration_cast<std::chrono::microseconds>(iterationEnd - iterationStart);

    numIterations++;

    if (iterationTime.count() >= 1'000'000)
    {
        double uspf = static_cast<double>(iterationTime.count()) / numIterations;
        double fps = numIterations * 1'000'000.0 / iterationTime.count();

        emit iterationTimeMeasured(uspf, fps);
        emit iterationPerformed();

        numIterations = 0;
        iterationStart = std::chrono::steady_clock::now();
    }
}



void MainWindow::computeUpdateFPS()
{
    updateEnd = std::chrono::steady_clock::now();
    updateTime = std::chrono::duration_cast<std::chrono::microseconds>(updateEnd - updateStart);

    numUpdates++;

    if (updateTime.count() >= 1'000'000)
    {
        double uspf = static_cast<double>(updateTime.count()) / numUpdates;
        double fps = numUpdates * 1'000'000.0 / updateTime.count();

        emit updateTimeMeasured(uspf, fps);

        numUpdates = 0;
        updateStart = std::chrono::steady_clock::now();
    }
}



void MainWindow::setIterationState(bool state)
{
    renderManager->setActive(state);
}



void MainWindow::setIterationTimerInterval(double newFPS)
{
    iterationFPS = newFPS;

    numIterations = 0;
    iterationStart = std::chrono::steady_clock::now();

    iterationTimer->setTimerInterval(newFPS);
}



void MainWindow::setUpdateTimerInterval(double newFPS)
{
    updateFPS = newFPS;

    numUpdates = 0;
    updateStart = std::chrono::steady_clock::now();

    updateTimer->setTimerInterval(newFPS);
}



void MainWindow::startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format)
{
    recorder = new Recorder(recordFilename, framesPerSecond, format);
    connect(recorder, &Recorder::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);
    recorder->startRecording();
}



void MainWindow::stopRecording()
{
    recorder->stopRecording();
    disconnect(recorder, &Recorder::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);
    delete recorder;
    recorder = nullptr;
}



int MainWindow::getFrameCount()
{
    return recorder ? recorder->frameNumber : 0;
}



void MainWindow::takeScreenshot(QString filename)
{
    QImage screenshot = morphoWidget->grabFramebuffer();
    screenshot.save(filename);
}



void MainWindow::showMidiWidget()
{
    midiListWidget->setVisible(!midiListWidget->isVisible());
}



void MainWindow::setSize(int width, int height)
{
    if (stackedLayout->currentWidget() == controlWidget)
        morphoWidget->resize(width, height);
    else
        controlWidget->resize(width, height);

    updateGeometry();
    stackedWidget->updateGeometry();
    QApplication::processEvents();

    resize(width, height);

    updateGeometry();
    stackedWidget->updateGeometry();
    QApplication::processEvents();

    renderManager->resize(width, height);
    plotsWidget->setSize(width, height);
}



void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (stackedLayout->currentWidget() == controlWidget)
        morphoWidget->resize(event->size());
    else
        controlWidget->resize(event->size());

    int width = event->size().width();
    int height = event->size().height();

    renderManager->resize(width, height);
    plotsWidget->setSize(width, height);
    controlWidget->updateWindowSizeLineEdits(width, height);
}



void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_Tab)
        {
            if (stackedLayout->currentWidget() == controlWidget)
            {
                stackedLayout->setCurrentWidget(morphoWidget);
                morphoWidget->update();
            }
            else {
                stackedLayout->setCurrentWidget(controlWidget);
            }
        }
        else if (event->key() == Qt::Key_PageUp)
        {
            opacity += 0.1;
            if (opacity > 1.0)
                opacity = 1.0;
            controlWidgetOpacityEffect->setOpacity(opacity);
            morphoWidget->update();
        }
        else if (event->key() == Qt::Key_PageDown)
        {
            opacity -= 0.1;
            if (opacity < 0.0)
                opacity = 0.0;
            controlWidgetOpacityEffect->setOpacity(opacity);
            morphoWidget->update();
        }
    }
    else if (event->key() == Qt::Key_Space)
    {
        renderManager->reset();
    }

    event->accept();
}



void MainWindow::closeEvent(QCloseEvent* event)
{
    iterationTimer->stop();
    updateTimer->stop();

    renderManager->setActive(false);

    midiListWidget->close();

    // graphWidget->close();
    graphWidget->clearScene();

    controlWidget->close();

    plotsWidget->close();

    QMainWindow::closeEvent(event);
}
