#include "applicationcontroller.h"



ApplicationController::ApplicationController()
{
    updateViewTimer.setInterval(1000.0 / 60.0);
    updateViewTimer.setTimerType(Qt::TimerType::PreciseTimer);
    updateViewTimer.setSingleShot(false);

    videoInControl = new VideoInputControl();

    factory = new Factory(videoInControl);

    renderManager = new RenderManager(factory);

    connect(factory, &Factory::newOperationCreated, renderManager, &RenderManager::initOperation);
    connect(factory, &Factory::replaceOpCreated, renderManager, &RenderManager::initOperation);
    connect(factory, &Factory::newSeedCreated, renderManager, &RenderManager::initSeed);

    connect(videoInControl, &VideoInputControl::cameraUsed, renderManager, &RenderManager::genImageTexture);
    connect(videoInControl, &VideoInputControl::cameraUnused, renderManager, &RenderManager::delImageTexture);
    connect(videoInControl, &VideoInputControl::numUsedCamerasChanged, renderManager, &RenderManager::setVideoTextures);
    connect(videoInControl, &VideoInputControl::newFrameImage, renderManager, &RenderManager::setFrameImage);

    nodeManager = new NodeManager(factory);

    overlay = new Overlay();

    outputWindow = new OutputWindow(renderManager->texWidth(), renderManager->texHeight(), overlay);

    plotsWidget = new PlotsWidget(renderManager);
    plotsWidget->setVisible(false);

    graphWidget = new GraphWidget(factory, nodeManager);
    graphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(graphWidget, &GraphWidget::selectedNodesChanged, nodeManager, &NodeManager::setSelectedNodeIds);

    configParser = new ConfigurationParser(factory, nodeManager, renderManager, graphWidget, &midiLinkManager);

    midiControl = new MidiControl();
    midiListWidget = new MidiListWidget();

    connect(midiControl, &MidiControl::inputPortAdded, midiListWidget, &MidiListWidget::addPortEntry);
    connect(midiControl, &MidiControl::inputPortRemoved, midiListWidget, &MidiListWidget::removePortEntry);
    connect(midiControl, &MidiControl::midiEnabled, nodeManager, &NodeManager::midiEnabled);
    connect(midiControl, &MidiControl::midiEnabled, factory, &Factory::setMidiEnabled);
    connect(midiControl, &MidiControl::ccInputMessageReceived, &midiLinkManager, &MidiLinkManager::updateMidiLinks);
    connect(midiListWidget, &MidiListWidget::multiLinkButtonChecked, &midiLinkManager, &MidiLinkManager::setMultiLink);
    connect(midiListWidget, &MidiListWidget::clearLinksButtonClicked, &midiLinkManager, &MidiLinkManager::clearLinks);
    connect(&midiLinkManager, &MidiLinkManager::multiLinkSet, midiListWidget, &MidiListWidget::toggleMultiLinkButton);

    midiControl->setObserver();

    controlWidget = new ControlWidget(renderManager->version(), graphWidget, midiListWidget);

    controlWidget->updateWindowSizeLineEdits(renderManager->texWidth(), renderManager->texHeight());

    connect(graphWidget, &GraphWidget::selectedNodesChanged, controlWidget, &ControlWidget::selectOpsTableRows);

    connect(renderManager, &RenderManager::frameReady, outputWindow, &OutputWindow::render);

    connect(&updateViewTimer, &QTimer::timeout, outputWindow, &OutputWindow::updateView);

    connect(outputWindow, &OutputWindow::openGLInitialized, this, [&]() {
        renderManager->init(outputWindow->context());
        // plotsWidget->init(outputWindow->context());

        renderManager->start();
        updateViewTimer.start();
    });
    connect(outputWindow, &OutputWindow::renderDone, this, &ApplicationController::measureFps);
    connect(outputWindow, &OutputWindow::renderDone, plotsWidget, &PlotsWidget::updatePlots);
    connect(outputWindow, &OutputWindow::supportedTexFormats, controlWidget, &ControlWidget::populateTexFormatComboBox);
    connect(outputWindow, &OutputWindow::fullScreenToggled, controlWidget, &ControlWidget::toggleFullScreenAction);
    connect(outputWindow, &OutputWindow::screenshot, controlWidget, &ControlWidget::screenshot);
    connect(outputWindow, &OutputWindow::record, controlWidget, &ControlWidget::toggleRecording);
    connect(outputWindow, &OutputWindow::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(plotsWidget, &PlotsWidget::selectedPointChanged, outputWindow, &OutputWindow::setCursor);
    connect(plotsWidget, &PlotsWidget::drawCursor, outputWindow, &OutputWindow::setDrawingCursor);
    connect(outputWindow, &OutputWindow::closing, this, &ApplicationController::onOutputWindowClose);
    connect(outputWindow, &OutputWindow::resetIterations, renderManager, &RenderManager::reset);
    connect(outputWindow, &OutputWindow::startPauseIts, controlWidget, &ControlWidget::toggleIterationState);

    connect(controlWidget, &ControlWidget::iterateStateChanged, this, &ApplicationController::setIterationState);
    connect(controlWidget, &ControlWidget::closing, this, &ApplicationController::onControlWidgetClose);

    connect(renderManager, &RenderManager::texturesChanged, nodeManager, &NodeManager::onTexturesChanged);
    connect(renderManager, &RenderManager::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);

    connect(nodeManager, &NodeManager::outputTextureChanged, renderManager, &RenderManager::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, outputWindow, &OutputWindow::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    connect(nodeManager, &NodeManager::sortedOperationsChanged, renderManager, &RenderManager::setSortedOperations);
    connect(nodeManager, &NodeManager::operationEdited, renderManager, &RenderManager::adjustOperationOrtho);
    connect(nodeManager, &NodeManager::parameterValueChanged, overlay, &Overlay::addMessage);
    connect(nodeManager, &NodeManager::midiSignalsCreated, &midiLinkManager, &MidiLinkManager::addMidiSignals);
    connect(nodeManager, &NodeManager::midiSignalsRemoved, &midiLinkManager, &MidiLinkManager::removeMidiSignals);
    connect(nodeManager, &NodeManager::sortedOpsDataChanged, controlWidget, &ControlWidget::populateSortedOperationsTable);

    connect(controlWidget, &ControlWidget::iterationFPSChanged, this, &ApplicationController::setIterationTimerInterval);
    connect(controlWidget, &ControlWidget::resetIterations, renderManager, &RenderManager::reset);
    connect(controlWidget, &ControlWidget::startRecording, renderManager, &RenderManager::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, renderManager, &RenderManager::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, renderManager, &RenderManager::takeScreenshot);
    connect(controlWidget, &ControlWidget::texFormatChanged, renderManager, &RenderManager::setTextureFormat);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &ApplicationController::setSize);
    connect(controlWidget, &ControlWidget::showPlotsWidget, plotsWidget, &QWidget::show);
    connect(controlWidget, &ControlWidget::overlayToggled, overlay, &Overlay::enable);
    connect(controlWidget, &ControlWidget::configRead, renderManager, &RenderManager::resetIterationNumer);
    connect(controlWidget, &ControlWidget::readConfig, configParser, &ConfigurationParser::read);
    connect(controlWidget, &ControlWidget::writeConfig, configParser, &ConfigurationParser::write);
    connect(controlWidget, &ControlWidget::nodesSelected, graphWidget, &GraphWidget::markNodes);
    connect(controlWidget, &ControlWidget::fullScreenToggled, outputWindow, &OutputWindow::toggleFullScreen);
    connect(controlWidget, &ControlWidget::autoResizeWindow, outputWindow, &OutputWindow::toggleAutoResize);

    connect(configParser, &ConfigurationParser::newImageSizeRead, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(configParser, &ConfigurationParser::newImageSizeRead, this, &ApplicationController::setSize);

    outputWindow->resize(1024, 1024);
    outputWindow->show();

    controlWidget->resize(1024, 1024);
    controlWidget->show();
}



ApplicationController::~ApplicationController()
{
    delete plotsWidget;
    delete controlWidget;
    delete configParser;
    delete nodeManager;
    delete factory;
    delete renderManager;
    delete outputWindow;
    delete overlay;
    delete videoInControl;
    delete midiControl;
}



void ApplicationController::measureFps()
{
    stepEnd = std::chrono::steady_clock::now();

    stepTime = std::chrono::duration_cast<std::chrono::nanoseconds>(stepEnd - stepStart);
    stepStart = stepEnd;

    multiStepTime = std::chrono::duration_cast<std::chrono::milliseconds>(stepEnd - multiStepStart);

    numSteps++;

    renderManager->adjustTimerInterval(stepTime.count());

    if (multiStepTime.count() >= 1'000)
    {
        double mSpf = static_cast<double>(multiStepTime.count()) / numSteps;
        double fps = numSteps * 1'000.0 / multiStepTime.count();

        controlWidget->updateIterationMetricsLabels(mSpf, fps);
        controlWidget->updateIterationNumberLabel(renderManager->iterationNumber());

        numSteps = 0;
        multiStepStart = std::chrono::steady_clock::now();
    }
}



void ApplicationController::setIterationState(bool state)
{
    if (state) {
        updateViewTimer.stop();
    }

    renderManager->setActive(state);

    if (state) {
        stepStart = std::chrono::steady_clock::now();
        multiStepStart = stepStart;
    } else {
        updateViewTimer.start();
    }
}



void ApplicationController::setIterationTimerInterval(double fps)
{
    renderManager->setTargetFps(fps);
}



void ApplicationController::showMidiWidget()
{
    midiListWidget->setVisible(!midiListWidget->isVisible());
}



void ApplicationController::setSize(int width, int height)
{
    renderManager->resize(width, height);
    outputWindow->setOutputTextureSize(width, height);
    plotsWidget->setSize(width, height);
}



void ApplicationController::closeAll()
{
    disconnect(graphWidget, &GraphWidget::selectedNodesChanged, controlWidget, &ControlWidget::selectOpsTableRows);

    renderManager->stop();

    plotsWidget->close();
    graphWidget->close();
}



void ApplicationController::onOutputWindowClose()
{
    closeAll();
    disconnect(controlWidget, &ControlWidget::closing, this, &ApplicationController::onControlWidgetClose);
    controlWidget->close();
}



void ApplicationController::onControlWidgetClose()
{
    closeAll();
    disconnect(outputWindow, &OutputWindow::closing, this, &ApplicationController::onOutputWindowClose);
    outputWindow->close();
}
