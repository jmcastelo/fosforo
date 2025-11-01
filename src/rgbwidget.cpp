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



#include "rgbwidget.h"
#include <QGraphicsOpacityEffect>


RGBWidget::RGBWidget(int w, int h, QWidget* parent) :
    QOpenGLWidget(parent),
    texWidth { w },
    texHeight { h }
{
    /*QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4); // Anti-aliasing
    format.setAlphaBufferSize(8); // Enable alpha channel
    setFormat(format);*/

    resize(512, 512);
    setUpdatesEnabled(false);
}



RGBWidget::~RGBWidget()
{
    if (vao && vao->isCreated())
        vao->destroy();

    if (vbo && vbo->isCreated())
        vbo->destroy();

    if (vao3D && vao3D->isCreated())
        vao3D->destroy();

    if (vbo3D && vbo3D->isCreated())
        vbo3D->destroy();

    if (program) delete program;
    if (program3D) delete program3D;
    if (vao) delete vao;
    if (vbo) delete vbo;
    if (vao3D) delete vao3D;
    if (vbo3D) delete vbo3D;
}



void RGBWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    program = new QOpenGLShaderProgram();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/rgb.vert"))
        qDebug() << "Vertex shader error:\n" << program->log();
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/rgb.frag"))
        qDebug() << "Fragment shader error:\n" << program->log();
    if (!program->link())
        qDebug() << "Shader link error:\n" << program->log();

    program3D = new QOpenGLShaderProgram();
    if (!program3D->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/3d.vert"))
        qDebug() << "Vertex shader error:\n" << program3D->log();
    if (!program3D->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/3d.frag"))
        qDebug() << "Fragment shader error:\n" << program3D->log();
    if (!program3D->link())
        qDebug() << "Shader link error:\n" << program3D->log();

    vao = new QOpenGLVertexArrayObject();
    vao->create();

    vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo->create();
    vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    setupBuffer(texWidth, texHeight);

    vao3D = new QOpenGLVertexArrayObject();
    vao3D->create();

    vbo3D = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vbo3D->create();
    vbo3D->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    computeTransformMatrix();
}



void RGBWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw RGB points

    program->bind();

    if (textureID)
        glBindTexture(GL_TEXTURE_2D, *textureID);
    else
        glBindTexture(GL_TEXTURE_2D, 0);

    vao->bind();
    glDrawArrays(GL_POINTS, 0, numPoints);
    vao->release();

    glBindTexture(GL_TEXTURE_2D, 0);

    program->release();

    // Draw RGB lines

    program3D->bind();
    vao3D->bind();

    GLint first = 0;
    foreach (GLuint nv, numVertices)
    {
        glDrawArrays(GL_LINES, first, nv);
        first += nv;
    }

    vao3D->release();
    program3D->release();
}



void RGBWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    computeTransformMatrix();
}



void RGBWidget::setTextureID(GLuint* id)
{
    textureID = id;
}



void RGBWidget::setupBuffer(int w, int h)
{
    vao->bind();
    vbo->bind();

    QList<QVector2D> texCoords;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            texCoords.append(QVector2D(x / static_cast<float>(w), y / static_cast<float>(h)));
        }
    }

    vbo->allocate(texCoords.constData(), texCoords.size() * sizeof(QVector2D));

    int texLocation = program->attributeLocation("texCoord");
    program->enableAttributeArray(texLocation);
    program->setAttributeBuffer(texLocation, GL_FLOAT, 0, 2);

    vbo->release();
    vao->release();

    numPoints = w * h;
}



void RGBWidget::setLines(QList<GLfloat> vertices)
{
    makeCurrent();

    vao3D->bind();

    vbo3D->bind();
    vbo3D->allocate(vertices.data(), vertices.size() * sizeof(GLfloat));

    program3D->bind();

    int posLocation = program3D->attributeLocation("pos");
    program3D->enableAttributeArray(posLocation);
    program3D->setAttributeBuffer(posLocation, GL_FLOAT, 0, 3);

    program3D->release();
    vbo3D->release();
    vao3D->release();

    doneCurrent();
}



void RGBWidget::computeTransformMatrix()
{
    projection.setToIdentity();
    projection.perspective(60.0f, static_cast<float>(width()) / static_cast<float>(height()), 0.01f, 10.0f);
    
    QVector3D eye = rotation.map(QVector3D(0.0f, 0.0f, 2.0f));
    QVector3D up = rotation.map(QVector3D(0.0f, 1.0f, 0.0f));

    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, QVector3D(0.0f, 0.0f, 0.0f), up);
    
    QMatrix4x4 model;
    model.setToIdentity();
    model.scale(scale);
    model.translate(-0.5f, -0.5f, -0.5f);

    QMatrix4x4 transform = projection * view * model;

    makeCurrent();

    program->bind();
    program->setUniformValue("transform", transform);
    program->release();

    program3D->bind();
    program3D->setUniformValue("transform", transform);
    program3D->release();

    doneCurrent();

    update();
}



void RGBWidget::closeEvent(QCloseEvent* event)
{
    emit closing();
    event->accept();
}



void RGBWidget::wheelEvent(QWheelEvent* event)
{
    scale += event->angleDelta().y() / 6000.0f;
    if (scale < 0.0f) scale = 0.0f;
    computeTransformMatrix();
}



void RGBWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        prevPos = event->pos();
    }
}



void RGBWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (!prevPos.isNull() && prevPos != event->pos())
        {
            QVector3D from = getArcBallVector(prevPos);
            QVector3D to = getArcBallVector(event->pos());

            rotationQuaternion = QQuaternion::rotationTo(from, to);
            rotationQuaternion.normalize();

            rotation.rotate(rotationQuaternion);

            computeTransformMatrix();
        }

        prevPos = event->pos();
    }
}



QVector3D RGBWidget::getArcBallVector(const QPoint& screenPoint)
{
    QVector3D vector = QVector3D(1.0f - 2.0f * screenPoint.x() / width(), 2.0f * screenPoint.y() / height() - 1.0f, 0.0f);

    float lengthSquared = vector.lengthSquared();

    if (lengthSquared <= 1.0f)
        vector.setZ(sqrt(1.0f - lengthSquared));
    else
        vector.normalize();

    return vector;
}
