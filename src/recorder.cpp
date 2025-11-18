#include "recorder.h"

#include <QUrl>
#include <QVideoFrame>



Recorder::Recorder(QString filename, qreal framesPerSecond, QMediaFormat format) : fps { framesPerSecond }
{
    recorder.setOutputLocation(QUrl::fromLocalFile(filename));
    recorder.setQuality(QMediaRecorder::NormalQuality);
    recorder.setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
    recorder.setVideoFrameRate(framesPerSecond);
    recorder.setVideoResolution(QSize());
    recorder.setMediaFormat(format);

    session.setRecorder(&recorder);
    session.setVideoFrameInput(&videoInput);

    connect(&videoInput, &QVideoFrameInput::readyToSendVideoFrame, this, &Recorder::setVideoFrameInputReady);
}



void Recorder::startRecording()
{
    recorder.record();
}



void Recorder::stopRecording()
{
    recorder.stop();
}



void Recorder::setVideoFrameInputReady()
{
    videoFrameInputReady = true;
}



void Recorder::sendVideoFrame(QImage* image)
{
    QVideoFrame frame(image->convertToFormat(QImage::Format_RGB888));

    frame.setStreamFrameRate(fps);
    qint64 start = static_cast<qint64>(frameNumber * 1'000'000.0 / fps);
    qint64 end = static_cast<qint64>((frameNumber + 1) * 1'000'000.0 / fps);
    frame.setStartTime(start);
    frame.setEndTime(end);

    bool sent = videoInput.sendVideoFrame(frame);
    if (sent)
    {
        frameNumber++;
        emit frameRecorded(frameNumber);
    }
    else {
        videoFrameInputReady = false;
    }
}
