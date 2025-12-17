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



#include "outputwindow.h"

#include <QSurfaceFormat>
#include <QPainter>
#include <QMutexLocker>



OutputWindow::OutputWindow(int texWidth, int texHeight, Overlay* overlay) :
    mTexWidth { texWidth },
    mTexHeight { texHeight },
    mOverlay { overlay }
{
    mCursor = QPointF(0.0, 0.0);

    setTitle("Fosforo: Output");
    setIcon(QIcon(QPixmap(":/icons/logo.png")));
}



OutputWindow::~OutputWindow()
{
    makeCurrent();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vbos[2] = { mOutVboPos, mOutVboTex };
    glDeleteBuffers(2, vbos);

    glDeleteVertexArrays(1, &mOutVao);

    delete mOutProgram;

    doneCurrent();

    delete mCursorVao;
    delete mCursorVbo;
    delete mCursorProgram;
}



void OutputWindow::wheelEvent(QWheelEvent* event)
{
    float angleDelta = event->angleDelta().y();
    setOutScaling(angleDelta);

    makeCurrent();
    setOutTransform();
    setCursorTransform();
    doneCurrent();

    event->accept();
}



void OutputWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ControlModifier)
        {
            QPointF delta = event->position() - mPrevPos;
            setOutTranslation(delta);

            makeCurrent();
            setOutTransform();
            setCursorTransform();
            doneCurrent();

            mPrevPos = event->position();
        }
        else if (mDrawingCursor) {
            setSelectedPoint(event->position());
        }
    }

    event->accept();
}



void OutputWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ControlModifier) {
            mPrevPos = event->position();
        }
        else if (mDrawingCursor) {
            setSelectedPoint(event->position());
        }
    }

    event->accept();
}



void OutputWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_F)
        {
            if (windowState() == Qt::WindowFullScreen)
            {
                toggleFullScreen(false);
                emit fullScreenToggled(false);
            }
            else
            {
                toggleFullScreen(true);
                emit fullScreenToggled(true);
            }
        }
        else if (event->key() == Qt::Key_S) {
            emit screenshot();
        }
        else if (event->key() == Qt::Key_R) {
            emit record();
        }
        else if (event->key() == Qt::Key_Space) {
            emit resetIterations();
        }
    }
    else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) && event->key() == Qt::Key_Space) {
        emit startPauseIts();
    }

    event->accept();
}



void OutputWindow::closeEvent(QCloseEvent* event)
{
    emit closing();

    QOpenGLWindow::closeEvent(event);
}



void OutputWindow::toggleFullScreen(bool checked)
{
    mFullScreen = checked;

    if (checked) {
        showFullScreen();
    }
    else {
        showNormal();
    }
}



void OutputWindow::toggleAutoResize(bool checked)
{
    mAutoResize = checked;

    if (mFullScreen) {
        toggleFullScreen(false);
        emit fullScreenToggled(false);
    }

    if (checked)
    {
        mTranslation = QPointF(0.0, 0.0);
        mScaleExp = 0.0;

        mOldWidth = width();
        mOldHeight = height();

        resize(mTexWidth, mTexHeight);
    }
    else
    {
        resize(mOldWidth, mOldHeight);
    }
}



void OutputWindow::setSelectedPoint(QPointF pos)
{
    mCursor = QTransform().translate(-mTranslation.x(), -mTranslation.y()).scale((mRight - mLeft) * pow(2.0, -mScaleExp) / width(), (mTop - mBottom) * pow(2.0, -mScaleExp) / height()).translate(-0.5 * width(), -0.5 * height()).map(pos);

    if (mCursor.x() < mVertLeft) {
        mCursor.setX(mVertLeft);
    }
    if (mCursor.x() > mVertRight) {
        mCursor.setX(mVertRight);
    }
    if (mCursor.y() < mVertBottom) {
        mCursor.setY(mVertBottom);
    }
    if (mCursor.y() > mVertTop) {
        mCursor.setY(mVertTop);
    }

    makeCurrent();
    updateCursor();
    doneCurrent();

    QPointF texPoint = QTransform().scale(mTexWidth / (mVertRight - mVertLeft), mTexHeight / (mVertTop - mVertBottom)).translate(-mVertLeft, -mVertBottom).map(mCursor);

    QPoint selTexPoint = QPoint(qMin(qFloor(texPoint.x()), mTexWidth - 1), qMin(qFloor(texPoint.y()), mTexHeight - 1));

    emit selectedPointChanged(selTexPoint);
}



void OutputWindow::setCursor(QPoint selPoint)
{
    mCursor = QTransform().translate(mVertLeft, mVertBottom).scale((mVertRight - mVertLeft) / mTexWidth, (mVertTop - mVertBottom) / mTexHeight).translate(0.5, 0.5).map(QPointF(selPoint));

    makeCurrent();
    updateCursor();
    doneCurrent();
}



