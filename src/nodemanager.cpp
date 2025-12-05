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



#include "nodemanager.h"



NodeManager::NodeManager(Factory* factory) :
    mFactory { factory }
{
    connect(mFactory, &Factory::newOperationCreated, this, &NodeManager::addOperationNode);
    connect(mFactory, &Factory::newOperationCreated, this, &NodeManager::connectOperation);
    connect(mFactory, &Factory::replaceOpCreated, this, &NodeManager::connectOperation);
    connect(mFactory, &Factory::replaceOpCreated, this, &NodeManager::replaceNodeOperation);
    connect(mFactory, &Factory::newSeedCreated, this, &NodeManager::addSeedNode);
    connect(mFactory, QOverload<OperationWidget*>::of(&Factory::newOpWidgetCreated), this, &NodeManager::connectOperationWidget);
    connect(mFactory, &Factory::newSeedWidgetCreated, this, &NodeManager::connectSeedWidget);
    connect(mFactory, &Factory::cleared, this, &NodeManager::removeAllNodes);

    /*availableOperations = {
        BilateralFilter::name,
        Brightness::name,
        ColorMix::name,
        ColorQuantization::name,
        Contrast::name,
        Convolution::name,
        Dilation::name,
        EqualizeHistogram::name,
        Erosion::name,
        GammaCorrection::name,
        Geometry::name,
        HueShift::name,
        Identity::name,
        Logistic::name,
        Mask::name,
        Median::name,
        Memory::name,
        MorphologicalGradient::name,
        Pixelation::name,
        Power::name,
        Rotation::name,
        Saturation::name,
        Scale::name,
        Value::name
    };*/
}



NodeManager::~NodeManager()
{
    foreach (ImageOperationNode* node, mOperationNodesMap) {
        delete node;
    }
}



void NodeManager::sortOperations()
{
    //QList<ImageOperation*> tmpSortedOperations = sortedOperations;

    QList<ImageOperation*> sortedOperations;

    QList<QPair<QUuid, QString>> sortedOperationsData;
    //QList<QUuid> unsortedOperationsIds;

    QMap<QUuid, ImageOperationNode*> pendingNodes = mOperationNodesMap;

    // Set operations as non-computed (pending)

    foreach (ImageOperationNode* node, mOperationNodesMap) {
        node->setComputed(false);
    }

    // First operations are those whose inputs are all blit or seed, they do not depend on pending operations
    // Also operations with no inputs but with some outputs, sorting algorithm depends on operations having inputs
    // Discard isolated nodes, with no inputs nor outputs

    foreach (ImageOperationNode* node, mOperationNodesMap)
    {
        if (node->numInputs() == 0 && node->numOutputs() == 0)
        {
            pendingNodes.remove(node->id());
            //unsortedOperationsIds.append(node->id);
        }
        else if ((node->numInputs() == 0 && node->numOutputs() > 0) || (node->numInputs() > 0 && node->numNonNormalInputs() == node->numInputs()))
        {
            sortedOperations.append(node->operation());
            sortedOperationsData.append(QPair<QUuid, QString>(node->id(), node->operation()->name()));
            pendingNodes.remove(node->id());
            node->setComputed(true);
        }
    }

    // Sorting algorithm

    while (!pendingNodes.empty())
    {
        QList<ImageOperationNode*> computedNodes;

        foreach (ImageOperationNode* node, pendingNodes)
        {
            if (node->allInputsComputed())
            {
                sortedOperations.append(node->operation());
                sortedOperationsData.push_back(QPair<QUuid, QString>(node->id(), node->operation()->name()));
                computedNodes.append(node);
                node->setComputed(true);
            }
        }

        foreach (ImageOperationNode* node, computedNodes)
        {
            pendingNodes.remove(node->id());
        }

        if (computedNodes.empty())
            break;
    }

    //if (tmpSortedOperations != sortedOperations)
        //emit sortedOperationsChanged(sortedOperationsData, unsortedOperationsIds);

    // mRenderManager->setSortedOperations(sortedOperations);
    emit sortedOperationsChanged(sortedOperations);
    emit sortedOpsDataChanged(sortedOperationsData);
}



