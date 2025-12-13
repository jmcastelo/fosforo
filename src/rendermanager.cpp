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



#include "rendermanager.h"

#include <QApplication>
#include <QMessageBox>
#include <QPainter>



RenderManager::RenderManager(Factory *factory, QObject *parent)
    : QThread { parent },
    mFactory { factory }
{
    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setSingleShot(true);

    setOutputImage();

    connect(&mTimer, &QChronoTimer::timeout, this, &RenderManager::iterate);
    connect(&mTimer, &QChronoTimer::timeout, &mTimer, &QChronoTimer::start);
}



void RenderManager::run()
{
    exec();
}



void RenderManager::stop()
{
    mTimer.stop();
    quit();
    wait();
}



void RenderManager::setTargetFps(double fps)
{
    QMutexLocker locker(&mutex);

    mFrequency = fps;
    mTimerNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(fps >= 1.0 ? 1.0 / fps: 0));

    mTimer.setInterval(mTimerNs);
}



void RenderManager::adjustTimerInterval(long stepTimeNs)
{
    QMutexLocker locker(&mutex);

    double correction = (mTimerNs.count() - stepTimeNs) / 1'000'000'000.0;

    if (mFrequency > 0)
    {
        double correctedTime = 1.0 / mFrequency + correction;
        if (correctedTime < 0) {
            correctedTime = 0;
        }

        mTimerNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(correctedTime));
        mTimer.setInterval(mTimerNs);
    }
}



void RenderManager::init(QOpenGLContext* context)
{
    // Create context

    mContext = new QOpenGLContext();
    mContext->setFormat(context->format());
    mContext->setShareContext(context);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(context->format());
    mSurface->create();

    // mContext = context;

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, mTexWidth, mTexHeight);

    // Framebuffer objects

    glGenFramebuffers(1, &mOutFbo);
    glGenFramebuffers(1, &mReadFbo);
    glGenFramebuffers(1, &mDrawFbo);

    // Vertex array object

    glGenVertexArrays(1, &mVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mVboPos);

    // Vertex buffer object: texture coordinates

    glGenBuffers(1, &mVboTex);

    // Setup VAO

    setVao();

    // Get and clamp maximum number of array texture layers

    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS , &mMaxArrayTexLayers);

    if (mMaxArrayTexLayers > mNumArrayTexLayers) {
        mMaxArrayTexLayers = mNumArrayTexLayers;
    }

    genArrayTexture(&mBlendArrayTexId, mNumArrayTexLayers);

    // Shader program

    mBlenderProgram = new QOpenGLShaderProgram();
    setBlenderProgram();

    // mIdentityProgram = new QOpenGLShaderProgram();
    // setIdentityProgram();

    // Frame texture: for video recording

    genTexture(&mFrameTexId, TextureFormat::RGBA8);

    // Pixel buffer object: generate and set up

    glGenBuffers(1, &mPbo);

    setPbo();

    mContext->doneCurrent();
}



RenderManager::~RenderManager()
{
    if (recorder) {
        delete recorder;
    }

    mContext->makeCurrent(mSurface);

    glDeleteFramebuffers(1, &mOutFbo);
    glDeleteFramebuffers(1, &mReadFbo);
    glDeleteFramebuffers(1, &mDrawFbo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vbos[2] = { mVboPos, mVboTex };
    glDeleteBuffers(2, vbos);

    glDeleteVertexArrays(1, &mVao);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glDeleteTextures(1, &mBlendArrayTexId);

    delete mBlenderProgram;
    // delete mIdentityProgram;

    glDeleteBuffers(1, &mPbo);

    mContext->doneCurrent();

    delete mOutputImage;

    delete mContext;
    delete mSurface;
}



bool RenderManager::active() const
{
    return mActive;
}



void RenderManager::setActive(bool set)
{
    mActive = set;

    if (mActive) {
        mTimer.start();
    } else {
        mTimer.stop();
    }
}



void RenderManager::iterate()
{
    if (mActive)
    {
        mContext->makeCurrent(mSurface);

        setImageTextures();

        if (!mSortedOperations.isEmpty())
        {
            copyTextures();
            shiftCopyArrayTextures();
            render();
        }

        foreach (Seed* seed, mFactory->seeds()) {
            seed->setClearTexture();
        }

        if (mGrabOutputTexture || mTakeScreenshot) {
            readOutputTexture();
        }

        glDeleteSync(mFence);
        mFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        if (mGrabOutputTexture || mTakeScreenshot) {
            grabOutputImage();
        }

        mContext->doneCurrent();

        emit frameReady(reinterpret_cast<quintptr>(mFence));

        mIterationNumber++;
    }
}



void RenderManager::setPbo()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, GLsizeiptr(mOutputImage->sizeInBytes()), nullptr, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}



