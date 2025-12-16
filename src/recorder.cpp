#include "recorder.h"

#include <QUrl>
#include <QVideoFrame>



Recorder::Recorder(QString filename, int framesPerSecond, QMediaRecorder::Quality quality, QMediaFormat format, bool yuv420p) :
    mFps { framesPerSecond },
    mYuv420p { yuv420p }
{
    mRecorder.setOutputLocation(QUrl::fromLocalFile(filename));
    mRecorder.setQuality(quality);
    mRecorder.setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
    mRecorder.setVideoFrameRate(mFps);
    mRecorder.setVideoResolution(QSize());
    mRecorder.setMediaFormat(format);

    mSession.setRecorder(&mRecorder);
    mSession.setVideoFrameInput(&mVideoInput);

    mFrameDelta = 1'000'000LL / mFps;

    connect(&mVideoInput, &QVideoFrameInput::readyToSendVideoFrame, this, &Recorder::setVideoFrameInputReady);
}



void Recorder::startRecording()
{
    mRecorder.record();
}



void Recorder::stopRecording()
{
    mRecorder.stop();
}



bool Recorder::isRecording()
{
    return mRecorder.recorderState() == QMediaRecorder::RecordingState && mVideoFrameInputReady;
}



void Recorder::setVideoFrameInputReady()
{
    mVideoFrameInputReady = true;
}



void Recorder::sendVideoFrame(QImage* image)
{
    QVideoFrame frame = copyImageToVideoFrame(image);

    frame.setStartTime(mFrameTime);
    frame.setEndTime(mFrameTime + mFrameDelta);
    frame.setStreamFrameRate(mFps);

    bool sent = mVideoInput.sendVideoFrame(frame);
    if (sent)
    {
        mFrameTime += mFrameDelta;
        mFrameNumber++;
        if (mFrameNumber % 60 == 0) {
            emit frameRecorded(mFrameNumber);
        }
    }
    else {
        mVideoFrameInputReady = false;
    }
}



QVideoFrame Recorder::copyImageToVideoFrame(QImage* image)
{
    if (!mYuv420p) {
        return QVideoFrame(image->convertToFormat(QImage::Format_RGB888));
    }
    else
    {
        QImage img = image->convertToFormat(QImage::Format_RGB32);

        int width = img.width();
        int height = img.height();

        // Allocate memory for YUV420P
        unsigned char* yuvData = new unsigned char[width * height * 3 / 2]; // Y + U + V

        // Convert RGB to YUV420P
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                QRgb rgb = img.pixel(x, y);
                int r = qRed(rgb);
                int g = qGreen(rgb);
                int b = qBlue(rgb);

                // Calculate Y, U, V
                int Y = (0.299 * r + 0.587 * g + 0.114 * b);
                int U = (-0.14713 * r - 0.28886 * g + 0.436 * b) + 128;
                int V = (0.615 * r - 0.51499 * g - 0.10001 * b) + 128;

                // Store Y
                yuvData[y * width + x] = Y;

                // Store U and V in subsampled format
                if (x % 2 == 0 && y % 2 == 0) {
                    yuvData[width * height + (y / 2) * (width / 2) + (x / 2)] = U;
                    yuvData[width * height + (width * height / 4) + (y / 2) * (width / 2) + (x / 2)] = V;
                }
            }
        }

        QVideoFrameFormat format(image->size(), QVideoFrameFormat::Format_YUV420P);

        QVideoFrame frame(format);
        frame.map(QVideoFrame::WriteOnly);
        std::memcpy(frame.bits(0), yuvData, width * height * 3 / 2);
        frame.unmap();

        delete[] yuvData;

        return frame;
    }
}
