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

#pragma once



#include <cmath>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QIcon>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include <QPoint>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QMouseEvent>



class RGBWidget : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    RGBWidget(int w, int h, QWidget* parent = nullptr);
    virtual ~RGBWidget() override;

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void setupBuffer(int w, int h);
    void setTextureID(GLuint *id);
    void setLines(QList<GLfloat> vertices);
    void setNumVertices(QList<GLuint> nv){ numVertices = nv; }

signals:
    void closing();

protected:
    void closeEvent(QCloseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject* vao = nullptr;
    QOpenGLBuffer* vbo = nullptr;

    QOpenGLShaderProgram* program3D = nullptr;
    QOpenGLVertexArrayObject* vao3D = nullptr;
    QOpenGLBuffer* vbo3D = nullptr;

    GLuint* textureID = nullptr;
    int texWidth, texHeight;
    int numPoints;

    QList<GLuint> numVertices;

    float scale = 1.0f;
    QMatrix4x4 rotation;
    QMatrix4x4 model;
    QMatrix4x4 projection;
    QQuaternion rotationQuaternion;
    QPoint prevPos;

    void computeTransformMatrix();
    QVector3D getArcBallVector(const QPoint& point);
};
