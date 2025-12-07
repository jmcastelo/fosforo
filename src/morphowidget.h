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



#ifndef MORPHOWIDGET_H
#define MORPHOWIDGET_H



#include "texformat.h"
#include "overlay.h"

#include <QOpenGLWindow>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QCloseEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QElapsedTimer>
#include <QMutex>



class MorphoWidget : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    MorphoWidget(int width_, int height_, Overlay *overlay_);
    virtual ~MorphoWidget() override;

    void initializeGL() override;
    void paintGL() override;
    void paintOverGL() override;
    void resizeGL(int width, int height) override;

    void setGrabOutputTexture(bool set);

signals:
    void supportedTexFormats(QList<TextureFormat> formats);
    void openGLInitialized();
    void sizeChanged(int width, int height);
    void selectedPointChanged(QPoint point);
    void scaleTransformChanged(QTransform transform);

    void fullScreenToggled(bool checked);

    void closing();

    void renderDone();

public slots:
    void setOutputTextureId(GLuint* pTexId);
    void resetZoom(int width, int height);
    void setDrawingCursor(bool on){ drawingCursor = on; }
    void setCursor(QPoint point);
    void render(quintptr fence);
    void toggleFullScreen(bool checked);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    GLuint* pOutTexId = nullptr;

    GLuint fbo = 0;

    QRect image;
    QRect frame;
    QTransform frameTransform;
    QPointF prevPos;

    QPointF selectedPoint;
    QTransform selectedPointTransform;
    QPointF cursor;
    bool drawingCursor = false;

    QRect prevFrame;

    QOpenGLShaderProgram* program = nullptr;
    QOpenGLVertexArrayObject* vao = nullptr;
    QOpenGLBuffer* vbo = nullptr;

    Overlay* overlay = nullptr;

    QMutex mutex;

    void setSelectedPoint(QPointF pos);
    void updateCursor();
    void getSupportedTexFormats();
};



#endif
