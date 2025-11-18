#ifndef RECORDER_H
#define RECORDER_H



#include <QObject>
#include <QVideoFrameInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QImage>



class Recorder : public QObject
{
    Q_OBJECT

public:
    unsigned int frameNumber = 0;
    qreal fps;
    QVideoFrameInput videoInput;

    Recorder(QString filename, qreal framesPerSecond, QMediaFormat format);

    void startRecording();
    void stopRecording();
    bool isRecording(){ return recorder.recorderState() == QMediaRecorder::RecordingState && videoFrameInputReady; }

public slots:
    void sendVideoFrame(QImage* image);

signals:
    void frameRecorded(int number);

private:
    QMediaCaptureSession session;
    QMediaRecorder recorder;
    bool videoFrameInputReady = true;

private slots:
    void setVideoFrameInputReady();
};

#endif // RECORDER_H
