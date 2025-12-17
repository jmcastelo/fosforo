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



#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H



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



class OutputWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    OutputWindow(int texWidth, int texHeight, Overlay *overlay);
    virtual ~OutputWindow() override;

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

    void fullScreenToggled(bool checked);
    void screenshot();
    void record();
    void resetIterations();
    void startPauseIts();

    void closing();

    void renderDone();

public slots:
    void setOutputTextureId(GLuint* pTexId);
    void setOutputTextureSize(GLuint width, GLuint height);

    void setDrawingCursor(bool on) { mDrawingCursor = on; }
    void setCursor(QPoint point);

    void render(quintptr fence);
    void updateView();

    void toggleFullScreen(bool checked);
    void toggleAutoResize(bool checked);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    GLuint* pOutTexId = nullptr;

    int mTexWidth;
    int mTexHeight;

    bool mAutoResize = false;
    bool mFullScreen = false;
    int mOldWidth;
    int mOldHeight;

    GLfloat mVertLeft;
    GLfloat mVertRight;
    GLfloat mVertBottom;
    GLfloat mVertTop;

    QPointF mTranslation;
    GLfloat mScaleExp;
    GLfloat mLeft;
    GLfloat mRight;
    GLfloat mBottom;
    GLfloat mTop;

    QOpenGLShaderProgram* mOutProgram = nullptr;
    GLuint mOutVao;
    GLuint mOutVboPos;
    GLuint mOutVboTex;

    QPointF mPrevPos;

    QTransform mSelectedPointTransform;
    QPointF mCursor;
    bool mDrawingCursor = false;

    QOpenGLShaderProgram* mCursorProgram = nullptr;
    QOpenGLVertexArrayObject* mCursorVao = nullptr;
    QOpenGLBuffer* mCursorVbo = nullptr;

    Overlay* mOverlay = nullptr;

    QMutex mMutex;

    void verticesCoords(int width, int height, GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top);

    void setVao();

    void setOutTranslation(QPointF delta);
    void setOutScaling(float delta);
    void setOutOrthographic(int width, int height);

    void setOutTransform();
    void setCursorTransform();

    void setSelectedPoint(QPointF pos);
    void updateCursor();

    void getSupportedTexFormats();
};



#endif