/*void NodeManager::clearOperation(QUuid id)
{
    //if (operationNodes.contains(id))
        //operationNodes.value(id)->operation->clear();
}*/



/*void NodeManager::clearAllOperations()
{
    foreach (ImageOperationNode* node, operationNodes)
        node->operation()->clear();
}*/



void NodeManager::setOutput(QUuid id)
{
    mOutputId = id;

    if (mOperationNodesMap.contains(id)) {
        pOutputTextureId = mOperationNodesMap.value(id)->pOutTextureId();
    }
    else if (mSeedsMap.contains(id)) {
        pOutputTextureId = mSeedsMap.value(id)->pOutTextureId();
    }
    else {
        pOutputTextureId = nullptr;
    }

    emit outputNodeChanged(id);
    emit outputTextureChanged(pOutputTextureId);
}



bool NodeManager::tryConnectOperations(QUuid srcId, QUuid dstId, float blendFactor)
{
    if (srcId != dstId)
    {
        if (mOperationNodesMap.contains(srcId) && mOperationNodesMap.contains(dstId) && !mOperationNodesMap.value(dstId)->isInput(srcId))
        {
            mOperationNodesMap.value(dstId)->addInput(mOperationNodesMap.value(srcId), new InputData(InputType::Normal, mOperationNodesMap.value(srcId)->pOutTextureId(), blendFactor));
            mOperationNodesMap.value(srcId)->addOutput(mOperationNodesMap.value(dstId));

            sortOperations();

            return true;
        }
        else if (mSeedsMap.contains(srcId) && mOperationNodesMap.contains(dstId) && !mOperationNodesMap.value(dstId)->isInput(srcId))
        {
            mOperationNodesMap.value(dstId)->addSeedInput(srcId, new InputData(InputType::Seed, mSeedsMap.value(srcId)->pOutTextureId(), blendFactor));

            sortOperations();

            return true;
        }
    }

    return false;
}



void NodeManager::connectOperations(QUuid srcId, QUuid dstId, float factor)
{
    bool connected = tryConnectOperations(srcId, dstId, factor);
    if (connected) {
        InputType type = mOperationNodesMap.value(dstId)->inputs().value(srcId)->type();
        emit nodesConnected(srcId, dstId, type, addEdgeWidget(srcId, dstId, blendFactor(srcId, dstId)));
    }
}



void NodeManager::connectOperations(QMap<QUuid, QMap<QUuid, InputData*>> connections)
{
    for (auto [dstId, iMap]: connections.asKeyValueRange())
    {
        for (auto [srcId, inData]: iMap.asKeyValueRange())
        {
            if (mOperationNodesMap.contains(srcId))
            {
                if (inData->type() == InputType::Normal)
                {
                    mOperationNodesMap.value(srcId)->enableBlit(false);
                    inData->setpTextureId(mOperationNodesMap.value(srcId)->pOutTextureId());
                }
                else if (inData->type() == InputType::Blit)
                {
                    mOperationNodesMap.value(srcId)->enableBlit(true);
                    inData->setpTextureId(mOperationNodesMap.value(srcId)->pOutTextureId());
                }

                mOperationNodesMap.value(dstId)->addInput(mOperationNodesMap.value(srcId), inData);
                mOperationNodesMap.value(srcId)->addOutput(mOperationNodesMap.value(dstId));

                emit nodesConnected(srcId, dstId, inData->type(), addEdgeWidget(srcId, dstId, inData->blendFactor()));
            }
            else if (mSeedsMap.contains(srcId))
            {
                inData->setpTextureId(mSeedsMap.value(srcId)->pOutTextureId());
                mOperationNodesMap.value(dstId)->addSeedInput(srcId, inData);
                emit nodesConnected(srcId, dstId, inData->type(), addEdgeWidget(srcId, dstId, inData->blendFactor()));
            }
        }
    }
}



