#include "applicationcontroller.h"



ApplicationController::ApplicationController()
{
    // iterationTimer = new TimerThread(iterationFPS, this);
    // updateTimer = new TimerThread(updateFPS, this);

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

    morphoWidget = new MorphoWidget(renderManager->texWidth(), renderManager->texHeight(), overlay);

    plotsWidget = new PlotsWidget(renderManager);
    plotsWidget->setVisible(false);

    graphWidget = new GraphWidget(factory, nodeManager);
    graphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(graphWidget, &GraphWidget::selectedNodesChanged, nodeManager, &NodeManager::setSelectedNodeIds);

    configParser = new ConfigurationParser(factory, nodeManager, renderManager, graphWidget, &midiLinkManager);

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

    controlWidget = new ControlWidget(graphWidget, renderManager, midiListWidget);
    controlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlWidget->setMinimumSize(0, 0);

    connect(graphWidget, &GraphWidget::selectedNodesChanged, controlWidget, &ControlWidget::selectOpsTableRows);

    // connect(iterationTimer, &TimerThread::timeout, this, &ApplicationController::beat);
    // connect(iterationTimer, &TimerThread::stepTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    // connect(iterationTimer, &TimerThread::stepPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);

    connect(renderManager, &RenderManager::frameReady, morphoWidget, &MorphoWidget::render);
    // connect(renderManager, &RenderManager::stepTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    // connect(renderManager, &RenderManager::stepPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    // connect(updateTimer, &TimerThread::timeout, morphoWidget, QOverload<>::of(&MorphoWidget::update));
    // connect(updateTimer, &TimerThread::stepTimeMeasured, controlWidget, &ControlWidget::updateUpdateMetricsLabels);

    // connect(this, &ApplicationController::iterationPerformed, controlWidget, &ControlWidget::updateIterationNumberLabel);
    // connect(this, &ApplicationController::iterationTimeMeasured, controlWidget, &ControlWidget::updateIterationMetricsLabels);
    // connect(this, &ApplicationController::updateTimeMeasured, controlWidget, &ControlWidget::updateUpdateMetricsLabels);

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [&]() {
        renderManager->init(morphoWidget->context());
        // plotsWidget->init(morphoWidget->context());

        // numIterations = 0;
        // numUpdates = 0;

        // iterationStart = std::chrono::steady_clock::now();
        // updateStart = std::chrono::steady_clock::now();

        // iterationTimer->start();
        // updateTimer->start();

        renderManager->start();
    });
    connect(morphoWidget, &MorphoWidget::renderDone, this, &ApplicationController::measureFps);
    connect(morphoWidget, &MorphoWidget::renderDone, plotsWidget, &PlotsWidget::updatePlots);
    connect(morphoWidget, &MorphoWidget::supportedTexFormats, controlWidget, &ControlWidget::populateTexFormatComboBox);
    connect(morphoWidget, &MorphoWidget::scaleTransformChanged, plotsWidget, &PlotsWidget::transformSources);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, plotsWidget, &PlotsWidget::setSelectedPoint);
    connect(plotsWidget, &PlotsWidget::selectedPointChanged, morphoWidget, &MorphoWidget::setCursor);
    connect(plotsWidget, &PlotsWidget::drawCursor, morphoWidget, &MorphoWidget::setDrawingCursor);
    connect(morphoWidget, &MorphoWidget::closing, this, &ApplicationController::closeAll);
    connect(morphoWidget, &MorphoWidget::sizeChanged, renderManager, &RenderManager::resize);
    connect(morphoWidget, &MorphoWidget::sizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::sizeChanged, plotsWidget, &PlotsWidget::setSize);

    connect(controlWidget, &ControlWidget::iterateStateChanged, this, &ApplicationController::setIterationState);
    connect(controlWidget, &ControlWidget::closing, this, &ApplicationController::closeAll);

    connect(renderManager, &RenderManager::texturesChanged, nodeManager, &NodeManager::onTexturesChanged);
    connect(renderManager, &RenderManager::frameRecorded, controlWidget, &ControlWidget::setVideoCaptureElapsedTimeLabel);

    connect(nodeManager, &NodeManager::outputTextureChanged, renderManager, &RenderManager::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, morphoWidget, &MorphoWidget::setOutputTextureId);
    connect(nodeManager, &NodeManager::outputTextureChanged, plotsWidget, &PlotsWidget::setTextureID);
    // connect(nodeManager, &NodeManager::outputFBOChanged, plotsWidget, &PlotsWidget::setFBO);
    connect(nodeManager, &NodeManager::sortedOperationsChanged, renderManager, &RenderManager::setSortedOperations);
    connect(nodeManager, &NodeManager::parameterValueChanged, overlay, &Overlay::addMessage);
    connect(nodeManager, &NodeManager::midiSignalsCreated, &midiLinkManager, &MidiLinkManager::addMidiSignals);
    connect(nodeManager, &NodeManager::midiSignalsRemoved, &midiLinkManager, &MidiLinkManager::removeMidiSignals);
    connect(nodeManager, &NodeManager::sortedOpsDataChanged, controlWidget, &ControlWidget::populateSortedOperationsTable);

    connect(controlWidget, &ControlWidget::iterationFPSChanged, this, &ApplicationController::setIterationTimerInterval);
    connect(controlWidget, &ControlWidget::startRecording, renderManager, &RenderManager::startRecording);
    connect(controlWidget, &ControlWidget::stopRecording, renderManager, &RenderManager::stopRecording);
    connect(controlWidget, &ControlWidget::takeScreenshot, renderManager, &RenderManager::takeScreenshot);
    connect(controlWidget, &ControlWidget::imageSizeChanged, this, &ApplicationController::setSize);
    connect(controlWidget, &ControlWidget::showMidiWidget, this, &ApplicationController::showMidiWidget);
    connect(controlWidget, &ControlWidget::showPlotsWidget, plotsWidget, &QWidget::show);
    connect(controlWidget, &ControlWidget::overlayToggled, overlay, &Overlay::enable);
    connect(controlWidget, &ControlWidget::readConfig, configParser, &ConfigurationParser::read);
    connect(controlWidget, &ControlWidget::writeConfig, configParser, &ConfigurationParser::write);
    connect(controlWidget, &ControlWidget::nodesSelected, graphWidget, &GraphWidget::markNodes);

    connect(configParser, &ConfigurationParser::newImageSizeRead, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(configParser, &ConfigurationParser::newImageSizeRead, this, &ApplicationController::setSize);

    // resize(renderManager->texWidth(), renderManager->texHeight());

    morphoWidget->resize(renderManager->texWidth(), renderManager->texHeight());
    morphoWidget->show();

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
    delete morphoWidget;
    delete overlay;
    delete videoInControl;
    // delete iterationTimer;
    // delete updateTimer;
}



