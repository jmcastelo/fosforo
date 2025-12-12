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



#include "morphowidget.h"

#include <QSurfaceFormat>
#include <QPainter>
#include <QMutexLocker>



MorphoWidget::MorphoWidget(int w, int h, Overlay* overlay_)
    : overlay { overlay_ }
{
    image = QRect(0, 0, w, h);
    // frame = image;

    selectedPoint = QPointF(w / 2, h / 2);
    cursor = QPointF(0.0, 0.0);

    overlay->setViewportRect(w, h);

    setTitle("Fosforo: Output");
    setIcon(QIcon(QPixmap(":/icons/logo.png")));
}



MorphoWidget::~MorphoWidget()
{
    makeCurrent();

    glDeleteFramebuffers(1, &fbo);
    vao->destroy();
    vbo->destroy();

    doneCurrent();

    delete vao;
    delete vbo;
    delete program;
}



void MorphoWidget::wheelEvent(QWheelEvent* event)
{
    // Frame

    QRect frameBefore = frame;

    qreal factor = pow(2.0, -event->angleDelta().y() / 1200.0);

    frameTransform.scale(factor, factor);

    if (frameTransform.m11() > 1.0 || frameTransform.m22() > 1.0) {
        frameTransform.reset();
    }

    frame = frameTransform.mapRect(image);

    // This transform is used to zoom following the pointer

    QTransform superscale = QTransform().scale((frameBefore.width() - frame.width()) / (frameTransform.m11() * width()), (frameBefore.height() - frame.height()) / (frameTransform.m22() * height()));
    QPointF increment = superscale.map(event->position());

    frame = frameTransform.translate(increment.x(), increment.y()).mapRect(image);

    // Keep frame within image

    if (frame.y() < image.y()) {
        frame = frameTransform.translate(0.0, (image.y() - frame.y()) / frameTransform.m22()).mapRect(image);
    }
    if (frame.y() + frame.height() > image.y() + image.height()) {
        frame = frameTransform.translate(0.0, (image.y() + image.height() - frame.y() - frame.height()) / frameTransform.m22()).mapRect(image);
    }
    if (frame.x() < image.x()) {
        frame = frameTransform.translate((image.x() - frame.x()) / frameTransform.m11(), 0.0).mapRect(image);
    }
    if (frame.x() + frame.width() > image.x() + image.width()) {
        frame = frameTransform.translate((image.x() + image.width() - frame.x() - frame.width()) / frameTransform.m11(), 0.0).mapRect(image);
    }

    // Cursor

    QPointF point = selectedPointTransform.map(selectedPoint);
    cursor.setX(2.0 * ((point.x() - frame.left()) / frame.width() - 0.5));
    cursor.setY(2.0 * (0.5 - (point.y() - frame.top()) / frame.height()));
    updateCursor();

    event->accept();
}



void MorphoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ControlModifier)
        {
            // Frame

            QPointF delta = QTransform().scale(static_cast<qreal>(image.width()) / width(), static_cast<qreal>(image.height()) / height()).map(prevPos - event->position());

            frame = frameTransform.translate(delta.x(), delta.y()).mapRect(image);

            if (frame.top() < image.top() || frame.bottom() > image.bottom()) {
                frame = frameTransform.translate(0.0, -delta.y()).mapRect(image);
            }
            if (frame.left() < image.left() || frame.right() > image.right()) {
                frame = frameTransform.translate(-delta.x(), 0.0).mapRect(image);
            }

            prevPos = event->pos();

            // Cursor

            if (prevFrame != frame)
            {
                QPointF point = selectedPointTransform.translate(static_cast<qreal>(prevFrame.left() - frame.left()) / image.width(), static_cast<qreal>(prevFrame.top() - frame.top()) / image.height()).map(selectedPoint);
                cursor.setX(2.0 * ((point.x() - frame.left()) / frame.width() - 0.5));
                cursor.setY(2.0 * (0.5 - (point.y() - frame.top()) / frame.height()));
                updateCursor();

                prevFrame = frame;
            }
        }
        else if (drawingCursor) {
            setSelectedPoint(event->position());
        }
    }

    event->accept();
}



void MorphoWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ControlModifier)
        {
            prevPos = event->position();
            prevFrame = frame;
        }
        else if (drawingCursor)
        {
            setSelectedPoint(event->position());
        }
    }

    event->accept();
}



void MorphoWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_F)
        {
            if (windowState() == Qt::WindowFullScreen)
            {
                showNormal();
                emit fullScreenToggled(false);
            }
            else
            {
                showFullScreen();
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

    event->accept();
}



void MorphoWidget::closeEvent(QCloseEvent* event)
{
    emit closing();

    QOpenGLWindow::closeEvent(event);
}



void MorphoWidget::toggleFullScreen(bool checked)
{
    if (checked) {
        showFullScreen();
    }
    else {
        showNormal();
    }
}


void MorphoWidget::setSelectedPoint(QPointF pos)
{
    QPointF clickedPoint = QTransform().scale(1.0 / width(), 1.0 / height()).map(pos);

    if (clickedPoint.x() < 0.0) {
        clickedPoint.setX(0.0);
    }
    if (clickedPoint.x() > 1.0) {
        clickedPoint.setX(1.0);
    }
    if (clickedPoint.y() < 0.0) {
        clickedPoint.setY(0.0);
    }
    if (clickedPoint.y() > 1.0) {
        clickedPoint.setY(1.0);
    }

    selectedPoint = QTransform().translate(frame.left(), frame.top()).scale(frame.width(), frame.height()).map(clickedPoint);

    QPoint point = QPoint(floor(selectedPoint.x()), floor(selectedPoint.y()));

    // Check boundaries
    // Note: right() = left() + width() - 1, bottom() = top() + height() - 1

    if (point.x() < image.left()) {
        point.setX(image.left());
    }
    if (point.x() > image.right()) {
        point.setX(image.right());
    }
    if (point.y() < image.top()) {
        point.setY(image.top());
    }
    if (point.y() > image.bottom()) {
        point.setY(image.bottom());
    }

    emit selectedPointChanged(point);

    cursor.setX(2.0 * (clickedPoint.x() - 0.5));
    cursor.setY(2.0 * (0.5 - clickedPoint.y()));
    updateCursor();
}



void MorphoWidget::setCursor(QPoint selPoint)
{
    QPointF point = selectedPointTransform.map(selPoint);
    cursor.setX(2.0 * ((point.x() - frame.left()) / frame.width() - 0.5));
    cursor.setY(2.0 * (0.5 - (point.y() - frame.top()) / frame.height()));
    updateCursor();
}



void MorphoWidget::resetZoom(int newWidth, int newHeight)
{
    QRect oldImage = image;

    image = QRect(0, 0, newWidth, newHeight);

    qreal scaleX = static_cast<qreal>(newWidth) / oldImage.width();
    qreal scaleY = static_cast<qreal>(newHeight) / oldImage.height();

    frameTransform.setMatrix(frameTransform.m11(), 0.0, 0.0, 0.0, frameTransform.m22(), 0.0, scaleX * frameTransform.dx(), scaleY * frameTransform.dy(), 1.0);

    frame = frameTransform.mapRect(image);

    QTransform scaleTransform = QTransform().scale(scaleX, scaleY);

    emit scaleTransformChanged(scaleTransform);
}



void MorphoWidget::adjustFrame(int width, int height)
{
    qreal scaleX = 1.0;
    qreal scaleY = 1.0;

    if (width > height)
    {

    }
}



void MorphoWidget::setOutputTextureId(GLuint* pTexId)
{
    pOutTexId = pTexId;
}



void MorphoWidget::updateCursor()
{
    GLfloat x = static_cast<GLfloat>(cursor.x());
    GLfloat y = static_cast<GLfloat>(cursor.y());

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

    makeCurrent();

    vao->bind();

    vbo->bind();
    vbo->allocate(cursorVertices, sizeof(cursorVertices));

    program->bind();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    vao->release();
    vbo->release();
    program->release();

    doneCurrent();
}



void MorphoWidget::getSupportedTexFormats()
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



void MorphoWidget::initializeGL()
{
    initializeOpenGLFunctions();

    //qDebug () << (const char*)context()->functions()->glGetString(GL_VERSION);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    glDisable(GL_DEPTH_TEST);

    glGenFramebuffers(1, &fbo);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cursor.vert")) {
        qDebug() << "Vertex shader error:\n" << program->log();
    }
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cursor.frag")) {
        qDebug() << "Fragment shader error:\n" << program->log();
    }
    if (!program->link()) {
        qDebug() << "Shader link error:\n" << program->log();
    }

    vao = new QOpenGLVertexArrayObject();
    vao->create();

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    updateCursor();
    getSupportedTexFormats();

    emit openGLInitialized();
}



void MorphoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind fbo as read frame buffer

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    if (pOutTexId) {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *pOutTexId, 0);
    }
    else {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    }

    // Render to default frame buffer (screen) from fbo

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(frame.x(), frame.y() + frame.height(), frame.x() + frame.width(), frame.y(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    if (drawingCursor)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        program->bind();
        vao->bind();

        glDrawArrays(GL_LINES, 0, 8);

        vao->release();
        program->release();

        glDisable(GL_BLEND);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}



void MorphoWidget::paintOverGL()
{
    QPainter painter;

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (overlay->isEnabled()) {
        overlay->paint(&painter);
    }

    painter.end();
}



void MorphoWidget::resizeGL(int w, int h)
{
    resetZoom(w, h);
    overlay->setViewportRect(w, h);
    emit sizeChanged(w, h);
}



void MorphoWidget::render(quintptr pFence)
{
    QMutexLocker locker(&mutex);

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