/*void NodeManager::connectCopiedOperations(QUuid srcId0, QUuid srcId1, QUuid dstId1, InputData* inData)
{
    if (mOperationNodesMap.contains(srcId0))
    {
        mCopiedOperationNodes.value(dstId1)->addInput(mCopiedOperationNodes.value(srcId1), new InputData(inData->type(), mCopiedOperationNodes.value(srcId1)->pOutTextureId(), inData->blendFactor()->value()));
        mCopiedOperationNodes.value(srcId1)->addOutput(mCopiedOperationNodes.value(dstId1));
    }
    else if (mSeedsMap.contains(srcId0))
    {
        mCopiedOperationNodes.value(dstId1)->addSeedInput(srcId1, new InputData(inData->type(), mSeedsMap.value(srcId0)->pOutTextureId(), inData->blendFactor()->value()));
    }
}*/



/*void NodeManager::connectCopiedOperationsB(QUuid srcId0, QUuid dstId0, QUuid srcId1, QUuid dstId1)
{
    if (mCopiedOperationNodes[1].contains(srcId0))
    {
        float factor = mCopiedOperationNodes[1].value(dstId0)->blendFactor(srcId0)->value();

        mCopiedOperationNodes[0].value(dstId1)->addInput(mCopiedOperationNodes[0].value(srcId1), new InputData(InputType::Normal, mCopiedOperationNodes[0].value(srcId1)->pOutTextureId(), factor));
        mCopiedOperationNodes[0].value(srcId1)->addOutput(mCopiedOperationNodes[0].value(dstId1));
    }
    else if (mCopiedSeeds[1].contains(srcId0))
    {
        float factor = mCopiedOperationNodes[1].value(dstId0)->blendFactor(srcId0)->value();
        mCopiedOperationNodes[0].value(dstId1)->addSeedInput(srcId1, new InputData(InputType::Seed, mCopiedSeeds[0].value(srcId1)->pOutTextureId(), factor));
    }
}*/



void NodeManager::disconnectOperations(QUuid srcId, QUuid dstId)
{
    if (mOperationNodesMap.contains(srcId))
    {
        mOperationNodesMap.value(dstId)->removeInput(mOperationNodesMap.value(srcId));
        mOperationNodesMap.value(srcId)->removeOutput(mOperationNodesMap.value(dstId));
    }
    else if (mSeedsMap.contains(srcId))
    {
        mOperationNodesMap.value(dstId)->removeSeedInput(srcId);
    }

    sortOperations();
}



void NodeManager::connectPastedNodes(QMap<QUuid, QUuid> isomorphism)
{
    for (auto [dstId0, dstId1] : isomorphism.asKeyValueRange())
    {
        auto dstNode1 = mOperationNodesMap.value(dstId1);

        for (auto [srcId0, inputData] : mCopiedInputData.value(dstId0).asKeyValueRange())
        {
            if (mCopiedOperations.contains(srcId0))
            {
                QUuid srcId1 = isomorphism.value(srcId0);
                auto srcNode1 = mOperationNodesMap.value(srcId1);
                InputData* newInputData = new InputData(inputData->type(), srcNode1->pOutTextureId(), inputData->blendFactor()->value());

                dstNode1->addInput(srcNode1, newInputData);
                srcNode1->addOutput(dstNode1);
                emit nodesConnected(srcId1, dstId1, inputData->type(), addEdgeWidget(srcId1, dstId1, newInputData->blendFactor()));
            }
            else if (mCopiedSeeds.contains(srcId0))
            {
                QUuid srcId1 = isomorphism.value(srcId0);
                InputData* newInputData = new InputData(inputData->type(), mSeedsMap.value(srcId1)->pOutTextureId(), inputData->blendFactor()->value());
                dstNode1->addSeedInput(srcId1, newInputData);
                emit nodesConnected(srcId1, dstId1, inputData->type(), addEdgeWidget(srcId1, dstId1, newInputData->blendFactor()));
            }
        }
    }

    sortOperations();
}



