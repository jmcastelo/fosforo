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



#include "seed.h"

#include <QFile>
#include <QPainter>



Seed::Seed()
{
    pOutTexId = new GLuint(0);
    pVideoTexId = new GLuint(0);

    mVideoDevId = QByteArray();
}



Seed::Seed(int type, bool fixed, QString imageFilename)
{
    pOutTexId = new GLuint(0);
    pVideoTexId = new GLuint(0);

    mVideoDevId = QByteArray();

    mType = type;
    mFixed = fixed;
    mImageFilename = imageFilename;
}



/*Seed::Seed(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* context, QOffscreenSurface *surface)
{
    // mContext = new QOpenGLContext();
    // mContext->setFormat(context->format());
    // mContext->setShareContext(context);
    // mContext->create();

    // mSurface = new QOffscreenSurface();
    // mSurface->setFormat(context->format());
    // mSurface->create();

    mContext = context;
    mSurface = surface;

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Vertex array object

    glGenVertexArrays(1, &mVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mVboPos);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    // Setup VAO

    setVao(width, height);

    // Generate textures with format and size given externally

    // pOutTexId = new GLuint(0);

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    mContext->doneCurrent();
}*/



/*Seed::Seed(GLenum texFormat, GLuint width, GLuint height, const Seed& seed) :
    mType { seed.mType },
    mFixed { seed.mFixed },
    mImageFilename { seed.mImageFilename }
{
    mContext = new QOpenGLContext();
    mContext->setFormat(seed.mContext->format());
    mContext->setShareContext(seed.mContext);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(seed.mContext->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    // Setup VAO

    setVao(width, height);

    // Generate textures with format and size given externally

    // pOutTexId = new GLuint(0);

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    mContext->doneCurrent();

    if (!mImageFilename.isEmpty())
        loadImage(mImageFilename);
}*/



Seed::~Seed()
{
    mContext->makeCurrent(mSurface);

    glDeleteFramebuffers(1, &mOutFbo);

    GLuint texIds[] = { mRandomTexId, mImageTexId, mClearTexId };
    glDeleteTextures(3, texIds);

    delete pOutTexId;
    delete pVideoTexId;

    delete mRandomProgram;

    mContext->doneCurrent();
}



void Seed::init(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* context, QOffscreenSurface* surface)
{
    mContext = context;
    mSurface = surface;

    mContext->makeCurrent(mSurface);

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Framebuffer object

    glGenFramebuffers(1, &mOutFbo);

    // Vertex array object

    glGenVertexArrays(1, &mVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mVboPos);

    // Initialize shader program

    mRandomProgram = new QOpenGLShaderProgram();
    setRandomProgram();

    // Setup VAO

    setVao(width, height);

    // Generate textures with format and size given externally

    // pOutTexId = new GLuint(0);

    genTextures(texFormat, width, height);

    // Clear texture

    clearTexture(mClearTexId);

    mContext->doneCurrent();

    loadImage(mImageFilename);
}



GLuint* Seed::pOutTextureId()
{
    return pOutTexId;
}



QList<GLuint*> Seed::textureIds()
{
    return QList<GLuint*> { &mRandomTexId, &mImageTexId, &mClearTexId };
}



void Seed::setVao(GLuint width, GLuint height)
{
    // Recompute vertices coordinates

    GLfloat w = static_cast<GLfloat>(width);
    GLfloat h = static_cast<GLfloat>(height);

    GLfloat ratio = w / h;

    GLfloat left, right, bottom, top;

    if (width > height)
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

    GLfloat vertCoords[] = {
        left, bottom,
        right, bottom,
        left, top,
        right, top
    };

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVboPos);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), vertCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    mTexWidth = width;
    mTexHeight = height;
}



void Seed::loadImage(QString filename)
{
    QFile imageFile(filename);

    if (imageFile.exists()) {
        mImage = QImage(filename).convertToFormat(QImage::Format_RGBA8888);

        mContext->makeCurrent(mSurface);
        resizeImage();
        mContext->doneCurrent();

        mImageFilename = filename;
    }
}