void OutputWindow::setOutputTextureId(GLuint* pTexId)
{
    pOutTexId = pTexId;
}



void OutputWindow::setOutputTextureSize(GLuint width, GLuint height)
{
    mTexWidth = width;
    mTexHeight = height;

    makeCurrent();
    setVao();
    doneCurrent();

    if (mAutoResize && !mFullScreen) {
        resize(mTexWidth, mTexHeight);
    }
}



void OutputWindow::verticesCoords(int width, int height, GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top)
{
    // Maintain aspect ratio

    GLfloat w = static_cast<GLfloat>(width);
    GLfloat h = static_cast<GLfloat>(height);

    GLfloat ratio = w / h;

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
}



void OutputWindow::setVao()
{
    // Recompute vertices coordinates

    verticesCoords(mTexWidth, mTexHeight, mVertLeft, mVertRight, mVertBottom, mVertTop);

    GLfloat vertCoords[] = {
        mVertLeft, mVertBottom,
        mVertRight, mVertBottom,
        mVertLeft, mVertTop,
        mVertRight, mVertTop
    };

    GLfloat texCoords[] = {
        0.0f, 0.0f,  // Bottom-left
        1.0f, 0.0f,  // Bottom-right
        0.0f, 1.0f,  // Top-left
        1.0f, 1.0f   // Top-right
    };

    glBindVertexArray(mOutVao);

    glBindBuffer(GL_ARRAY_BUFFER, mOutVboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), vertCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mOutVboTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}



void OutputWindow::setOutTranslation(QPointF delta)
{
    mTranslation += QTransform().scale((mRight - mLeft) * pow(2.0, -mScaleExp) / width(), (mTop - mBottom) * pow(2.0, -mScaleExp) / height()).map(delta);
}



void OutputWindow::setOutScaling(float delta)
{
    mScaleExp += 2.0e-4 * delta;
}



void OutputWindow::setOutOrthographic(int width, int height)
{
    verticesCoords(width, height, mLeft, mRight, mBottom, mTop);
}


void OutputWindow::setOutTransform()
{
    QMatrix4x4 outTransform;
    outTransform.setToIdentity();
    outTransform.ortho(mLeft, mRight, mBottom, mTop, -1.0, 1.0);
    outTransform.scale(pow(2.0, mScaleExp), -pow(2.0, mScaleExp));
    outTransform.translate(mTranslation.x(), mTranslation.y());

    mOutProgram->bind();
    int location = mOutProgram->uniformLocation("transform");
    mOutProgram->setUniformValue(location, outTransform);
    mOutProgram->release();
}



void OutputWindow::setCursorTransform()
{
    QMatrix4x4 cursorTransform;
    cursorTransform.setToIdentity();
    cursorTransform.ortho(mLeft, mRight, mBottom, mTop, -1.0, 1.0);
    cursorTransform.scale(pow(2.0, mScaleExp), -pow(2.0, mScaleExp));
    cursorTransform.translate(mTranslation.x(), mTranslation.y());

    mCursorProgram->bind();
    int location = mCursorProgram->uniformLocation("transform");
    mCursorProgram->setUniformValue(location, cursorTransform);
    mCursorProgram->release();
}



void OutputWindow::updateCursor()
{
    GLfloat x = static_cast<GLfloat>(mCursor.x());
    GLfloat y = static_cast<GLfloat>(mCursor.y());

    GLfloat cursorVertices[] = {
        x, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, y, 1.0f, 1.0f, 1.0f, 0.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        x, y, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, y, 1.0f, 1.0f, 1.0f, 0.0f
    };

    // makeCurrent();

    mCursorVao->bind();

    mCursorVbo->bind();
    mCursorVbo->allocate(cursorVertices, sizeof(cursorVertices));

    mCursorProgram->bind();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    mCursorVao->release();
    mCursorVbo->release();
    mCursorProgram->release();

    // doneCurrent();
}