void NodeManager::setOperationInputType(QUuid srcId, QUuid dstId, InputType type)
{
    if (mOperationNodesMap.contains(srcId))
    {
        mOperationNodesMap.value(dstId)->setInputType(srcId, type);
        sortOperations();
    }
    /*else if (mCopiedOperationNodes.contains(srcId))
    {
        mCopiedOperationNodes.value(dstId)->setInputType(srcId, type);
    }*/
}



/*void NodeManager::pasteOperations()
{
    foreach (auto copiedNode, mCopiedOperationNodes) {
        ImageOperationNode* node = new ImageOperationNode(*copiedNode);
        mOperationNodesMap.insert(node->id(), node);
    }

    foreach (auto copiedSeed, mCopiedSeeds) {
        Seed* seed = new Seed(*copiedSeed);
        QUuid id = QUuid::createUuid();
        mCopiedSeeds.insert(id, seed);
    }

    sortOperations();
}*/



void NodeManager::swapTwoOperations(QUuid id1, QUuid id2)
{
    //ImageOperation* operation1 = operationNodes.value(id1)->operation->clone();
    //operation1->setParameters();

    //ImageOperation* operation2 = operationNodes.value(id2)->operation->clone();
    //operation2->setParameters();

    ImageOperation* operation1 = new ImageOperation(*mOperationNodesMap.value(id1)->operation());
    ImageOperation* operation2 = new ImageOperation(*mOperationNodesMap.value(id2)->operation());

    mOperationNodesMap.value(id1)->setOperation(operation2);
    mOperationNodesMap.value(id2)->setOperation(operation1);

    sortOperations();

    if (isOutput(id1)) {
        setOutput(id1);
    }
    else if (isOutput(id2)) {
        setOutput(id2);
    }
}



Number<float>* NodeManager::blendFactor(QUuid srcId, QUuid dstId)
{
    return mOperationNodesMap.value(dstId)->blendFactor(srcId);
}



void NodeManager::setBlendFactor(QUuid srcId, QUuid dstId, float factor)
{
    mOperationNodesMap.value(dstId)->setBlendFactor(srcId, factor);
}



void NodeManager::equalizeBlendFactors(QUuid id)
{
    mOperationNodesMap.value(id)->equalizeBlendFactors();
}



ImageOperation* NodeManager::getOperation(QUuid id)
{
    return mOperationNodesMap.contains(id) ? mOperationNodesMap.value(id)->operation() : nullptr;
}



/*QUuid NodeManager::addOperation(QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    operationNodes.insert(id, node);

    return id;
}*/



void NodeManager::copyOperation(QUuid srcId)
{
    ImageOperation *op = new ImageOperation(*mOperationNodesMap.value(srcId)->operation());
    mCopiedOperations.insert(srcId, op);

    QMap<QUuid, InputData*> inputData;
    for (auto [id, inData] : mOperationNodesMap.value(srcId)->inputs().asKeyValueRange()) {
        inputData.insert(id, new InputData(inData->type(), inData->pTextureId(), inData->blendFactor()->value()));
    }
    mCopiedInputData.insert(srcId, inputData);
}



/*void NodeManager::setOperation(QUuid id, QString operationName)
{
    ImageOperation* operation = newOperation(operationName);

    operationNodes.value(id)->setOperation(operation);

    // If it's output node, set new output texture id

    if (isOutput(id))
        setOutput(id);

    sortOperations();
}*/



void NodeManager::removeOperationNode(QUuid id)
{
    if (mOperationNodesMap.contains(id))
    {
        // Delete operation

        mFactory->deleteOperation(mOperationNodesMap.value(id)->operation());

        // Delete node

        delete mOperationNodesMap.value(id);
        mOperationNodesMap.remove(id);

        if (id == mOutputId) {
            setOutput(QUuid());
        }

        sortOperations();
    }
}



