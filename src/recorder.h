#ifndef RECORDER_H
#define RECORDER_H



#include <QObject>
#include <QVideoFrameInput>
#include <QAudioInput>
#include <QMediaCaptureSession>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QImage>



class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder(QString filename, int framesPerSecond, QMediaRecorder::Quality quality, QMediaFormat format, bool yuv420p);

    void startRecording();
    void stopRecording();
    bool isRecording();

public slots:
    void sendVideoFrame(QImage* image);

signals:
    void frameRecorded(int number);

private:
    unsigned int mFrameNumber = 0;
    qint64 mFrameTime = 0;
    qint64 mFrameDelta;
    qint64 mFps;
    bool mYuv420p;

    QVideoFrameInput mVideoInput;
    QMediaCaptureSession mSession;
    QMediaRecorder mRecorder;
    bool mVideoFrameInputReady = true;

private slots:
    void setVideoFrameInputReady();

private:
    QVideoFrame copyImageToVideoFrame(QImage* image);
};

#endif // RECORDER_H
