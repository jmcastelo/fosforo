

#ifndef VIDEOINPUTCONTROL_H
#define VIDEOINPUTCONTROL_H



#include <QObject>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCamera>
#include <QMap>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QImage>



class VideoInput : public QObject
{
    Q_OBJECT
public:
    explicit VideoInput(QCameraDevice cameraDevice, QObject* parent = nullptr);
    ~VideoInput();

    void setCameraActive(bool active);

signals:
    void videoFrameChanged(const QVideoFrame& videoFrame);

private:
    QMediaCaptureSession mCaptureSession;
    QCamera* mCamera;
    QVideoSink* mVideoSink;
};



class VideoInputControl : public QObject
{
    Q_OBJECT
public:
    explicit VideoInputControl(QObject *parent = nullptr);
    ~VideoInputControl();

    QList<QString> cameraDescriptions();
    QByteArray cameraId(int index);

    void useCamera(int index);
    void unuseCamera(QByteArray camId);

    QImage* frameImage(QByteArray camId);

signals:
    void cameraUsed(QByteArray camId);
    void cameraUnused(QByteArray camId);
    void numUsedCamerasChanged();

    void newFrameImage(QByteArray devId, const QImage image);

private slots:
    void setVideoInputs();

private:
    QMediaDevices mMediaDevices;
    QMap<QByteArray, QString> mCameraDescMap;
    QMap<QByteArray, VideoInput*> mVideoInMap;
    QMap<QByteArray, int> mNumUsedCamerasMap;
    // QMap<QByteArray, QImage> mFrameImageMap;
};



#endif // VIDEOINPUTCONTROL_H