void Seed::resizeImage()
{
    if (!mImage.isNull())
    {
        qreal sx = static_cast<qreal>(mTexWidth)  / mImage.width();
        qreal sy = static_cast<qreal>(mTexHeight) / mImage.height();
        qreal scale = qMin(1.0, qMin(sx, sy));

        int displayWidth = qRound(mImage.width() * scale);
        int displayHeight = qRound(mImage.height() * scale);

        int offsetX = (mTexWidth  - displayWidth) / 2;
        int offsetY = (mTexHeight - displayHeight) / 2;

        QImage scaled = mImage.scaled(displayWidth, displayHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QImage buffer(mTexWidth, mTexHeight, QImage::Format_RGBA8888);
        buffer.fill(Qt::black);

        QPainter painter(&buffer);
        painter.drawImage(offsetX, offsetY, scaled);

        glBindTexture(GL_TEXTURE_2D, mImageTexId);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTexWidth, mTexHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.constBits());

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}



QByteArray Seed::videoDevId() const
{
    return mVideoDevId;
}



void Seed::setVideoDevId(QByteArray devId)
{
    mVideoDevId = devId;
}



void Seed::setVideoTexture(GLuint texId)
{
    *pVideoTexId = texId;
}



int Seed::type() const
{
    return mType;
}


void Seed::setType(int type)
{
    mType = type;

    draw();
}



bool Seed::fixed() const
{
    return mFixed;
}



void Seed::setFixed(bool set)
{
    mFixed = set;

    setOutTextureId();
}



QString Seed::imageFilename() const
{
    return mImageFilename;
}



void Seed::draw()
{
    if (mType == 0 || mType == 1) {
        drawRandom(mType == 1);
    }

    mCleared = false;

    setOutTextureId();
}



void Seed::setClearTexture()
{
    if (!mCleared && !mFixed)
    {
        *pOutTexId = mClearTexId;
        mCleared = true;
    }
}



void Seed::setOutTextureId()
{
    if (mFixed || !mCleared)
    {
        if (mType == 0 || mType == 1) {
            *pOutTexId = mRandomTexId;
        }
        else if (mType == 2) {
            *pOutTexId = mImageTexId;
        }
        else if (mType == 3) {
            *pOutTexId = *pVideoTexId;
        }
    }
    else {
        *pOutTexId = mClearTexId;
    }
}



void Seed::genTextures(GLenum texFormat, GLuint width, GLuint height)
{
    // Allocated on immutable storage (glTexStorage2D)

    foreach (GLuint* texId, textureIds())
    {
        glGenTextures(1, texId);
        glBindTexture(GL_TEXTURE_2D, *texId);
        glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, width, height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}



void Seed::clearTexture(GLuint texId)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void Seed::setRandomProgram()
{
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/random.vert"))
        qDebug() << "Vertex shader error:\n" << mRandomProgram->log();
    if (!mRandomProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/random.frag"))
        qDebug() << "Fragment shader error:\n" << mRandomProgram->log();
    if (!mRandomProgram->link())
        qDebug() << "Shader link error:\n" << mRandomProgram->log();

    // Set in attribute: vertex coordinates

    mRandomProgram->bind();

    mRandomProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    mRandomProgram->enableAttributeArray(0);

    mRandomProgram->release();
}


void Seed::drawRandom(bool grayscale)
{
    mContext->makeCurrent(mSurface);

    glBindFramebuffer(GL_FRAMEBUFFER, mOutFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRandomTexId, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    mRandomProgram->bind();

    std::uniform_real_distribution<float> distribution(0, 1);
    float randomNumber = distribution(mGenerator);

    mRandomProgram->setUniformValue("randomNumber", randomNumber);
    mRandomProgram->setUniformValue("grayscale", grayscale);

    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    mRandomProgram->release();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mContext->doneCurrent();
}