/*void NodeManager::enableOperation(QUuid id, bool enabled)
{
    operationNodes.value(id)->operation->enable(enabled);
}



bool NodeManager::isOperationEnabled(QUuid id)
{
    return operationNodes.value(id)->operation->enabled();
}*/



EdgeWidget* NodeManager::addEdgeWidget(QUuid srcId, QUuid dstId, Number<float>* blendFactor)
{
    EdgeWidget* edgeWidget = new EdgeWidget(blendFactor, mOperationNodesMap.contains(srcId), mFactory);

    QUuid id = QUuid::createUuid();

    /*connect(edgeWidget, &EdgeWidget::blendFactorChanged, this, [=, this](float newFactor) {
        setBlendFactor(srcId, dstId, newFactor);
    });*/

    connect(edgeWidget, &EdgeWidget::edgeTypeChanged, this, [=, this](bool predge) {
        if (predge) {
            setOperationInputType(srcId, dstId, InputType::Blit);
        }
        else {
            setOperationInputType(srcId, dstId, InputType::Normal);
        }
    });

    connect(edgeWidget, &EdgeWidget::operationInsert, this, [=, this](int index) {
        QUuid opId;
        mFactory->addAvailableOperation(index, opId);

        connectOperations(srcId, opId, 1.0);
        connectOperations(opId, dstId, 1.0);

        emit nodeInserted(srcId, dstId, opId);
    });

    connect(edgeWidget, &EdgeWidget::remove, this, [=, this]() {
        emit midiSignalsRemoved(id);
        disconnectOperations(srcId, dstId);
        emit nodesDisconnected(srcId, dstId);
    });

    emit midiSignalsCreated(id, edgeWidget->midiSignals());

    connect(this, &NodeManager::midiEnabled, edgeWidget, &EdgeWidget::toggleMidiAction);

    return edgeWidget;
}



void NodeManager::addOperationNode(QUuid id, ImageOperation* operation)
{
    ImageOperationNode* node = new ImageOperationNode(id, operation);
    mOperationNodesMap.insert(id, node);
}



void NodeManager::connectOperation(QUuid id, ImageOperation* operation)
{
    foreach (auto parameter, operation->uniformParameters<float>())
    {
        connect(parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant value) {
            QString paramName = parameter->name();
            if (parameter->numItems() > 1) {
                paramName += "[" + QString::number(i) + "]";
            }
            if (parameter->editable()) {
                emit parameterValueChanged(id, operation->name(), paramName, QString::number(value.toFloat(), 'f', 6));
            }
        });
    }

    foreach (auto parameter, operation->uniformParameters<int>())
    {
        connect(parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant value) {
            QString paramName = parameter->name();
            if (parameter->numItems() > 1) {
                paramName += "[" + QString::number(i) + "]";
            }
            if (parameter->editable()) {
                emit parameterValueChanged(id, operation->name(), parameter->name(), QString::number(value.toInt()));
            }
        });
    }

    foreach (auto parameter, operation->uniformParameters<unsigned int>())
    {
        connect(parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant value) {
            QString paramName = parameter->name();
            if (parameter->numItems() > 1) {
                paramName += "[" + QString::number(i) + "]";
            }
            if (parameter->editable()) {
                emit parameterValueChanged(id, operation->name(), parameter->name(), QString::number(value.toUInt()));
            }
        });
    }

    foreach (auto parameter, operation->mat4UniformParameters())
    {
        connect(parameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant value) {
            QString numberName = parameter->numberNames().at(i);
            if (parameter->editable()) {
                emit parameterValueChanged(id, operation->name(), parameter->name() + " " + numberName, QString::number(value.toFloat(), 'f', 6));
            }
        });
    }
}



void NodeManager::replaceNodeOperation(QUuid id, ImageOperation* operation)
{
    mOperationNodesMap.value(id)->setOperation(operation);
    sortOperations();
}



