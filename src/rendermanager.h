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



#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H



#include "texformat.h"
#include "imageoperation.h"
#include "seed.h"
#include "factory.h"
#include "videoinputcontrol.h"

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QImage>



class RenderManager : public QObject, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    RenderManager(Factory* factory, VideoInputControl* videoInCtrl);
    ~RenderManager();

    void init(QOpenGLContext* shareContext);

    bool active() const;
    void setActive(bool set);

    void iterate();

    QImage outputImage();
    QList<float> rgbPixel(QPoint pos);

    TextureFormat texFormat();
    void setTextureFormat(TextureFormat format);

    GLuint texWidth();
    GLuint texHeight();

    void clearAllOpsTextures();

    void drawAllSeeds();

    void resetIterationNumer();
    int iterationNumber();

signals:
    void texturesChanged();

public slots:
    void resize(GLuint width, GLuint height);
    void setOutputTextureId(GLuint* pTexId);
    void initOperation(QUuid id, ImageOperation* operation);
    void initSeed(QUuid id, Seed* seed);
    void setSortedOperations(QList<ImageOperation*> sortedOperations);
    void reset();

    void genImageTexture(QByteArray devId);
    void delImageTexture(QByteArray devId);
    void setVideoTextures();
    void setImageTexture(GLuint texId, QImage *image);

private:
    Factory* mFactory;
    VideoInputControl* mVideoInputControl;

    QOpenGLContext* mContext = nullptr;
    QOffscreenSurface* mSurface = nullptr;

    GLuint mOutFbo = 0;
    GLuint mReadFbo = 0;
    GLuint mDrawFbo = 0;

    QList<ImageOperation*> mSortedOperations;

    GLuint mTexWidth = 2048;
    GLuint mTexHeight = 2048;

    GLuint mOldTexWidth = 2048;
    GLuint mOldTexHeight = 2048;

    TextureFormat mTexFormat = TextureFormat::RGBA8;

    QImage* mOutputImage = nullptr;
    QImage::Format mOutputImageFormat = QImage::Format_RGBA8888;

    GLint mMaxArrayTexLayers;
    const GLint mNumArrayTexLayers = 32;

    QOpenGLShaderProgram* mBlenderProgram;
    // QOpenGLShaderProgram* mIdentityProgram;

    GLuint mVao;
    GLuint mVboPos;
    GLuint mVboTex;

    GLuint* mOutputTexId = nullptr;
    GLuint mBlendArrayTexId = 0;

    bool mActive = false;
    unsigned int mIterationNumber = 0;

    GLuint mFrameTexId = 0;

    const int mPboCount = 3;
    QList<GLuint> mPbos;
    int mSubmitIndex = 0;
    int mReadIndex = 0;
    QList<GLsync> mFences;

    QMap<QByteArray, GLuint> mVideoTextures;

    void setPbos();
    void setOutputImage();

    void setBlenderProgram();
    // void setIdentityProgram();

    void verticesCoords(GLfloat& left, GLfloat& right, GLfloat& bottom, GLfloat& top);
    void setVao();
    void adjustOrtho();

    void genTexture(GLuint* texId, TextureFormat texFormat);
    void genOpTextures(ImageOperation* operation);
    void resizeTextures();

    void blitTextures(GLuint srcTexId, GLuint srcTexWidth, GLuint srcTexHeight, GLuint newTexId, GLuint dstTexWidth, GLuint dstTexHeight);

    void copyTexturesToBlendArrayTexture(QList<GLuint*> textures);

    void genArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);
    void recreateArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);

    void shiftCopyArrayTextures();

    void clearTexture(GLuint* texId);
    void clearArrayTexture(GLuint* arrayTexId, GLsizei arrayTexDepth);

    void copyTextures();
    void blend(ImageOperation* operation);
    void renderOperation(ImageOperation* operation);
    void render();
};



#endif // RENDERMANAGER_H
