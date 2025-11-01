/*
*  Copyright 2025 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/fosforo>
*
*  This file is part of Fosforo.
*
*  Fosforo is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Fosforo is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Fosforo.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H



#include "nodemanager.h"
#include "rendermanager.h"
#include "operationwidget.h"
#include "graphwidget.h"
#include "plotswidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QTabWidget>
#include <QStatusBar>
#include <QApplication>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QIntValidator>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QSlider>
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QMessageBox>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QScrollArea>
#include <QMediaFormat>



// ControlWidget: contains Fosforo's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    ControlWidget(double itFPS, double updFPS, GraphWidget* graphWidget, NodeManager* nodeManager, RenderManager* renderManager, PlotsWidget* plotsWidget, QWidget *parent = nullptr);
    ~ControlWidget();

signals:
    void iterateStateChanged(bool state);

    void seedDrawn();

    void updateStateChanged(bool state);

    void showMidiWidget();

    void overlayToggled(bool show);

    void imageSizeChanged(int width, int height);

    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();
    void takeScreenshot(QString filename);

    void iterationFPSChanged(double newFPS);
    void updateFPSChanged(double newFPS);

    void readConfig(QString filename);
    void writeConfig(QString filename);

public slots:
    void reset();

    void updateWindowSizeLineEdits(int width, int height);

    void populateTexFormatComboBox(QList<TextureFormat> formats);

    void updateIterationNumberLabel();
    void updateIterationMetricsLabels(double uspf, double fps);
    void updateUpdateMetricsLabels(double uspf, double fps);

    void setVideoCaptureElapsedTimeLabel(int frameNumber);
    //void setupMidi(QString portName, bool open);
    //void updateMidiLinks(QString portName, int key, int value);

protected:
    //void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    NodeManager* mNodeManager;
    RenderManager* mRenderManager;
    PlotsWidget* mPlotsWidget;

    //OperationNode* selectedOperationNode = nullptr;
    //SeedNode* selectedSeedNode = nullptr;

    QToolBar* systemToolBar;
    //QToolBar* nodesToolBar;

    QWidget* displayOptionsWidget;
    QWidget* recordingOptionsWidget;
    QWidget* sortedOperationWidget;

    QTableWidget* sortedOperationsTable;
    QList<QPair<QUuid, QString>> sortedOperationsData;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* screenshotAction;
    QAction* displayOptionsAction;
    QAction* recordingOptionsAction;
    QAction* sortedOperationsAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    qreal framesPerSecond = 60.0;
    QString outputDir = QDir::currentPath();
    QMediaFormat format;
    QList<QMediaFormat::VideoCodec> supportedVideoCodecs;
    QList<QMediaFormat::FileFormat> supportedFileFormats;
    QComboBox* fileFormatsComboBox;
    QComboBox* videoCodecsComboBox;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* iterationFPSLabel;
    QLabel* timePerUpdateLabel;
    QLabel* updateFPSLabel;

    QLineEdit* windowWidthLineEdit;
    QLineEdit* windowHeightLineEdit;

    QComboBox* texFormatComboBox;

    QLabel* videoCaptureElapsedTimeLabel;

    QMap<QUuid, OperationWidget*> operationsWidgets;

    QMap<QString, QMap<int, Number<float>*>> midiFloatLinks;
    QMap<QString, QMap<int, Number<int>*>> midiIntLinks;
    Number<float>* linkingFloat = nullptr;
    Number<int>* linkingInt = nullptr;
    bool anyMidiPortOpen = false;

    //QMap<QUuid, BlendFactorWidget*> blendFactorWidgets;

    bool overlayEnabled = false;

    //QScrollArea* scrollArea;
    //QWidget* scrollWidget;
    //QHBoxLayout* scrollLayout;

    void constructSystemToolBar();
    void constructDisplayOptionsWidget(double itsFPS, double updFPS);
    void constructRecordingOptionsWidget();
    void constructSortedOperationWidget();

    QString textureFormatToString(TextureFormat format);

    void populateFileFormatsComboBox();
    void populateVideoCodecsComboBox();

    //void updateScrollLayout(QWidget* widget);
    //void updateScrollArea();

private slots:
    void iterate();
    void record();
    void setScreenshotFilename();
    void setOutputDir();
    void toggleDisplayOptionsWidget();
    void toggleRecordingOptionsWidget();
    //void toggleSortedOperationWidget();
    void plotsActionTriggered();
    void loadConfig();
    void saveConfig();
    void toggleOverlay();
    void about();

    void populateSortedOperationsTable(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData);
    //void populateScrollLayout(QList<QPair<QUuid, QString>> data, QList<QUuid> unsortedData);
    //void selectNodesToMark();

    //void createParametersWidget(QUuid id);
    //void setUpBlendFactorWidget(BlendFactorWidget* widget);

    // void setUpMidiLinks(bool midiOn);

    // void midiLinkParametersWidget(QUuid id);
    // void midiUnlinkParametersWidget(QUuid id);

    //void midiLinkBlendFactorWidget(QUuid id);
    //void midiUnlinkBlendFactorWidget(QUuid id);

    //void overlayLinkBlendFactorWidget(QUuid id);
    //void overlayUnlinkBlendFactorWidget(QUuid id);

    //void showParametersWidget(QUuid id);
    //void removeParametersWidget(QUuid id);
    //void updateParametersWidget(QUuid id);
    //void updateParametersWidgetsBorder(QWidget* widget);
    //void removeAllParametersWidgetsBorder();
    //void removeOneParametersWidgetBorder(QWidget* widget);

    //void constructSingleNodeToolBar(Node* node);
    //void constructMultipleNodesToolBar();

    //void setNodeOperation(QAction* action);
    //void enableNodeOperation(bool checked);
    //void removeNodeOperation();
    //void setSeedNodeType();
    //void setSeedNodeFixed(bool checked);
    //void removeSeedNode();
};



#endif // CONTROLWIDGET_H