void OutputWindow::getSupportedTexFormats()
{
    QList<TextureFormat> allFormats =
    {
        TextureFormat::RGBA2,
        TextureFormat::RGBA4,
        TextureFormat::RGBA8,
        //TextureFormat::RGBA8_SNORM,
        //TextureFormat::RGB10_A2,
        //TextureFormat::RGB10_A2UI,
        TextureFormat::RGBA12,
        //TextureFormat::SRGB8_ALPHA8,
        TextureFormat::RGBA16,
        TextureFormat::RGBA16F,
        TextureFormat::RGBA32F
        //TextureFormat::RGBA8I,
        //TextureFormat::RGBA8UI,
        //TextureFormat::RGBA16I,
        //TextureFormat::RGBA16UI,
        //TextureFormat::RGBA32I,
        //TextureFormat::RGBA32UI
    };
    QList<TextureFormat> supportedFormats;

    GLint supported = GL_FALSE;

    makeCurrent();

    foreach (TextureFormat format, allFormats)
    {
        glGetInternalformativ(GL_TEXTURE_2D, static_cast<GLenum>(format), GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
        if (supported == GL_TRUE) {
            supportedFormats.append(format);
        }
    }

    doneCurrent();

    emit supportedTexFormats(supportedFormats);
}



void OutputWindow::initializeGL()
{
    initializeOpenGLFunctions();

    //qDebug () << (const char*)context()->functions()->glGetString(GL_VERSION);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glDisable(GL_DEPTH_TEST);

    // Image

    mOutProgram = new QOpenGLShaderProgram();
    if (!mOutProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/screen.vert")) {
        qDebug() << "Vertex shader error:\n" << mOutProgram->log();
    }
    if (!mOutProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/screen.frag")) {
        qDebug() << "Fragment shader error:\n" << mOutProgram->log();
    }
    if (!mOutProgram->link()) {
        qDebug() << "Shader link error:\n" << mOutProgram->log();
    }

    // Vertex array object

    glGenVertexArrays(1, &mOutVao);

    // Vertex buffer object: vertices positions

    glGenBuffers(1, &mOutVboPos);

    // Vertex buffer object: texture coordinates

    glGenBuffers(1, &mOutVboTex);

    setVao();

    // Set output transformation

    mTranslation = QPointF(0.0, 0.0);
    mScaleExp = 0.0;

    setOutTransform();

    // Cursor

    mCursorProgram = new QOpenGLShaderProgram();
    if (!mCursorProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cursor.vert")) {
        qDebug() << "Vertex shader error:\n" << mCursorProgram->log();
    }
    if (!mCursorProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cursor.frag")) {
        qDebug() << "Fragment shader error:\n" << mCursorProgram->log();
    }
    if (!mCursorProgram->link()) {
        qDebug() << "Shader link error:\n" << mCursorProgram->log();
    }

    mCursorVao = new QOpenGLVertexArrayObject();
    mCursorVao->create();

    mCursorVbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mCursorVbo->create();
    mCursorVbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    updateCursor();
    setCursorTransform();

    getSupportedTexFormats();

    emit openGLInitialized();
}



void OutputWindow::paintGL()
{
    // Draw output texture

    GLuint outTexId = 0;
    if (pOutTexId) {
        outTexId = *pOutTexId;
    }

    glBindVertexArray(mOutVao);

    glClear(GL_COLOR_BUFFER_BIT);

    mOutProgram->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outTexId);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    mOutProgram->release();

    glBindVertexArray(0);

    // Draw cursor

    if (mDrawingCursor)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        mCursorProgram->bind();
        mCursorVao->bind();

        glDrawArrays(GL_LINES, 0, 8);

        mCursorVao->release();
        mCursorProgram->release();

        glDisable(GL_BLEND);
    }
}



void OutputWindow::paintOverGL()
{
    QPainter painter;

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (mOverlay->isEnabled()) {
        mOverlay->paint(&painter);
    }

    painter.end();
}



void OutputWindow::resizeGL(int width, int height)
{
    setOutOrthographic(width, height);

    makeCurrent();
    setOutTransform();
    setCursorTransform();
    doneCurrent();

    mOverlay->setViewportRect(width, height);
}



void OutputWindow::render(quintptr pFence)
{
    QMutexLocker locker(&mMutex);

    context()->makeCurrent(this);

    GLsync fence = reinterpret_cast<GLsync>(pFence);

    if (fence)
    {
        while (true)
        {
            GLenum syncRes = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000);
            /*switch (syncRes)
            {
            case GL_ALREADY_SIGNALED: qDebug() << "GL: ALREADY_SIGNALED"; break;
            case GL_CONDITION_SATISFIED: qDebug() << "GL: CONDITION_SATISFIED"; break;
            case GL_TIMEOUT_EXPIRED: qDebug() << "GL: TIMEOUT_EXPIRED"; break;
            case GL_WAIT_FAILED: qDebug() << "GL: WAIT_FAILED"; break;
            }*/
            if (syncRes == GL_CONDITION_SATISFIED || syncRes == GL_ALREADY_SIGNALED) break;
        }
    }

    paintGL();
    paintOverGL();

    context()->swapBuffers(this);

    context()->doneCurrent();

    emit renderDone();
}



void OutputWindow::updateView()
{
    context()->makeCurrent(this);

    paintGL();
    paintOverGL();

    context()->swapBuffers(this);

    context()->doneCurrent();
}
