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



#include "operationwidget.h"
#include "graphwidget.h"
#include "midilistwidget.h"
#include "texformat.h"

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
#include <QMediaRecorder>



// ControlWidget: contains Fosforo's controls

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    ControlWidget(QString version, GraphWidget* graphWidget, MidiListWidget* midiWidget, QWidget *parent = nullptr);
    ~ControlWidget();

signals:
    void iterateStateChanged(bool state);

    void resetIterations();

    void seedDrawn();

    void showMidiWidget();
    void showPlotsWidget();

    void overlayToggled(bool show);
    void fullScreenToggled(bool checked);

    void imageSizeChanged(int width, int height);

    void texFormatChanged(TextureFormat format);

    void startRecording(QString recordFilename, int framesPerSecond, QMediaRecorder::Quality quality, QMediaFormat format, bool yuv420p);
    void stopRecording();
    void takeScreenshot(QString filename);

    void iterationFPSChanged(double newFPS);

    void configRead();
    void readConfig(QString filename);
    void writeConfig(QString filename);

    void nodesSelected(QList<QUuid> nodeIds);

    void closing();

public slots:
    void updateWindowSizeLineEdits(int width, int height);

    void populateTexFormatComboBox(QList<TextureFormat> formats);

    void populateSortedOperationsTable(QList<QPair<QUuid, QString>> sortedData);
    void selectOpsTableRows(QList<QUuid> selNodeIds);

    void updateIterationNumberLabel(int itNum);
    void updateIterationMetricsLabels(double mSpf, double fps);

    void setVideoCaptureElapsedTimeLabel(int frameNumber);
    //void setupMidi(QString portName, bool open);
    //void updateMidiLinks(QString portName, int key, int value);

    void screenshot();
    void toggleRecording();

    void toggleFullScreenAction(bool checked);

protected:
    //void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QString mVersion;

    //OperationNode* selectedOperationNode = nullptr;
    //SeedNode* selectedSeedNode = nullptr;

    QToolBar* systemToolBar;
    //QToolBar* nodesToolBar;

    QTabWidget* optionsWidget;
    QWidget* displayOptionsWidget;
    QWidget* recordingOptionsWidget;

    QWidget* sortedOperationWidget;
    QTableWidget* sortedOperationsTable;
    QList<QPair<QUuid, QString>> sortedOperationsData;

    QAction* iterateAction;
    QAction* recordAction;
    QAction* sortedOperationsAction;
    QAction* fullScreenAction;
    QAction* saveConfigAction;
    QAction* loadConfigAction;

    int framesPerSecond = 60;
    QString outputDir;
    QMediaRecorder::Quality quality = QMediaRecorder::NormalQuality;
    QMediaFormat format;
    QList<QMediaFormat::VideoCodec> supportedVideoCodecs;
    QList<QMediaFormat::FileFormat> supportedFileFormats;
    QComboBox* fileFormatsComboBox;
    QComboBox* videoCodecsComboBox;
    bool yuv420p = false;

    QStatusBar* statusBar;

    QLabel* iterationNumberLabel;
    QLabel* timePerIterationLabel;
    QLabel* iterationFPSLabel;

    QLineEdit* windowWidthLineEdit;
    QLineEdit* windowHeightLineEdit;

    QComboBox* texFormatComboBox;

    QLabel* videoCaptureElapsedTimeLabel;

    QMap<QUuid, OperationWidget*> operationsWidgets;

    void constructDisplayOptionsWidget();
    void constructRecordingOptionsWidget();
    void constructSortedOperationWidget();
    void constructOptionsWidget(MidiListWidget* midiOptionsWidget);
    void constructSystemToolBar();

    QString textureFormatToString(TextureFormat format);

    void populateFileFormatsComboBox(QMediaFormat::FileFormat fileFormat);
    void populateVideoCodecsComboBox(QMediaFormat::VideoCodec videoCodec);

private slots:
    void iterate(bool checked);
    void setOutputDir();
    void record(bool checked);
    void loadConfig();
    void saveConfig();
    void toggleOverlay(bool checked);
    void about();

    //void populateScrollLayout(QList<QPair<QUuid, QString>> data, QList<QUuid> unsortedData);
    void selectNodesToMark();

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