void NodeManager::connectOperationWidget(OperationWidget* widget)
{
    connect(widget, &OperationWidget::remove, this, &NodeManager::nodeRemoved);
    connect(widget, &OperationWidget::remove, this, &NodeManager::midiSignalsRemoved);
    connect(widget, &OperationWidget::remove, this, &NodeManager::removeOperationNode);
    connect(widget, &OperationWidget::remove, this, &NodeManager::removeSelectedNodes);

    connect(widget, &OperationWidget::copy, this, &NodeManager::copySelectedNodes);

    connect(widget, &OperationWidget::outputChanged, this, &NodeManager::setOutput);
    connect(this, &NodeManager::outputNodeChanged, widget, &OperationWidget::toggleOutputAction);

    connect(widget, &OperationWidget::connectTo, this, [=, this]() {
        if (connSrcId.isNull())
        {
            // Set id of source node
            connSrcId = widget->id();
        }
        else
        {
            connectOperations(connSrcId, widget->id(), 1.0);
            connSrcId = QUuid();
        }
    });

    connect(widget, &OperationWidget::equalizeBlendFactors, this, &NodeManager::equalizeBlendFactors);

    emit midiSignalsCreated(widget->id(), widget->midiSignals());

    connect(this, &NodeManager::midiEnabled, widget, &OperationWidget::toggleMidiButton);
}



/*QPair<QUuid, OperationWidget*> NodeManager::addNewOperation()
{
    ImageOperation* operation = mRenderManager->createNewOperation();

    QUuid id = QUuid::createUuid();

    ImageOperationNode *node = new ImageOperationNode(id);
    node->operation = operation;
    operationNodes.insert(id, node);

    OperationWidget* opWidget = new OperationWidget(operation, false, true);

    connect(opWidget, &OperationWidget::outputChanged, this, [=, this](bool checked){
        if (checked)
        {
            setOutput(id);
            emit outputNodeChanged(opWidget);
        }
        else
        {
            setOutput(QUuid());
            emit outputNodeChanged(nullptr);
        }
    });

    connect(this, &NodeManager::outputNodeChanged, opWidget, &OperationWidget::toggleOutputAction);

    connect(opWidget, &OperationWidget::connectTo, this, [=, this](){
        if (connSrcId.isNull())
            connSrcId = id;
        else if (connectOperations(connSrcId, id, 1.0))
        {
            emit nodesConnected(connSrcId, id, addEdgeWidget(connSrcId, id, 1.0));
            connSrcId = QUuid();
        }
        else
            connSrcId = QUuid();
    });

    connect(opWidget, &OperationWidget::remove, this, [=, this](){
        emit nodeRemoved(id);
        removeOperation(id);
    });

    return QPair<QUuid, OperationWidget*>(id, opWidget);*/

