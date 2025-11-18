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



#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H



#include "texformat.h"
#include "imageoperation.h"
#include "seed.h"
#include "factory.h"
#include "recorder.h"

#include <QThread>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QImage>
#include <QChronoTimer>
#include <QMutex>
#include <QQueue>



class RenderManager : public QThread, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    RenderManager(Factory* factory, QObject* parent = nullptr);
    ~RenderManager();

    void run() override;
    void stop();
    void setTargetFps(double fps);
    void adjustTimerInterval(long stepTimeNs);

    void init(QOpenGLContext* context);

    bool active() const;
    void setActive(bool set);

    void sendOutputImage(bool set);
    QList<float> rgbPixel(QPoint pos);

    TextureFormat texFormat();
    void setTextureFormat(TextureFormat format);

    GLuint texWidth();
    GLuint texHeight();

    void clearAllOpsTextures();

    void drawAllSeeds();

    void resetIterationNumer();
    int iterationNumber();

    QString version();

signals:
    void texturesChanged();
    void frameRecorded(int number);
    void frameReady(quintptr fence);

public slots:
    void iterate();
    void resize(GLuint width, GLuint height);
    void setOutputTextureId(GLuint* pTexId);
    void initOperation(QUuid id, ImageOperation* operation);
    void initSeed(QUuid id, Seed* seed);
    void setSortedOperations(QList<ImageOperation*> sortedOperations);
    void reset();

    void genImageTexture(QByteArray devId);
    void delImageTexture(QByteArray devId);
    void setVideoTextures();
    void setFrameImage(QByteArray devId, QImage image);

    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();

    void takeScreenshot(QString filename);

private:
    QString mVersion = "1.0 alpha";

    Factory* mFactory;

    QChronoTimer mTimer;
    QMutex mutex;

    double mFrequency;
    std::chrono::nanoseconds mTimerNs;

    QOpenGLContext* mContext = nullptr;
    QOffscreenSurface* mSurface = nullptr;

    GLuint mOutFbo = 0;
    GLuint mReadFbo = 0;
    GLuint mDrawFbo = 0;

    QList<ImageOperation*> mSortedOperations;

    GLuint mTexWidth = 2048;
    GLuint mTexHeight = 2048;

    GLuint mOldTexWidth = 2048;
    GLuint mOldTexHeight = 2048;

    TextureFormat mTexFormat = TextureFormat::RGBA8;

    bool mSendOutputImage = false;
    QImage* mOutputImage = nullptr;
    QImage::Format mOutputImageFormat = QImage::Format_RGBA8888;

    GLint mMaxArrayTexLayers;
    const GLint mNumArrayTexLayers = 32;

    QOpenGLShaderProgram* mBlenderProgram;
    // QOpenGLShaderProgram* mIdentityProgram;

    GLuint mVao;
    GLuint mVboPos;
    GLuint mVboTex;

    GLuint* mOutputTexId = nullptr;
    GLuint mBlendArrayTexId = 0;

    bool mActive = false;
    unsigned int mIterationNumber = 0;

    GLuint mFrameTexId = 0;
    GLuint mPbo = 0;
    GLsync mFence = 0;
    bool mGrabOutputTexture = false;
    bool mTakeScreenshot = false;
    QString mScreenshotFilename;

    QMap<QByteArray, GLuint> mVideoTextures;
    QMap<QByteArray, QImage> mFrameImageMap;

    void setPbo();
    void setOutputImage();
    void readOutputTexture();
    void grabOutputImage();

    Recorder* recorder = nullptr;

    void setBlenderProgram();
    // void setIdentityProgram();

    void verticesCoords(GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top);
    void setVao();
    void adjustOrtho();

    void genTexture(GLuint* texId, TextureFormat texFormat);
    void genOpTextures(ImageOperation* operation);
    void resizeTextures();

    void blitTextures(GLuint srcTexId, GLuint srcTexWidth, GLuint srcTexHeight, GLuint newTexId, GLuint dstTexWidth, GLuint dstTexHeight);

    void copyTexturesToBlendArrayTexture(QList<GLuint*> textures);

    void genArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);
    void recreateArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);

    void shiftCopyArrayTextures();

    void clearTexture(GLuint* texId);
    void clearArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);

    void copyTextures();
    void blend(ImageOperation* operation);
    void renderOperation(ImageOperation* operation);
    void render();

    void setImageTextures();
};



#endif // RENDERMANAGER_H