void RenderManager::setOutputImage()
{
    if (mOutputImage) {
        delete mOutputImage;
    }

    mOutputImage = new QImage(mTexWidth, mTexHeight, QImage::Format_RGBA8888);
    mOutputImage->fill(Qt::black);
}



/*QImage RenderManager::outputImage()
{
    QImage image(mTexWidth, mTexHeight, QImage::Format_RGBA8888);

    if (mOutputTexId)
    {
        mContext->makeCurrent(mSurface);

        fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        GLenum status;
        do {
            status = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
        } while (status == GL_TIMEOUT_EXPIRED);

        glDeleteSync(fence);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mOutFbo);

        glReadPixels(0, 0, mTexWidth, mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        mContext->doneCurrent();
    }
    else
    {
        image.fill(Qt::black);
    }

    return image;
}*/



void RenderManager::sendOutputImage(bool set)
{
    mSendOutputImage = set;
}



void RenderManager::readOutputTexture()
{
    if (mOutputTexId)
    {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        GLuint* pReadTexId = mOutputTexId;

        if (mTexFormat != TextureFormat::RGBA8)
        {
            blitTextures(*mOutputTexId, mTexWidth, mTexHeight, mFrameTexId, mTexWidth, mTexHeight);
            pReadTexId = &mFrameTexId;
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo);

        glGetTextureSubImage(*pReadTexId, 0, 0, 0, 0, mTexWidth, mTexHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, GLsizei(mOutputImage->sizeInBytes()), 0);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
}


void RenderManager::grabOutputImage()
{
    if (mOutputTexId)
    {
        // mContext->makeCurrent(mSurface);

        if (mFence)
        {
            while (true)
            {
                GLenum syncRes = glClientWaitSync(mFence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
                /*switch (syncRes)
                {
                case GL_ALREADY_SIGNALED: qDebug() << "RM: ALREADY_SIGNALED"; break;
                case GL_CONDITION_SATISFIED: qDebug() << "RM: CONDITION_SATISFIED"; break;
                case GL_TIMEOUT_EXPIRED: qDebug() << "RM: TIMEOUT_EXPIRED"; break;
                case GL_WAIT_FAILED: qDebug() << "RM: WAIT_FAILED"; break;
                }*/
                if (syncRes == GL_CONDITION_SATISFIED || syncRes == GL_ALREADY_SIGNALED) break;
            }
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo);

        void* mapped = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, GLsizeiptr(mOutputImage->sizeInBytes()), GL_MAP_READ_BIT);

        if (mapped)
        {
            std::memcpy(mOutputImage->bits(), mapped, mOutputImage->sizeInBytes());
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        else
        {
            mOutputImage->fill(Qt::black);
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        // mContext->doneCurrent();
    }
    else
    {
        mOutputImage->fill(Qt::black);
    }

    // emit outputImageReady(*mOutputImage);

    if (mTakeScreenshot) {
        mOutputImage->save(mScreenshotFilename);
        mTakeScreenshot = false;
    }

    if (mGrabOutputTexture && recorder) {
        recorder->sendVideoFrame(mOutputImage);
    }
}



void RenderManager::takeScreenshot(QString filename)
{
    mScreenshotFilename = filename;
    mTakeScreenshot = true;
}



void RenderManager::startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format)
{
    recorder = new Recorder(recordFilename, framesPerSecond, format);

    connect(recorder, &Recorder::frameRecorded, this, &RenderManager::frameRecorded);

    mGrabOutputTexture = true;

    recorder->startRecording();
}



void RenderManager::stopRecording()
{
    recorder->stopRecording();

    mGrabOutputTexture = false;

    disconnect(recorder, &Recorder::frameRecorded, this, &RenderManager::frameRecorded);

    delete recorder;
    recorder = nullptr;
}



QList<float> RenderManager::rgbPixel(QPoint pos)
{
    QList<float> rgb(3, 0.0f);

    if (mOutputTexId)
    {
        mContext->makeCurrent(mSurface);

        glGetTextureSubImage(*mOutputTexId, 0, pos.x(), pos.y(), 0, 1, 1, 1, GL_RGB, GL_FLOAT, rgb.size() * sizeof(float), rgb.data());

        mContext->doneCurrent();
    }

    return rgb;
}



TextureFormat RenderManager::texFormat()
{
    return mTexFormat;
}



void RenderManager::setTextureFormat(TextureFormat format)
{
    mTexFormat = format;

    QList<GLuint*> oldTexIds;

    foreach (Seed* seed, mFactory->seeds()) {
        oldTexIds.append(seed->textureIds());
    }

    foreach (ImageOperation* operation, mFactory->operations()) {
        oldTexIds.append(operation->textureIds());
    }

    mContext->makeCurrent(mSurface);

    foreach (GLuint* oldTexId, oldTexIds)
    {
        GLuint newTexId = 0;
        genTexture(&newTexId, mTexFormat);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *oldTexId, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexId, 0);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, mTexWidth, mTexHeight, 0, 0, mTexWidth, mTexHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glDeleteTextures(1, oldTexId);
        *oldTexId = newTexId;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    recreateArrayTexture(&mBlendArrayTexId, mNumArrayTexLayers);

    foreach (ImageOperation* operation, mFactory->operations()) {
        if (operation->sampler2DArrayAvail()) {
            recreateArrayTexture(operation->arrayTextureId(), operation->arrayTextureDepth());
        }
    }

    mContext->doneCurrent();

    foreach (Seed* seed, mFactory->seeds()) {
        seed->setOutTextureId();
    }

    foreach (ImageOperation* operation, mFactory->operations())
    {
        operation->setBlitInTextureId();
        operation->setOutTextureId();
    }

    emit texturesChanged();
}



GLuint RenderManager::texWidth()
{
    return mTexWidth;
}



GLuint RenderManager::texHeight()
{
    return mTexHeight;
}



void RenderManager::resetIterationNumer()
{
    mIterationNumber = 0;
}



int RenderManager::iterationNumber()
{
    return mIterationNumber;
}



QString RenderManager::version()
{
    return mVersion;
}


void RenderManager::resize(GLuint width, GLuint height)
{
    mOldTexWidth = mTexWidth;
    mOldTexHeight = mTexHeight;

    mTexWidth = width;
    mTexHeight = height;

    setOutputImage();

    if (mContext)
    {
        mContext->makeCurrent(mSurface);

        setVao();
        foreach (Seed* seed, mFactory->seeds()) {
            seed->setVao(width, height);
        }

        setPbo();

        glViewport(0, 0, mTexWidth, mTexHeight);

        resizeTextures();
        recreateArrayTexture(&mBlendArrayTexId, mNumArrayTexLayers);

        foreach (ImageOperation* operation, mFactory->operations()) {
            if (operation->sampler2DArrayAvail()) {
                recreateArrayTexture(operation->arrayTextureId(), operation->arrayTextureDepth());
            }
        }

        mContext->doneCurrent();

        adjustOrtho();
    }
}



void RenderManager::setOutputTextureId(GLuint* pTexId)
{
    mOutputTexId = pTexId;
}



void RenderManager::initOperation(QUuid id, ImageOperation* operation)
{
    Q_UNUSED(id)

    // Init and link shaders

    operation->init(mContext, mSurface);
    operation->linkShaders();

    // Adjust orthographic projection if any

    adjustOperationOrtho(operation);

    // Set parameters

    operation->setAllParameters();

    genOpTextures(operation);
}



void RenderManager::initSeed(QUuid id, Seed* seed)
{
    Q_UNUSED(id)

    seed->init(static_cast<GLenum>(mTexFormat), mTexWidth, mTexHeight, mContext, mSurface);
}


void RenderManager::adjustOperationOrtho(ImageOperation* operation)
{
    GLfloat left, right, bottom, top;
    verticesCoords(left, right, bottom, top);

    operation->adjustOrtho(left, right, bottom, top);
}


void RenderManager::setSortedOperations(QList<ImageOperation*> sortedOperations)
{
    mSortedOperations = sortedOperations;
}



void RenderManager::reset()
{
    clearAllOpsTextures();
    drawAllSeeds();
    resetIterationNumer();
}



void RenderManager::genImageTexture(QByteArray devId)
{
    if (!mVideoTextures.contains(devId))
    {
        GLuint newTexId = 0;
        mContext->makeCurrent(mSurface);
        genTexture(&newTexId, mTexFormat);
        mContext->doneCurrent();

        mVideoTextures.insert(devId, newTexId);
        mFrameImageMap.insert(devId, QImage());
    }
}



void RenderManager::delImageTexture(QByteArray devId)
{
    if (mVideoTextures.contains(devId))
    {
        mContext->makeCurrent(mSurface);
        glDeleteTextures(1, &mVideoTextures[devId]);
        mContext->doneCurrent();

        mVideoTextures.remove(devId);
        mFrameImageMap.remove(devId);
    }
}


void RenderManager::setVideoTextures()
{
    foreach (Seed* seed, mFactory->seeds())
    {
        QByteArray devId = seed->videoDevId();
        seed->setVideoTexture(mVideoTextures.value(devId, 0));
    }
}



void RenderManager::setFrameImage(QByteArray devId, QImage image)
{
    mFrameImageMap[devId] = image;
}



void RenderManager::setImageTextures()
{
    for (auto [devId, image] : mFrameImageMap.asKeyValueRange())
    {
        if (!image.isNull() && mVideoTextures.contains(devId))
        {
            qreal sx = static_cast<qreal>(mTexWidth)  / image.width();
            qreal sy = static_cast<qreal>(mTexHeight) / image.height();
            qreal scale = qMin(1.0, qMin(sx, sy));

            int displayWidth = qRound(image.width() * scale);
            int displayHeight = qRound(image.height() * scale);

            int offsetX = (mTexWidth  - displayWidth) / 2;
            int offsetY = (mTexHeight - displayHeight) / 2;

            QImage scaled = image.scaled(displayWidth, displayHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            QImage buffer(mTexWidth, mTexHeight, QImage::Format_RGBA8888);
            buffer.fill(Qt::black);

            QPainter painter(&buffer);
            painter.drawImage(offsetX, offsetY, scaled);

            // mContext->makeCurrent(mSurface);

            glBindTexture(GL_TEXTURE_2D, mVideoTextures[devId]);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTexWidth, mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.constBits());

            glBindTexture(GL_TEXTURE_2D, 0);

            // mContext->doneCurrent();
        }
    }
}



void RenderManager::setBlenderProgram()
{
    mBlenderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/blender.vert");
    mBlenderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/blender.frag");
    mBlenderProgram->link();

    // Set blender shader attributes

    if (mBlenderProgram->isLinked())
    {
        mBlenderProgram->bind();

        // Vertices coordinates attribute (location = 0)

        mBlenderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
        mBlenderProgram->enableAttributeArray(0);

        // Texture coordinates attribute (location = 1)

        mBlenderProgram->setAttributeBuffer(1, GL_FLOAT, 0, 2);
        mBlenderProgram->enableAttributeArray(1);

        // Input array texture (uniform sampler2DArray inArrayTex)
        // Set texture unit 0

        GLint locArrayTex = mBlenderProgram->uniformLocation("inArrayTex");
        if (locArrayTex >= 0) {
            glUniform1i(locArrayTex, 0);
        }

        mBlenderProgram->release();
    }
}



/*void RenderManager::setIdentityProgram()
{
    mIdentityProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/identity.vert");
    mIdentityProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/identity.frag");
    mIdentityProgram->link();

    // Set blender shader attributes

    if (mIdentityProgram->isLinked())
    {
        mIdentityProgram->bind();

        // Vertices coordinates attribute (lcoation = 0)

        mIdentityProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
        mIdentityProgram->enableAttributeArray(0);

        // Texture coordinates attribute (lcoation = 1)

        mIdentityProgram->setAttributeBuffer(1, GL_FLOAT, 0, 2);
        mIdentityProgram->enableAttributeArray(1);

        // Input texture unit (uniform sampler2D inTexture)

        GLint locSampler = mIdentityProgram->uniformLocation("inTexture");
        if (locSampler >= 0)
            glUniform1i(locSampler, 0);

        mIdentityProgram->release();
    }
}*/



void RenderManager::verticesCoords(GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top)
{
    // Maintain aspect ratio

    GLfloat w = static_cast<GLfloat>(mTexWidth);
    GLfloat h = static_cast<GLfloat>(mTexHeight);

    GLfloat ratio = w / h;

    if (mTexWidth > mTexHeight)
    {
        top = 1.0f;
        bottom = -top;
        right = top * ratio;
        left = -right;
    }
    else
    {
        right = 1.0f;
        left = -right;
        top = right / ratio;
        bottom = -top;
    }
}



void RenderManager::setVao()
{
    // Recompute vertices coordinates

    GLfloat left, right, bottom, top;
    verticesCoords(left, right, bottom, top);

    GLfloat vertCoords[] = {
        left, bottom,
        right, bottom,
        left, top,
        right, top
    };

    GLfloat texCoords[] = {
        0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  // Bottom-right
        0.0f, 1.0f,  // Top-left
        1.0f, 1.0f   // Top-right
    };

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), vertCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mVboTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}



void RenderManager::adjustOrtho()
{
    GLfloat left, right, bottom, top;
    verticesCoords(left, right, bottom, top);

    foreach (ImageOperation* operation, mFactory->operations()) {
        operation->adjustOrtho(left, right, bottom, top);
    }

    // Orthographic projection: blender

    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.ortho(left, right, bottom, top, -1.0, 1.0);

    mContext->makeCurrent(mSurface);
    mBlenderProgram->bind();

    int locOrtho = mBlenderProgram->uniformLocation("ortho");
    mBlenderProgram->setUniformValue(locOrtho, matrix);

    mBlenderProgram->release();
    mContext->doneCurrent();
}



void RenderManager::genTexture(GLuint* texId, TextureFormat texFormat)
{
    // Allocated on immutable storage (glTexStorage2D)
    // To be called within active OpenGL context

    glGenTextures(1, texId);

    glBindTexture(GL_TEXTURE_2D, *texId);

    glTexStorage2D(GL_TEXTURE_2D, 1, static_cast<GLenum>(texFormat), mTexWidth, mTexHeight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}



void RenderManager::genOpTextures(ImageOperation* operation)
{
    mContext->makeCurrent(mSurface);

    foreach (GLuint* texId, operation->textureIds())
    {
        genTexture(texId, mTexFormat);
        clearTexture(texId);
    }

    if (operation->sampler2DArrayAvail()) {
        genArrayTexture(operation->arrayTextureId(), operation->arrayTextureDepth());
    }

    mContext->doneCurrent();
}



void RenderManager::genArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth)
{
    glGenTextures(1, arrayTexId);

    glBindTexture(GL_TEXTURE_2D_ARRAY, *arrayTexId);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, static_cast<GLenum>(mTexFormat), mTexWidth, mTexHeight, arrayTexDepth);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}



