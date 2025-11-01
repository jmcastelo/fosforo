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



#ifndef SEED_H
#define SEED_H



#include <random>
#include <QOpenGLExtraFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QImage>
#include <QString>
#include <QDebug>



class Seed : protected QOpenGLExtraFunctions
{
public:
    Seed();
    Seed(int type, bool fixed, QString imageFilename);
    Seed(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* context, QOffscreenSurface* surface);
    Seed(GLenum texFormat, GLuint width, GLuint height, const Seed& seed);
    ~Seed();

    void init(GLenum texFormat, GLuint width, GLuint height, QOpenGLContext* context, QOffscreenSurface* surface);

    GLuint* pOutTextureId();
    QList<GLuint*> textureIds();

    void draw();
    void setClearTexture();
    void setOutTextureId();

    void setVao(GLuint width, GLuint height);

    void genTextures(GLenum texFormat, GLuint width, GLuint height);

    void loadImage(QString filename);

    int type() const;
    void setType(int type);

    bool fixed() const;
    void setFixed(bool set);

    QString imageFilename() const;
    void resizeImage();

    QByteArray videoDevId() const;
    void setVideoDevId(QByteArray devId);

    void setVideoTexture(GLuint texId);

private:
    int mType = 0;
    bool mFixed = false;
    bool mCleared = true;

    QString mImageFilename;
    QImage mImage;

    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;

    GLuint mOutFbo = 0;
    GLuint mVao = 0;
    GLuint mVboPos = 0;

    GLuint mRandomTexId = 0;
    GLuint mImageTexId = 0;
    GLuint mClearTexId = 0;
    GLuint* pOutTexId = nullptr;

    GLuint* pVideoTexId = nullptr;
    QByteArray mVideoDevId;

    GLuint mTexWidth;
    GLuint mTexHeight;

    QOpenGLShaderProgram* mRandomProgram;

    std::default_random_engine mGenerator;

    void clearTexture(GLuint texId);

    void setRandomProgram();

    void drawRandom(bool grayscale);
};



#endif // SEED_H