/*
    if (operationName == BilateralFilter::name)
    {
        operation = new BilateralFilter(false, sharedContext, 3, 1.0f, 10.0f, 0.1f, 1.0f);
    }
    else if (operationName == Brightness::name)
    {
        operation = new Brightness(false, sharedContext, 0.0f, 1.0f);
    }
    else if (operationName == ColorMix::name)
    {
        std::vector<float> rgbMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        operation = new ColorMix(false, sharedContext, rgbMatrix, 1.0f);
    }
    else if (operationName == ColorQuantization::name)
    {
        operation = new ColorQuantization(false, sharedContext, 10, 10, 10, 1.0f);
    }
    else if (operationName == Contrast::name)
    {
        operation = new Contrast(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Convolution::name)
    {
        std::vector<float> kernel = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        operation = new Convolution(false, sharedContext, kernel, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Dilation::name)
    {
        operation = new Dilation(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == EqualizeHistogram::name)
    {
        operation = new EqualizeHistogram(false, sharedContext, 1, 64, 1.0f);
    }
    else if (operationName == Erosion::name)
    {
        operation = new Erosion(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == GammaCorrection::name)
    {
        operation = new GammaCorrection(false, sharedContext, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Geometry::name)
    {
        operation = new Geometry(false, sharedContext, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, GL_NEAREST);
    }
    else if (operationName == HueShift::name)
    {
        operation = new HueShift(false, sharedContext, 0.0f, 1.0f);
    }
    else if (operationName == Identity::name)
    {
        operation = new Identity(false, sharedContext);
    }
    else if (operationName == Logistic::name)
    {
        operation = new Logistic(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Mask::name)
    {
        operation = new Mask(false, sharedContext, 0.0f, 0.5f);
    }
    else if (operationName == Median::name)
    {
        operation = new Median(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Memory::name)
    {
        operation = new Memory(false, sharedContext, 1, 0.1f, 0.5f);
    }
    else if (operationName == MorphologicalGradient::name)
    {
        operation = new MorphologicalGradient(false, sharedContext, 1.0f, 1.0f, 1.0f);
    }
    else if (operationName == Pixelation::name)
    {
        operation = new Pixelation(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Power::name)
    {
        operation = new Power(false, sharedContext, 1.0f, 1.0f);
    }
    else if (operationName == Rotation::name)
    {
        operation = new Rotation(false, sharedContext, 0.0f, GL_NEAREST);
    }
    else if (operationName == Saturation::name)
    {
        operation = new Saturation(false, sharedContext, 0.5f, 1.0f);
    }
    else if (operationName == Scale::name)
    {
        operation = new Scale(false, sharedContext, 1.0f, GL_NEAREST);
    }
    else if (operationName == Value::name)
    {
        operation = new Value(false, sharedContext, 0.5f, 1.0f);
    }

    if (operation)
        operation->setParameters();

    return operation;
}*/



void NodeManager::addSeedNode(QUuid id, Seed* seed)
{
    mSeedsMap.insert(id, seed);
}



void NodeManager::connectSeedWidget(QUuid id, SeedWidget* widget)
{
    Q_UNUSED(id)

    connect(widget, &SeedWidget::remove, this, &NodeManager::nodeRemoved);
    connect(widget, &SeedWidget::remove, this, &NodeManager::removeSeedNode);
    connect(widget, &SeedWidget::remove, this, &NodeManager::removeSelectedNodes);

    connect(widget, &SeedWidget::copy, this, &NodeManager::copySelectedNodes);

    connect(widget, &SeedWidget::outputChanged, this, &NodeManager::setOutput);

    connect(this, &NodeManager::outputNodeChanged, widget, &SeedWidget::toggleOutputAction);

    connect(widget, &SeedWidget::connectTo, this, [=, this]() {
        if (connSrcId.isNull()) {
            connSrcId = widget->id();
        }
        else
        {
            connectOperations(connSrcId, widget->id(), 1.0);
            connSrcId = QUuid();
        }
    });

    connect(widget, &SeedWidget::typeChanged, this, [=, this](){
        if (isOutput(widget->id()))
        {
            pOutputTextureId = mSeedsMap.value(widget->id())->pOutTextureId();

            emit outputTextureChanged(pOutputTextureId);
            //emit outputFBOChanged(seeds.value(id)->getFBO());
        }

        resetInputSeedTexId(widget->id());
    });
}



/*QPair<QUuid, SeedWidget *> NodeManager::addSeed()
{
    Seed* seed = mRenderManager->createNewSeed();

    QUuid id = QUuid::createUuid();
    seeds.insert(id, seed);

    SeedWidget* seedWidget = new SeedWidget(seed);

    connect(seedWidget, &SeedWidget::outputChanged, this, [=, this](bool checked){
        if (checked)
        {
            setOutput(id);
            emit outputNodeChanged(seedWidget);
        }
        else
        {
            setOutput(QUuid());
            emit outputNodeChanged(nullptr);
        }
    });

    connect(this, &NodeManager::outputNodeChanged, seedWidget, &SeedWidget::toggleOutputAction);

    connect(seedWidget, &SeedWidget::connectTo, this, [=, this](){
        if (connSrcId.isNull())
            connSrcId = id;
        else if (connectOperations(connSrcId, id, 1.0))
        {
            emit nodesConnected(connSrcId, id, addEdgeWidget(connSrcId, id, 1.0));
            connSrcId = QUuid();
        }
        else
            connSrcId = QUuid();
    });

    connect(seedWidget, &SeedWidget::typeChanged, this, [=, this](){
        if (isOutput(id))
        {
            mOutputTextureId = seeds.value(id)->pOutTextureId();

            emit outputTextureChanged(mOutputTextureId);
            //emit outputFBOChanged(seeds.value(id)->getFBO());
        }

        resetInputSeedTexId(id);
    });

    connect(seedWidget, &SeedWidget::remove, this, [=, this](){
        emit nodeRemoved(id);
        removeSeed(id);
    });

    return QPair<QUuid, SeedWidget*>(id, seedWidget);
}*/