void RenderManager::blitTextures(GLuint srcTexId, GLuint srcTexWidth, GLuint srcTexHeight, GLuint dstTexId, GLuint dstTexWidth, GLuint dstTexHeight)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexId, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexId, 0);

    glBlitFramebuffer(0, 0, srcTexWidth, srcTexHeight, 0, 0, dstTexWidth, dstTexHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}



void RenderManager::resizeTextures()
{
    // Store old texture Ids, generate new textures and blit old to new ones

    QList<GLuint*> oldTexIds;

    foreach (Seed* seed, mFactory->seeds()) {
        foreach (GLuint* texId, seed->textureIds()) {
            oldTexIds.append(texId);
        }
    }

    foreach (ImageOperation* operation, mFactory->operations()) {
        foreach (GLuint* texId, operation->textureIds()) {
            oldTexIds.append(texId);
        }
    }

    foreach (GLuint* oldTexId, oldTexIds)
    {
        GLuint newTexId = 0;
        genTexture(&newTexId, mTexFormat);

        blitTextures(*oldTexId, mOldTexWidth, mOldTexHeight, newTexId, mTexWidth, mTexHeight);

        glDeleteTextures(1, oldTexId);
        *oldTexId = newTexId;
    }

    // Resize frame texture

    GLuint newTexId = 0;
    genTexture(&newTexId, TextureFormat::RGBA8);
    blitTextures(mFrameTexId, mOldTexWidth, mOldTexHeight, newTexId, mTexWidth, mTexHeight);
    glDeleteTextures(1, &mFrameTexId);
    mFrameTexId = newTexId;

    // Resize video textures

    for (auto [devId, texId] : mVideoTextures.asKeyValueRange())
    {
        GLuint newTexId = 0;
        genTexture(&newTexId, mTexFormat);

        blitTextures(texId, mOldTexWidth, mOldTexHeight, newTexId, mTexWidth, mTexHeight);

        glDeleteTextures(1, &texId);
        mVideoTextures[devId] = newTexId;
    }

    setVideoTextures();

    // Set output texture Ids

    foreach (Seed* seed, mFactory->seeds())
    {
        seed->resizeImage();
        seed->setOutTextureId();
    }

    foreach (ImageOperation* operation, mFactory->operations())
    {
        operation->setBlitInTextureId();
        operation->setOutTextureId();
        // operation->resetInputData();
    }

    emit texturesChanged();
}



