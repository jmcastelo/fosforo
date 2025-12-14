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



#include "rendermanager.h"
#include "rgbwidget.h"
#include "colorpath.h"

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QTransform>



class PlotsWidget : public QWidget
{
    Q_OBJECT

public:
    PlotsWidget(RenderManager* renderManager, QWidget* parent = nullptr);
    ~PlotsWidget();

    // void init(QOpenGLContext* mainContext);
    bool isEnabled(){ return enabled; }

signals:
    void selectedPointChanged(QPoint point);
    void drawCursor(bool on);

public slots:
    // void setFBO(GLuint theFBO) { fbo = theFBO; };
    void setTextureID(GLuint* texId);
    void setSize(int width, int height);
    void setSelectedPoint(QPoint point);
    void transformSources(QTransform transform);
    void updatePlots();

private:
    RenderManager* mRenderManager;
    RGBWidget* rgbWidget;

    // QOpenGLContext* context;
    // QOffscreenSurface* surface;

    // GLuint fbo = 0;
    GLuint* textureID = nullptr;

    GLuint mTexWidth;
    GLuint mTexHeight;

    QPoint cursor;

    bool enabled = false;

    QList<ColorPath> colorPaths;
    QList<QPoint> sources;
    QList<GLfloat> allVertices;
    QList<GLuint> numVertices;

    int numIts = 100;

    QComboBox* selectPathComboBox;
    QLineEdit* xCoordLineEdit;
    QLineEdit* yCoordLineEdit;
    QIntValidator* xCoordValidator;
    QIntValidator* yCoordValidator;
    QLineEdit* numItsLineEdit;

    void checkPoint(QPoint &point);
    void setVertices();
    void setPixelRGB();

private slots:
    void setCoords();
    void addColorPath();
    void removeColorPath();
    void setControls(int index);
    void setNumIts();
};