void NodeManager::copySeed(QUuid srcId)
{
    // QUuid id = QUuid::createUuid();
    Seed* seed = mSeedsMap.value(srcId);
    Seed* newSeed = new Seed(seed->type(), seed->fixed(), seed->imageFilename());

    mCopiedSeeds.insert(srcId, newSeed);
    // mIsomorphism.insert(srcId, id);
}



void NodeManager::removeSeedNode(QUuid id)
{
    if (mSeedsMap.contains(id))
    {
        mFactory->deleteSeed(mSeedsMap.value(id));
        mSeedsMap.remove(id);

        if (mOutputId == id) {
            setOutput(QUuid());
        }

        foreach (ImageOperationNode* node, mOperationNodesMap) {
            node->removeSeedInput(id);
        }

        sortOperations();
    }
}



int NodeManager::getSeedType(QUuid id)
{
    return mSeedsMap.value(id)->type();
}



void NodeManager::setSeedType(QUuid id, int set)
{
    mSeedsMap.value(id)->setType(set);
}



void NodeManager::resetInputSeedTexId(QUuid id)
{
    if (mSeedsMap.contains(id)) {
        foreach (ImageOperationNode* opNode, mOperationNodesMap) {
            opNode->setInputSeedTexId(id, mSeedsMap[id]->pOutTextureId());
        }
    }
}



bool NodeManager::isSeedFixed(QUuid id)
{
    return mSeedsMap.value(id)->fixed();
}



void NodeManager::setSeedFixed(QUuid id, bool fixed)
{
    mSeedsMap.value(id)->setFixed(fixed);
}



void NodeManager::onTexturesChanged()
{
    setOutput(mOutputId);
}



void NodeManager::setSelectedNodeIds(QList<QUuid> selNodeIds)
{
    mSelectedNodeIds = selNodeIds;
}



void NodeManager::copyNode(QUuid id)
{
    if (mOperationNodesMap.contains(id)) {
        copyOperation(id);
    }
    else if (mSeedsMap.contains(id)) {
        copySeed(id);
    }
}


void NodeManager::copySelectedNodes(QUuid id)
{
    mCopiedOperations.clear();
    mCopiedSeeds.clear();
    mCopiedInputData.clear();

    if (!mSelectedNodeIds.contains(id) || (mSelectedNodeIds.contains(id) && mSelectedNodeIds.size() == 1)) {
        copyNode(id);
    }
    else
    {
        foreach (QUuid selId, mSelectedNodeIds) {
            copyNode(selId);
        }
    }

    emit nodesCopied(mCopiedOperations, mCopiedSeeds);
}



void NodeManager::removeSelectedNodes(QUuid id)
{
    foreach (QUuid selId, mSelectedNodeIds) {
        if (selId != id) {
            emit nodeRemoved(selId);
            emit midiSignalsRemoved(selId);
            removeOperationNode(selId);
            removeSeedNode(selId);
        }
    }
}



void NodeManager::removeAllNodes()
{
    foreach (ImageOperationNode* node, mOperationNodesMap) {
        delete node;
    }
    mOperationNodesMap.clear();

    mSeedsMap.clear();
}