/*void ApplicationController::beat()
{
    // timer.start();

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

    // morphoWidget->update();
    // morphoWidget->render();

    // qint64 cpuTimeNs = timer.nsecsElapsed();
    // qDebug() << "CPU time (ms):" << cpuTimeNs / 1'000'000.0;
}*/



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
        controlWidget->updateIterationNumberLabel();

        numSteps = 0;
        multiStepStart = std::chrono::steady_clock::now();
    }
}



/*void ApplicationController::computeIterationFPS()
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



void ApplicationController::computeUpdateFPS()
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
}*/



void ApplicationController::setIterationState(bool state)
{
    renderManager->setActive(state);

    if (state) {
        stepStart = std::chrono::steady_clock::now();
        multiStepStart = stepStart;
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
    morphoWidget->resize(width, height);
    renderManager->resize(width, height);
    plotsWidget->setSize(width, height);
}



void ApplicationController::closeAll()
{
    // iterationTimer->stop();
    // updateTimer->stop();

    renderManager->stop();

    if (morphoWidget->isVisible()) {
        morphoWidget->close();
    }
    if (controlWidget->isVisible()) {
        controlWidget->close();
    }
}


/*void ApplicationController::resizeEvent(QResizeEvent* event)
{
    QApplicationController::resizeEvent(event);

    if (stackedLayout->currentWidget() == controlWidget)
        morphoWidget->resize(event->size());
    else
        controlWidget->resize(event->size());

    int width = event->size().width();
    int height = event->size().height();

    renderManager->resize(width, height);
    plotsWidget->setSize(width, height);
    controlWidget->updateWindowSizeLineEdits(width, height);
}*/



/*void ApplicationController::keyPressEvent(QKeyEvent* event)
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
}*/



/*void ApplicationController::closeEvent(QCloseEvent* event)
{
    iterationTimer->stop();
    // updateTimer->stop();

    renderManager->setActive(false);

    midiListWidget->close();

    // graphWidget->close();
    graphWidget->clearScene();

    controlWidget->close();

    plotsWidget->close();

    QApplicationController::closeEvent(event);
}*/