void RenderManager::recreateArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glDeleteTextures(1, arrayTexId);
    genArrayTexture(arrayTexId, arrayTexDepth);
}



void RenderManager::shiftCopyArrayTextures()
{
    foreach (ImageOperation* operation, mSortedOperations)
    {
        if (operation->sampler2DArrayAvail())
        {
            // Shift layers back by copying

            for (GLint z = operation->arrayTextureDepth() - 2; z >= 0; z--) {
                glCopyImageSubData(*operation->arrayTextureId(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, z, *operation->arrayTextureId(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, z + 1, mTexWidth, mTexHeight, 1);
            }

            // Copy input texture to first layer

            glCopyImageSubData(operation->inTextureId(), GL_TEXTURE_2D, 0, 0, 0, 0, *operation->arrayTextureId(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, mTexWidth, mTexHeight, 1);
        }
    }
}



void RenderManager::copyTexturesToBlendArrayTexture(QList<GLuint*> textures)
{
    int nTextures = textures.size();
    if (nTextures > mMaxArrayTexLayers) {
        nTextures = mMaxArrayTexLayers;
    }

    for (int i = 0; i < nTextures; i++) {
        glCopyImageSubData(*textures[i], GL_TEXTURE_2D, 0, 0, 0, 0, mBlendArrayTexId, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, mTexWidth, mTexHeight, 1);
    }
}



void RenderManager::clearTexture(GLuint* texId)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void RenderManager::clearArrayTexture(GLuint* texId, GLsizei arrayTexDepth)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);

    for (GLsizei layer = 0; layer < arrayTexDepth; layer++)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, *texId, 0, layer);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void RenderManager::clearAllOpsTextures()
{
    QList<GLuint*> texIds;

    foreach (ImageOperation* operation, mFactory->operations()) {
        texIds.append(operation->textureIds());
    }

    mContext->makeCurrent(mSurface);

    foreach (GLuint* texId, texIds) {
        clearTexture(texId);
    }

    foreach (ImageOperation* operation, mFactory->operations()) {
        if (operation->sampler2DArrayAvail()) {
            clearArrayTexture(operation->arrayTextureId(), operation->arrayTextureDepth());
        }
    }

    mContext->doneCurrent();
}



void RenderManager::drawAllSeeds()
{
    foreach (Seed* seed, mFactory->seeds()) {
        seed->draw();
    }
}



void RenderManager::copyTextures()
{
    // Expects active OpenGL context

    foreach (ImageOperation* operation, mSortedOperations) {
        if (operation->blitEnabled()) {
            glCopyImageSubData(operation->blitInTextureId(), GL_TEXTURE_2D, 0, 0, 0, 0, operation->blitOutTextureId(), GL_TEXTURE_2D, 0, 0, 0, 0, mTexWidth, mTexHeight, 1);
        }
    }
}



void RenderManager::blend(ImageOperation *operation)
{
    // Copy input textures to blend 2D array texture and wait for copy to finish before using it

    copyTexturesToBlendArrayTexture(operation->inputTextures());
    // glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    // Bind blend output texture, where render will occur

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->blendOutTextureId(), 0);

    glClear(GL_COLOR_BUFFER_BIT);

    mBlenderProgram->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mBlendArrayTexId);

    // Set blend factors

    QList<float> weights(mNumArrayTexLayers, 0.0f);
    int i = 0;
    for (auto factor : operation->inputBlendFactors())
    {
        weights[i++] = factor->value();

        if (i > mNumArrayTexLayers) {
            break;
        }
    }

    GLint locWeights = mBlenderProgram->uniformLocation("weights");
    glUniform1fv(locWeights, mNumArrayTexLayers, weights.constData());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Clean up

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    mBlenderProgram->release();
}



void RenderManager::renderOperation(ImageOperation* operation)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, operation->outTextureId(), 0);

    glClear(GL_COLOR_BUFFER_BIT);

    operation->program()->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, operation->inTextureId());

    glBindSampler(0, operation->samplerId());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindSampler(0, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    operation->program()->release();
}



void RenderManager::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);

    glBindVertexArray(mVao);

    foreach (ImageOperation* operation, mSortedOperations)
    {
        if (operation->blendEnabled()) {
            blend(operation);
        }

        operation->render();
    }

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
