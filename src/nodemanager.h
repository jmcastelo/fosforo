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



#ifndef NODEMANAGER_H
#define NODEMANAGER_H



#include "imageoperationnode.h"
#include "factory.h"
#include "imageoperation.h"
#include "seed.h"
#include "inputdata.h"
#include "operationwidget.h"
#include "seedwidget.h"
#include "edgewidget.h"
#include "midisignals.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QOpenGLContext>
#include <QUuid>
#include <QMap>



class NodeManager : public QObject
{
    Q_OBJECT

public:
    // NodeManager(RenderManager* renderManager);
    NodeManager(Factory* factory);
    ~NodeManager();

    bool tryConnectOperations(QUuid srcId, QUuid dstId, float blendFactor);
    void connectOperations(QUuid srcId, QUuid dstId, float blendFactor);

    void connectOperations(QMap<QUuid, QMap<QUuid, InputData*>> conections);

    void connectCopiedOperationsA(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1);
    void disconnectOperations(QUuid srcId, QUuid dstId);

    void setOperationInputType(QUuid srcId, QUuid dstId, InputType type);

    EdgeWidget* addEdgeWidget(QUuid srcId, QUuid dstId, Number<float>* blendFactor);

    Number<float>* blendFactor(QUuid srcId, QUuid dstId);
    void setBlendFactor(QUuid srcId, QUuid dstId, float factor);

    ImageOperation* getOperation(QUuid id);
    // QPair<QUuid, OperationWidget*> addNewOperation();
    // QUuid addOperation(QString operationName);
    QUuid copyOperation(QUuid srcId);
    // void setOperation(QUuid id, QString operationName);

    // void enableOperation(QUuid id, bool enabled);
    // bool isOperationEnabled(QUuid id);

    // QPair<QUuid, SeedWidget*> addSeed();
    QUuid copySeed(QUuid srcId);

    int getSeedType(QUuid id);
    void setSeedType(QUuid id, int set);
    void resetInputSeedTexId(QUuid id);
    bool isSeedFixed(QUuid id);
    void setSeedFixed(QUuid id, bool fixed);

    bool isOutput(QUuid id) { return id == mOutputId; }

    QUuid outputId() { return mOutputId; }
    GLuint getOUtputFBO() { return outputFBO; }
    // GLuint outputTextureID() { return *mOutputTextureId; }

    void pasteOperations();

    void swapTwoOperations(QUuid id1, QUuid id2);

    void sortOperations();

    // void clearOperation(QUuid id);
    // void clearAllOperations();

    QMap<QUuid, ImageOperationNode*> operationNodesMap() const { return mOperationNodesMap; }
    QMap<QUuid, Seed*> seedsMap() { return mSeedsMap; }

    bool isNode(QUuid id) { return mOperationNodesMap.contains(id) || mSeedsMap.contains(id); }

    QString version = "1.0 alpha";

signals:
    void outputNodeChanged(QUuid id);
    void outputFBOChanged(GLuint fbo);
    void outputTextureChanged(GLuint* pTexId);
    // void sortedOperationsChanged(QList<QPair<QUuid, QString>> sortedData, QList<QUuid> unsortedData);
    void sortedOperationsChanged(QList<ImageOperation*> operations);
    void nodesConnected(QUuid srcId, QUuid dstId, InputType type, EdgeWidget* widget);
    void nodeRemoved(QUuid id);
    void nodesDisconnected(QUuid srcId, QUuid dstId);
    void nodeInserted(QUuid srcId, QUuid dstId, QUuid opId);

    void midiSignalsCreated(QUuid id, MidiSignals* midisSignals);
    void midiSignalsRemoved(QUuid id);

    void midiEnabled(bool enabled);

    void parameterValueChanged(QUuid id, QString operationName, QString parameterName, QString value);

public slots:
    void setOutput(QUuid id);
    void onTexturesChanged();

private:
    // RenderManager* mRenderManager;
    Factory* mFactory;

    QMap<QUuid, ImageOperationNode*> mOperationNodesMap;
    QMap<QUuid, ImageOperationNode*> copiedOperationNodes[2];

    QMap<QUuid, Seed*> mSeedsMap;
    QMap<QUuid, Seed*> copiedSeeds[2];

    QUuid connSrcId;

    GLuint outputFBO;
    QUuid mOutputId;
    GLuint* pOutputTextureId = nullptr;

private slots:
    void addOperationNode(QUuid id, ImageOperation* operation);
    void addSeedNode(QUuid id, Seed* seed);

    void connectOperation(QUuid id, ImageOperation* operation);
    void replaceNodeOperation(QUuid id, ImageOperation* operation);

    void connectOperationWidget(OperationWidget* widget);
    void connectSeedWidget(QUuid id, SeedWidget* widget);

    void removeOperationNode(QUuid id);
    void removeSeedNode(QUuid id);

    void removeAllNodes();

    void equalizeBlendFactors(QUuid id);
};



#endif // NODEMANAGER_H
